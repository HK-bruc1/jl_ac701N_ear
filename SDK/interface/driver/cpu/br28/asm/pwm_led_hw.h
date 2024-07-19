#ifndef __PWM_LED_HW_H__
#define __PWM_LED_HW_H__

#include "cpu/pwm_led_debug.h"

/*******************************************************************
*   本文件为LED灯配置的接口头文件
*
*	约定:
*		1)两盏灯为单IO双LED接法;
*		2)LED0: RLED, 蓝色, 低电平亮灯;
*		3)LED1: BLED, 红色, 高电平亮灯;
*********************************************************************/

// LED实现的效果有:
// 1.两盏LED全亮;
// 2.LED单独亮灭;
// 3.LED单独慢闪和快闪;
// 4.LED 5s内单独闪一次和两次;
// 5.LED交替快闪和慢闪;
// 6.LED单独呼吸;
// 7.LED交替呼吸

#define PWM_LED_TWO_IO_ENABLE
/********************************************************************
	修改LED灯IO口驱动能力, 挡位: 0 ~ 3
		0: 2.4mA(8mA mos + 120Ωres)
		1: 8mA(8mA mos)
		2: 18.4mA(24mA mos + 120Ωres)
		3: 24mA(24mA mos)
*********************************************************************/
void pwm_led_io_max_drive_set(u8 strength);

/******************* PWM 模块是否开启 *********************/
bool is_led_module_on();

//=================================================================================//
//@brief: 自定义设置单灯闪状态
//@input: void
//		led_index: 0: led0, 1:led1, 2:led0 & led1(互闪)
//		led0_bright, LED0亮度: 0 ~ 500
// 		led1_bright, LED1亮度: 0 ~ 500
// 		led1_bright: led1亮度,
// 		period: 闪灯周期(ms), 多少ms闪一下,
// 		start_light_time: 在周期中开始亮灯的时间, -1: 周期最后亮灯
// 		light_time: 灯亮持续时间,
//@return: void
//@note:
//=================================================================================//
void pwm_led_one_flash_display(u8 led_index, u16 led0_bright, u16 led1_bright,
                               u32 period, u32 start_light_time, u32 light_time);

//=================================================================================//
//@brief: 自定义设置单灯双闪状态
//@input:
// 		led_index: 0: led0, 1:led1, 3:led0 & led1(互闪)
//		led0_bright, LED0亮度: 0 ~ 500
// 		led1_bright, LED1亮度: 0 ~ 500
// 		period: 闪灯周期(ms), 多少ms闪一下
//  	first_light_time: 第一次亮灯持续时间,
// 		second_light_time: 第二次亮灯持续时间,
// 		gap_time: 两次亮灯时间间隔,
//@return: void
//@note:
//=================================================================================//
void pwm_led_double_flash_display(u8 led_index, u16 led0_bright, u16 led1_bright,
                                  u32 period, u32 first_light_time, u32 gap_time, u32 second_light_time);


//=================================================================================//
//@brief: 自定义设置呼吸模式
//@input:
//		led_index: 0: led0, 1:led1, 2:led0 & led1(交互呼吸)
//		breathe_time: 呼吸周期(灭->最亮->灭), 设置范围: 500ms以上;
// 		led0_bright: led0呼吸到最亮的亮度(0 ~ 500);
// 		led1_bright: led1呼吸到最亮的亮度(0 ~ 500);
// 		led0_light_delay_time: led0最高亮度延时(0 ~ 100ms);
// 		led1_light_delay_time: led1最高亮度延时(0 ~ 100ms);
// 		led_blink_delay_time: led0和led1灭灯延时(0 ~ 20000ms), 0 ~ 20S;
//@return: void
//@note:
//=================================================================================//
void pwm_led_breathe_display(u8 led_index, u16 breathe_time, u16 led0_bright, u16 led1_bright,
                             u32 led0_light_delay_time, u32 led1_light_delay_time, u32 led_blink_delay_time);

//=================================================================================//
//@brief: 注册LED周期中断函数, 每个LED周期结束后会调用一次, 可以统计指定状态闪烁多少次
//@input:
//@return: void
//@note:
//=================================================================================//
void pwm_led_register_irq(void (*func)(void));

struct pwm_led_hw_ctrl {
    u8 led_id;
    u8 state; 	//0: off, 1: on
    u8 gpio;
    u8 logic; 	//0: 低电平灯亮, 1: 高电平灯亮
    u8 bright; 	//0 ~ 100级
    u16 breath_time; 	//0 ~ 5000ms
    u8 breath_rate; 	//0 ~ 20
};

struct pwm_led_hw_keep_data {

    u16 prev_breath_time;
    u8 keep_data_flag: 1; //如果是周期循环模式则需要保存数据.配置工具的ACTION必须两条以上
    u8 flash_effect_flag: 1; //如果是闪灯灯效就标记一下

};

extern struct pwm_led_hw_keep_data led_hw_keep_data;

void pwm_led_hw_display_ctrl(struct pwm_led_hw_ctrl *ctrl);

void pwm_led_hw_display_ctrl_clear(void);

void pwm_led_hw_io_enable(u8 gpio);

void led_ui_hw_timer_add(u32 period, void *handler, void *priv);

void led_ui_hw_timer_delete(void);
void pwm_led_hw_register_two_io_irq(void);
void log_pwm_led_info();
void led_pwm_pre_set();

void led_pin_set_enable(u8 gpio);

void led_pin_set_disable(u8 disable_pin);

struct pwm_led_status_t {
    u8 dir;
    u8 level;
    u16 cur_cnt;
    u16 cnt_max;
    u16 next_cnt_max;
    u32 cnt_unit;
};


void pwm_led_get_sync_status(struct pwm_led_status_t *status);
u32 pwm_led_set_sync(struct pwm_led_status_t *status, u32 how_long_ago, u32 *sync_time);

/* --------------------------------------------------------------------------*/
/**
 * @brief 关闭led模块
 */
/* --------------------------------------------------------------------------*/
void pwm_led_close(void);

void pwm_led_close_irq(void);

#endif //__PWM_LED_HW_H__


