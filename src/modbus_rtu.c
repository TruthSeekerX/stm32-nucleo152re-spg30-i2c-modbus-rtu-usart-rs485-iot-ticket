#include "modbus_rtu.h"

#include <stdlib.h>

#include "crc.h"

/**
 * \brief Create an modbus_rtu_t object
 * \return modbus_rtu_t type struct
 * \author siyuan xu, e2101066@edu.vamk.fi, Jan.2023
 */
modbus_rtu_t modbus_rtu_create(void) {
    modbus_rtu_t modbus_rtu_object = {0, 0, 0, 0, 0};
    return modbus_rtu_object;
}

/**
 * \brief Run a modbus rtu request
 * \param[in] modbus_rtu_frame - Address + PDU + CRC, PDU = Function code + Data
 * \author siyuan xu, e2101066@edu.vamk.fi, Jan.2023
 */
void modbusRtu_RunRequest(const uint8_t *const modbus_rtu_frame, void *data) {
    MODBUS_RTU_ERR err;
    uint8_t        functionCode;

    err = modbusRtu_CrcCheck(modbus_rtu_frame);
    if (err == MODBUS_RTU_ERR_BAD_CRC) {
        debug_console("BAD CRC!\n\r");
        modbusRtu_ErrorReply(modbus_rtu_frame, (uint8_t)err);
        return;
    } else if (err == MODBUS_RTU_SUCCESS) {
        debug_console("CRC SUCCESS!\n\r");
        // Validate Function Code
        functionCode = modbus_rtu_frame[FUNCTION_CODE];
        err          = modbusRtu_FunctionCodeValidation(functionCode);
        if (MODBUS_RTU_SUCCESS != err) {
            debug_console("BAD FUNCTION CODE!\n\r");
            modbusRtu_ErrorReply(modbus_rtu_frame, (uint8_t)err);
            return;
        } else {
            debug_console("FUNCTION CODE Accepted!\n\r");
            switch (modbus_rtu_frame[FUNCTION_CODE]) {
                case READ_DO:
                    // TBD
                    break;
                case READ_DI:
                    // TBD
                    break;
                case READ_AO:
                    // TBD
                    break;
                case READ_AI:
                    // TBD
                    err = modbusRtu_ReadInputRegister(modbus_rtu_frame, data);
                    break;
                case WRITE_ONE_DO:
                    // TBD
                    break;
                case WRITE_ONE_AO:
                    // TBD
                    break;
                default:
                    break;
            }
            if (MODBUS_RTU_SUCCESS != err) {
                modbusRtu_ErrorReply(modbus_rtu_frame, (uint8_t)err);
            } else {
                debug_console("Modbus RTU request execution successful!\n\r");
            }
        }
    }
}

/**
 * \brief Validate the slave address in request frame
 * \param[in] address - the slave address in request frame
 * \return MODBUS_RTU_SUCCESS when success, MODBUS_RTU_ERR_BAD_SLAVE_ADDR when failed
 * \author siyuan xu, e2101066@edu.vamk.fi, Jan.2023
 */
MODBUS_RTU_ERR modbusRtu_AddressValidation(const uint8_t address) {
    if (address != MODBUS_RTU_SLAVE_ADDR_THIS) {
        return MODBUS_RTU_ERR_BAD_SLAVE_ADDR;
    } else {
        return MODBUS_RTU_SUCCESS;
    }
}

/**
 * \brief Reply to Modbus RTU Master when exception occures
 * \param[in] modbus_rtu_frame - Address + PDU + CRC, PDU = Function code + Data
 * \param[in] modbus_exception_code - MODBUS_RTU_ERR code
 * \author siyuan xu, e2101066@edu.vamk.fi, Jan.2023
 */
void modbusRtu_ErrorReply(const uint8_t *const modbus_rtu_frame,
				MODBUS_RTU_ERR        modbus_exception_code) {
    uint8_t  modbus_reply_frame[MODBUS_FRAME_ERROR_REPLY_LENGTH];
    uint16_t crc                                 = 0;
    modbus_reply_frame[SLAVE_ADDRESS]            = modbus_rtu_frame[SLAVE_ADDRESS];
    modbus_reply_frame[FUNCTION_CODE]            = modbus_rtu_frame[FUNCTION_CODE] + 0x80;
    modbus_reply_frame[ERROR_REPLY_DATA]         = modbus_exception_code;
    crc                                          = CRC16(modbus_reply_frame, 3);
    modbus_reply_frame[ERROR_REPLY_CHECKSUM_HI]  = (uint8_t)(crc >> 8);
    modbus_reply_frame[ERROR_REPLY_CHECKSUM_LOW] = (uint8_t)(crc & 0xff);
    modbusRtu_SendData(modbus_reply_frame, MODBUS_FRAME_ERROR_REPLY_LENGTH);
}

/**
 * \brief Normal reply to Modbus RTU Master
 * \param[in] modbus_rtu_frame - Address + PDU + CRC, PDU = Function code + Data
 * \param[in] data - The address of the requested data (coil, discrete input, register...)
 * \author siyuan xu, e2101066@edu.vamk.fi, Jan.2023
 */
void modbusRtu_Reply(const uint8_t *const modbus_rtu_frame, const uint8_t *data) {
    uint8_t  modbus_reply_frame[MODBUS_FRAME_REPLY_LENGTH];
    uint16_t crc                           = 0;
    modbus_reply_frame[SLAVE_ADDRESS]      = modbus_rtu_frame[SLAVE_ADDRESS];
    modbus_reply_frame[FUNCTION_CODE]      = modbus_rtu_frame[FUNCTION_CODE];
    modbus_reply_frame[REPLY_BYTE_COUNT]   = 2;
    modbus_reply_frame[REPLY_DATA_HI]      = data[0];
    modbus_reply_frame[REPLY_DATA_LOW]     = data[1];
    crc                                    = CRC16(modbus_reply_frame, 3);
    modbus_reply_frame[REPLY_CHECKSUM_HI]  = (uint8_t)(crc >> 8);
    modbus_reply_frame[REPLY_CHECKSUM_LOW] = (uint8_t)(crc & 0xff);
    modbusRtu_SendData(modbus_reply_frame, MODBUS_FRAME_REPLY_LENGTH);
}

/**
 * \brief CRC-16 error check for the Modbus RTU reqeust frame
 * \param[in] modbus_rtu_frame - Address + PDU + CRC, PDU = Function code + Data
 * \return MODBUS_RTU_SUCCESS when success, MODBUS_RTU_ERR_BAD_CRC when failed
 * \author siyuan xu, e2101066@edu.vamk.fi, Jan.2023
 */
MODBUS_RTU_ERR modbusRtu_CrcCheck(const uint8_t *const modbus_rtu_frame) {
    uint16_t crc_checksum = 0;
    uint16_t crc_calcuate = 0;
    char    *crc_str[10];
    crc_calcuate = CRC16(modbus_rtu_frame, 6);
    debug_console("CRC_CAL=");
    debug_console(itoa(crc_calcuate, crc_str, 10));

    crc_checksum =
        ((uint16_t)modbus_rtu_frame[CHECKSUM_HI] << 8) | (uint16_t)modbus_rtu_frame[CHECKSUM_LOW];

    if (crc_calcuate != crc_checksum) {
        return MODBUS_RTU_ERR_BAD_CRC;
    }

    return MODBUS_RTU_SUCCESS;
}

/**
 * \brief Validate the function code from the Modbus RTU reqeust frame
 * \param[in] function_code - The function code
 * \return MODBUS_RTU_SUCCESS when success, MODBUS_RTU_ERR_BAD_FUNCTION_CODE when failed
 * \author siyuan xu, e2101066@edu.vamk.fi, Jan.2023
 */
MODBUS_RTU_ERR modbusRtu_FunctionCodeValidation(const uint8_t function_code) {
    if ((function_code < READ_DO) || (function_code > WRITE_ONE_AO)) {
        return MODBUS_RTU_ERR_BAD_FUNCTION_CODE;
    } else {
        return MODBUS_RTU_SUCCESS;
    }
}

/**
 * \brief Validate the register address from the Modbus RTU reqeust frame
 * \param[in] reg_addr - The register address
 * \return MODBUS_RTU_SUCCESS when success, MODBUS_RTU_ERR_BAD_REGISTER_ADDR when failed
 * \author siyuan xu, e2101066@edu.vamk.fi, Jan.2023
 */
MODBUS_RTU_ERR modbusRtu_RegisterAddressValidation(const uint16_t reg_addr) {
    if ((reg_addr < REG_ADDR_CO2) || (reg_addr > REG_ADDR_SERIAL_ID)) {
        return MODBUS_RTU_ERR_BAD_REGISTER_ADDR;
    } else {
        return MODBUS_RTU_SUCCESS;
    }
}
