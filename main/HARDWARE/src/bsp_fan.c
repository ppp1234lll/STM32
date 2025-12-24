/********************************************************************************
* @File name  : 风扇驱动
* @Description:  
* @Author     : ZHLE
*  Version Date        Modification Description
	16、输出控制：
	    风扇:          PA12

********************************************************************************/

#include "bsp_fan.h"
#include "delay.h"

#define FAN1_CTRL   PAout(12)

/*
*********************************************************************************************************
*	函 数 名: bsp_InitFan
*	功能说明: 风扇初始化
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitFan(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);

	GPIO_InitStructure.GPIO_Pin    = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode   = GPIO_Mode_OUT;			// 输出
	GPIO_InitStructure.GPIO_OType  = GPIO_OType_PP;  		// 推挽输出
	GPIO_InitStructure.GPIO_PuPd   = GPIO_PuPd_NOPULL;   	// 上拉
	GPIO_InitStructure.GPIO_Speed  = GPIO_Speed_100MHz; 	// 高速GPIO
	GPIO_Init(GPIOA,&GPIO_InitStructure);
  GPIO_ResetBits(GPIOA,GPIO_Pin_12);
}

/*
*********************************************************************************************************
*	函 数 名: fan_control
*	功能说明: 继电器控制
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void fan_control( FAN_STATUS state)
{
  FAN1_CTRL = (state?0:1);
}

/*
*********************************************************************************************************
*	函 数 名: fan_test
*	功能说明: 风扇测试
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void fan_test(void)
{
	while(1)
	{
		fan_control(FAN_ON); 
		delay_ms(5000);
		fan_control(FAN_OFF);  
		delay_ms(5000);	
	}
}


