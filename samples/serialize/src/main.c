#include <stddef.h>
#include <string.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/kernel.h>

#include "dynamixel/serializer.h"
#include "dynamixel/instruction_packet.h"

void test_function(dxl_err_t status, const dxl_inst_packet_t *inst_packet, uint8_t *buffer, size_t buffer_length){
    __ASSERT(status == DXL_OK, "Error composing func");
    printk("Status returned %d\n", status);
    
    int num_bytes = dxl_serialize_instruction_packet(inst_packet, buffer, buffer_length);
    __ASSERT(num_bytes > 0, "Error serializing example packet");
    for(int i = 0; i < num_bytes; i++){
        printk("%02x ", buffer[i]);
        k_msleep(500);
    }
    printk("\n%d bytes written\n", num_bytes);
}

int main() {
    printk("Hello world\n");

    uint8_t buffer[20];
    size_t buffer_length = 20;
    uint8_t ids[] = {0x01, 0x02};
    uint16_t values[] = {0x00FF, 0x00AA};

    dxl_inst_packet_t inst_packet;
    dxl_err_t status = dxl_make_sync_write_i16(&inst_packet, 0x20, ids, 2, values);
    test_function(status, &inst_packet, buffer, buffer_length);

    // Testing PING packet
    status = dxl_make_ping(&inst_packet, 0x01);
    test_function(status, &inst_packet, buffer, buffer_length);

    // Testing WRITE packet
    int8_t params[] = {0x00,0x02, 0x00, 0x00};
    status = dxl_make_write(&inst_packet, 0x74, 0x01, params, 4);
    test_function(status, &inst_packet, buffer, buffer_length);

    return 0;
}

// todo: ping (0x01), write (0x03), read(x02)
// VERIFICAR SE A DIVISAO DE SYNC WRITE E SYNC READ ESTA CORRETA.