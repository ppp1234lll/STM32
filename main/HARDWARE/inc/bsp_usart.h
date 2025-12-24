#ifndef _BSP_USART_H_
#define _BSP_USART_H_

#include "sys.h"

void bsp_InitUart(uint32_t baudrate);
void Usart_Send_Str(uint8_t *buff, uint16_t len);
void usart_test(void);


#endif
