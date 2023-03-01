#ifndef I2C_H
#define I2C_H
#include <stdint.h>
#include <stddef.h>
#include "stm32l1xx.h"

void I2C_Stop(void);
void I2C_StartTransmission(const uint8_t address);
void I2C_WriteCommand(const size_t length, const uint8_t *command);
void I2C_WriteData(const size_t length, const uint8_t *command);
void I2C_Read(const uint8_t address, const size_t data_length, uint8_t *data);

#endif
