#ifndef DYNAMIXEL_SERIALIZER_H
#define DYNAMIXEL_SERIALIZER_H

#include <stddef.h>
#include <stdint.h>

#include "dynamixel/protocol.h"
#include "dynamixel/error.h"

/**
 * @brief Writes the content of an instruction packet to a bytes buffer.
 * @param packet: the instruction packet to be serialized.
 * @param buffer: the bytes buffer to write to.
 * @param length: the length of the bytes buffer.
 * @return the number of bytes written to the packet if successful, otherwise
 * returns an error code.
 */
int dxl_serialize_instruction_packet(
        const dxl_inst_packet_t *packet,
        uint8_t *buffer,
        size_t length);

/**
 * @brief Deserializes the bytes buffer to a status packet. The behavior of this
 * function is undefined if the buffer provided is not a status packet.
 * @param packet: a pointer to the packet that will hold the result.
 * @param buffer: the bytes buffer to read.
 * @param length: the length of the bytes buffer.
 * @return a DXL_* status flag.
 */
dxl_err_t dxl_deserialize_status_packet(
        dxl_status_packet_t *packet,
        const uint8_t *buffer,
        size_t length);

#endif