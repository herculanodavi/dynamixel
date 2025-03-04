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
dxl_err_t dxl_make_sync_write_i16(dxl_inst_packet_t *packet, uint16_t address, const dxl_id_t *ids,
				  uint16_t num_ids, const int16_t *values);

/**
 * @brief Fills the instruction packet structure with a sync read of a 2-byte value.
 * @param packet: the instruction packet to be filled.
 * @param address: the memory address to read from.
 * @param ids: the ids of the target servos.
 * @param num_ids: number of target servos.
 * @return a DXL_* status flag.
 */
dxl_err_t dxl_make_sync_read_i16(dxl_inst_packet_t *packet, uint16_t address, const dxl_id_t *ids,
				 uint16_t num_ids);

#endif