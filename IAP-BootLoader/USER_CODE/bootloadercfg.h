/********************************************************************************************************************
* IAP Bootloader framework.
* Copyright (C) 2022 Joe Leon(Liyang Qiao).  All rights reserved.
* 
* This bootloader example is based on LPC1768.
*
* Using zhouligong's lpc17xx framework, user app must be downloaded at the start address of 0x10000, which will 
* require the following settings in keil.
*   1. Options -> Target -> Read/Only Memory Areas, set on-chip IROM1 to 0x10000, 0x70000
*   2. Debug -> Debugger Setting-> Flash Download, set address range to 0x10000, 0x70000
*   3. in cfg_file/Startup/InChip.sct file, set ROM_LOAD 0x00010000 and VECTOR 0x00010000
*
* Press KEY1 and reset the board will enter recovery mode(LED4 will keep blink), send binary image(.bin) through 
* serial port and the program will be written to flash and then be executed.
********************************************************************************************************************/
#ifndef __IAP_BOOTLOADER_CONFIG_HEADER__ 
#define __IAP_BOOTLOADER_CONFIG_HEADER__

#define configBCB_MAGIC 0xdeadbeef

//Total flash size of a chip in KB
#define configCHIP_TOTAL_FLASH_SIZE 512

//Don't modify this
#define configIAP_CONTROL_BLOCK_SIZE 4096

//On which sector the bcb is stored
#define configIAP_CONTROL_BLOCK_SECTOR 15

//On which sector user application is stored
#define configIAP_USER_APP_SECTOR 16

//Sector size of the flash on chip
#define configCHIP_FLASH_SECTOR_SIZE 4096

//User app start address, e.g. lpc17xx is 0x00000000, stm32 is 0x80000000
#define configUSER_CODE_START_ADDR 0

//Pre-allocated size for bootloader program(BCB included)
#define configIAP_BOOTLOADER_CODE_SIZE 0x10000

//Temp ram memory for data transmition
#define configTEMP_RAM_ADDR 0x2007C000

//Temp ram memory size
#define configTEMP_RAM_SIZE 16 * 1024

//Don't modify this
#define configIAP_BOOTLOADER_USER_APP_ENTRY (configUSER_CODE_START_ADDR + configIAP_BOOTLOADER_CODE_SIZE)

//How many program slots can be in the flash
#define configPROGRAM_MAX_SLOT 2

//Development test only, don't modify this
#define configUSING_ASM_CONTEXT_SWITCH 1

#endif //__IAP_BOOTLOADER_CONFIG_HEADER__
