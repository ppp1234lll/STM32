/********************************************************************************
* @File name  : 4G模块
* @Description: 串口2-对应4G
* @Author     : ZHLE
*  Version Date        Modification Description
	12、ML307（4G模块）：串口2，波特率115200，引脚分配为：   
        4G-TXD：    PD5
        4G-RXD：    PD6
********************************************************************************/

#include "bsp_uart_gprs.h"
#include "bsp_dma.h"
#include "GPRS.h"

#define UART_GPRS_TX_GPIO_CLK               RCC_AHB1Periph_GPIOD
#define UART_GPRS_TX_GPIO_PORT              GPIOD
#define UART_GPRS_TX_PIN                    GPIO_Pin_5

#define UART_GPRS_RX_GPIO_CLK               RCC_AHB1Periph_GPIOD
#define UART_GPRS_RX_GPIO_PORT              GPIOD
#define UART_GPRS_RX_PIN                    GPIO_Pin_6

#define UART_GPRS_CLK                       RCC_APB1Periph_USART2
#define UART_GPRS                           USART2
#define UART_GPRS_IRQn                      USART2_IRQn
#define UART_GPRS_IRQHandler                USART2_IRQHandler

#define UART_GPRS_RX_DMA 

#ifdef UART_GPRS_RX_DMA
#define UART_GPRS_TX_DMA_STREAM             DMA1_Stream6
#define UART_GPRS_RX_DMA_STREAM             DMA1_Stream5

#define UART_GPRS_TX_DMA_CH               	DMA_Channel_4
#define UART_GPRS_RX_DMA_CH                 DMA_Channel_4
#endif

/* 参数 */
#define UART_GPRS_RX_MAX  2048

uint8_t uart_gprs_rx_buff[UART_GPRS_RX_MAX] = {0};

/*
*********************************************************************************************************
*	函 数 名: bsp_InitUart_GPRS
*	功能说明: 初始化串口硬件 
*	形    参: baudrate: 波特率
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitUart_GPRS(uint32_t bound)
{
	/* GPIO端口设置 */
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* 时钟初始化 */
	RCC_AHB1PeriphClockCmd(UART_GPRS_TX_GPIO_CLK|UART_GPRS_RX_GPIO_CLK, ENABLE); 
	RCC_APB1PeriphClockCmd(UART_GPRS_CLK , ENABLE);

	/* 串口对应引脚复用映射 */
	GPIO_PinAFConfig(UART_GPRS_TX_GPIO_PORT,GPIO_PinSource5, GPIO_AF_USART2); 
	GPIO_PinAFConfig(UART_GPRS_RX_GPIO_PORT,GPIO_PinSource6,GPIO_AF_USART2);
	
	/* USART端口配置 */
	GPIO_InitStructure.GPIO_Pin   = UART_GPRS_TX_PIN; 
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;			// 输出
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;  		// 推挽输出
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;   	// 上拉
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; 	// 高速GPIO
	GPIO_Init(UART_GPRS_TX_GPIO_PORT, &GPIO_InitStructure);	

	GPIO_InitStructure.GPIO_Pin   = UART_GPRS_RX_PIN;		 
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;			// 输出
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;  		// 推挽输出
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;   	// 上拉
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; 	// 高速GPIO
	GPIO_Init(UART_GPRS_RX_GPIO_PORT, &GPIO_InitStructure);			

	NVIC_InitStructure.NVIC_IRQChannel = UART_GPRS_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		  
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  
	NVIC_Init(&NVIC_InitStructure);			

	/* USART 初始化设置 */
	USART_DeInit(UART_GPRS);
	USART_InitStructure.USART_BaudRate = bound;									// 串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						// 字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							// 一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;								// 无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					// 收发模式
	USART_Init(UART_GPRS, &USART_InitStructure);	 

	#ifdef UART_GPRS_RX_DMA
	DMA_Config( UART_GPRS_RX_DMA_STREAM,\
	            UART_GPRS_RX_DMA_CH,\
							(uint32_t)&(UART_GPRS->DR),\
							(uint32_t)uart_gprs_rx_buff,
							DMA_DIR_PeripheralToMemory,\
							UART_GPRS_RX_MAX);
	DMA_Enable(UART_GPRS_RX_DMA_STREAM,UART_GPRS_RX_MAX);
	USART_ITConfig(UART_GPRS, USART_IT_IDLE, ENABLE);
	USART_DMACmd(UART_GPRS,USART_DMAReq_Rx,ENABLE);
	#else
	USART_ITConfig(UART_GPRS, USART_IT_RXNE, ENABLE);  // 开启串口接受中断
	#endif
	
	USART_ClearITPendingBit(UART_GPRS, USART_IT_TC);
	USART_Cmd(UART_GPRS, ENABLE);                      // 使能串口 
}

/*
*********************************************************************************************************
*	函 数 名: uart_gprs_send_char
*	功能说明: 向串口发送1个字节。
*	形    参: 
*	@ch			: 待发送的字节数据
*	返 回 值: 无
*********************************************************************************************************
*/
void uart_gprs_send_char(uint8_t ch)
{
	UART_GPRS->DR = (uint8_t)ch;
	while ((UART_GPRS->SR & 0X40) == 0);
}

/*
*********************************************************************************************************
*	函 数 名: uart_gprs_send_str
*	功能说明: 向串口发送字符串。
*	形    参:  
*	@buff		: 字符串指针
*	@len		: 发送数据长度
*	返 回 值: 无
*********************************************************************************************************
*/
void uart_gprs_send_str(uint8_t *buff, uint16_t len)
{
	while(len--) {
		uart_gprs_send_char(buff[0]);
		buff++;
	}
}
/*
*********************************************************************************************************
*	函 数 名: UART_GPRS_IRQHandler
*	功能说明: 供中断服务程序调用，通用串口中断处理函数
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void UART_GPRS_IRQHandler(void)
{
#ifdef UART_GPRS_RX_DMA
	uint16_t size = 0;
	
	if (USART_GetITStatus(UART_GPRS, USART_IT_IDLE) != RESET) 
	{
		USART_ClearITPendingBit(UART_GPRS, USART_IT_IDLE);
		USART_ReceiveData(UART_GPRS);
		
		DMA_Cmd(UART_GPRS_RX_DMA_STREAM, DISABLE);/* 停止DMA */
		size = UART_GPRS_RX_MAX - DMA_GetCurrDataCounter(UART_GPRS_RX_DMA_STREAM);
		gprs_get_receive_data_function(uart_gprs_rx_buff,size);

		DMA_Enable(UART_GPRS_RX_DMA_STREAM,UART_GPRS_RX_MAX);/* 设置传输模式 */
	}
#else
	static uint8_t test = 0;
	uint8_t res = 0;
	
	if (USART_GetITStatus(UART_GPRS, USART_IT_RXNE) != RESET) {
		USART_ClearITPendingBit(UART_GPRS, USART_IT_RXNE);
		res = USART_ReceiveData(UART_GPRS);
//		printf("%c",res);
		uart_gprs_rx_buff[test++] = res;
	}
#endif
}



