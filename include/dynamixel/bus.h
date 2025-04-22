#pragma once

#include <zephyr/drivers/uart.h>
#include <zephyr/device.h>

#include "dynamixel/protocol.h"
#include "dynamixel/error.h"

/**
 * @brief Send a dynamixel message and do not expect a response (fire and forget).
 */
int dynamixel_send(const struct device *dev, const dxl_inst_packet_t *packet);

/**
 * @brief Send a dynamixel message and expect a response
 */
int dynamixel_send_receive(const struct device *dev, const dxl_inst_packet_t *packet,
	dxl_status_packet_t *status_packet, int32_t timeout_us);