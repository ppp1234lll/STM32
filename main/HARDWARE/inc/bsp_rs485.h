
#ifndef _BSP_RS485_H_
#define _BSP_RS485_H_

#include "sys.h"

/* 提供给其他C文件调用的函数 */
void bsp_InitRS485(uint32_t bound);
void rs485_send_char(uint8_t ch);
void rs485_send_str(uint8_t *data, uint16_t len);

void rs485_test(void);

#endif
