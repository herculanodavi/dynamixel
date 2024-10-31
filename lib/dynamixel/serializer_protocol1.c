#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "dynamixel/serializer.h"

int dxl_serialize_instruction_packet(const dxl_inst_packet_t *packet, uint8_t *buffer,
				     size_t length)
{
	if (packet == NULL || buffer == NULL) {
		return DXL_PTR_ERROR;
	}
	size_t overhead = DXL_INST_PACKET_LEN(0);
	if (packet->params_length > UINT8_MAX - overhead) {
		// Total length would overflow uint8
		return DXL_ERROR;
	}
	size_t bytes_to_write = packet->params_length + overhead;
	if (length < bytes_to_write) {
		// Buffer is too small for serialization
		return DXL_ERROR;
	}

	size_t offset = 0;

	// header
	uint8_t header[] = DXL_HEADER;
	memcpy(buffer, header, DXL_PACKET_HEADER_LEN);
	offset += DXL_PACKET_HEADER_LEN;
	// packet id
	buffer[offset++] = packet->id;
	// length field
	dxl_length_t length_field =
		packet->params_length + DXL_PACKET_INST_LEN + DXL_PACKET_CHKSUM_LEN;
	buffer[offset++] = length_field;
	// instruction
	buffer[offset++] = packet->instruction;
	// parameters
	memcpy(buffer + offset, packet->params, packet->params_length);
	offset += packet->params_length;
	// chksum
	uint32_t sum = 0;
	for (size_t i = DXL_INST_PACKET_ID_POS;
	     i < DXL_INST_PACKET_CHKSUM_POS(packet->params_length); ++i) {
		sum += buffer[i];
	}
	buffer[offset++] = ~((uint8_t)(sum % 0xFF));

	return offset;
}

dxl_err_t dxl_deserialize_status_packet(dxl_status_packet_t *packet, const uint8_t *buffer,
					size_t length)
{
	return DXL_ERROR; // Stubs
}