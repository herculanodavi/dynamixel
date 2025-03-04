#include <string.h>
#include <stdlib.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(dxl_parser, CONFIG_DYNAMIXEL_LOG_LEVEL);

#include "dynamixel/parser.h"
#include "dynamixel/protocol.h"

#define DXL_MAX_PACKET_SIZE DXL_STATUS_PACKET_LEN(DXL_MAX_NUM_PARAMS)

typedef enum {
	DXL_PARSER_STATE_HEADER,
	DXL_PARSER_STATE_ID,
	DXL_PARSER_STATE_LENGTH,
	DXL_PARSER_STATE_SEEK_END
} dxl_parser_state_t;

#ifdef CONFIG_DYNAMIXEL_LOG_LEVEL_DBG
const char *m_state_names[] = {"STATE_HEADER", "STATE_ID", "STATE_LENGTH", "STATE_SEEK_END"};
#endif

typedef struct dxl_parser {
	uint8_t ping_pong_buffer[2][DXL_MAX_PACKET_SIZE];
	uint8_t ping_pong_buffer_which;
	uint16_t current_params_length;
	size_t cursor;
	dxl_parser_state_t state;
	dxl_parser_match_callback on_match;
	void *arg;
} dxl_parser_t;

static uint8_t *dxl_parser_get_buffer(dxl_parser_handle parser)
{
	__ASSERT(parser->ping_pong_buffer_which < 2, "Unexpected value");
	return parser->ping_pong_buffer[parser->ping_pong_buffer_which];
}

static void dxl_parser_switch_buffer(dxl_parser_handle parser)
{
	parser->ping_pong_buffer_which = !parser->ping_pong_buffer_which;
}

dxl_parser_handle dxl_parser_create()
{
	dxl_parser_handle parser = malloc(sizeof(dxl_parser_t));
	dxl_parser_reset_state(parser);
	parser->ping_pong_buffer_which = 0;
	return parser;
}

dxl_parser_handle dxl_parser_create_static(void *block, size_t size)
{
	__ASSERT(size >= sizeof(dxl_parser_t), "Size of memory block should be at least %d",
		 sizeof(dxl_parser_t));
	dxl_parser_handle parser = (dxl_parser_handle)block;
	dxl_parser_reset_state(parser);
	parser->ping_pong_buffer_which = 0;
	return parser;
}

size_t dxl_get_handle_size()
{
	return sizeof(dxl_parser_t);
}

dxl_err_t dxl_parser_set_match_callback(dxl_parser_handle parser,
					dxl_parser_match_callback callback)
{
	if (parser == NULL) {
		return DXL_PTR_ERROR;
	}
	parser->on_match = callback;
	return DXL_OK;
}

dxl_err_t dxl_parser_set_arg(dxl_parser_handle parser, void *arg)
{
	if (parser == NULL) {
		return DXL_PTR_ERROR;
	}

	parser->arg = arg;

	return DXL_OK;
}

dxl_err_t dxl_parser_reset_state(dxl_parser_handle parser)
{
	if (parser == NULL) {
		return DXL_PTR_ERROR;
	}

	parser->cursor = 0;
	parser->current_params_length = 0;
	parser->state = DXL_PARSER_STATE_HEADER;

	return DXL_OK;
}

dxl_err_t dxl_parser_feed_bytes(dxl_parser_handle parser, const uint8_t *buffer, size_t length)
{
	if (parser == NULL) {
		return DXL_PTR_ERROR;
	}
	if (buffer == NULL && length > 0) {
		return DXL_PTR_ERROR;
	}

	uint8_t *output_buffer = dxl_parser_get_buffer(parser);

	for (size_t i = 0; i < length; ++i) {
		bool matched = false;
		output_buffer[parser->cursor] = buffer[i];

		switch (parser->state) {
		case DXL_PARSER_STATE_HEADER: {
			const uint8_t header[] = DXL_HEADER;
			uint8_t header_byte = header[parser->cursor];

			if (buffer[i] != header_byte) {
				LOG_DBG("Header invalid, reset state.");
				dxl_parser_reset_state(parser);
			} else if (parser->cursor == DXL_STATUS_PACKET_ID_POS - 1) {
				LOG_DBG("Change state to DXL_PARSER_STATE_ID");
				parser->state = DXL_PARSER_STATE_ID;
			}
			break;
		}
		case DXL_PARSER_STATE_ID: {
			LOG_DBG("Change state to DXL_PARSER_STATE_LENGTH");
			parser->state = DXL_PARSER_STATE_LENGTH;
			break;
		}
		case DXL_PARSER_STATE_LENGTH: {
			parser->current_params_length =
				(buffer[i] << 8) | (parser->current_params_length >> 8);

			if (parser->cursor == DXL_STATUS_PACKET_INST_POS - 1) {
				LOG_DBG("Parsed length is %d", parser->current_params_length);
				LOG_DBG("Change state to DXL_PARSER_STATE_SEEK_END");
				parser->state = DXL_PARSER_STATE_SEEK_END;
			}
			break;
		}
		case DXL_PARSER_STATE_SEEK_END: {
			size_t expected_packet_length =
				DXL_STATUS_PACKET_LEN(parser->current_params_length);
			if (parser->cursor == expected_packet_length - 1) {
				matched = true;
			}
			break;
		}
		}

		if (matched) {
			LOG_DBG("Reached match");
			if (parser->on_match != NULL) {
				parser->on_match(parser, output_buffer, parser->cursor + 1,
						 parser->arg);
			}
			dxl_parser_reset_state(parser);
			dxl_parser_switch_buffer(parser);
			matched = false;
		} else {
			parser->cursor++;
		}
	}

	return DXL_OK;
}