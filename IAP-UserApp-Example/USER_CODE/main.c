/********************************************************************************************************
* IAP user application framework.
* Copyright (C) 2022 Joe Leon(Liyang Qiao).  All rights reserved.
* 
* This example is based on LPC1768.
*********************************************************************************************************/
#include "..\config.h"
#include "bitband.h"

#define LED1 (1 << 3)
#define LED1ON()    FIO0DIR |= LED1; FIO0CLR = LED1
#define LED1OFF()    FIO0DIR |= LED1; FIO0SET = LED1

#define LED2 (1 << 2)
#define LED2ON()    FIO0DIR |= LED2; FIO0CLR = LED2
#define LED2OFF()    FIO0DIR |= LED2; FIO0SET = LED2

#define LED3 (1 << 1)
#define LED3ON()    FIO0DIR |= LED3; FIO0CLR = LED3
#define LED3OFF()    FIO0DIR |= LED3; FIO0SET = LED3

#define LED4 (1 << 0)
#define LED4ON()    FIO0DIR |= LED4; FIO0CLR = LED4
#define LED4OFF()    FIO0DIR |= LED4; FIO0SET = LED4


void myDelay (INT32U  ulTime)
{
    INT32U i;   
    i = 0;
    while (ulTime--) {
        for (i = 0; i < 5000; i++);
    }
}

void timer0Isr (void)
{
	//uint32_t addr = 0;
	T0IR         = 0x01;
	
	//addr = GET_BITBAND_ADDRESS(0x400FFFFC, 31);
	//addr = 0;
	
	if (FIO0PIN & LED1) 
	{
		LED1ON();
		LED2ON();
		LED3ON();
		LED4ON();
	} 
	else 
	{
		LED1OFF();
		LED2OFF();
		LED3OFF();
		LED4OFF();
	}
}

void timer0Init (void)
{
    T0TCR  = 0x02;
    T0IR   = 1;
    T0CTCR = 0;
    T0TC   = 0;
    T0PR   = 0;
    T0MR0  = FPCLK;                                                   /* 1S中断1次                  */
    T0MCR  = 0x03;                                                      /* 匹配后产生中断               */
    
		zyIsrSet(NVIC_TIMER0, (unsigned long)timer0Isr, PRIO_TWO);          /* 设置中断并使能               */
    T0TCR  = 0x01;                                                      /* 启动定时器                   */
}

int main (void)
{
	targetInit();
	pinInit(); 
	timer0Init();

	while (1);
}

/*********************************************************************************************************
  End Of File
*********************************************************************************************************/
