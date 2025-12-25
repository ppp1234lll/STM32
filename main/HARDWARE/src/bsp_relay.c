#include "bsp_relay.h"
#include "delay.h"

/*
	3、继电器
			 继电器1：    PE8 
			 继电器2：    PE9
			 继电器3：    PE10
			 继电器4：    PE11
			 继电器5：    PE12
			 继电器6：    PE13
			 继电器7：    PE14
			 继电器8：    PE15		
*/
#define RELAY1_CTRL   PEout(8)
#define RELAY2_CTRL   PEout(9)
#define RELAY3_CTRL   PEout(10)
#define RELAY4_CTRL   PEout(11)
#define RELAY5_CTRL   PEout(12)
#define RELAY6_CTRL   PEout(13)
#define RELAY7_CTRL   PEout(14)
#define RELAY8_CTRL   PEout(15)

typedef struct
{
	uint8_t relay[8]; 
} relay_t;

relay_t sg_relay_t;

/*
*********************************************************************************************************
*	函 数 名: bsp_InitRelay
*	功能说明: 继电器初始化
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitRelay(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE);

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT; // 输出
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;  // 上拉
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
	GPIO_Init(GPIOE,&GPIO_InitStructure);
	GPIO_ResetBits(GPIOE,GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);
	relay_control(RELAY_1,RELAY_ON);
	relay_control(RELAY_2,RELAY_ON);
	relay_control(RELAY_3,RELAY_ON);
	relay_control(RELAY_4,RELAY_ON);
	relay_control(RELAY_5,RELAY_ON);
	relay_control(RELAY_6,RELAY_ON);
	relay_control(RELAY_7,RELAY_ON);
	relay_control(RELAY_8,RELAY_ON);	
}

/*
*********************************************************************************************************
*	函 数 名: relay_control
*	功能说明: 继电器控制
*	形    参:  dev  : 继电器序号
*	           state: 继电器状态
*	返 回 值: 无
*********************************************************************************************************
*/
void relay_control(RELAY_DEV dev, RELAY_STATUS state)
{
	switch(dev)
	{
		case RELAY_1:
			sg_relay_t.relay[RELAY_1] = state;
			RELAY1_CTRL = (state?0:1);
			break;
			
		case RELAY_2:
			sg_relay_t.relay[RELAY_2] = state;
			RELAY2_CTRL = (state?0:1);
			break;

		case RELAY_3:
			sg_relay_t.relay[RELAY_3] = state;
			RELAY3_CTRL = (state?0:1);
			break;

		case RELAY_4:
			sg_relay_t.relay[RELAY_4] = state;
			RELAY4_CTRL = (state?0:1);
			break;
			
		case RELAY_5:
			sg_relay_t.relay[RELAY_5] = state;
			RELAY5_CTRL = (state?0:1);
			break;
			
		case RELAY_6:
			sg_relay_t.relay[RELAY_6] = state;
			RELAY6_CTRL = (state?0:1);
			break;
			
		case RELAY_7:
			sg_relay_t.relay[RELAY_7] = state;
			RELAY7_CTRL = (state?0:1);
			break;
			
		case RELAY_8:
			sg_relay_t.relay[RELAY_8] = state;
			RELAY8_CTRL = (state?0:1);
			break;
			
		default:	break;
	}
}

/*
*********************************************************************************************************
*	函 数 名: relay_get_status
*	功能说明: 获取继电器状态
*	形    参: 无
*	返 回 值: 继电器状态
*********************************************************************************************************
*/
uint8_t relay_get_status(RELAY_DEV dev)
{
	return sg_relay_t.relay[dev];
}

/*
*********************************************************************************************************
*	函 数 名: relay_test
*	功能说明: 继电器测试
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void relay_test(void)
{
	while(1)
	{
		for(uint8_t i=0;i<8;i++)
		{
			relay_control((RELAY_DEV)i,RELAY_ON); 
			delay_ms(500);
		}
		delay_ms(2000);
		
		for(uint8_t i=0;i<8;i++)
		{
			relay_control((RELAY_DEV)i,RELAY_OFF); 
			delay_ms(500);
		}
		delay_ms(2000);
	}
}


