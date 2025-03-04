#include "dynamixel/instruction_packet.h"

#include <stddef.h>

#include "dynamixel/protocol.h"

dxl_err_t dxl_make_sync_write_i16(dxl_inst_packet_t *packet, uint16_t address, const dxl_id_t *ids,
				  uint16_t num_ids, const int16_t *values)
{
	if (ids == NULL || values == NULL) {
		return DXL_PTR_ERROR;
	}
	// Fill parameters with data
	uint16_t needed_params_len = num_ids * (sizeof(dxl_id_t)    // id
						+ sizeof(int16_t)); // value
	if (DXL_MAX_NUM_PARAMS < needed_params_len) {
		// Provided buffer is too small
		return DXL_ERROR;
	}
	for (size_t i = 0; i < num_ids; i++) {
		packet->params[3 * i] = ids[i];
		packet->params[3 * i + 1] = values[i] & 0xFF;
		packet->params[3 * i + 2] = (values[i] >> 8) & 0xFF;
	}
	packet->params_length = 3 * num_ids;
	packet->id = DXL_ID_BROADCAST;
	packet->instruction = DXL_INST_SYNC_WRITE;
	packet->protocol_version = DYNAMIXEL_PROTOCOL_VERSION;

	return DXL_OK;
}

dxl_err_t dxl_make_sync_read_i16(dxl_inst_packet_t *packet, uint16_t address, const dxl_id_t *ids,
				 uint16_t num_ids)
{
	if (ids == NULL) {
		return DXL_PTR_ERROR;
	}
	// Fill parameters with data
	uint16_t needed_params_len = num_ids * sizeof(dxl_id_t);
	if (DXL_MAX_NUM_PARAMS < needed_params_len) {
		// Provided buffer is too small
		return DXL_ERROR;
	}

	memcpy(packet->params, ids, num_ids);
	packet->params_length = num_ids;
	packet->id = DXL_ID_BROADCAST;
	packet->instruction = DXL_INST_SYNC_READ;
	packet->protocol_version = DYNAMIXEL_PROTOCOL_VERSION;

	return DXL_OK;
}