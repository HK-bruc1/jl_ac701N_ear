#ifndef ICSD_AEQ_CONFIG_H
#define ICSD_AEQ_CONFIG_H

#include "generic/typedef.h"

extern u8 sz_ref_test[];
extern float target_headset[];


extern const int AEQ_objFunc_type ;
extern const float AEQ_GAIN_LIMIT_ALL ;

/***************************************************************/
// aeq
/***************************************************************/
extern const int aeq_type[];
extern const float AEQ_Vrange_M[];
extern const float AEQ_Biquad_init_M[];

extern const float aeq_Weight_M[];
extern const float AEQ_Gold_csv_M[];
extern const float aeq_degree_set_0[];
extern const int   AEQ_IIR_NUM_FLEX;
extern const int   AEQ_IIR_NUM_FIX;
extern const int   AEQ_IIR_COEF;
extern const float AEQ_FSTOP_IDX;
extern const float AEQ_FSTOP_IDX2;
extern const int   AEQ_ITER_MAX;

#endif
