#ifndef CRC_H
#define CRC_H
#include <stddef.h>
#include <stdint.h>

uint8_t  CRC8(const uint8_t *data, const size_t length, const uint8_t polynomial,
              const uint8_t crc_init, const uint8_t final_xor);
uint16_t CRC16(const uint8_t *nData, const uint16_t wLength);

#endif
