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
#ifndef __IAP_BOOTLOADER_HEADER__ 
#define __IAP_BOOTLOADER_HEADER__

#include <stdint.h>
#include <string.h>

#include "bootloadercfg.h"
#include "IAP.h"

//When manipulating flash we have to disable the isr to prevent unexpected hard fault.
#define ENABLE_ISR  __asm("cpsie i")
#define DISABLE_ISR __asm("cpsid i")

//Current BCB version
#define BCB_VERSION 0x00

//The following structs are 1 bytes aligned, the flash memory layout must be identical to the program definition.
#pragma pack(1)
	typedef struct ProgramInfo_s
	{
		uint32_t 	start_addr;					//Start address of the program, usually the NVIC vector
		uint32_t 	version;						//Version of the program
		uint32_t 	size;								//Size of the program binary image
		uint32_t 	crc32;							//CRC32 of the whole program binary image
		uint8_t 	valid;							//Is this program slot valid? 
		uint8_t 	run_this;						//Should we force run this program, regardless of the lower program version?
	} ProgramInfo_t;

	//BCB(Bootloader control block) definition
	typedef struct BCB_s
	{
		uint32_t 	magic;							//Magic number of the BCB
		uint32_t 	version;						//BCB version
		uint8_t 	max_slots;					//Max program slots in the flash.
		uint8_t 	update_required;		//Should bootloader make a flash program update?
		
		ProgramInfo_t slot[configPROGRAM_MAX_SLOT];	//Program information.
	} BCB_t;
#pragma pack()

//-----------------------------------------------------------------------------
// RunApplicationAtAddress
// 
// Hand over control of cpu to user application, once bootloader call this,
// it will never return to bootloader until an extern reset has been done.
//-----------------------------------------------------------------------------
void RunApplicationAtAddress(uint32_t address);

//-----------------------------------------------------------------------------
// GetBootloaderControlBlock
// 
// This gets the stored BCB in flash back.
//-----------------------------------------------------------------------------
BCB_t* GetBootloaderControlBlock(void);

//-----------------------------------------------------------------------------
// SaveBootloaderControlBlock
// 
// This saves the stored BCB to the flash.
//-----------------------------------------------------------------------------
void SaveBootloaderControlBlock(BCB_t* pData);

//-----------------------------------------------------------------------------
// InitializeNewBCB
// 
// The flash doesn't have BCB stored or the version is too old, initialize 
// a new and valid one.
//-----------------------------------------------------------------------------
void InitializeNewBCB(BCB_t* pData);

//-----------------------------------------------------------------------------
// FindValidProgramEntry
// 
// This select a ready program in the BCB and return the srtart addrerss of the
// program image.
//-----------------------------------------------------------------------------
uint32_t FindValidProgramEntry(BCB_t* pData);

//-----------------------------------------------------------------------------
// WriteDataToFlash
// 
// This writes the received binary image in the temp memory to the flash.
// Function will automatically prepare the sectors to be operated according
// to the passed address, offset and data length.
// 
// Note that flash address must be 512 bytes aligned.
//-----------------------------------------------------------------------------
void WriteDataToFlash(uint32_t flash_address, uint32_t ram_address, uint32_t length);

//-----------------------------------------------------------------------------
// EraseTargetSectors
// 
// Erase the target sectors for further writes.
//-----------------------------------------------------------------------------
void EraseTargetSectors(uint32_t flash_address, uint32_t length);

#endif //__IAP_BOOTLOADER_HEADER__ 
