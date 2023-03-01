/*
 * usart_config.h
 *
 *  Created on: Feb 24, 2023
 *      Author: Siyuan Xu
 */

#ifndef USART_CONFIG_H_
#define USART_CONFIG_H_

#include <stddef.h>
#include <stdint.h>

#define F_CPU                     32000000UL
#define BAUDRATE                  9600U
#define USART_BRR_VAL             (uint32_t)(F_CPU / BAUDRATE)
#define USART1_RX_DMA_BUFFER_SIZE 8
#define USART2_RX_DMA_BUFFER_SIZE 8

extern uint8_t usart1_rx_dma_buffer[USART1_RX_DMA_BUFFER_SIZE];
extern char    usart2_rx_dma_buffer[USART2_RX_DMA_BUFFER_SIZE];

void USART1_dma_init(void);
void USART1_write(const uint8_t data);
void USART1_RX_Buffer_Reset();
void DMA1_Channel15_Reload(void);
void rs485_send_data(const uint8_t* data, const size_t len);

void USART2_dma_init(void);
char USART2_read(void);
void USART2_write(char data);
void DMA1_Channel16_Reload(void);
void USART2_send_string(const char* string);
void USART2_send_data(const void* data, size_t len);

#endif /* USART_CONFIG_H_ */
