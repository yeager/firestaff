#include "redmcsb_f1002_call_f0132_video_blit.h"
#include "redmcsb_f1032_hatch_box_pc34_compat.h"
#include "redmcsb_f1033_hatch_box.h"
#include "redmcsb_f1052_wait_for_scan_line_pc34_compat.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "check failed at %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        return 1; \
    } \
} while (0)

typedef struct {
    int calls;
    uint8_t *source;
    uint8_t *destination;
    int16_t *xyz;
    int16_t x;
    int16_t y;
    int16_t source_width;
    int16_t destination_width;
    int16_t transparent_color;
    int16_t flip;
} BlitCapture;

static BlitCapture g_blit_capture;

static void capture_blit(
    uint8_t *bitmap_source,
    uint8_t *bitmap_destination,
    int16_t *xyz,
    int16_t x,
    int16_t y,
    int16_t source_pixel_width,
    int16_t destination_pixel_width,
    int16_t transparent_color,
    int16_t flip)
{
    g_blit_capture.calls++;
    g_blit_capture.source = bitmap_source;
    g_blit_capture.destination = bitmap_destination;
    g_blit_capture.xyz = xyz;
    g_blit_capture.x = x;
    g_blit_capture.y = y;
    g_blit_capture.source_width = source_pixel_width;
    g_blit_capture.destination_width = destination_pixel_width;
    g_blit_capture.transparent_color = transparent_color;
    g_blit_capture.flip = flip;
}

static int test_f1002_source_named_wrapper_forwards_width_words(void)
{
    int16_t source_storage[] = { 13, 21, 0x1122, 0x3344 };
    int16_t destination_storage[] = { 34, 55, 0x5566, 0x7788 };
    int16_t xyz[] = { 1, 2, 3, 4 };
    uint8_t *source = (uint8_t *)&source_storage[2];
    uint8_t *destination = (uint8_t *)&destination_storage[2];

    memset(&g_blit_capture, 0, sizeof(g_blit_capture));
    F1002_Call_F0132_VIDEO_Blit(
        source,
        destination,
        xyz,
        7,
        9,
        15,
        1,
        capture_blit);

    CHECK(g_blit_capture.calls == 1);
    CHECK(g_blit_capture.source == source);
    CHECK(g_blit_capture.destination == destination);
    CHECK(g_blit_capture.xyz == xyz);
    CHECK(g_blit_capture.x == 7);
    CHECK(g_blit_capture.y == 9);
    CHECK(g_blit_capture.source_width == 13);
    CHECK(g_blit_capture.destination_width == 34);
    CHECK(g_blit_capture.transparent_color == 15);
    CHECK(g_blit_capture.flip == 1);
    return 0;
}

static int test_f1002_null_callback_is_noop(void)
{
    int16_t source_storage[] = { 13, 21, 0x1122 };
    int16_t destination_storage[] = { 34, 55, 0x5566 };

    memset(&g_blit_capture, 0, sizeof(g_blit_capture));
    F1002_Call_F0132_VIDEO_Blit(
        (uint8_t *)&source_storage[2],
        (uint8_t *)&destination_storage[2],
        0,
        0,
        0,
        0,
        0,
        0);
    CHECK(g_blit_capture.calls == 0);
    return 0;
}

typedef struct {
    int calls;
    uint8_t *screen_bitmap;
    int16_t *xyz;
    int16_t color;
    int16_t screen_pixel_width;
} HatchCapture;

static void capture_hatch(
    uint8_t *screen_bitmap,
    int16_t *xyz,
    int16_t color,
    int16_t screen_pixel_width)
{
    HatchCapture *capture = (HatchCapture *)screen_bitmap;

    capture->calls++;
    capture->screen_bitmap = screen_bitmap;
    capture->xyz = xyz;
    capture->color = color;
    capture->screen_pixel_width = screen_pixel_width;
}

static int test_f1032_pc34_boundary_fails_closed(void)
{
    CHECK(redmcsb_f1032_hatch_box_pc34_compat() == false);
    CHECK(F1032_GRF1_12_HatchBox() == false);
    return 0;
}

static int test_f1033_binds_screen_arguments_to_primitive(void)
{
    HatchCapture capture;
    int16_t xyz[] = { 2, 4, 6, 8 };

    memset(&capture, 0, sizeof(capture));
    F1033_HatchBox_Unreferenced(
        capture_hatch,
        (uint8_t *)&capture,
        xyz,
        11,
        320);

    CHECK(capture.calls == 1);
    CHECK(capture.screen_bitmap == (uint8_t *)&capture);
    CHECK(capture.xyz == xyz);
    CHECK(capture.color == 11);
    CHECK(capture.screen_pixel_width == 320);
    return 0;
}

static int test_f1033_null_primitive_is_noop(void)
{
    F1033_HatchBox_Unreferenced(0, 0, 0, 0, 0);
    return 0;
}

static int test_f1052_pc34_wait_is_noop(void)
{
    redmcsb_f1052_wait_for_scan_line_pc34_compat(123);
    F1052_WaitForScanLine(-4);
    return 0;
}

static int test_source_evidence_names_bundle(void)
{
    const char *f1002 = redmcsb_f1002_call_f0132_video_blit_source_evidence();
    const char *f1032 = redmcsb_f1032_hatch_box_source_evidence_pc34();
    const char *f1033 = redmcsb_f1033_hatch_box_source_evidence();
    const char *f1052 = redmcsb_f1052_wait_for_scan_line_source_evidence_pc34();

    CHECK(f1002 != 0);
    CHECK(strstr(f1002, "F1002_Call_F0132_VIDEO_Blit") != 0);
    CHECK(strstr(f1002, "BASE.C:1202") != 0);
    CHECK(f1032 != 0);
    CHECK(strstr(f1032, "F1032_GRF1_12_HatchBox") != 0);
    CHECK(strstr(f1032, "BLITFILL.C:287") != 0);
    CHECK(f1033 != 0);
    CHECK(strstr(f1033, "F1033_HatchBox_Unreferenced") != 0);
    CHECK(strstr(f1033, "BLITFILL.C:428") != 0);
    CHECK(f1052 != 0);
    CHECK(strstr(f1052, "F1052_WaitForScanLine") != 0);
    CHECK(strstr(f1052, "FILLBOX.C:17") != 0);
    return 0;
}

int main(void)
{
    CHECK(test_f1002_source_named_wrapper_forwards_width_words() == 0);
    CHECK(test_f1002_null_callback_is_noop() == 0);
    CHECK(test_f1032_pc34_boundary_fails_closed() == 0);
    CHECK(test_f1033_binds_screen_arguments_to_primitive() == 0);
    CHECK(test_f1033_null_primitive_is_noop() == 0);
    CHECK(test_f1052_pc34_wait_is_noop() == 0);
    CHECK(test_source_evidence_names_bundle() == 0);
    return 0;
}
