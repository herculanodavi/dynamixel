#ifndef DYNAMIXEL_PROTOCOL1_H
#define DYNAMIXEL_PROTOCOL1_H

#include <stdint.h>

#define DXL_HEADER       {0xFF, 0xFF}

// DXL fields length
#define DXL_PACKET_HEADER_LEN 2
#define DXL_PACKET_ID_LEN     1
#define DXL_PACKET_LEN_LEN    1
#define DXL_PACKET_INST_LEN   1
#define DXL_PACKET_ERR_LEN    1
#define DXL_PACKET_CHKSUM_LEN    2

// Packet positions
// -- Instruction packet
#define DXL_INST_PACKET_ID_POS               (DXL_PACKET_HEADER_LEN)
#define DXL_INST_PACKET_LEN_POS              (DXL_INST_PACKET_ID_POS + DXL_PACKET_ID_LEN)
#define DXL_INST_PACKET_INST_POS             (DXL_INST_PACKET_LEN_POS + DXL_PACKET_LEN_LEN)
#define DXL_INST_PACKET_PARAM_POS            (DXL_INST_PACKET_INST_POS + DXL_PACKET_INST_LEN)
#define DXL_INST_PACKET_CHKSUM_POS(param_len)   (DXL_INST_PACKET_PARAM_POS + (param_len))
// -- Status packet
#define DXL_STATUS_PACKET_ID_POS             (DXL_PACKET_HEADER_LEN)
#define DXL_STATUS_PACKET_LEN_POS            (DXL_STATUS_PACKET_ID_POS + DXL_PACKET_ID_LEN)
#define DXL_STATUS_PACKET_INST_POS           (DXL_STATUS_PACKET_LEN_POS + DXL_PACKET_LEN_LEN)
#define DXL_STATUS_PACKET_ERR_POS            (DXL_STATUS_PACKET_INST_POS + DXL_PACKET_INST_LEN)
#define DXL_STATUS_PACKET_PARAM_POS          (DXL_STATUS_PACKET_INST_POS + DXL_PACKET_ERR_LEN)
#define DXL_STATUS_PACKET_CHKSUM_POS(param_len) (DXL_STATUS_PACKET_PARAM_POS + (param_len))

// Packet lengths (not the value in the frame, but the total length of the frame).
#define DXL_INST_PACKET_LEN(param_len) (DXL_INST_PACKET_CHKSUM_POS(param_len) + DXL_PACKET_CHKSUM_LEN)
#define DXL_STATUS_PACKET_LEN(param_len)                                                          \
	(DXL_STATUS_PACKET_CHKSUM_POS(param_len) + DXL_PACKET_CHKSUM_LEN)

typedef enum {
    DXL_INPUT_VOLTAGE_ERROR = 1 << 0,
    DXL_ANGLE_LIMIT_ERROR = 1 << 1,
    DXL_OVERHEATING_ERROR = 1 << 2,
    DXL_RANGE_ERROR = 1 << 3,
    DXL_CHKSUM_ERROR = 1 << 4,
    DXL_OVERLOAD_ERROR = 1 << 5,
    DXL_INSTRUCTION_ERROR = 1 << 6
} dxl_error_code_t;

typedef uint8_t dxl_id_t;

typedef uint8_t dxl_length_t;

#endif