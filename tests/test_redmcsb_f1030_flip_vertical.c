#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1030_flip_vertical.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

typedef struct redmcsb_f1030_capture {
    unsigned int call_count;
    uint8_t *bitmap;
    int16_t pixel_width;
    int16_t pixel_height;
} redmcsb_f1030_capture;

static redmcsb_f1030_capture redmcsb_f1030_last_capture;

static void capture_flip_vertical(uint8_t *bitmap,
                                  int16_t pixel_width,
                                  int16_t pixel_height)
{
    redmcsb_f1030_last_capture.call_count++;
    redmcsb_f1030_last_capture.bitmap = bitmap;
    redmcsb_f1030_last_capture.pixel_width = pixel_width;
    redmcsb_f1030_last_capture.pixel_height = pixel_height;
}

int main(void)
{
    int16_t bitmap_storage[] = { 48, 19, 0, 0, 0, 0, 0, 0 };
    uint8_t *bitmap = (uint8_t *)(void *)&bitmap_storage[2];
    const char *evidence;
    (void)evidence;

    redmcsb_f1030_flip_vertical(capture_flip_vertical, bitmap);
    assert(redmcsb_f1030_last_capture.call_count == 1U);
    assert(redmcsb_f1030_last_capture.bitmap == bitmap);
    assert(redmcsb_f1030_last_capture.pixel_width == 48);
    assert(redmcsb_f1030_last_capture.pixel_height == 19);

    evidence = redmcsb_f1030_flip_vertical_source_evidence();
    assert(strstr(evidence, "BASE.C:1577-1580") != NULL);
    assert(strstr(evidence, "DEFS.H:3444-3445") != NULL);
    assert(strstr(evidence, "FLIPVERT.C:12-20") != NULL);
    puts("ok: ReDMCSB F1030 vertical-flip dispatch");
    return 0;
}
