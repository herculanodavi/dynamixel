#include <stddef.h>
#include <string.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/kernel.h>

#include "dynamixel/serializer.h"
#include "dynamixel/instruction_packet.h"

int main() {
    printk("Hello world\n");

    uint8_t buffer[20];
    size_t buffer_length = 20;
    uint8_t ids[] = {0x01, 0x02};
    uint16_t values[] = {0x00FF, 0x00AA};

    dxl_inst_packet_t inst_packet;
    dxl_err_t status = dxl_make_sync_write_i16(&inst_packet, 0x20, ids, 2, values);
    __ASSERT(status == DXL_OK, "Error composing sync write packet");
    printk("Status returned %d\n", status);
    
    int num_bytes = dxl_serialize_instruction_packet(&inst_packet, buffer, buffer_length);
    __ASSERT(num_bytes > 0, "Error serializing example packet");
    printk("%d bytes written\n", num_bytes);

    return 0;
}