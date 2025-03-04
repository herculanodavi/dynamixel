#ifndef DYNAMIXEL_ERROR_H
#define DYNAMIXEL_ERROR_H

// API Error codes, not status packet error codes
typedef enum {
	DXL_OK = 0,
	DXL_ERROR = 1,
	DXL_PTR_ERROR = 2,
	DXL_NOT_IMPLEMENTED = 3,
	DXL_MALFORMED_PACKET = 4
} dxl_err_t;

#endif