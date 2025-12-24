#include "bsp_timer.h"
#include "appconfig.h"

/*
	全局运行时间，单位1ms
	最长可以表示 24.85天，如果你的产品连续运行时间超过这个数，则必须考虑溢出问题
*/
__IO int32_t g_iRunTime = 0;
/*
*********************************************************************************************************
*	函 数 名: TIM3_Int_Init
*	功能说明: 通用定时器3中断初始化
*	形    参: 无
*	arr：自动重装值。
*	psc：时钟预分频数
*	返 回 值: 无
*	定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
*	Ft=定时器工作频率,单位:Mhz
*********************************************************************************************************
*/
void TIM3_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);  			// 使能TIM3时钟
	
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  					// 定时器分频
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; 	// 向上计数模式
	TIM_TimeBaseInitStructure.TIM_Period=arr;   					// 自动重装载值
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);
		
	NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn; 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03; 
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);//清除更新中断请求位
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); 						// 允许定时器3更新中断
	TIM_Cmd(TIM3,ENABLE); 	
}
/*
*********************************************************************************************************
*	函 数 名: TIM6_Int_Init
*	功能说明: 基本定时器6中断初始化
*	形    参: 无
*	arr：自动重装值。
*	psc：时钟预分频数
*	返 回 值: 无
*********************************************************************************************************
*/
void TIM6_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6,ENABLE);  ///使能TIM3时钟
	
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //定时器分频
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInitStructure.TIM_Period=arr;   //自动重装载值
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	TIM_TimeBaseInit(TIM6,&TIM_TimeBaseInitStructure);
	
	TIM_ClearITPendingBit(TIM6,TIM_IT_Update);//清空中断状态
	TIM_ITConfig(TIM6,TIM_IT_Update,ENABLE); //允许定时器3更新中断
	TIM_Cmd(TIM6,ENABLE); //使能定时器3
	
	NVIC_InitStructure.NVIC_IRQChannel	=	TIM6_DAC_IRQn; //定时器3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority= 1; //抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority= 2; //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
/*
*********************************************************************************************************
*	函 数 名: TIM4_Int_Init
*	功能说明: 通用定时器4中断初始化
*	形    参: 无
*	arr：自动重装值。
*	psc：时钟预分频数
*	返 回 值: 无
*********************************************************************************************************
*/
void TIM4_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);  
	
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;    // 定时器分频
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; 	// 向上计数模式
	TIM_TimeBaseInitStructure.TIM_Period=arr;       // 自动重装载值
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseInitStructure);
		
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn; 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_ClearITPendingBit(TIM4, TIM_IT_Update);// 清除更新中断请求位
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE);   // 允许定时器3更新中断
	TIM_Cmd(TIM4,ENABLE); 	
}

/*
*********************************************************************************************************
*	函 数 名: TIM4_IRQHandler
*	功能说明: TIM4 中断服务程序
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void TIM4_IRQHandler(void)
{	
//	static uint16_t time4_count =0;
	if(TIM_GetITStatus(TIM4,TIM_IT_Update) != RESET)
	{
//		time4_count++;
//		if(time4_count >= 3)
//		{
//			time4_count =0;
//			printf("time4 test\n");
//		}	
		
		port_scan_timer_function();
		rtsp_timer_function();
		
		TIM_ClearITPendingBit(TIM4,TIM_IT_Update);//清空中断状态
	}
} 

/*
*********************************************************************************************************
*	函 数 名: TIM6_DAC_IRQHandler
*	功能说明: TIM6 中断服务程序
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void TIM6_DAC_IRQHandler(void)
{	
//	static uint16_t time6_count =0;
	if(TIM_GetITStatus(TIM6,TIM_IT_Update) != RESET)
	{
//		time6_count++;
//		if(time6_count >= 3)
//		{
//			time6_count =0;
//			printf("time6 test\n");
//		}	
		TIM_ClearITPendingBit(TIM6,TIM_IT_Update);//清空中断状态
	}
} 
static uint32_t sg_reboot_time = 0;

/************************************************************
*
* Function name	: 
* Description	: 
* Parameter		: 
* Return		: 
*	
************************************************************/
void set_reboot_time_function(uint32_t time)
{
	lfs_unmount(&g_lfs_t);
	sg_reboot_time = time;
}

/************************************************************
*
* Function name	: 
* Description	: 
* Parameter		: 
* Return		: 
*	
************************************************************/
static void device_reboot_timer_function(void)
{
	if(sg_reboot_time != 0) {
		sg_reboot_time--;
		if(sg_reboot_time==0) {
			System_SoftReset();
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: TIM3_IRQHandler
*	功能说明: TIM3 中断服务程序
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) //溢出中断
	{
		/* 全局运行时间每1ms增1 */
		g_iRunTime++;
		if (g_iRunTime == 0x7FFFFFFF)	/* 这个变量是 int32_t 类型，最大数为 0x7FFFFFFF */
		{
			g_iRunTime = 0;
		}
//		if(g_iRunTime == 1000)
//		{
//			g_iRunTime =0;
//			printf("time3 test\n");
//		}	
		lwip_ping_timer_function();
		app_com_time_function();
		app_sys_operate_timer_function();
		eth_ping_timer_function();
		led_flicker_control_timer_function();
		device_reboot_timer_function();
		bl0910_run_timer_function();
		com_queue_time_function();
		bl0942_run_timer_function();
	}
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //清除中断标志位

}

/*
*********************************************************************************************************
*	函 数 名: Timer_GetTick
*	功能说明: 获取运行时间
*	形    参: 无 
*	返 回 值: 运行时间
*********************************************************************************************************
*/
uint32_t Timer_GetTick(void)
{
  return g_iRunTime;
}


