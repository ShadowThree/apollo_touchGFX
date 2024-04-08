#include "gpio_i2c.h"

#define GPIO_I2C_LOG_EN		1

#if GPIO_I2C_LOG_EN
	#include "dbger.h"
	#define GPIO_I2C_DBG(fmt, ...)	LOG_DBG(fmt, ##__VA_ARGS__)
	#define GPIO_I2C_ERR(fmt, ...)	LOG_ERR(fmt, ##__VA_ARGS__)
#else
	#define GPIO_I2C_DBG(fmt, ...)
	#define GPIO_I2C_ERR(fmt, ...)
#endif

#define GPIO_I2C_ACK 	0
#define GPIO_I2C_NACK 1

#define SCL_H(n)					HAL_GPIO_WritePin(gpio_i2c_pin[n].scl_port, gpio_i2c_pin[n].scl_pin, GPIO_PIN_SET)
#define SCL_L(n)					HAL_GPIO_WritePin(gpio_i2c_pin[n].scl_port, gpio_i2c_pin[n].scl_pin, GPIO_PIN_RESET)
#define SDA_H(n)					HAL_GPIO_WritePin(gpio_i2c_pin[n].sda_port, gpio_i2c_pin[n].sda_pin, GPIO_PIN_SET)
#define SDA_L(n)					HAL_GPIO_WritePin(gpio_i2c_pin[n].sda_port, gpio_i2c_pin[n].sda_pin, GPIO_PIN_RESET)
#define SDA_READ(n)				HAL_GPIO_ReadPin(gpio_i2c_pin[n].sda_port, gpio_i2c_pin[n].sda_pin)
#define SDA_WRITE(n, x)		HAL_GPIO_WritePin(gpio_i2c_pin[n].sda_port, gpio_i2c_pin[n].sda_pin, (x)?GPIO_PIN_SET:GPIO_PIN_RESET)


GPIO_I2C_PIN_t gpio_i2c_pin[GPIO_I2C_NUM] = {
	{.scl_port = GPIOH, .scl_pin = GPIO_PIN_6, .sda_port = GPIOI, .sda_pin = GPIO_PIN_3}
};

void gpio_i2c_delay(uint32_t us)
{
	us *= (SystemCoreClock / 1000000 / 5);
	for(uint32_t i = 0; i < us; i++) {
		__ASM("nop");
	}
}

static void gpio_i2c_set_sda_output(uint8_t idx)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = gpio_i2c_pin[idx].sda_pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(gpio_i2c_pin[idx].sda_port, &GPIO_InitStruct);
}

static void gpio_i2c_set_sda_input(uint8_t idx)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = gpio_i2c_pin[idx].sda_pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(gpio_i2c_pin[idx].sda_port, &GPIO_InitStruct);
}

// START: when CLK is high, SDA change from high to low
static void gpio_i2c_start(uint8_t idx)
{
	gpio_i2c_set_sda_output(idx);
  SCL_H(idx);
	SDA_H(idx);
	gpio_i2c_delay(5);
	SDA_L(idx);
	gpio_i2c_delay(1);	// Must less than 0.6us in standard IIC
	SCL_L(idx);
}

// STOP: when CLK is high, SDA change from low to high
static void gpio_i2c_stop(uint8_t idx)
{
	gpio_i2c_set_sda_output(idx);
	SDA_L(idx);
	SCL_H(idx);
	gpio_i2c_delay(5);
	SDA_H(idx);
	gpio_i2c_delay(5);
}

static uint8_t gpio_i2c_get_ack(uint8_t idx)
{
  uint8_t ack;
	SCL_L(idx);
	SDA_H(idx);
	gpio_i2c_set_sda_input(idx);
	gpio_i2c_delay(5);
	SCL_H(idx);
	gpio_i2c_delay(1);
	ack = SDA_READ(idx);
	gpio_i2c_delay(4);
	SCL_L(idx);
  return ack;
}

/**
 * @param	ack	0=ack, 1=nack
 */
static void gpio_i2c_set_ack(uint8_t idx, uint8_t ack)
{
  gpio_i2c_set_sda_output(idx);
	SCL_L(idx);
  if (ack)
  {
    SDA_H(idx);
  }
  else
  {
    SDA_L(idx);
  }
  gpio_i2c_delay(5);
  SCL_H(idx);
  gpio_i2c_delay(5);
  SCL_L(idx);
}

/**
 * @brief if ACK is low, return 0, if ACK is high, return 1
 *
 * @param data
 * @return uint8_t
 */
static void gpio_i2c_write_byte(uint8_t idx, uint8_t data)
{
	gpio_i2c_set_sda_output(idx);
	
  for (uint8_t i = 0; i < 8; i++)
  {
		SCL_L(idx);
		SDA_WRITE(idx, (data & 0x80) >> 7);
		data <<= 1;
		gpio_i2c_delay(5);
		SCL_H(idx);
		gpio_i2c_delay(5);
  }
  SCL_L(idx);
}

static uint8_t gpio_i2c_read_byte(uint8_t idx, uint8_t ack)
{
	uint8_t recv = 0;
	
	gpio_i2c_delay(5);
	gpio_i2c_set_sda_input(idx);
	for(uint8_t i = 0; i < 8; i++) {
		SCL_L(idx);
		gpio_i2c_delay(5);
		SCL_H(idx);
		recv <<= 1;
		gpio_i2c_delay(3);
		if(SDA_READ(idx)) {
			recv++;
		}
		gpio_i2c_delay(2);
	}
	SCL_L(idx);
	gpio_i2c_set_ack(idx, ack);
	return recv;
}

GPIO_I2C_STA_t gpio_i2c_write(uint8_t idx, uint8_t addr, uint8_t *data, size_t len)
{
  gpio_i2c_start(idx);
  gpio_i2c_write_byte(idx, addr);
  if (GPIO_I2C_ACK != gpio_i2c_get_ack(idx))
  {
    gpio_i2c_stop(idx);
    return GPIO_I2C_STA_NACK;
  }
  for (size_t i = 0; i < len; i++)
  {
    gpio_i2c_write_byte(idx, data[i]);
    if (GPIO_I2C_ACK != gpio_i2c_get_ack(idx)) {
			gpio_i2c_stop(idx);
			return GPIO_I2C_STA_NACK;
		}
  }
  gpio_i2c_stop(idx);
  return GPIO_I2C_STA_OK;
}

GPIO_I2C_STA_t gpio_i2c_read(uint8_t idx, uint8_t addr, uint8_t *data, size_t len)
{

  gpio_i2c_start(idx);
  gpio_i2c_write_byte(idx, addr | 0x01);
  if (GPIO_I2C_ACK == gpio_i2c_get_ack(idx))
  {
    for(uint8_t i = 0; i < len; i++) {
			data[i] = gpio_i2c_read_byte(idx, (i!=(len-1))?GPIO_I2C_ACK:GPIO_I2C_NACK);
		}
		gpio_i2c_stop(idx);
		return GPIO_I2C_STA_OK;
  } else {
		gpio_i2c_stop(idx);
		return GPIO_I2C_STA_NACK;
	}
}

GPIO_I2C_STA_t gpio_i2c_delay_test(void)
{
	#define NUM_TEST	10
	uint32_t tick[NUM_TEST];
	
	for(uint8_t i = 0; i < NUM_TEST; i++) {
		tick[i] = HAL_GetTick();
		gpio_i2c_delay(1000000);
		tick[i] = HAL_GetTick() - tick[i];
		GPIO_I2C_DBG("GPIO I2C delay test%02d: %d\n", i+1, tick[i]);
		if(tick[i] != 1000) {
			GPIO_I2C_ERR("GPIO I2C delay test failed\n");
			return GPIO_I2C_STA_DELAY_NG;
		}
	}
	return GPIO_I2C_STA_OK;
}
