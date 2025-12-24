/********************************************************************************
* @File name  : 电能计量驱动
* @Description: 硬件SPI通信
* @Author     : ZHLE
*  Version Date        Modification Description
	6、BL0910单相计量芯片: 硬件SPI3
	   引脚分配为： BL_SCLK：  PC10
                  BL_MISO： PC11 
                  BL_MOSI： PC11

********************************************************************************/

#include "bsp_hspi_energy.h"
#include "bsp.h"

/*
	6、BL0910单相计量芯片: 硬件SPI3
	   引脚分配为： BL_SCLK：  PC10
                  BL_MISO：  PC11			
                  BL_MOSI：  PC12
*/

#define HARD_SPI_SCLK_GPIO_CLK		RCC_AHB1Periph_GPIOC
#define HARD_SPI_SCLK_GPIO 				GPIOC
#define HARD_SPI_SCLK_PIN  				GPIO_Pin_10

#define HARD_SPI_MOSI_GPIO_CLK		RCC_AHB1Periph_GPIOC
#define HARD_SPI_MOSI_GPIO 				GPIOC
#define HARD_SPI_MOSI_PIN 				GPIO_Pin_12

#define HARD_SPI_MISO_GPIO_CLK		RCC_AHB1Periph_GPIOC
#define HARD_SPI_MISO_GPIO 				GPIOC
#define HARD_SPI_MISO_PIN 				GPIO_Pin_11

#define HARD_SPI_SCLK		PCout(10)
#define HARD_SPI_MOSI		PCout(12)
#define HARD_SPI_MISO		PCin(11)

/*
*********************************************************************************************************
*	函 数 名: bsp_InitHSPI
*	功能说明: 配置SPI GPIO
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitHSPI(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd( HARD_SPI_SCLK_GPIO_CLK|HARD_SPI_MOSI_GPIO_CLK|    
													HARD_SPI_MISO_GPIO_CLK,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin   = HARD_SPI_SCLK_PIN;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;			// 输出
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(HARD_SPI_SCLK_GPIO,&GPIO_InitStructure); 	

	GPIO_InitStructure.GPIO_Pin   = HARD_SPI_MOSI_PIN;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;			// 输出
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(HARD_SPI_MOSI_GPIO,&GPIO_InitStructure); 
	
	GPIO_InitStructure.GPIO_Pin   = HARD_SPI_MISO_PIN;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(HARD_SPI_MISO_GPIO,&GPIO_InitStructure);

}


/*
*********************************************************************************************************
*	函 数 名: hspi_delay
*	功能说明: spi延时
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void hspi_delay(uint16_t time)	
{
	do
	{
	} while (--time);

}

/*
*********************************************************************************************************
*	函 数 名: HSPI_ReadWriteByte
*	功能说明: 读写字节函数
*	形    参: 
*	@TxData		: 写入字节
*	返 回 值: 无
*********************************************************************************************************
*/
uint8_t HSPI_ReadWriteByte(uint8_t TxData)
{
	uint8_t RecevieData=0;
	uint8_t i = 0;

	for(i=0; i<8; i++)
	{
		HARD_SPI_SCLK=0;
		hspi_delay(6);
		if(TxData&0x80) HARD_SPI_MOSI=1;
		else HARD_SPI_MOSI=0;
		TxData<<=1;
		hspi_delay(6);
		HARD_SPI_SCLK=1;  // 上升沿采样
		hspi_delay(6);
		RecevieData<<=1;
		if(HARD_SPI_MISO) RecevieData |= 0x01;
		else RecevieData &= ~0x01;   // 下降沿接收数据
		hspi_delay(6);
	}
	HARD_SPI_SCLK=0;  // idle情况下SCK为电平
	hspi_delay(6);
	return RecevieData;
}

/*
*********************************************************************************************************
*	函 数 名: HSPI_WriteByte
*	功能说明: 模拟 SPI 写一个字节
*	形    参: 
*	@TxData		: 写入字节
*	返 回 值: 无
*********************************************************************************************************
*/
void HSPI_WriteByte(uint8_t TxData)  
{
	uint8_t i = 0;  
	for(i=0; i<8; i++) 
	{
		HARD_SPI_SCLK = 0;                //拉低时钟，即空闲时钟为低电平， CPOL=0；
		if(TxData&0x80) HARD_SPI_MOSI=1;
		else HARD_SPI_MOSI=0;
		TxData<<=1;
		hspi_delay(15); 
		HARD_SPI_SCLK=1;          // 上升沿采样 //CPHA=0   
		hspi_delay(15); 
	}
	HARD_SPI_SCLK = 0;         // 最后SPI发送完后，拉低时钟，进入空闲状态；
}

/*
*********************************************************************************************************
*	函 数 名: HSPI_Write_Multi_Byte
*	功能说明: 模拟 SPI 写数据
*	形    参: 
*	@TxData		: 写入字节
*	返 回 值: 无
*********************************************************************************************************
*/
void HSPI_Write_Multi_Byte(uint8_t *buff, uint16_t len)
{
	while(len--) {
		HSPI_WriteByte(buff[0]);
		buff++;
	}
}

/*
*********************************************************************************************************
*	函 数 名: HSPI_ReadByte
*	功能说明: 模拟 SPI 读一个字节
*	形    参: 
*	返 回 值: 读到的数据
*********************************************************************************************************
*/
uint8_t HSPI_ReadByte(void)
{
	uint8_t i = 0;
	uint8_t RecevieData=0;
	for(i=0; i<8; i++) 
	{
		HARD_SPI_SCLK = 1;           //拉低时钟，即空闲时钟为低电平；  
		hspi_delay(15); 
		HARD_SPI_SCLK = 0;   
		RecevieData<<=1;
		if(HARD_SPI_MISO) 
			RecevieData |= 0x01;
		else 
			RecevieData &= ~0x01;   // 下降沿接收数据
		hspi_delay(15); 
	}
	return RecevieData;
}


/*
*********************************************************************************************************
*	函 数 名: HSPI_test
*	功能说明: SPI测试
*	形    参: 
*	返 回 值:  
*********************************************************************************************************
*/
void HSPI_test(void)
{
	while(1)
	{
		HSPI_ReadByte();
	}
}


