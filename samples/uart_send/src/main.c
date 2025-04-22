#include <zephyr/sys/__assert.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "dynamixel/bus.h"
#include "dynamixel/instruction_packet.h"

LOG_MODULE_REGISTER(main, CONFIG_SOC_LOG_LEVEL);

const struct device *dxl = DEVICE_DT_GET(DT_NODELABEL(dxl0));

int main()
{
	uint8_t ids[] = {0x01, 0x02};
	uint16_t values[] = {0x00FF, 0x00AA};

	dxl_inst_packet_t inst_packet;
	dxl_err_t status = dxl_make_sync_write_i16(&inst_packet, 0x20, ids, 2, values);
	__ASSERT(status == DXL_OK, "Error composing sync write packet");
	LOG_INF("dxl_make_sync_write_i16 returned %d", status);

	int ret = dynamixel_send_receive(dxl, &inst_packet, NULL, 39000); // 0 packets are expected
	LOG_INF("dynamixel_send_receive status: %d", ret);

	status = dxl_make_sync_read_i16(&inst_packet, 0x20, ids, 2);
	__ASSERT(status == DXL_OK, "Error composing sync write packet");
	LOG_INF("dxl_make_sync_read_i16 returned %d", status);
	dxl_status_packet_t status_packets[2];

	ret = dynamixel_send_receive(dxl, &inst_packet, status_packets, 5000);
	LOG_INF("dynamixel_send_receive status: %d", ret);

	return 0;
}