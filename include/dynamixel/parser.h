#ifndef DYNAMIXEL_PARSER_H
#define DYNAMIXEL_PARSER_H

#include <stddef.h>
#include <stdint.h>

#include "dynamixel/error.h"

typedef struct dxl_parser* dxl_parser_handle;

typedef void(*dxl_parser_match_callback)(dxl_parser_handle, uint8_t*, size_t, void*);

dxl_parser_handle dxl_parser_create();

dxl_parser_handle dxl_parser_create_static(void* block, size_t size);

size_t dxl_get_handle_size();

dxl_err_t dxl_parser_set_match_callback(dxl_parser_handle parser, dxl_parser_match_callback callback);

dxl_err_t dxl_parser_set_arg(dxl_parser_handle parser, void* arg);

dxl_err_t dxl_parser_reset_state(dxl_parser_handle parser);

dxl_err_t dxl_parser_feed_bytes(dxl_parser_handle parser, const uint8_t* buffer, size_t length);

#endif