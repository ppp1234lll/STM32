/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : 基于OTA方式的固件升级
*
*            注意事项：
*              1. 调试串口设置，波特率115200，数据位8，奇偶校验位无，停止位1。
*              2. 务必将编辑器的缩进参数和TAB设置为4来阅读本文件，要不代码显示不整齐。
*
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2025-11-13    FN          1. CMSIS软包版本 V5.5.0
*                                     2. 标准库
*
*	Copyright (C), 2018-2030,  蜂鸟物联网
*
*********************************************************************************************************
*/	

#include "main.h"  

// 定义
#define PWR_TST_READ   	GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_0)    // 12V检测


/* 本.c文件调用的函数 */
static void DeviceRstReason(void);
static void read_boot_update_param(struct BOOT_UPDATE_PARAM *boot_update_param);
static void write_boot_update_param(struct BOOT_UPDATE_PARAM *boot_update_param);
static void update_check_function(void);

/*
*********************************************************************************************************
*	函 数 名: main
*	功能说明: c程序入口
*	形    参: 无
*	返 回 值: 错误代码(无需处理)
*********************************************************************************************************
*/
int main(void)
{
	system_setup_function(); 						// 初始化时钟
	usart_debug_init_function(115200); 	// 初始化调试串口
	led_gpio_init_function();  					// 初始化指示灯
	DeviceRstReason();                  // 判断程序复位原因
	
	printf("\n/bootloader/main() 111 ...\n");
	W25QXX_Init();  										// 初始化外部存储

	printf("\n/bootloader/main() 222 ...\n");
	IWDG_Init(4,1000);       						// 初始化看门狗

	printf("\n/bootloader/main() 333 ...\n");
	update_check_function(); // 更新检测

	printf("\n/bootloader/main() 444 ...\n");
}

/*
*********************************************************************************************************
*	函 数 名: system_setup_function
*	功能说明: 系统启动初始化
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void system_setup_function(void)
{
	SystemInit();

	delay_init(168);  //初始化延时函数
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
}

/*
*********************************************************************************************************
*	函 数 名: update_check_function
*	功能说明: 更新检测
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void update_check_function(void)
{
	struct BOOT_UPDATE_PARAM boot_update_param = {0};
	unsigned int ii;
	unsigned int read_addr = 0, write_addr = 0;
	unsigned char *app_buff = NULL;
	uint8_t count = 30;
	////

	while(count--)
	{
    IWDG_Feed();
		delay_ms(100);
		if(PWR_TST_READ == 0) 
			break; 
	}
  count = 200;
  while(count--)
	{
     delay_ms(10);
     IWDG_Feed();
  }
	
	mymem_init(SRAMIN);	// 内存初始化

	// 读取升级参数
	read_boot_update_param(&boot_update_param);

	// 判断升级标志,直接跳转
	if( boot_update_param.is_update != 1 )
	{
		printf("\n无需升级，直接执行 main 模块 ...\n");
		IWDG_Feed();
		iap_load_app(MAIN_APP_ADDR); // 执行FLASH APP代码
		return;
	}

	// 执行更新
	printf("\n执行升级程序 ..... \n");
	boot_update_param.is_update = false; // 关闭标志
	write_boot_update_param(&boot_update_param); // 保存升级参数

	app_buff = (unsigned char *)mymalloc(SRAMIN, (boot_update_param.section_size + 64));

	printf("\n执行升级参数,section_count: %u, section_size: %u\n", boot_update_param.section_count, boot_update_param.section_size);
	
	// 写入BIN文件
	for(ii = 0; ii < boot_update_param.section_count; ii++)
	{
		IWDG_Feed();
		led_show_control(ii % 256); // led灯效果

		// 读一块
		read_addr = UPDATA_SPIFLASH_ADDR + (ii * boot_update_param.section_size);
		W25QXX_Read(app_buff, read_addr, boot_update_param.section_size);

		// 写入一块
		write_addr = MAIN_APP_ADDR + (ii * boot_update_param.section_size);
		iap_write_appbin(write_addr, app_buff, boot_update_param.section_size);
	}  
	
	myfree(SRAMIN, (void *)app_buff);

	IWDG_Feed();

	printf("\n升级完毕！跳转 ...\n");
	iap_load_app(MAIN_APP_ADDR); // 执行FLASH APP代码
}

/*
*********************************************************************************************************
*	函 数 名: DeviceRstReason
*	功能说明: 判断硬件重启原因
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DeviceRstReason(void)
{
	if( SET == RCC_GetFlagStatus( RCC_FLAG_PORRST) )
	{
		printf("POR/PDR重上电启动\n");
	}
	if( SET == RCC_GetFlagStatus( RCC_FLAG_BORRST) )
	{
		printf("POR/PDR/BOR重上电启动\n");
	}
	if( SET == RCC_GetFlagStatus(RCC_FLAG_SFTRST) )
	{
		printf("软复位启动\n");
	}
	if( SET == RCC_GetFlagStatus(RCC_FLAG_IWDGRST) )
	{
		printf("独立看门狗启动\n");
	}
	if( SET == RCC_GetFlagStatus(RCC_FLAG_WWDGRST) )
	{
		printf("窗口看门狗启动\n");
	}
	if( (RESET == RCC_GetFlagStatus(RCC_FLAG_SFTRST))  &&
		  (RESET == RCC_GetFlagStatus(RCC_FLAG_IWDGRST)) &&
	    (RESET == RCC_GetFlagStatus(RCC_FLAG_WWDGRST)) &&
		  (SET   == RCC_GetFlagStatus(RCC_FLAG_PINRST))
   	)
	{
		printf("硬复位启动\n");
	}
	RCC_ClearFlag() ;
}

/*
*********************************************************************************************************
*	函 数 名: led_gpio_init_function
*	功能说明: LED初始化
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void led_gpio_init_function(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOE,ENABLE); //使能GPIOB|GPIOE的时钟

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_8|GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode	=	GPIO_Mode_OUT;			 
	GPIO_InitStructure.GPIO_OType	=	GPIO_OType_PP;  		 
	GPIO_InitStructure.GPIO_PuPd	=	GPIO_PuPd_NOPULL;  	 
	GPIO_InitStructure.GPIO_Speed	=	GPIO_Speed_100MHz; 	 
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_8|GPIO_Pin_9);

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode	=	GPIO_Mode_OUT;			// 输出
	GPIO_InitStructure.GPIO_OType	=	GPIO_OType_PP;  		// 推挽输出
	GPIO_InitStructure.GPIO_PuPd	=	GPIO_PuPd_NOPULL;  	// 上拉
	GPIO_InitStructure.GPIO_Speed	=	GPIO_Speed_100MHz; 	// 高速GPIO
	GPIO_Init(GPIOE,&GPIO_InitStructure);
	GPIO_SetBits(GPIOE,GPIO_Pin_0);
	
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode	=	GPIO_Mode_IN;			// 输出
	GPIO_InitStructure.GPIO_PuPd	=	GPIO_PuPd_NOPULL;  	// 上拉
	GPIO_InitStructure.GPIO_Speed	=	GPIO_Medium_Speed; 	// 高速GPIO
	GPIO_Init(GPIOD,&GPIO_InitStructure);
}

/*
*********************************************************************************************************
*	函 数 名: led_show_control
*	功能说明: LED灯光控制，显示更新进度
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void led_show_control(uint8_t mode)
{
	static uint8_t flag = 0;
	uint8_t num = mode % 100;
	if(num<50 && flag == 0) {
		flag = 1;
		GPIO_ResetBits(GPIOB,GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9);
		GPIO_ResetBits(GPIOE,GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6);
	} else if(num >=50 && flag == 1){
		flag = 0;
		GPIO_SetBits(GPIOB,GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9);
		GPIO_SetBits(GPIOE,GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6);
	}
}


/*
*********************************************************************************************************
*	函 数 名: read_boot_update_param
*	功能说明: 读取更新信息
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void read_boot_update_param(struct BOOT_UPDATE_PARAM *boot_update_param)
{
	W25QXX_Read((uint8_t*)boot_update_param, UPDATA_PARAM_ADDR, sizeof(struct BOOT_UPDATE_PARAM));
}

/*
*********************************************************************************************************
*	函 数 名: write_boot_update_param
*	功能说明: 保存升级参数
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void write_boot_update_param(struct BOOT_UPDATE_PARAM *boot_update_param)
{
	W25QXX_Write((uint8_t *)boot_update_param, UPDATA_PARAM_ADDR, sizeof(struct BOOT_UPDATE_PARAM));
}

/****************************************** END OF FILE **********************************************/
