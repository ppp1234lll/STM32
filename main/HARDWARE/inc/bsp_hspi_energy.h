#ifndef __BSP_HSPI_ENERGY_H
#define __BSP_HSPI_ENERGY_H

#include "sys.h"
								  
/* 提供给其他C文件调用的函数 */
void    bsp_InitHSPI(void);			 		   // 初始化SPI口
uint8_t HSPI_ReadWriteByte(uint8_t TxData); // SPI总线读写一个字节

void HSPI_WriteByte(uint8_t TxData) ;
void HSPI_Write_Multi_Byte(uint8_t *buff, uint16_t len);	 
uint8_t HSPI_ReadByte(void);	 
void HSPI_test(void);

#endif


