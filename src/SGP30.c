#include "sgp30.h"

#include "crc.h"
#include "i2c.h"
#include "utils.h"

/* SGP30 constants */
const uint8_t Init_air_quality[2]        = {0x20, 0x03};
const uint8_t Measure_air_quality[2]     = {0x20, 0x08};
const uint8_t Get_baseline[2]            = {0x20, 0x15};
const uint8_t Set_baseline[2]            = {0x20, 0x1e};
const uint8_t Set_humidity[2]            = {0x20, 0x61};
const uint8_t Measure_test[2]            = {0x20, 0x32};
const uint8_t Get_feature_set_version[2] = {0x20, 0x2f};
const uint8_t Measure_raw_signals[2]     = {0x20, 0x50};
const uint8_t Get_seiral_id[2]           = {0x36, 0x82};

/* Private function delcaration */
static inline void s_SetAirQuality(sgp30_t *const sgp_data, const uint8_t *const sgp_binary_data) {
    sgp_data->CO2  = ((uint16_t)sgp_binary_data[0] << 8) + (uint16_t)sgp_binary_data[1];
    sgp_data->TVOC = ((uint16_t)sgp_binary_data[3] << 8) + (uint16_t)sgp_binary_data[4];
}

static inline void s_SetBaseline(sgp30_t *const sgp_data, const uint8_t *const sgp_binary_data) {
    sgp_data->baselineCO2  = ((uint16_t)sgp_binary_data[0] << 8) + (uint16_t)sgp_binary_data[1];
    sgp_data->baselineTVOC = ((uint16_t)sgp_binary_data[3] << 8) + (uint16_t)sgp_binary_data[4];
}

static inline void s_SetFeatureSet(sgp30_t *const sgp_data, const uint8_t *const sgp_binary_data) {
    sgp_data->featureSetVersion =
        ((uint16_t)sgp_binary_data[0] << 8) + (uint16_t)sgp_binary_data[1];
}

static inline void s_SetRawData(sgp30_t *const sgp_data, const uint8_t *const sgp_binary_data) {
    sgp_data->H2      = ((uint16_t)sgp_binary_data[0] << 8) + (uint16_t)sgp_binary_data[1];
    sgp_data->ethanol = ((uint16_t)sgp_binary_data[3] << 8) + (uint16_t)sgp_binary_data[4];
}

static inline void s_SetSerialId(sgp30_t *const sgp_data, const uint8_t *const sgp_binary_data) {
    sgp_data->serialID =
        ((uint64_t)sgp_binary_data[0] << 40) + ((uint64_t)sgp_binary_data[1] << 32) +
        ((uint64_t)sgp_binary_data[3] << 24) + ((uint64_t)sgp_binary_data[4] << 16) +
        ((uint64_t)sgp_binary_data[6] << 8) + ((uint64_t)sgp_binary_data[7]);
}

/* Public functions declaration */
/**
 * \author siyuan xu, e2101066@edu.vamk.fi, Jan.2023
 * \brief Create a sgp30 object and initialise it
 * \return Returns a sgp30 object with all fields initialised to 0
 */
sgp30_t sgp30_create() {
    sgp30_t sgp30 = {0, 0, 0, 0, 0, 0, 0, 0};
    return sgp30;
}

/**
 * \author siyuan xu, e2101066@edu.vamk.fi, Jan.2023
 * \brief Start air quality measurement. Initialization period takes about 15s.
 * \return SGP30_SUCCESS
 * \details After InitAirQuality, MeasureAirQuality should be called in regular intervals of 1s.
 * During initialization phase, returns fixed values of 400 ppm CO2eq and 0ppb TVOC.
 */
SGP30ERR sgp30_InitAirQuality() {
    I2C1_init();
    I2C_StartTransmission(SGP30_ADDR);
    I2C_WriteCommand(2, Init_air_quality);
    delay_ms(10);
    I2C_Stop();
    return SGP30_SUCCESS;
}

/**
 * \author siyuan xu, e2101066@edu.vamk.fi, Jan.2023
 * \brief Measure and calculate CO2eq and total VOC(TVOC)
 * \param[out] sgp_data - The memory address where the date would be stored, 6 bytes.
 * \return SGP30_SUCCESS, SGP30_ERR_BAD_CRC
 * \details After InitAirQuality, MeasureAirQuality should be called in regular intervals of 1s.
 * During initialization phase, returns fixed values of 400 ppm CO2eq and 0ppb TVOC. For better
 * accuracy, should SetBaseline according to GetBaseline.
 */
SGP30ERR sgp30_MeasureAirQuality(sgp30_t *const sgp_data) {
    uint8_t crc_co2 = 0, crc_tvoc = 0;
    uint8_t binary_data[6];

    I2C_StartTransmission(SGP30_ADDR);
    I2C_WriteCommand(2, Measure_air_quality);
    delay_ms(12);
    I2C_Read(SGP30_ADDR, 6, binary_data);

    // CRC check
    crc_co2 = CRC8(binary_data, 2, SGP30_CRC8_POLY, SGP30_CRC8_INIT, SGP30_CRC8_XOR);
    if (crc_co2 != binary_data[2]) {
        return SGP30_ERR_BAD_CRC;
    }

    crc_tvoc = CRC8(&binary_data[3], 2, SGP30_CRC8_POLY, SGP30_CRC8_INIT, SGP30_CRC8_XOR);
    if (crc_tvoc != binary_data[5]) {
        return SGP30_ERR_BAD_CRC;
    }

    s_SetAirQuality(sgp_data, binary_data);

    return SGP30_SUCCESS;
}

/**
 * \author siyuan xu, e2101066@edu.vamk.fi, Jan.2023
 * \brief Measure and calculate CO2eq and total VOC(TVOC) baseline.
 * \param[out] sgp_data - The memory address where the date would be stored, 6 bytes.
 * \return SGP30_SUCCESS, SGP30_ERR_BAD_CRC
 * \details For better accuracy, returned data from GetBaseline should be stored and SetBaseline
 * every hour.
 */
SGP30ERR spg30_GetBaseLine(sgp30_t *const sgp_data) {
    uint8_t crc_co2 = 0, crc_tvoc = 0;
    uint8_t binary_data[6];

    I2C_StartTransmission(SGP30_ADDR);
    I2C_WriteCommand(2, Get_baseline);
    delay_ms(10);
    I2C_Read(SGP30_ADDR, 6, binary_data);

    // CRC check
    crc_co2 = CRC8(binary_data, 2, SGP30_CRC8_POLY, SGP30_CRC8_INIT, SGP30_CRC8_XOR);
    if (crc_co2 != binary_data[2]) {
        return SGP30_ERR_BAD_CRC;
    }

    crc_tvoc = CRC8(&binary_data[3], 2, SGP30_CRC8_POLY, SGP30_CRC8_INIT, SGP30_CRC8_XOR);
    if (crc_tvoc != binary_data[5]) {
        return SGP30_ERR_BAD_CRC;
    }

    s_SetBaseline(sgp_data, binary_data);

    return SGP30_SUCCESS;
}

/**
 * \brief Set CO2eq and total VOC(TVOC) baseline for compensation algorithm.
 * \param[in] baseline_eco2 - The baseline value of CO2eq, ppm. max 60000
 * \param[in] baseline_tvoc - The baseline value of TVOC, ppb. max 60000
 * \return SGP30_SUCCESS, SGP30_BAD_BASELINE
 * \author siyuan xu, e2101066@edu.vamk.fi, Jan.2023
 * \details For better accuracy, returned data from GetBaseline should be stored and SetBaseline
 * every hour.
 */
SGP30ERR sgp30_SetBaseline(const uint16_t baseline_eco2, const uint16_t baseline_tvoc) {
    // validate input
    if (baseline_eco2 > 60000 || baseline_tvoc > 60000) {
        return SGP30_BAD_BASELINE;
    }

    uint8_t binary_data[6];
    binary_data[0] = baseline_eco2 >> 8;
    binary_data[1] = baseline_eco2 & 0x0f;
    binary_data[2] = CRC8(binary_data, 2, SGP30_CRC8_POLY, SGP30_CRC8_INIT, SGP30_CRC8_XOR);
    binary_data[3] = baseline_tvoc >> 8;
    binary_data[4] = baseline_tvoc & 0x0f;
    binary_data[5] = CRC8(binary_data + 3, 2, SGP30_CRC8_POLY, SGP30_CRC8_INIT, SGP30_CRC8_XOR);

    I2C_StartTransmission(SGP30_ADDR);
    I2C_WriteCommand(2, Set_baseline);
    I2C_WriteData(6, binary_data);
    delay_ms(10);
    I2C_Stop();

    return SGP30_SUCCESS;
}

/**
 * \author siyuan xu, e2101066@edu.vamk.fi, Jan.2023
 * \brief Set humidity compensation for the air quality signals (CO2eq and TVOC) and sensor raw
 * signals (H2-signal and Ethanol_signal).
 * \param[in] humidity - The absolute humidity of the environment.
 * \return SGP30_SUCCESS, SGP30_BAD_HUMIDITY
 * \details The 2 data bytes represent humidity values as a fixed-point 8.8bit number with a minimum
 * value of 0x0001 (=1/256 g/m3) and a maximum value of 0xFFFF (255 g/m3 + 255/256 g/m3). For
 * instance, sending a value of 0x0F80 corresponds to a humidity value of 16.50 g/m3(16 g/m3 +
 * 128/256 g/m3). After setting a new humidity value, this value will be used by the on-chip
 * humidity compensation algorithm until a new humidity value is set using the Set_humidity command.
 * Restarting the sensor (power-on or soft reset) or sending a value of 0x0000 (= 0 g/m3) sets the
 * humidity value used for compensation to its default value (0x0B92 = 11.57 g/m3) until a new
 * humidity value is sent. Sending a humidity value of 0x0000 can therefore be used to turn off the
 * humidity compensation.
 */
SGP30ERR spg30_SetAbsoluteHumidity(const double humidity) {
    // validate input
    if (humidity > (255 + 255.0 / 256)) {
        return SGP30_BAD_HUMIDITY;
    }
    uint8_t  binary_data[3];
    uint16_t m_humidity = humidity * 100;  // maximum humidity value is 255+255/256. In this case
                                           // uint16_t can contain the converion.
    binary_data[0] = m_humidity >> 8;
    binary_data[1] = (uint8_t)((m_humidity & 0x0f) * 256 / 100);  // ex.
    binary_data[2] = CRC8(binary_data, 2, SGP30_CRC8_POLY, SGP30_CRC8_INIT, SGP30_CRC8_XOR);

    I2C_StartTransmission(SGP30_ADDR);
    I2C_WriteCommand(2, Set_baseline);
    I2C_WriteData(3, binary_data);
    delay_ms(10);
    I2C_Stop();
    return SGP30_SUCCESS;
}

/**
 * \author siyuan xu, e2101066@edu.vamk.fi, Jan.2023
 * \brief The command Measure_test which is included for integration and production line testing
 * runs an on-chip self-test.
 * \return SGP30_SUCCESS, SGP30_ERR_BAD_CRC
 * \details In case of a successful self-test the sensor returns the
 * fixed binary_data pattern 0xD400 (with correct CRC).
 */
SGP30ERR sgp30_MeasureTest() {
    uint8_t crc = 0;
    uint8_t binary_data[3];

    I2C_StartTransmission(SGP30_ADDR);
    I2C_WriteCommand(2, Measure_test);
    delay_ms(220);
    I2C_Read(SGP30_ADDR, 3, binary_data);

    // CRC check
    crc = CRC8(binary_data, 2, SGP30_CRC8_POLY, SGP30_CRC8_INIT, SGP30_CRC8_XOR);
    if (crc != binary_data[2]) {
        return SGP30_ERR_BAD_CRC;
    }

    return SGP30_SUCCESS;
}

/**
 * \author siyuan xu, e2101066@edu.vamk.fi, Jan.2023
 * \brief The SGP30 features a versioning system for the available set of measurement commands and
 * on-chip algorithms.
 * \param[out] sgp_data - The memory address where the date would be stored, 3 bytes.
 * \return SGP30_SUCCESS, SGP30_ERR_BAD_CRC
 * \details The sensor responds with 2 data bytes (MSB first) and 1 CRC byte.
 */
SGP30ERR sgp30_GetFeatureSetVersion(sgp30_t *const sgp_data) {
    uint8_t crc = 0;
    uint8_t binary_data[3];

    I2C_StartTransmission(SGP30_ADDR);
    I2C_WriteCommand(2, Get_feature_set_version);
    delay_ms(2);
    I2C_Read(SGP30_ADDR, 3, binary_data);

    // CRC check
    crc = CRC8(binary_data, 2, SGP30_CRC8_POLY, SGP30_CRC8_INIT, SGP30_CRC8_XOR);
    if (crc != binary_data[2]) {
        return SGP30_ERR_BAD_CRC;
    }

    s_SetFeatureSet(sgp_data, binary_data);

    return SGP30_SUCCESS;
}

/**
 * \author siyuan xu, e2101066@edu.vamk.fi, Jan.2023
 * \brief Returns the sensor raw signals which are used as inputs for the on-chip calibration and
 * baseline compensation algorithms.
 * \param[out] sgp_data - The memory address where the date would be stored, 6 bytes.
 * \return SGP30_SUCCESS, SGP30_ERR_BAD_CRC
 * \details The measurement to which the sensor responds with 2 data bytes (MSB first) and 1 CRC
 * byte. for 2 sensor raw signals in the order H2_signal (sout_H2) and Ethanol_signal (sout_EthOH).
 */
SGP30ERR sgp30_MeasureRawSignals(sgp30_t *const sgp_data) {
    uint8_t crc_h2 = 0, crc_ethanol = 0;
    uint8_t binary_data[6];

    I2C_StartTransmission(SGP30_ADDR);
    I2C_WriteCommand(2, Measure_raw_signals);
    delay_ms(25);
    I2C_Read(SGP30_ADDR, 6, binary_data);

    // CRC check
    crc_h2 = CRC8(binary_data, 2, SGP30_CRC8_POLY, SGP30_CRC8_INIT, SGP30_CRC8_XOR);
    if (crc_h2 != binary_data[2]) {
        return SGP30_ERR_BAD_CRC;
    }

    crc_ethanol = CRC8(&binary_data[3], 2, SGP30_CRC8_POLY, SGP30_CRC8_INIT, SGP30_CRC8_XOR);
    if (crc_ethanol != binary_data[5]) {
        return SGP30_ERR_BAD_CRC;
    }

    s_SetRawData(sgp_data, binary_data);

    return SGP30_SUCCESS;
}

/**
 * \author siyuan xu, e2101066@edu.vamk.fi, Jan.2023
 * \brief The readout of the serial ID register can be used to identify the chip and verify the
 * presence of the sensor.
 * \param[out] sgp_data - The memory address where the date would be stored, 9 bytes.
 * \return SGP30_SUCCESS, SGP30_ERR_BAD_CRC
 * \details The get serial ID command returns 3 words, and every word is followed by an 8-bit CRC
 * checksum. Together the 3 words constitute a unique serial ID with a length of 48 bits. The ID
 * returned with this command are represented in the big endian (or MSB first) format.
 */
SGP30ERR sgp30_GetSerialId(sgp30_t *const sgp_data) {
    uint8_t crc = 0;
    uint8_t binary_data[9];

    I2C_StartTransmission(SGP30_ADDR);
    I2C_WriteCommand(2, Get_seiral_id);
    delay_ms(5);
    I2C_Read(SGP30_ADDR, 9, binary_data);

    // CRC check
    crc = CRC8(binary_data, 2, SGP30_CRC8_POLY, SGP30_CRC8_INIT, SGP30_CRC8_XOR);
    if (crc != binary_data[2]) {
        return SGP30_ERR_BAD_CRC;
    }

    crc = CRC8(&binary_data[3], 2, SGP30_CRC8_POLY, SGP30_CRC8_INIT, SGP30_CRC8_XOR);
    if (crc != binary_data[5]) {
        return SGP30_ERR_BAD_CRC;
    }

    crc = CRC8(&binary_data[6], 2, SGP30_CRC8_POLY, SGP30_CRC8_INIT, SGP30_CRC8_XOR);
    if (crc != binary_data[8]) {
        return SGP30_ERR_BAD_CRC;
    }

    s_SetSerialId(sgp_data, binary_data);

    return SGP30_SUCCESS;
}
