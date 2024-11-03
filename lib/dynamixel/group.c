#include "dynamixel/group.h"

#include <string.h>

static dxl_err_t dxl_split_sync_write(const dxl_inst_packet_t *source, const dxl_id_group_t *groups,
				      size_t num_groups, dxl_inst_packet_t *packets);
static dxl_err_t dxl_split_fast_sync_read(const dxl_inst_packet_t *source,
					  const dxl_id_group_t *groups, size_t num_groups,
					  dxl_inst_packet_t *packets);

dxl_err_t dxl_split_packet(const dxl_inst_packet_t *source, const dxl_id_group_t *groups,
			   size_t num_groups, dxl_inst_packet_t *packets)
{
	if (source == NULL) {
		return DXL_PTR_ERROR;
	}

	switch (source->instruction) {
	case DXL_INST_SYNC_WRITE:
		return dxl_split_sync_write(source, groups, num_groups, packets);
	case DXL_INST_FAST_SYNC_READ:
		return dxl_split_fast_sync_read(source, groups, num_groups, packets);
	default:
		return DXL_NOT_IMPLEMENTED;
	}
}

/**
 * @brief Search for all IDs in source that belong to the group and copy to the packet
 */
static void search_id_and_copy(const dxl_id_group_t *id_group, const uint8_t *params,
			       size_t params_length, size_t offset, size_t spacing,
			       dxl_inst_packet_t *packet)
{
	// From group
	dxl_id_t *group_ids = id_group->ids;
	const size_t num_group_ids = id_group->num_ids;

	// From packet
	dxl_id_t *packet_ids = params + offset;
	const size_t num_packet_ids = (params_length - offset) / spacing;

	// Two-pointer approach for searching id matches
	size_t i = 0;
	size_t j = 0;
	while (i < num_group_ids && j < num_packet_ids) {
		if (group_ids[i] == packet_ids[j * spacing + offset]) {
			// A match happened, write the id and the data to the packet
			uint8_t *source = params[j * spacing + offset];
			uint8_t *destination = packet->params[packet->params_length];
			memcpy(destination, source, spacing);
			packet->params_length += spacing;
			i++;
			j++;
		} else if (group_ids[i] < packet_ids[j * spacing]) {
			i++;
		} else {
			j++;
		}
	}
}

static dxl_err_t dxl_split_sync_write(const dxl_inst_packet_t *source, const dxl_id_group_t *groups,
				      size_t num_groups, dxl_inst_packet_t *packets)
{
	// 3rd and 4th bytes are the length of written bytes in little-endian format
	const size_t write_length = ((source->params[3] & 0xFF) << 8) | (source->params[2] & 0xFF);
	// Position of the first byte that is an ID
	const size_t first_id_pos = 4;
	// Distance in bytes between ids
	const size_t id_spacing = write_length + 1;

	// Copy until first id offset
	memcpy(source->params, packet->params, first_id_pos);

	for (size_t i = 0; i < num_groups; i++) {
		const dxl_id_group_t *group = &(groups[i]);
		dxl_inst_packet_t *packet = &(packets[i]);

		packet->id = source->id;
		packet->instruction = source->instruction;
		packet->protocol_version = source->protocol_version;
		search_id_and_copy(group, source->params, source->params_length, first_id_pos,
				   id_spacing, packet);
	}
}

static dxl_err_t dxl_split_fast_sync_read(const dxl_inst_packet_t *source,
					  const dxl_id_group_t *groups, size_t num_groups,
					  dxl_inst_packet_t *packets)
{
	// Position of the first byte that is an ID
	const size_t first_id_pos = 4;
	// Distance in bytes between ids
	const size_t id_spacing = 1;

	// Copy until first id offset
	memcpy(source->params, packet->params, first_id_pos);

	for (size_t i = 0; i < num_groups; i++) {
		const dxl_id_group_t *group = &(groups[i]);
		dxl_inst_packet_t *packet = &(packets[i]);

		packet->id = source->id;
		packet->instruction = source->instruction;
		packet->protocol_version = source->protocol_version;
		search_id_and_copy(group, source->params, source->params_length, first_id_pos,
				   id_spacing, packet);
	}
}