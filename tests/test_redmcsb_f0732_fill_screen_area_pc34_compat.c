#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f0732_fill_screen_area_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

typedef struct {
    int call_count;
    void *seen_context;
    uint8_t *bitmap;
    int16_t box[4];
    uint16_t color_bits;
    int16_t width;
} redmcsb_f0732_capture_pc34_compat;

static void capture_fill_box(void *context, uint8_t *bitmap, int16_t *box,
                             int16_t color, int16_t width)
{
    redmcsb_f0732_capture_pc34_compat *capture = context;

    capture->call_count++;
    capture->seen_context = context;
    capture->bitmap = bitmap;
    memcpy(capture->box, box, sizeof(capture->box));
    capture->color_bits = (uint16_t)color;
    capture->width = width;
}

int main(void)
{
    redmcsb_f0732_capture_pc34_compat capture = { 0 };
    redmcsb_f0732_video_driver_pc34_compat driver = {
        capture_fill_box,
        &capture
    };
    int16_t zone[4] = { 10, 20, 3, 2 };
    int16_t full_screen[4] = { 0, 0, 320, 200 };

    redmcsb_f0732_fill_screen_area_pc34_compat(&driver, zone, UINT16_C(0x800F));
    assert(capture.call_count == 1);
    assert(capture.seen_context == &capture);
    assert(capture.bitmap == NULL);
    assert(capture.box[0] == 10);
    assert(capture.box[1] == 12);
    assert(capture.box[2] == 20);
    assert(capture.box[3] == 21);
    assert(capture.color_bits == UINT16_C(0x800F));
    assert(capture.width == REDMCSB_F0732_SCREEN_PIXEL_WIDTH_PC34);

    redmcsb_f0732_fill_screen_area_pc34_compat(&driver, full_screen, 6U);
    assert(capture.call_count == 2);
    assert(capture.box[0] == 0);
    assert(capture.box[1] == 319);
    assert(capture.box[2] == 0);
    assert(capture.box[3] == 199);
    assert(capture.color_bits == 6U);

    assert(strstr(redmcsb_f0732_fill_screen_area_source_evidence_pc34(),
                  "BLITFILL.C:199-209") != NULL);

    puts("ok: ReDMCSB F0732 PC 3.4 fill-screen-area dispatch");
    return 0;
}
