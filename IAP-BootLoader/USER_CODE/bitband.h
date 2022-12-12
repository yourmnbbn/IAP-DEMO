/********************************************************************************************************
* Bit-Band Helper Header for Cortex-M3/M4.
* Copyright (C) 2022 Joe Leon(Liyang Qiao).  All rights reserved.
* 
* This bit-band operation can be used as atomic operation, it's useful when running a RTOS or variables
* which are going to be used both in ISR and general service code to prevent data race.
*
* Example interface:
*   BITBAND_SET(address, bit_num) //Set one bit
*   BITBAND_CLR(address, bit_num) //Clear one bit
*   BITBAND_GET(address, bit_num) //Read one bit
*
* High performance example:
*   //Following code can't be compiled in C but C++.
*   //Assuming we want to flip bit[6] in address 0x20001CFD multiple times in a loop, we just have
*   //to calculate target bit-band address once
*   uint32_t bitband_addr = GET_BITBAND_ADDRESS(0x20001CFD, 6);
*   for(int i = 0; i < 10; i++)
*   {
*     *(uint32_t*)bitband_addr = i % 2; //Faster than BITBAND_REF(0x20001CFD, 6) = i % 2;(-o0)
*   }
*   
* Note that if your target MCU doesn't support bit-band operation, you can still use this interface
* for a better compatibility.
*********************************************************************************************************/
#ifndef __BITBAND_HELPER_HEADER__ 
#define __BITBAND_HELPER_HEADER__

#include <stdint.h>

/*******************************************************************
*                             Configs
********************************************************************/
//Bit-band configs, if this is set, calling macro BITBAND_REF with address without bit-band support will stay in an infinite loop
//If you care about the performance you can disable this, but make sure you won't pass any non-bitband address or there would be chaos.
#define configBITBAND_ADDR_CHECK    1

//Does your target MCU support bit-band? For better compatibility.
#define configMCU_SUPPORT_BITBAND   1

/*******************************************************************
*                         Implementations
********************************************************************/
//bit-band address check fault
#define BITBAND_FAULT() while(1)

//Check if the provided address support bit band, the valid address should be 0x20000000(bit[0])-0x200FFFFC(bit[31]) and 0x40000000(bit[0])-0x400FFFFF(bit[31])
#if (configBITBAND_ADDR_CHECK == 1)
  #define CHECK_BITBAND_VALID(address) \
            if(!(((uint32_t)address >= 0x20000000 && (uint32_t)address <= 0x200FFFFF) || ((uint32_t)address >= 0x40000000 && (uint32_t)address <= 0x400FFFFF))) \
              BITBAND_FAULT()
#else
  #define CHECK_BITBAND_VALID(address)
#endif
		
//Get base address of a bit-band support memory area to calculate the offset
#define GET_BASE_BITBAND_ADDR(address)            ((uint32_t)address > 0x2FFFFFFF ? 0x40000000 : 0x20000000)
		
//Calculate the corresponding bit-band address
#define GET_BITBAND_ADDRESS(address, bit_num)     (GET_BASE_BITBAND_ADDR(address) + 0x02000000 + ((uint32_t)address - GET_BASE_BITBAND_ADDR(address)) * 32 + bit_num * 4)

// Bit-band reference getter, bit_num starts from 0
// Example usage: 
//    BITBAND_REF(0x20001CFD, 5) = 1; //Set the 6th bit of byte in address 0x20001CFD to 1
#define BITBAND_REF(address, bit_num)																		\
          CHECK_BITBAND_VALID(address); 																\
          *(volatile uint32_t*)GET_BITBAND_ADDRESS(address, bit_num)

//Set one bit
#if (configMCU_SUPPORT_BITBAND == 1)
  #define BITBAND_SET(address, bit_num)           BITBAND_REF(address, bit_num) = 1
#else
  #define BITBAND_SET(address, bit_num)           *(volatile uint32_t*)address |= (1 << bit_num)
#endif

//Clear one bit
#if (configMCU_SUPPORT_BITBAND == 1)
  #define BITBAND_CLR(address, bit_num)           BITBAND_REF(address, bit_num) = 0
#else	
  #define BITBAND_CLR(address, bit_num)           *(volatile uint32_t*)address &= ~(1 << bit_num)
#endif

//Read one bit
#if (configMCU_SUPPORT_BITBAND == 1)
  #define BITBAND_GET(address, bit_num)           BITBAND_REF(address, bit_num)
#else
  #define BITBAND_GET(address, bit_num)           (*(volatile uint32_t*)address & (1 << bit_num)) ? 1 : 0
#endif

#endif //__BITBAND_HELPER_HEADER__
