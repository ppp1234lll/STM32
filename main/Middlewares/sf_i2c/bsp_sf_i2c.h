/**
  ******************************************************************************
  * @file    bsp_i2c.h 
  * @author  Xiao Yang 260384793@qq.com
  * @version V1.0.0
  * @date    2021-10-06
  * @brief   
  ******************************************************************************
  */

#ifndef __BSP_SF_I2C_H
#define __BSP_SF_I2C_H

/* Include -------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/*******************************************************************************
 * i2c0 physical interface
 ******************************************************************************/
#define I2C0_SCL_PORT           GPIOB
#define I2C0_SCL_PIN            GPIO_PIN_8
#define I2C0_SCL_CLK_ENABLE()   __HAL_RCC_GPIOB_CLK_ENABLE();
#define I2C0_SDA_PORT           GPIOB
#define I2C0_SDA_PIN            GPIO_PIN_9
#define I2C0_SDA_CLK_ENABLE()   __HAL_RCC_GPIOB_CLK_ENABLE();
 
#define I2C0_NAME           (char*)"I2C0"

/* Exported functions --------------------------------------------------------*/
void bsp_i2c_init(void);

#endif
