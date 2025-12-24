#ifndef _BSP_TIMER_H
#define _BSP_TIMER_H

#include "sys.h"

void set_reboot_time_function(uint32_t time);
void TIM3_Int_Init(u16 arr,u16 psc);
void TIM6_Int_Init(u16 arr,u16 psc);
void TIM4_Int_Init(u16 arr,u16 psc);

uint32_t Timer_GetTick(void);
#endif
