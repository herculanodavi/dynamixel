#ifndef DYNAMIXEL_PROTOCOL_H
#define DYNAMIXEL_PROTOCOL_H

#include <stdint.h>

#define DXL_MAX_NUM_PARAMS (CONFIG_DYNAMIXEL_MAX_NUM_SERVOS * CONFIG_DYNAMIXEL_MAX_NUM_BYTES)


#ifdef CONFIG_DYNAMIXEL_PROTOCOL_1
#define DYNAMIXEL_PROTOCOL_VERSION DXL_PROTOCOL_VERSION_1
#include "dynamixel/protocol1.h"
#else
#ifdef CONFIG_DYNAMIXEL_PROTOCOL_2 
#define DYNAMIXEL_PROTOCOL_VERSION DXL_PROTOCOL_VERSION_2
#include "dynamixel/protocol2.h"
#else
#error "No dynamixel protocol version, select one in Kconfig"
#endif
#endif


typedef enum {
	DXL_PROTOCOL_VERSION_1 = 1,
	DXL_PROTOCOL_VERSION_2 = 2
} dxl_protocol_version_t;

#define DXL_ID_BROADCAST 0xFE

typedef enum {
	DXL_INST_PING = 0x01,
	DXL_INST_READ = 0x02,
	DXL_INST_WRITE = 0x03,
	DXL_INST_REG_WRITE = 0x04,
	DXL_INST_ACTION = 0x05,
	DXL_INST_FACTORY_RESET = 0x06,
	DXL_INST_REBOOT = 0x08,
	DXL_INST_CLEAR = 0x10,
	DXL_INST_CTRL_TABLE_BACKUP = 0x20,
	DXL_INST_STATUS = 0x55,
	DXL_INST_SYNC_READ = 0x82,
	DXL_INST_SYNC_WRITE = 0x83,
	DXL_INST_FAST_SYNC_WRITE = 0x8A,
	DXL_INST_BULK_READ = 0x92,
	DXL_INST_BULK_WRITE = 0x93,
	DXL_INST_FAST_BULK_READ = 0x9A
} dxl_instruction_t;

typedef struct dxl_inst_packet {
	dxl_id_t id;
	dxl_instruction_t instruction;
	uint8_t params[DXL_MAX_NUM_PARAMS];
	dxl_length_t params_length;
	dxl_protocol_version_t protocol_version;
} dxl_inst_packet_t;

typedef struct dxl_status_packet {
	dxl_id_t id;
	dxl_instruction_t instruction;
	dxl_error_code_t error;
	uint8_t params[DXL_MAX_NUM_PARAMS];
	dxl_length_t params_length;
	dxl_protocol_version_t protocol_version;
} dxl_status_packet_t;

#endif