#include "usart_config.h"

#include <string.h>

#include "stm32l1xx.h"
#include "utils.h"

/*
 * usart_config.c
 *
 *  Created on: Feb 24, 2023
 *      Author: Siyuan Xu
 */

/**
 * \brief           Initialize USART1 with DMA in normal mode on RX
 */
void USART1_dma_init(void) {
    /*
     * USART1 GPIO and DMA configuration
     *
     * PA9/D8   ------> USART1_TX
     * PA10/D2  ------> USART1_RX
     * PA7/D11	------> TX_EN
     * USART2_RX --> DMA1_channel_5
     */

    // ref. manual p.260
    RCC->AHBENR        |= RCC_AHBENR_DMA1EN;  // DMA1 clock enable
    DMA1_Channel5->CCR &= ~(DMA_CCR_DIR |     /*!< Data transfer direction 0 = p->m, 1 = m->p */
                            DMA_CCR_PINC |    /*!< Peripheral increment mode */
                            DMA_CCR_PSIZE |   /*!< PSIZE[1:0] bits (Peripheral size) 0 = 8-bits */
                            DMA_CCR_MSIZE |   /*!< MSIZE[1:0] bits (Memory size) 0 = 8-bits */
                            DMA_CCR_PL |      /*!< PL[1:0] bits(Channel Priority level) 0 = low */
                            DMA_CCR_CIRC |    /*!< Disable Circular mode */
                            DMA_CCR_MEM2MEM); /*!< Memory to memory mode disable */

    DMA1_Channel5->CCR |= (DMA_CCR_HTIE | /*!< Half Transfer interrupt enable */
                           DMA_CCR_TCIE | /*!< Transfer complete interrupt enable */
                           DMA_CCR_MINC); /*!< Memory increment mode */

    DMA1_Channel5->CPAR  = (uint32_t) & (USART1->DR); /*!< set peripheral address as USART1-DR */
    DMA1_Channel5->CMAR  = (uint32_t)usart1_rx_dma_buffer;      /*!< Set buffer address */
    DMA1_Channel5->CNDTR = (uint16_t)USART1_RX_DMA_BUFFER_SIZE; /*!< Set data length */

    /* DMA interrupt init */
    NVIC_SetPriority(DMA1_Channel5_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    NVIC_EnableIRQ(DMA1_Channel5_IRQn);

    /* USART configuration */
    // ref. manual p.247
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;  // set bit 14 (USART1 clock EN)
    RCC->AHBENR  |= RCC_AHBENR_GPIOAEN;    // enable GPIOA port clock bit 0 (GPIOA EN)
    GPIOA->AFR[1] |=
        (0x07 << GPIO_AFRH_AFRH1_Pos);  // GPIOA_AFRH9 as AF7 0b111 = 0x07 p.188,AF7 p.177
    GPIOA->AFR[1] |=
        (0x07 << GPIO_AFRH_AFRH2_Pos);  // GPIOA_AFRH10 as AF7 0b111 = 0x07 p.188,AF7 p.177
    GPIOA->MODER |=
        (0x02
         << GPIO_MODER_MODER9_Pos);  // MODER2=PA9(TX) to mode 0b10=alternate function mode. p184
    GPIOA->MODER |=
        (0x02
         << GPIO_MODER_MODER10_Pos);  // MODER3=PA10(RX) to mode 0b10=alternate function mode. p184
    GPIOA->MODER |= (0x01 << GPIO_MODER_MODER7_Pos);  // PA7 as output mode 01=digital output
    GPIOA->PUPDR |= (0x02 << GPIO_PUPDR_PUPDR7_Pos);  // set connect PA7 with pulldown 0b10=pulldown
    GPIOA->ODR   &= ~GPIO_ODR_ODR_7;                  // Disable TX and Enable RX

    USART1->BRR = USART_BRR_VAL;      // 9600 BAUD and crystal 32MHz. p710, D05
    USART1->CR1 |= USART_CR1_TE;      // TE bit. p739-740. Enable transmit
    USART1->CR1 |= USART_CR1_RE;      // RE bit. p739-740. Enable receiver
    USART1->CR3 |= USART_CR3_DMAR;    /*!< DMA Enable Receiver */
    USART1->CR1 |= USART_CR1_IDLEIE;  // Enable idle line detection interrupt

    /* USART interrupt */
    NVIC_SetPriority(USART1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    NVIC_EnableIRQ(USART1_IRQn);

    /* Enable USART and DMA */
    DMA1_Channel5->CCR |= DMA_CCR_EN;    /*!< Channel enable*/
    USART1->CR1        |= USART_CR1_UE;  // UE bit. p739-740. Uart enable
}

/**
 * \brief           Enable TX will disable ~RX
 */
static inline void RS485_TX_Enable() { GPIOA->ODR |= GPIO_ODR_ODR_7; }

/**
 * \brief           Disable TX will enable ~RX
 */
static inline void RS485_TX_Disable() { GPIOA->ODR &= ~GPIO_ODR_ODR_7; }

/**
 * \brief           Send a byte to USART1
 * \param[in]       data: the byte to send
 */
void USART1_write(const uint8_t data) {
    // wait while TX buffer is empty
    while (!(USART1->SR & 0x0080)) {
    }                     // TXE: Transmit data register empty. p736-737
    USART1->DR = (data);  // p739
}

/* Interrupt handlers here */

/**
 * \brief           DMA1 channel5 interrupt handler for USART1 RX
 */
// void DMA1_Channel5_IRQHandler(void) {
//     /* Check half-transfer complete interrupt */
//     if (DMA1->ISR & DMA_ISR_HTIF5) {
//         USART2_send_string("USART1 DMA half-transfer interrupt!\r\n");
//         DMA1->IFCR |= DMA_IFCR_CHTIF5; /*!< Channel 5 Half Transfer clear */
//     }

//     /* Check transfer-complete interrupt */
//     if (DMA1->ISR & DMA_ISR_TCIF5) {
//         USART2_send_string("USART1 DMA transfer-complete interrupt!\r\n");
//         DMA1->IFCR |= DMA_IFCR_CTCIF5; /*!< Channel 5 Transfer Complete clear */
//         rs485_send_data(usart1_rx_dma_buffer, USART1_RX_DMA_BUFFER_SIZE);
//         //		USART2_send_data(usart1_rx_dma_buffer, USART1_RX_DMA_BUFFER_SIZE);
//         //		DMA1_Channel15_Reset();
//         //		USART1_RX_Buffer_Reset();
//     }
// }

/**
 * \brief           USART1 global interrupt handler
 */
// void USART1_IRQHandler(void) {
//     uint32_t status = USART1->SR;
//     uint8_t  data;
//     /* Check for IDLE line interrupt */
//     if (status & USART_SR_IDLE) {
//         USART2_send_string("USART1 Idle-line interrupt!\r\n");
//         data = USART1->DR; /* Clear IDLE line flag */
//         DMA1_Channel15_Reload();
//         USART1_RX_Buffer_Reset();
//     }
// }

/**
 * \brief           Reset USART1_rx buffer
 */
void USART1_RX_Buffer_Reset(void) { memset(usart1_rx_dma_buffer, 0, USART1_RX_DMA_BUFFER_SIZE); }

/**
 * \brief           Reload and re-enable DMA1_channel5
 */
void DMA1_Channel15_Reload(void) {
    USART2_send_string("Reset DMA1_Channel5_CNDTR!\r\n");
    DMA1_Channel5->CCR   &= ~DMA_CCR_EN;                        /*!< Channel disable*/
    DMA1_Channel5->CNDTR = (uint16_t)USART1_RX_DMA_BUFFER_SIZE; /*!< Set data length */
    DMA1_Channel5->CCR   |= DMA_CCR_EN;                         /*!< Channel enable*/
}

/**
 * \brief           Send string to USART1
 * \param[in]       str: String to send
 */
void rs485_send_data(const uint8_t *data, const size_t len) {
    RS485_TX_Enable();
    for (size_t i = 0; i < len; i++) {
        USART1_write(data[i]);
    }
    while (!(USART2->SR & USART_SR_TC)) {
    }             /*!< Transmission Complete */
    delay_ms(3);  // measured with oscilloscope, it can be a bit shorter
    RS485_TX_Disable();
}

/**
 * \brief           Initialize USART2 with DMA in normal mode on RX
 */
void USART2_dma_init(void) {
    /*
     * USART2 GPIO Configuration
     *
     * PA2   ------> USART2_TX
     * PA3   ------> USART2_RX
     * USART2_RX --> DMA1_channel_6
     */

    // ref. manual p.260
    RCC->AHBENR        |= RCC_AHBENR_DMA1EN;  // DMA1 clock enable
    DMA1_Channel6->CCR &= ~(DMA_CCR_DIR |     /*!< Data transfer direction 0 = p->m, 1 = m->p */
                            DMA_CCR_PINC |    /*!< Peripheral increment mode */
                            DMA_CCR_PSIZE |   /*!< PSIZE[1:0] bits (Peripheral size) 0 = 8-bits */
                            DMA_CCR_MSIZE |   /*!< MSIZE[1:0] bits (Memory size) 0 = 8-bits */
                            DMA_CCR_PL |      /*!< PL[1:0] bits(Channel Priority level) 0 = low */
                            DMA_CCR_CIRC |    /*!< Circular mode */
                            DMA_CCR_MEM2MEM); /*!< Memory to memory mode disable */

    DMA1_Channel6->CCR |= (DMA_CCR_HTIE | /*!< Half Transfer interrupt enable */
                           DMA_CCR_TCIE | /*!< Transfer complete interrupt enable */
                           DMA_CCR_MINC); /*!< Memory increment mode */

    DMA1_Channel6->CPAR  = (uint32_t) & (USART2->DR); /*!< set peripheral address as USART2-DR */
    DMA1_Channel6->CMAR  = (uint32_t)usart2_rx_dma_buffer;      /*!< Set buffer address */
    DMA1_Channel6->CNDTR = (uint16_t)USART2_RX_DMA_BUFFER_SIZE; /*!< Set data length */

    /* DMA interrupt init */
    NVIC_SetPriority(DMA1_Channel6_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    NVIC_EnableIRQ(DMA1_Channel6_IRQn);

    /* USART configuration */
    // ref. manual p.247
    RCC->APB1ENR  |= RCC_APB1ENR_USART2EN;  // set bit 17 (USART2 EN)
    RCC->AHBENR   |= RCC_AHBENR_GPIOAEN;    // enable GPIOA port clock bit 0 (GPIOA EN)
    GPIOA->AFR[0] |= 0x00000700;            // GPIOx_AFRL p.188,AF7 p.177
    GPIOA->AFR[0] |= 0x00007000;            // GPIOx_AFRL p.188,AF7 p.177
    GPIOA->MODER  |= 0x00000020;  // MODER2=PA2(TX) to mode 10=alternate function mode. p184
    GPIOA->MODER  |= 0x00000080;  // MODER3=PA3(RX) to mode 10=alternate function mode. p184

    USART2->BRR = USART_BRR_VAL;      // 9600 BAUD and crystal 32MHz. p710, D05
    USART2->CR1 |= USART_CR1_TE;      // TE bit. p739-740. Enable transmit
    USART2->CR1 |= USART_CR1_RE;      // RE bit. p739-740. Enable receiver
    USART2->CR3 |= USART_CR3_DMAR;    /*!< DMA Enable Receiver */
    USART2->CR1 |= USART_CR1_IDLEIE;  // Enable idle line detection interrupt

    /* USART interrupt */
    NVIC_SetPriority(USART2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    NVIC_EnableIRQ(USART2_IRQn);

    /* Enable USART and DMA */
    DMA1_Channel6->CCR |= DMA_CCR_EN;    /*!< Channel enable*/
    USART2->CR1        |= USART_CR1_UE;  // UE bit. p739-740. Uart enable
}

/**
 * \brief           Send a character to USART2
 * \param[in]       data: the character to send
 */
void USART2_write(char data) {
    // wait while TX buffer is empty

    while (!(USART2->SR & 0x0080)) {
    }                     // TXE: Transmit data register empty. p736-737
    USART2->DR = (data);  // p739
}

/**
 * \brief           Send a string to USART2
 * \param[in]       string: the string to send
 */
void USART2_send_string(const char *string) {
    size_t len = strlen(string);
    for (; len > 0; len--, string++) {
        USART2_write(*string);
    }
}

/**
 * \brief           Send an array of bytes to USART2
 * \param[in]       data: the data to send
 * \param[in]       len:  the length of the array
 */
void USART2_send_data(const void *data, size_t len) {
    char *d = (char *)data;
    for (; len > 0; len--, d++) {
        USART2_write(*d);
    }
}

/**
 * \brief           Send an array of bytes to USART2
 * \return          One character from USART2
 */
char USART2_read() {
    char data = 0;
    // wait while RX buffer is data is ready to be read
    while (!(USART2->SR & 0x0020)) {
    }                   // Bit 5 RXNE: Read data register not empty
    data = USART2->DR;  // p739
    return data;
}

/**
 * \brief           Rest USART2 RX buffer
 */
void USART2_RX_Buffer_Reset() { memset(usart2_rx_dma_buffer, 0, USART2_RX_DMA_BUFFER_SIZE); }

/**
 * \brief           Reload and re-enable DMA1_channel5
 */
void DMA1_Channel16_Reload() {
    USART2_send_string("Reset DMA1_Channel6_CNDTR!\r\n");
    DMA1_Channel6->CCR   &= ~DMA_CCR_EN;                        /*!< Channel disable*/
    DMA1_Channel6->CNDTR = (uint16_t)USART2_RX_DMA_BUFFER_SIZE; /*!< Set data length */
    DMA1_Channel6->CCR   |= DMA_CCR_EN;                         /*!< Channel enable*/
}
