/*
 * utils.c
 *
 *  Created on: Feb 24, 2023
 *      Author: Siyuan Xu
 */
#include "stm32l1xx.h"

void delay_us(const unsigned long delay)
{
	unsigned long i=0;
		  SysTick->LOAD=32-1; //32 000 000 = 1s so 32 = 1 us
		  SysTick->VAL=0;
		  //enable counter, use processor clock, M3 Generic User Guide p. 159
		  SysTick->CTRL=5;

	  while(i<delay)
	  {
		  //CTRL register bit 16  returns 1 if timer counted to 0 since last time this was read.
		  while(!((SysTick->CTRL)&0x10000)){} //M3 Generic User Guide p. 159
		  i++;
	  }
}

void delay_ms(const unsigned long delay)
{
	unsigned long i=0;
		  SysTick->LOAD=32000-1; //32 000 000 = 1s so 32 = 1 us
		  SysTick->VAL=0;
		  //enable counter, use processor clock, M3 Generic User Guide p. 159
		  SysTick->CTRL=5;

	  while(i<delay)
	  {
		  //CTRL register bit 16  returns 1 if timer counted to 0 since last time this was read.
		  while(!((SysTick->CTRL)&0x10000)){} //M3 Generic User Guide p. 159
		  i++;
	  }
}
