#include "bootloader.h"

#if configUSING_ASM_CONTEXT_SWITCH
	//New compiler?
	#if __IS_ARM_COMPILER_6__
		void RunApplicationAtAddress(uint32_t address)
		{
			//We don't want any isr to interrupt this process
			__asm("cpsid i");
			
			//Update VTOR
			__asm("ldr r1, =0xE000ED08");
			__asm("str r0, [ r1 ]");
			
			//The NVIC vector always have to be stored in the start address of each binary image
			__asm("ldr r1, [ r0 ]");
			
			//Reset the main stack to the initial value of the user app
			__asm("msr msp, r1");
			__asm("dsb");
			__asm("isb");
			
			//Reset pc to the application entry, bootloader hand over cpu control to the user app
			__asm("ldr pc, [ r0, #4 ]");
		}
	#else //#if __IS_ARM_COMPILER_6__
		__asm void RunApplicationAtAddress(uint32_t address)
		{
			//We don't want any isr to interrupt this process
			cpsid i
			
			//Update VTOR
			ldr r1, =0xE000ED08
			str r0, [ r1 ]
			
			//The NVIC vector always have to be stored in the start address of each binary image
			ldr r1, [ r0 ]
			
			//Reset the main stack to the initial value of the user app
			msr msp, r1
			dsb
			isb
			
			//Reset pc to the application entry, bootloader hand over cpu control to the user app
			ldr pc, [ r0, #4 ]
		}
	#endif //#if __IS_ARM_COMPILER_6__
#else //configUSING_ASM_CONTEXT_SWITCH
	void RunApplicationAtAddress(uint32_t address)
	{
		//We don't want any isr to interrupt this process
		DISABLE_ISR;
		
		//Relocate the NVIC Vector
		*(uint32_t*)0xE000ED08 = address;
		//The stack is not recovered to the origion value by this implementation, it's not recommended to use this implementation.
		(*(void (**)(void))(address + 4))();
	}
#endif //configUSING_ASM_CONTEXT_SWITCH

BCB_t* GetBootloaderControlBlock(void)
{
	return (BCB_t*)(configCHIP_FLASH_SECTOR_SIZE * configIAP_CONTROL_BLOCK_SECTOR);
}

void SaveBootloaderControlBlock(BCB_t* pData)
{
	//Size can only be 512, 1024 or 4096
	uint16_t write_size = 512;
	
	//Figure out how many bytes we have to write in total
	if(sizeof(BCB_t) > 1024)
		write_size = 4096;
	else if(sizeof(BCB_t) > 512)
		write_size = 1024;
	
	//Prepare data in temp ram, fill 0xFF to unused memory
	memset((void*)(configTEMP_RAM_ADDR + sizeof(BCB_t)), 0xFF, write_size - sizeof(BCB_t));
	memcpy((void*)configTEMP_RAM_ADDR, pData, sizeof(BCB_t));
	
	//Write data to flash
	DISABLE_ISR;
	sectorPrepare(configIAP_CONTROL_BLOCK_SECTOR, configIAP_CONTROL_BLOCK_SECTOR);
	sectorErase(configIAP_CONTROL_BLOCK_SECTOR, configIAP_CONTROL_BLOCK_SECTOR);
	
	sectorPrepare(configIAP_CONTROL_BLOCK_SECTOR, configIAP_CONTROL_BLOCK_SECTOR);
	ramToFlash(configCHIP_FLASH_SECTOR_SIZE * configIAP_CONTROL_BLOCK_SECTOR, configTEMP_RAM_ADDR, write_size);
	ENABLE_ISR;
}

void InitializeNewBCB(BCB_t* data)
{
	memset(data, 0, sizeof(BCB_t));
	
	data->magic = configBCB_MAGIC;
	data->version = BCB_VERSION;
	data->max_slots = configPROGRAM_MAX_SLOT;
	
	data->slot[0].start_addr = configIAP_BOOTLOADER_USER_APP_ENTRY;
	data->slot[0].valid = 1;
	data->slot[0].run_this = 0;
}

uint32_t FindValidProgramEntry(BCB_t* pData)
{
	uint8_t program_itor = 0;
	
	for(program_itor = 0; program_itor < configPROGRAM_MAX_SLOT; ++program_itor)
	{
		if(pData->slot[program_itor].run_this)
			return pData->slot[program_itor].start_addr;
	}
	
	return configUSER_CODE_START_ADDR;
}

void EraseTargetSectors(uint32_t flash_address, uint32_t length)
{
	uint8_t start_sector = flash_address / configCHIP_FLASH_SECTOR_SIZE;
	uint8_t end_sector = (flash_address + length) / configCHIP_FLASH_SECTOR_SIZE;
	
	DISABLE_ISR;
	sectorPrepare(start_sector, end_sector);
	sectorErase(start_sector, end_sector);
	ENABLE_ISR;
}

void WriteDataToFlash(uint32_t flash_address, uint32_t ram_address, uint32_t length)
{
	uint8_t start_sector = flash_address / configCHIP_FLASH_SECTOR_SIZE;
	uint8_t end_sector = (flash_address + length) / configCHIP_FLASH_SECTOR_SIZE;
	uint8_t sector_count = end_sector - start_sector;
	uint8_t itor = 0;
	
	uint32_t bytes_left = 0;
	uint16_t last_write_size = 512;
	uint16_t bytes_fill = 0;
	uint32_t base_addr = 0;
	
	DISABLE_ISR;

	for(itor = 0; itor < sector_count + 1; ++itor)
	{
		base_addr = flash_address + itor * configCHIP_FLASH_SECTOR_SIZE;
		
		//Last sector
		if(itor == sector_count)
		{
			bytes_left = length - configCHIP_FLASH_SECTOR_SIZE * sector_count;
			
			if(bytes_left > 1024)
				last_write_size = 4096;
			else if(bytes_left > 512)
				last_write_size = 1024;
			
			//Fill 0xFF to non-used area of flash
			//Note that improper slicing of ram data would cause data corruption
			bytes_fill = last_write_size - bytes_left;
			memset((void*)(ram_address + bytes_left), 0xFF, bytes_fill);
			
			sectorPrepare(start_sector, end_sector);
			ramToFlash(base_addr, ram_address + itor * configCHIP_FLASH_SECTOR_SIZE, last_write_size);
			break;
		}
		
		sectorPrepare(start_sector, end_sector);
		ramToFlash(base_addr, ram_address + itor * configCHIP_FLASH_SECTOR_SIZE, configCHIP_FLASH_SECTOR_SIZE);
	}
	
	ENABLE_ISR;
}

