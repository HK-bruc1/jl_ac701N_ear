#include "app_config.h"
#include "timer.h"
#include "gSensor/gSensor_manage.h"
#include "hr_sensor/hrSensor_manage.h"

#if TCFG_GSENSOR_ENABLE || TCFG_HRSENSOR_ENABLE

extern int gsensor_data_get(short *buf);
#define SENSOR_KICK  (Q_MSG + 1)
u16 cb_timer_id = 0;

static void refresh_sensor_data_check_cb(void *parm)
{
    os_taskq_post_type("app_sensor", SENSOR_KICK, 0, NULL);
}

void sensor_del_data_check_cb()
{
    if (cb_timer_id != 0) {
        sys_s_hi_timer_del(cb_timer_id);
        cb_timer_id = 0;
    }
}

void app_sensor_task(void *p)
{
    short accel_data[1 + 64 * sizeof(axis_info_t)]; //lenght + 64 frame acceleromter data
    memset(accel_data, 0, sizeof(accel_data));

    cb_timer_id = sys_s_hi_timer_add(NULL, refresh_sensor_data_check_cb, 320); //320ms 这个timer是读gsensor数据，并输入给心率，hx3918是320ms读取ppg数据


    os_time_dly(100);
#if TCFG_HRSENSOR_ENABLE
    hr_sensor_measure_hr_start(0, 0);
#endif
    /* hr_sensor_measure_spo2_start(0, 0); */

    int msg[8];
    int ret;
    while (1) {
        ret = os_taskq_pend("taskq", msg, ARRAY_SIZE(msg));
        if (ret != OS_TASKQ) {
            continue;
        }
        switch (msg[0]) {
        case SENSOR_KICK:
#if TCFG_GSENSOR_ENABLE
            gsensor_data_get((short *)accel_data);                            //读加速度数据
#endif
#if TCFG_HRSENSOR_ENABLE
            hr_sensor_io_ctl(INPUT_ACCELEROMETER_DATA, accel_data);  //加速度数据输入给心率
#endif
            //如果有其它算法需要使用加速度数据可在此添加
            break;
        default:
            break;
        }

    }
}


void app_sensor_init(void)
{
    task_create(app_sensor_task, NULL, "app_sensor");
}


#endif
