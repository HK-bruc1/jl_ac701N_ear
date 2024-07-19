#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".irflt.data.bss")
#pragma data_seg(".irflt.data")
#pragma const_seg(".irflt.text.const")
#pragma code_seg(".irflt.text")
#endif
#include "cpu/includes.h"
#include "asm/irflt.h"
#include "timer.h"
#include "gpio.h"
#include "gptimer.h"

#define ir_log 		printf
/* #define ir_log(...)	 */

IR_CODE  ir_code;       ///<红外遥控信息

static u8 cmp_start = 0;
static int ir_timer_id;

static u8 ir_io_level = 0;
static u8 ir_io = 0;

/*----------------------------------------------------------------------------*/
/**@brief   time1红外中断服务函数
   @param   void
   @param   void
   @return  void
   @note    void timer1_ir_isr(void)
*/
/*----------------------------------------------------------------------------*/
static void ir_isr_cb(int tid)
{

    u32 time_cnt;
    u32 time_us;
    u8 cap;

    time_cnt = gptimer_get_capture_count(ir_timer_id);
    gptimer_set_count(ir_timer_id, 0); //CNT清0
    time_us = gptimer_tick2us(ir_timer_id, time_cnt);
    cap = time_us / 1000; //转成ms

    ir_code.boverflow = 0;

    if (cmp_start < 3) {
        return;
    }

    /* putchar('0' + (cap/10)); */
    /* putchar('0' + (cap%10)); */

    if (cap <= 1) {
        ir_code.wData >>= 1;
        ir_code.bState++;
    } else if (cap == 2) {
        ir_code.wData >>= 1;
        ir_code.wData |= 0x8000;
        ir_code.bState++;
    }

    if (ir_code.bState == 16) {
        ir_code.wUserCode = ir_code.wData;
    }
    if (ir_code.bState == 33) {
        ir_code.bState = 1;
    }
}

/*----------------------------------------------------------------------------*/
/**@brief   获取ir按键值
   @param   void
   @param   void
   @return  void
   @note    void get_irkey_value(void)
*/
/*----------------------------------------------------------------------------*/
u8 get_irflt_value(void)
{
    u8 tkey = 0xff;
    if (ir_code.bState != 32) {
        return tkey;
    }
    if ((((u8 *)&ir_code.wData)[0] ^ ((u8 *)&ir_code.wData)[1]) == 0xff) {
        tkey = (u8)ir_code.wData;
    } else {
        ir_code.bState = 0;
    }
    return tkey;
}


static void ir_timeout(void *priv)
{
    ir_code.boverflow++;
    if (ir_code.boverflow > 56) { //56*2ms ~= 112ms
        ir_code.boverflow = 56;
        ir_code.bState = 0;
    }
    cmp_start ++;
    if (cmp_start > 3) {
        cmp_start = 3;
    }
}

void ir_timeout_set(void)
{
    sys_s_hi_timer_add(NULL, ir_timeout, 2); //2ms
}

static u8 ir_io_sus = 0;
u8 ir_io_suspend(void)
{
    if (ir_io_sus) {
        return 1;
    }
    if (ir_code.boverflow < 7) { //14ms内，红外接收有可能在忙碌
        return 1;
    }
    ir_io_level = gpio_read(ir_io);

    gptimer_pause(ir_timer_id);

    ir_io_sus = 1;
    return 0;
}

u8 ir_io_resume(void)
{
    if (!ir_io_sus) {
        return 0;
    }
    ir_io_sus = 0;
    gpio_set_mode(IO_PORT_SPILT(ir_io), PORT_INPUT_PULLUP_10K);
    delay_nops(10);
    if ((ir_io_level) && (ir_io_level != (gpio_read(ir_io)))) {
        ir_code.boverflow = 0;
    }
    cmp_start = 0;

    gptimer_resume(ir_timer_id);

    return 0;
}

void irflt_config(u8 port)
{
    puts("irflt_config init\n");
    struct gptimer_capture_config ir_capture_config;
    ir_capture_config.port = port / 16;
    ir_capture_config.pin = port % 16;
    ir_capture_config.edge_type = GPTIMER_CAPTRUE_EDGE_FAILL;
    ir_capture_config.irq_cb = ir_isr_cb;
    ir_capture_config.filter = 93750; //48m/512
    ir_capture_config.tid = -1; //自动分配timer
    ir_capture_config.irq_priority = 5; //中断优先级默认1

    ir_timer_id = gptimer_capture_init(&ir_capture_config);
    printf("ir timer id:%d\n", ir_timer_id);
    ir_io = port;
    cmp_start = 0;

    gpio_set_mode(IO_PORT_SPILT(port), PORT_INPUT_PULLUP_10K);//开上拉

    gptimer_start(ir_timer_id);
}


void irflt_int_info()
{
    JL_TIMER_TypeDef *ir_timer;
    ir_timer = TIMER_BASE_ADDR + ir_timer_id * TIMER_OFFSET;

    ir_log("RFLT_CON = 0x%x", JL_IR->RFLT_CON);
    ir_log("IR_TIME_REG = 0x%x", ir_timer->CON);
}
