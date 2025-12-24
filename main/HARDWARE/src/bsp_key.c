#include "bsp_key.h"
#include "appconfig.h"
/*
	4、输入检测
		    按键(恢复出厂设置):    PD2
		    箱门检测:              PA11
		    12V电源输入监测:       PD0
		    水浸 :                 PD13	
        输入检测1：           PD14
        输入检测2：           PD15
        输入检测3：           PC8
        市电火-地：           PA3
        市电零-地：           PA4
        市电零火线：          PA5
				
				光敏电阻输入：        PC0
*/
#define RESET_KEY_READ 			GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_2)   	// 复位检测
#define OPEN_DOOR_READ  		GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_11)     // 箱门检测
#define PWR_TST_READ   	 		GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_0)     // 12V检测
#define WATER_READ   	 			GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_13)   	// 浸水检测
#define INPUT1_READ   	 		GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_14)   	// 输入1
#define INPUT2_READ   	 		GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_15)   	// 输入2
#define INPUT3_READ   	 		GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_8)   	// 输入3
#define LIGHT_READ   	 		  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_0)   	// 输入3
/*
*********************************************************************************************************
*	函 数 名: bsp_InitKey
*	功能说明: 初始化按键.  
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitKey(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOC,ENABLE);

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0|GPIO_Pin_2|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;  // 输入
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  // 上拉输入
	GPIO_Init(GPIOD,&GPIO_InitStructure); 	// 初始化GPIOD
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;  // 输入
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  // 上拉输入
	GPIO_Init(GPIOA,&GPIO_InitStructure); 	// 初始化GPIOC
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0|GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;  // 输入
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  // 上拉输入
	GPIO_Init(GPIOC,&GPIO_InitStructure); 	// 初始化GPIOC
  
}

/*
*********************************************************************************************************
*	函 数 名: key_test
*	功能说明: 按键测试.  
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void key_test(void)
{
	while(1)
	{
		printf("RESET = %d...",RESET_KEY_READ);
		printf("DOOR  = %d...",OPEN_DOOR_READ);
		printf("PWR   = %d...",PWR_TST_READ);
		printf("WATER = %d...",WATER_READ);
		printf("IN1   = %d...",INPUT1_READ);
		printf("IN2   = %d...",INPUT2_READ);
		printf("IN3   = %d...",INPUT3_READ);
		printf("LIGHT = %d\n ",LIGHT_READ);
		delay_ms(1000);		
	}
}

