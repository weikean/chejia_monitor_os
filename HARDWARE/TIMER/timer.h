#ifndef _TIMER_H
#define _TIMER_H
#include "sys.h"

#define SendingDelay	104

extern u8 Times,First_Int,shijian;
extern vu8 Timer0_start;
extern vu8 Time_fac;

void TIM1_Configuration(void);
void TIM2_Configuration(void);
void TIM3_Configuration(void);
void Timer4_Init_Config(void);
void TIM5_Configuration(void);
void Delay_Ms(__IO uint32_t nTime);
void TIM6_Int_Init(u16 arr,u16 psc);
void NVIC_Configuration(void);
#endif
