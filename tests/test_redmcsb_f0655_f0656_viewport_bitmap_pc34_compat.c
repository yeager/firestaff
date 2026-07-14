#include "redmcsb_f0655_f0656_viewport_bitmap_pc34_compat.h"

#include <stdio.h>
#include <string.h>

typedef struct test_bitmap {
    int16_t width;
    int16_t height;
    uint8_t pixels[16];
} test_bitmap;

typedef struct blit_capture {
    int calls;
    const uint8_t *source;
    uint8_t *destination;
    int16_t xyz[4];
    int16_t x;
    int16_t y;
    int16_t source_width;
    int16_t destination_width;
    int16_t transparent;
    int16_t flip;
    int resolve_calls;
    int resolve_result;
} blit_capture;

static void capture_blit(void *context, const uint8_t *source,
                         uint8_t *destination, const int16_t xyz[4],
                         int16_t x, int16_t y, int16_t source_width,
                         int16_t destination_width, int16_t transparent,
                         int16_t flip)
{
    blit_capture *capture = (blit_capture *)context;

    capture->calls += 1;
    capture->source = source;
    capture->destination = destination;
    memcpy(capture->xyz, xyz, sizeof(capture->xyz));
    capture->x = x;
    capture->y = y;
    capture->source_width = source_width;
    capture->destination_width = destination_width;
    capture->transparent = transparent;
    capture->flip = flip;
}

static int resolve_zone(void *context, const uint8_t *bitmap, int16_t xyz[4],
                        int16_t zone, int16_t *x, int16_t *y)
{
    blit_capture *capture = (blit_capture *)context;

    (void)bitmap;
    capture->resolve_calls += 1;
    if (!capture->resolve_result || zone != 701) return 0;
    xyz[0] = 11;
    xyz[1] = 12;
    xyz[2] = 13;
    xyz[3] = 14;
    *x = 3;
    *y = 5;
    return 1;
}

static int expect(int condition, const char *message)
{
    if (condition) return 1;
    fprintf(stderr, "FAIL: %s\n", message);
    return 0;
}

int main(void)
{
    test_bitmap source = {4, 3, {1, 2, 3, 4}};
    test_bitmap copy = {0, 0, {0}};
    test_bitmap viewport = {320, 136, {0}};
    blit_capture capture;
    redmcsb_f0655_f0656_renderer_pc34_compat renderer;
    int ok = 1;

    memset(&capture, 0, sizeof(capture));
    memset(&renderer, 0, sizeof(renderer));
    renderer.video_blit = capture_blit;
    renderer.resolve_viewport_zone = resolve_zone;
    renderer.context = &capture;
    renderer.viewport_bitmap = viewport.pixels;
    renderer.viewport_pixel_width = 320;

    ok &= expect(redmcsb_f0655_copy_bitmap_and_flip_pc34_compat(
                     &renderer, source.pixels, copy.pixels,
                     REDMCSB_F0655_F0656_PC34_FLIP_HORIZONTAL) == 1,
                 "F0655 dispatches");
    ok &= expect(copy.width == 4 && copy.height == 3,
                 "F0655 copies the four-byte dimensions prefix");
    ok &= expect(capture.calls == 1 && capture.source == source.pixels &&
                     capture.destination == copy.pixels,
                 "F0655 forwards source and destination");
    ok &= expect(capture.xyz[0] == 0 && capture.xyz[1] == 0 &&
                     capture.xyz[2] == 3 && capture.xyz[3] == 2,
                 "F0655 uses the source-dimension zone");
    ok &= expect(capture.x == 0 && capture.y == 0 &&
                     capture.source_width == 4 && capture.destination_width == 4 &&
                     capture.transparent == -1 && capture.flip == 1,
                 "F0655 forwards PC F0132 arguments");

    capture.calls = 0;
    capture.resolve_result = 1;
    ok &= expect(redmcsb_f0656_blit_bitmap_to_viewport_zone_with_transparency_pc34_compat(
                     &renderer, source.pixels, 701, 10) == 1,
                 "F0656 dispatches after F0635 success");
    ok &= expect(capture.resolve_calls == 1 && capture.calls == 1,
                 "F0656 calls F0635 once then F0132 once");
    ok &= expect(capture.destination == viewport.pixels && capture.x == 3 &&
                     capture.y == 5 && capture.source_width == 4 &&
                     capture.destination_width == 320 && capture.transparent == 10 &&
                     capture.flip == 0,
                 "F0656 forwards resolved zone and transparency");
    ok &= expect(capture.xyz[0] == 11 && capture.xyz[1] == 12 &&
                     capture.xyz[2] == 13 && capture.xyz[3] == 14,
                 "F0656 preserves F0635 XYZ output");

    capture.calls = 0;
    capture.resolve_result = 0;
    ok &= expect(redmcsb_f0656_blit_bitmap_to_viewport_zone_with_transparency_pc34_compat(
                     &renderer, source.pixels, 701, 10) == 0 && capture.calls == 0,
                 "F0656 does not blit when F0635 fails");
    ok &= expect(redmcsb_f0655_copy_bitmap_and_flip_pc34_compat(
                     NULL, source.pixels, copy.pixels, 0) == 0,
                 "F0655 rejects missing renderer");
    ok &= expect(strstr(redmcsb_f0655_f0656_viewport_bitmap_source_evidence_pc34(),
                        "BASE.C") != NULL,
                 "source evidence is available");
    return ok ? 0 : 1;
}
