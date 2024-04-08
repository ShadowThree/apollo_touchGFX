#include "nand_flash.h"

NAND_FLASH_STA_t Nand_Flash_test(void)
{
	HAL_StatusTypeDef hal_sta = HAL_OK;
	NAND_IDTypeDef nand_id;
	NAND_AddressTypeDef nand_addr = {.Page = 0, .Block = 1, .Plane = 0};
	uint8_t buf[NAND_FLASH_PAGE_SIZE] = {0};
	
	hal_sta = HAL_NAND_Reset(&hnand1);
	if(hal_sta != HAL_OK) {
		NAND_ERR("Nand Flash reset err[%d]\n", hal_sta);
		return NAND_FLASH_RESET_ERR;
	}
	
	hal_sta = HAL_NAND_Read_ID(&hnand1, &nand_id);
	if(hal_sta != HAL_OK) {
		NAND_ERR("Nand Flash read ID err[%d]\n", hal_sta);
		return NAND_FLASH_READ_ID_ERR;
	} else {
		// should be 0xADDC9095
		NAND_DBG("Nand Flash ID[0x%02x%02x%02x%02x]\n", nand_id.Maker_Id, nand_id.Device_Id, nand_id.Third_Id, nand_id.Fourth_Id);
	}
	
	hal_sta = HAL_NAND_Erase_Block(&hnand1, &nand_addr);
	if(hal_sta != HAL_OK) {
		NAND_ERR("Nand Flash erase err[%d]\n", hal_sta);
		return NAND_FLASH_ERASE_ERR;
	}
	
	memset(buf, 0, NAND_FLASH_PAGE_SIZE);
	hal_sta = HAL_NAND_Read_Page_8b(&hnand1, &nand_addr, buf, 1);
	if(hal_sta != HAL_OK) {
		NAND_ERR("Nand Flash read after erase err[%d]\n", hal_sta);
		return NAND_FLASH_READ_ERR;
	}
	
	#if 0
	LOG_DBG("Nand Flash read data(after erase):\n");
	for(uint8_t i = 0; i < 100; i++) {
		if(i != 0 && i % 10 == 0) {
			LOG_DBG("\n");
			HAL_Delay(20);
		}
		LOG_DBG(" %02x", buf[i]);
	}
	LOG_DBG("\n\n");
	#endif
	
	for(uint16_t i = 0; i < NAND_FLASH_PAGE_SIZE; i++) {
		buf[i] = i % 256;
	}
	hal_sta = HAL_NAND_Write_Page_8b(&hnand1, &nand_addr, buf, 1);
	if(hal_sta != HAL_OK) {
		NAND_ERR("Nand Flash write err[%d]\n", hal_sta);
		return NAND_FLASH_WRITE_ERR;
	}
	
	memset(buf, 0, NAND_FLASH_PAGE_SIZE);
	hal_sta = HAL_NAND_Read_Page_8b(&hnand1, &nand_addr, buf, 1);
	if(hal_sta != HAL_OK) {
		NAND_ERR("Nand Flash read after write err[%d]\n", hal_sta);
		return NAND_FLASH_READ_ERR;
	}
	
	#if 0
	LOG_DBG("Nand Flash read data(after write):\n");
	for(uint8_t i = 0; i < 100; i++) {
		if(i != 0 && i % 10 == 0) {
			LOG_DBG("\n");
			HAL_Delay(20);
		}
		LOG_DBG(" %02x", buf[i]);
	}
	LOG_DBG("\n\n");
	#endif
	
	// check data
	for(uint16_t i = 0; i < NAND_FLASH_PAGE_SIZE; i++) {
		if(buf[i] != (i%256)) {
			NAND_ERR("Nand Flash check buf[%d]=[%d] err\n", i, buf[i]);
			return NAND_FLASH_CHECK_DATA_ERR;
		}
	}
	
	return NAND_FLASH_OK;
}
