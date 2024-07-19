#include "app_msg.h"
#include "app_main.h"
#include "bt_tws.h"
#include "battery_manager.h"
#include "asm/charge.h"
#include "led_ui_api.h"

#define LOG_TAG             "[PWM_LED]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#if (TCFG_PWMLED_ENABLE)


extern const struct led_state_map g_led_state_table[];



#define PWM_LED_IO_1T1  1
#define PWM_LED_IO_1T2  2
#define PWM_LED_IO_2T2  3

struct led_hw_config {
    u8 led_name;
    u8 gpio;
    u8 led_logic; //enum led_logic_table
    u8 bright;
};


struct pwm_led_handler {
    u8 action_index;
    u8 curr_state_name;
    u16 prev_breath_time;
    struct list_head sta_list;
};


static DEFINE_SPINLOCK(g_led_lock);
static struct pwm_led_handler *__this;


const struct led_hw_config pwm_led_table[] = {
#if TCFG_LED_BLUE_ENABLE
    { LED_BLUE, TCFG_LED_BLUE_IO, TCFG_LED_BLUE_LOGIC },
#endif
#if TCFG_LED_RED_ENABLE
    { LED_RED,  TCFG_LED_RED_IO,  TCFG_LED_RED_LOGIC },
#endif
};


static void led_ui_timer_add(void *handle, u32 timeout)
{
    led_ui_hw_timer_add(timeout, handle, NULL);
}

const struct led_hw_config *get_led_hw_cfg(u8 led_name)
{
    for (int i = 0; i < ARRAY_SIZE(pwm_led_table); i++) {
        if (pwm_led_table[i].led_name == led_name) {
            return &pwm_led_table[i];
        }
    }

    return NULL;
}

static struct led_state_obj *get_first_state_obj()
{
    struct led_state_obj *obj;
    obj = list_first_entry(&__this->sta_list, struct led_state_obj, entry);
    if (&obj->entry == &__this->sta_list) {
        return NULL;
    }
    return obj;
}

void pwm_led_status_display(const struct led_state_item *action,
                            const struct led_hw_config *hw_config)
{
    struct pwm_led_hw_ctrl ctrl;

    ctrl.led_id = hw_config->led_name;
    ctrl.state = action->on_off == 1 ? 1 : 0;
    ctrl.gpio = hw_config->gpio;
    ctrl.logic = hw_config->led_logic;
    ctrl.bright = action->brightiness;
    ctrl.breath_time = action->time_msec;
    ctrl.breath_rate = action->breathing_rate;

    if (action->breathing_rate) {
        /* 3.使用发现单io推双灯验证推2个呼吸灯效则使用了特殊模式操作，推三个则使用了使用led_ui_state_do_action函数，推灯出现重复操作现象，灯效被打断。 */
        /* 3.因此通过以下策略： */
        /* 在do_action的时候，如果第一次检测到是呼吸波形，第二次也是呼吸波形并且时间和上一次的一致就跳过不操作呼吸 */
        /* 4.目前单io和双io都支持两个以上的呼吸灯效 */
        if (__this->prev_breath_time == action->time_msec) {
            __this->prev_breath_time =  0;
            return;
        }
        __this->prev_breath_time = action->time_msec;
    }

    printf("led: %d, %d, %d\n", ctrl.state, ctrl.gpio, ctrl.logic);

    pwm_led_hw_display_ctrl(&ctrl);
}

static void led_do_next_action(void *p)
{
    struct led_state_obj *obj = (struct led_state_obj *)p;
    obj->timer = 0;
    led_ui_state_machine_run();
}

static void led_ui_state_do_action(struct led_state_obj *obj)
{
    int msg[2];
    const struct led_hw_config *hw_config;
    const struct led_state_item *status = &obj->table[__this->action_index];

    hw_config = get_led_hw_cfg(status->led_name);
    if (!hw_config) {
        r_printf("led_uuid: 0x%x, not found", status->led_name);
        return;
    }

    pwm_led_status_display(status, hw_config);

    __this->action_index++;

    //Next Action
    switch (status->action) {
    case LED_ACTION_WAIT:
        if (status->time_msec) {
            obj->timer = sys_hi_timeout_add(obj, led_do_next_action, status->time_msec);
        } else {
            led_ui_state_do_action(obj);
        }
        break;
    case LED_ACTION_CONTINUE:
        led_ui_state_do_action(obj);
        break;
    case LED_ACTION_END:
        __this->action_index = 0;
        msg[0] = LED_MSG_STATE_END;
        msg[1] = obj->callback_flag;
        spin_lock(&g_led_lock);
        __list_del_entry(&obj->entry);
        free(obj);
        spin_unlock(&g_led_lock);
        app_send_message_from(MSG_FROM_PWM_LED, 8, msg);
        break;
    case LED_ACTION_LOOP:
        __this->action_index = 0;
        if (status->time_msec) {
            obj->timer = sys_hi_timeout_add(obj, led_do_next_action, status->time_msec);
        } else {
            led_ui_state_do_action(obj);
        }
        break;
    default:
        break;
    }
}


void led_ui_state_machine_run()
{
    struct led_state_obj *obj;

    spin_lock(&g_led_lock);

    obj = get_first_state_obj();
    if (!obj) {
        goto __exit;
    }
    __this->curr_state_name = obj->name;

    if (obj->time) {
        const struct led_state_time *time = obj->time;
        const struct led_hw_config *hw_config;

        r_printf("pwm_led_period_mode: %d\n", time->mode);

        hw_config = get_led_hw_cfg(obj->led_name);
        if (!hw_config) {
            r_printf("led_uuid: 0x%x, not found", obj->led_name);
            goto __exit;
        }
        void pwm_led_hw_led_enable(u8 led_index);
        pwm_led_hw_led_enable(obj->led_name);

        pwm_led_hw_display_ctrl_clear();

        switch (time->mode) {
        case PERIOD_SINGLE_FLASH:
            printf("---single_flash: %d, %d, %d\n", time->red_bright,
                   time->period, time->light_time);
            pwm_led_one_flash_display(obj->led_name, time->red_bright, time->blue_bright,
                                      time->period, -1, time->light_time);
            led_hw_keep_data.flash_effect_flag = 1;
            break;
        case PERIOD_DOUBLE_FLASH:
            pwm_led_double_flash_display(obj->led_name, time->red_bright, time->blue_bright,
                                         time->period, time->first_light_time, time->gap_time,
                                         time->second_light_time);
            break;
        case PERIOD_SINGLE_BREATHE:
            pwm_led_breathe_display(obj->led_name, time->red_bright, time->blue_bright,
                                    time->period,
                                    time->red_light_delay_time,
                                    time->blue_light_delay_time,
                                    time->blink_delay_time);
            break;
        case PERIOD_DUAL_FLASH:
            pwm_led_one_flash_display(2, time->red_bright, time->blue_bright,
                                      time->period, -1, -1);
            break;
        default:
            break;
        }
    } else if (obj->table) {
        led_ui_state_do_action(obj);
    }

__exit:
    spin_unlock(&g_led_lock);
}

void led_ui_add_state(struct led_state_obj *s)
{
    int insert = 0;
    int machine_run = 0;
    struct led_state_obj *p, *n;

    spin_lock(&g_led_lock);

    if (s->time || s->table[s->table_size - 1].action == LED_ACTION_LOOP ||
        !(s->disp_mode & DISP_INTR_CURRENT)) {
        list_for_each_entry_safe(p, n, &__this->sta_list, entry) {
            if (p->time || p->table[p->table_size - 1].action == LED_ACTION_LOOP) {
                __list_del_entry(&p->entry);
                free(p);
                continue;
            }
        }
    }
    if (!(s->disp_mode & DISP_INTR_CURRENT)) {
        goto __add_tail;
    }

    list_for_each_entry_safe(p, n, &__this->sta_list, entry) {
        if (p->disp_mode & DISP_NON_INTR) {
            continue;
        }
        if (p->timer) {
            sys_hi_timer_del(p->timer);
            p->timer = 0;
        }
        if (!(s->disp_mode & DISP_RECOVER_CURRENT) ||
            !(p->disp_mode & DISP_RECOVERABLE)) {
            __list_del_entry(&p->entry);
            free(p);
            continue;
        }
        if (insert == 0) {
            insert = 1;
            __list_add(&s->entry, &__this->sta_list, &p->entry);
        }
    }

__add_tail:
    list_add_tail(&s->entry, &__this->sta_list);

    /* __set_status: */
    if (s->entry.prev == &__this->sta_list) {
        machine_run = 1;
    }
    spin_unlock(&g_led_lock);

    if (machine_run) {
        led_ui_state_machine_run();
    }
}

void led_ui_set_state(enum led_state_name name, enum led_disp_mode disp_mode,
                      u8 callback_flag)
{
    struct led_state_obj *obj;
    const struct led_state_map *map;

    for (int i = 0; ; i++) {
        map = &g_led_state_table[i];
        if (map->name == 0) {
            break;
        }
        if (name != map->name) {
            continue;
        }
        const u8 *arg1 = (const u8 *)map->arg1;

        obj = zalloc(sizeof(*obj));
        if (!obj) {
            return;
        }
        obj->name           = name;
        obj->disp_mode      = disp_mode;
        obj->callback_flag  = callback_flag;
        if (arg1[0] >= PERIOD_SINGLE_FLASH) {
            obj->time       = (const struct led_state_time *)arg1;
            obj->led_name   = map->arg0;
        } else {
            obj->table      = (const struct led_state_item *)arg1;
            obj->table_size = map->arg0;
            if (disp_mode == 0 &&
                obj->table[obj->table_size - 1].action != LED_ACTION_LOOP) {
                obj->disp_mode = DISP_NON_INTR | DISP_INTR_CURRENT | DISP_RECOVER_CURRENT;
            }
        }
        led_ui_add_state(obj);
        break;
    }
}


enum led_state_name led_ui_get_state_name()
{
    return __this->curr_state_name;
}

static void pwm_led_ui_action_pre(void)
{
    //切换新的灯效后需要重置该变量
    led_hw_keep_data.prev_breath_time =  0;
    //默认不需要保存数据
    led_hw_keep_data.keep_data_flag =  0;

    pwm_led_close_irq();
}

void pwm_led_state_sync_del(void);

int led_ui_do_display(u16 uuid, u8 sync)
{
    r_printf("led_ui_do_display: %d\n", uuid);
#if TCFG_USER_TWS_ENABLE
    //退出同步的资源释放
    pwm_led_state_sync_del();
#endif
    pwm_led_ui_action_pre();

    led_ui_set_state(uuid, DISP_INTR_CURRENT, 0);

    return 0;
}


void pwm_led_hw_io_set(u8 led_index, u8 gpio, u8 logic);

int pwm_led_early_init()
{
    __this = zalloc(sizeof(struct pwm_led_handler));
    INIT_LIST_HEAD(&__this->sta_list);

    for (int i = 0; i < ARRAY_SIZE(pwm_led_table); i++) {
        pwm_led_hw_io_set(pwm_led_table[i].led_name, pwm_led_table[i].gpio,
                          pwm_led_table[i].led_logic);
    }

    return 0;
}
platform_initcall(pwm_led_early_init);

#endif

