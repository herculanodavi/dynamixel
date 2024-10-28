#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <zephyr/sys/crc.h>

#include "dynamixel/dynamixel.h"

#define DXL2_ID_BROADCAST 0xFE
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

int dxl2_serialize_instruction_packet(const dxl2_inst_packet_t *packet, uint8_t *buffer,
				      size_t length)
{
	if (packet == NULL || buffer == NULL) {
		return DXL_PTR_ERROR;
	}
	size_t overhead = DXL2_INST_PACKET_LEN(0);
	if (packet->params_length > UINT16_MAX - overhead) {
		return DXL_ERROR;
	}
	size_t bytes_to_write = packet->params_length + overhead;
	if (length < bytes_to_write) {
		return DXL_ERROR;
	}

	size_t offset = 0;

	// header
	uint8_t header[] = DXL2_HEADER;
	memcpy(buffer, header, DXL2_PACKET_HEADER_LEN);
	offset += DXL2_PACKET_HEADER_LEN;
	// packet id
	buffer[offset++] = packet->id;
	// length field
	uint16_t length_field = packet->params_length + DXL2_PACKET_INST_LEN + DXL2_PACKET_CRC_LEN;
	write_little_endian_u16(buffer + offset, length_field);
	offset += DXL2_PACKET_LEN_LEN;
	// instruction
	buffer[offset++] = packet->instruction;
	// parameters
	memcpy(buffer + offset, packet->params, packet->params_length);
	offset += packet->params_length;
	// crc
	uint16_t crc_value = crc16(DXL2_CRC_POLY, DXL2_CRC_INITIAL, buffer, offset);
	write_little_endian_u16(buffer + offset, crc_value);
	offset += DXL2_PACKET_CRC_LEN;

	return offset;
}

dxl_err_t dxl2_make_sync_write_i16(dxl2_inst_packet_t *packet, uint16_t address, uint8_t *ids,
				   uint16_t num_ids, int16_t *values)
{
	if (ids == NULL || values == NULL) {
		return DXL_PTR_ERROR;
	}
	// Fill parameters with data
	uint16_t needed_params_len = num_ids * (sizeof(uint8_t)     // id
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
	packet->id = DXL2_ID_BROADCAST;
	packet->instruction = DXL2_INST_SYNC_WRITE;
	return DXL_OK;
}

dxl_err_t dxl2_deserialize_status_packet(dxl2_status_packet_t *packet, const uint8_t *buffer,
					 size_t length)
{
	if (packet == NULL) {
		return DXL_PTR_ERROR;
	}

	size_t overhead = DXL2_STATUS_PACKET_LEN(0);
	if (length < overhead) {
		// Input length is smaller than an empty packet
		return DXL_ERROR;
	}

	uint16_t length_field = read_little_endian_u16(buffer + DXL2_STATUS_PACKET_LEN_POS);
	uint16_t params_length =
		length_field - (DXL2_PACKET_INST_LEN + DXL2_PACKET_CRC_LEN + DXL2_PACKET_ERR_LEN);
	if (length < DXL2_STATUS_PACKET_LEN(params_length)) {
		// Input length is smaller than specified in the length field
		return DXL_ERROR;
	}
	if (params_length > DXL_MAX_NUM_PARAMS) {
		// Input lenght of parameters is greater than maximum
		return DXL_ERROR;
	}

	packet->id = buffer[DXL2_STATUS_PACKET_ID_POS];
	packet->instruction = buffer[DXL2_STATUS_PACKET_INST_POS];
	packet->error = buffer[DXL2_STATUS_PACKET_ERR_POS];
	packet->params_length = params_length;
	memcpy(packet->params, &(buffer[DXL2_STATUS_PACKET_PARAM_POS]), params_length);

	return DXL_OK;
}