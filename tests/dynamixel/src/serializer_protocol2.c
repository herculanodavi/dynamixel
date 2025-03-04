#include <zephyr/ztest.h>
#include "dynamixel/serializer.h"

// Ping example, ID1
const uint8_t ping_instruction_packet[] = {0xFF, 0xFF, 0xFD, 0x00, 0x01,
					   0x03, 0x00, 0x01, 0x19, 0x4E};
const size_t ping_instruction_packet_len =
	sizeof(ping_instruction_packet) / sizeof(ping_instruction_packet[0]);

// Read example, ID1(XM430-W210) : Present Position(132, 0x0084, 4[byte]) = 166(0x000000A6)
const uint8_t read_instruction_packet[] = {0xFF, 0xFF, 0xFD, 0x00, 0x01, 0x07, 0x00,
					   0x02, 0x84, 0x00, 0x04, 0x00, 0x1D, 0x15};
const size_t read_instruction_packet_len =
	sizeof(read_instruction_packet) / sizeof(read_instruction_packet[0]);

ZTEST(dxl_tests, test_serializer_protocol2)
{
	const size_t buffer_size = 50;
	uint8_t buffer[buffer_size];
	dxl_inst_packet_t packet;
    size_t written_len;
	int err = 0;

	err = dxl_serialize_instruction_packet(NULL, buffer, buffer_size);
	zassert_equal(err, DXL_PTR_ERROR);
	err = dxl_serialize_instruction_packet(&packet, NULL, buffer_size);
	zassert_equal(err, DXL_PTR_ERROR);
	packet.params_length = UINT16_MAX; // Total length would overflow
	err = dxl_serialize_instruction_packet(&packet, buffer, buffer_size);
	zassert_equal(err, DXL_ERROR);

	// Test ping output
	memset(&packet, 0, sizeof(packet));
	packet.id = 0x01;
	packet.instruction = DXL_INST_PING;
	packet.params_length = 0;
	err = dxl_serialize_instruction_packet(&packet, buffer, ping_instruction_packet_len - 1);
	zassert_equal(err, DXL_ERROR); // Buffer too small
	written_len = dxl_serialize_instruction_packet(&packet, buffer, buffer_size);
	zassert_equal(ping_instruction_packet_len, written_len);
	zassert_mem_equal(buffer, ping_instruction_packet, ping_instruction_packet_len);

	// Test read output
	memset(&packet, 0, sizeof(packet));
	packet.id = 0x01;
	packet.instruction = DXL_INST_READ;
	packet.params_length = 4;
	uint8_t params[] = {0x84, 0x00, 0x04, 0x00};
	memcpy(&(packet.params), params, packet.params_length);
	err = dxl_serialize_instruction_packet(&packet, buffer, read_instruction_packet_len - 1);
	zassert_equal(err, DXL_ERROR); // Buffer too small
	written_len = dxl_serialize_instruction_packet(&packet, buffer, buffer_size);
	zassert_equal(read_instruction_packet_len, written_len);
	zassert_mem_equal(buffer, read_instruction_packet, read_instruction_packet_len);
}