#ifndef __BSP_RNG_H
#define __BSP_RNG_H	 

#include "sys.h" 


u8  bsp_InitRNG(void);			//RNG初始化 
u32 RNG_Get_RandomNum(void);//得到随机数
int RNG_Get_RandomRange(int min,int max);//生成[min,max]范围的随机数

void rng_test(void);

#endif

















