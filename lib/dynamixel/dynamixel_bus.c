#include "dynamixel/bus.h"

#include <string.h>

#include <zephyr/logging/log.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/counter.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/time_units.h>

#include "dynamixel/serializer.h"
#include "dynamixel/parser.h"

#define DT_DRV_COMPAT         dynamixel_bus
#define DATA_BUFFER_LENGTH    128
#define TX_RING_BUFFER_LENGTH 128

LOG_MODULE_REGISTER(dynamixel_bus, CONFIG_DYNAMIXEL_LOG_LEVEL);

enum dynamixel_rx_state {
	DXL_STATE_IDLE,
	DXL_STATE_ACTIVE
};

enum dynamixel_rx_event {
	DXL_EVENT_SUCCESS = BIT(0),
	DXL_EVENT_TIMEOUT = BIT(1),
	DXL_EVENT_ERROR = BIT(2),
	DXL_EVENT_ANY = 0xFFFF
};

#if CONFIG_DYNAMIXEL_LOG_LEVEL == LOG_LEVEL_DBG
static const char *dynamixel_event_to_str(uint16_t event)
{
    switch (event) {
    case DXL_EVENT_SUCCESS:
        return "DXL_EVENT_SUCCESS";
    case DXL_EVENT_TIMEOUT:
        return "DXL_EVENT_TIMEOUT";
    case DXL_EVENT_ERROR:
        return "DXL_EVENT_ERROR";
    case DXL_EVENT_ANY:
        return "DXL_EVENT_ANY";
    default:
        return "UNKNOWN_EVENT";
    }
}
#endif

struct dynamixel_config {
	const struct device *uart_dev;
	const struct gpio_dt_spec dir_gpio;
	const struct device *timeout_counter;
	int timeout_channel;
	const uint8_t *motor_ids;
	size_t motor_count;
};

struct dynamixel_data {
	uint8_t buffer[DATA_BUFFER_LENGTH];
	size_t buffer_length;

	// TX
	uint8_t tx_rb_memory[TX_RING_BUFFER_LENGTH];
	struct ring_buf tx_ring_buffer;
	bool is_transmitting;

	// RX
	dxl_parser_handle parser;
	enum dynamixel_rx_state rx_state;
	size_t status_packet_idx;
	size_t requested_packets;
	dxl_status_packet_t *status_packets;
	struct k_event *rx_event;
};

static void uart_isr_cb(const struct device *uart_dev, void *user_data)
{
	const struct device *dxl_dev = user_data;
	const struct dynamixel_config *config = dxl_dev->config;
	struct dynamixel_data *data = dxl_dev->data;
	uint8_t c;

	if (!uart_irq_update(uart_dev)) {
		return;
	}

	LOG_DBG("rx: %d, tx: %d, cplt: %d", uart_irq_rx_ready(uart_dev),
		uart_irq_tx_ready(uart_dev), uart_irq_tx_complete(uart_dev));

	// Handle incoming data
	if (uart_irq_rx_ready(uart_dev)) {
		while (uart_fifo_read(uart_dev, &c, 1)) {
			if (data->parser) {
				dxl_parser_feed_bytes(data->parser, &c, 1);
			}
			LOG_DBG("rx: %c (%d)", c, c);
		}
	}

	// Handle outgoing data
	if (uart_irq_tx_ready(uart_dev)) {
		uint8_t *tx_data;
		uint32_t tx_size =
			ring_buf_get_claim(&data->tx_ring_buffer, &tx_data, TX_RING_BUFFER_LENGTH);
		if (tx_size > 0) {
			uint32_t sent = uart_fifo_fill(uart_dev, tx_data, tx_size);
			ring_buf_get_finish(&data->tx_ring_buffer, sent);
			LOG_DBG("rb -> tx_fifo, %d bytes", sent);
		}
	}

	if (uart_irq_tx_complete(uart_dev) && data->is_transmitting) {
		// Turn the direction back to receiving
		LOG_WRN("tx complete");
		gpio_pin_set_dt(&config->dir_gpio, GPIO_OUTPUT_INACTIVE);
		uart_irq_tx_disable(config->uart_dev);
		data->is_transmitting = false;
	}
}

static int uart_async_send(const struct device *dev, uint8_t *buffer, size_t length)
{
	const struct dynamixel_config *config = dev->config;
	struct dynamixel_data *data = dev->data;

	// Turn the direction to sending
	int status = gpio_pin_set_dt(&config->dir_gpio, GPIO_OUTPUT_ACTIVE);
	if (status < 0) {
		LOG_ERR("Error setting bus to transmit");
		return -EIO;
	}

	unsigned int lock = irq_lock();
	int enqueued = 0;
	if (!data->is_transmitting) {
		LOG_DBG("not transmitting, enqueuing fifo");
		data->is_transmitting = true;
		enqueued = uart_fifo_fill(config->uart_dev, buffer, length);
		LOG_DBG("fifo fill returned %d", enqueued);
		uart_irq_tx_enable(config->uart_dev);
	}

	LOG_DBG("enqueuing ring buffer");
	enqueued += ring_buf_put(&data->tx_ring_buffer, buffer + enqueued, length - enqueued);
	if (length > enqueued) {
		LOG_ERR("tx ring buffer is full");
		return -ENOMSG;
	}
	LOG_DBG("sent %d bytes", enqueued);
	irq_unlock(lock);

	return 0;
}

static void complete_rx(const struct device *dev, enum dynamixel_rx_event event)
{
	LOG_DBG("rx finished with flag: %s", dynamixel_event_to_str(event));
	struct dynamixel_data *data = dev->data;
	const struct dynamixel_config *config = dev->config;

	data->rx_state = DXL_STATE_IDLE;
	counter_cancel_channel_alarm(config->timeout_counter, config->timeout_channel);
	k_event_post(data->rx_event, event);
}

static void parser_match_callback(dxl_parser_handle parser, uint8_t *buffer, size_t length,
				  void *arg)
{
	const struct device *dev = arg;
	struct dynamixel_data *data = dev->data;

	LOG_DBG("Parser match callback");

	if (data->rx_state != DXL_STATE_ACTIVE) {
		return;
	}

	dxl_err_t err = dxl_deserialize_status_packet(
		&data->status_packets[data->status_packet_idx], buffer, length);
	if (err != DXL_OK) {
		complete_rx(dev, DXL_EVENT_ERROR);
		return;
	}

	data->status_packet_idx++;
	if (data->status_packet_idx == data->requested_packets) {
		complete_rx(dev, DXL_EVENT_SUCCESS);
	}
}

static void deadline_callback(const struct device *counter, uint8_t chan_id, uint32_t ticks,
			      void *user_data)
{
	LOG_WRN("Receive timed out!");
	complete_rx(user_data, DXL_EVENT_TIMEOUT);
}

static int set_deadline(const struct device *dev, uint32_t time_us)
{
	const struct dynamixel_config *config = dev->config;

	uint32_t counter_ticks = counter_us_to_ticks(config->timeout_counter, time_us);

	if (counter_ticks == UINT32_MAX) {
		LOG_ERR("Counter saturated for this timeout value");
	}

	if (counter_ticks == 0) {
		return -EINVAL;
	}

	struct counter_alarm_cfg alarm_cfg = {
		.callback = deadline_callback,
		.ticks = counter_ticks,
		.flags = 0,
		.user_data = (void *)dev,
	};

	return counter_set_channel_alarm(config->timeout_counter, config->timeout_channel,
					 &alarm_cfg);
}

int dynamixel_send(const struct device *dev, const dxl_inst_packet_t *packet)
{
	struct dynamixel_data *data = dev->data;

	int packet_length =
		dxl_serialize_instruction_packet(packet, data->buffer, data->buffer_length);
	if (packet_length < 0) {
		return -EIO;
	}

	return uart_async_send(dev, data->buffer, packet_length);
}

int dynamixel_send_receive(const struct device *dev, const dxl_inst_packet_t *packet,
			   dxl_status_packet_t *status_packet, int32_t timeout_us)
{
	struct dynamixel_data *data = dev->data;
	int ret;

	// Set up timer alarm for receive
	k_event_clear(data->rx_event, DXL_EVENT_ANY);
	ret = set_deadline(dev, timeout_us);
	if (ret != 0) {
		return ret;
	}

	data->requested_packets = packet->num_status_packet;
	data->status_packets = status_packet;
	data->status_packet_idx = 0;
	data->rx_state = DXL_STATE_ACTIVE;

	// Send packet
	LOG_INF("sending packet");
	ret = dynamixel_send(dev, packet);
	if (ret != 0) {
		return ret;
	}

	// If no packets are expected, do not wait
	if (data->requested_packets == 0) {
		complete_rx(dev, DXL_EVENT_SUCCESS);
		return 0;
	}

	// Wait for result
	LOG_INF("waiting for result");
	k_timeout_t timeout = {k_us_to_ticks_ceil32(timeout_us)};
	ret = k_event_wait(data->rx_event, DXL_EVENT_ANY, true, timeout);
	if (ret == 0) {
		LOG_ERR("Event queue timed out");
		return ret;
	}

	int num_packets = data->status_packet_idx;
	LOG_INF("got %d packets", num_packets);
	return 0;
}

int dynamixel_init(const struct device *dev)
{
	const struct dynamixel_config *config = dev->config;
	struct dynamixel_data *data = dev->data;

	int ret = uart_irq_callback_user_data_set(config->uart_dev, uart_isr_cb, (void *)dev);
	if (ret) {
		LOG_ERR("Failed to set UART callback: %d\n", ret);
		return ret;
	}

	// Init DXL parser
	dxl_parser_handle parser = dxl_parser_create();
	dxl_parser_set_match_callback(parser, parser_match_callback);
	dxl_parser_set_arg(parser, (void *)dev);
	data->parser = parser;
	if (!data->parser) {
		LOG_ERR("Failed to create DXL parser");
		return -1;
	}

	ring_buf_init(&data->tx_ring_buffer, TX_RING_BUFFER_LENGTH, data->tx_rb_memory);
	uart_irq_rx_enable(config->uart_dev);
	counter_start(config->timeout_counter);

	return 0;
}

#define DYNAMIXEL_INIT(inst)                                                                       \
	static const uint8_t dynamixel_motor_ids_##inst[] = DT_INST_PROP(inst, motor_ids);         \
	K_EVENT_DEFINE(dynamixel_event_##inst);                                                    \
                                                                                                   \
	static const struct dynamixel_config dynamixel_config_##inst = {                           \
		.uart_dev = DEVICE_DT_GET(DT_INST_PHANDLE(inst, uart)),                            \
		.dir_gpio = GPIO_DT_SPEC_INST_GET(inst, direction_gpios),                          \
		.motor_ids = dynamixel_motor_ids_##inst,                                           \
		.motor_count = DT_INST_PROP_LEN(inst, motor_ids),                                  \
		.timeout_counter = DEVICE_DT_GET(DT_INST_PHANDLE(inst, timeout_timer)),            \
		.timeout_channel = DT_INST_PROP(inst, timeout_timer_channel)};                     \
                                                                                                   \
	static struct dynamixel_data dynamixel_data_##inst = {.buffer_length = DATA_BUFFER_LENGTH, \
							      .is_transmitting = false,            \
							      .rx_state = DXL_STATE_IDLE,          \
							      .status_packet_idx = 0,              \
							      .rx_event =                          \
								      &dynamixel_event_##inst};    \
                                                                                                   \
	DEVICE_DT_INST_DEFINE(inst, &dynamixel_init, NULL, &dynamixel_data_##inst,                 \
			      &dynamixel_config_##inst, POST_KERNEL, 60, NULL);

DT_INST_FOREACH_STATUS_OKAY(DYNAMIXEL_INIT)