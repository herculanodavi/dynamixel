#ifndef DYNAMIXEL_INSTRUCTION_PACKET_H
#define DYNAMIXEL_INSTRUCTION_PACKET_H

#include "dynamixel/protocol.h"
#include "dynamixel/error.h"

/**
 * @brief Fills the instruction packet structure with a sync write of a 2-byte value.
 * @param packet: the instruction packet to be filled.
 * @param address: the memory address to write.
 * @param ids: the ids of the target servos.
 * @param num_ids: number of target servos.
 * @param values: values to write to the servos. Must have <num_ids> length.
 * @return a DXL_* status flag.
 */
dxl_err_t dxl_make_sync_write_i16(
        dxl_inst_packet_t *packet,
        uint16_t address,
        uint8_t *ids,
        uint16_t num_ids,
        int16_t *values);

/**
 * @brief Fills the instruction packet structure with a Ping command that checks whether the Packet has arrived at a device with the same ID as the specified packet ID .
 * @param packet: the instruction packet to be filled.
 * @param id: the id of the device that should receive the ping and process it.
 * @return a DXL_* status flag.
 */
dxl_err_t dxl_make_ping(
        dxl_inst_packet_t *packet,
        dxl_id_t id);

/**
 * @brief Fills the instruction packet structure with a write of an array of bytes.
 * @param packet: the instruction packet to be filled.
 * @param address: the memory address to write.
 * @param id: the id of the target servos.
 * @param params: The array of params it is going to be sent to the servo.
 * @param num_params: The length of the parameters it is going to be sent.
 * @return a DXL_* status flag.
 */
dxl_err_t dxl_make_write(
        dxl_inst_packet_t *packet,
        uint16_t address,
        dxl_id_t id,
        int8_t *params,
        uint16_t num_params);

#endif