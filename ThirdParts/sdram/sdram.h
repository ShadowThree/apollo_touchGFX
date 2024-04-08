/**
 *	@author	shadowthreed@gmail.com
 *	@date		20240408
 *
 ****** HOW TO USE *******
 
 0. if MCU's MPU is enabled, be sure it's the correct config for external SDRAM;
		Then, change the configs in this file.
 
 1. SDRAM init in fmc.c :
 
		SDRAM_Initialization_Sequence(&hsdram1);
		
 2. SDRAM test:
		
		// external SDRAM test
		if(EXT_SDRAM_TEST_OK != extSDRAM_test()) {
			LOG_ERR("SDRAM test err\n");
		} else {
			LOG_DBG("SDRAM test OK\n");
		}
		
 */
#ifndef __SDRAM_H__
#define __SDRAM_H__

#include "main.h"

#define EXT_SDRAM_LOG_EN	1
#if EXT_SDRAM_LOG_EN
	#include "dbger.h"
	#define SDRAM_ERR(fmt, ...)		LOG_ERR(fmt, ##__VA_ARGS__)
#else
	#define SDRAM_ERR(fmt, ...)
#endif

#define EXT_SDRAM_TEST_OK			0
#define EXT_SDRAM_TEST_ERR		1

#define SDRAM_BANK1_ADDR      ((uint32_t)0xC0000000)	// 8MBytes
#define SDRAM_BANK2_ADDR      ((uint32_t)0xC0800000)	// 8MBytes
#define SDRAM_BANK3_ADDR      ((uint32_t)0xC1000000)	// 8MBytes
#define SDRAM_BANK4_ADDR      ((uint32_t)0xC1800000)	// 8MBytes

#define SDRAM_WORD_NUM				(16 * 1024 * 1024)		// 16Mword, each word 16 bit
#define SDRAM_BYTE_NUM				(32 * 1024 * 1024)		// totally 0x02000000 bytes

// SDRAM_CLK = 130MHz
// 8K Refresh Cycles/64 mS  => 64ms/8K = 7.8us				(datasheet)
// 7.8us / (1 / 130MHz) = 1014
// so the refresh count must less than 1014 (usually less than 20)
#define REFRESH_COUNT					((uint32_t)1014 - 20)   /* SDRAM refresh counter */
#define SDRAM_TIMEOUT        	((uint32_t)0xFFFF)

typedef uint16_t SDRAM_WORD_t;

void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram);
uint8_t extSDRAM_test(void);

#endif	/* __SDRAM_H__ */
