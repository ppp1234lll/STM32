/********************************************************************************
* @File name  : RTC实时时钟驱动文件
* @Description: 基于STM32的RTC驱动，支持时间/日期设置、读取、闹钟配置及唤醒定时器功能
* @Author     : ZHLE
*  Version Date        Modification Description
********************************************************************************/
#include "bsp_rtc.h"
#include "delay.h"
#include "time.h"
#include "bsp.h"
#include "app.h"
#include "gsm.h"

// 时钟源宏定义
//#define RTC_CLOCK_SOURCE_LSE      
#define RTC_CLOCK_SOURCE_LSI

// 异步分频因子
#define ASYNCHPREDIV         0X7F
// 同步分频因子
#define SYNCHPREDIV          0XFF
// 闹钟相关宏定义
#define ALARM_HOURS               1   // 0~23
#define ALARM_MINUTES             00  // 0~59
#define ALARM_SECONDS             00  // 0~59

#define ALARM_MASK                RTC_AlarmMask_DateWeekDay
#define ALARM_DATE_WEEKDAY_SEL    RTC_AlarmDateWeekDaySel_WeekDay
#define ALARM_DATE_WEEKDAY        2
#define RTC_Alarm_X               RTC_Alarm_A

// 备份域寄存器宏定义
#define RTC_BKP_DRX          RTC_BKP_DR0
// 写入到备份寄存器的数据宏定义
#define RTC_BKP_DATA         0x3232

// 声明内部函数：通过年月日计算星期
static uint8_t get_week_form_time(uint16_t year,uint8_t month,uint8_t day);

/*
*********************************************************************************************************
*	函 数 名: bsp_InitRTC
*	功能说明: RTC初始化。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
#define LSE_STARTUP_TIMEOUT     ((uint16_t)0x05000)  // LSE启动超时计数

uint8_t bsp_InitRTC(void)
{
	RTC_InitTypeDef RTC_InitStructure;
  __IO uint16_t StartUpCounter = 0;
	FlagStatus LSEStatus = RESET;	
	
	/*使能 PWR 时钟*/
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
  /* PWR_CR:DBF置1，使能RTC、RTC备份寄存器和备份SRAM的访问 */
  PWR_BackupAccessCmd(ENABLE);

#if defined (RTC_CLOCK_SOURCE_LSI) 
  /* 使用LSI作为RTC时钟源会有误差 
	 * 默认选择LSE作为RTC的时钟源
	 */
  /* 使能LSI */ 
  RCC_LSICmd(ENABLE);
  /* 等待LSI稳定 */  
  do
  {
    LSEStatus = RCC_GetFlagStatus(RCC_FLAG_LSIRDY);  // 检查LSE就绪标志
    StartUpCounter++;
  }while((LSEStatus == RESET) && (StartUpCounter != LSE_STARTUP_TIMEOUT));
  /* 选择LSI做为RTC的时钟源 */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);

#elif defined (RTC_CLOCK_SOURCE_LSE)

  /* 使能LSE */ 
  RCC_LSEConfig(RCC_LSE_ON);
   /* 等待LSE稳定 */   
  do
  {
    LSEStatus = RCC_GetFlagStatus(RCC_FLAG_LSERDY);  // 检查LSE就绪标志
    StartUpCounter++;
  }while((LSEStatus == RESET) && (StartUpCounter != LSE_STARTUP_TIMEOUT));

  /* 选择LSE做为RTC的时钟源 */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);    

#endif /* RTC_CLOCK_SOURCE_LSI */

  /* 使能RTC时钟 */
  RCC_RTCCLKCmd(ENABLE);

  /* 等待 RTC APB 寄存器同步 */
  RTC_WaitForSynchro();
   
/*=====================初始化同步/异步预分频器的值======================*/
	/* 驱动日历的时钟ck_spare = LSE/[(255+1)*(127+1)] = 1HZ */
	
	/* 设置异步预分频器的值 */
	RTC_InitStructure.RTC_AsynchPrediv = ASYNCHPREDIV;
	/* 设置同步预分频器的值 */
	RTC_InitStructure.RTC_SynchPrediv = SYNCHPREDIV;	
	RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24; 
	/* 用RTC_InitStructure的内容初始化RTC寄存器 */
	if (RTC_Init(&RTC_InitStructure) == ERROR)
	{
//		printf("\n\r RTC 时钟初始化失败 \r\n");
	}	
	
  if (RTC_ReadBackupRegister(RTC_BKP_DRX) != RTC_BKP_DATA)
  {
		// 初始化默认时间（11:48:00 上午）
		RTC_Set_Time(11,48,0,RTC_H12_AM);
		// 初始化默认日期（2024年5月16日，星期4）
		RTC_Set_Date(24,5,16,4);
  }
	RTC_AlarmSet();  // 每天12:00:00触发
	return 0;
}

/*
*********************************************************************************************************
*	函 数 名: RTC_Set_Time
*	功能说明: 设置RTC时间（支持12小时制）
*	形    参: 
*	@hour		: 小时（12小时制：1-12；24小时制：0-23，需配合HourFormat）
*	@min		: 分钟（0-59）
*	@sec		: 秒钟（0-59）
*	@ampm		: 12小时制上下午标识 @ref RTC_AM_PM_Definitions：RTC_H12_AM（上午）/RTC_H12_PM（下午）
*	返 回 值: ErrorStatus枚举值
*				      SUCCESS(1)：设置成功
*				      ERROR(0)：设置失败（如RTC未进入初始化模式）
*********************************************************************************************************
*/
ErrorStatus RTC_Set_Time(u8 hour,u8 min,u8 sec,u8 ampm)
{
	RTC_TimeTypeDef RTC_TimeStructure;
	
	// 初始化时间
	RTC_TimeStructure.RTC_H12 = ampm;
	RTC_TimeStructure.RTC_Hours = hour;        
	RTC_TimeStructure.RTC_Minutes = min;      
	RTC_TimeStructure.RTC_Seconds = sec;      
	RTC_SetTime(RTC_Format_BIN, &RTC_TimeStructure);
	RTC_WriteBackupRegister(RTC_BKP_DRX, RTC_BKP_DATA);

	return SUCCESS;
}

/*
*********************************************************************************************************
*	函 数 名: TimeBySecond
*	功能说明: 根据Unix时间戳（秒数）设置RTC时间（含时区转换：UTC+8）
*	形    参: 
*	@second		: Unix时间戳（从1970-01-01 00:00:00 UTC开始的秒数）
*	返 回 值: 无
*********************************************************************************************************
*/
void TimeBySecond(u32 second)
{
	struct tm *pt,t;
	rtc_time_t time_t;
	second += 8*60*60;  // 时区转换：UTC+8，将传入的UTC时间戳转为本地时间
	pt = localtime(&second);
	
	if(pt == NULL)
		return;
	t=*pt;
	t.tm_year+=1900;  // tm_year是从1900年开始的偏移，转为4位完整年份
	t.tm_mon++;        // tm_mon是0-11，转为1-12的月份
	time_t.year  = t.tm_year;  // 4位年份（如2024）
	time_t.month = t.tm_mon;   // 月份（1-12）
	time_t.data  = t.tm_mday;  // 日期（1-31，变量名data实际为day）
	time_t.hour  = t.tm_hour;  // 小时（24小时制，0-23）
	time_t.min   = t.tm_min;   // 分钟（0-59）
	time_t.sec   = t.tm_sec;   // 秒钟（0-59）
	/* 调用统一时间设置接口 */
	RTC_set_Time(time_t);
}

/*
*********************************************************************************************************
*	函 数 名: RTC_Set_Date
*	功能说明: 设置RTC日期
*	形    参: 
*	@year		: 年份（两位数，如24代表2024年）
*	@month		: 月份（1-12）
*	@date		: 日期（1-31）
*	@week		: 星期（1-7，1=周一，7=周日，具体需配合RTC硬件定义）
*	返 回 值: ErrorStatus枚举值
*				     SUCCESS(1)：设置成功
*				     ERROR(0)：设置失败（如RTC未进入初始化模式）
*********************************************************************************************************
*/
ErrorStatus RTC_Set_Date(u8 year,u8 month,u8 date,u8 week)
{
	RTC_DateTypeDef RTC_DateStructure;
	
  // 初始化日期	
	RTC_DateStructure.RTC_WeekDay = week;       
	RTC_DateStructure.RTC_Date = date;         
	RTC_DateStructure.RTC_Month = month;         
	RTC_DateStructure.RTC_Year = year;        
	RTC_SetDate(RTC_Format_BIN, &RTC_DateStructure);
	RTC_WriteBackupRegister(RTC_BKP_DRX, RTC_BKP_DATA);

	return SUCCESS;
}

/* 月份天数对照表（用于计算星期） */										 
const uint8_t cg_table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5};
/*
*********************************************************************************************************
*	函 数 名: get_week_form_time
*	功能说明: 通过年月日计算对应的星期几
*	形    参: 
*	@year		: 4位完整年份（如2024）
*	@month		: 月份（1-12）
*	@day		: 日期（1-31）
*	返 回 值: 星期标识（0=周日，1=周一，...，6=周六，需根据实际需求调整）
*********************************************************************************************************
*/
static uint8_t get_week_form_time(uint16_t year,uint8_t month,uint8_t day)
{	
	uint16_t temp2;
	uint8_t yearH,yearL;
	
	yearH=year/100;	yearL=year%100; 
	// 若为21世纪（yearH>19），年份低位加100（适配计算公式）
	if (yearH>19)yearL+=100;
	// 计算核心逻辑（仅支持1900年以后）  
	temp2=yearL+yearL/4;
	temp2=temp2%7; 
	temp2=temp2+day+cg_table_week[month-1];
	// 若为闰年且月份小于3，需减1（修正闰年2月的影响）
	if (yearL%4==0&&month<3)temp2--;
	
	return(temp2%7);
}	

/*
*********************************************************************************************************
*	函 数 名: RTC_set_Time
*	功能说明: 统一设置RTC时间和日期（自动计算星期）
*	形    参: 
*	@rtc		: rtc_time_t类型结构体，包含时间日期信息
*				  rtc.year：4位完整年份（如2024）
*				  rtc.month：月份（1-12）
*				  rtc.data：日期（1-31，变量名data实际为day）
*				  rtc.hour：小时（24小时制，0-23）
*				  rtc.min：分钟（0-59）
*				  rtc.sec：秒钟（0-59）
*	返 回 值: 无
*********************************************************************************************************
*/
void RTC_set_Time(rtc_time_t rtc)
{
	// 自动计算星期并赋值
	rtc.week = get_week_form_time(rtc.year,rtc.month,rtc.data);
	
	// 使能PWR时钟（RTC属于备份域，需先开启PWR时钟）
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, DISABLE); 
	delay_ms(10);
	
	// 根据小时判断12小时制的上下午（小时>12为下午，否则为上午）
	if(rtc.hour > 12)
		RTC_Set_Time(rtc.hour,rtc.min,rtc.sec,RTC_H12_PM);
	else
		RTC_Set_Time(rtc.hour,rtc.min,rtc.sec,RTC_H12_AM);

	// 设置日期（年份需转为两位数：如2024→24）
	RTC_Set_Date(rtc.year-2000,rtc.month,rtc.data,rtc.week);
	// 重复设置一次日期（确保硬件稳定写入）
	RTC_Set_Date(rtc.year-2000,rtc.month,rtc.data,rtc.week);
	
	delay_ms(10);
	
	// 恢复PWR时钟使能
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE); 	
}

/*
*********************************************************************************************************
*	函 数 名: RTC_Get_Time
*	功能说明: 读取当前RTC时间和日期
*	形    参: 
*	@rtc		: 指向rtc_time_t类型的指针，用于存储读取到的时间日期
*				  rtc.year：4位完整年份（如2024，由硬件两位数年份+2000得到）
*				  rtc.month：月份（1-12）
*				  rtc.data：日期（1-31，变量名data实际为day）
*				  rtc.hour：小时（24小时制，0-23）
*				  rtc.min：分钟（0-59）
*				  rtc.sec：秒钟（0-59）
*				  rtc.week：星期（1-7，由硬件直接读取）
*	返 回 值: 无
*********************************************************************************************************
*/
void RTC_Get_Time(rtc_time_t *rtc)
{
	RTC_DateTypeDef RTC_DateTypeInitStructure;
	RTC_TimeTypeDef RTC_TimeTypeInitStructure;

	// 读取当前时间（小时、分钟、秒钟）
	RTC_GetTime(RTC_Format_BIN,&RTC_TimeTypeInitStructure);
	rtc->hour = RTC_TimeTypeInitStructure.RTC_Hours;
	rtc->min  = RTC_TimeTypeInitStructure.RTC_Minutes;
	rtc->sec  = RTC_TimeTypeInitStructure.RTC_Seconds;

	// 读取当前日期（年份、月份、日期、星期）
	RTC_GetDate(RTC_Format_BIN,&RTC_DateTypeInitStructure);
	rtc->year  = RTC_DateTypeInitStructure.RTC_Year+2000;  // 转为4位完整年份
	rtc->month = RTC_DateTypeInitStructure.RTC_Month;
	rtc->data  = RTC_DateTypeInitStructure.RTC_Date;
	rtc->week  = RTC_DateTypeInitStructure.RTC_WeekDay;
}

/*
*********************************************************************************************************
*	函 数 名: time_to_second_function
*	功能说明: 时间（年月日时分秒）转换为Unix时间戳（秒数）- 适配NB通信
*	形    参: 
*	@time		: 时间数组，格式为[年,月,日,时,分,秒]（年为4位完整年份，如2024）
*	@second		: 指向uint32_t的指针，用于存储转换后的Unix时间戳
*	返 回 值: 无
*********************************************************************************************************
*/
void time_to_second_function(uint32_t *time, uint32_t *second)
{
	struct tm pt;
	
	pt.tm_year = time[0]+100;  // tm_year是从1900年开始的偏移（如2024→124=2024-1900）
	pt.tm_mon  = time[1] - 1;  // tm_mon是0-11，需将1-12的月份减1
	pt.tm_mday = time[2];      // 日期（1-31）
	pt.tm_hour = time[3];      // 小时（24小时制，0-23）
	pt.tm_min  = time[4];      // 分钟（0-59）
	pt.tm_sec  = time[5];      // 秒钟（0-59）
	
	*second = mktime(&pt);  // 转换为Unix时间戳
}

/*
*********************************************************************************************************
*	函 数 名: local_to_utc_time
*	功能说明: 本地时间转换为UTC时间（支持时区偏移计算，处理跨月/跨年）
*	形    参:  
*	@utc_time	: 指向rtc_time_t的指针，用于存储转换后的UTC时间
*	@timezone	: 时区偏移（本地时间=UTC时间+timezone，如UTC+8时区传入-8，UTC-5传入5）
*	@local_time	: 输入的本地时间（rtc_time_t结构体，含年/月/日/时/分/秒/星期）
*	返 回 值: 无
*********************************************************************************************************
*/
void local_to_utc_time(rtc_time_t *utc_time, int8_t timezone, rtc_time_t local_time)
{
	int year,month,day,hour,week;
	int lastday = 0;			// 当前月份的总天数
	int lastlastday = 0;		// 上一个月份的总天数

	// 初始化本地时间参数
	year	= local_time.year;	// 4位完整年份
	month = local_time.month;	// 月份（1-12）
	day 	= local_time.data;	// 日期（1-31，变量名data实际为day）
	hour 	= local_time.hour + timezone;  // 计算UTC小时（本地小时+时区偏移）
	week  = local_time.week;	// 星期（1-7）
	
	// 计算当前月份和上一个月份的总天数（处理闰年2月）
	// 大月（1,3,5,7,8,10,12）：31天；小月（4,6,9,11）：30天；2月：平年28天，闰年29天
	if(month==1 || month==3 || month==5 || month==7 || month==8 || month==10 || month==12)
	{
		lastday = 31;  // 当前月份为大月，31天
		lastlastday = 30;  // 上一个月份默认30天（需特殊处理3月和1月）
		
		if(month == 3)  // 当前月份是3月，上一个月份是2月（需判断闰年）
		{
			if((year%400 == 0)||(year%4 == 0 && year%100 != 0))// 闰年判断
				lastlastday = 29;
			else
				lastlastday = 28;
		}
		
		if(month == 8 || month == 1)  // 当前月份是8月（上一个月7月，31天）或1月（上一个月12月，31天）
			lastlastday = 31;
	}
	else if(month == 4 || month == 6 || month == 9 || month == 11)  // 当前月份为小月，30天
	{
		lastday = 30;
		lastlastday = 31;  // 上一个月份为大月，31天
	}
	else  // 当前月份是2月（需判断闰年）
	{
		lastlastday = 31;  // 上一个月份是1月，31天
		if((year%400 == 0)||(year%4 == 0 && year%100 != 0))// 闰年
			lastday = 29;
		else
			lastday = 28;
	}

	// 处理小时>=24的情况（跨天，日期+1，星期+1）
	if(hour >= 24)
	{					
		hour -= 24;
		day += 1;
		week += 1;		// 星期加1
		if(week > 7)
			week = 1;  // 星期超过7则重置为1（1=周一）  
		// 处理日期超过当前月份总天数（跨月，月份+1）
		if(day > lastday)
		{ 		
			day -= lastday;
			month += 1;
			// 处理月份超过12（跨年，年份+1）
			if(month > 12)
			{
				month -= 12;
				year += 1;
			}
		}
	}
	
	// 处理小时<0的情况（跨天，日期-1，星期-1）
	if(hour < 0)
	{
		hour += 24;
		day -= 1;
		week -= 1;
		if(week < 1)
			week = 7;  // 星期小于1则重置为7（7=周日）
		// 处理日期<1（跨月，月份-1）
		if(day < 1)
		{
			day = lastlastday;  // 日期设为上一个月份的总天数
			month -= 1;
			// 处理月份<1（跨年，年份-1）
			if(month < 1)
			{
				month = 12;
				year -= 1;
			}
		}
	}

	// 赋值转换后的UTC时间
	utc_time->year  = year;
	utc_time->month = month;
	utc_time->data  = day;
	utc_time->week 	= week;
	utc_time->hour  = hour;
	utc_time->min	 	= local_time.min;  // 分钟不变
	utc_time->sec	 	= local_time.sec;  // 秒钟不变
}

/*
*********************************************************************************************************
*	函 数 名: RTC_AlarmSet
*	功能说明: 使能 RTC 闹钟中断
*	形    参: 无
*	返 回 值: 无
*    要使能 RTC 闹钟中断，需按照以下顺序操作：
* 1. 将 EXTI 线 17 配置为中断模式并将其使能，然后选择上升沿有效。
* 2. 配置 NVIC 中的 RTC_Alarm IRQ 通道并将其使能。
* 3. 配置 RTC 以生成 RTC 闹钟（闹钟 A 或闹钟 B）。
*********************************************************************************************************
*/
void RTC_AlarmSet(void)
{
    NVIC_InitTypeDef  NVIC_InitStructure;
    EXTI_InitTypeDef  EXTI_InitStructure;
    RTC_AlarmTypeDef  RTC_AlarmStructure;

    /*=============================第①步=============================*/
    /* RTC 闹钟中断配置 */
    /* EXTI 配置 */
    EXTI_ClearITPendingBit(EXTI_Line17);
    EXTI_InitStructure.EXTI_Line = EXTI_Line17;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /*=============================第②步=============================*/
    /* 使能RTC闹钟中断 */
    NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /*=============================第③步=============================*/
    /* 失能闹钟 ，在设置闹钟时间的时候必须先失能闹钟*/
    RTC_AlarmCmd(RTC_Alarm_X, DISABLE);
    /* 设置闹钟时间 */
    RTC_AlarmStructure.RTC_AlarmTime.RTC_H12     = RTC_H12_AM;
    RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours   = ALARM_HOURS;
    RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = ALARM_MINUTES;
    RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = ALARM_SECONDS;
    RTC_AlarmStructure.RTC_AlarmMask = ALARM_MASK;
    RTC_AlarmStructure.RTC_AlarmDateWeekDaySel = ALARM_DATE_WEEKDAY_SEL;
    RTC_AlarmStructure.RTC_AlarmDateWeekDay = ALARM_DATE_WEEKDAY;

    /* 配置RTC Alarm X（X=A或B） 寄存器 */
    RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_X, &RTC_AlarmStructure);

    /* 使能 RTC Alarm X 中断 */
    RTC_ITConfig(RTC_IT_ALRA, ENABLE);

    /* 使能闹钟 */
    RTC_AlarmCmd(RTC_Alarm_X, ENABLE);

    /* 清除闹钟中断标志位 */
    RTC_ClearFlag(RTC_FLAG_ALRAF);
    /* 清除 EXTI Line 17 悬起位 (内部连接到RTC Alarm) */
    EXTI_ClearITPendingBit(EXTI_Line17);
}
// 闹钟中断服务函数
void RTC_Alarm_IRQHandler(void)
{
	if (RTC_GetITStatus(RTC_IT_ALRA) != RESET) {
	  printf("rtc alarm start success\n");
		app_system_softreset();
		RTC_ClearITPendingBit(RTC_IT_ALRA);
		EXTI_ClearITPendingBit(EXTI_Line17);
	}
}

/*
*********************************************************************************************************
*	函 数 名: RTC_Get_Time_Test
*	功能说明: RTC时间读取测试函数（循环打印当前时间）
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void RTC_Get_Time_Test(void)
{
	static rtc_time_t rtc_test;
	while(1)
	{
		RTC_Get_Time(&rtc_test);
		// 打印格式：日期-月份-年份，星期，时间（时:分:秒）
		printf("data:%d-%d-%d,week:%d,time:%d:%d:%d",rtc_test.year,rtc_test.month,rtc_test.data,
																		rtc_test.week,rtc_test.hour,rtc_test.min,rtc_test.sec);
		delay_ms(1000);  // 每秒刷新一次
	}
}







