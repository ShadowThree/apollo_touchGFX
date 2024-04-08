/**
 *	@author	shadowthreed@gmail.com
 *	@date		20240408
 *
 *	How to use:

0. Enable CT_INT interrupt in cubeMx, or just disregard it if you do NOT want use GT911 in interrupt mode;
	 Config the macro GT911_LOG_EN, GT911_ADDR and NUM_TOUCH_SUPPORT as you want.
	 
1. init gt911 (must init it before "MX_TouchGFX_Init();", because it will stop the interrupt which cause the HAL_Delay() can't work.)

	#include "gt911.h"
	GT911_STA_t gt911_sta = GT911_STA_OK;
	HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);		// delete it if GT911 is NOT work in interrupt mode;
	gt911_sta = gt911_init();
	if(GT911_STA_OK != gt911_sta) {
		LOG_ERR("GT911 init err[%d]\n", gt911_sta);
	} else {
		LOG_DBG("GT911 init OK\n");
		HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);		// delete it if GT911 is NOT work in interrupt mode;
	}
	
2. get the touch coordinate in interrupt:

	if(GPIO_Pin == CT_INT_Pin) {
		gt911_get_touch((COORDINATE_t*)touch_coordinate, &num_touched);
		if(num_touched) {
			INT_DBG(" detect %d touch(s):\n", num_touched);
			for(uint8_t i = 0; i < num_touched; i++) {
				INT_DBG("\tP%d(%d, %d)\n", i, touch_coordinate[i].x, touch_coordinate[i].y);
			}
		}
	}
	
3. send the coordinate to touchFGX in STM32TouchController::sampleTouch():

		if(num_touched) {
			num_touched = 0;
			x = touch_coordinate[0].x;
			y = touch_coordinate[0].y;
			return true;
		}
    return false;
	
 */
 
#ifndef __GT911_H__
#define __GT911_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "main.h"

#define GT911_LOG_EN	1
#if GT911_LOG_EN
	#include "dbger.h"
	#define GT911_DBG(fmt, ...)		LOG_DBG(fmt, ##__VA_ARGS__)
	#define GT911_ERR(fmt, ...)		LOG_ERR(fmt, ##__VA_ARGS__)
#else
	#define GT911_DBG(fmt, ...)
	#define GT911_ERR(fmt, ...)
#endif

#define GT911_ADDR	(0x14 << 1)			// 0x28
//#define GT911_ADDR	(0x5D << 1)			// 0xBA
	
#define GT911_DELAY(ms)			HAL_Delay(ms)		// delay in ms

#define NUM_TOUCH_SUPPORT		(2)		// range [1, 5]

typedef enum {
	GT911_STA_OK = 0,
	GT911_STA_NOT_RESET,
	GT911_STA_GET_ID_WRITE_ERR,
	GT911_STA_GET_ID_READ_ERR,
	GT911_STA_GET_CONFIG_VERSION_WRITE_ERR,
	GT911_STA_GET_CONFIG_VERSION_READ_ERR,
	GT911_STA_GET_FW_VERSION_WRITE_ERR,
	GT911_STA_GET_FW_VERSION_READ_ERR,
	GT911_STA_GET_STATE_WRITE_ERR,
	GT911_STA_GET_STATE_READ_ERR,
	GT911_STA_SET_STATE_ERR,
	GT911_STA_GET_TOUCH_WRITE_ERR,
	GT911_STA_GET_TOUCH_READ_ERR,
	GT911_STA_GET_ALL_CONFIG_WRITE_ERR,
	GT911_STA_GET_ALL_CONFIG_READ_ERR,
	GT911_STA_GET_ALL_CONFIG_CHECK_ERR,
	GT911_STA_SET_ALL_CONFIG_ERR
} GT911_STA_t;

typedef struct {
	uint16_t x;
	uint16_t y;
} COORDINATE_t;

extern volatile COORDINATE_t touch_coordinate[NUM_TOUCH_SUPPORT];
extern volatile uint8_t num_touched;

GT911_STA_t gt911_init(void);
GT911_STA_t gt911_get_touch(volatile COORDINATE_t* touch_coordinate, volatile uint8_t* num_of_touch_detect);

#ifdef __cplusplus
}
#endif

#endif	/* __GT911_H__ */
