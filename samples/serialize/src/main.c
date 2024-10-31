#include <stddef.h>
#include <string.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/kernel.h>

#include "dynamixel/serializer.h"

int main() {
    printk("Hello world\n");

    uint8_t buffer[20];
    size_t buffer_length = 20;
    uint8_t params[] = {0x01, 0x02};
    uint16_t params_length = 2;

    dxl_inst_packet_t inst_packet;
    inst_packet.id = 0x01;
    inst_packet.instruction = DXL_INST_SYNC_READ;
    memcpy(inst_packet.params, params, params_length);
    inst_packet.params_length = params_length;

    int num_bytes = dxl_serialize_instruction_packet(&inst_packet, buffer, buffer_length);
    __ASSERT(num_bytes > 0, "Error serializing example packet");
    printk("%d bytes written\n", num_bytes);

    return 0;
}