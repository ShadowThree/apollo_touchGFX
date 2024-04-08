/**
 *	@author	shadowthreed@gmail.com
 *	@date		20240408
 *
 How to use:
 
 0. if MCU have MPU, need config it for NAND flash;
 
 1. config the micro NAND_FLASH_ADDR;
 
 2. call the Nand Flash test function:
 
			NAND_FLASH_STA_t nand_flash_sta = NAND_FLASH_OK;
			nand_flash_sta = Nand_Flash_test();
			if(nand_flash_sta != NAND_FLASH_OK) {
				LOG_ERR("Nand Flash test err[%d]\n", nand_flash_sta);
			} else {
				LOG_DBG("Nand Flash test OK\n");
			}
 */
#ifndef __NAND_FLASH_H__
#define __NAND_FLASH_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "fmc.h"

#define NAND_LOG_EN		1
#ifdef NAND_LOG_EN
	#include "dbger.h"
	#define NAND_DBG(fmt, ...)		LOG_DBG(fmt, ##__VA_ARGS__)
	#define NAND_ERR(fmt, ...)		LOG_ERR(fmt, ##__VA_ARGS__)
#else
	#define NAND_DBG(fmt, ...)
	#define NAND_ERR(fmt, ...)
#endif

typedef enum {
	NAND_FLASH_OK	= 0,
	NAND_FLASH_RESET_ERR,
	NAND_FLASH_READ_ID_ERR,
	NAND_FLASH_ERASE_ERR,
	NAND_FLASH_WRITE_ERR,
	NAND_FLASH_READ_ERR,
	NAND_FLASH_CHECK_DATA_ERR
} NAND_FLASH_STA_t;

#define NAND_FLASH_ADDR				(NAND_DEVICE)				// 0x80000000
#define NAND_FLASH_SIZE				(2048 * 64 * 4096)	// (2K+64)Bytes * 64Pages * 4096Blocks = 512MBytes
#define NAND_FLASH_PAGE_SIZE	(2048)							// pageSize = 2KBytes

NAND_FLASH_STA_t Nand_Flash_test(void);

#ifdef __cplusplus
}
#endif

#endif	/* __NAND_FLASH_H__ */
