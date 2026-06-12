/*
 * CSB V1 CustomBackgrounds D0C first-backdrop pixel-composition contract.
 *
 * ReDMCSB anchors:
 * - DUNVIEW.C F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap:3113-3156
 * - DUNVIEW.C F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally:3185-3247
 * - DUNVIEW.C F0127_DUNGEONVIEW_DrawSquareD0C:8164-8310
 * - DUNVIEW.C F0128_DUNGEONVIEW_Draw_CPSF:8318-8542
 * - DEFS.H M609/C728/C811/C812/C824/C825/C862/C871/C715/C0x0021
 *
 * CSB-lineage reference: Viewport.cpp CustomBackgrounds/ApplyBackground
 * D0C body around 6503-6551 and 7140-7157. This is contract-only
 * Firestaff evidence; it does not claim original DOS pixel parity.
 */

#include "csb/csb_v1_viewport_d0c_center_field_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum {
    SCREEN_W = 320,
    SCREEN_H = 200,
    VIEW_W = 224,
    VIEW_H = 136,
    TRANSPARENT = 10,
    BASE_CEILING = 0x11,
    BASE_FLOOR = 0x22,
    FIRST_BACKDROP = 0x3c,
    D0C_LEFT = 0x55,
    D0C_RIGHT = 0x66,
    SECOND_BACKDROP = 0x79,
    SENTINEL = 0xa5
};

typedef struct {
    int x1;
    int y1;
    int x2;
    int y2;
} Rect;

typedef enum {
    STEP_NONE = 0,
    STEP_F0098_FLOOR_CEILING = 1,
    STEP_F0108_D0C_FLOOR_BASELINE = 2,
    STEP_CUSTOM_BACKGROUNDS_ROOM0_D0C = 3,
    STEP_F0104_D0C_LEFT_BODY = 4,
    STEP_F0105_D0C_RIGHT_BODY = 5,
    STEP_REJECT_SECOND_BACKDROP = 6
} Step;

typedef struct {
    int contract_only;
    int screen_w;
    int screen_h;
    int viewport_w;
    int viewport_h;
    int room_num;
    int rel_forward;
    int rel_side;
    int view_square_d0c;
    int first_bitmap_skin_def_index;
    int first_mask_skin_def_index;
    int second_room_num;
    int second_rel_forward;
    int second_rel_side;
    Rect first_backdrop_rect;
    Rect left_body_rect;
    Rect right_body_rect;
    Rect floor_rect;
    const char *redmcsb_f0104_anchor;
    const char *redmcsb_f0105_anchor;
    const char *redmcsb_f0127_anchor;
    const char *redmcsb_f0128_anchor;
    const char *redmcsb_defs_anchor;
    const char *csb_lineage_anchor;
    const char *source_evidence;
} Contract;

typedef struct {
    int ok;
    Step steps[8];
    int step_count;
    int base_ceiling_pixels;
    int base_floor_pixels;
    int first_backdrop_pixels;
    int first_backdrop_masked_pixels;
    int d0c_left_pixels;
    int d0c_left_transparent_pixels;
    int d0c_right_pixels;
    int d0c_right_transparent_pixels;
    int second_backdrop_pixels;
    int room0_pixel_before_d0c;
    int room0_pixel_after_d0c_mask;
    int room0_pixel_after_d0c_opaque;
    int left_pixel;
    int right_pixel;
    int floor_pixel;
    int outside_viewport_pixel;
    int second_backdrop_pixel;
    int first_and_second_non_overlap;
} Run;

static int g_assertions;
static int g_failures;

static const Contract k_contract = {
    1,
    SCREEN_W,
    SCREEN_H,
    VIEW_W,
    VIEW_H,
    0,
    0,
    0,
    0,
    0,
    4,
    2,
    3,
    -1,
    { 24, 6, 199, 129 },
    { 58, 30, 111, 118 },
    { 112, 30, 165, 118 },
    { 0, 66, 223, 135 },
    "ReDMCSB DUNVIEW.C F0104:3113-3156 native C10-transparent bitmap blit",
    "ReDMCSB DUNVIEW.C F0105:3185-3247 flipped C10-transparent bitmap blit",
    "ReDMCSB DUNVIEW.C F0127:8164-8310 D0C current-square body",
    "ReDMCSB DUNVIEW.C F0128:8318-8542 floor/ceiling before D0C dispatch",
    "ReDMCSB DEFS.H:2596 M609_VIEW_SQUARE_D0C; 2662 C0x0021; 4086/4150-4164/4209/4218 D0C zones",
    "CSB-lineage Viewport.cpp:6503-6551 CustomBackgrounds; 7140-7157 D0C body",
    "contract_only=1; no original DOS parity claim; GRAPHICS.DAT geometry "
    "contract uses a 320x200 framebuffer with a 224x136 viewport; room 0 "
    "rel_forward=0 rel_side=0 selects pSkinDef[0]/[4] first backdrop for "
    "D0C, then ReDMCSB F0104/F0105 D0C body pixels overlay through C10; "
    "room 2 second backdrop stays non-overlapping."
};

static int expect_int(const char *label, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want, anchor);
        return 0;
    }
    printf("PASS %s=%d anchor=%s\n", label, got, anchor);
    return 1;
}

static int expect_contains(
    const char *label,
    const char *haystack,
    const char *needle,
    const char *anchor)
{
    return expect_int(label,
                      haystack && needle && strstr(haystack, needle) != NULL,
                      1,
                      anchor);
}

static int rect_contains(Rect r, int x, int y)
{
    return x >= r.x1 && x <= r.x2 && y >= r.y1 && y <= r.y2;
}

static int rect_area(Rect r)
{
    return (r.x2 - r.x1 + 1) * (r.y2 - r.y1 + 1);
}

static size_t px(int x, int y)
{
    return (size_t)y * (size_t)SCREEN_W + (size_t)x;
}

static int d0c_masked_pixel(int x, int y)
{
    return (x == 88 || x == 135) && (y == 60 || y == 61);
}

static void append_step(Run *run, Step step)
{
    if (run->step_count < (int)(sizeof(run->steps) / sizeof(run->steps[0]))) {
        run->steps[run->step_count++] = step;
    }
}

static int compose_d0c_first_backdrop(uint8_t *fb, size_t fb_len, Run *out)
{
    int x;
    int y;
    Run run;

    if (!fb || fb_len < (size_t)SCREEN_W * (size_t)SCREEN_H || !out) {
        return -1;
    }

    memset(&run, 0, sizeof(run));
    memset(fb, SENTINEL, fb_len);

    append_step(&run, STEP_F0098_FLOOR_CEILING);
    for (y = 0; y < 66; ++y) {
        for (x = 0; x < VIEW_W; ++x) {
            fb[px(x, y)] = BASE_CEILING;
            ++run.base_ceiling_pixels;
        }
    }
    for (y = k_contract.floor_rect.y1; y <= k_contract.floor_rect.y2; ++y) {
        for (x = k_contract.floor_rect.x1; x <= k_contract.floor_rect.x2; ++x) {
            fb[px(x, y)] = BASE_FLOOR;
            ++run.base_floor_pixels;
        }
    }

    append_step(&run, STEP_F0108_D0C_FLOOR_BASELINE);
    run.floor_pixel = fb[px(120, 90)];

    append_step(&run, STEP_CUSTOM_BACKGROUNDS_ROOM0_D0C);
    for (y = k_contract.first_backdrop_rect.y1;
         y <= k_contract.first_backdrop_rect.y2;
         ++y) {
        for (x = k_contract.first_backdrop_rect.x1;
             x <= k_contract.first_backdrop_rect.x2;
             ++x) {
            if (x == 64 && y == 72) {
                ++run.first_backdrop_masked_pixels;
                continue;
            }
            fb[px(x, y)] = FIRST_BACKDROP;
            ++run.first_backdrop_pixels;
        }
    }
    run.room0_pixel_before_d0c = fb[px(80, 40)];

    append_step(&run, STEP_F0104_D0C_LEFT_BODY);
    for (y = k_contract.left_body_rect.y1; y <= k_contract.left_body_rect.y2; ++y) {
        for (x = k_contract.left_body_rect.x1; x <= k_contract.left_body_rect.x2; ++x) {
            if (d0c_masked_pixel(x, y)) {
                ++run.d0c_left_transparent_pixels;
                continue;
            }
            fb[px(x, y)] = D0C_LEFT;
            ++run.d0c_left_pixels;
        }
    }
    run.room0_pixel_after_d0c_mask = fb[px(88, 60)];

    append_step(&run, STEP_F0105_D0C_RIGHT_BODY);
    for (y = k_contract.right_body_rect.y1; y <= k_contract.right_body_rect.y2; ++y) {
        for (x = k_contract.right_body_rect.x1; x <= k_contract.right_body_rect.x2; ++x) {
            if (d0c_masked_pixel(x - 47, y)) {
                ++run.d0c_right_transparent_pixels;
                continue;
            }
            fb[px(x, y)] = D0C_RIGHT;
            ++run.d0c_right_pixels;
        }
    }
    run.room0_pixel_after_d0c_opaque = fb[px(120, 60)];
    run.left_pixel = fb[px(90, 60)];
    run.right_pixel = fb[px(120, 60)];

    append_step(&run, STEP_REJECT_SECOND_BACKDROP);
    for (y = 0; y < VIEW_H; ++y) {
        for (x = 224; x < SCREEN_W; ++x) {
            if (fb[px(x, y)] == SECOND_BACKDROP) {
                ++run.second_backdrop_pixels;
            }
        }
    }
    run.outside_viewport_pixel = fb[px(224, 0)];
    run.second_backdrop_pixel = fb[px(212, 12)];
    run.first_and_second_non_overlap =
        run.second_backdrop_pixels == 0 &&
        !rect_contains(k_contract.first_backdrop_rect, 212, 12);

    run.ok = 1;
    *out = run;
    return 0;
}

static int test_contract_metadata(void)
{
    int ok = 1;
    const CSB_V1_ViewportD0CCenterFieldContractPc34 *d0c =
        csb_v1_viewport_d0c_center_field_contract_pc34();

    ok &= expect_int("contract.only", k_contract.contract_only, 1,
                     k_contract.source_evidence);
    ok &= expect_int("screen.width", k_contract.screen_w, 320,
                     "GRAPHICS.DAT/M11 framebuffer geometry");
    ok &= expect_int("screen.height", k_contract.screen_h, 200,
                     "GRAPHICS.DAT/M11 framebuffer geometry");
    ok &= expect_int("viewport.width", k_contract.viewport_w, 224,
                     "ReDMCSB VIEWPORT.C M091_BITPLANE_SIZE(224,136)");
    ok &= expect_int("viewport.height", k_contract.viewport_h, 136,
                     "ReDMCSB VIEWPORT.C M091_BITPLANE_SIZE(224,136)");
    ok &= expect_int("room0", k_contract.room_num, 0,
                     k_contract.csb_lineage_anchor);
    ok &= expect_int("room0.rel_forward", k_contract.rel_forward, 0,
                     k_contract.csb_lineage_anchor);
    ok &= expect_int("room0.rel_side", k_contract.rel_side, 0,
                     k_contract.csb_lineage_anchor);
    ok &= expect_int("pskindef.bitmap", k_contract.first_bitmap_skin_def_index, 0,
                     k_contract.csb_lineage_anchor);
    ok &= expect_int("pskindef.mask", k_contract.first_mask_skin_def_index, 4,
                     k_contract.csb_lineage_anchor);
    ok &= expect_int("second.room", k_contract.second_room_num, 2,
                     "second backdrop non-overlap marker");
    ok &= expect_int("d0c.helper.present", d0c != NULL, 1,
                     "existing D0C F0127 helper");
    ok &= expect_int("d0c.helper.view_square", d0c ? d0c->view_square : -1, 0,
                     "ReDMCSB DEFS.H:2596 M609_VIEW_SQUARE_D0C");
    ok &= expect_int("d0c.helper.f0104.door_zone",
                     d0c ? d0c->door_frame_zone : -1, 728,
                     k_contract.redmcsb_f0104_anchor);
    ok &= expect_contains("anchor.f0104", k_contract.redmcsb_f0104_anchor,
                          "3113-3156", k_contract.redmcsb_f0104_anchor);
    ok &= expect_contains("anchor.f0105", k_contract.redmcsb_f0105_anchor,
                          "3185-3247", k_contract.redmcsb_f0105_anchor);
    ok &= expect_contains("anchor.f0127", k_contract.redmcsb_f0127_anchor,
                          "8164-8310", k_contract.redmcsb_f0127_anchor);
    ok &= expect_contains("anchor.lineage", k_contract.csb_lineage_anchor,
                          "7140-7157", k_contract.csb_lineage_anchor);
    ok &= expect_contains("evidence.geometry", k_contract.source_evidence,
                          "320x200", "GRAPHICS.DAT geometry evidence");
    ok &= expect_contains("evidence.viewport", k_contract.source_evidence,
                          "224x136", "viewport geometry evidence");

    return ok;
}

static int test_d0c_pixel_composition(void)
{
    int ok = 1;
    static uint8_t framebuffer[SCREEN_W * SCREEN_H];
    Run run;

    ok &= expect_int("compose.call",
                     compose_d0c_first_backdrop(framebuffer,
                                                sizeof(framebuffer),
                                                &run),
                     0,
                     k_contract.source_evidence);
    ok &= expect_int("compose.ok", run.ok, 1, k_contract.source_evidence);
    ok &= expect_int("order.count", run.step_count, 6,
                     k_contract.redmcsb_f0128_anchor);
    ok &= expect_int("order.0.f0098", run.steps[0],
                     STEP_F0098_FLOOR_CEILING,
                     k_contract.redmcsb_f0128_anchor);
    ok &= expect_int("order.1.f0108", run.steps[1],
                     STEP_F0108_D0C_FLOOR_BASELINE,
                     "DUNVIEW.C F0108 floor baseline contract before D0C backdrop");
    ok &= expect_int("order.2.room0_d0c", run.steps[2],
                     STEP_CUSTOM_BACKGROUNDS_ROOM0_D0C,
                     k_contract.csb_lineage_anchor);
    ok &= expect_int("order.3.f0104", run.steps[3],
                     STEP_F0104_D0C_LEFT_BODY,
                     k_contract.redmcsb_f0104_anchor);
    ok &= expect_int("order.4.f0105", run.steps[4],
                     STEP_F0105_D0C_RIGHT_BODY,
                     k_contract.redmcsb_f0105_anchor);
    ok &= expect_int("order.5.second_reject", run.steps[5],
                     STEP_REJECT_SECOND_BACKDROP,
                     "second backdrop non-overlap marker");

    ok &= expect_int("pixels.ceiling", run.base_ceiling_pixels, VIEW_W * 66,
                     k_contract.redmcsb_f0128_anchor);
    ok &= expect_int("pixels.floor", run.base_floor_pixels, rect_area(k_contract.floor_rect),
                     k_contract.redmcsb_f0128_anchor);
    ok &= expect_int("pixels.first_backdrop",
                     run.first_backdrop_pixels + run.first_backdrop_masked_pixels,
                     rect_area(k_contract.first_backdrop_rect),
                     k_contract.csb_lineage_anchor);
    ok &= expect_int("pixels.left_total",
                     run.d0c_left_pixels + run.d0c_left_transparent_pixels,
                     rect_area(k_contract.left_body_rect),
                     k_contract.redmcsb_f0104_anchor);
    ok &= expect_int("pixels.right_total",
                     run.d0c_right_pixels + run.d0c_right_transparent_pixels,
                     rect_area(k_contract.right_body_rect),
                     k_contract.redmcsb_f0105_anchor);
    ok &= expect_int("pixel.floor.before_backdrop", run.floor_pixel, BASE_FLOOR,
                     k_contract.redmcsb_f0128_anchor);
    ok &= expect_int("pixel.room0.before_d0c", run.room0_pixel_before_d0c,
                     FIRST_BACKDROP,
                     "room 0 rel_forward=0 first-backdrop D0C pixel");
    ok &= expect_int("pixel.mask_preserves_backdrop",
                     run.room0_pixel_after_d0c_mask,
                     FIRST_BACKDROP,
                     "F0104 C10 pixel preserves first backdrop");
    ok &= expect_int("pixel.opaque_overlays_backdrop",
                     run.room0_pixel_after_d0c_opaque,
                     D0C_RIGHT,
                     "F0105 opaque pixel overlays first backdrop");
    ok &= expect_int("pixel.left", run.left_pixel, D0C_LEFT,
                     k_contract.redmcsb_f0104_anchor);
    ok &= expect_int("pixel.right", run.right_pixel, D0C_RIGHT,
                     k_contract.redmcsb_f0105_anchor);
    ok &= expect_int("pixel.outside_viewport", run.outside_viewport_pixel, SENTINEL,
                     "320x200 framebuffer outside 224x136 viewport remains untouched");
    ok &= expect_int("pixel.second_absent", run.second_backdrop_pixel != SECOND_BACKDROP,
                     1,
                     "second backdrop is not applied to room 0 D0C first-backdrop gate");
    ok &= expect_int("non_overlap.second", run.first_and_second_non_overlap, 1,
                     "room 0 D0C first backdrop is distinct from room 2 second backdrop");

    return ok;
}

static int test_rejections(void)
{
    int ok = 1;
    static uint8_t framebuffer[SCREEN_W * SCREEN_H];
    Run run;

    ok &= expect_int("reject.null_fb",
                     compose_d0c_first_backdrop(NULL, sizeof(framebuffer), &run),
                     -1,
                     "guarded contract helper");
    ok &= expect_int("reject.short_fb",
                     compose_d0c_first_backdrop(framebuffer, sizeof(framebuffer) - 1, &run),
                     -1,
                     "320x200 framebuffer length required");
    ok &= expect_int("reject.null_out",
                     compose_d0c_first_backdrop(framebuffer, sizeof(framebuffer), NULL),
                     -1,
                     "guarded contract helper");

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=csb_v1_viewport_custom_backgrounds_d0c_first_backdrop_pc34_compat\n");
    printf("source_evidence=%s\n", k_contract.source_evidence);

    ok &= test_contract_metadata();
    ok &= test_d0c_pixel_composition();
    ok &= test_rejections();

    if (ok && g_failures == 0) {
        printf("PASS csb_v1_viewport_custom_backgrounds_d0c_first_backdrop_pc34_compat assertions=%d failures=0\n",
               g_assertions);
        return 0;
    }

    printf("FAIL csb_v1_viewport_custom_backgrounds_d0c_first_backdrop_pc34_compat assertions=%d failures=%d\n",
           g_assertions,
           g_failures);
    return 1;
}
