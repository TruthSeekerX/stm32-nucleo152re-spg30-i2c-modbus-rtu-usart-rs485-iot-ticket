#include "I2C.h"
/**
 * \brief Generate a I2C Start signal S
 * \return no return
 * \author siyuan xu, e2101066@edu.vamk.fi, Jan.2023
 */
static inline void I2C_Start(void) {
    while (I2C1->SR2 & 2) {}    // wait until bus not busy
    I2C1->CR1 &= ~0x800;  // Acknowledge clear p.682

    I2C1->CR1 |= 0x100;  // generate start p.694
    while (!(I2C1->SR1 & 1)) {
    }  // wait until start condition generated
}

/**
 * \brief Generate a I2C Stop signal P
 * \return no return
 * \author siyuan xu, e2101066@edu.vamk.fi, Jan.2023
 */
void I2C_Stop(void) {
    I2C1->CR1 |= (1 << 9);  // generate stop
}

/**
 * \brief Generate a I2C stop signal XCK
 * \return no return
 * \author siyuan xu, e2101066@edu.vamk.fi, Jan.2023
 */
static inline void I2C_DisableACK(void) {
    I2C1->CR1 &= ~(1 << 10);  // disable acknowledge p.682
}

/**
 * \brief Start IC2 Transmission
 * \param[in] address - I2C address of the sensor
 * \return no return
 * \author siyuan xu, e2101066@edu.vamk.fi, Jan.2023
 */
void I2C_StartTransmission(const uint8_t address) {
    volatile int tmp;

    I2C_Start();
    I2C1->DR = address << 1;  // transmit slave address
    while (!(I2C1->SR1 & 2)) {} // wait until end of address transmission p.690

    tmp = I2C1->SR2;  // Reading I2C_SR2 after reading I2C_SR1 clears the ADDR flag p691
    while (!(I2C1->SR1 & 0x80)) {}  // wait until data register empty p.689
}

/**
 * \brief Send a IC2 Write Request
 * \param[in] length - The number of bytes in command
 * \param[in] command - The address of the command to be send
 * \return no return
 * \author siyuan xu, e2101066@edu.vamk.fi, Jan.2023
 */
void I2C_WriteCommand(const size_t length, const uint8_t *command) {
    for (size_t i = 0; i < length; i++) {
        I2C1->DR = *command++;  // send data
        while (!(I2C1->SR1 & 0x80)) {}  // wait until data register empty p.689
    }
}

/**
 * \brief Send a IC2 Write Request
 * \param[in] data_length - The number of bytes in data
 * \param[in] data - The address of the data to be send, ex. can be a command or actual data
 * \return no return
 * \author siyuan xu, e2101066@edu.vamk.fi, Jan.2023
 */
void I2C_WriteData(const size_t data_length, const uint8_t *data) {
    for (size_t i = 0; i < data_length; i++) {
        I2C1->DR = *data++;  // send data
        while (!(I2C1->SR1 & 0x80)) {}  // wait until data register empty p.689
    }

    while (!(I2C1->SR1 & 4)) {}  // wait until byte transfer finished p.690
}

/**
 * \brief Send a IC2 Read Request
 * \param[in] address - I2C address of the sensor
 * \param[in] data_length - The number of bytes in data (3 by default Data_MSB, Data_LSB, Data_CRC)
 * \param[out] data - The address where the returned data would be stored
 * \return no return
 * \author siyuan xu, e2101066@edu.vamk.fi, Jan.2023
 */
void I2C_Read(const uint8_t address, const size_t data_length, uint8_t *data) {
    volatile int tmp;

    I2C1->CR1 |= 0x100;  // generate repeated start p.694
    while (!(I2C1->SR1 & 1)) {}     // wait until start condition generated
    I2C1->DR = address << 1 | 1;    // transmit slave address
    while (!(I2C1->SR1 & 2)) {}     // wait until end of address transmission p.690

    tmp = I2C1->SR2;         // Reading I2C_SR2 after reading I2C_SR1 clears the ADDR flag p691
    I2C1->CR1 |= (1 << 10);  // Enable acknowledge p.683

    for (size_t i = 0; i < data_length; i++)  // read data from chip
    {
        while (!(I2C1->SR1 & 0x40)) {}  // wait until RXNE flag is set
        (*data++) = I2C1->DR;           // read data from DR
    }
    I2C_Stop();
    I2C_DisableACK();
}
