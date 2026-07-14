#include "redmcsb_f0690_copy_pixel_line_to_screen_pc34_compat.h"

#include <assert.h>
#include <stdint.h>

typedef struct {
    const uint8_t *pixel_line;
    uint16_t destination_x;
    uint16_t destination_pixel_index;
    int16_t pixel_count;
    unsigned int calls;
} RedmcsbF0690CapturePc34Compat;

static void capture_copy(void *context,
                         const uint8_t *bitmap_pixel_line,
                         uint16_t destination_x,
                         uint16_t destination_pixel_index,
                         int16_t pixel_count)
{
    RedmcsbF0690CapturePc34Compat *capture = context;

    capture->pixel_line = bitmap_pixel_line;
    capture->destination_x = destination_x;
    capture->destination_pixel_index = destination_pixel_index;
    capture->pixel_count = pixel_count;
    capture->calls++;
}

int main(void)
{
    static const uint8_t pixel_line[] = { 0x12U, 0x34U, 0x56U };
    RedmcsbF0690CapturePc34Compat capture = { 0 };
    const RedmcsbF0690VideoDriverPc34Compat video_driver = {
        capture_copy,
        &capture
    };

    redmcsb_f0690_copy_pixel_line_to_screen_pc34_compat(
        &video_driver, pixel_line, 6410U, 37);

    assert(capture.calls == 1U);
    assert(capture.pixel_line == pixel_line);
    assert(capture.destination_x == 10U);
    assert(capture.destination_pixel_index == 6410U);
    assert(capture.pixel_count == 37);
    return 0;
}
