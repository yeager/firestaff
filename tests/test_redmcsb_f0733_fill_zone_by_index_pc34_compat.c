#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f0733_fill_zone_by_index_pc34_compat.h"

typedef struct {
    int get_zone_call_count;
    int fill_box_call_count;
    void *seen_context;
    int16_t seen_zone_index;
    int16_t *resolved_zone;
    int16_t *seen_zone;
    int16_t seen_color;
    int16_t seen_width;
    int16_t seen_height;
} redmcsb_f0733_capture_pc34_compat;

static int16_t *capture_get_zone(
    void *context,
    int16_t zone_index,
    int16_t zone_xyz[4])
{
    redmcsb_f0733_capture_pc34_compat *capture = context;

    capture->get_zone_call_count++;
    capture->seen_context = context;
    capture->seen_zone_index = zone_index;
    zone_xyz[0] = 1;
    zone_xyz[1] = 2;
    zone_xyz[2] = 3;
    zone_xyz[3] = 4;
    return capture->resolved_zone;
}

static void capture_fill_box(
    void *context,
    int16_t *zone_xyz,
    int16_t color,
    int16_t screen_pixel_width,
    int16_t screen_pixel_height)
{
    redmcsb_f0733_capture_pc34_compat *capture = context;

    capture->fill_box_call_count++;
    capture->seen_context = context;
    capture->seen_zone = zone_xyz;
    capture->seen_color = color;
    capture->seen_width = screen_pixel_width;
    capture->seen_height = screen_pixel_height;
}

int main(void)
{
    static int16_t resolved_zone[] = { 7, 11, 13, 17 };
    redmcsb_f0733_capture_pc34_compat capture = { 0 };
    redmcsb_f0733_graphics_pc34_compat graphics = {
        capture_get_zone,
        capture_fill_box,
        &capture
    };

    capture.resolved_zone = resolved_zone;
    redmcsb_f0733_fill_zone_by_index_pc34_compat(
        &graphics, INT16_MIN, INT16_MAX);

    assert(capture.get_zone_call_count == 1);
    assert(capture.fill_box_call_count == 1);
    assert(capture.seen_context == &capture);
    assert(capture.seen_zone_index == INT16_MIN);
    assert(capture.seen_zone == resolved_zone);
    assert(capture.seen_color == INT16_MAX);
    assert(capture.seen_width == INT16_C(320));
    assert(capture.seen_height == INT16_C(200));

    capture.resolved_zone = NULL;
    redmcsb_f0733_fill_zone_by_index_pc34_compat(&graphics, INT16_C(-1), 0);

    assert(capture.get_zone_call_count == 2);
    assert(capture.fill_box_call_count == 2);
    assert(capture.seen_zone_index == INT16_C(-1));
    assert(capture.seen_zone == NULL);
    assert(capture.seen_color == 0);
    assert(strstr(redmcsb_f0733_fill_zone_by_index_source_evidence_pc34(),
                  "BLITFILL.C:225-238") != NULL);

    puts("ok: ReDMCSB F0733 PC 3.4 zone lookup and screen fill dispatch");
    return 0;
}
