#ifndef DYNAMIXEL_GROUP_H
#define DYNAMIXEL_GROUP_H

#include <cstddef.h>

#include "dynamixel/protocol.h"
#include "dynamixel/error.h"

typedef struct dxl_id_group {
    dxl_id_t ids[DXL_MAX_NUM_SERVOS];
    size_t num_ids;
} dxl_id_group_t;

dxl_err_t dxl_split_packet(
    const dxl_inst_packet_t* source,
    const dxl_id_group_t* groups,
    size_t num_groups,
    dxl_inst_packet_t* packets
);

#endif