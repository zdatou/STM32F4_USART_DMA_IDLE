#ifndef _TIMER_H
#define _TIMER_H
#include "sys.h"

void TIM_Init(u8 num, u16 arr, u16 psc);
void TIM_Base_Start(u8 num);
void TIM_Base_Stop(u8 num);
void Start_Tick(void);
u32 Stop_Tick(void);
#endif

