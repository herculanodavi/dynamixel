#ifndef LIB_DYNAMIXEL_H
#define LIB_DYNAMIXEL_H

#include "stddef.h"
#include "stdint.h"

#define DXL_MAX_NUM_PARAMS (CONFIG_DYNAMIXEL_MAX_NUM_SERVOS * CONFIG_DYNAMIXEL_MAX_NUM_BYTES)

// Error codes
#define DXL_OK 0
#define DXL_ERROR -1
#define DXL_PTR_ERROR -2

typedef int dxl_err_t;

typedef enum {
    DXL2_INST_PING = 0x01,
    DXL2_INST_READ = 0x02,
    DXL2_INST_WRITE = 0x03,
    DXL2_INST_REG_WRITE = 0x04,
    DXL2_INST_ACTION = 0x05,
    DXL2_INST_FACTORY_RESET = 0x06,
    DXL2_INST_REBOOT = 0x08,
    DXL2_INST_CLEAR = 0x10,
    DXL2_INST_CTRL_TABLE_BACKUP = 0x20,
    DXL2_INST_STATUS = 0x55,
    DXL2_INST_SYNC_READ = 0x82,
    DXL2_INST_SYNC_WRITE = 0x83,
    DXL2_INST_FAST_SYNC_WRITE = 0x8A,
    DXL2_INST_BULK_READ = 0x92,
    DXL2_INST_BULK_WRITE = 0x93,
    DXL2_INST_FAST_BULK_READ = 0x9A
} dxl2_instruction_t;

typedef enum {
    DXL2_RESULT_FAIL = 0x01,
    DXL2_INST_ERROR = 0x02,
    DXL2_CRC_ERROR = 0x03,
    DXL2_DATA_RANGE_ERROR = 0x04,
    DXL2_DATA_LENGTH_ERROR = 0x05,
    DXL2_DATA_LIMIT_ERROR = 0x06,
    DXL2_ACCESS_ERROR = 0x07
} dxl2_error_t;

typedef struct dxl2_inst_packet {
    uint8_t id;
    dxl2_instruction_t instruction;
    uint8_t params[DXL_MAX_NUM_PARAMS];
    uint16_t params_length;
} dxl2_inst_packet_t;

typedef struct dxl2_status_packet {
    uint8_t id;
    dxl2_instruction_t instruction;
    dxl2_error_t error;
    uint8_t params[DXL_MAX_NUM_PARAMS];
    uint16_t params_length;
} dxl2_status_packet_t;

/**
 * @brief Writes the content of an instruction packet to a bytes buffer.
 * @param packet: the instruction packet to be serialized.
 * @param buffer: the bytes buffer to write to.
 * @param length: the length of the bytes buffer.
 * @return the number of bytes written to the packet if successful, otherwise
 * returns an error code.
 */
int dxl2_serialize_instruction_packet(
        const dxl2_inst_packet_t *packet,
        uint8_t *buffer,
        size_t length);

/**
 * @brief Fills the instruction packet structure with a sync write of a 2-byte value.
 * @param packet: the instruction packet to be filled.
 * @param address: the memory address to write.
 * @param ids: the ids of the target servos.
 * @param num_ids: number of target servos.
 * @param values: values to write to the servos. Must have <num_ids> length.
 * @return a DXL_* status flag.
 */
dxl_err_t dxl2_make_sync_write_i16(
        dxl2_inst_packet_t *packet,
        uint16_t address,
        uint8_t *ids,
        uint16_t num_ids,
        int16_t *values);

/**
 * @brief Deserializes the bytes buffer to a status packet. The behavior of this
 * function is undefined if the buffer provided is not a status packet.
 * @param packet: a pointer to the packet that will hold the result.
 * @param buffer: the bytes buffer to read.
 * @param length: the length of the bytes buffer.
 * @return a DXL_* status flag.
 */
dxl_err_t dxl2_deserialize_status_packet(
        dxl2_status_packet_t *packet,
        const uint8_t *buffer,
        size_t length);

#endif