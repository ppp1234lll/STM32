
#include "appconfig.h"

ChipID_t g_chipid_t;

/*
*********************************************************************************************************
*	函 数 名:  start_system_init_function
*	功能说明:  初始化所有的硬件设备
*	形    参:  无
*	返 回 值:  无
*********************************************************************************************************
*/
void start_system_init_function(void)
{
	cJSON_Hooks hook;                // 初始化JSON 

	IWDG_Init(4,1000);               // 初始化看门狗(硬件、软件2s)
	start_get_device_id_function();  // 获取本机ID
	mymem_init(SRAMIN);              // 内存初始化
	
	hook.malloc_fn = mymalloc_sramin;// 内存分配
	hook.free_fn   = myfree_sramin;  // 内存释放
	cJSON_InitHooks(&hook);          // 初始化自定义的内存分配和释放函数
	
	bsp_InitLed();                   // LED初始化（已测试）
	bsp_InitRelay();				         // 继电器初始化（未测试）	
	bsp_InitKey();
	ebtn_APP_Keys_Init();            // 初始化按键模块（已测试）
	bsp_InitFan();                   // 风扇初始化(已测试)	
	bsp_InitRTC();								   // RTC初始化 (已测试)
	bsp_InitRS485(115200); 	
	bsp_InitUart_GPS(9600);

	IWDG_Feed();
	TIM3_Int_Init(1000-1,84-1);       // 定时器3初始化 1KHz(已测试)
	TIM6_Int_Init(10000-1,8400-1);    // 定时器6初始化 1Hz(已测试)
	TIM4_Int_Init(10000-1,8400-1);    // 定时器4初始化 1Hz(已测试)	

	hal_lis3dh_init(true);           // 陀螺仪初始化 IIC (已测试)
	aht20_init_function();					 // 温湿度初始化(已测试)	
//	BH1750_Init();                   // 光照度初始化(已测试)	
	bl0910_init_function();						// 电能检测初始化(已测试)
	bl0939_init_function();           // 电能检测初始化(失败)
	bl0910_test();
	
	IWDG_Feed();
	W25QXX_Init();			 					 // 初始化spiflash

	save_init_function();	
	com_recevie_function_init();			// 初始化接收缓冲区
	app_get_storage_param_function();	// 获取本地存储的数据
	update_status_init();							// 更新检测

	IWDG_Feed();
}
/************************************************************
*
* Function name	: start_get_device_id_function
* Description	: 获取本机ID
* Parameter		: 
* Return		: 
*	STM2F1_UUID_ADDR  0X1FFFF7E8   // 任意的一个数
	STM2F3_UUID_ADDR  0X1FFFF7AC   // 任意的一个数
	STM2F4_UUID_ADDR  0X1FFF7A10   // 任意的一个数
	STM2F7_UUID_ADDR  0X1FF0F420   // 任意的一个数
************************************************************/
void start_get_device_id_function(void)
{
	volatile uint32_t addr;
	addr  = 0x1FFF822E;
	addr -= 0x800;
	addr -= 0x1e;	
	
	g_chipid_t.id[0] = *(__I uint32_t *)(addr + 0x00);
	g_chipid_t.id[1] = *(__I uint32_t *)(addr + 0x04);
	g_chipid_t.id[2] = *(__I uint32_t *)(addr + 0x08);
}

/************************************************************
*
* Function name	: start_get_device_id_str
* Description	: 获取本机ID
* Parameter		: 
* Return		: 
*	
************************************************************/
void start_get_device_id_str(uint8_t *str)
{
	sprintf((char*)str,"%04X%04X%04X",g_chipid_t.id[0],g_chipid_t.id[1],g_chipid_t.id[2]);
}
void start_get_device_id(uint32_t *id)
{
	id[0] = g_chipid_t.id[0];
	id[1] = g_chipid_t.id[1];
	id[2] = g_chipid_t.id[2];
}

/* APP线程 */
#define APP_TASK_PRIO		6
#define APP_STK_SIZE		512
__align(8) CCMRAM static OS_STK START_TASK_STK[APP_STK_SIZE];
void app_task(void *argument);

/* 网络线程 */
#define ETH_TASK_PRIO		9
#define ETH_STK_SIZE		320
__align(8) CCMRAM static OS_STK ETH_TASK_STK[ETH_STK_SIZE];
void eth_task(void *argument);

/* 检测线程 */
#define DET_TASK_PRIO		7
#define DET_STK_SIZE		320
__align(8) CCMRAM static OS_STK DET_TASK_STK[DET_STK_SIZE];
void det_task(void *argument);

/* 无线线程 */
#define GSM_TASK_PRIO		8
#define GSM_STK_SIZE		320
CCMRAM OS_STK GSM_TASK_STK[GSM_STK_SIZE];
void gsm_task(void *argument);

/* 打印线程 */
#define PRINT_TASK_PRIO		29   /* 任务优先级 */
#define PRINT_STK_SIZE		256  /* 任务堆栈大小 */
CCMRAM OS_STK 	PRINT_TASK_STK[PRINT_STK_SIZE]; /* 任务堆栈 */
void print_task(void *argument); /* 任务函数 */

/* 查看任务堆栈 */
#define STORAGESTACK_PRIO		30
#define STORAGESTACK_STK_SIZE 		128
OS_STK STORAGESTACK_STK[STORAGESTACK_STK_SIZE];
void storagestack_task(void *p_arg);

/************************************************************
*
* Function name	: start_creat_task_function
* Description	: 创建任务
* Parameter		: 
* Return		: 
*	
************************************************************/
void start_creat_task_function(void)
{
	OS_CPU_SR cpu_sr;
	INT8U  err;			
	OS_ENTER_CRITICAL();   // 关中断
	
	/* 创建任务 */
	OSTaskCreateExt(	 app_task, 																					//建立扩展任务(任务代码指针) 
										(void *)0,																					//传递参数指针 
										(OS_STK*)&START_TASK_STK[APP_STK_SIZE-1], 					//分配任务堆栈栈顶指针 
										(INT8U)APP_TASK_PRIO, 															//分配任务优先级 
										(INT16U)APP_TASK_PRIO,															//(未来的)优先级标识(与优先级相同) 
										(OS_STK *)&START_TASK_STK[0], 											//分配任务堆栈栈底指针 
										(INT32U)APP_STK_SIZE, 															//指定堆栈的容量(检验用) 
										(void *)0,																					//指向用户附加的数据域的指针 
										(INT16U)OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);		//建立任务设定选项 
	OSTaskNameSet(APP_TASK_PRIO, (INT8U *)(void *)"app", &err);
										
	OSTaskCreateExt(	 eth_task, 																					//建立扩展任务(任务代码指针) 
										(void *)0,																					//传递参数指针 
										(OS_STK*)&ETH_TASK_STK[ETH_STK_SIZE-1], 					  //分配任务堆栈栈顶指针 
										(INT8U)ETH_TASK_PRIO, 															//分配任务优先级 
										(INT16U)ETH_TASK_PRIO,															//(未来的)优先级标识(与优先级相同) 
										(OS_STK *)&ETH_TASK_STK[0], 											  //分配任务堆栈栈底指针 
										(INT32U)ETH_STK_SIZE, 															//指定堆栈的容量(检验用) 
										(void *)0,																					//指向用户附加的数据域的指针 
										(INT16U)OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);		//建立任务设定选项 
	OSTaskNameSet(ETH_TASK_PRIO, (INT8U *)(void *)"eth", &err);
										
	OSTaskCreateExt(	 det_task, 																					//建立扩展任务(任务代码指针) 
										(void *)0,																					//传递参数指针 
										(OS_STK*)&DET_TASK_STK[DET_STK_SIZE-1], 					//分配任务堆栈栈顶指针 
										(INT8U)DET_TASK_PRIO, 															//分配任务优先级 
										(INT16U)DET_TASK_PRIO,															//(未来的)优先级标识(与优先级相同) 
										(OS_STK *)&DET_TASK_STK[0], 											//分配任务堆栈栈底指针 
										(INT32U)DET_STK_SIZE, 															//指定堆栈的容量(检验用) 
										(void *)0,																					//指向用户附加的数据域的指针 
										(INT16U)OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);		//建立任务设定选项 
	OSTaskNameSet(DET_TASK_PRIO, (INT8U *)(void *)"det", &err);

	OSTaskCreateExt(	 gsm_task, 																					//建立扩展任务(任务代码指针) 
										(void *)0,																					//传递参数指针 
										(OS_STK*)&GSM_TASK_STK[GSM_STK_SIZE-1], 					//分配任务堆栈栈顶指针 
										(INT8U)GSM_TASK_PRIO, 															//分配任务优先级 
										(INT16U)GSM_TASK_PRIO,															//(未来的)优先级标识(与优先级相同) 
										(OS_STK *)&GSM_TASK_STK[0], 											//分配任务堆栈栈底指针 
										(INT32U)GSM_STK_SIZE, 															//指定堆栈的容量(检验用) 
										(void *)0,																					//指向用户附加的数据域的指针 
										(INT16U)OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);		//建立任务设定选项 
	OSTaskNameSet(GSM_TASK_PRIO, (INT8U *)(void *)"gsm", &err);

	OSTaskCreateExt(	 print_task,                                        //建立扩展任务(任务代码指针) 
										(void *)0,																					//传递参数指针 
										(OS_STK*)&PRINT_TASK_STK[PRINT_STK_SIZE-1], 					//分配任务堆栈栈顶指针 
										(INT8U)PRINT_TASK_PRIO, 															//分配任务优先级 
										(INT16U)PRINT_TASK_PRIO,															//(未来的)优先级标识(与优先级相同) 
										(OS_STK *)&PRINT_TASK_STK[0], 											//分配任务堆栈栈底指针 
										(INT32U)PRINT_STK_SIZE, 															//指定堆栈的容量(检验用) 
										(void *)0,																					//指向用户附加的数据域的指针 
										(INT16U)OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);		//建立任务设定选项 
	OSTaskNameSet(PRINT_TASK_PRIO, (INT8U *)(void *)"print", &err);
										
//	OSTaskCreateExt(	 storagestack_task, 															 //建立扩展任务(任务代码指针) 
//										(void *)0,																					//传递参数指针 
//										(OS_STK*)&STORAGESTACK_STK[STORAGESTACK_STK_SIZE-1], //分配任务堆栈栈顶指针 
//										(INT8U)STORAGESTACK_PRIO, 															//分配任务优先级 
//										(INT16U)STORAGESTACK_PRIO,															//(未来的)优先级标识(与优先级相同) 
//										(OS_STK *)&STORAGESTACK_STK[0], 											//分配任务堆栈栈底指针 
//										(INT32U)STORAGESTACK_STK_SIZE, 												 //指定堆栈的容量(检验用) 
//										(void *)0,																					//指向用户附加的数据域的指针 
//										(INT16U)OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);		//建立任务设定选项 
//	OSTaskNameSet(STORAGESTACK_PRIO, (INT8U *)(void *)"storagestack", &err);
	
	OS_EXIT_CRITICAL();  		 															// 开中断
}

/*
*********************************************************************************************************
*	函 数 名: app_task
*	功能说明: 主任务
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void app_task(void *argument)
{
	OSStatInit();	  	// 初始化统计任务
	app_task_function();
}

/*
*********************************************************************************************************
*	函 数 名: eth_task
*	功能说明: 网口检测任务:一直对网口进行轮询
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void eth_task(void *argument)
{
	eth_network_line_status_detection_function();
}

/*
*********************************************************************************************************
*	函 数 名: det_task
*	功能说明: 检测任务
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void det_task(void *argument)
{
	det_task_function();
}

/*
*********************************************************************************************************
*	函 数 名: gsm_task
*	功能说明: 无线通信任务
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void gsm_task(void *argument)
{
	gsm_task_function();
}

/*
*********************************************************************************************************
*	函 数 名: print_task
*	功能说明: 打印任务
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void print_task(void *argument)
{
	print_task_function();
}	

/*
*********************************************************************************************************
*	函 数 名: storagestack_task
*	功能说明: 任务堆栈
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void storagestack_task(void *p_arg)
{
	OS_TCB *ptcb;       //定义一个任务控制块，结构体指针
	OS_STK_DATA stkDat;   //定义堆栈结构体变量 
	while(1)
	{
		ptcb = &OSTCBTbl[0];//将指针指向任务表的第一个任务
		printf("************************************ App Task Debug Info ***********************************\r\n");
		printf("  Prio    Used     Free    Per     TaskName\r\n");
		while (ptcb != NULL)//轮询每一个任务
		{
				OSTaskStkChk(ptcb->OSTCBPrio, &stkDat);//Check task stack
				printf("   %2d    %5d    %5d    %02d%%     %s\r\n", ptcb->OSTCBPrio, stkDat.OSUsed, stkDat.OSFree, (stkDat.OSUsed * 100)/(stkDat.OSUsed + stkDat.OSFree), ptcb->OSTCBTaskName);        
				ptcb = ptcb->OSTCBPrev;//Previous TCB list
		}
		printf("\r\n");
		OSTimeDlyHMSM(0,0,10,0); //延时3s
	}
}

