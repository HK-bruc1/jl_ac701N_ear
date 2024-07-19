#ifndef __TIMER_H__
#define __TIMER_H__

#include "typedef.h"

#define     TIMER_PSET_DIV_1            (0b0000<<4)
#define     TIMER_PSET_DIV_2            (0b0100<<4)
#define     TIMER_PSET_DIV_4            (0b0001<<4)
#define     TIMER_PSET_DIV_8            (0b0101<<4)
#define     TIMER_PSET_DIV_16           (0b0010<<4)
#define     TIMER_PSET_DIV_32           (0b0110<<4)
#define     TIMER_PSET_DIV_64           (0b0011<<4)
#define     TIMER_PSET_DIV_128          (0b0111<<4)
#define     TIMER_PSET_DIV_256          (0b1000<<4)
#define     TIMER_PSET_DIV_512          (0b1100<<4)
#define     TIMER_PSET_DIV_1024         (0b1001<<4)
#define     TIMER_PSET_DIV_2048         (0b1101<<4)
#define     TIMER_PSET_DIV_4096         (0b1010<<4)
#define     TIMER_PSET_DIV_8192         (0b1110<<4)
#define     TIMER_PSET_DIV_16384        (0b1011<<4)
#define     TIMER_PSET_DIV_32768        (0b1111<<4)

#define     TIMER_SSEL_LSB              (0<<10)
#define     TIMER_SSEL_OSC              (1<<10)
#define     TIMER_SSEL_HTC              (2<<10)
#define     TIMER_SSEL_LRC              (3<<10)
#define     TIMER_SSEL_RC16M            (4<<10)
#define     TIMER_SSEL_STD_12M          (5<<10)
#define     TIMER_SSEL_STD_24M          (6<<10)
#define     TIMER_SSEL_STD_48M          (7<<10)
#define     TIMER_SSEL_STD_IO_MUX_IN    (15<<10)

#define     TIMER_CLR_PND                BIT(14)
#define     TIMER_PND                    BIT(15)

#endif
