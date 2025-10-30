#ifndef _AUDIO_GENERAL_CONFIG_H_
#define _AUDIO_GENERAL_CONFIG_H_

#include "generic/typedef.h"

/*
 * TWS耳机双单元驱动
 * 1、使用场景：可使用DAC的左右声道分开推耳机的不同驱动单元
 * 2、配置环境：
    （1）使能TWS
    （2）DAC配置为立体声
    （3）可视化界面使用"TWS耳机"框图
    （4）可通过"声道拆分"分别对两通道数据进行处理，以实现不同驱动单元的需求
 * 3、输出现象：
    （1）对耳使用：
        一台输出"L L"数据(DAC通道0/通道1，均输出左声道数据)
        另一台输出"R R"数据(DAC通道0/通道1，均输出右声道数据)
    （2）单耳使用：
        输出"LRMIX LRMIX"数据(DAC通道0/通道1，均输出左右声道混合数据)
 */
#define AUDIO_TWS_DUAL_DRIVER_ENABLE               0



#endif
