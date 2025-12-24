#include "bsp_led.h"
#include "delay.h"

/*
	2、指示灯（原理图）：
		外接电源指示灯  POWER_OUT  : PE6
		外接网口指示灯  LAN_OUT    : PE5 
		
		系统状态指示灯  STATE      : PB8
		网口指示灯      LAN        : PB9
		4G指示灯        GPRS       : PE0
*/

#define LED_STATE       PBout(8)
#define LED_LAN         PBout(9)
#define LED_GPRS        PEout(0)
#define LED_LAN_OUT     PEout(5)
#define LED_PWR_OUT     PEout(6)

#define LED_STATE_TOG   GPIO_ReadOutputDataBit(GPIOB,GPIO_Pin_8) 
#define LED_LAN_STA     GPIO_ReadOutputDataBit(GPIOB,GPIO_Pin_9) 
#define LED_GPRS_TOG    GPIO_ReadOutputDataBit(GPIOE,GPIO_Pin_0) 
#define LED_LAN_O_TOG   GPIO_ReadOutputDataBit(GPIOE,GPIO_Pin_5) 

/* 指示灯闪烁时间*/
#define FLICKER_TIME_Q	(200)
#define FLICKER_TIME 		(500)
#define FLICKER_TIME_1S (1000)

/* 指示灯状态变量*/
typedef struct
{
	uint8_t gprs;
	uint8_t lan;
	uint8_t state;
	uint8_t lan_out;
	uint8_t power_out;
} led_flicker_t;

led_flicker_t sg_ledflicker_t = {0};

/*
*********************************************************************************************************
*	函 数 名: bsp_InitLed
*	功能说明: 初始化指示灯控制io:默认不开启
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitLed(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOE,ENABLE);

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_8|GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;			// 输出
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;  		// 推挽输出
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;   // 上拉
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; 	// 高速GPIO
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_8|GPIO_Pin_9);

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0|GPIO_Pin_5|GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;			// 输出
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;  		// 推挽输出
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;   // 上拉
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; 	// 高速GPIO
	GPIO_Init(GPIOE,&GPIO_InitStructure);
	GPIO_SetBits  (GPIOE,GPIO_Pin_0);
	GPIO_ResetBits(GPIOE,GPIO_Pin_5|GPIO_Pin_6);
}

/*
*********************************************************************************************************
*	函 数 名: led_control_function
*	功能说明: 点亮指定的LED指示灯。
*	形    参:  dev  : 指示灯序号
*	           state: 指示灯状态
*	返 回 值: 无
*********************************************************************************************************
*/
void led_control_function(LD_DEV dev, LED_STATUS state)
{
	switch(dev)
	{
		case LD_STATE:  // 系统运行指示灯
			sg_ledflicker_t.state = state;
			switch(state) 
			{
        case LD_ON:	 LED_STATE = 0;break;
        case LD_OFF: LED_STATE = 1;break;
        default:break;
      }
      break;
			
		case LD_GPRS:  // 4G指示灯
			sg_ledflicker_t.gprs = state;
			switch(state) 
			{
        case LD_ON:			LED_GPRS = 0;break;
        case LD_OFF:		LED_GPRS = 1;break;
        default:break;
			}
			break;
			
		case LD_LAN:   // 有线指示灯
			sg_ledflicker_t.lan = state;
			switch(state) 
			{
        case LD_ON:			LED_LAN = 0;break;
        case LD_OFF:		LED_LAN = 1;break;
        default:break;
			}
			break;

		default:		break;
	}
}

/*
*********************************************************************************************************
*	函 数 名: led_out_control_function
*	功能说明: 点亮外部LED指示灯。
*	形    参:  dev  : 指示灯序号
*	           state: 指示灯状态
*	返 回 值: 无
*********************************************************************************************************
*/
void led_out_control_function(LD_DEV dev, LED_STATUS state)
{
	switch(dev)
	{
		case LD_PWR_O:	
			LED_PWR_OUT = state;
		break;
		case LD_LAN_O:
			sg_ledflicker_t.lan_out = state;
			switch(state) 
			{
				case LD_ON:			LED_LAN_OUT = 1;break;
				case LD_OFF:		LED_LAN_OUT = 0;break;
				default:break;
			}
			break;
		default:			break;
	}
}

/*
*********************************************************************************************************
*	函 数 名: led_flicker_control_timer_function
*	功能说明: led闪动
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void led_flicker_control_timer_function(void)
{
	static uint16_t count   = 0;
	static uint16_t count2	= 0;
	static uint16_t count3	= 0;
	
	count++;
	if(count > FLICKER_TIME)
	{
		count = 0;
		if(sg_ledflicker_t.gprs == LD_FLICKER)		/* 显示无线网络状态 */
		{
			LED_GPRS = !LED_GPRS_TOG;
		}

		if(sg_ledflicker_t.lan == LD_FLICKER)		/* 显示有线网络状态 */
		{
			LED_LAN = !LED_LAN_STA;
		} 
	
		if(sg_ledflicker_t.state == LD_FLICKER) 	/* 系统状态灯 */
		{
			LED_STATE = !LED_STATE_TOG;
		}
		if(sg_ledflicker_t.lan_out == LD_FLICKER) 	/* 外部主网络状态 */
		{
			LED_LAN_OUT = !LED_LAN_O_TOG;
		}
	}

	count3++;
	if(count3 > FLICKER_TIME_Q)
	{
		count3 = 0;
		if(sg_ledflicker_t.gprs == LD_FLIC_Q)		/* 显示无线网络状态 */
			LED_GPRS = !LED_GPRS_TOG;

		if(sg_ledflicker_t.lan == LD_FLIC_Q)		/* 显示有线网络状态 */
			LED_LAN = !LED_LAN_STA;
	}
}

/*
*********************************************************************************************************
*	函 数 名: led_test
*	功能说明: led测试
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void led_test(void)
{
	while(1)
	{
		GPIO_SetBits  (GPIOB,GPIO_Pin_8|GPIO_Pin_9);
		GPIO_SetBits  (GPIOE,GPIO_Pin_0);
		GPIO_ResetBits(GPIOE,GPIO_Pin_5|GPIO_Pin_6);
		delay_ms(1000);
		GPIO_ResetBits(GPIOB,GPIO_Pin_8|GPIO_Pin_9);
		GPIO_ResetBits(GPIOE,GPIO_Pin_0);
		GPIO_SetBits  (GPIOE,GPIO_Pin_5|GPIO_Pin_6);
		delay_ms(1000);	
	}
}
/******************************************  (END OF FILE) **********************************************/


