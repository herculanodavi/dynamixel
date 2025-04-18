#include "dynamixel/instruction_packet.h"

#include <stddef.h>

#include "dynamixel/protocol.h"

dxl_err_t dxl_make_sync_write_i16(dxl_inst_packet_t *packet, uint16_t address, uint8_t *ids, uint16_t num_ids, int16_t *values)
{
	if (ids == NULL || values == NULL) {
		return DXL_PTR_ERROR;
	}
	// Fill parameters with data
	uint16_t needed_params_len = num_ids * (sizeof(uint8_t) // id
						+ sizeof(int16_t)) // value
						+ 4; // address + length 
	if (DXL_MAX_NUM_PARAMS < needed_params_len) {
		// Provided buffer is too small
		return DXL_ERROR;
	}
	packet->params[0] = address&0xFF;
	packet->params[1] = (address >> 8)&0xFF;

	packet->params[2] = 0x02;
	packet->params[3] = 0x00;

	for (size_t i = 0; i < num_ids; i++) {
		packet->params[3 * i + 4] = ids[i];
		packet->params[3 * i + 5] = values[i] & 0xFF;
		packet->params[3 * i + 6] = (values[i] >> 8) & 0xFF;
	}
	packet->params_length = 3 * num_ids + 4;
	packet->id = DXL_ID_BROADCAST;
	packet->instruction = DXL_INST_SYNC_WRITE;
    packet->protocol_version = DYNAMIXEL_PROTOCOL_VERSION;

	return DXL_OK;
}

dxl_err_t dxl_make_ping(dxl_inst_packet_t *packet, dxl_id_t id)
{
	if (id == 0xFF) { // The maximum ID should be FE, which is the broadcast 
		return DXL_ERROR;
	}
	packet->params_length = 0; // there is no payload
	packet->id = id;
	packet->instruction = DXL_INST_PING;
    packet->protocol_version = DYNAMIXEL_PROTOCOL_VERSION;

	return DXL_OK;
}

dxl_err_t dxl_make_write(dxl_inst_packet_t *packet,	uint16_t address, dxl_id_t id, int8_t *params,	uint16_t num_params)
{
	if (params == NULL) {
		return DXL_PTR_ERROR;
	}
	// Fill parameters with data
	uint16_t needed_params_len = num_params // number of parameters
						+ 2; // address 
	if (DXL_MAX_NUM_PARAMS < needed_params_len) {
		// Provided buffer is too small
		return DXL_ERROR;
	}
	packet->params[0] = address&0xFF;
	packet->params[1] = (address >> 8)&0xFF;

	for (size_t i = 0; i < num_params; i++) 
		packet->params[i+2] = params[i];

	packet->params_length = needed_params_len;
	packet->id = id;
	packet->instruction = DXL_INST_WRITE;
    packet->protocol_version = DYNAMIXEL_PROTOCOL_VERSION;

	return DXL_OK;
}