#include "dm1_v1_viewport_3d_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum { DST_W = 48, DST_H = 41, DST_SIZE = DST_W * DST_H };

static int expect_int(const char* label, int got, int want)
{
    if (got != want) {
        printf("FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    printf("ok %s=%d\n", label, got);
    return 1;
}

static int expect_pixel(const char* label, const uint8_t* pixels, int x, int y, int want)
{
    return expect_int(label, pixels[y * DST_W + x], want);
}

static int expect_all_else_seeded(const char* label,
                                  const uint8_t* pixels,
                                  int expected_x,
                                  int expected_y)
{
    for (int y = 0; y < DST_H; ++y) {
        for (int x = 0; x < DST_W; ++x) {
            if (x == expected_x && y == expected_y) continue;
            if (pixels[y * DST_W + x] != 0xFFu) {
                printf("FAIL %s changed x=%d y=%d value=%d\n",
                       label, x, y, pixels[y * DST_W + x]);
                return 0;
            }
        }
    }
    printf("ok %s\n", label);
    return 1;
}

static void clear_src(uint8_t* src, int count)
{
    for (int i = 0; i < count; ++i) src[i] = 10;
}

int main(void)
{
    uint8_t dst[DST_SIZE];
    uint8_t d3r[8 * 4];
    uint8_t d3c[8 * 4];
    uint8_t d2c[12 * 6];
    DM1_DoorButtonBitmapSpan spans[DM1_VIEW_DOOR_BUTTON_COUNT];
    const DM1_WallFrame *redmcsb_frame;
    int ok = 1;

    printf("probe=dm1_v1_door_button_viewport_pc34_compat\n");

    redmcsb_frame = dm1_v1_viewport_get_door_button_frame_pc34(1, DM1_VIEW_DOOR_BUTTON_D3R);
    ok &= expect_int("redmcsb D3R x1", redmcsb_frame ? redmcsb_frame->left_x : -1, 199);
    ok &= expect_int("redmcsb D3R y1", redmcsb_frame ? redmcsb_frame->top_y : -1, 41);
    redmcsb_frame = dm1_v1_viewport_get_door_button_frame_pc34(1, DM1_VIEW_DOOR_BUTTON_D3C);
    ok &= expect_int("redmcsb D3C x1", redmcsb_frame ? redmcsb_frame->left_x : -1, 136);
    redmcsb_frame = dm1_v1_viewport_get_door_button_frame_pc34(1, DM1_VIEW_DOOR_BUTTON_D2C);
    ok &= expect_int("redmcsb D2C x1", redmcsb_frame ? redmcsb_frame->left_x : -1, 144);

    clear_src(d3r, (int)sizeof(d3r));
    clear_src(d3c, (int)sizeof(d3c));
    clear_src(d2c, (int)sizeof(d2c));

    /* F0110 lines 4163 and 4204 select the C0/C1/C2 view coordinate set
     * and blit source X/Y 0/0.  Source X n therefore lands at x1+n. */
    d3r[1] = 0x31u;
    d3c[2 * 8 + 4] = 0x42u;
    d2c[3 * 12 + 2] = 0x53u;
    d2c[3 * 12 + 8] = 0x64u; /* outside the clipped destination span */

    memset(spans, 0, sizeof(spans));
    spans[DM1_VIEW_DOOR_BUTTON_D3R].frame = (DM1_WallFrame){ 40, 45, 2, 5, 8, 4, 0, 0 };
    spans[DM1_VIEW_DOOR_BUTTON_D3R].pixels = d3r;
    spans[DM1_VIEW_DOOR_BUTTON_D3R].source_width = 8;
    spans[DM1_VIEW_DOOR_BUTTON_D3R].source_height = 4;

    spans[DM1_VIEW_DOOR_BUTTON_D3C].frame = (DM1_WallFrame){ 12, 17, 9, 12, 8, 4, 0, 0 };
    spans[DM1_VIEW_DOOR_BUTTON_D3C].pixels = d3c;
    spans[DM1_VIEW_DOOR_BUTTON_D3C].source_width = 8;
    spans[DM1_VIEW_DOOR_BUTTON_D3C].source_height = 4;

    spans[DM1_VIEW_DOOR_BUTTON_D2C].frame = (DM1_WallFrame){ 44, 55, 35, 40, 12, 6, 0, 0 };
    spans[DM1_VIEW_DOOR_BUTTON_D2C].pixels = d2c;
    spans[DM1_VIEW_DOOR_BUTTON_D2C].source_width = 12;
    spans[DM1_VIEW_DOOR_BUTTON_D2C].source_height = 6;

    memset(dst, 0xFF, sizeof(dst));
    ok &= expect_int("D3R drawn count",
        dm1_v1_viewport_draw_door_button_pc34(dst, DST_W, DST_H, DST_W, 1,
                                              DM1_VIEW_DOOR_BUTTON_D3R, spans,
                                              DM1_VIEW_DOOR_BUTTON_COUNT),
        1);
    ok &= expect_pixel("D3R source x maps to x1+1", dst, 41, 2, 0x31);
    ok &= expect_pixel("D3R C10 transparent preserves seed", dst, 40, 2, 0xFF);
    ok &= expect_all_else_seeded("D3R outside blit untouched", dst, 41, 2);

    memset(dst, 0xFF, sizeof(dst));
    ok &= expect_int("D3C drawn count",
        dm1_v1_viewport_draw_door_button_pc34(dst, DST_W, DST_H, DST_W, 1,
                                              DM1_VIEW_DOOR_BUTTON_D3C, spans,
                                              DM1_VIEW_DOOR_BUTTON_COUNT),
        1);
    ok &= expect_pixel("D3C source x maps to x1+4", dst, 16, 11, 0x42);
    ok &= expect_pixel("D3C C10 transparent preserves seed", dst, 12, 9, 0xFF);
    ok &= expect_all_else_seeded("D3C outside blit untouched", dst, 16, 11);

    memset(dst, 0xFF, sizeof(dst));
    ok &= expect_int("D2C clipped drawn count",
        dm1_v1_viewport_draw_door_button_pc34(dst, DST_W, DST_H, DST_W, 1,
                                              DM1_VIEW_DOOR_BUTTON_D2C, spans,
                                              DM1_VIEW_DOOR_BUTTON_COUNT),
        1);
    ok &= expect_pixel("D2C source x maps to x1+2", dst, 46, 38, 0x53);
    ok &= expect_pixel("D2C clipped source remains outside", dst, 47, 38, 0xFF);
    ok &= expect_pixel("D2C C10 transparent preserves seed", dst, 44, 35, 0xFF);
    ok &= expect_all_else_seeded("D2C outside blit untouched", dst, 46, 38);

    if (!ok) return 1;
    printf("ok: DM1 V1 door-button viewport blit matches ReDMCSB F0110 C10 gate\n");
    return 0;
}
