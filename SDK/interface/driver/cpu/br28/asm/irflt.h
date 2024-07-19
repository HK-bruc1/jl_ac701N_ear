#ifndef __KEY_DRV_IR_H__
#define __KEY_DRV_IR_H__

typedef struct _IR_CODE {
    u16 wData;          //<键值
    u16 wUserCode;      //<用户码
    u8  bState;         //<接收状态
    u8  boverflow;      //<红外信号超时
} IR_CODE;


struct irflt_platform_data {
    u8 irflt_io;
    u8 timer;
};

#define IRFLT_PLATFORM_DATA_BEGIN(data) \
		static const struct irflt_platform_data data = {

#define IRFLT_PLATFORM_DATA_END() \
};


extern const struct device_operations irflt_dev_ops;

void irflt_config(u8 port);
u8 get_irflt_value(void);
void ir_timeout_set(void);

#endif

