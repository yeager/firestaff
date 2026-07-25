#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1002_call_f0132_video_blit.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

typedef struct {
    int16_t pixel_width;
    int16_t pixel_height;
    uint8_t pixels[16];
} bitmap_storage;

_Static_assert(offsetof(bitmap_storage, pixels) == 2 * sizeof(int16_t),
               "test bitmap pixel data must follow the two-int16 header");

typedef struct {
    uint8_t *source;
    uint8_t *destination;
    int16_t *xyz;
    int16_t x;
    int16_t y;
    int16_t source_width;
    int16_t destination_width;
    int16_t transparent_color;
    int16_t flip;
    unsigned int call_count;
} blit_capture;

static blit_capture capture;

static void capture_video_blit(uint8_t *bitmap_source,
                               uint8_t *bitmap_destination,
                               int16_t *xyz,
                               int16_t x,
                               int16_t y,
                               int16_t source_pixel_width,
                               int16_t destination_pixel_width,
                               int16_t transparent_color,
                               int16_t flip)
{
    capture.source = bitmap_source;
    capture.destination = bitmap_destination;
    capture.xyz = xyz;
    capture.x = x;
    capture.y = y;
    capture.source_width = source_pixel_width;
    capture.destination_width = destination_pixel_width;
    capture.transparent_color = transparent_color;
    capture.flip = flip;
    ++capture.call_count;
}

int main(void)
{
    bitmap_storage source = { 37, 9, { 0U } };
    bitmap_storage destination = { 320, 200, { 0U } };
    int16_t xyz[] = { 11, 29, 13, 31 };
    const char *evidence;
    (void)evidence;

    redmcsb_f1002_call_f0132_video_blit(
        source.pixels, destination.pixels, xyz, -7, 12, -1, 0x4000,
        capture_video_blit);

    assert(capture.call_count == 1U);
    assert(capture.source == source.pixels);
    assert(capture.destination == destination.pixels);
    assert(capture.xyz == xyz);
    assert(capture.x == -7);
    assert(capture.y == 12);
    assert(capture.source_width == 37);
    assert(capture.destination_width == 320);
    assert(capture.transparent_color == -1);
    assert(capture.flip == 0x4000);

    evidence = redmcsb_f1002_call_f0132_video_blit_source_evidence();
    assert(strstr(evidence, "BASE.C:1202-1212") != NULL);
    assert(strstr(evidence, "MEDIA458_P20JA_P20JB") != NULL);
    assert(strstr(evidence, "DEFS.H:3444") != NULL);
    puts("ok: ReDMCSB F1002 F0132 blit forwarding");
    return 0;
}
