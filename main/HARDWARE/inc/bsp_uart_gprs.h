#ifndef _BSP_UART_GPRS_H_
#define _BSP_UART_GPRS_H_

#include "sys.h"

/* 提供给其他C文件调用的函数 */
void bsp_InitUart_GPRS(uint32_t bound);
void uart_gprs_send_str(uint8_t *buff, uint16_t len);

#endif
