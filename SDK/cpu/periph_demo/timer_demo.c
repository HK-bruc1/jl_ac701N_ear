#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".timer_demo.data.bss")
#pragma data_seg(".timer_demo.data")
#pragma const_seg(".timer_demo.text.const")
#pragma code_seg(".timer_demo.text")
#endif
#include "gptimer.h"
#include "os/os_api.h"
void timer_pwm_demo()
{
    const struct gptimer_pwm_config pwm_config = {
        .port = PORTA, //指定输出IO，PA组
        .pin = 0, //指定输出IO，BIT(0)脚  输出口PA0,
        .freq = 1000,//输出频率1Khz
        .pwm_duty_X10000 = 5123, //占空比51.23%
        .tid = -1,  //填-1,内部自动分配timer
    };

    int pwm_tid = gptimer_pwm_init(&pwm_config); //初始化pwm配置
    gpio_set_mode(PORTA, BIT(0), PORT_OUTPUT_LOW); //IO设为输出
    gptimer_start(pwm_tid); //启动pwm
    gptimer_set_pwm_freq(pwm_tid, 2000); //设置频率2000
    gptimer_set_pwm_duty(pwm_tid, 8000); //设置占空比 75%

    //绑定更多io到同一个time_pwm ,多个io输出相同频率，相同占空比，完全镜像的pwm
    /* gpio_set_mode(PORTA, BIT(2), PORT_OUTPUT_LOW); //IO设为输出 */
    /* gpio_set_function(PORTA, PORT_PIN_2, PORT_FUNC_TIMER0_PWM + pwm_tid); */
    // 释放手动绑定的io，避免漏电
    /* gpio_set_mode(PORTA, BIT(2), PORT_INPUT_PULLUP_10K); //IO设为输入 */
    /* gpio_disable_function(PORTA, PORT_PIN_2, PORT_FUNC_TIMER0_PWM + pwm_tid); */

    /* gptimer_pause(pwm_tid); //pwm暂停 */
    /* gptimer_resume(pwm_tid); //pwm恢复 */
    /* gptimer_deinit(pwm_tid); //pwm取消初始化 */
}
