#include "gt911.h"
#include "gpio_i2c.h"

#define GT911_REG_CONFIG_VERSION		(0x8047)
#define GT911_REG_ID								(0x8140)
#define GT911_REG_FIRMWARE_VERSION	(0x8144)
#define GT911_REG_STATE_TOUCH_NUM		(0x814E)
#define GT911_REG_POINT1_X					(0x8150)		// NOT 0x8158

typedef struct {
	uint16_t X_resolution;
	uint16_t Y_resolution;
	uint8_t num_of_touch_support;
	uint8_t reverseX;
	uint8_t reverseY;
	uint8_t swithX2Y;
	uint8_t software_noise_reduction;
} GT911_CONFIG_t;

volatile COORDINATE_t touch_coordinate[NUM_TOUCH_SUPPORT] = {0};
volatile uint8_t num_touched = 0;
static uint8_t gt911_is_reset = 0;

static GT911_CONFIG_t gt911_config = {
	.X_resolution = 1024,
	.Y_resolution = 600,
	.num_of_touch_support = NUM_TOUCH_SUPPORT,
	.reverseX = 0,
	.reverseY = 0,
	.swithX2Y = 1,
	.software_noise_reduction = 1
};

static void gt911_reset(uint8_t addr);
static GT911_STA_t gt911_get_id(void);
static GT911_STA_t gt911_get_firmware_version(void);
static GT911_STA_t gt911_get_config_version(void);
static GT911_STA_t gt911_get_state(uint8_t* state);
static GT911_STA_t gt911_set_state(uint8_t state);
static GT911_STA_t gt911_set_all_config(uint8_t* buf);
static GT911_STA_t gt911_get_all_config(uint8_t* buf);
static void gt911_set_int_exit(void);

GT911_STA_t gt911_init(void)
{
	uint8_t rxBuf[200];
	uint8_t txBuf[200];
	GT911_STA_t sta = GT911_STA_OK;
	gt911_reset(GT911_ADDR);
	sta |= gt911_get_id();
	sta |= gt911_get_firmware_version();
	sta |= gt911_get_config_version();
	sta |= gt911_get_all_config(rxBuf);
	
	rxBuf[1] = gt911_config.X_resolution & 0x00FF;
	rxBuf[2] = (gt911_config.X_resolution >> 8) & 0x00FF;
	rxBuf[3] = gt911_config.Y_resolution & 0x00FF;
	rxBuf[4] = (gt911_config.Y_resolution >> 8) & 0x00FF;
	rxBuf[5] = gt911_config.num_of_touch_support;
	rxBuf[6] = 0;
	rxBuf[6] |= gt911_config.reverseY << 7;
	rxBuf[6] |= gt911_config.reverseX << 6;
	rxBuf[6] |= gt911_config.swithX2Y << 3;
	rxBuf[6] |= gt911_config.software_noise_reduction << 2;
	memcpy(&txBuf[2], rxBuf, 184);
	sta |= gt911_set_all_config(txBuf);
	gt911_set_int_exit();
	return sta;
}

GT911_STA_t gt911_get_touch(volatile COORDINATE_t* touch_coordinate, volatile uint8_t* num_of_touch_detect)
{
	if(gt911_is_reset == 0) {
		GT911_ERR("GT911 is NOT reset before using!\n");
		return GT911_STA_NOT_RESET;
	}
	
	uint8_t txBuf[2], rxBuf[4];
	uint8_t gt911_touch_state;
	GPIO_I2C_STA_t gpio_i2c_sta = GPIO_I2C_STA_OK;
	GT911_STA_t gt911_sta = GT911_STA_OK;
	
	gt911_sta = gt911_get_state(&gt911_touch_state);
	if(gt911_sta != GT911_STA_OK) {
		GT911_ERR("get GT911 state err[%d]\n", gt911_sta);
		return gt911_sta;
	}
	
	if((gt911_touch_state & 0x80) != 0) {
		gt911_touch_state &= 0x0F;
		if(gt911_touch_state != 0) {
			*num_of_touch_detect = gt911_touch_state;
			for(uint8_t i = 0; i < *num_of_touch_detect; i++) {
				txBuf[0] = ((GT911_REG_POINT1_X + (i*8)) & 0xFF00) >> 8;
				txBuf[1] = (GT911_REG_POINT1_X + (i*8)) & 0xFF;
				gpio_i2c_sta = GPIO_I2C1_WRITE(GT911_ADDR, txBuf, 2);
				if(gpio_i2c_sta != GPIO_I2C_STA_OK) {
					GT911_ERR("get touchP%d write err[%d]\n", i, gpio_i2c_sta);
					return GT911_STA_GET_TOUCH_WRITE_ERR;
				}
				gpio_i2c_sta = GPIO_I2C1_READ(GT911_ADDR, rxBuf, 4);
				if(gpio_i2c_sta != GPIO_I2C_STA_OK) {
					GT911_ERR("get touchP%d read err[%d]\n", i, gpio_i2c_sta);
					return GT911_STA_GET_TOUCH_READ_ERR;
				}
				touch_coordinate[i].x = ((uint16_t)(rxBuf[1]) << 8) | rxBuf[0];
				touch_coordinate[i].y = ((uint16_t)(rxBuf[3]) << 8) | rxBuf[2];
			}
		}
	}
	return gt911_set_state(0);
}

static GT911_STA_t gt911_set_state(uint8_t state)
{
	uint8_t txBuf[3];
	GPIO_I2C_STA_t gpio_i2c_sta = GPIO_I2C_STA_OK;
	
	txBuf[0] = (GT911_REG_STATE_TOUCH_NUM & 0xFF00) >> 8;
	txBuf[1] = GT911_REG_STATE_TOUCH_NUM & 0xFF;
	txBuf[2] = state;
	gpio_i2c_sta = GPIO_I2C1_WRITE(GT911_ADDR, txBuf, 3);
	if(gpio_i2c_sta != GPIO_I2C_STA_OK) {
		GT911_ERR("GT911 set state err[%d]\n", gpio_i2c_sta);
		return GT911_STA_SET_STATE_ERR;
	}
	//GT911_DBG("GT911 set state[0x%02x]\n", state);
	return GT911_STA_OK;
}

static GT911_STA_t gt911_get_state(uint8_t* state)
{
	uint8_t txBuf[2];
	GPIO_I2C_STA_t gpio_i2c_sta = GPIO_I2C_STA_OK;
	
	txBuf[0] = (GT911_REG_STATE_TOUCH_NUM & 0xFF00) >> 8;
	txBuf[1] = GT911_REG_STATE_TOUCH_NUM & 0xFF;
	gpio_i2c_sta = GPIO_I2C1_WRITE(GT911_ADDR, txBuf, 2);
	if(gpio_i2c_sta != GPIO_I2C_STA_OK) {
		GT911_ERR("GT911 get state write err[%d]\n", gpio_i2c_sta);
		return GT911_STA_GET_STATE_WRITE_ERR;
	}

	gpio_i2c_sta = GPIO_I2C1_READ(GT911_ADDR, state, 1);
	if(gpio_i2c_sta != GPIO_I2C_STA_OK) {
		GT911_ERR("GT911 get state read err[%d]\n", gpio_i2c_sta);
		return GT911_STA_GET_STATE_READ_ERR;
	}
	//GT911_DBG("get GT911 state[0x%02x]\n", state[0]);
	return GT911_STA_OK;
}

static GT911_STA_t gt911_set_all_config(uint8_t* buf)
{
	GPIO_I2C_STA_t gpio_i2c_sta;
	
	// calculate the checksum
	buf[186] = 0;
	for(uint8_t i = 2; i < 186; i++) {
		buf[186] += buf[i];
	}
	buf[186] = ~buf[186] + 1;		// 0x80FF: config chksum register
	buf[187] = 1;								// 0x8100: config fresh register
	
	#if 0		// prite the config will be writed
	GT911_DBG("\nGT911 config will be writed:\n");
	for(uint8_t i = 2; i < 188; i++) {
		if((i != 2) && (i % 10 == 2)) {
			GT911_DBG("\n");
			HAL_Delay(20);
		}
		GT911_DBG(" %02x", buf[i]);
	}
	GT911_DBG("\n\n");
	#endif
	
	buf[0] = (GT911_REG_CONFIG_VERSION & 0xFF00) >> 8;
	buf[1] = GT911_REG_CONFIG_VERSION & 0xFF;
	gpio_i2c_sta = GPIO_I2C1_WRITE(GT911_ADDR, buf, 188);
	if(GPIO_I2C_STA_OK != gpio_i2c_sta) {
		GT911_ERR("set GT911 all configs err[%d]\n", gpio_i2c_sta);
		return GT911_STA_SET_ALL_CONFIG_ERR;
	}
	//GT911_DBG("set GT911 all configs OK\n");
	return GT911_STA_OK;
}

static GT911_STA_t gt911_get_all_config(uint8_t* buf)
{
	uint8_t txBuf[2];
	GPIO_I2C_STA_t gpio_i2c_sta;
	
	txBuf[0] = (GT911_REG_CONFIG_VERSION >> 8) & 0xFF;
	txBuf[1] = GT911_REG_CONFIG_VERSION & 0xFF;
	gpio_i2c_sta = GPIO_I2C1_WRITE(GT911_ADDR, txBuf, 2);
	if(gpio_i2c_sta != GPIO_I2C_STA_OK) {
		GT911_ERR("GT911 get all config write err[%d]\n", gpio_i2c_sta);
		return GT911_STA_GET_ALL_CONFIG_WRITE_ERR;
	}
	
	gpio_i2c_sta = GPIO_I2C1_READ(GT911_ADDR, buf, 185);
	if(gpio_i2c_sta != GPIO_I2C_STA_OK) {
		GT911_ERR("GT911 get all config read err[%d]\n", gpio_i2c_sta);
		return GT911_STA_GET_ALL_CONFIG_READ_ERR;
	}
	
	// checksum 
	buf[185] = 0;
	for(uint8_t i = 0; i < 184; i++) {
		buf[185] += buf[i];
	}
	buf[185] = ~buf[185] + 1;
	if(buf[184] != buf[185]) {
		GT911_ERR("GT911 get all config checksum err. read[0x%02x] cal[0x%02x]\n", buf[184], buf[185]);
		return GT911_STA_GET_ALL_CONFIG_CHECK_ERR;
	}
	
	#if	0	// print all config params
	GT911_DBG("\nGT911 config we get:\n");
	for(uint8_t i = 0; i < 185; i++) {
		if((i != 0) && (i % 10 == 0)) {
			GT911_DBG("\n");
			HAL_Delay(20);
		}
		GT911_DBG(" %02x", buf[i]);
	}
	GT911_DBG("\nchecksum[0x%02x]\n\n", buf[184]);
	#endif
	
	
		
	return GT911_STA_OK;
}

static void gt911_set_int_output(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = CT_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CT_INT_GPIO_Port, &GPIO_InitStruct);
}

static void gt911_set_int_exit(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = CT_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(CT_INT_GPIO_Port, &GPIO_InitStruct);
}

static void gt911_reset(uint8_t addr)
{
	GT911_DBG("set GT911 addr[0x%02x]\n", addr);
	gt911_set_int_output();
	HAL_GPIO_WritePin(CT_RST_GPIO_Port, CT_RST_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(CT_INT_GPIO_Port, CT_INT_Pin, GPIO_PIN_RESET);
	GT911_DELAY(10);
	if(addr == 0x28) {
		HAL_GPIO_WritePin(CT_INT_GPIO_Port, CT_INT_Pin, GPIO_PIN_SET);
	}
	GT911_DELAY(1);
	HAL_GPIO_WritePin(CT_RST_GPIO_Port, CT_RST_Pin, GPIO_PIN_SET);
	GT911_DELAY(5);
	HAL_GPIO_WritePin(CT_INT_GPIO_Port, CT_INT_Pin, GPIO_PIN_RESET);
	GT911_DELAY(50);
	
	gt911_is_reset = 1;
}

static GT911_STA_t gt911_get_firmware_version(void)
{
	uint8_t txBuf[2];
	uint8_t rxBuf[2];
	GPIO_I2C_STA_t gpio_i2c_sta;
	
	txBuf[0] = (GT911_REG_FIRMWARE_VERSION >> 8) & 0xFF;
	txBuf[1] = GT911_REG_FIRMWARE_VERSION & 0xFF;
	gpio_i2c_sta = GPIO_I2C1_WRITE(GT911_ADDR, txBuf, 2);
	if(gpio_i2c_sta != GPIO_I2C_STA_OK) {
		GT911_DBG("get GT911 firmware version write err[%d]\n", gpio_i2c_sta);
		return GT911_STA_GET_FW_VERSION_WRITE_ERR;
	}
	
	gpio_i2c_sta = GPIO_I2C1_READ(GT911_ADDR, rxBuf, 2);
	if(gpio_i2c_sta != GPIO_I2C_STA_OK) {
		GT911_DBG("get GT911 firmware version read err[%d]\n", gpio_i2c_sta);
		return GT911_STA_GET_FW_VERSION_READ_ERR;
	}
	GT911_DBG("get GT911 firmwareVersion[0x%02x%02x]\n", rxBuf[1], rxBuf[0]);
	return GT911_STA_OK;
}

static GT911_STA_t gt911_get_config_version(void)
{
	uint8_t txBuf[2];
	uint8_t rxBuf[1];
	GPIO_I2C_STA_t gpio_i2c_sta;
	
	txBuf[0] = (GT911_REG_CONFIG_VERSION >> 8) & 0xFF;
	txBuf[1] = GT911_REG_CONFIG_VERSION & 0xFF;
	gpio_i2c_sta = GPIO_I2C1_WRITE(GT911_ADDR, txBuf, 2);
	if(gpio_i2c_sta != GPIO_I2C_STA_OK) {
		GT911_DBG("get GT911 config version write err[%d]\n", gpio_i2c_sta);
		return GT911_STA_GET_CONFIG_VERSION_WRITE_ERR;
	}
	
	gpio_i2c_sta = GPIO_I2C1_READ(GT911_ADDR, rxBuf, 1);
	if(gpio_i2c_sta != GPIO_I2C_STA_OK) {
		GT911_DBG("get GT911 config version read err[%d]\n", gpio_i2c_sta);
		return GT911_STA_GET_CONFIG_VERSION_READ_ERR;
	}
	GT911_DBG("get GT911 configVersion[0x%02x]\n", rxBuf[0]);
	return GT911_STA_OK;
}

static GT911_STA_t gt911_get_id(void)
{
	uint8_t txBuf[2];
	uint8_t rxBuf[4];
	GPIO_I2C_STA_t gpio_i2c_sta;
	
	txBuf[0] = (GT911_REG_ID >> 8) & 0xFF;
	txBuf[1] = GT911_REG_ID & 0xFF;
	gpio_i2c_sta = GPIO_I2C1_WRITE(GT911_ADDR, txBuf, 2);
	if(gpio_i2c_sta != GPIO_I2C_STA_OK) {
		GT911_DBG("read GT911 ID write err[%d]\n", gpio_i2c_sta);
		return GT911_STA_GET_ID_WRITE_ERR;
	}
	gpio_i2c_sta = GPIO_I2C1_READ(GT911_ADDR, rxBuf, 4);
	if(gpio_i2c_sta != GPIO_I2C_STA_OK) {
		GT911_DBG("read GT911 ID read err[%d]\n", gpio_i2c_sta);
		return GT911_STA_GET_ID_READ_ERR;
	}
	GT911_DBG("get GT911 ID[0x%02x%02x%02x%02x]\n", rxBuf[0], rxBuf[1], rxBuf[2], rxBuf[3]);	// 0x39313100
	return GT911_STA_OK;
}
