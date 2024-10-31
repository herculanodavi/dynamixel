#include <string.h>
#include <stdlib.h>
#include <zephyr/sys/__assert.h>
#include "dynamixel/parser.h"

#define DXL2_MAX_PACKET_SIZE DXL2_STATUS_PACKET_LEN(DXL_MAX_NUM_PARAMS)

typedef enum {
	DXL2_PARSER_STATE_HEADER,
	DXL2_PARSER_STATE_ID,
	DXL2_PARSER_STATE_LENGTH,
	DXL2_PARSER_STATE_SEEK_END
} dxl2_parser_state_t;

typedef struct dxl2_parser {
	uint8_t ping_pong_buffer[2][DXL2_MAX_PACKET_SIZE];
	uint8_t ping_pong_buffer_which;
	uint8_t current_params_length;
	size_t cursor;
	dxl2_parser_state_t state;
	dxl2_parser_match_callback on_match;
	void *arg;
} dxl2_parser_t;

static uint8_t *dxl2_parser_get_buffer(dxl2_parser_handle parser)
{
	__ASSERT(parser->ping_pong_buffer_which < 2, "Unexpected value");
	return parser->ping_pong_buffer[parser->ping_pong_buffer_which];
}

static void dxl2_parser_switch_buffer(dxl2_parser_handle parser)
{
	parser->ping_pong_buffer_which = !parser->ping_pong_buffer_which;
}

dxl2_parser_handle dxl2_parser_create()
{
	dxl2_parser_handle parser = malloc(sizeof(dxl2_parser_t));
	dxl2_parser_reset_state(parser);
	parser->ping_pong_buffer_which = 0;
	return parser;
}

dxl2_parser_handle dxl2_parser_create_static(uint8_t *block, size_t size)
{
	__ASSERT(size >= sizeof(dxl2_parser_t), "Size of memory block should be at least %d", size);
	dxl2_parser_handle parser = (dxl2_parser_handle) block;
	dxl2_parser_reset_state(parser);
	parser->ping_pong_buffer_which = 0;
	return parser;
}

dxl_err_t dxl2_parser_set_match_callback(dxl2_parser_handle parser, dxl2_parser_match_callback callback)
{
	if(parser == NULL) {
		return DXL_PTR_ERROR;
	}
	parser->on_match = callback;
	return DXL_OK;
}

dxl_err_t dxl2_parser_set_arg(dxl2_parser_handle parser, void *arg)
{
	if(parser == NULL) {
		return DXL_PTR_ERROR;
	}

	parser->arg = arg;
	
	return DXL_OK;
}

dxl_err_t dxl2_parser_reset_state(dxl2_parser_handle parser)
{
	if(parser == NULL) {
		return DXL_PTR_ERROR;
	}
	
	parser->cursor = 0;
	parser->current_params_length = 0;
	parser->state = DXL2_PARSER_STATE_HEADER;
	parser->on_match = NULL;

	return DXL_OK;
}

dxl_err_t dxl2_parser_feed_bytes(dxl2_parser_handle parser, const uint8_t *buffer, size_t length)
{
	if(parser == NULL) {
		return DXL_PTR_ERROR;
	}
	if(buffer == NULL && length > 0) {
		return DXL_PTR_ERROR;
	}

	uint8_t* output_buffer = dxl2_parser_get_buffer(parser);

	for (size_t i = 0; i < length; ++i) {
		output_buffer[parser->cursor] = buffer[i];

		switch (parser->state) {
		case DXL2_PARSER_STATE_HEADER: {
			const uint8_t header[] = DXL2_HEADER;
			uint8_t header_byte = header[parser->cursor];

			if (buffer[i] != header_byte) {
				dxl2_parser_reset_state(parser);
			} else if (parser->cursor == DXL2_STATUS_PACKET_ID_POS - 1) {
				parser->state = DXL2_PARSER_STATE_LENGTH;
			}
			break;
		}
		case DXL2_PARSER_STATE_ID: {
			parser->state = DXL2_PARSER_STATE_LENGTH;
			break;
		}
		case DXL2_PARSER_STATE_LENGTH: {
			parser->current_params_length =
				(parser->current_params_length << 8) | buffer[i];

			if (parser->cursor == DXL2_STATUS_PACKET_INST_POS - 1) {
				parser->state = DXL2_PARSER_STATE_SEEK_END;
			}
			break;
		}
		case DXL2_PARSER_STATE_SEEK_END: {
			size_t expected_packet_length =
				DXL2_STATUS_PACKET_LEN(parser->current_params_length);
			if (parser->cursor == expected_packet_length - 1) {
				if (parser->on_match != NULL) {
					parser->on_match(parser, output_buffer,
							 expected_packet_length, parser->arg);
				}
				dxl2_parser_reset_state(parser);
				dxl2_parser_switch_buffer(parser);
				break;
			}
		}
		}
		parser->cursor++;
	}

	return DXL_OK;
}