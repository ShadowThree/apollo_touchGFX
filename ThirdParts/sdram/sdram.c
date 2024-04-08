#include "sdram.h"

#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)

void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram)
{
  __IO uint32_t tmpmrd = 0;
	FMC_SDRAM_CommandTypeDef Command;
  /* Step 1:  Configure a clock configuration enable command */
  Command.CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
  Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
  Command.AutoRefreshNumber = 1;
  Command.ModeRegisterDefinition = 0;

  /* Send the command */
  HAL_SDRAM_SendCommand(hsdram, &Command, SDRAM_TIMEOUT);

  /* Step 2: Insert 100 us minimum delay */
  /* Inserted delay is equal to 1 ms due to systick time base unit (ms) */
  HAL_Delay(1);

  /* Step 3: Configure a PALL (precharge all) command */
  Command.CommandMode = FMC_SDRAM_CMD_PALL;
  Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
  Command.AutoRefreshNumber = 1;
  Command.ModeRegisterDefinition = 0;

  /* Send the command */
  HAL_SDRAM_SendCommand(hsdram, &Command, SDRAM_TIMEOUT);

  /* Step 4 : Configure a Auto-Refresh command */
  Command.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
  Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
  Command.AutoRefreshNumber = 8;
  Command.ModeRegisterDefinition = 0;

  /* Send the command */
  HAL_SDRAM_SendCommand(hsdram, &Command, SDRAM_TIMEOUT);

  /* Step 5: Program the external memory mode register */
  tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1 |
           SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL |
           SDRAM_MODEREG_CAS_LATENCY_3 |
           SDRAM_MODEREG_OPERATING_MODE_STANDARD |
           SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

  Command.CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
  Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
  Command.AutoRefreshNumber = 1;
  Command.ModeRegisterDefinition = tmpmrd;

  /* Send the command */
  HAL_SDRAM_SendCommand(hsdram, &Command, SDRAM_TIMEOUT);

  /* Step 6: Set the refresh rate counter */
  /* Set the device refresh rate */
  HAL_SDRAM_ProgramRefreshRate(hsdram, REFRESH_COUNT);
}

// must access in word-aligned
uint8_t extSDRAM_test(void)
{
	// write data to extSDRAM
	for(uint32_t i = 0; i < SDRAM_WORD_NUM; i += sizeof(SDRAM_WORD_t)) {
		*(SDRAM_WORD_t*)(SDRAM_BANK1_ADDR + i) = i % 0xFFFF;
	}
	
	// check the data
	for(uint32_t i = 0; i < SDRAM_WORD_NUM; i += sizeof(SDRAM_WORD_t)) {
		if(*(SDRAM_WORD_t*)(SDRAM_BANK1_ADDR + i) != (i % 0xFFFF)) {
			SDRAM_ERR("EXT SDRAM test err at %d\n", i);
			return EXT_SDRAM_TEST_ERR;
		}
	}
	return EXT_SDRAM_TEST_OK;
}
