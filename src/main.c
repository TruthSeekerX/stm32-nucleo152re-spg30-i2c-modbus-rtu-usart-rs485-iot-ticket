/*
******************************************************************************
File:     main.c
Info:     Generated by Atollic TrueSTUDIO(R) 9.3.0   2023-02-24

The MIT License (MIT)
Copyright (c) 2019 STMicroelectronics

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

******************************************************************************
*/

/* Includes */
#include <stddef.h>
#include "stm32l1xx.h"


/* Private typedef */
/* Private define  */
/* Private macro */
/* Private variables */
/* Private function prototypes */
/* Private functions */
void SetSysClock(void);

/**
**===========================================================================
**
**  Abstract: main program
**
**===========================================================================
*/
int main(void)
{
  int i = 0;

  /**
  *  IMPORTANT NOTE!
  *  See the <system_*.c> file and how/if the SystemInit() function updates 
  *  SCB->VTOR register. Sometimes the symbol VECT_TAB_SRAM needs to be defined 
  *  when building the project if code has been located to RAM and interrupts 
  *  are used. Otherwise the interrupt table located in flash will be used.
  *  E.g.  SCB->VTOR = 0x20000000;  
  */

  /* Configure the system clock to 32 MHz and update SystemCoreClock */
  SetSysClock();
  SystemCoreClockUpdate();

  /* TODO - Add your application code here */

  /* Infinite loop */
  while (1)
  {
    i++;
  }
  return 0;
}


/**
  * @brief  Configures the System clock frequency, AHB/APBx prescalers and Flash
  *         settings.
  * @note   This function should be called only once the RCC clock configuration
  *         is reset to the default reset state (done in SystemInit() function).
  * @param  None
  * @retval None
  */
#define HSI_STARTUP_TIMEOUT   ((uint16_t)0x0500) /* Time out for HSI start up */

void SetSysClock(void)
{
  __IO uint32_t StartUpCounter = 0, HSIStatus = 0;

  /* Enable HSI */
  RCC->CR |= ((uint32_t)RCC_CR_HSION);

  /* Wait till HSI is ready and if Time out is reached exit */
  do
  {
    HSIStatus = RCC->CR & RCC_CR_HSIRDY;
  } while((HSIStatus == 0) && (StartUpCounter != HSI_STARTUP_TIMEOUT));

  if ((RCC->CR & RCC_CR_HSIRDY) != RESET)
  {
    HSIStatus = (uint32_t)0x01;
  }
  else
  {
    HSIStatus = (uint32_t)0x00;
  }

  if (HSIStatus == (uint32_t)0x01)
  {
    /*  PLL configuration: PLLCLK = (HSI * 6)/3 = 32 MHz */
    RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLMUL |RCC_CFGR_PLLDIV));
    RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLSRC_HSI | RCC_CFGR_PLLMUL4 | RCC_CFGR_PLLDIV2 );
  }

  else
  {
    /* If HSI fails to start-up, the application will have wrong clock
    configuration. User can add here some code to deal with this error */
  }
  
  /* Enable 64-bit access */
  FLASH->ACR |= FLASH_ACR_ACC64;
    
  /* Enable Prefetch Buffer */
  FLASH->ACR |= FLASH_ACR_PRFTEN;
    
  /* Flash 1 wait state */
  FLASH->ACR |= FLASH_ACR_LATENCY;
    
  /* Power enable */
  RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    
  /* Select the Voltage Range 1 (1.8 V) */
  PWR->CR = PWR_CR_VOS_0;
    
  /* Wait Until the Voltage Regulator is ready */
  while((PWR->CSR & PWR_CSR_VOSF) != RESET)
  {
  }
    
  /* HCLK = SYSCLK /1*/
  RCC->CFGR |= (uint32_t)RCC_CFGR_HPRE_DIV1;
    
  /* PCLK2 = HCLK /1*/
  RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE2_DIV1;
    
  /* PCLK1 = HCLK /1*/
  RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE1_DIV1;
    
  /* Enable PLL */
  RCC->CR |= RCC_CR_PLLON;
    
  /* Wait till PLL is ready */
  while((RCC->CR & RCC_CR_PLLRDY) == 0)
  {
  }
    
  /* Select PLL as system clock source */
  RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
  RCC->CFGR |= (uint32_t)RCC_CFGR_SW_PLL;
    
  /* Wait till PLL is used as system clock source */
  while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)RCC_CFGR_SWS_PLL)
  {
  }
}