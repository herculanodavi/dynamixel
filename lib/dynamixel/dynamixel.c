#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <zephyr/sys/crc.h>

#include "dynamixel/serializer.h"

static void write_little_endian_u16(uint8_t *destination, uint16_t value)
{
	*destination = value & 0xFF;
	destination++;
	value = value >> 8;
	*destination = value & 0xFF;
}

static uint16_t read_little_endian_u16(const uint8_t *source)
{
	uint16_t value = *(source + 1);
	value = value << 8;
	value |= *(source) & 0xFF;
	return value;
}

int dxl_serialize_instruction_packet(const dxl_inst_packet_t *packet, uint8_t *buffer,
				      size_t length)
{
	if (packet == NULL || buffer == NULL) {
		return DXL_PTR_ERROR;
	}
	size_t overhead = DXL_INST_PACKET_LEN(0);
	if (packet->params_length > UINT16_MAX - overhead) {
		// Total length would overflow uint16
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
	uint16_t length_field = packet->params_length + DXL_PACKET_INST_LEN + DXL_PACKET_CRC_LEN;
	write_little_endian_u16(buffer + offset, length_field);
	offset += DXL_PACKET_LEN_LEN;
	// instruction
	buffer[offset++] = packet->instruction;
	// parameters
	memcpy(buffer + offset, packet->params, packet->params_length);
	offset += packet->params_length;
	// crc
	uint16_t crc_value = crc16(DXL_CRC_POLY, DXL_CRC_INITIAL, buffer, offset);
	write_little_endian_u16(buffer + offset, crc_value);
	offset += DXL_PACKET_CRC_LEN;

	return offset;
}

dxl_err_t dxl_deserialize_status_packet(dxl_status_packet_t *packet, const uint8_t *buffer,
					 size_t length)
{
	if (packet == NULL) {
		return DXL_PTR_ERROR;
	}

	size_t overhead = DXL_STATUS_PACKET_LEN(0);
	if (length < overhead) {
		// Input length is smaller than an empty packet
		return DXL_ERROR;
	}

	uint16_t length_field = read_little_endian_u16(buffer + DXL_STATUS_PACKET_LEN_POS);
	uint16_t params_length =
		length_field - (DXL_PACKET_INST_LEN + DXL_PACKET_CRC_LEN + DXL_PACKET_ERR_LEN);
	if (length < DXL_STATUS_PACKET_LEN(params_length)) {
		// Input length is smaller than specified in the length field
		return DXL_ERROR;
	}
	if (params_length > DXL_MAX_NUM_PARAMS) {
		// Input lenght of parameters is greater than maximum
		return DXL_ERROR;
	}

	packet->id = buffer[DXL_STATUS_PACKET_ID_POS];
	packet->instruction = buffer[DXL_STATUS_PACKET_INST_POS];
	packet->error = buffer[DXL_STATUS_PACKET_ERR_POS];
	packet->params_length = params_length;
	packet->protocol_version = DYNAMIXEL_PROTOCOL_VERSION;
	memcpy(packet->params, &(buffer[DXL_STATUS_PACKET_PARAM_POS]), params_length);

	return DXL_OK;
}