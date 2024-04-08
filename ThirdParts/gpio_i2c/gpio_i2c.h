/**
 *	@author	shadowthreed@gmail.com
 *	@date		20240408
 *
 ******* HOW TO USE ********
 
 1. config the SCL and SDA pin in cubeMx with pull-up output;
 2. modify/add your port and pin in gpio_i2c_pin;
 3. offer the interface GPIO_I2Cx_WRITE and GPIO_I2Cx_READ.
 
 */
#ifndef __GPIO_I2C_H__
#define __GPIO_I2C_H__

#include <stdint.h>
#include "main.h"

#define GPIO_I2C_NUM	1

#define GPIO_I2C1_WRITE(addr, data, len)	gpio_i2c_write(0, addr, data, len)
#define GPIO_I2C1_READ(addr, data, len)		gpio_i2c_read(0, addr, data, len)

typedef enum
{
  GPIO_I2C_STA_OK = 0,
  GPIO_I2C_STA_ERR,
  GPIO_I2C_STA_NACK,
	GPIO_I2C_STA_DELAY_NG
} GPIO_I2C_STA_t;

typedef struct {
	GPIO_TypeDef* scl_port;
	uint16_t	scl_pin;
	GPIO_TypeDef* sda_port;
	uint16_t sda_pin;
} GPIO_I2C_PIN_t;

GPIO_I2C_STA_t gpio_i2c_write(uint8_t idx, uint8_t addr, uint8_t *data, size_t len);
GPIO_I2C_STA_t gpio_i2c_read(uint8_t idx, uint8_t addr, uint8_t *data, size_t len);
//GPIO_I2C_STA_t gpio_i2c_delay_test(void);

#endif /* __GPIO_I2C_H__ */
