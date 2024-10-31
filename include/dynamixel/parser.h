#ifndef DYNAMIXEL_PARSER_H
#define DYNAMIXEL_PARSER_H

#include <stddef.h>
#include <stdint.h>

#include "dynamixel/dynamixel.h"

typedef struct dxl2_parser* dxl2_parser_handle;

typedef void(*dxl2_parser_match_callback)(dxl2_parser_handle, uint8_t*, size_t, void*);

dxl2_parser_handle dxl2_parser_create();

dxl2_parser_handle dxl2_parser_create_static(uint8_t* block, size_t size);

dxl_err_t dxl2_parser_set_match_callback(dxl2_parser_handle parser, dxl2_parser_match_callback callback);

dxl_err_t dxl2_parser_set_arg(dxl2_parser_handle parser, void* arg);

dxl_err_t dxl2_parser_reset_state(dxl2_parser_handle parser);

dxl_err_t dxl2_parser_feed_bytes(dxl2_parser_handle parser, const uint8_t* buffer, size_t length);

#endif