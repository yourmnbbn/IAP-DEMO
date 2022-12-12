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
#include "..\config.h"
#include "bootloadercfg.h"
#include "bootloader.h"
#include "crc32.h"

#define LED4 (1 << 0)
#define LED4ON()    FIO0DIR |= LED4; FIO0CLR = LED4
#define LED4OFF()    FIO0DIR |= LED4; FIO0SET = LED4

#define KEY1 (1 << 11)
#define KEY2 (1 << 10)

#define UARTBPS 115200

volatile uint32_t 	g_RecvLength 			= 0;
volatile uint32_t 	g_SysTickCount 		= 0;
volatile uint8_t 		g_OverflowFlag 		= 0;
volatile uint8_t 		g_RecvBinaryImage = 0;

//-----------------------------------------------------------------------------
// HighResolutionDelay
// 
// High resolution delay function in ms, tick count reference is from systick.
//-----------------------------------------------------------------------------
void HighResolutionDelay(INT32U ulTime)
{
	uint8_t overflow = ((uint32_t)(-1) - g_SysTickCount) < ulTime;
	uint8_t flag = g_OverflowFlag;
	uint32_t current_tick = g_SysTickCount;
	
	if(overflow)
	{
		while(flag == g_OverflowFlag);
	}

	while(g_SysTickCount <= current_tick + ulTime);
}

//-----------------------------------------------------------------------------
// sysTickInit
// 
// Initialize system tick
//-----------------------------------------------------------------------------
void sysTickInit()
{
	//Enable and set sys tick interval 1ms
	STCTRL = (1 << 0) | (1 << 1) | (1 << 2);
	STRELOAD = (FCCLK / 1000) - 1;
}

//-----------------------------------------------------------------------------
// sysTickShut
// 
// Shutdown system tick
//-----------------------------------------------------------------------------
void sysTickShut()
{
	STCTRL &= ~(1 << 0);
	STCTRL &= ~(1 << 1);
}

//-----------------------------------------------------------------------------
// sysTickIsr
// 
// System tick ISR
//-----------------------------------------------------------------------------
void sysTickIsr()
{
	if(g_SysTickCount == (uint32_t)-1)
	{
		g_OverflowFlag = g_OverflowFlag == 0 ? 1 : 0;
	}
	
	++g_SysTickCount;
}

//-----------------------------------------------------------------------------
// LED_Blink
// 
// Make led1 blink.
//-----------------------------------------------------------------------------
void LED_Blink(INT8U count, INT32U interval)
{
	for(; count > 0; --count)
	{
		LED4ON();
		HighResolutionDelay(interval);
		LED4OFF();
		HighResolutionDelay(interval);
	}
}

//-----------------------------------------------------------------------------
// uart0Isr
// 
// UART0 interrupt service routine
//-----------------------------------------------------------------------------
void  uart0Isr (void)
{
  volatile INT8U i; 
	static uint32_t index = 0;
	
	while ((U0IIR & 0x01) == 0) 
	{   
		switch (U0IIR & 0x0E) 
		{        
			case 0x04:                 
				for (i = 0; i < 7; i++) {                 				  
						((uint8_t*)configTEMP_RAM_ADDR)[index++] = U0RBR;
				}
				break;
				
			case 0x0C:               
				while ((U0LSR & 0x01) == 0x01) {        
						((uint8_t*)configTEMP_RAM_ADDR)[index++] = U0RBR;
				}
				
				g_RecvBinaryImage = 1;
				g_RecvLength = index;
				index = 0;
				break;
						
			default:
				break;
		}
	} 
}

//-----------------------------------------------------------------------------
// uart0Init
// 
// UART0 initialization
//-----------------------------------------------------------------------------
void  uart0Init (void)
{
	INT16U usFdiv;
	
	U0LCR  = 0x83;                                                 
	usFdiv = (FPCLK / 16) / UARTBPS;                                    
	U0DLM  = usFdiv / 256;
	U0DLL  = usFdiv % 256;
	U0LCR  = 0x03;
	U0FCR  = 0x87;
	U0IER  = 0x01;
	
	zyIsrSet(NVIC_UART0, NULL, PRIO_ONE);
}

//-----------------------------------------------------------------------------
// main
// 
// Bootloader program entrance
//-----------------------------------------------------------------------------
int main (void)
{
	BCB_t bcb;
	BCB_t* bcb_read = NULL;

	targetInit();
	pinInit();                                                                  
	sysTickInit();
	
	FIO2DIR &= ~KEY1; 
	FIO2DIR &= ~KEY2; 
	
	//Initialize IAP function address
	IAP_Entry = (void(*)())IAP_ENTER_ADR;
	
	//Check bcb
	bcb_read = GetBootloaderControlBlock();
	if(bcb_read->magic != configBCB_MAGIC || bcb_read->version != BCB_VERSION)
	{
		InitializeNewBCB(&bcb);
		SaveBootloaderControlBlock(&bcb);
	}
	
	//Incase we're updating the program and save the midification.
	memcpy(&bcb, bcb_read, sizeof(BCB_t));
	
	//Hold KEY1 while reset will enter recover mode, LED4 keep blink
	if((FIO2PIN & KEY1) == 0)
	{
		uart0Init();
		
		for( ;; )
		{
			LED_Blink(1, 250);
			
			//Press key2 to exit recover mode
			if((FIO2PIN & KEY2) == 0)
				break;
			
			if(g_RecvBinaryImage == 1)
			{
				g_RecvBinaryImage = 0;
				
				//Writes the binary image to the flash
				EraseTargetSectors(bcb_read->slot[0].start_addr, g_RecvLength);
				WriteDataToFlash(bcb_read->slot[0].start_addr, configTEMP_RAM_ADDR, g_RecvLength);
				
				//Save the new bcb info to the flash
				bcb.slot[0].size = g_RecvLength;
				bcb.slot[0].valid = 1;
				bcb.slot[0].run_this = 1;
				bcb.slot[0].crc32 = crc32buf((uint8_t*)bcb_read->slot[0].start_addr, g_RecvLength);
				
				SaveBootloaderControlBlock(&bcb);
				break;
			}
		}
		
		zyIsrClr(NVIC_UART0);
	}
	
	//LED4 blink 5 times, meaning we are in the bootloader.
	LED_Blink(5, 100);
	
	//Shutdown any system resources and peripherals we've used.
	sysTickShut();
	
	//bootloader program ends here, hand over control and will never return.
	RunApplicationAtAddress(FindValidProgramEntry(&bcb));
}
