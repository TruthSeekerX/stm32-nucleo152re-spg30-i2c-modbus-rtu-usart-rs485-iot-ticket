/*
 * iwdg.c
 *
 *  Created on: Feb 28, 2023
 *      Author: Siyuan Xu
 */
#include "stm32l1xx.h"

/**
 * \brief           Initialize Independent Watchdog
 */
void IWDG_init(void){
	IWDG->KR = 0x5555;		/*!< Enable access to IWDG_PR and IWDG_RLR */
	IWDG->PR |= 0x06;		/*!< Set prescaler to LSI(37KHz) / 256 = 6.918ms */
	while(!(IWDG->SR & IWDG_SR_PVU)){}	/*!< Wait for prescaler update ready */

	IWDG->RLR = 433 - 1;	/*!< Set 433 ticks to Reload register, about 3 seconds by current prescaler */
	while(!(IWDG->SR & IWDG_SR_RVU)){} /*!< Wait for reload register update ready */

	IWDG->KR = 0xCCCC;		/*!< Starts the watchdog */
}

/**
 * \brief           Feeding(Reset) the watchdog
 */
void IWDG_feed(void){
	IWDG->KR = 0xAAAA;
}
