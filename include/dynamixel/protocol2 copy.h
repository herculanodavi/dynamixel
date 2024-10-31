#ifndef DYNAMIXEL_PROTOCOL2_DEFS_H
#define DYNAMIXEL_PROTOCOL2_DEFS_H

#include <stdint.h>

#define DXL2_CRC_POLY     0x8005
#define DXL2_CRC_INITIAL  0
#define DXL2_HEADER       {0xFF, 0xFF, 0xFD, 0x00}

// DXL fields length
#define DXL2_PACKET_HEADER_LEN 4
#define DXL2_PACKET_ID_LEN     1
#define DXL2_PACKET_LEN_LEN    2
#define DXL2_PACKET_INST_LEN   1
#define DXL2_PACKET_ERR_LEN    1
#define DXL2_PACKET_CRC_LEN    2

// Packet positions
// -- Instruction packet
#define DXL2_INST_PACKET_ID_POS               (DXL2_PACKET_HEADER_LEN)
#define DXL2_INST_PACKET_LEN_POS              (DXL2_INST_PACKET_ID_POS + DXL2_PACKET_ID_LEN)
#define DXL2_INST_PACKET_INST_POS             (DXL2_INST_PACKET_LEN_POS + DXL2_PACKET_LEN_LEN)
#define DXL2_INST_PACKET_PARAM_POS            (DXL2_INST_PACKET_INST_POS + DXL2_PACKET_INST_LEN)
#define DXL2_INST_PACKET_CRC_POS(param_len)   (DXL2_INST_PACKET_PARAM_POS + (param_len))
// -- Status packet
#define DXL2_STATUS_PACKET_ID_POS             (DXL2_PACKET_HEADER_LEN)
#define DXL2_STATUS_PACKET_LEN_POS            (DXL2_STATUS_PACKET_ID_POS + DXL2_PACKET_ID_LEN)
#define DXL2_STATUS_PACKET_INST_POS           (DXL2_STATUS_PACKET_LEN_POS + DXL2_PACKET_LEN_LEN)
#define DXL2_STATUS_PACKET_ERR_POS            (DXL2_STATUS_PACKET_INST_POS + DXL2_PACKET_INST_LEN)
#define DXL2_STATUS_PACKET_PARAM_POS          (DXL2_STATUS_PACKET_INST_POS + DXL2_PACKET_ERR_LEN)
#define DXL2_STATUS_PACKET_CRC_POS(param_len) (DXL2_STATUS_PACKET_PARAM_POS + (param_len))

// Packet lengths (not the value in the frame, but the total length of the frame).
#define DXL2_INST_PACKET_LEN(param_len) (DXL2_INST_PACKET_CRC_POS(param_len) + DXL2_PACKET_CRC_LEN)
#define DXL2_STATUS_PACKET_LEN(param_len)                                                          \
	(DXL2_STATUS_PACKET_CRC_POS(param_len) + DXL2_PACKET_CRC_LEN)

typedef enum {
    DXL2_RESULT_FAIL = 0x01,
    DXL2_INST_ERROR = 0x02,
    DXL2_CRC_ERROR = 0x03,
    DXL2_DATA_RANGE_ERROR = 0x04,
    DXL2_DATA_LENGTH_ERROR = 0x05,
    DXL2_DATA_LIMIT_ERROR = 0x06,
    DXL2_ACCESS_ERROR = 0x07
} dxl_error_code_t;

typedef uint8_t dxl_id_t;

typedef uint16_t dxl_length_t;

#endif