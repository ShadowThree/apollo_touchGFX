#include "nand_flash.h"

#if !USE_FOR_CREATE_FLM
#define TEST_NUM    97
NAND_FLASH_STA_t NAND_FLASH_test(void)
{
    HAL_StatusTypeDef hal_sta = HAL_OK;
    NAND_IDTypeDef nand_id = {0};
    uint8_t buf[PAGE_SIZE] = {0};
    
   hal_sta = HAL_NAND_Reset(&hnand1);
    if(hal_sta == HAL_OK) {
        NAND_FLASH_DBG("nand flash reset OK\n");
    } else {
        NAND_FLASH_ERR("nand flash reset err[%d]\n", hal_sta);
        return NAND_FLASH_RESET_ERR;
    }
    
    // read ID (0xADF1001D for H27U1G8F2BTR)    // 0xADDC9095 for H27U4G8F2ETR
    hal_sta = HAL_NAND_Read_ID(&hnand1, &nand_id);
    if(hal_sta == HAL_OK) {
        NAND_FLASH_DBG("nand flash id[0x%02x%02x%02x%02x]\n", nand_id.Maker_Id, nand_id.Device_Id, nand_id.Third_Id, nand_id.Fourth_Id);
    } else {
        NAND_FLASH_ERR("read nand flash id err[%d]\n", hal_sta);
        return NAND_FLASH_READ_ID_ERR;
    }
    
    NAND_AddressTypeDef nandAddr = {.Page = 0, .Block = BLOCK_NUM-1, .Plane = 0};   // last block
    hal_sta = HAL_NAND_Erase_Block(&hnand1, &nandAddr);
    if(hal_sta == HAL_OK) {
        NAND_FLASH_DBG("erase nand flash OK\n");
    } else {
        NAND_FLASH_ERR("erase nand flash err[%d]\n", hal_sta);
        return NAND_FLASH_ERASE_BLOCK_ERR;
    }
    
    memset(buf, 0, PAGE_SIZE);
    hal_sta = HAL_NAND_Read_Page_8b(&hnand1, &nandAddr, buf, 1);
    if(hal_sta == HAL_OK) {
        NAND_FLASH_DBG("read nand flash after erase OK\n");
    } else {
        NAND_FLASH_ERR("read nand flash after erase err[%d]\n", hal_sta);
        return NAND_FLASH_READ_ERR;
    }
    #if 0   // print the data
    NAND_FLASH_DBG("\nread data after erase:\n");
    for(uint8_t i = 0; i < 100; i++) {
        if((i != 0) && (i % 10 == 0)) {
            NAND_FLASH_DBG("\n");
            HAL_Delay(20);
        }
        NAND_FLASH_DBG(" %02x", buf[i]);
    }
    NAND_FLASH_DBG("\n\n");
    #endif
    
    for(uint16_t i = 0; i < PAGE_SIZE; i++) {
        buf[i] = i % TEST_NUM;
    }
    hal_sta = HAL_NAND_Write_Page_8b(&hnand1, &nandAddr, buf, 1);
    if(hal_sta == HAL_OK) {
        NAND_FLASH_DBG("write nand flash OK\n");
    } else {
        NAND_FLASH_ERR("write nand flash err[%d]\n", hal_sta);
        return NAND_FLASH_WRITE_ERR;
    }
    
    memset(buf, 0, PAGE_SIZE);
    hal_sta = HAL_NAND_Read_Page_8b(&hnand1, &nandAddr, buf, 1);
    if(hal_sta == HAL_OK) {
        NAND_FLASH_DBG("read nand flash after write OK\n");
    } else {
        NAND_FLASH_ERR("read nand flash after write err[%d]\n", hal_sta);
        return NAND_FLASH_READ_ERR;
    }
    
    for(uint16_t i = 0; i < PAGE_SIZE; i++) {

        #if 1   // print the data
        if(i % 16 == 0) {
            NAND_FLASH_DBG("\n %03d:", i / 16 + 1);
            HAL_Delay(50);
        }
        NAND_FLASH_DBG(" %02x(%02x)", buf[i], i % TEST_NUM);
        #endif

        if(buf[i] != (i % TEST_NUM)) {
            NAND_FLASH_ERR("\nread buf[%d] err[0x%02x 0x%02x]\n", i, buf[i], i % TEST_NUM);
            return NAND_FLASH_CHECK_ERR;
        }
    }
    NAND_FLASH_DBG("\n");
    return NAND_FLASH_STA_OK;
}
#else
NAND_FLASH_STA_t NAND_FLASH_test(void)
{
    return NAND_FLASH_STA_OK;
}
#endif

NAND_FLASH_STA_t NAND_FLASH_erase_block(unsigned long adr)
{
    HAL_StatusTypeDef hal_sta = HAL_OK;
    adr -= NAND_FLASH_ADDR;
    NAND_AddressTypeDef nand_addr = {.Page = 0, .Block = adr / BLOCK_SIZE, .Plane = 0};    // blockSize=128KB
    hal_sta = HAL_NAND_Erase_Block(&hnand1, &nand_addr);
    if(hal_sta != HAL_OK) {
        //NAND_FLASH_ERR("Nand Flash erese block err[%d]\n", hal_sta);
        return NAND_FLASH_ERASE_BLOCK_ERR;
    }
    //NAND_FLASH_DBG("Nand Flash erase block OK\n");
    return NAND_FLASH_STA_OK;
}

NAND_FLASH_STA_t NAND_FLASH_erase_chip(void)
{
    HAL_StatusTypeDef hal_sta = HAL_OK;
    for(uint32_t i = 0; i < BLOCK_NUM; i++) {
        NAND_AddressTypeDef nand_addr = {.Page = 0, .Block = i, .Plane = 0}; 
        hal_sta = HAL_NAND_Erase_Block(&hnand1, &nand_addr);
        if(hal_sta != HAL_OK) {
            //NAND_FLASH_ERR("Nand Flash erese block err[%d]\n", hal_sta);
         	return NAND_FLASH_ERASE_CHIP_ERR;
        }
    }
    //NAND_FLASH_DBG("Nand Flash erase chip OK\n");
    return NAND_FLASH_STA_OK;
}

NAND_FLASH_STA_t NAND_FLASH_write_page(unsigned long adr, unsigned long sz, unsigned char *buf)
{
    HAL_StatusTypeDef hal_sta = HAL_OK;
    adr -= NAND_FLASH_ADDR;
    NAND_AddressTypeDef nand_addr = {.Page = adr % BLOCK_SIZE / PAGE_SIZE, .Block = adr / BLOCK_SIZE, .Plane = 0};
    hal_sta = HAL_NAND_Write_Page_8b(&hnand1, &nand_addr, buf, 1);
    if(hal_sta != HAL_OK) {
        //NAND_FLASH_ERR("Nand Flash write err[%d]\n", hal_sta);
        return NAND_FLASH_WRITE_ERR;
    }
    //NAND_FLASH_DBG("Nand Flash write OK\n");
    return NAND_FLASH_STA_OK;
}

NAND_FLASH_STA_t NAND_FLASH_read_page(unsigned long adr, unsigned long sz, unsigned char *buf)
{
    HAL_StatusTypeDef hal_sta = HAL_OK;
    adr -= NAND_FLASH_ADDR;
    NAND_AddressTypeDef nand_addr = {.Page = adr % BLOCK_SIZE / PAGE_SIZE, .Block = adr / BLOCK_SIZE, .Plane = 0};
    hal_sta = HAL_NAND_Read_Page_8b(&hnand1, &nand_addr, buf, 1);
    if(hal_sta != HAL_OK) {
        //NAND_FLASH_ERR("Nand Flash write err[%d]\n", hal_sta);
        return NAND_FLASH_READ_ERR;
    }
    //NAND_FLASH_DBG("Nand Flash write OK\n");
    return NAND_FLASH_STA_OK;
}

#if !USE_FOR_CREATE_FLM
NAND_FLASH_STA_t NAND_FLASH_algorithm_interface_test(void)
{
    #define TEST_ADDR   (NAND_FLASH_ADDR + PLANE_NUM * PLANE_SIZE - BLOCK_SIZE)     // last block
    NAND_FLASH_STA_t nand_flash_sta = NAND_FLASH_STA_OK;
    uint8_t buf[PAGE_SIZE] = {0};
    
    // nand_flash_sta = NAND_FLASH_erase_chip();
    // if(nand_flash_sta != NAND_FLASH_STA_OK) {
    //     NAND_FLASH_ERR("Nand Flash erase chip err[%d]\n", nand_flash_sta);
    //     return NAND_FLASH_ERASE_CHIP_ERR;
    // }
    // NAND_FLASH_DBG("Nand Flash erase chip OK\n");
    
    for(uint32_t i = 0; i < PAGE_SIZE; i++) {
        buf[i] = i % TEST_NUM;
    }
    nand_flash_sta = NAND_FLASH_write_page(TEST_ADDR, PAGE_SIZE, buf);
    if(nand_flash_sta != NAND_FLASH_STA_OK) {
        NAND_FLASH_ERR("Nand Flash write page err[%d]\n", nand_flash_sta);
   		return NAND_FLASH_WRITE_ERR;
    }
    NAND_FLASH_DBG("Nand Flash write page OK\n");
    
    memset(buf, 0, PAGE_SIZE);
    nand_flash_sta = NAND_FLASH_read_page(TEST_ADDR, PAGE_SIZE, buf);
      if(nand_flash_sta != NAND_FLASH_STA_OK) {
        NAND_FLASH_ERR("Nand Flash read page err[%d]\n", nand_flash_sta);
        return NAND_FLASH_READ_ERR;
    }
    NAND_FLASH_DBG("Nand Flash read page OK\n");
    
    for(uint32_t i = 0; i < PAGE_SIZE; i++) {

        #if 1   // print data
        if(i % 16 == 0) {
            NAND_FLASH_DBG("\n %03d:", i / 16 + 1);
            HAL_Delay(50);
        }
        NAND_FLASH_DBG(" %02x(%02x)", buf[i], i % TEST_NUM);
        #endif

        if(buf[i] != i % TEST_NUM) {
            NAND_FLASH_ERR("\nNand Flash check data err, buf[%d] = %d\n", i, buf[i]);
            return NAND_FLASH_CHECK_ERR;
        }
    }
    NAND_FLASH_DBG("\nNand Flash check data OK\n");
    
    nand_flash_sta = NAND_FLASH_erase_block(TEST_ADDR);
    if(nand_flash_sta != NAND_FLASH_STA_OK) {
        NAND_FLASH_ERR("Nand Flash erase block err[%d]\n", nand_flash_sta);
        return NAND_FLASH_ERASE_BLOCK_ERR;
    }
    NAND_FLASH_DBG("Nand Flash erase block OK\n");
    
    return NAND_FLASH_STA_OK;
}
#else
NAND_FLASH_STA_t NAND_FLASH_algorithm_interface_test(void)
{
    return NAND_FLASH_STA_OK;
}
#endif

#if !USE_FOR_CREATE_FLM

#define TEST_BY_SECTOR  1

#if TEST_BY_SECTOR
static const uint8_t nand_flash_test_data[PAGE_SIZE] __attribute__((section("ExtFlashSec"))) __attribute__((aligned(4))) = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
};
NAND_FLASH_STA_t NAND_FLASH_algorithm_result_test(void)
{
    LOG_DBG("extFlash data addr[0x%08x]\n", nand_flash_test_data);
    uint8_t nand_read_data[PAGE_SIZE] = {0};
    NAND_FLASH_read_page((unsigned long)nand_flash_test_data, PAGE_SIZE, nand_read_data);
    for(uint16_t i = 0; i < PAGE_SIZE; i++) {
        #if 1   // print data
        if(i % 16 == 0) {
            NAND_FLASH_DBG("\n %03d:", i / 16 + 1);
            HAL_Delay(50);
        }
        NAND_FLASH_DBG(" %02x(%02x)", nand_read_data[i], nand_flash_test_data[i]);
        #endif

        if(nand_read_data[i] != nand_flash_test_data[i]) {
            NAND_FLASH_ERR("\nNand Flash algorithm result err at [%d]\n", i);
            return NAND_FLASH_ALGORITHM_RESULT_ERR;
        }
    }
    NAND_FLASH_DBG("\nNand Flash algorithm result test OK\n");
    return NAND_FLASH_STA_OK;
}
#endif

#if TEST_BY_ADDR
#define NAND_FLASH_TEST_DATA_ADDR   NAND_FLASH_ADDR   // page0
//#define NAND_FLASH_TEST_DATA_ADDR   (NAND_FLASH_ADDR + PLANE_NUM * PLANE_SIZE - PAGE_SIZE)   // last block
//#define NAND_FLASH_TEST_DATA_ADDR   0x08002800    // internal Flash
static const uint8_t nand_flash_test_data[PAGE_SIZE] __attribute__((at(NAND_FLASH_TEST_DATA_ADDR))) = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
};
NAND_FLASH_STA_t NAND_FLASH_algorithm_result_test(void)
{
    LOG_DBG("extFlash data addr[0x%08x]\n", nand_flash_test_data);
    uint8_t nand_read_data[PAGE_SIZE] = {0};
    NAND_FLASH_read_page(NAND_FLASH_TEST_DATA_ADDR, PAGE_SIZE, nand_read_data);      // Only can read the Nand Flash area
    for(uint16_t i = 0; i < PAGE_SIZE; i++) {
				#if 1   // print data
        if(i % 16 == 0) {
            NAND_FLASH_DBG("\n %03d:", i / 16 + 1);
            HAL_Delay(50);
        }
        NAND_FLASH_DBG(" %02x(%02x)", nand_read_data[i], nand_flash_test_data[i]);
        #endif
        if(nand_read_data[i] != nand_flash_test_data[i]) {
            NAND_FLASH_ERR("\nNand Flash algorithm result err at [%d]\n", i);
            return NAND_FLASH_ALGORITHM_RESULT_ERR;
        }
    }
    NAND_FLASH_DBG("\nNand Flash algorithm result test OK\n");
    return NAND_FLASH_STA_OK;
}
#endif

#else
NAND_FLASH_STA_t NAND_FLASH_algorithm_result_test(void) 
{
    return NAND_FLASH_STA_OK;
}
#endif
