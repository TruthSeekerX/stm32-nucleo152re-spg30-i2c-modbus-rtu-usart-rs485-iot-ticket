#include "i2c.h"

/**
 * \brief Initialize I2C1
 * \author Jani Ahvonen
 */
void I2C1_init(void) {
    RCC->AHBENR  |= 2;          // Enable GPIOB clock PB8(D15)=SCL,PB9(D14)=SDA.
    RCC->APB1ENR |= (1 << 21);  // Enable I2C1_EN clock

    // configures PB8,PB9 to I2C1_EN
    GPIOB->AFR[1] &= ~0x000000FF;  // PB8,PB9 I2C1 SCL, SDA. AFRH8 and AFRH9. clear
    GPIOB->AFR[1] |= 0x00000044;   // GPIOx_AFRL p.189,AF4=I2C1(0100 BIN) p.177
    GPIOB->MODER  &= ~0x000F0000;  // PB8 and PB9 clear
    GPIOB->MODER  |= 0x000A0000;   // Alternate function mode PB8,PB9
    GPIOB->OTYPER |= 0x00000300;   // output open-drain. p.184
    GPIOB->PUPDR  &= ~0x000F0000;  // no pull-up resistors for PB8 and PB9 p.185

    I2C1->CR1 = 0x8000;    // software reset I2C1 SWRST p.682
    I2C1->CR1 &= ~0x8000;  // stop reset
    I2C1->CR2 = 0x0020;    // peripheral clock 32 MHz

    /*how to calculate CCR
    TPCLK1=1/32MHz=31,25ns
    tI2C_bus=1/100kHz=10us=10000ns
    tI2C_bus_div2=10000ns/2=5000ns
    CCR value=tI2C_bus_div2/TPCLK1=5000ns/31,25ns=160
    p. 692*/
    I2C1->CCR = 160;

    // maximum rise time in sm mode = 1000ns. Equation 1000 ns/TPCK1
    I2C1->TRISE = 33;       // 1000ns/31,25ns=32, p.693
    I2C1->CR1   |= 0x0001;  // eripheral enable (I2C1)
}

/**
 * \brief Generate a I2C Start signal S
 * \author siyuan xu, e2101066@edu.vamk.fi, Jan.2023
 */
static inline void I2C_Start(void) {
    while (I2C1->SR2 & 2) {
    }                     // wait until bus not busy
    I2C1->CR1 &= ~0x800;  // Acknowledge clear p.682

    I2C1->CR1 |= 0x100;  // generate start p.694
    while (!(I2C1->SR1 & 1)) {
    }  // wait until start condition generated
}

/**
 * \brief Generate a I2C Stop signal P
 * \author siyuan xu, e2101066@edu.vamk.fi, Jan.2023
 */
void I2C_Stop(void) {
    I2C1->CR1 |= (1 << 9);  // generate stop
}

/**
 * \brief Generate a I2C stop signal XCK
 * \author siyuan xu, e2101066@edu.vamk.fi, Jan.2023
 */
static inline void I2C_DisableACK(void) {
    I2C1->CR1 &= ~(1 << 10);  // disable acknowledge p.682
}

/**
 * \brief Start IC2 Transmission
 * \param[in] address - I2C address of the sensor
 * \author siyuan xu, e2101066@edu.vamk.fi, Jan.2023
 */
void I2C_StartTransmission(const uint8_t address) {
    volatile int tmp;

    I2C_Start();
    I2C1->DR = address << 1;  // transmit slave address
    while (!(I2C1->SR1 & 2)) {
    }  // wait until end of address transmission p.690

    tmp = I2C1->SR2;  // Reading I2C_SR2 after reading I2C_SR1 clears the ADDR flag p691
    while (!(I2C1->SR1 & 0x80)) {
    }  // wait until data register empty p.689
}

/**
 * \brief Send a IC2 Write Request
 * \param[in] length - The number of bytes in command
 * \param[in] command - The address of the command to be send
 * \author siyuan xu, e2101066@edu.vamk.fi, Jan.2023
 */
void I2C_WriteCommand(const size_t length, const uint8_t *command) {
    for (size_t i = 0; i < length; i++) {
        I2C1->DR = *command++;  // send data
        while (!(I2C1->SR1 & 0x80)) {
        }  // wait until data register empty p.689
    }
}

/**
 * \brief Send a IC2 Write Request
 * \param[in] data_length - The number of bytes in data
 * \param[in] data - The address of the data to be send, ex. can be a command or actual data
 * \author siyuan xu, e2101066@edu.vamk.fi, Jan.2023
 */
void I2C_WriteData(const size_t data_length, const uint8_t *data) {
    for (size_t i = 0; i < data_length; i++) {
        I2C1->DR = *data++;  // send data
        while (!(I2C1->SR1 & 0x80)) {
        }  // wait until data register empty p.689
    }

    while (!(I2C1->SR1 & 4)) {
    }  // wait until byte transfer finished p.690
}

/**
 * \brief Send a IC2 Read Request
 * \param[in] address - I2C address of the sensor
 * \param[in] data_length - The number of bytes in data (3 by default Data_MSB, Data_LSB, Data_CRC)
 * \param[out] data - The address where the returned data would be stored
 * \author siyuan xu, e2101066@edu.vamk.fi, Jan.2023
 */
void I2C_Read(const uint8_t address, const size_t data_length, uint8_t *data) {
    volatile int tmp;

    I2C1->CR1 |= 0x100;  // generate repeated start p.694
    while (!(I2C1->SR1 & 1)) {
    }                             // wait until start condition generated
    I2C1->DR = address << 1 | 1;  // transmit slave address
    while (!(I2C1->SR1 & 2)) {
    }  // wait until end of address transmission p.690

    tmp       = I2C1->SR2;   // Reading I2C_SR2 after reading I2C_SR1 clears the ADDR flag p691
    I2C1->CR1 |= (1 << 10);  // Enable acknowledge p.683

    for (size_t i = 0; i < data_length; i++)  // read data from chip
    {
        while (!(I2C1->SR1 & 0x40)) {
        }                      // wait until RXNE flag is set
        (*data++) = I2C1->DR;  // read data from DR
    }
    I2C_Stop();
    I2C_DisableACK();
}
