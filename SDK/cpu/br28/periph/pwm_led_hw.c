/* #define __LOG_LEVEL  __LOG_WARN */
/*#define __LOG_LEVEL  __LOG_DEBUG*/
/*#define __LOG_LEVEL  __LOG_VERBOSE*/
#define __LOG_LEVEL  __LOG_INFO

#include "gpio.h"
#include "cpu/includes.h"
#include "asm/pwm_led_hw.h"
#include "system/timer.h"
#include "app_config.h"
#include "classic/tws_api.h"
#include "system/spinlock.h"

/*******************************************************************
*	推灯注意事项:
*		1)PWM_CON1的BIT(4), OUT_LOGIC位一定要设置为1;
*		2)PWM_CON1的BIT(0), PWM0_INV位一定要设置为0;
*		3)在非呼吸灯效果下, 单IO双LED模式, PWM_PRD_DIV寄存器和PWM_BRI_PRD设置成相同值;
*		4)在闪灯模式下, PWM_BRI_PRD(亮度值)固定下来后, PWM0的BRI_DUTY0和BRI_DUTY1一定不能超过PWM_BRI_PRD;
*********************************************************************/
/* #ifdef SUPPORT_MS_EXTENSIONS */
/* #pragma bss_seg(	".pwm_led_bss") */
/* #pragma data_seg(	".pwm_led_data") */
/* #pragma const_seg(	".pwm_led_const") */
/* #pragma code_seg(	".pwm_led_code") */
/* #endif */

//#define PWM_LED_TEST_MODE 		//LED模块测试函数

/* #define PWM_LED_DEBUG_ENABLE */
/* 这个调试宏迁移到.h进行控制 */
#ifdef PWM_LED_DEBUG_ENABLE
#define led_debug PWMLED_HW_LOG_DEBUG

#define led_err PWMLED_HW_LOG_ERR

/* #define led_debug(fmt, ...) 	g_printf("[PWM_LED] "fmt, ##__VA_ARGS__) */
/* #define led_err(fmt, ...) 		r_printf("[PWM_LED_ERR] "fmt, ##__VA_ARGS__) */
#else
#define led_debug(...)
#define led_err(...)
#endif

//////////////////////////////////////////////////////////////

#define PWMLED_IO_NUMBER   2

#define PWM_LED_SFR 	SFR

//========================= 0.模块开关相关寄存器
#define LED_PWM_ENABLE 					(JL_PLED->CON0 |= BIT(0))
#define LED_PWM_DISABLE 				(JL_PLED->CON0 &= ~BIT(0))
#define IS_PWM_LED_ON    				(JL_PLED->CON0 & BIT(0))

#define RESET_PWM_CON0 					(JL_PLED->CON0 = 0)
#define RESET_PWM_CON1 					(JL_PLED->CON1 = 0)
#define RESET_PWM_CON2 					(JL_PLED->CON2 = 0)
#define RESET_PWM_CON3 					(JL_PLED->CON3 = 0)

//========================= 1.时钟设置相关寄存器
//PWM0 CLK DIV
#define	CLK_DIV_1 			0
#define CLK_DIV_4           1
#define CLK_DIV_16          2
#define CLK_DIV_64          3

#define CLK_DIV_x2(x)       (0x04 | x)
#define CLK_DIV_x256(x)		(0x08 | x)
#define CLK_DIV_x2_x256(x)	(0x0c | x)
//SOURCE
//SOURCE -- DIV  --> PWM0
//[1:0] 00:div1  01:div4  10:div16  11:div64
//[2]:  0:x1     1:x2
//[3]:  0:x1     1:x256
//DIV = [1:0] * [2] * [3]
#define LED_PWM0_CLK_DIV(x)			PWM_LED_SFR(JL_PLED->CON0, 4, 4, x)
//PWM0 -- DIV --> PWM1
//PWM_CON3[3:0] -- PWM_PRD_DIVL[7:0]  //12bit
#define LED_PWM1_CLK_DIV(x)	   	    	{do {JL_PLED->PRD_DIVL = ((x-1) & 0xFF); JL_PLED->CON3 &= ~(0xF); JL_PLED->CON3 |= (((x-1) >> 8) & 0xF);} while(0);}

//========================= 2.亮度设置相关寄存器
//最高亮度级数设置
//PWM_BRI_PRDH[1:0] -- PWM_BRI_PRDL[7:0]  //10bit
#define LED_BRI_DUTY_CYCLE(x)			{do {JL_PLED->BRI_PRDL = (x & 0xFF); JL_PLED->BRI_PRDH = ((x >> 8) & 0x3);} while(0);}
//高电平亮灯亮度设置
#define LED_BRI_DUTY1_SET(x)			{do {JL_PLED->BRI_DUTY0L = (x & 0xFF); JL_PLED->BRI_DUTY0H = ((x >> 8) & 0x3);} while(0);}
//低电平亮灯亮度设置
#define LED_BRI_DUTY0_SET(x)			{do {JL_PLED->BRI_DUTY1L = (x & 0xFF); JL_PLED->BRI_DUTY1H = ((x >> 8) & 0x3);} while(0);}
//同步LED0 <-- LED1亮度
#define LED_BRI_DUTY0_SYNC1() 			{do {JL_PLED->BRI_DUTY0L = JL_PLED->BRI_DUTY1L; JL_PLED->BRI_DUTY0H = JL_PLED->BRI_DUTY1H;} while(0);}
//同步LED1 <-- LED0亮度
#define LED_BRI_DUTY1_SYNC0() 			{do {JL_PLED->BRI_DUTY1L = JL_PLED->BRI_DUTY0L; JL_PLED->BRI_DUTY1H = JL_PLED->BRI_DUTY0H;} while(0);}


//========================= 3.亮灭设置相关寄存器
//duty_cycle 固定为255或者设置PWM_DUTY3, 8bit
//   	 	  _duty1_	 	  _duty3_
//		      |     |         |     |
//		      |     |         |     |
//0__ __ duty0|     | __ duty2|     |__ __255/PWM_DUTY3

//PWM1 duty_cycle 选择
#define LED_PWM1_DUTY_FIX_SET 			(JL_PLED->CON1 &= ~BIT(3))  //固定PWM1周期为0xFF
#define LED_PWM1_DUTY_VARY_SET(x) 		{do {JL_PLED->CON1 |= BIT(3); JL_PLED->DUTY3 = x;} while(0);}

#define LED_PWM1_DUTY0_SET(x)			(JL_PLED->DUTY0 = x)
#define LED_PWM1_DUTY1_SET(x)			(JL_PLED->DUTY1 = x)
#define LED_PWM1_DUTY2_SET(x)			(JL_PLED->DUTY2 = x)
#define LED_PWM1_DUTY3_SET(x)			(JL_PLED->DUTY3 = x) 	//可以设置为PWM1_DUTY_CYCLE
#define LED_PWM1_PRD_SEL_DIS			(JL_PLED->CON1 &= ~BIT(3))

#define LED_PWM1_PRD_SEL_EN				(JL_PLED->CON1 |= BIT(3))
#define LED_PWM1_DUTY0_EN				(JL_PLED->CON1 |= BIT(4))
#define LED_PWM1_DUTY1_EN				(JL_PLED->CON1 |= BIT(5))
#define LED_PWM1_DUTY2_EN				(JL_PLED->CON1 |= BIT(6))
#define LED_PWM1_DUTY3_EN				(JL_PLED->CON1 |= BIT(7))

#define LED_PWM1_ALL_DUTY_DIS			(JL_PLED->CON1 &= ~(0xF << 4))
#define LED_PWM1_DUTY0_DIS				(JL_PLED->CON1 &= ~BIT(4))
#define LED_PWM1_DUTY1_DIS				(JL_PLED->CON1 &= ~BIT(5))
#define LED_PWM1_DUTY2_DIS				(JL_PLED->CON1 &= ~BIT(6))
#define LED_PWM1_DUTY3_DIS				(JL_PLED->CON1 &= ~BIT(7))

//========================= 5.输出取反相关寄存器
//以下几个需要设置为固定值
#define LED_PWM0_INV_DISABLE 			(JL_PLED->CON1 &= ~BIT(0))
#define LED_PWM1_INV_DISABLE 			(JL_PLED->CON1 &= ~BIT(1)) //周期从灭灯开始
#define LED_PWM1_INV_ENABLE 			(JL_PLED->CON1 |= BIT(1))  //周期从亮灯开始
#define LED_PWM_OUT_LOGIC_SET 			(JL_PLED->CON3 |= BIT(4))
//以下几个可以灵活设置
#define LED_PWM_OUTPUT_INV_ENABLE 		(JL_PLED->CON1 |= BIT(2))
#define LED_PWM_OUTPUT_INV_DISABLE 		(JL_PLED->CON1 &= ~BIT(2))

//========================= 6.与周期变色相关寄存器
#define LED_PWM_SHIFT_DUTY_SET(x) 		PWM_LED_SFR(JL_PLED->CON2, 4, 4, x)

//========================= 7.与驱动强度相关寄存器
#define LED_PWM_IO_MAX_DRIVE(x) 		PWM_LED_SFR(JL_PLED->CON2, 0, 2, x)

//========================= 8.与中断相关寄存器
#define LED_PWM_INT_EN 					(JL_PLED->CON3 |= BIT(5))
#define LED_PWM_INT_DIS 				(JL_PLED->CON3 &= ~BIT(5))
#define LED_PWM_CLR_PENDING 			(JL_PLED->CON3 |= BIT(6))

//========================= 9.与呼吸模式相关寄存器
#define LED_PWM_BREATHE_ENABLE 			(JL_PLED->CON0 |= BIT(1))
#define LED_PWM_BREATHE_DISABLE 		(JL_PLED->CON0 &= ~BIT(1))
//LED呼吸灭灯延时设置, 16bit
#define LED_PWM_BREATHE_BLANK_TIME_SET(x) 	{do {LED_PWM1_DUTY2_EN; LED_PWM1_DUTY3_EN; LED_PWM1_DUTY2_SET((x & 0xFF)); LED_PWM1_DUTY3_SET((x >> 8) & 0xFF)} while(0)}
//LED呼吸灯(低电平灯)最高亮度延时设置, 8bit
#define LED0_PWM_BREATHE_LIGHT_TIME_SET(x) 	{do {LED_PWM1_DUTY0_EN; LED_PWM1_DUTY0_SET(x)} while(0)}
//LED呼吸灯(高电平灯)最高亮度延时设置, 8bit
#define LED1_PWM_BREATHE_LIGHT_TIME_SET(x) 	{do {LED_PWM1_DUTY1_EN; LED_PWM1_DUTY1_SET(x)} while(0)}


#define PLED_CNT_READ_KICK_START() 		(JL_PLED->CON4 |= BIT(4))
#define PLED_CNT_READ_WAIT_READY() 		while(!(JL_PLED->CON4 & BIT(5)))
#define PLED_GET_READ_CNT() 		(JL_PLED->CNT_RD)
#define PLED_LOCAL_SYNC_REMOTE(v) 	(JL_PLED->CNT_SYNC = v)
#define PLED_LOCAL_SYNC_DEC_CNT(v) 	(JL_PLED->CNT_DEC = v)

enum pwm_led_clk_source {
    PWM_LED_CLK_RESERVED0,  //PWM_LED_CLK_OSC32K, no use
    PWM_LED_CLK_RC32K, 		//use
    PWM_LED_CLK_RESERVED1,  //PWM_LED_CLK_BTOSC_12M, no use
    PWM_LED_CLK_RESERVED2, 	//PWM_LED_CLK_RCLK_250K, no use
    PWM_LED_CLK_BTOSC_24M,  //use
};

#define PLED_CNT_READ_KICK_START() 		(JL_PLED->CON4 |= BIT(4))
#define PLED_CNT_READ_WAIT_READY() 		while(!(JL_PLED->CON4 & BIT(5)))
#define PLED_GET_READ_CNT() 		(JL_PLED->CNT_RD)
#define PLED_LOCAL_SYNC_REMOTE(v) 	(JL_PLED->CNT_SYNC = v)
#define PLED_LOCAL_SYNC_DEC_CNT(v) 	(JL_PLED->CNT_DEC = v)

struct pwm_led {
    u8 clock;
    //中断相关
    void (*pwm_led_extern_isr)(void);
    void (*pwm_led_local_isr)(void);
};

enum _PWM0_CLK {
    PWM0_CLK_32K, //for rc normal
    //BT24M
    PWM0_CLK_46K, //24M / 512 = 46875, for normal period < 22s
    PWM0_CLK_23K, //24M / 1024 = 23437, for normal period > 22s
};

//#define PWM0_RC_CLK32K_VALUE 	32000
extern u32 __get_lrc_hz(void);
#define PWM0_RC_CLK32K_VALUE 	(__get_lrc_hz() / 4)
#define PWM0_BT_CLK23K_VALUE 	23437
#define PWM0_BT_CLK46K_VALUE 	46875

#define PWM_LED_USER_DEFINE_MODE 0xFE

static struct pwm_led _led;

#define __this 		(&_led)

//=================================================================================//
// P11_P2M_CLK_CON0[7 : 5]:
// 000: None;
// 001: RC_250K, open @ P3_ANA_CON0[7];
// 010: RC_16M, open @ P11_CLK_CON0[1];
// 011: LRC_OSC_200K, open @ P3_LRC_CON0[0]
// 100: RTC_OSC;
// 101: SYS_CLK;
//=================================================================================//
static void pwm_clock_set(u8 _clock)
{
    u8 clock = _clock;
    u8 clk_val = 0;

    switch (clock) {
    case PWM_LED_CLK_RC32K:
        //RC_32K
        clk_val = 0b011; //LRC_OSC_200K
        /* P3_LRC_CON0 |= BIT(0); //Enable this clock */

        //RC_16M
        /* clk_val = 0b010; */
        /* P11_CLK_CON0 |= BIT(1); //Enable this clock */

        //RC_250K
        /* clk_val = 0b001; */
        /* P3_ANA_CON0 |= BIT(7); //Enable this clock */
        break;
    case PWM_LED_CLK_BTOSC_24M:
        //SYS_CLK:
        clk_val = 0b101;
        break;

    default:
        break;
    }

    //MO Note: Make sure P11 part have been patch key
    P11_P2M_CLK_CON0 &= ~(0b111 << 5);
    P11_P2M_CLK_CON0 |= (clk_val << 5);
    __this->clock = clock;
    PWMLED_HW_LOG_DEBUG("clock = 0x%x, clk_val= %d, P11_P2M_CLK_CON0 = 0x%x", __this->clock, clk_val, ((P11_P2M_CLK_CON0 >> 5) & 0b111));
}

//提供给低功耗系统判断是否sniff下需要提高电压，解决灯抖动的问题
u8 is_pwm_led_on()
{
    return (IS_PWM_LED_ON) ? 1 : 0;
}

enum {
    PWM_LED_HW_STATE_OFF,
    PWM_LED_HW_STATE_ON,
    PWM_LED_HW_STATE_BREATHE,
};

struct logic_state {
    //驱动的状态
    u8 state; 	//0: 灭灯, 1: 亮灯, 2: 呼吸
    //驱动的亮度
    u8 bright;
    /* 驱动的io */
    u8 io;
    u8 logic;
    u8 io_en: 1;
};


struct pwm_led_hw {
    u8 init: 1;
    u8 recover_flag: 1; //如果两个灯都处于灭的状态，就彻底关闭模块和IO设高阻，节省功耗。但是下次使用灯效需要根据logic_state恢复相应的模块设置。

    //记录当前灯的驱动状态
    struct logic_state logic_tbl[PWMLED_IO_NUMBER];
};

static struct pwm_led_hw led_hw = {0};

struct pwm_led_hw_keep_data led_hw_keep_data = {0};



/*
 * IO使能注意, 否则有可能会出现在显示效果前会出现闪烁
 * 1)设置PU, PD为1;
 * 2)设置DIR, OUT为1;
 * 3)最后设置为方向为输出;
 */
void led_pin_set_enable(u8 gpio)
{

#ifdef PWM_LED_DEBUG_ENABLE
    u32 rets_addr;
    __asm__ volatile("%0 = rets ;" : "=r"(rets_addr));
    PWMLED_HW_LOG_DEBUG("rets_addr :0x%x", rets_addr);
#endif

    PWMLED_HW_LOG_DEBUG("led pin set enable = %d", gpio);
    CPU_CRITICAL_ENTER();
    u8 tmp = IS_PWM_LED_ON;
    LED_PWM_DISABLE; //防止切IO时模块在工作会有闪一下的现象
    gpio_set_function(IO_PORT_SPILT(gpio), PORT_FUNC_PWM_LED);
    if (tmp) {
        LED_PWM_ENABLE;
    }
    CPU_CRITICAL_EXIT();

    for (u8 io_count = 0; io_count < PWMLED_IO_NUMBER; io_count ++) {
        if (led_hw.logic_tbl[io_count].io == gpio) {
            led_hw.logic_tbl[io_count].io_en = 1;
        }
    }
}

//把IO设置为高阻
void led_pin_set_disable(u8 disable_pin)
{

#ifdef PWM_LED_DEBUG_ENABLE
    u32 rets_addr;
    __asm__ volatile("%0 = rets ;" : "=r"(rets_addr));
    PWMLED_HW_LOG_DEBUG("rets_addr :0x%x", rets_addr);
#endif

    PWMLED_HW_LOG_DEBUG("led pin set disable = %d", disable_pin);


    gpio_disable_function(disable_pin / 16, BIT(disable_pin % 16), PORT_FUNC_PWM_LED);
    if (led_hw.recover_flag == 0) {
        for (u8 io_count = 0; io_count < PWMLED_IO_NUMBER; io_count ++) {
            if (led_hw.logic_tbl[io_count].io == disable_pin) {
                led_hw.logic_tbl[io_count].io_en = 0;
            }
        }
    }

}



static void _pwm_led_close_all_recover(void)
{
#ifdef PWM_LED_DEBUG_ENABLE
    u32 rets_addr;
    __asm__ volatile("%0 = rets ;" : "=r"(rets_addr));
    PWMLED_HW_LOG_DEBUG("rets_addr :0x%x", rets_addr);
#endif

    PWMLED_HW_LOG_DEBUG("%s\n", __func__);

    CPU_CRITICAL_ENTER();

    for (u8 io_count = 0; io_count < PWMLED_IO_NUMBER; io_count ++) {
        if (led_hw.logic_tbl[io_count].io_en == 1) {
            led_pin_set_enable(led_hw.logic_tbl[io_count].io);
        }
    }

    CPU_CRITICAL_EXIT();

}

void _pwm_led_close_all(void)
{
#ifdef PWM_LED_IO_TEST_SDK
    LED_TIMER_TEST_PORT1->OUT ^= BIT(LED_TIMER_TEST_PIN1);
#endif

    PWMLED_HW_LOG_DEBUG("%s\n", __func__);


    CPU_CRITICAL_ENTER();

    LED_PWM_DISABLE;
    LED_PWM_BREATHE_DISABLE;

    for (u8 io_count = 0; io_count < PWMLED_IO_NUMBER; io_count ++) {
        led_pin_set_disable(led_hw.logic_tbl[io_count].io);
    }

    CPU_CRITICAL_EXIT();


    /* _led_pwm_bright_set(0xFF, 0, 0); */

#ifdef PWM_LED_IO_TEST_SDK
    LED_TIMER_TEST_PORT1->OUT ^= BIT(LED_TIMER_TEST_PIN1);
#endif
}


#ifdef PWM_LED_TWO_IO_ENABLE

static u8 led_irq_active = 0;
/* --------------------------------------------------------------------------*/
/**
 *
 * @return 0：不可进
 *         1：可进
 */
/* ----------------------------------------------------------------------------*/
static u8 led_irq_idle_query(void)
{
    return (led_irq_active == 0);
}

REGISTER_LP_TARGET(led_irq_target) = {
    .name = "led_irq",
    .is_idle = led_irq_idle_query,
};

u16 pwm_led_wakeup_time_id = 0;
void pwm_led_wait_irq_timer(void *priv)
{
    //定义变量等待led的irq起来才允许进入低功耗流程，避免再次被低功耗屏蔽led的中断
    led_irq_active = 1;
    pwm_led_wakeup_time_id = 0;
}

void pwm_led_timer_out_wakeup_add(void)
{

    struct pwm_led_status_t led_wake_data;

    //读取当前的led模块的计数值状态
    pwm_led_get_sync_status(&led_wake_data);

    //计算距离下一次亮灯周期的时间
    u32 pwm_led_wakeup_time = (led_wake_data.cnt_max - led_wake_data.cur_cnt) * led_wake_data.cnt_unit / 1000;

    //第一次肯定是led_isr先起来，后面循环周期就是usr_timer先起来
    //这里的-5ms是为了usr_timer先起来
    if (pwm_led_wakeup_time > 5) {
        pwm_led_wakeup_time -= 5;
    }

    if (pwm_led_wakeup_time_id) {

        usr_timeout_modify(pwm_led_wakeup_time_id, pwm_led_wakeup_time);
    } else {
        //定义usr_timer在下一次亮灯周期前唤醒，等待led的irq来切灯
        pwm_led_wakeup_time_id = usr_timeout_add(NULL, pwm_led_wait_irq_timer, pwm_led_wakeup_time, 1);

    }

    //irq已经起来,清除变量
    led_irq_active = 0;
//
#if 0
    printf("cnt_max=%d", led_wake_data.cnt_max);
    printf("cur_cnt=%d", led_wake_data.cur_cnt);
    printf("cnt_unit=%d", led_wake_data.cnt_unit);
    printf("pwm_led_wakeup_time=%d", pwm_led_wakeup_time);
#endif //#if 0
    //

}

//
#if 1
u8 pwm_led_io_toggle = 0;
#endif //#if 0
//
___interrupt
void pwm_led_hw_isr_func(void)
{
    LED_PWM_CLR_PENDING;
//
#if 0
    pwm_led_io_toggle = !pwm_led_io_toggle;
    if (pwm_led_io_toggle) {

        /* void led_pin_set_disable(u8 disable_pin) */
        JL_PORTC->DIR |= BIT(2);
        JL_PORTC->PU  &= ~BIT(2);
        JL_PORTC->PD  &= ~BIT(2);

        JL_PORTC->DIR &= ~BIT(3);
        JL_PORTC->PU  |= BIT(3);
        JL_PORTC->PD  |= BIT(3);

    } else {
        JL_PORTC->DIR |= BIT(3);
        JL_PORTC->PU  &= ~BIT(3);
        JL_PORTC->PD  &= ~BIT(3);

        JL_PORTC->DIR &= ~BIT(2);
        JL_PORTC->PU  |= BIT(2);
        JL_PORTC->PD  |= BIT(2);

    }
    PWMLED_HW_LOG_DEBUG("pwm_led_io_toggle: %d", pwm_led_io_toggle);
#endif //#if 0
    //

    pwm_led_io_toggle = !pwm_led_io_toggle;
    //如果led0 不亮，则亮
    if (pwm_led_io_toggle) {
        led_pin_set_enable(led_hw.logic_tbl[0].io);
        led_pin_set_disable(led_hw.logic_tbl[1].io);
    } else {

        led_pin_set_enable(led_hw.logic_tbl[1].io);
        led_pin_set_disable(led_hw.logic_tbl[0].io);
    }

    PWMLED_HW_LOG_DEBUG("pwm_led_io_toggle: %d", pwm_led_io_toggle);

#if (TCFG_LOWPOWER_LOWPOWER_SEL == 1)
    //因为led_isr无法唤醒低功耗，需要定义usr_timer来提前唤醒低功耗
    pwm_led_timer_out_wakeup_add();
#endif

}

void pwm_led_hw_register_two_io_irq(void)
{
    PWMLED_HW_LOG_DEBUG("pwm_led_hw_register_two_io_irq");

    LED_PWM_INT_DIS;
    LED_PWM_CLR_PENDING;
    request_irq(IRQ_PWM_LED_IDX, 1, pwm_led_hw_isr_func, 0);

    LED_PWM_INT_EN;
}

#endif

___interrupt
static void pwm_led_isr_func(void)
{
    LED_PWM_CLR_PENDING;
    PWMLED_HW_LOG_DEBUG("led isr");
    if (__this->pwm_led_extern_isr) {
        __this->pwm_led_extern_isr();
    }
    if (__this->pwm_led_local_isr) {
        __this->pwm_led_local_isr();
    }
}

//index = 1: 内部切IO中断;
//index = 0: 外部注册中断;
static void _pwm_led_register_irq(void (*func)(void), u8 index)
{
    LED_PWM_INT_DIS;
    LED_PWM_CLR_PENDING;

    if (func) {
        if (index) {
            __this->pwm_led_local_isr = func;
        } else {
            __this->pwm_led_extern_isr = func;
        }
        request_irq(IRQ_PWM_LED_IDX, 1, pwm_led_isr_func, 0);
        LED_PWM_INT_EN;
    }
}


static void _pwm_led_close_irq(void)
{
    unrequest_irq(IRQ_PWM_LED_IDX);
    LED_PWM_INT_DIS;

    __this->pwm_led_extern_isr = NULL;
    __this->pwm_led_local_isr = NULL;
}

void pwm_led_close_irq(void)
{
    PWMLED_HW_LOG_DEBUG("pwm_led_close_irq");
    _pwm_led_close_irq();
}

void log_pwm_led_info()
{
#ifdef PWM_LED_DEBUG_ENABLE
#if 1
    u32 rets_addr;
    __asm__ volatile("%0 = rets ;" : "=r"(rets_addr));

    printf("rets_addr :0x%x", rets_addr);

    printf("======== PWM LED CONFIG ======");
    printf("P11_P2M_CLK_CON0 = 0x%x, lrc = %d", P11_P2M_CLK_CON0, __get_lrc_hz());
    printf("JL_PLED->CON0 	= 0x%x", JL_PLED->CON0);
    printf("JL_PLED->CON1 	= 0x%x", JL_PLED->CON1);
    printf("JL_PLED->CON2 	= 0x%x", JL_PLED->CON2);
    printf("JL_PLED->CON3 	= 0x%x", JL_PLED->CON3);
    printf("JL_PLED->PRD_DIVL     = 0x%x", JL_PLED->PRD_DIVL);

    printf("JL_PLED->BRI_PRDL      = 0x%x", JL_PLED->BRI_PRDL);
    printf("JL_PLED->BRI_PRDH      = 0x%x", JL_PLED->BRI_PRDH);

    printf("JL_PLED->BRI_DUTY0L    = 0x%x", JL_PLED->BRI_DUTY0L);
    printf("JL_PLED->BRI_DUTY0H    = 0x%x", JL_PLED->BRI_DUTY0H);

    printf("JL_PLED->BRI_DUTY1L    = 0x%x", JL_PLED->BRI_DUTY1L);
    printf("JL_PLED->BRI_DUTY1H    = 0x%x", JL_PLED->BRI_DUTY1H);

    printf("JL_PLED->DUTY0    = 0x%x", JL_PLED->DUTY0);
    printf("JL_PLED->DUTY1    = 0x%x", JL_PLED->DUTY1);
    printf("JL_PLED->DUTY2    = 0x%x", JL_PLED->DUTY2);
    printf("JL_PLED->DUTY3    = 0x%x", JL_PLED->DUTY3);

    printf("JL_PORTC->DIR = 0x%x", JL_PORTC->DIR);
    printf("JL_PORTC->OUT = 0x%x", JL_PORTC->OUT);
    printf("JL_PORTC->PU0 = 0x%x", JL_PORTC->PU);
    printf("JL_PORTC->PD0 = 0x%x", JL_PORTC->PD);
    printf("JL_PORTC->DIE = 0x%x", JL_PORTC->DIE);
    printf("JL_PORTC->DIEH = 0x%x", JL_PORTC->DIEH);

    printf("JL_PORTC->IN = 0x%x", JL_PORTC->IN);
    printf("JL_PORTC->OUT = 0x%x", JL_PORTC->OUT);
    printf("JL_PORTC->DIR = 0x%x", JL_PORTC->DIR);
    printf("JL_PORTC->HD0 = 0x%x", JL_PORTC->HD0);
    printf("JL_PORTC->HD = 0x%x", JL_PORTC->HD);
#endif
#endif
}


/**
 * @brief: pwm0 时钟设置
 * 默认RC = 32K,
 * 呼吸BT24M = 93750Hz
 * 普通BT24M = 46875Hz/23437Hz
 *
 * @param: clk0
 * @return int
 */
static int _led_pwm0_clk_set(enum _PWM0_CLK clk0)
{
    u8 pwm0_clk_div_val = 0;

    if (__this->clock == PWM_LED_CLK_RC32K) {
        if (clk0 != PWM0_CLK_32K) {
            return -1;
        }
        //RC32k div 1 = 32k
        //pwm0_clk_div_val = CLK_DIV_1;
        //RC200k div 4 = 50k
        pwm0_clk_div_val = CLK_DIV_4;

        //RC16M div 512 = 32k
        /* pwm0_clk_div_val = CLK_DIV_x2(CLK_DIV_1) | CLK_DIV_x256(CLK_DIV_1); */

        //RC250K div 8 = 32k
        /* pwm0_clk_div_val = CLK_DIV_x2(CLK_DIV_4); */
    } else if (__this->clock == PWM_LED_CLK_BTOSC_24M) {
#if CONFIG_FPGA_ENABLE
        //12M
        if (clk0 == PWM0_CLK_46K) {
            pwm0_clk_div_val = CLK_DIV_x256(CLK_DIV_1);  //12M div 256 = 46875Hz
        } else if (clk0 == PWM0_CLK_23K) {
            pwm0_clk_div_val = CLK_DIV_x256(CLK_DIV_x2(CLK_DIV_1));  //12M div 512 = 23437Hz
        } else {
            return -1;
        }
#else
        if (clk0 == PWM0_CLK_46K) {
            pwm0_clk_div_val = CLK_DIV_x256(CLK_DIV_x2(CLK_DIV_1));  //24M div 512 = 46875Hz
        } else if (clk0 == PWM0_CLK_23K) {
            pwm0_clk_div_val = CLK_DIV_x256(CLK_DIV_4);  //24M div 1024 = 23437Hz
        } else {
            return -1;
        }
#endif /* #if CONFIG_FPGA_ENABLE */
    }

    LED_PWM0_CLK_DIV(pwm0_clk_div_val);

    return 0;
}



void led_pwm_pre_set()
{
    PWMLED_HW_LOG_DEBUG("%s\n", __func__);
    LED_PWM_DISABLE;
    LED_PWM_BREATHE_DISABLE;
}


/////////////////////////////
/**
 * @brief: 设置led灯亮度
 * @param: bri_max, 最大亮度, 10bit, 0 ~ 1023
 * @param: bri_duty0, LED0亮度, 10bit, 0 ~ 1023
 * @param: bri_duty1, LED1亮度, 10bit, 0 ~ 1023
 *
 * @return void
 */
static void _led_pwm_bright_set(u16 bri_max, u16 bri_duty0, u16 bri_duty1)
{
    bri_max = bri_max >= 1024 ? 1023 : bri_max;
    bri_duty0 = bri_duty0 >= 1024 ? 1023 : bri_duty0;
    bri_duty1 = bri_duty1 >= 1024 ? 1023 : bri_duty1;

    bri_duty0 = bri_duty0 >= bri_max ? bri_max : bri_duty0;
    bri_duty1 = bri_duty1 >= bri_max ? bri_max : bri_duty1;

    LED_BRI_DUTY_CYCLE(bri_max);
    LED_BRI_DUTY0_SET(bri_duty0);
    LED_BRI_DUTY1_SET(bri_duty1);
}


static void _led_pwm_bright_set_by_logic(u8 logic, u16 bri_max, u16 bri_duty)
{
    bri_max = bri_max >= 1024 ? 1023 : bri_max;
    bri_duty = bri_duty >= 1024 ? 1023 : bri_duty;

    LED_BRI_DUTY_CYCLE(bri_max);
    if (logic == 0) {
        LED_BRI_DUTY0_SET(bri_duty);
    } else {
        LED_BRI_DUTY1_SET(bri_duty);
    }
}


/**
 * @brief: 设置PWM输出逻辑,
 * @param: pwm_inv_en, 最后pwm波形输出逻辑(默认是高电平灯亮),
			0: 不取反, 高电平灯亮; 1: 取反, 低电平灯亮;
 * @param: shift_num
 	是否需要互闪,
			0: 单闪; 1 ~ : 互闪;
 *
 * @return void
 */
static void _led_pwm_output_logic_set(u8 pwm_inv_en, u8 shift_num)
{
    if (pwm_inv_en) {
        LED_PWM_OUTPUT_INV_ENABLE;
    } else {
        LED_PWM_OUTPUT_INV_DISABLE;
    }
    LED_PWM_OUT_LOGIC_SET;
    LED_PWM_SHIFT_DUTY_SET(shift_num);
}

/**
 * @brief: 设置PWM1亮灭设置, 可以实现一个周期亮灭1次和2次,
 * @param: pwm_inv_en, 最后pwm波形输出逻辑(默认是高电平灯亮),
			0: 不取反, 高电平灯亮; 1: 取反, 低电平灯亮;
 * @param: shift_num
 	是否需要互闪,
			0: 单闪; 1 ~ : 互闪;
 *duty_cycle 固定为255或者设置PWM_DUTY3, 8bit
 *   	 	  _duty1_	 	  _duty3_
 *		      |     |         |     |
 *		      |     |         |     |
 *0__ __ duty0|     | __ duty2|     |__ __255/PWM_DUTY3
 * @return void
 */
static void _led_pwm1_duty_set(u8 duty_prd, u8 duty0, u8 duty1, u8 duty2, u8 breathe)
{
    if (duty_prd != 0xFF) {
        if (breathe == 0) {
            duty0 = duty0 > duty_prd ? duty_prd : duty0;
            duty1 = duty1 > duty_prd ? duty_prd : duty1;
            duty2 = duty2 > duty_prd ? duty_prd : duty2;
            LED_PWM1_PRD_SEL_EN;
            LED_PWM1_DUTY3_DIS;
        } else {
            LED_PWM1_PRD_SEL_DIS; //呼吸模式, duty0/1是亮灯延时, {duty3,duty2}是灭灯延时
            LED_PWM1_DUTY3_EN;
        }

        LED_PWM1_DUTY3_SET(duty_prd);
    } else {
        LED_PWM1_PRD_SEL_DIS;
        LED_PWM1_DUTY3_DIS;
    }
    if (duty0) {
        LED_PWM1_DUTY0_SET(duty0);
        LED_PWM1_DUTY0_EN;
    } else {
        LED_PWM1_DUTY0_DIS;
    }
    if (duty1) {
        LED_PWM1_DUTY1_SET(duty1);
        LED_PWM1_DUTY1_EN;
    } else {
        LED_PWM1_DUTY1_DIS;
    }
    if (duty2) {
        LED_PWM1_DUTY2_SET(duty2);
        LED_PWM1_DUTY2_EN;
    } else {
        LED_PWM1_DUTY2_DIS;
    }
}


/**
 * @brief:
 * @param: module_en = 0, breathe_en = 0, 关LED模块
 * @param: module_en = 0, breathe_en = 1, 关LED模块
 * @param: module_en = 1, breathe_en = 0, 开LED普通闪烁模式
 * @param: module_en = 1, breathe_en = 1, 开LED呼吸模式
 * @return void
 */
static void _led_pwm_module_enable(u8 module_en, u8 breathe_en)
{

    PWMLED_HW_LOG_DEBUG("module_en = %d, breathe_en = %d", module_en, breathe_en);

    if (breathe_en) {
        //LED_PWM1_INV_ENABLE; //亮灯开始
        LED_PWM_BREATHE_ENABLE;
    } else {
        //LED_PWM1_INV_DISABLE; //灭灯开始
        LED_PWM_BREATHE_DISABLE;
    }

    if (module_en) {
        LED_PWM_ENABLE;
    } else {
        LED_PWM_DISABLE;
    }

    log_pwm_led_info();
}

/**
 * @brief: 设置pwm0时钟分频
 *          clk0_div
 * pwm0_clk --------> pwm1_clk
 * @param: clk0_div, 12bit, 0 ~ 4095
 *
 * @return void
 */
static void _led_pwm1_clk_set(u16 clk0_div)
{
    clk0_div = clk0_div > 4096 ? 4096 : clk0_div;
    LED_PWM1_CLK_DIV(clk0_div);
}

/**
 * @brief: 关 led 配置, 亮度也设置为0
 *
 * @param led_index: 0: led0, 1:led1, 2:led0 & led1
 * @param led0_bright: 0 ~ 100
 * @param led1_bright: 0 ~ 100
 *
 * @return void
 */
static void _pwm_led_off_display(void)
{
    PWMLED_HW_LOG_DEBUG("%s\n", __func__);
    //TODO: set bright duty 0: avoid set on case
    led_pwm_pre_set(); //led disable
    _led_pwm_bright_set(0xFF, 0, 0);
}


void pwm_led_close(void)
{
    //TODO: set bright duty 0: avoid set on case
    led_pwm_pre_set(); //led disable
    _led_pwm_bright_set(0xFF, 0, 0);
}

/**
 * @brief: led 常亮显示设置
 * @param led_index: 0: led0, 1:led1, 2:led0 & led1
 * @param led0_bright, LED0亮度: 0 ~ 500
 * @param led1_bright, LED1亮度: 0 ~ 500
 *
 * @return void
 */
void _pwm_led_on_display(u8 led_index, u16 led0_bright, u16 led1_bright)
{
    //step1: pwm0 clock
    if (__this->clock == PWM_LED_CLK_RC32K) {
        _led_pwm0_clk_set(PWM0_CLK_32K);
    } else {
        _led_pwm0_clk_set(PWM0_CLK_46K);
    }
    //step2: pwm1 clock
    _led_pwm1_clk_set(4);

    //bright set
    u8 shift_num = 0;
    u8 out_inv = 0;
    u16 led0_bri_duty = 0;
    u16 led1_bri_duty = 0;
    u16 led_bri_prd = 200;

    led0_bri_duty = led0_bright;
    led1_bri_duty = led1_bright;

    switch (led_index) {
    case 0:
        out_inv = 1;
        break;
    case 1:
        break;
    case 2:
        shift_num = 1;
        led_bri_prd = 160;
        break;
    default:
        PWMLED_HW_LOG_DEBUG("%s led index err", __func__);
        return;
        break;
    }

    led0_bri_duty = (led0_bri_duty * led_bri_prd) / 500;
    led1_bri_duty = (led1_bri_duty * led_bri_prd) / 500; //调试数据

    //step3: bright亮度
    _led_pwm_bright_set(led_bri_prd, led0_bri_duty, led1_bri_duty);

    //step4: 1.输出取反, 2.变色(互闪);
    _led_pwm_output_logic_set(out_inv, shift_num);

    //step5: 周期亮灭配置
    //pwm1 duty0, duty1, duty2
    _led_pwm1_duty_set(40, 2, 0, 0, 0);

    //step6: enable led module
    _led_pwm_module_enable(1, 0);
}

static void __pwm_led_flash_common_handle(u8 led_index, u16 led0_bright, u16 led1_bright, u32 period)
{
//step1: pwm0 clock
    u16 pwm0_prd = 500;
    u16 pwm0_div = 0;
    if (__this->clock == PWM_LED_CLK_RC32K) {
        _led_pwm0_clk_set(PWM0_CLK_32K);
        pwm0_div = (PWM0_RC_CLK32K_VALUE * period) / (1000 * 256);
    } else {
        if (period < 22000) {
            _led_pwm0_clk_set(PWM0_CLK_46K);
            pwm0_div = (PWM0_BT_CLK46K_VALUE * period) / (1000 * 256);
        } else {
            _led_pwm0_clk_set(PWM0_CLK_23K);
            pwm0_div = (PWM0_BT_CLK23K_VALUE * period) / (1000 * 256);
            pwm0_prd = 300;
            led0_bright = (led0_bright * 300) / 500;
            led1_bright = (led1_bright * 300) / 500;
        }
    }
    //step2: pwm1 clock
    _led_pwm1_clk_set(pwm0_div);

    //bright set
    u8 shift_num = 0;
    u8 out_inv = 0;
    u16 led0_bri_duty = 0;
    u16 led1_bri_duty = 0;

    switch (led_index) {
    case 0:
        led0_bri_duty = led0_bright;
        led1_bri_duty = 0;
        out_inv = 1;
        break;

    case 1:
        led0_bri_duty = 0;
        led1_bri_duty = led1_bright;
        break;
    case 2:
        shift_num = 1;
        led0_bri_duty = led0_bright;
        led1_bri_duty = led1_bright;
        break;
    default:
        PWMLED_HW_LOG_ERR("%s led index err", __func__);
        return;
        break;
    }

    //step3: bright亮度
    _led_pwm_bright_set(pwm0_prd, led0_bri_duty, led1_bri_duty);

    //step4: 1.输出取反, 2.变色(互闪);
    _led_pwm_output_logic_set(out_inv, shift_num);
}

/**
 * @brief: led 周期闪一次显示设置
 * @param  led_index: 0: led0, 1:led1, 2:led0 & led1(互闪)
		  	led0_bright: led0亮度(0 ~ 500),
		  	led1_bright: led1亮度(0 ~ 500),
		  	period: 闪灯周期(ms), 多少ms闪一下(100 ~ 20000), 100ms - 20S,
			start_light_time: 在周期中开始亮灯的时间, -1: 周期最后亮灯, 默认填-1即可,
			light_time: 灯亮持续时间,
 *
 * @return void
 */
static void _pwm_led_one_flash_display(u8 led_index, u16 led0_bright, u16 led1_bright,
                                       u32 period, u32 start_light_time, u32 light_time)
{
    __pwm_led_flash_common_handle(led_index, led0_bright, led1_bright, period);
    //step5: 周期亮灭配置
    //pwm1 duty0, duty1, duty2
    u8 pwm1_duty0 = 0;
    u8 pwm1_duty1 = 0;
    if (start_light_time != -1) {
        if (start_light_time >= period) {
            PWMLED_HW_LOG_ERR("start_light_time config err");
            _pwm_led_off_display(); //led off
            return;
        }
        //指定从哪个时间亮,
        pwm1_duty0 = (256 * start_light_time) / period;
        pwm1_duty0 = (pwm1_duty0) ? pwm1_duty0 : 2;
        if ((start_light_time + light_time) > period) {
            pwm1_duty1 = 0; //只开duty0
        } else {
            pwm1_duty1 = (256 * light_time) / period;
            pwm1_duty1 = (pwm1_duty1) ? pwm1_duty1 : 2;
            if ((pwm1_duty0 + pwm1_duty1) > 0xFF) {
                pwm1_duty1 = 0 ;
            } else {
                pwm1_duty1 += pwm1_duty0;
            }
        }
    } else {
        pwm1_duty0 = (256 * light_time) / period;
        pwm1_duty0 = (pwm1_duty0) ? pwm1_duty0 : 2;
        pwm1_duty0 = 256 - pwm1_duty0;
    }

    if ((led_index == 2) && (light_time == -1)) { //互闪, 占满整个周期
        pwm1_duty0 = 2;
        pwm1_duty1 = 0;
    }
    _led_pwm1_duty_set(0xFF, pwm1_duty0, pwm1_duty1, 0, 0);

    //step6: enable led module
    _led_pwm_module_enable(1, 0);
}


/**
 * @brief: led 周期闪一次显示设置
 * @param  led_index: 0: led0, 1:led1, 2:led0 & led1(互闪)
		  	led0_bright: led0亮度,
		  	led1_bright: led1亮度,
		  	period: 闪灯周期(ms), 多少ms闪一下
			first_light_time: 第一次亮灯持续时间,
			second_light_time: 第二次亮灯持续时间,
			gap_time: 两次亮灯时间间隔,
 * @param led0_bright, LED0亮度: 0 ~ 500
 * @param led1_bright, LED1亮度: 0 ~ 500
 *
 * @return void
 */
static void _pwm_led_double_flash_display(u8 led_index, u16 led0_bright, u16 led1_bright,
        u32 period, u32 first_light_time, u32 gap_time, u32 second_light_time)
{
    __pwm_led_flash_common_handle(led_index, led0_bright, led1_bright, period);

    //step5: 周期亮灭配置
    //pwm1 duty0, duty1, duty2
    u8 pwm1_duty0 = 0;
    u8 pwm1_duty1 = 0;
    u8 pwm1_duty2 = 0;

    pwm1_duty2 = (256 * second_light_time) / period;
    pwm1_duty2 = (pwm1_duty2) ? (0xFF - pwm1_duty2) : (0xFF - 2);

    pwm1_duty1 = (256 * gap_time) / period;
    pwm1_duty1 = (pwm1_duty1) ? (pwm1_duty2 - pwm1_duty1) : (pwm1_duty2 - 2);

    pwm1_duty0 = (256 * first_light_time) / period;
    pwm1_duty0 = (pwm1_duty0) ? (pwm1_duty1 - pwm1_duty0) : (pwm1_duty1 - 2);

    _led_pwm1_duty_set(0xFF, pwm1_duty0, pwm1_duty1, pwm1_duty2, 0);

    //step6: enable led module
    _led_pwm_module_enable(1, 0);
}

/**
 * @brief: led 周期呼吸显示,
 * @param  led_index: 0: led0, 1:led1, 2:led0 & led1(交互呼吸)
			breathe_time: 呼吸周期(灭->最亮->灭), 设置范围: 500ms以上;
		   led0_bright: led0呼吸到最亮的亮度(0 ~ 500);
		   led1_bright: led1呼吸到最亮的亮度(0 ~ 500);
		   led0_light_delay_time: led0最高亮度延时(0 ~ 100ms);
		   led1_light_delay_time: led1最高亮度延时(0 ~ 100ms);
		   led_blink_delay_time: led0和led1灭灯延时(0 ~ 20000ms), 0 ~ 20S;
 *
 * @return void
 */
static void _pwm_led_breathe_display(u8 led_index, u16 breathe_time, u16 led0_bright, u16 led1_bright,
                                     u32 led0_light_delay_time, u32 led1_light_delay_time, u32 led_blink_delay_time)
{
    //亮灯函数的亮度限制需要大于0避免发生除0异常，小于0则直接使用关灯函数节省功耗
    if (led0_bright <= 0) {
        led0_bright = 1;
    }

    if (led1_bright <= 0) {
        led1_bright = 1;
    }

    PWMLED_HW_LOG_SUCC("led_index:%d,breathe_time:%d,led0_bright:%d,led1_bright:%d,led0_light_delay_time:%d, led1_light_delay_time:%d, led_blink_delay_time:%d", led_index, breathe_time, led0_bright,  led1_bright, led0_light_delay_time, led1_light_delay_time, led_blink_delay_time);

    u16 led0_bri_duty = led0_bright;
    u16 led1_bri_duty = led1_bright;
    u16 pwm1_div = 0;
    u16 Tpwm1 = 0;
    pwm1_div = led0_bri_duty > led1_bri_duty ? led0_bri_duty : led1_bri_duty;

    breathe_time = breathe_time / 2; //呼吸总时间, 单个灭到最亮的时间
    //step1: pwm0 clock
    if (__this->clock == PWM_LED_CLK_RC32K) {
        _led_pwm0_clk_set(PWM0_CLK_32K);
        /* pwm1_div = breathe_time * 32 / pwm1_div; */
        /* Tpwm1 = (pwm1_div * 1000 / 32000); //ms */
        pwm1_div = breathe_time * (PWM0_RC_CLK32K_VALUE / 1000) / pwm1_div;
        Tpwm1 = (pwm1_div * 1000 / PWM0_RC_CLK32K_VALUE); //ms
    } else {
        _led_pwm0_clk_set(PWM0_CLK_46K);
        pwm1_div = breathe_time * 46 / pwm1_div;
        Tpwm1 = (pwm1_div * 1000 / 46000); //ms
    }
    //step2: pwm1 clock
    _led_pwm1_clk_set(pwm1_div);

    //bright set
    u8 shift_num = 0;
    u8 out_inv = 0;

    switch (led_index) {
    case 0:
        led1_bri_duty = 0;
        out_inv = 1;
        break;

    case 1:
        led0_bri_duty = 0;
        break;
    case 2:
        shift_num = 2;
        break;
    default:
        PWMLED_HW_LOG_ERR("%s led index err", __func__);
        return;
    }

    //step3: bright亮度
    _led_pwm_bright_set(500, led0_bri_duty, led1_bri_duty);

    //step4: 1.输出取反, 2.变色(互闪);
    _led_pwm_output_logic_set(out_inv, shift_num);

    //step5: 周期亮灭配置
    //pwm1 duty0, duty1, duty2
    u8 pwm1_duty0 = 0;
    u8 pwm1_duty1 = 0;
    u8 pwm1_duty2 = 0;
    u8 pwm1_duty3 = 0xFF;

    if (Tpwm1 == 0) {
        led0_light_delay_time *= 2;
        led1_light_delay_time *= 2;
        led_blink_delay_time *= 2;
        Tpwm1 = 1;
    }

    //最高亮度延时
    pwm1_duty0 = led0_light_delay_time / Tpwm1;
    pwm1_duty1 = led1_light_delay_time / Tpwm1;
    //灭灯延时,{duty3, duty2}, 16bit
    pwm1_duty2 = (led_blink_delay_time / Tpwm1) & 0xFF;
    pwm1_duty3 = ((led_blink_delay_time / Tpwm1) >> 8) & 0xFF;

    _led_pwm1_duty_set(pwm1_duty3, pwm1_duty0, pwm1_duty1, pwm1_duty2, 1);

    //step6: enable led module
    _led_pwm_module_enable(1, 1);
}


//=================================================================================//
//                        		以下为 LED API                    				   //
//=================================================================================//

//=================================================================================//
//@brief: LED显示周期复位, 重新开始一个周期, 可用于同步等操作
//@input: void
//@return: void
//@note:
//=================================================================================//
void pwm_led_display_mode_reset(void)
{
    if (IS_PWM_LED_ON) {
        LED_PWM_DISABLE;
        LED_PWM_ENABLE;
    }
}

//=================================================================================//
//@brief: 修改LED灯IO口驱动能力,
//@input: void
//@return: 当前LED显示模式
//@note:
//挡位: 0 ~ 3
//	0: 2.4mA(8mA mos + 120Ωres)
//	1: 8mA(8mA mos)
// 	2: 18.4mA(24mA mos + 120Ωres)
// 	3: 24mA(24mA mos)
//=================================================================================//
void pwm_led_io_max_drive_set(u8 strength)
{
    LED_PWM_IO_MAX_DRIVE(strength);
}

//=================================================================================//
//@brief: 获取LED模块是否开启, 可用于sniff灯同步
//@input: void
//@return: 0: 模块开启; 1: 模块关闭
//@note:
//=================================================================================//
bool is_led_module_on()
{
    return IS_PWM_LED_ON;
}

//=================================================================================//
//@brief: 自定义设置单灯闪状态
//@input: void
//		led_index: 0: led0, 1:led1, 2:led0 & led1(互闪)
// 		led0_bright: led0亮度(0 ~ 500),
// 		led1_bright: led1亮度(0 ~ 500),
// 		period: 闪灯周期(ms), 多少ms闪一下,
// 		start_light_time: 在周期中开始亮灯的时间, -1: 周期最后亮灯, 默认填-1即可,
// 		light_time: 灯亮持续时间(ms),
//@return: void
//@note:
//=================================================================================//
void pwm_led_one_flash_display(u8 led_index, u16 led0_bright, u16 led1_bright,
                               u32 period, u32 start_light_time, u32 light_time)
{
    PWMLED_HW_LOG_DEBUG("led_index:%d,led0_bright:%d,led1_bright:%d,period:%d,start_light_time:%d,light_time:%d\n", led_index,  led0_bright,  led1_bright, period,  start_light_time, light_time);
    _pwm_led_close_irq();
    _pwm_led_one_flash_display(led_index, led0_bright, led1_bright,
                               period, start_light_time, light_time);
}


//=================================================================================//
//@brief: 自定义设置单灯双闪状态
//@input:
// 		led_index: 0: led0, 1:led1, 2:led0 & led1(互闪)
// 		led0_bright: led0亮度,
// 		led1_bright: led1亮度,
// 		period: 闪灯周期(ms), 多少ms闪一下
//  	first_light_time: 第一次亮灯持续时间,
// 		second_light_time: 第二次亮灯持续时间,
// 		gap_time: 两次亮灯时间间隔,
//		led0_bright, LED0亮度: 0 ~ 500
// 		led1_bright, LED1亮度: 0 ~ 500
//@return: void
//@note:
//=================================================================================//
void pwm_led_double_flash_display(u8 led_index, u16 led0_bright, u16 led1_bright,
                                  u32 period, u32 first_light_time, u32 gap_time, u32 second_light_time)
{
    PWMLED_HW_LOG_DEBUG("led_index:%d,led0_bright:%d,led1_bright:%d,period:%d,first_light_time:%d,gap_time:%d,second_light_time:%d\n", led_index,  led0_bright,  led1_bright, period,  first_light_time, gap_time, second_light_time);

    _pwm_led_close_irq();
    _pwm_led_double_flash_display(led_index, led0_bright, led1_bright,
                                  period, first_light_time, gap_time, second_light_time);
}


//=================================================================================//
//@brief: 自定义设置呼吸模式
//@input:
//		led_index: 0: led0, 1:led1, 2:led0 & led1(互闪)
// 		led0_bright: led0亮度,
// 		led1_bright: led1亮度,
// 		period: 闪灯周期(ms), 多少ms闪一下,
// 		start_light_time: 在周期中开始亮灯的时间, -1: 周期最后亮灯
// 		light_time: 灯亮持续时间,
//		led0_bright, LED0亮度: 0 ~ 500
// 		led1_bright, LED1亮度: 0 ~ 500
//@return: void
//@note:
//=================================================================================//
void pwm_led_breathe_display(u8 led_index, u16 breathe_time, u16 led0_bright, u16 led1_bright,
                             u32 led0_light_delay_time, u32 led1_light_delay_time, u32 led_blink_delay_time)
{
    PWMLED_HW_LOG_DEBUG("led_index:%d,breathe_time:%d,led0_bright:%d,led1_bright:%d,led0_light_delay_time:%d, led1_light_delay_time:%d, led_blink_delay_time:%d\n", led_index, breathe_time, led0_bright,  led1_bright, led0_light_delay_time, led1_light_delay_time, led_blink_delay_time);

    _pwm_led_close_irq();
    _pwm_led_breathe_display(led_index, breathe_time, led0_bright, led1_bright,
                             led0_light_delay_time, led1_light_delay_time, led_blink_delay_time);
}


//=================================================================================//
//@brief: 注册LED周期中断函数, 每个LED周期结束后会调用一次, 可以统计指定状态闪烁多少次
//@input:
//@return: void
//@note:
//=================================================================================//
void pwm_led_register_irq(void (*func)(void))
{
    line_inf;
    _pwm_led_register_irq(func, 1);
}

#if 0
/**
 * @param JL_TIMERx : JL_TIMER0/1/2/3/4/5
 * @param pwm_io : JL_PORTA_01, JL_PORTB_02,,,等等，br30支持任意普通IO
 * @param fre : 频率，单位Hz
 * @param duty : 初始占空比，0~10000对应0~100%
 */
void timer_pwm_init(JL_TIMER_TypeDef *JL_TIMERx, u32 pwm_io, u32 fre, u32 duty)
{
    u32 io_clk = 0;
    gpio_set_die(pwm_io, 1);
    gpio_set_pull_up(pwm_io, 0);
    gpio_set_pull_down(pwm_io, 0);
    gpio_set_direction(pwm_io, 0);
    switch ((u32)JL_TIMERx) {
    case (u32)JL_TIMER0 :
        JL_IOMAP->CON0 |= BIT(0);
        io_clk = 24000000;
        gpio_set_fun_output_port(pwm_io, FO_TMR0_PWM, 0, 1);
        break;
    case (u32)JL_TIMER1 :
        JL_IOMAP->CON0 |= BIT(1);
        io_clk = 12000000;
        gpio_set_fun_output_port(pwm_io, FO_TMR1_PWM, 0, 1);
        break;
    case (u32)JL_TIMER2 :
        JL_IOMAP->CON0 |= BIT(2);
        io_clk = 24000000;
        gpio_set_fun_output_port(pwm_io, FO_TMR2_PWM, 0, 1);
        break;
    case (u32)JL_TIMER3 :
        bit_clr_ie(IRQ_TIME3_IDX);
        JL_IOMAP->CON0 |= BIT(3);
        io_clk = 12000000;
        gpio_set_fun_output_port(pwm_io, FO_TMR3_PWM, 0, 1);
        break;
    case (u32)JL_TIMER4 :
        JL_IOMAP->CON0 |= BIT(4);
        io_clk = 24000000;
        gpio_set_fun_output_port(pwm_io, FO_TMR4_PWM, 0, 1);
        break;
    case (u32)JL_TIMER5 :
        JL_IOMAP->CON0 |= BIT(5);
        io_clk = 12000000;
        gpio_set_fun_output_port(pwm_io, FO_TMR5_PWM, 0, 1);
        break;
    default:
        return;
    }
    //初始化timer
    JL_TIMERx->CON = 0;
    JL_TIMERx->CON |= (0b01 << 2);						//时钟源选择IO_CLK
    JL_TIMERx->CON |= (0b0001 << 4);					//时钟源再4分频
    JL_TIMERx->CNT = 0;									//清计数值
    JL_TIMERx->PRD = io_clk / (4 * fre);				//设置周期
    JL_TIMERx->CON |= (0b01 << 0);						//计数模式
    JL_TIMERx->CON |= BIT(8);							//PWM使能
    //设置初始占空比
    JL_TIMERx->PWM = (JL_TIMERx->PRD * duty) / 10000;	//0~10000对应0~100%
}

void set_timer_pwm_duty(JL_TIMER_TypeDef *JL_TIMERx, u32 duty)
{
    JL_TIMERx->PWM = (JL_TIMERx->PRD * duty) / 10000;	//0~10000对应0~100%
}

#endif  /* #if 0 */

//=============================================================//
//             			单独亮灭控制                           //
//=============================================================//


static void pwm_led_hw_bright_set(u8 logic, u8 bright)
{
    u16 led_bri_prd = 200;
    u16 led_bri_duty = (bright * led_bri_prd) / 100;

    //step3: bright亮度
    _led_pwm_bright_set(led_bri_prd, led_bri_duty, led_bri_duty);
}

static void pwm_led_hw_out_logic_set(u8 logic)
{
    u8 out_inv = 0;
    u8 shift_num = 0;

    if (logic == 0) {
        out_inv = 1;
    }
    if (logic == 2) {
        shift_num = 1;
    }

    //step4: 1.输出取反, 2.变色(互闪);
    _led_pwm_output_logic_set(out_inv, shift_num);
}


static void pwm_led_hw_brearhe_delay(void)
{
    LED_PWM1_DUTY2_SET(0xFF);
    LED_PWM1_DUTY3_SET(0xFF);
    LED_PWM1_PRD_SEL_DIS; //呼吸模式, duty0/1是亮灯延时, {duty3,duty2}是灭灯延时
    LED_PWM1_DUTY3_EN;
    LED_PWM1_DUTY2_EN;
}

static void pwm_led_hw_duty_set(void)
{
    //step5: 周期亮灭配置
    _led_pwm1_duty_set(40, 1, 0, 0, 0);
}

static void pwm_led_hw_on_off_ctrl(u8 on_off)
{
    _led_pwm_module_enable(on_off, 0);
}

static void pwm_led_hw_init(void)
{
    LED_PWM_DISABLE;
    LED_PWM_BREATHE_DISABLE;  //呼吸灯使能位

    RESET_PWM_CON0;
    RESET_PWM_CON1;
    RESET_PWM_CON2;
    RESET_PWM_CON3;

    LED_PWM_OUT_LOGIC_SET;
    LED_PWM_CLR_PENDING;

    //pwm_clock_set(PWM_LED_CLK_RC32K);

    //step1: pwm0 clock
    //_led_pwm0_clk_set(PWM0_CLK_32K);

    //step2: pwm1 clock
    _led_pwm1_clk_set(4);

    pwm_led_hw_duty_set();
    PWMLED_HW_LOG_DEBUG("pwm_led_hw_init_version_v1.5");
}


static void pwm_led_hw_breath_ctrl(struct pwm_led_hw_ctrl *ctrl)
{
    PWMLED_HW_LOG_SUCC("state: %d, io: 0x%x, logic: %d, bright: %d, breath_rate: %d, breath_time: %d", ctrl->state, ctrl->gpio, ctrl->logic, ctrl->bright, ctrl->breath_rate, ctrl->breath_time);

    int index = 0;
    if (led_hw.logic_tbl[0].io != ctrl->gpio ||
        led_hw.logic_tbl[0].logic != ctrl->logic) {
        index = 1;
    }
    if (led_hw.logic_tbl[index].state == 2) {
        PWMLED_HW_LOG_DEBUG("led_hw.logic_tbl[ctrl->logic].state: %d", led_hw.logic_tbl[ctrl->logic].state);

        pwm_led_hw_brearhe_delay(); //灭灯延时
        return;
    }
    u16 led_bright = ctrl->bright * 500 / 100;
    u16 _breath_time = ctrl->breath_time * 2;
    u16 breathe_time = (u32)(_breath_time *  ctrl->breath_rate) / 20;
    u16 led_light_delay_time = _breath_time - breathe_time; //亮灯延时

    PWMLED_HW_LOG_DEBUG("breath display: breath_time: %d, delay time: %d, bright: %d", breathe_time, led_light_delay_time, led_bright);

    local_irq_disable();
    _pwm_led_breathe_display(ctrl->logic, breathe_time, led_bright, led_bright,
                             led_light_delay_time, led_light_delay_time, 0);
    led_hw.logic_tbl[index].state = 2;
    led_hw.logic_tbl[!index].state = 0;

    local_irq_enable();
}

void pwm_led_hw_display_ctrl(struct pwm_led_hw_ctrl *ctrl)
{
    u8 logic;
    u8 index = ctrl->led_id;

    PWMLED_HW_LOG_DEBUG("ctrl->state: %d, ctrl->gpio: 0x%x, ctrl->logic: %d, ctrl->bright: %d, ctrl->breath_rate: %d, ctrl->breath_time: %d",
                        ctrl->state, ctrl->gpio, ctrl->logic, ctrl->bright, ctrl->breath_rate, ctrl->breath_time);

    if (led_hw.init == 0) {
        led_pin_set_enable(ctrl->gpio);  //一个IO推两个灯
        led_hw.logic_tbl[0].io = ctrl->gpio;
        led_hw.logic_tbl[0].logic = ctrl->logic;
        //_pwm_led_on_display(0, 0, 0);
        pwm_clock_set(PWM_LED_CLK_RC32K);
        led_hw.init = 1;
    } else {
        if (led_hw.recover_flag == 1) {
            led_hw.recover_flag = 0;
            _pwm_led_close_all_recover();
        }
        //初始化完成
        if (led_hw.logic_tbl[0].io == ctrl->gpio &&
            led_hw.logic_tbl[0].logic == ctrl->logic) {
            //单io推
            if (led_hw.logic_tbl[1].io != ctrl->gpio) {

                if (led_hw.logic_tbl[1].io) {
                    //关闭其他io
                    led_pin_set_disable(led_hw.logic_tbl[1].io);
                }
            }
        } else {
            //多io推
            //index = 1: 内部切IO中断;
            index = 1;
            led_hw.logic_tbl[1].io = ctrl->gpio;
            led_hw.logic_tbl[1].logic = ctrl->logic;

            if (led_hw.logic_tbl[0].io != ctrl->gpio) {
                //关闭其他io
                led_pin_set_disable(led_hw.logic_tbl[0].io);
            }
        }
        //把需要控制的io接入模块
        led_pin_set_enable(ctrl->gpio);
    }

    _led_pwm0_clk_set(PWM0_CLK_32K);
    if (ctrl->breath_rate) {
        pwm_led_hw_breath_ctrl(ctrl);
        //会更新state
        return;
    }

    if (ctrl->state == 0) {
        local_irq_disable();
        led_hw.logic_tbl[index].state = 0;
        if ((led_hw.logic_tbl[0].state == 0) &&
            (led_hw.logic_tbl[1].state == 0)) {
            if (led_hw_keep_data.keep_data_flag == 0) {
                led_hw.recover_flag = 1;
                _pwm_led_close_all();

            }
        } else {
            //只需要关一个io,那就需要特殊处理一下
            //关当前logic:
            pwm_led_hw_on_off_ctrl(0);
            pwm_led_hw_out_logic_set(!ctrl->logic);
            pwm_led_hw_on_off_ctrl(1);
        }
        local_irq_enable();
    } else {
        //状态, 亮度, 点亮逻辑
        if (led_hw.logic_tbl[!index].state == 1) {
            logic = 2; //同时亮
            led_pin_set_enable(led_hw.logic_tbl[!index].io);
        } else {
            logic = ctrl->logic;
        }
        local_irq_disable();
        //pwm_led_hw_on_off_ctrl(0);
        pwm_led_hw_init();
        pwm_led_hw_bright_set(ctrl->logic, ctrl->bright);
        pwm_led_hw_out_logic_set(logic);
        pwm_led_hw_on_off_ctrl(1);
        led_hw.logic_tbl[index].state = 1;
        led_hw.logic_tbl[index].bright = ctrl->bright;
        local_irq_enable();
    }
}

void pwm_led_hw_display_ctrl_clear(void)
{
    pwm_led_hw_on_off_ctrl(0);
    led_hw.logic_tbl[0].state = 0;
    led_hw.logic_tbl[0].bright = 0;
    led_hw.logic_tbl[1].state = 0;
    led_hw.logic_tbl[1].bright = 0;
}

void pwm_led_hw_io_enable(u8 gpio)
{
    PWMLED_HW_LOG_DEBUG("led_hw.init = %d ,led_hw.recover_flag = %d", led_hw.init, led_hw.recover_flag);
    if (led_hw.init == 0) {
        led_pin_set_enable(gpio);  //一个IO推两个灯
        led_hw.logic_tbl[0].io = gpio;
        led_hw.logic_tbl[1].io = gpio;
        pwm_clock_set(PWM_LED_CLK_RC32K);
        led_hw.init = 1;
    }

    if (led_hw.recover_flag == 1) {
        _pwm_led_close_all_recover();
        led_hw.recover_flag = 0;
    }

    pwm_led_hw_init();
}

void pwm_led_hw_io_set(u8 led_index, u8 gpio, u8 logic)
{
    if (led_hw.init == 0) {
        led_hw.logic_tbl[0].io = gpio;
        led_hw.logic_tbl[0].logic = logic;
        led_hw.logic_tbl[1].io = gpio;
        led_hw.logic_tbl[1].logic = logic;
        pwm_clock_set(PWM_LED_CLK_RC32K);
        led_hw.init = 1;
    } else {
        led_hw.logic_tbl[led_index].io = gpio;
        led_hw.logic_tbl[led_index].logic = logic;
    }
}

void pwm_led_hw_led_enable(u8 led_index)
{
    led_pin_set_enable(led_hw.logic_tbl[led_index].io);

    if (led_hw.recover_flag == 1) {
        _pwm_led_close_all_recover();
        led_hw.recover_flag = 0;
    }

    pwm_led_hw_init();
}

//===============================================================//
//                       LED UI Slot Timer                       //
//===============================================================//
/////////////////////16SLOT TIME////////////////////////
#ifdef CONFIG_NEW_BREDR_ENABLE
extern u8 slot_timer_get();
extern void slot_timer_set(u8 timer, void (*handler)(void *), void *priv, int slot);
extern void slot_timer_reset(u8 timer, int slot);
extern void slot_timer_put(u8 timer);
#define __timer_16slot_get 				slot_timer_get
#define __timer_16slot_put 				slot_timer_put
#define __timer_16slot_set 				slot_timer_set
#define __timer_16slot_reset(timer, slot_num) \
	do { \
		if (timer != 0xFF) { \
			slot_timer_reset(timer, slot_num); \
		} \
	} while(0) \

#define __pwm_led_get_16slot_num(x) 	(x * 1000 / 625)

#else /* #ifdef CONFIG_NEW_BREDR_ENABLE */

extern u8 timer_16slot_get();
extern void timer_16slot_put(u8 i);
extern void timer_16slot_set(u8 i, void *timer_handle, void *priv, u32 slot_num);
extern void timer_16slot_reset(u8 i, u32 slot_num);
#define __timer_16slot_get 				timer_16slot_get
#define __timer_16slot_put 				timer_16slot_put
#define __timer_16slot_set 				timer_16slot_set
#define __timer_16slot_reset 			timer_16slot_reset

#define __pwm_led_get_16slot_num(x) 	(x * 1000 / (625 * 16))

#endif /* #ifdef CONFIG_NEW_BREDR_ENABLE */

static u8 led_ui_slot_timer = 0;
static u16 led_ui_hi_timer = 0;
static spinlock_t slot_timer_lock = {0};

void led_ui_hw_timer_add(u32 period, void *handler, void *priv)
{

    /* int slot_num = __pwm_led_get_16slot_num(period); */

    spin_lock(&slot_timer_lock);

    led_ui_hi_timer = usr_timeout_add(NULL, handler, period, 1);
#if 0
    if (led_ui_slot_timer != 0) {
        __timer_16slot_reset(led_ui_slot_timer, slot_num);
    } else {
        led_ui_slot_timer = __timer_16slot_get();
        if (led_ui_slot_timer == 0xFF) {
            //Slot timer Not Support
            led_ui_slot_timer = 0;
            PWMLED_HW_LOG_DEBUG("led use user timer");
            led_ui_hi_timer = usr_timeout_add(NULL, handler, period, 1);
        } else {
            __timer_16slot_set(led_ui_slot_timer, (void *)handler, (void *)priv, slot_num);
        }
    }
#endif //#if 0
    //
    spin_unlock(&slot_timer_lock);
}

void led_ui_hw_timer_delete(void)
{
    spin_lock(&slot_timer_lock);

    if (led_ui_slot_timer != 0) {
        __timer_16slot_put(led_ui_slot_timer);
        led_ui_slot_timer = 0;
    }

    if (led_ui_hi_timer != 0) {
        usr_timeout_del(led_ui_hi_timer);
        led_ui_hi_timer = 0;
    }

    spin_unlock(&slot_timer_lock);
}

