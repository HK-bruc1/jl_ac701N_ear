#include "generic/typedef.h"
#include "classic/tws_api.h"
#include "led/led_ui_manager.h"
#include "led_ui_api.h"
#include "app_config.h"
#include "system/timer.h"

#if TCFG_PWMLED_ENABLE
#if TCFG_USER_TWS_ENABLE

//这个不能够在init的时候malloc优化，因为并不知道什么时候才切灯效
u32 led_tws_state_sync_id = 0;

/* #define LED_SYNC_SWITCH_ENABLE */

/* #define LED_SYNC_SEND_DYNAMIC_TIME_ENABLE */

#ifdef LED_SYNC_SEND_DYNAMIC_TIME_ENABLE
u32 prev_sync_time = -1;
#endif

//ms
#define PLED_FISRT_SYNC_TIME        11000

//以下三个均是根据环境采样出来的经验偏差估计值，允许存在较大的误差
//经测试，人眼能够看出错开的最短时间,是亮灯时间100ms的一半
#define PLED_PERIOD_LIGHT_TIME 	50

//偏差特别小的时候，最长的一次通信同步时间
#define PLED_PERIOD_SYNC_MAX_TIME 	21000
//偏差特别大的时候，最短的通信同步时间
#define PLED_PERIOD_SYNC_MIN_TIME 	1000

/* 下面的变量可以可以init之后释放做内存空间优化 */
//只有一个变量没必要专门规划malloc内存来节省空间和降低代码灵活性，因为一直内存指针就4字节了
struct led_tws_sync_ctl_t {
    u8 led_tws_state_sync_recv_enable : 1;
    //一般就3次重试失败就重新初始化灯效同步
    u8 led_tws_state_sync_retry_cnt : 2;
} led_tws_sync_ctl;


void led_tws_state_sync(int uuid, int msec, u8 force_sync);

bool led_tws_sync_check(void)
{
    PWMLED_LOG_DEBUG("tws_api_get_tws_state = %d ,TWS_STA_SIBLING_CONNECTED = %d, tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED = %d", tws_api_get_tws_state(), TWS_STA_SIBLING_CONNECTED, tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED);
    return tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED;
}

//
#if 0
void led_tws_state_sync_by_name(char *name, int msec, u8 force_sync)
{
    u32 uuid = led_ui_JBHash((u8 *)name, strlen(name));

    led_tws_state_sync(uuid, msec, force_sync);
}
#endif //#if 0
//

//*********************************************************************************
/* 通信发起多次同步亮灯操作 */
enum {
    PWMLED_TWS_SYNC_CMD_SET  = 0,
    PWMLED_TWS_SYNC_CMD_READ,
    PWMLED_TWS_SYNC_CMD_STOP,
};


struct pwm_led_send_data_t {
    u8 cmd;
    u16 uuid;
    /* u16 lrc; */
    u32 mclkn;
    struct pwm_led_status_t status;
};



#define TWS_FUNC_ID_PLED      TWS_FUNC_ID('P', 'L', 'E', 'D')


//*********************************************************************************

//释放资源
void pwm_led_state_sync_del(void)
{

//
#if 0
    u32 rets_addr;
    __asm__ volatile("%0 = rets ;" : "=r"(rets_addr));

    PWMLED_LOG_DEBUG("rets_addr: 0x%x", rets_addr); //master: err = 1(Tx), slave: 2(Rx)
#endif //#if 0
    //

    if (led_tws_state_sync_id) {
        PWMLED_LOG_INFO("led_tws_state_sync: sys_timer_del: id=%d", led_tws_state_sync_id);
        sys_timer_del(led_tws_state_sync_id);
        led_tws_state_sync_id = 0;
    }

}
//释放+发停止包
void pwm_led_state_sync_exit(void)
{
//
#if 0
    u32 rets_addr;
    __asm__ volatile("%0 = rets ;" : "=r"(rets_addr));

    PWMLED_LOG_DEBUG("rets_addr: 0x%x", rets_addr); //master: err = 1(Tx), slave: 2(Rx)
#endif //#if 0
    //
    struct pwm_led_send_data_t pled_sync_remote_data;

    pwm_led_state_sync_del();
    pled_sync_remote_data.cmd = PWMLED_TWS_SYNC_CMD_STOP;
    pled_sync_remote_data.uuid = led_ui_get_state_name();
    //给对方发停止包
    tws_api_send_data_to_sibling(&pled_sync_remote_data, sizeof(struct pwm_led_send_data_t), TWS_FUNC_ID_PLED);
    //不再处理包
    led_tws_sync_ctl.led_tws_state_sync_recv_enable = 0;
}

static void pwmled_tws_sync_cmd_set_handle(struct pwm_led_send_data_t *pled_sync_remote_data)
{
    /* if 灯效不相同，就不支持同步,led_state_sync_handler同步灯效亮起函数保证了是相同的灯效*/

    //获取本地的蓝牙时间点(tws里面的cnt)标记
    u32 local_mclkn = tws_conn_get_mclkn(NULL);
    //tws里面的cnt 的差值转换为slot，slot *625 = 时间差(us)
    u32 how_long_ago =  bredr_clkn2offset(pled_sync_remote_data->mclkn, local_mclkn) * 625;

    //两个样机的时间差,us
    u32 sync_time = 0;
    /* 时间差和灯效同步信息写入pwm_led模块驱动 */
    s32 ret = pwm_led_set_sync(&(pled_sync_remote_data->status), how_long_ago, &sync_time);

    PWMLED_LOG_SUCC("local_mclkn：%d,>>>>>> recv remote_mclkn：%d >>>>>>", local_mclkn, pled_sync_remote_data->mclkn);
    /*PWMLED_LOG_INFO("ret = %d, how_long_ago：%dus , %ds,sync_time = %d", ret, how_long_ago, how_long_ago / 1000000, sync_time);*/

    //*********************************************************************************

    //设置失败，需要再发一包过来
    //失败就再定时用灯效周期的中间值再试一下
    if (ret == 1) {
        PWMLED_LOG_INFO("pwm_led_set_sync fail,request repeat cnt = %d", led_tws_sync_ctl.led_tws_state_sync_retry_cnt);

        led_tws_sync_ctl.led_tws_state_sync_retry_cnt ++;

        if (led_tws_sync_ctl.led_tws_state_sync_retry_cnt == 3) {
            //3次都不成功，就重新初始化灯效
            //立即删除同步定时器

            pwm_led_state_sync_del();


            PWMLED_LOG_DEBUG("pled_sync_remote_data->uuid: 0x%x", pled_sync_remote_data->uuid);

            //添加控制变量在重新初始化灯效同步的后续一段时间内，忽略不处理,后续的上一个重试流程的对方包因为发包延迟等原因导致的包
            led_tws_sync_ctl.led_tws_state_sync_recv_enable  = 0;
            //主机从机都有可能，强制发起重新初始化灯效同步
            led_tws_state_sync(pled_sync_remote_data->uuid, 300, 1);

        }

        //*********************************************************************************
        //复用变量
        pled_sync_remote_data->cmd = PWMLED_TWS_SYNC_CMD_READ;
        tws_api_send_data_to_sibling(pled_sync_remote_data, sizeof(struct pwm_led_send_data_t), TWS_FUNC_ID_PLED);

    } else {
//
#ifdef LED_SYNC_SEND_DYNAMIC_TIME_ENABLE
        /* if (get_rom_cfg_led_sync_send_dynamic_time_enable() == 1) { */

        //该处所做的同步操作是基于lrc时钟频率（不准）但是线性丢失精度的前提做的同步，如果因为环境温飘突变过大，则需要等待下一次同步周期才可以尝试运算时钟偏差补偿。

        //运算结果显示是属于快的时钟,在最小同步周期和最大同步周期之间比例取同步时间
        sync_time = sync_time / 1000;
        if (sync_time  > PLED_PERIOD_LIGHT_TIME) {
            sync_time = PLED_PERIOD_LIGHT_TIME;
        }

        u32 next_sync_time = PLED_PERIOD_LIGHT_TIME - sync_time;

        next_sync_time = next_sync_time * PLED_PERIOD_SYNC_MAX_TIME / PLED_PERIOD_LIGHT_TIME; //ms

        if (next_sync_time < PLED_PERIOD_SYNC_MIN_TIME) {
            next_sync_time = PLED_PERIOD_SYNC_MIN_TIME;
        }

        PWMLED_LOG_DEBUG("cal next_sync_time = %d", next_sync_time);
        if (prev_sync_time != next_sync_time) {
            prev_sync_time = next_sync_time;
            sys_timer_modify(led_tws_state_sync_id, next_sync_time);
            PWMLED_LOG_INFO("set next_sync_time = %d", next_sync_time);
        }

        /* } */
#endif //#if 0
        //

    }

}


static void pwm_led_state_sync_read_send(void *priv);
static void pwm_led_tws_sibling_data_deal(void *_data, u16 len, bool rx)
{
    struct pwm_led_send_data_t *pled_sync_remote_data = (struct pwm_led_send_data_t *)_data;

    if (rx) {
        PWMLED_LOG_DEBUG("pled_sync_remote_data->cmd = %d", pled_sync_remote_data->cmd);

        if (pled_sync_remote_data->cmd == PWMLED_TWS_SYNC_CMD_STOP) {
            PWMLED_LOG_INFO("PWMLED_TWS_SYNC_CMD_STOP");
            pwm_led_state_sync_del();
            led_tws_sync_ctl.led_tws_state_sync_recv_enable = 0;
            return;
        }

        if (led_tws_sync_ctl.led_tws_state_sync_recv_enable == 0) {
            PWMLED_LOG_ERR("led_tws_sync_ctl.led_tws_state_sync_recv_enable == 0");
            return ;
        }

        if (pled_sync_remote_data->uuid != led_ui_get_state_name()) {
            PWMLED_LOG_ERR("pled_sync_remote_data->uuid != led_ui_get_state_name()");
            pwm_led_state_sync_exit();
            return;
        }

        switch (pled_sync_remote_data->cmd) {
        case PWMLED_TWS_SYNC_CMD_SET:
            /*PWMLED_LOG_INFO("PWMLED_TWS_SYNC_CMD_SET");*/
            pwmled_tws_sync_cmd_set_handle(pled_sync_remote_data);
            break;

        case PWMLED_TWS_SYNC_CMD_READ:
            PWMLED_LOG_INFO("PWMLED_TWS_SYNC_CMD_READ");
            pwm_led_state_sync_read_send((void *)((u32)(pled_sync_remote_data->uuid)));

            break;


        default:
            PWMLED_LOG_INFO("not surpport cmd");
            break;
        }

    }

}


REGISTER_TWS_FUNC_STUB(pled_sync_stub) = {
    .func_id = TWS_FUNC_ID_PLED,
    .func    = pwm_led_tws_sibling_data_deal,
};



//=========================================================//
//读取本地计数-->发给对方
//=========================================================//
static void pwm_led_state_sync_read_send(void *priv)
{
    struct pwm_led_send_data_t local_data;

    if (led_tws_sync_check() == 0)  {
        PWMLED_LOG_INFO("TWS_STA_SIBLING_DISCONNECTED: sys_timer_del: id=%d", led_tws_state_sync_id);

        //如果tws断开了，就删除timer
        pwm_led_state_sync_del();
    }

    local_data.uuid = led_ui_get_state_name();
    local_data.cmd = PWMLED_TWS_SYNC_CMD_SET;

    pwm_led_get_sync_status(&(local_data.status));

    local_data.mclkn = tws_conn_get_mclkn(NULL);

    //开启处理包，现在可以处理接收到的新一轮流程的包.必须放这里，等一下timer的一段时间
    led_tws_sync_ctl.led_tws_state_sync_recv_enable = 1;

    PWMLED_LOG_SUCC(">>>>>> send local_mclkn：%d >>>>>>", local_data.mclkn);
    PWMLED_LOG_DEBUG("local_data.uuid: 0x%x", local_data.uuid);
    tws_api_send_data_to_sibling(&local_data, sizeof(struct pwm_led_send_data_t), TWS_FUNC_ID_PLED);

}


//*********************************************************************************
/* 约定发起第一次同步亮灯操作 */

/*
 * LED状态同步
 */
static void led_state_sync_handler(int uuid, int err)
{
    PWMLED_LOG_DEBUG("%s: uuid: 0x%x, err: 0x%x", __func__, uuid, err); //master: err = 1(Tx), slave: 2(Rx)

    led_ui_do_display((u16)uuid, 1);

#ifdef LED_SYNC_SWITCH_ENABLE
    return ;
#endif

    //SPECIAL_PERIOD_FLASH can tws sync，usr_timer推则不支持
    //双io推双灯和非周期灯效不支持
    if (led_hw_keep_data.flash_effect_flag != 1) {
        PWMLED_LOG_INFO("sync not support some dual IO push dual lights,not support synchronized breathing light effects,no periodic flashing lights no need"); //master: err = 1(Tx), slave: 2(Rx)
        return ;
    }

    //
#if 0
    /* if all on off 就不timer多次通信同步，同步起来一次即可 */
    if (uuid < PWM_LED_BLUE_OFF_RED_SLOW_FLASH || uuid == PWM_LED_RED_OFF_BLUE_ON) {
        PWMLED_LOG_INFO("all  on&off no need sync");
        return ;
    }
#endif //#if 0
    //

    //清除重试状态
    led_tws_sync_ctl.led_tws_state_sync_retry_cnt = 0;

#ifdef LED_SYNC_SEND_DYNAMIC_TIME_ENABLE
    /* 初始化变量 */
    prev_sync_time = -1;
#endif

    pwm_led_io_toggle = 0;

    u32 led_tws_state_sync_time = PLED_FISRT_SYNC_TIME;
    //11s通信同步一次灯效，允许systimer产生20s误差
    led_tws_state_sync_id = sys_timer_add(NULL, pwm_led_state_sync_read_send, led_tws_state_sync_time);
    PWMLED_LOG_INFO("led_tws_state_sync: sys_timer_add: id=%d, time=%d", led_tws_state_sync_id, led_tws_state_sync_time);
}

TWS_SYNC_CALL_REGISTER(tws_led_sync) = {
    .uuid = 0x2A1E3095,
    .task_name = "app_core",
    .func = led_state_sync_handler,
};


void led_tws_state_sync(int uuid, int msec, u8 force_sync)
{
    if ((tws_api_get_role() == TWS_ROLE_MASTER) || force_sync) {
        PWMLED_LOG_INFO("%s:role: %d, send sync: 0x%x, force_sync: %d", __FUNCTION__, tws_api_get_role(), uuid, force_sync);

        tws_api_sync_call_by_uuid(0x2A1E3095, uuid, msec);

    }
}

#endif
#endif

