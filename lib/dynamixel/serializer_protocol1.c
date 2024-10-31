#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "dynamixel/serializer.h"

int dxl_serialize_instruction_packet(const dxl_inst_packet_t *packet, uint8_t *buffer,
				      size_t length)
{
    return DXL_ERROR; // Stub
}

dxl_err_t dxl_deserialize_status_packet(dxl_status_packet_t *packet, const uint8_t *buffer,
					 size_t length)
{
    return DXL_ERROR; // Stubs
}