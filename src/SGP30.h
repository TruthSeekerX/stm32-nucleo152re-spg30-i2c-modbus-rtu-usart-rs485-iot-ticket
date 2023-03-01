#ifndef SGP30_H
#define SGP30_H
#include <stdint.h>
/* SGP30 I2C addresses */
#define SGP30_ADDR (uint8_t)0x58

/* SGP30 command addresses */
#define I2C_RESET_COMMAND (uint8_t)0x06

/* SGP30 default values*/
#define MEASURE_TEST_OK (uint16_t)0xd400

/* CRC-8 in SGP30 */
#define SGP30_CRC8_POLY (uint8_t)0x31  // x^8 + x^5 + x^4 + 1
#define SGP30_CRC8_INIT (uint8_t)0xff
#define SGP30_CRC8_XOR  (uint8_t)0x00

/* SGP30 data structure */
#define SGP30_MSB 0
#define SGP30_LSB 1

typedef enum { SGP30_SUCCESS = 0, SGP30_ERR_BAD_CRC, SGP30_SELF_TEST_FAIL } SGP30ERR;

typedef struct sgp30_type sgp30_t;

struct sgp30_type {
    uint16_t CO2;
    uint16_t TVOC;
    uint16_t baselineCO2;
    uint16_t baselineTVOC;
    uint16_t featureSetVersion;
    uint16_t H2;
    uint16_t ethanol;
    uint64_t serialID;
};

/* SGP30 function prototypes */
sgp30_t  sgp30_create(void);
SGP30ERR sgp30_InitAirQuality(void);
SGP30ERR sgp30_MeasureAirQuality(sgp30_t *const sgp_data);
SGP30ERR spg30_GetBaseLine(sgp30_t *const sgp_data);
void     sgp30_SetBaseline(const uint16_t baseline_eco2, const uint16_t baseline_tvoc);
void     spg30_SetHumidity(const uint16_t humidity);
SGP30ERR sgp30_MeasureTest(void);
SGP30ERR sgp30_GetFeatureSetVersion(sgp30_t *const sgp_data);
SGP30ERR sgp30_MeasureRawSignals(sgp30_t *const sgp_data);
SGP30ERR sgp30_GetSerialId(sgp30_t *const sgp_data);

#endif
