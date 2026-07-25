#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1033_hatch_box.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

typedef struct redmcsb_f1033_capture {
    unsigned int call_count;
    uint8_t *screen_bitmap;
    int16_t *xyz;
    int16_t color;
    int16_t screen_pixel_width;
} redmcsb_f1033_capture;

static redmcsb_f1033_capture redmcsb_f1033_last_capture;

static void capture_hatch_box(uint8_t *screen_bitmap,
                              int16_t *xyz,
                              int16_t color,
                              int16_t screen_pixel_width)
{
    redmcsb_f1033_last_capture.call_count++;
    redmcsb_f1033_last_capture.screen_bitmap = screen_bitmap;
    redmcsb_f1033_last_capture.xyz = xyz;
    redmcsb_f1033_last_capture.color = color;
    redmcsb_f1033_last_capture.screen_pixel_width = screen_pixel_width;
}

int main(void)
{
    uint8_t screen_bitmap[4] = { 0 };
    int16_t xyz[4] = { -3, 7, 18, 9 };
    const char *evidence;
    (void)evidence;

    redmcsb_f1033_hatch_box(capture_hatch_box, screen_bitmap, xyz, 14, 320);
    assert(redmcsb_f1033_last_capture.call_count == 1U);
    assert(redmcsb_f1033_last_capture.screen_bitmap == screen_bitmap);
    assert(redmcsb_f1033_last_capture.xyz == xyz);
    assert(redmcsb_f1033_last_capture.color == 14);
    assert(redmcsb_f1033_last_capture.screen_pixel_width == 320);
    assert(xyz[0] == -3);
    assert(xyz[1] == 7);
    assert(xyz[2] == 18);
    assert(xyz[3] == 9);

    evidence = redmcsb_f1033_hatch_box_source_evidence();
    assert(strstr(evidence, "BLITFILL.C:428-433") != NULL);
    assert(strstr(evidence, "FILLBOX.C:625-640") != NULL);
    puts("ok: ReDMCSB F1033 screen hatch-box dispatch");
    return 0;
}
