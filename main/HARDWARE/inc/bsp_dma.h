#ifndef _BSP_DMA_H_
#define _BSP_DMA_H_

#include "sys.h"

/* 提供给其他C文件调用的函数 */
void DMA_Config(DMA_Stream_TypeDef *DMA_Streamx,uint32_t chx,uint32_t par,uint32_t mar,uint32_t dir,uint16_t ndtr);
void DMA_Enable(DMA_Stream_TypeDef *DMA_Streamx,uint16_t ndtr);


#endif
