#ifndef _ICSD_COMMON_H
#define _ICSD_COMMON_H

#include "generic/typedef.h"
#include "math.h"

//#define abs(x)  ((x)>0?(x):-(x) )
//#define fabs(x) ((x) < 0 ? -(x) : (x))

#define SWARM_NUM 		60
#define ICSD_TARLEN    	120//120
#define ICSD_TARLEN_L	40//8+32
#define ICSD_DRPPNT   	10// 0,46,92,138,184
#define ICSD_FLEN 		((ICSD_TARLEN+ICSD_TARLEN_L-ICSD_DRPPNT)/2)  //96

typedef struct icsd_nmss_config_inst {
    int flex_seq[6];
    int n;
    float rho;
    float chi;
    float psi;
    float sigma;
    int maxnum;
    float usual_delta;
    float zero_term_delta;
} icsd_nmss_conf;

typedef struct icsd_nmss_cost_inst {
    float	J;
    float	param[6];
} icsd_nmss_cost;

typedef struct icsd_filter_param_inst {
    float b[3];
    float a[3];
} icsd_biquad_filter;



extern float cos_float(float x);
extern float sin_float(float x);
extern float log10_float(float x);
extern float exp_float(float x);
extern float root_float(float n);
extern float angle_float(float x, float y);
extern unsigned int hw_fft_config(int N, int log2N, int is_same_addr, int is_ifft, int is_real);
extern void hw_fft_run(unsigned int fft_config, const int *in, int *out);

extern float icsd_pow10(float n);
extern float icsd_cos_hq(float x);
extern float icsd_sin_hq(float x);
extern void  icsd_complex_div(float *input1, float *input2, float *out, int len);


extern void  icsd_FFT_radix1024(int *in, int *out);
extern void  icsd_HanningWin_pwr(s16 *input, int *output, int len);

extern void  icsd_common_version();
//



extern float abs_float(float f);
extern void HanningWin_pwr(s16 *input, int *output, int len);
extern void FFT_radix1024(int *in_cur, int *out);
extern void FFT_radix256(int *in_cur, int *out);
extern void FFT_lowfreq4096(s16 *in_cur, float *out, int len);
//---------------------------
// multi float  conj
//---------------------------
extern void complex_mulf(float *input1, float *input2, float *out, int len);
//---------------------------
// multi conj
//---------------------------
extern void complex_mul(int *input1, float *input2, float *out, int len);
extern void unwrap(float *pha1, float *pha2);

//---------------------------
// div
//---------------------------
extern float mean_cal(float *data0, u8 len);
extern float min_cal(float *data, u8 len);
extern double devide_float(float data_0, float data_1);
extern double devide_double(double data_0, double data_1);

extern void complex_muln_2(float *input1, float *input2, float *input3, float *out, int len, float alpha);
extern void complex_div(float *input1, float *input2, float *out, int len);
extern void biquad2ab(float gain, float f, float q, float *a0, float *a1, float *a2, float *b0, float *b1, float *b2, int type);
extern void biquad2ab_double(float gain, float f, float q, double *a0, double *a1, double *a2, double *b0, double *b1, double *b2, int type);
extern void icsd_biquad2ab_out(float gain, float f, float fs, float q, double *a0, double *a1, double *a2, double *b0, double *b1, double *b2, int type);
extern void biquad2ab_double_pn(float gain, float f, float q, double *a0, double *a1, double *a2,
                                double *b0, double *b1, double *b2, int type);

extern void icsd_data_dsfn(s16 *input, s16 *output, int len, u8 n);

//hard upspeed
extern float anc_pow10(float n);
extern double log10_anc(double x);
extern void icsd_anc_fft(int *in, int *out);
extern void icsd_anc_fft256(int *in, int *out);

extern float cos_hq(float x);
extern float sin_hq(float x);
extern void complex_muln(float *input1, float *input2, float *out, int len);
extern void icsd_complex_mul(int *input1, float *input2, float *out, int len);

#endif
