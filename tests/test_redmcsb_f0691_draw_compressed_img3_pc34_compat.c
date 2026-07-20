#include "redmcsb_f0691_draw_compressed_img3_pc34_compat.h"
#include "redmcsb_f0690_copy_pixel_line_to_screen_pc34_compat.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

struct capture {
    uint8_t lines[4][16];
    size_t destinations[4];
    size_t widths[4];
    size_t count;
};

struct driver_capture {
    size_t call_count;
    uint16_t source_x;
    uint16_t destination;
    int16_t count;
};

static void capture_driver_line(void *context,
                                const uint8_t *packed_pixel_line,
                                uint16_t source_x,
                                uint16_t destination,
                                int16_t count)
{
    struct driver_capture *capture = context;

    /* NDEBUG keeps the parameter assert-only; mark it used for -Werror. */
    (void)packed_pixel_line;
    assert(packed_pixel_line != NULL);
    capture->call_count++;
    capture->source_x = source_x;
    capture->destination = destination;
    capture->count = count;
}

static void capture_line(void *context,
                         const uint8_t *packed_pixel_line,
                         size_t destination_pixel_index,
                         size_t pixel_count)
{
    struct capture *capture = context;

    assert(capture->count < 4U);
    memcpy(capture->lines[capture->count], packed_pixel_line,
           (pixel_count + 1U) / 2U);
    capture->destinations[capture->count] = destination_pixel_index;
    capture->widths[capture->count] = pixel_count;
    capture->count++;
}

static void test_palette_direct_skip_and_row_wrap(void)
{
    /* width=3, height=2; palette={1,2,3,4,5,6}; commands:
     * 0,1,2 then skip, literal(9),5. */
    static const uint8_t graphic[] = {
        3, 0, 2, 0, 0x12, 0x34, 0x56, 0x01, 0x26, 0x79, 0x50
    };
    uint8_t pixel_line[2] = { 0xab, 0xcd };
    struct capture capture = { 0 };

    (void)graphic; (void)pixel_line; (void)capture; /* NDEBUG assert-only */
    assert(redmcsb_f0691_draw_compressed_img3_pc34_compat(
        graphic, sizeof(graphic), 10, 20, pixel_line, sizeof(pixel_line),
        capture_line, &capture));
    assert(capture.count == 2U);
    assert(capture.destinations[0] == 6410U);
    assert(capture.destinations[1] == 6730U);
    assert(capture.widths[0] == 3U && capture.widths[1] == 3U);
    assert(capture.lines[0][0] == 0x12U && capture.lines[0][1] == 0x3dU);
    /* F0691 keeps the shared pixel line: command 6 leaves the prior high
     * nibble (palette color 1), rather than introducing a transparent fill. */
    assert(capture.lines[1][0] == 0x19U && capture.lines[1][1] == 0x6dU);
}

static void test_extended_rle_count_spans_complete_rows(void)
{
    /* F0688: 15,15,15,0,0,1,2 decodes the long count 18. */
    static const uint8_t graphic[] = {
        18, 0, 1, 0, 0x12, 0x34, 0x56, 0x8f, 0xff, 0x00, 0x12
    };
    uint8_t pixel_line[9] = { 0 };
    struct capture capture = { 0 };

    (void)graphic; (void)capture; /* NDEBUG assert-only */
    assert(redmcsb_f0691_draw_compressed_img3_pc34_compat(
        graphic, sizeof(graphic), 0, 0, pixel_line, sizeof(pixel_line),
        capture_line, &capture));
    assert(capture.count == 1U);
    assert(capture.widths[0] == 18U);
    for (size_t index = 0U; index < sizeof(pixel_line); index++) {
        assert(capture.lines[0][index] == 0x11U);
    }
}

static void test_rejects_malformed_stream_without_sink(void)
{
    static const uint8_t graphic[] = {
        2, 0, 1, 0, 0x12, 0x34, 0x56, 0x08
    };
    uint8_t pixel_line[1] = { 0xa5 };
    struct capture capture = { 0 };

    (void)graphic; (void)pixel_line; (void)capture; /* NDEBUG assert-only */
    assert(!redmcsb_f0691_draw_compressed_img3_pc34_compat(
        graphic, sizeof(graphic), 0, 0, pixel_line, sizeof(pixel_line),
        capture_line, &capture));
    assert(capture.count == 0U);
    assert(pixel_line[0] == 0xa5U);
}

static void test_f0690_forwards_pc34_video_driver_arguments(void)
{
    uint8_t pixel_line[1] = { 0x12 };
    struct driver_capture capture = { 0 };
    RedmcsbF0690VideoDriverPc34Compat driver = {
        capture_driver_line, &capture
    };

    redmcsb_f0690_copy_pixel_line_to_screen_pc34_compat(
        &driver, pixel_line, 964U, 2);
    assert(capture.call_count == 1U);
    assert(capture.source_x == 4U);
    assert(capture.destination == 964U);
    assert(capture.count == 2);
}

int main(void)
{
    (void)capture_line; /* referenced inside asserts only under NDEBUG */
    test_palette_direct_skip_and_row_wrap();
    test_extended_rle_count_spans_complete_rows();
    test_rejects_malformed_stream_without_sink();
    test_f0690_forwards_pc34_video_driver_arguments();
    return 0;
}
