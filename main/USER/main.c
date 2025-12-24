#include "appconfig.h"

#define HARDWARE_VERSION               "V1.0.0"
#define SOFTWARE_VERSION               "V0.1.0"

static void system_setup(void);

/*
*********************************************************************************************************
*	函 数 名: main
*	功能说明: 主函数
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
int main(void)
{
	system_setup();                  // 初始化时钟
	bsp_InitUart(115200);            // 调试串口初始化
	start_system_init_function();      // 初始化硬件

	OSInit();                     // UCOS初始化
	start_creat_task_function();  // 初始化任务
	OSStart();                    // 开启UCOS

}

/*
*********************************************************************************************************
*	函 数 名: system_setup
*	功能说明: 初始化
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void system_setup(void)
{
	SystemInit();
	
	delay_init(168);                                // 初始化延时函数
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	// 中断分组配置

	/* Initialize cryptographic library */
	if (cmox_initialize(NULL) != CMOX_INIT_SUCCESS)
		printf("error\n"); 

	/* CmBacktrace initialize */
	cm_backtrace_init("CmBacktrace", HARDWARE_VERSION, SOFTWARE_VERSION);
}

