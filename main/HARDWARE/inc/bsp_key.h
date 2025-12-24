/********************************************************************************
* @File name  : 按键模块
* @Description: 头文件
* @Author     : ZHLE
*  Version Date        Modification Description
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
********************************************************************************/

#ifndef _KEY_H_
#define _KEY_H_

#include "sys.h"


//引脚定义
/*******************************************************/

/*******************************************************/

/* 按键ID, 主要用于bsp_KeyState()函数的入口参数 */
//typedef enum
//{
//	RESET_K1 = 0,
//	DOOR_K2,
//	PWR_K3,
//	WATER_K4,
//	INT1_K5,
//	INT2_K6,
//	INT3_K7,
//	AC_LP_K8,
//	AC_NP_K9,
//	AC_LN_K10,
//	KEY_ALL
//}KEY_ID_E;

/* 函数声明 */
void bsp_InitKey(void);
void key_detection_function(void);

void open_door_detection(void); // 箱门检测
void pwr_tst_detection(void);		// 市电检测
void spd_status_detection(void);  // 20230721
void sim_status_detection(void);  // 20230721
void water_status_detection(void);

uint8_t key_get_state_function(uint8_t key_num);
void key_test(void);

#endif
