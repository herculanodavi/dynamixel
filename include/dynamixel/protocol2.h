#ifndef DYNAMIXEL_PROTOCOL2_H
#define DYNAMIXEL_PROTOCOL2_H

#include <stdint.h>

#define DXL_CRC_POLY     0x8005
#define DXL_CRC_INITIAL  0
#define DXL_HEADER       {0xFF, 0xFF, 0xFD, 0x00}

// DXL fields length
#define DXL_PACKET_HEADER_LEN 4
#define DXL_PACKET_ID_LEN     1
#define DXL_PACKET_LEN_LEN    2
#define DXL_PACKET_INST_LEN   1
#define DXL_PACKET_ERR_LEN    1
#define DXL_PACKET_CRC_LEN    2

// Packet positions
// -- Instruction packet
#define DXL_INST_PACKET_ID_POS               (DXL_PACKET_HEADER_LEN)
#define DXL_INST_PACKET_LEN_POS              (DXL_INST_PACKET_ID_POS + DXL_PACKET_ID_LEN)
#define DXL_INST_PACKET_INST_POS             (DXL_INST_PACKET_LEN_POS + DXL_PACKET_LEN_LEN)
#define DXL_INST_PACKET_PARAM_POS            (DXL_INST_PACKET_INST_POS + DXL_PACKET_INST_LEN)
#define DXL_INST_PACKET_CRC_POS(param_len)   (DXL_INST_PACKET_PARAM_POS + (param_len))
// -- Status packet
#define DXL_STATUS_PACKET_ID_POS             (DXL_PACKET_HEADER_LEN)
#define DXL_STATUS_PACKET_LEN_POS            (DXL_STATUS_PACKET_ID_POS + DXL_PACKET_ID_LEN)
#define DXL_STATUS_PACKET_INST_POS           (DXL_STATUS_PACKET_LEN_POS + DXL_PACKET_LEN_LEN)
#define DXL_STATUS_PACKET_ERR_POS            (DXL_STATUS_PACKET_INST_POS + DXL_PACKET_INST_LEN)
#define DXL_STATUS_PACKET_PARAM_POS          (DXL_STATUS_PACKET_INST_POS + DXL_PACKET_ERR_LEN)
#define DXL_STATUS_PACKET_CRC_POS(param_len) (DXL_STATUS_PACKET_PARAM_POS + (param_len))

// Packet lengths (not the value in the frame, but the total length of the frame).
#define DXL_INST_PACKET_LEN(param_len) (DXL_INST_PACKET_CRC_POS(param_len) + DXL_PACKET_CRC_LEN)
#define DXL_STATUS_PACKET_LEN(param_len)                                                          \
	(DXL_STATUS_PACKET_CRC_POS(param_len) + DXL_PACKET_CRC_LEN)

typedef enum {
    DXL_RESULT_FAIL = 0x01,
    DXL_INST_ERROR = 0x02,
    DXL_CRC_ERROR = 0x03,
    DXL_DATA_RANGE_ERROR = 0x04,
    DXL_DATA_LENGTH_ERROR = 0x05,
    DXL_DATA_LIMIT_ERROR = 0x06,
    DXL_ACCESS_ERROR = 0x07
} dxl_error_code_t;

typedef uint8_t dxl_id_t;

typedef uint16_t dxl_length_t;

#endif