#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f0735_fill_viewport_box_pc34_compat.h"

typedef struct {
    int call_count;
    void *seen_context;
    int16_t *seen_xyz;
    int16_t seen_color;
    int16_t seen_bitmap_pixel_width;
} redmcsb_f0735_capture_pc34_compat;

static void capture_fill_box(
    void *context,
    int16_t *xyz,
    int16_t color,
    int16_t bitmap_pixel_width)
{
    redmcsb_f0735_capture_pc34_compat *capture = context;

    capture->call_count++;
    capture->seen_context = context;
    capture->seen_xyz = xyz;
    capture->seen_color = color;
    capture->seen_bitmap_pixel_width = bitmap_pixel_width;
}

int main(void)
{
    int16_t xyz[] = { INT16_C(-4), INT16_C(8), INT16_C(219), INT16_C(135) };
    redmcsb_f0735_capture_pc34_compat capture = { 0 };
    redmcsb_f0735_graphics_pc34_compat graphics = {
        capture_fill_box,
        &capture
    };

    redmcsb_f0735_fill_viewport_box_pc34_compat(
        &graphics, xyz, INT16_MAX);

    assert(capture.call_count == 1);
    assert(capture.seen_context == &capture);
    assert(capture.seen_xyz == xyz);
    assert(capture.seen_color == INT16_MAX);
    assert(capture.seen_bitmap_pixel_width == INT16_C(224));

    redmcsb_f0735_fill_viewport_box_pc34_compat(
        &graphics, NULL, INT16_MIN);

    assert(capture.call_count == 2);
    assert(capture.seen_xyz == NULL);
    assert(capture.seen_color == INT16_MIN);
    assert(capture.seen_bitmap_pixel_width == INT16_C(224));
    assert(strstr(redmcsb_f0735_fill_viewport_box_source_evidence_pc34(),
                  "F0735_FillViewportBox") != NULL);

    puts("ok: ReDMCSB F0735 PC 3.4 viewport fill dispatch");
    return 0;
}
