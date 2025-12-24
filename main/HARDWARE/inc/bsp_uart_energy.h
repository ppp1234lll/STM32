/********************************************************************************
* @File name  : bsp_uart_energy.h
* @Description: 电能计量芯片-串口通信驱动
* @Author     : FENGNIAO
*  Version Date    Modification Description
	7、单相计量芯片: 串口1，波特率4800，
	   引脚分配为：  USART1_TX： PA9
                   USART1_RX： PA10

********************************************************************************/

#ifndef _BSP_UART_ENERGY_H_
#define _BSP_UART_ENERGY_H_
#include "sys.h"

/* 宏定义 */
#define UART_ENERGY_TX_GPIO_CLK               RCC_AHB1Periph_GPIOA
#define UART_ENERGY_TX_GPIO_PORT              GPIOA
#define UART_ENERGY_TX_PIN                    GPIO_Pin_9

#define UART_ENERGY_RX_GPIO_CLK               RCC_AHB1Periph_GPIOA
#define UART_ENERGY_RX_GPIO_PORT              GPIOA
#define UART_ENERGY_RX_PIN                    GPIO_Pin_10

#define UART_ENERGY_USART_CLK                 RCC_APB1Periph_UART5
#define UART_ENERGY_USART                     UART5
#define UART_ENERGY_IRQn                      UART5_IRQn
#define UART_ENERGY_IRQHandler                UART5_IRQHandler


/* 提供给其他C文件调用的函数 */
void bsp_InitUART_Energy(uint32_t bound);
void uart_energy_send_str(uint8_t *data, uint16_t len);

#endif
