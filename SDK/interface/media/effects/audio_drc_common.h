#ifndef _AUDIO_DRC_COMMON_H_
#define _AUDIO_DRC_COMMON_H_

struct threshold_group {
    float in_threshold;
    float out_threshold;
};
struct mdrc_common_param {
    int is_bypass;
    int way_num;                                    //段数
    int order;                                      //阶数
    int low_freq;                                   //低分频点
    int high_freq;                                  //高分频点
};
#endif

