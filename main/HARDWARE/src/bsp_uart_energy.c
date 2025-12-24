/********************************************************************************
* @File name  : 电能计量驱动
* @Description: 串口通信
* @Author     : ZHLE
*  Version Date        Modification Description
	7、单相计量芯片: 串口1，波特率4800，
	   引脚分配为：  USART1_TX： PA9
                   USART1_RX： PA10

********************************************************************************/

#include "bsp_uart_energy.h"
#include "BL0910.h"
#include "bsp_dma.h"

#define UART_ENERGY_RX_DMA 

#define UART_ENERGY_TX_DMA_STREAM             DMA2_Stream7
#define UART_ENERGY_RX_DMA_STREAM             DMA2_Stream2

#define UART_ENERGY_TX_DMA_CH                 DMA_Channel_4
#define UART_ENERGY_RX_DMA_CH                 DMA_Channel_4

/* 参数 */
#define UART_ENERGY_RX_MAX 100

uint8_t elec_recv_buff[UART_ENERGY_RX_MAX] = {0};

/*
*********************************************************************************************************
*	函 数 名: bsp_InitUART_Energy
*	功能说明: 配置串口
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitUART_Energy(u32 bound)
{
	/* GPIO端口设置 */
	GPIO_InitTypeDef  GPIO_InitStructure  = {0};
	USART_InitTypeDef USART_InitStructure = {0};
	NVIC_InitTypeDef  NVIC_InitStructure  = {0};

	RCC_AHB1PeriphClockCmd(UART_ENERGY_TX_GPIO_CLK|UART_ENERGY_RX_GPIO_CLK, ENABLE);  
	RCC_APB2PeriphClockCmd(UART_ENERGY_USART_CLK,ENABLE);

	/* 串口对应引脚复用映射 */
	GPIO_PinAFConfig(UART_ENERGY_TX_GPIO_PORT,GPIO_PinSource9, GPIO_AF_UART5); 						// GPIOC10复用为UART4
	GPIO_PinAFConfig(UART_ENERGY_RX_GPIO_PORT,GPIO_PinSource10,GPIO_AF_UART5); 						// GPIOC11复用为UART4
	
	/* USART端口配置 */
	GPIO_InitStructure.GPIO_Pin   = UART_ENERGY_TX_PIN; 	
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF;		  // 复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		// 速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 			// 推挽复用输出
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_UP; 			// 上拉
	GPIO_Init(UART_ENERGY_TX_GPIO_PORT,&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin   = UART_ENERGY_RX_PIN; 	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;		  // 复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		// 速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 			// 推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 		  // 上拉
	GPIO_Init(UART_ENERGY_RX_GPIO_PORT,&GPIO_InitStructure);
	
	/* USART 初始化设置 */
	USART_DeInit(UART_ENERGY_USART);
	USART_InitStructure.USART_BaudRate = bound;									// 串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						// 字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							// 一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;							// 偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					// 收发模式
	USART_Init(UART_ENERGY_USART, &USART_InitStructure);	 

	/* Usart NVIC 配置 */
	NVIC_InitStructure.NVIC_IRQChannel = UART_ENERGY_IRQn; // 串口中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority= 2; // 抢占优先级2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1 ; // 子优先级1
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; // IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	

	#ifdef UART_ENERGY_RX_DMA
	DMA_Config(UART_ENERGY_RX_DMA_STREAM,UART_ENERGY_RX_DMA_CH,(uint32_t)&(UART_ENERGY_USART->DR),(uint32_t)elec_recv_buff,DMA_DIR_PeripheralToMemory,UART_ENERGY_RX_MAX);
	USART_ITConfig(UART_ENERGY_USART, USART_IT_IDLE, ENABLE); // 开启空闲中断
	USART_DMACmd(UART_ENERGY_USART, USART_DMAReq_Rx, ENABLE);
	#else
	USART_ITConfig(UART_ENERGY_USART, USART_IT_RXNE, ENABLE); // 开启相关中断
	#endif
	
	USART_ClearFlag(UART_ENERGY_USART, USART_FLAG_TC);
	USART_Cmd(UART_ENERGY_USART, ENABLE); // 使能串口4
}

/*
*********************************************************************************************************
*	函 数 名: uart_energy_send_char
*	功能说明: 向串口发送1个字节。
*	形    参: 
*	@ch			: 待发送的字节数据
*	返 回 值: 无
*********************************************************************************************************
*/
void uart_energy_send_char(uint8_t ch)
{
	UART_ENERGY_USART->DR = (int8_t) ch;
	while((UART_ENERGY_USART->SR&0X40)==0);
}

/*
*********************************************************************************************************
*	函 数 名: uart_energy_send_str
*	功能说明: 向串口发送字符串。
*	形    参: 
*	@buff		: 字符串指针
*	@len		: 发送数据长度
*	返 回 值: 无
*********************************************************************************************************
*/
void uart_energy_send_str(uint8_t *data, uint16_t len)
{
	while(len--) {
		uart_energy_send_char(data[0]);
		data++;
	}
}

/*
*********************************************************************************************************
*	函 数 名: USART_DEBUG_IRQHandler
*	功能说明: 供中断服务程序调用，通用串口中断处理函数
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void UART_ENERGY_IRQHandler(void)
{
	#ifdef UART_ENERGY_RX_DMA
	uint16_t size = 0;

	/* 空闲中断接收 */
	if( USART_GetITStatus(UART_ENERGY_USART,USART_IT_IDLE) != RESET ) 
	{
		USART_ClearITPendingBit(UART_ENERGY_USART,USART_IT_IDLE);
		USART_ReceiveData(UART_ENERGY_USART);

		DMA_Cmd(UART_ENERGY_RX_DMA_STREAM, DISABLE); // 关闭DMA，准备重新配置
		size = UART_ENERGY_RX_MAX - DMA_GetCurrDataCounter(UART_ENERGY_RX_DMA_STREAM); // 计算接收数据长度					
		bl0910_get_rec_data_function(elec_recv_buff,size); // 填充接收到的数据		
		DMA_Enable(UART_ENERGY_RX_DMA_STREAM,UART_ENERGY_RX_MAX);
	}
	#else
	static uint8_t test = 0;
	uint8_t Res = 0;
	if(USART_GetITStatus(UART_ENERGY_USART,USART_IT_RXNE)!=RESET)
	{
		USART_ClearITPendingBit(UART_ENERGY_USART,USART_IT_RXNE);
		Res = USART_ReceiveData(UART_ENERGY_USART);
		elec_recv_buff[test++] = Res;
		printf("02x\n,Res");
	}
	#endif
}



