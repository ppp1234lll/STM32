#ifndef __BSP_SPI_FLASH_H
#define __BSP_SPI_FLASH_H

#include "sys.h"
	
/* 提供给其他C文件调用的函数 */	
void bsp_InitSPIFlash(void);			 		   // 初始化SPI口
uint8_t SPI_ReadWriteByte(uint8_t TxData); // SPI总线读写一个字节
		 
#endif

