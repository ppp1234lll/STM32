/**
  ******************************************************************************
  * @file    bsp_i2c.c 
  * @author  Xiao Yang 260384793@qq.com
  * @version V2.0.0
  * @date    2023-05-25
  * @brief   
  ******************************************************************************
  */

/* Include -------------------------------------------------------------------*/
#include "bsp_sf_i2c.h"
#include "sf_i2c.h"

/* Private function ----------------------------------------------------------*/
static void i2c0_sda_pin_low(void);
static void i2c0_sda_pin_high(void);
static void i2c0_scl_pin_low(void);
static void i2c0_scl_pin_high(void);
static uint8_t i2c0_sda_pin_read(void);
static void i2c0_sda_pin_dir_input(void);
static void i2c0_sda_pin_dir_output(void);


/**
 * @brief  i2c software delay function, used to control the i2c bus speed
 * @param  dev : Pointer to iic structure
 * @return none
 */
static void i2c_delay(const uint32_t us)
{
    __IO uint32_t i = us;

    while(i--);
}



/* Private variables ---------------------------------------------------------*/
// 定义i2c驱动对象
static struct sf_i2c_dev i2c0_dev = {
    .name               = "i2c0",
    .speed              = 500, /*! speed:105Hz */
    .delay_us           = i2c_delay,
    .ops.sda_low        = i2c0_sda_pin_low,
    .ops.sda_high       = i2c0_sda_pin_high,
    .ops.scl_low        = i2c0_scl_pin_low,
    .ops.scl_high       = i2c0_scl_pin_high,
    .ops.sda_read_level = i2c0_sda_pin_read,
    .ops.sda_set_input  = i2c0_sda_pin_dir_input,
    .ops.sda_set_output = i2c0_sda_pin_dir_output,
};

/*! Set i2c sda pin low level */
static void i2c0_sda_pin_low(void)
{
   HAL_GPIO_WritePin(I2C0_SDA_PORT, I2C0_SDA_PIN, GPIO_PIN_RESET);
}

/*! Set i2c sda pin high level */
static void i2c0_sda_pin_high(void)
{
    HAL_GPIO_WritePin(I2C0_SDA_PORT, I2C0_SDA_PIN, GPIO_PIN_SET);
}

/*! Set i2c scl pin low level */
static void i2c0_scl_pin_low(void)
{
   HAL_GPIO_WritePin(I2C0_SCL_PORT, I2C0_SCL_PIN, GPIO_PIN_RESET);
}

/*! Set i2c scl pin high level */
static void i2c0_scl_pin_high(void)
{
    HAL_GPIO_WritePin(I2C0_SCL_PORT, I2C0_SCL_PIN, GPIO_PIN_SET);
}

/*! Read i2c sda pin level */
static uint8_t i2c0_sda_pin_read(void)
{
    return HAL_GPIO_ReadPin(I2C0_SDA_PORT, I2C0_SDA_PIN);
}

/*! Switch i2c sda pin dir input */
static void i2c0_sda_pin_dir_input(void)
{

}

/*! Switch i2c sda pin dir output */
static void i2c0_sda_pin_dir_output(void)
{

}

/**
 * @brief  Initialization i2c0 physical interface
 * @param  none
 * @return none
 */
static void i2c0_port_init(void)
{
    // Config pin output direction
    GPIO_InitTypeDef GPIO_InitStructer;

    /* 时钟使能 */
    I2C0_SCL_CLK_ENABLE();
    I2C0_SDA_CLK_ENABLE();
    
    /* 配置SCL引脚 */
    GPIO_InitStructer.Pin = I2C0_SCL_PIN;
    GPIO_InitStructer.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructer.Pull = GPIO_PULLUP;
    GPIO_InitStructer.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(I2C0_SCL_PORT, &GPIO_InitStructer);
    
    /* 配置SDA引脚 */
    GPIO_InitStructer.Pin = I2C0_SDA_PIN;
    GPIO_InitStructer.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStructer.Pull = GPIO_PULLUP;
    GPIO_InitStructer.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(I2C0_SDA_PORT, &GPIO_InitStructer);
}

/**
 * @brief  Initialization board i2c0 interface
 * @param  none
 * @return none
 */
void bsp_i2c_init(void)
{
    /*! i2c physical layer initialization */
    i2c0_port_init();
    /*! i2c software layer initialization */
    sf_i2c_init(&i2c0_dev);
}

