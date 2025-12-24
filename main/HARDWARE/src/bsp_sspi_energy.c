/********************************************************************************
* @File name  : 电能计量驱动
* @Description: 模拟SPI通信
* @Author     : ZHLE
*  Version Date        Modification Description
	7、BL0942单相计量芯片: 软件SPI
	   引脚分配为： RCD_SCLK：  PB10
                  RCD_MISO：  PB14		
                  RCD_MOSI：  PB15
********************************************************************************/

#include "bsp_sspi_energy.h"
#include "delay.h"
#include "includes.h"


#define SOFT_SPI_SCLK_GPIO_CLK		RCC_AHB1Periph_GPIOB   
#define SOFT_SPI_SCLK_GPIO 				GPIOB
#define SOFT_SPI_SCLK_PIN  				GPIO_Pin_10

#define SOFT_SPI_MOSI_GPIO_CLK		RCC_AHB1Periph_GPIOB
#define SOFT_SPI_MOSI_GPIO 				GPIOB
#define SOFT_SPI_MOSI_PIN 				GPIO_Pin_15

#define SOFT_SPI_MISO_GPIO_CLK		RCC_AHB1Periph_GPIOB
#define SOFT_SPI_MISO_GPIO 				GPIOB
#define SOFT_SPI_MISO_PIN 				GPIO_Pin_14

#define SOFT_SPI_SCLK		PBout(10)
#define SOFT_SPI_MOSI		PBout(15)
#define SOFT_SPI_MISO		PBin(14)
 	    					

/*
*********************************************************************************************************
*	函 数 名: bsp_InitSSPI
*	功能说明: 配置模拟SPI GPIO。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitSSPI(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd( SOFT_SPI_SCLK_GPIO_CLK|SOFT_SPI_MOSI_GPIO_CLK|
													SOFT_SPI_MISO_GPIO_CLK,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin   = SOFT_SPI_SCLK_PIN;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;			
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SOFT_SPI_SCLK_GPIO,&GPIO_InitStructure); 	

	GPIO_InitStructure.GPIO_Pin   = SOFT_SPI_MOSI_PIN;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;			
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SOFT_SPI_MOSI_GPIO,&GPIO_InitStructure); 
	
	GPIO_InitStructure.GPIO_Pin   = SOFT_SPI_MISO_PIN;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SOFT_SPI_MISO_GPIO,&GPIO_InitStructure);
	
}

/*
*********************************************************************************************************
*	函 数 名: sspi_delay
*	功能说明: 软件SPI延时
*	形    参: time 时间
*	返 回 值: 无
*********************************************************************************************************
*/
void sspi_delay(uint16_t time)	
{
	do
	{
	} while (--time);

}

/*
*********************************************************************************************************
*	函 数 名: SSPI_ReadWriteByte
*	功能说明: 读写字节函数 
*	形    参: 
*	@TxData		: 写入字节
*	返 回 值: 读取到的字节
*********************************************************************************************************
*/
uint8_t SSPI_ReadWriteByte(uint8_t TxData)
{
	uint8_t RecevieData=0;
	uint8_t i = 0;

	for(i=0; i<8; i++)
	{
		SOFT_SPI_SCLK=0;
		sspi_delay(20);
		if(TxData&0x80) SOFT_SPI_MOSI=1;
		else SOFT_SPI_MOSI=0;
		TxData<<=1;
		sspi_delay(20);
		SOFT_SPI_SCLK=1;  // 上升沿采样
		sspi_delay(20);
		RecevieData<<=1;
		if(SOFT_SPI_MISO) RecevieData |= 0x01;
		else RecevieData &= ~0x01;   // 下降沿接收数据
		sspi_delay(20);
	}
	SOFT_SPI_SCLK=0;  // idle情况下SCK为电平
	sspi_delay(20);
	return RecevieData;
}

/*
*********************************************************************************************************
*	函 数 名: SSPI_WriteByte
*	功能说明: 模拟 SPI 写一个字节
*	形    参: 
*	@TxData		: 写入字节
*	返 回 值:  
*********************************************************************************************************
*/
void SSPI_WriteByte(uint8_t TxData)  
{
	uint8_t i = 0;  
	for(i=0; i<8; i++) 
	{
		SOFT_SPI_SCLK = 0; //CPOL=0        //拉低时钟，即空闲时钟为低电平， CPOL=0；
		if(TxData&0x80) SOFT_SPI_MOSI=1;
		else SOFT_SPI_MOSI=0;
		TxData<<=1;
		sspi_delay(20); 
		SOFT_SPI_SCLK=1;                   // 上升沿采样 //CPHA=0  
		sspi_delay(20); 
	}
	SOFT_SPI_SCLK = 0;                   // 最后SPI发送完后，拉低时钟，进入空闲状态；
}
/*
*********************************************************************************************************
*	函 数 名: SSPI_Write_Multi_Byte
*	功能说明: 模拟 SPI 写多个字节
*	形    参: 
*	@TxData		: 写入字节
*	返 回 值:  
*********************************************************************************************************
*/
void SSPI_Write_Multi_Byte(uint8_t *buff, uint16_t len)
{
	while(len--) {
		SSPI_WriteByte(buff[0]);
		buff++;
	}
}

/*
*********************************************************************************************************
*	函 数 名: SSPI_ReadByte
*	功能说明: 模拟 SPI 读一个字节
*	形    参: 
*	返 回 值: 读取到的字节
*********************************************************************************************************
*/
uint8_t SSPI_ReadByte(void)
{
	uint8_t i = 0;
	uint8_t RecevieData=0;
	for(i=0; i<8; i++) 
	{
		SOFT_SPI_SCLK = 1;           //拉低时钟，即空闲时钟为低电平；  
		sspi_delay(20); 
		SOFT_SPI_SCLK = 0;   
		RecevieData<<=1;
		if(SOFT_SPI_MISO) RecevieData |= 0x01;
		else RecevieData &= ~0x01;   // 下降沿接收数据
		sspi_delay(20); 
	}
	SOFT_SPI_SCLK=0;  // idle情况下SCK为电平
	return RecevieData;
}  


/*
*********************************************************************************************************
*	函 数 名: SSPI_test
*	功能说明: SPI测试
*	形    参: 
*	返 回 值:  
*********************************************************************************************************
*/
void SSPI_test(void)
{
	while(1)
	{
		SSPI_ReadByte();
	}
}






