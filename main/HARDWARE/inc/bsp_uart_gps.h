#ifndef _BSP_UART_GPS_H_
#define _BSP_UART_GPS_H_

#include "sys.h"

/* 提供给其他C文件调用的函数 */
void bsp_InitUart_GPS(uint32_t bound);
void uart_gps_send_str(uint8_t *buff, uint16_t len);

#endif
