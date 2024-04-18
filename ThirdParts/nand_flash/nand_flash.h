/**
 ***** HOW TO USE *****
 
  NAND_FLASH_STA_t nand_flash_sta = NAND_FLASH_STA_OK;
	LOG_DBG("***** Nand Flash test *********\n");
  nand_flash_sta = NAND_FLASH_test();
  if(nand_flash_sta != NAND_FLASH_STA_OK) {
      LOG_ERR("Nand Flash test err[%d]\n", nand_flash_sta);
  } else {
      LOG_DBG("Nand Flash test ok\n");
  }
  
	LOG_DBG("***** Nand AG interface test *********\n");
  nand_flash_sta = NAND_FLASH_algorithm_interface_test();
  if(nand_flash_sta != NAND_FLASH_STA_OK) {
	  LOG_ERR("Nand Flash algorithm test err[%d]\n", nand_flash_sta);
  } else {
      LOG_DBG("Nand Flash algorithm test ok\n");
  }
  
	LOG_DBG("***** Nand AG result test *********\n");
  nand_flash_sta = NAND_FLASH_algorithm_result_test();
  if(nand_flash_sta != NAND_FLASH_STA_OK) {
      LOG_ERR("Nand Flash algorithm result test err[%d]\n", nand_flash_sta);
  } else {
			LOG_DBG("Nand Flash algorithm result test OK\n");
	}
	LOG_DBG("***** TEST OVER *********\n");
 
 */
#ifndef __NAND_FLASH_H__
#define __NAND_FLASH_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include "fmc.h"

// just set to 1 when you need to create the FLM file. refer to LAT1198
#define USE_FOR_CREATE_FLM  0

#define NAND_FLASH_LOG_EN   1
#if NAND_FLASH_LOG_EN
    #include "dbger.h"
    #define NAND_FLASH_DBG(fmt, ...)        LOG_DBG(fmt, ##__VA_ARGS__)
    #define NAND_FLASH_ERR(fmt, ...)        LOG_ERR(fmt, ##__VA_ARGS__)
#else
    #define NAND_FLASH_DBG(fmt, ...)
    #define NAND_FLASH_ERR(fmt, ...)
#endif

// H27U1G8F2BTR params
#define PAGE_SIZE       (2048)              // 2KB
#define BLOCK_SIZE      (2048 * 64)         // 2KB * 64pages
#define BLOCK_NUM       (4096)              // totally 1024 blocks
#define PLANE_SIZE      (2048 * 64 * 4096)  // 512MB
#define PLANE_NUM    	(1)

// ARM FLY STM32-V6
#define NAND_FLASH_ADDR (0x80000000)

typedef enum {
    NAND_FLASH_STA_OK = 0,
    NAND_FLASH_RESET_ERR,
    NAND_FLASH_READ_ID_ERR,
    NAND_FLASH_ERASE_BLOCK_ERR,
		NAND_FLASH_ERASE_CHIP_ERR,
    NAND_FLASH_READ_ERR,
    NAND_FLASH_WRITE_ERR,
    NAND_FLASH_CHECK_ERR,
    NAND_FLASH_ALGORITHM_RESULT_ERR
} NAND_FLASH_STA_t;

// sample test for NandFlash Read & Write
NAND_FLASH_STA_t NAND_FLASH_test(void);

// below interfaces is compatible with flash program algorithm, refer to LAT1198
NAND_FLASH_STA_t NAND_FLASH_erase_block(unsigned long adr);
NAND_FLASH_STA_t NAND_FLASH_erase_chip(void);
NAND_FLASH_STA_t NAND_FLASH_write_page(unsigned long adr, unsigned long sz, unsigned char *buf);

// tese above 3 interfases
NAND_FLASH_STA_t NAND_FLASH_algorithm_interface_test(void);
NAND_FLASH_STA_t NAND_FLASH_read_page(unsigned long adr, unsigned long sz, unsigned char *buf);

// test the algorithm result
NAND_FLASH_STA_t NAND_FLASH_algorithm_result_test(void);

#ifdef __cplusplus
}
#endif

#endif  /* __NAND_FLASH_H__ */
