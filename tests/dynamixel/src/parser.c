#include <zephyr/ztest.h>
#include "dynamixel/parser.h"

static const uint8_t status_packet[] = {0xFF, 0xFF, 0xFD, 0x00, 0x01, 0x07, 0x00,
					0x55, 0x00, 0x06, 0x04, 0x26, 0x65, 0x5D};
static const uint8_t status_packet_wrong_crc[] = {0xFF, 0xFF, 0xFD, 0x00, 0x01, 0x07, 0x00,
						  0x55, 0x00, 0x06, 0x04, 0x26, 0x65, 0x5A};
static size_t status_packet_length = sizeof(status_packet) / sizeof(status_packet[0]);

static void test_parser_match_callback(dxl_parser_handle parser, uint8_t *buffer, size_t size,
				       void *arg)
{
	uint8_t *hits = arg;
	(*hits)++;
}

ZTEST(dxl_tests, test_parser)
{
	dxl_err_t err = DXL_OK;
	uint8_t hits = 0;
	dxl_parser_handle parser = dxl_parser_create();
	zassert_true(parser != NULL);

	err = dxl_parser_set_match_callback(parser, test_parser_match_callback);
	zassert_equal(err, 0, "Failed to set callback");
	err = dxl_parser_set_arg(parser, &hits);
	zassert_equal(err, 0, "Failed to add arg to callback");

	// Parser hits when a valid message is incoming
	dxl_parser_feed_bytes(parser, status_packet, status_packet_length);
	zassert_equal(hits, 1, "Match on valid message failed");
	dxl_parser_feed_bytes(parser, status_packet, status_packet_length);
	zassert_equal(hits, 2, "Match on valid message repeat failed");

	// Parser hits with wrong CRC (error detection happens afterwards)
    dxl_parser_feed_bytes(parser, status_packet_wrong_crc, status_packet_length);
	zassert_equal(hits, 3, "Match on message with wrong CRC failed");

    // Parser doesn't hit with incomplete message (which is the second one)
    dxl_parser_feed_bytes(parser, status_packet, status_packet_length - 5);
    dxl_parser_feed_bytes(parser, status_packet, status_packet_length - 3);
    zassert_equal(hits, 4, "Match happened on incomplete message");

    // Parser hits when receiving a message after a reset
    dxl_parser_reset_state(parser);
    dxl_parser_feed_bytes(parser, status_packet, status_packet_length);
	zassert_equal(hits, 5, "Match failed after reset");
}