/*
 * test_m11_asset_loader_blit_pc34_compat.c
 *
 * CTest unit for the public M11 asset-loader blit helpers.
 * The loader expands GRAPHICS.DAT entries into M11_AssetSlot pixels, but
 * the compositing helpers operate on that plain slot shape.  This test
 * uses a synthetic slot so the base renderer contract is covered without
 * requiring user-supplied GRAPHICS.DAT.  An opt-in DM1 M648 lane consumes
 * FIRESTAFF_DM1_GRAPHICS_DAT when real PC34 media is available.
 *
 * Source of truth:
 *   - include/asset_loader_m11.h public blit API.
 *   - src/shared/asset_loader_m11.c clipping, nearest scaling, mirror,
 *     transparent-color, and replacement-color implementation.
 *   - ReDMCSB DUNVIEW.C:3916 champion-portrait extraction shape for
 *     M11_AssetLoader_BlitSubRectScaled().
 *
 * Honest scope: no live SDL presentation.  The opt-in lane verifies the
 * original M648 bitmap decode and its C10-transparent source-pixel blit.
 */

#include "asset_loader_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FB_W 12
#define FB_H 10
#define FB_SIZE (FB_W * FB_H)
#define SENTINEL 0xEEu

static int failures = 0;
static int checks = 0;

static void check_true(const char* name, int ok) {
    ++checks;
    if (!ok) {
        ++failures;
        printf("FAIL %s\n", name);
    } else {
        printf("PASS %s\n", name);
    }
}

static unsigned int fnv1a(const unsigned char* bytes, int count) {
    unsigned int h = 2166136261u;
    int i;
    for (i = 0; i < count; ++i) {
        h ^= (unsigned int)bytes[i];
        h *= 16777619u;
    }
    return h;
}

static void fill_fb(unsigned char* fb, unsigned char value) {
    memset(fb, value, FB_SIZE);
}

static int pixel_eq(const unsigned char* fb, int x, int y, unsigned char value) {
    return x >= 0 && x < FB_W && y >= 0 && y < FB_H &&
           fb[y * FB_W + x] == value;
}

static int rect_eq(const unsigned char* fb,
                   int dstX,
                   int dstY,
                   int w,
                   int h,
                   const unsigned char* expected,
                   int expectedStride) {
    int x, y;
    for (y = 0; y < h; ++y) {
        for (x = 0; x < w; ++x) {
            unsigned char actual = fb[(dstY + y) * FB_W + (dstX + x)];
            unsigned char want = expected[y * expectedStride + x];
            if (actual != want) {
                return 0;
            }
        }
    }
    return 1;
}

static int outside_rect_unchanged(const unsigned char* fb,
                                  int x0,
                                  int y0,
                                  int w,
                                  int h,
                                  unsigned char value) {
    int x, y;
    for (y = 0; y < FB_H; ++y) {
        for (x = 0; x < FB_W; ++x) {
            int inside = x >= x0 && x < x0 + w && y >= y0 && y < y0 + h;
            if (!inside && fb[y * FB_W + x] != value) {
                return 0;
            }
        }
    }
    return 1;
}

static void test_invalid_slots_do_not_mutate(const M11_AssetSlot* slot) {
    unsigned char fb[FB_SIZE];
    M11_AssetSlot invalid = *slot;
    unsigned int before;

    fill_fb(fb, SENTINEL);
    before = fnv1a(fb, FB_SIZE);
    M11_AssetLoader_Blit(NULL, fb, FB_W, FB_H, 1, 1, -1);
    M11_AssetLoader_BlitScaled(NULL, fb, FB_W, FB_H, 1, 1, 4, 4, -1);
    M11_AssetLoader_BlitScaledMirror(NULL, fb, FB_W, FB_H, 1, 1, 4, 4, -1);
    M11_AssetLoader_BlitScaledReplace(NULL, fb, FB_W, FB_H, 1, 1, 4, 4,
                                      -1, 9, 90, 10, 91);
    M11_AssetLoader_BlitScaledMirrorReplace(NULL, fb, FB_W, FB_H, 1, 1,
                                            4, 4, -1, 9, 90, 10, 91);
    M11_AssetLoader_BlitSubRectScaled(NULL, fb, FB_W, FB_H, 1, 1, 4, 4,
                                      0, 0, 2, 2, -1);
    check_true("invalid_null_slots_do_not_mutate", fnv1a(fb, FB_SIZE) == before);

    invalid.loaded = 0;
    M11_AssetLoader_Blit(&invalid, fb, FB_W, FB_H, 1, 1, -1);
    invalid.loaded = 1;
    invalid.pixels = NULL;
    M11_AssetLoader_BlitScaled(&invalid, fb, FB_W, FB_H, 1, 1, 4, 4, -1);
    M11_AssetLoader_BlitScaled(slot, fb, FB_W, FB_H, 1, 1, 0, 4, -1);
    M11_AssetLoader_BlitSubRectScaled(slot, fb, FB_W, FB_H, 1, 1, 4, 4,
                                      0, 0, 0, 2, -1);
    check_true("invalid_loaded_or_zero_dimensions_do_not_mutate",
               fnv1a(fb, FB_SIZE) == before);
}

static void test_base_blit_and_clipping(const M11_AssetSlot* slot,
                                        const unsigned char* pixels) {
    unsigned char fb[FB_SIZE];
    static const unsigned char clipped[9] = {
        8,  9, 10,
        13, 14, 15,
        18, 19, 20
    };

    fill_fb(fb, SENTINEL);
    M11_AssetLoader_Blit(slot, fb, FB_W, FB_H, 2, 3, -1);
    check_true("blit_full_slot_pixels_at_destination",
               rect_eq(fb, 2, 3, 5, 4, pixels, 5));
    check_true("blit_full_slot_preserves_outside_rect",
               outside_rect_unchanged(fb, 2, 3, 5, 4, SENTINEL));

    fill_fb(fb, SENTINEL);
    M11_AssetLoader_Blit(slot, fb, FB_W, FB_H, 2, 1, 0);
    check_true("blit_transparent_color_preserves_zero_pixel",
               pixel_eq(fb, 4, 1, SENTINEL));
    check_true("blit_transparent_color_keeps_neighbors",
               pixel_eq(fb, 3, 1, 2) && pixel_eq(fb, 5, 1, 4));

    fill_fb(fb, SENTINEL);
    M11_AssetLoader_Blit(slot, fb, FB_W, FB_H, -2, -1, -1);
    check_true("blit_negative_destination_clips_source_origin",
               rect_eq(fb, 0, 0, 3, 3, clipped, 3));
    check_true("blit_negative_destination_preserves_rest",
               outside_rect_unchanged(fb, 0, 0, 3, 3, SENTINEL));
}

static void test_region_blit(const M11_AssetSlot* slot) {
    unsigned char fb[FB_SIZE];
    static const unsigned char region[9] = {
        6,  7,  8,
        11, 12, 13,
        16, 17, 18
    };
    static const unsigned char clipped[4] = {
        13, 14,
        18, 19
    };

    fill_fb(fb, SENTINEL);
    M11_AssetLoader_BlitRegion(slot, -1, 1, 4, 3, fb, FB_W, FB_H, 1, 1, -1);
    check_true("blit_region_clips_negative_source_rect",
               rect_eq(fb, 1, 1, 3, 3, region, 3));
    check_true("blit_region_preserves_outside_rect",
               outside_rect_unchanged(fb, 1, 1, 3, 3, SENTINEL));

    fill_fb(fb, SENTINEL);
    M11_AssetLoader_BlitRegion(slot, 1, 1, 3, 3, fb, FB_W, FB_H, -1, -1, -1);
    check_true("blit_region_clips_negative_destination_rect",
               rect_eq(fb, 0, 0, 2, 2, clipped, 2));
}

static void test_scaled_and_mirror_blits(const M11_AssetSlot* slot) {
    unsigned char fb[FB_SIZE];

    fill_fb(fb, SENTINEL);
    M11_AssetLoader_BlitScaled(slot, fb, FB_W, FB_H, 0, 0, 10, 8, -1);
    check_true("scaled_nearest_expands_top_left_2x2",
               pixel_eq(fb, 0, 0, 1) && pixel_eq(fb, 1, 0, 1) &&
               pixel_eq(fb, 0, 1, 1) && pixel_eq(fb, 1, 1, 1));
    check_true("scaled_nearest_samples_exact_multiple_columns",
               pixel_eq(fb, 2, 0, 2) && pixel_eq(fb, 4, 0, 0) &&
               pixel_eq(fb, 8, 6, 20));
    check_true("scaled_nearest_preserves_outside_rect",
               outside_rect_unchanged(fb, 0, 0, 10, 8, SENTINEL));

    fill_fb(fb, SENTINEL);
    M11_AssetLoader_BlitScaled(slot, fb, FB_W, FB_H, 1, 1, 10, 8, 0);
    check_true("scaled_transparent_zero_preserves_target_pixel",
               pixel_eq(fb, 5, 1, SENTINEL));
    check_true("scaled_transparent_zero_keeps_later_pixels",
               pixel_eq(fb, 9, 7, 20));

    fill_fb(fb, SENTINEL);
    M11_AssetLoader_BlitScaledMirror(slot, fb, FB_W, FB_H, 0, 0, 10, 8, -1);
    check_true("scaled_mirror_reverses_first_row",
               pixel_eq(fb, 0, 0, 5) && pixel_eq(fb, 2, 0, 4) &&
               pixel_eq(fb, 4, 0, 0) && pixel_eq(fb, 8, 0, 1));
    check_true("scaled_mirror_reverses_last_row",
               pixel_eq(fb, 0, 6, 20) && pixel_eq(fb, 8, 6, 16));
}

static void test_replacement_blits(const M11_AssetSlot* slot) {
    unsigned char fb[FB_SIZE];

    fill_fb(fb, SENTINEL);
    M11_AssetLoader_BlitScaledReplace(slot, fb, FB_W, FB_H, 0, 0, 5, 4,
                                      0, 9, 90, 10, 91);
    check_true("scaled_replace_remaps_configured_colors",
               pixel_eq(fb, 3, 1, 90) && pixel_eq(fb, 4, 1, 91));
    check_true("scaled_replace_applies_transparency_before_replacement",
               pixel_eq(fb, 2, 0, SENTINEL));
    check_true("scaled_replace_leaves_unmatched_pixels",
               pixel_eq(fb, 0, 1, 6) && pixel_eq(fb, 4, 3, 20));

    fill_fb(fb, SENTINEL);
    M11_AssetLoader_BlitScaledMirrorReplace(slot, fb, FB_W, FB_H, 0, 0,
                                            5, 4, -1, 9, 90, 10, 91);
    check_true("scaled_mirror_replace_mirrors_then_remaps",
               pixel_eq(fb, 0, 1, 91) && pixel_eq(fb, 1, 1, 90) &&
               pixel_eq(fb, 4, 1, 6));
}

static void test_subrect_scaled(const M11_AssetSlot* slot) {
    unsigned char fb[FB_SIZE];

    fill_fb(fb, SENTINEL);
    M11_AssetLoader_BlitSubRectScaled(slot, fb, FB_W, FB_H, 2, 2, 6, 4,
                                      1, 1, 3, 2, -1);
    check_true("subrect_scaled_extracts_source_window_top_row",
               pixel_eq(fb, 2, 2, 7) && pixel_eq(fb, 3, 2, 7) &&
               pixel_eq(fb, 4, 2, 8) && pixel_eq(fb, 6, 2, 9));
    check_true("subrect_scaled_extracts_source_window_bottom_row",
               pixel_eq(fb, 2, 4, 12) && pixel_eq(fb, 4, 4, 13) &&
               pixel_eq(fb, 6, 4, 14));
    check_true("subrect_scaled_preserves_outside_rect",
               outside_rect_unchanged(fb, 2, 2, 6, 4, SENTINEL));

    fill_fb(fb, SENTINEL);
    M11_AssetLoader_BlitSubRectScaled(slot, fb, FB_W, FB_H, -1, -1, 4, 4,
                                      3, 2, 5, 4, -1);
    check_true("subrect_scaled_clips_destination_and_skips_source_oob",
               pixel_eq(fb, 0, 0, 20) && pixel_eq(fb, 1, 0, SENTINEL) &&
               pixel_eq(fb, 2, 0, SENTINEL));
}

static void test_dm1_m648_real_graphics_dat(void) {
    const char* graphicsPath = getenv("FIRESTAFF_DM1_GRAPHICS_DAT");
    M11_AssetLoader loader;
    const M11_AssetSlot* m648;
    unsigned char framebuffer[288 * 8];
    unsigned long opaqueCount = 0;
    int pixelsMatch = 1;
    int i;

    if (!graphicsPath || !graphicsPath[0]) {
        printf("SKIP dm1_m648_real_graphics_dat (FIRESTAFF_DM1_GRAPHICS_DAT unset)\n");
        return;
    }
    memset(&loader, 0, sizeof(loader));
    check_true("dm1_m648_real.init", M11_AssetLoader_Init(&loader, graphicsPath));
    if (!M11_AssetLoader_IsReady(&loader)) {
        return;
    }
    m648 = M11_AssetLoader_Load(&loader, 258U);
    check_true("dm1_m648_real.load_index_258", m648 != NULL);
    if (!m648) {
        M11_AssetLoader_Shutdown(&loader);
        return;
    }
    check_true("dm1_m648_real.dimensions", m648->width == 288 && m648->height == 8);
    if (m648->width != 288 || m648->height != 8) {
        M11_AssetLoader_Shutdown(&loader);
        return;
    }
    memset(framebuffer, SENTINEL, sizeof(framebuffer));
    M11_AssetLoader_Blit(m648, framebuffer, 288, 8, 0, 0, 10);
    for (i = 0; i < (int)sizeof(framebuffer); ++i) {
        if (m648->pixels[i] == 10) {
            if (framebuffer[i] != SENTINEL) {
                pixelsMatch = 0;
            }
        } else {
            ++opaqueCount;
            if (framebuffer[i] != m648->pixels[i]) {
                pixelsMatch = 0;
            }
        }
    }
    ++checks;
    if (opaqueCount != 908) {
        ++failures;
        printf("FAIL dm1_m648_real.opaque_count got=%lu want=908\n", opaqueCount);
    } else {
        printf("PASS dm1_m648_real.opaque_count=908\n");
    }
    check_true("dm1_m648_real.pixel_blit_matches_graphics_dat", pixelsMatch);
    M11_AssetLoader_Shutdown(&loader);
}

int main(void) {
    static const unsigned char pixels[20] = {
        1,  2,  0,  4,  5,
        6,  7,  8,  9, 10,
        11, 12, 13, 14, 15,
        16, 17, 18, 19, 20
    };
    M11_AssetSlot slot;

    memset(&slot, 0, sizeof(slot));
    slot.loaded = 1;
    slot.graphicIndex = 123u;
    slot.width = 5;
    slot.height = 4;
    slot.pixels = (unsigned char*)pixels;

    test_invalid_slots_do_not_mutate(&slot);
    test_base_blit_and_clipping(&slot, pixels);
    test_region_blit(&slot);
    test_scaled_and_mirror_blits(&slot);
    test_replacement_blits(&slot);
    test_subrect_scaled(&slot);
    test_dm1_m648_real_graphics_dat();

    printf("# summary: %d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
