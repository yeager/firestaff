#include "dm1_v1_ceiling_pit_viewport_pc34_compat.h"
#include "dm1_v1_explosion_bitmap_viewport_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int expect_int(const char* label, int got, int want)
{
    if (got != want) {
        printf("FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    printf("ok %s=%d\n", label, got);
    return 1;
}

static int expect_ptr_nonnull(const char* label, const void *ptr)
{
    if (!ptr) {
        printf("FAIL %s got=NULL\n", label);
        return 0;
    }
    printf("ok %s=non-null\n", label);
    return 1;
}

static int expect_ptr_null(const char* label, const void *ptr)
{
    if (ptr) {
        printf("FAIL %s got=non-null\n", label);
        return 0;
    }
    printf("ok %s=NULL\n", label);
    return 1;
}

static void fill_source(unsigned char *source, int width, int height, unsigned char base)
{
    int y;
    for (y = 0; y < height; ++y) {
        int x;
        for (x = 0; x < width; ++x) {
            source[y * width + x] = (unsigned char)(base + ((x + y) % 5));
        }
    }
    source[0] = DM1_V1_CEILING_PIT_TRANSPARENT_PC34;
}

static int untouched_outside_rect(const unsigned char *dest,
                                  int dest_width,
                                  int dest_height,
                                  int stride,
                                  const DM1V1CeilingPitViewportRectPc34 *rect,
                                  unsigned char sentinel)
{
    int y;
    for (y = 0; y < dest_height; ++y) {
        int x;
        for (x = 0; x < dest_width; ++x) {
            int inside = rect &&
                         x >= rect->x && x < rect->x + rect->width &&
                         y >= rect->y && y < rect->y + rect->height;
            if (!inside && dest[y * stride + x] != sentinel) {
                return 0;
            }
        }
    }
    return 1;
}

static int test_ceiling_pit_draws(void)
{
    enum { W = 224, H = 136, STRIDE = 224 };
    unsigned char dest[W * H];
    unsigned char source[96 * 5];
    const DM1V1CeilingPitViewportRectPc34 *rect;
    int ok = 1;
    int writes;

    printf("ceilingPitF0112=DUNVIEW.C:4341-4470\n");

    memset(dest, 0xEE, sizeof(dest));
    fill_source(source, 80, 5, 20);
    rect = dm1_v1_ceiling_pit_viewport_rect_pc34(DM1_V1_GRAPHIC_CEILING_PIT_D2L_PC34,
                                                  DM1_V1_ZONE_CEILING_PIT_D2L_PC34,
                                                  0);
    writes = dm1_v1_ceiling_pit_viewport_draw_pc34(DM1_V1_GRAPHIC_CEILING_PIT_D2L_PC34,
                                                   DM1_V1_ZONE_CEILING_PIT_D2L_PC34,
                                                   3,
                                                   4,
                                                   0,
                                                   dest,
                                                   W,
                                                   H,
                                                   STRIDE,
                                                   source,
                                                   80,
                                                   5);
    ok &= expect_int("D2L writes", writes, 399);
    ok &= expect_int("D2L transparent preserved", dest[rect->y * STRIDE + rect->x], 0xEE);
    ok &= expect_int("D2L first opaque", dest[rect->y * STRIDE + rect->x + 1], source[1]);
    ok &= expect_int("D2L outside untouched", untouched_outside_rect(dest, W, H, STRIDE, rect, 0xEE), 1);

    memset(dest, 0xEE, sizeof(dest));
    fill_source(source, 80, 5, 30);
    rect = dm1_v1_ceiling_pit_viewport_rect_pc34(DM1_V1_GRAPHIC_CEILING_PIT_D2L_PC34,
                                                  DM1_V1_ZONE_CEILING_PIT_D2R_PC34,
                                                  1);
    writes = dm1_v1_ceiling_pit_viewport_draw_pc34(DM1_V1_GRAPHIC_CEILING_PIT_D2L_PC34,
                                                   DM1_V1_ZONE_CEILING_PIT_D2R_PC34,
                                                   5,
                                                   6,
                                                   1,
                                                   dest,
                                                   W,
                                                   H,
                                                   STRIDE,
                                                   source,
                                                   80,
                                                   5);
    ok &= expect_int("D2R writes", writes, 399);
    ok &= expect_int("D2R flipped transparent preserved", dest[rect->y * STRIDE + rect->x + 79], 0xEE);
    ok &= expect_int("D2R first flipped opaque", dest[rect->y * STRIDE + rect->x], source[79]);
    ok &= expect_int("D2R outside untouched", untouched_outside_rect(dest, W, H, STRIDE, rect, 0xEE), 1);

    memset(dest, 0xEE, sizeof(dest));
    fill_source(source, 96, 5, 40);
    rect = dm1_v1_ceiling_pit_viewport_rect_pc34(DM1_V1_GRAPHIC_CEILING_PIT_D2C_PC34,
                                                  DM1_V1_ZONE_CEILING_PIT_D2C_PC34,
                                                  0);
    writes = dm1_v1_ceiling_pit_viewport_draw_pc34(DM1_V1_GRAPHIC_CEILING_PIT_D2C_PC34,
                                                   DM1_V1_ZONE_CEILING_PIT_D2C_PC34,
                                                   7,
                                                   8,
                                                   0,
                                                   dest,
                                                   W,
                                                   H,
                                                   STRIDE,
                                                   source,
                                                   96,
                                                   5);
    ok &= expect_int("D2C writes", writes, 479);
    ok &= expect_int("D2C transparent preserved", dest[rect->y * STRIDE + rect->x], 0xEE);
    ok &= expect_int("D2C outside untouched", untouched_outside_rect(dest, W, H, STRIDE, rect, 0xEE), 1);

    memset(dest, 0xEE, sizeof(dest));
    writes = dm1_v1_ceiling_pit_viewport_draw_pc34(999,
                                                   DM1_V1_ZONE_CEILING_PIT_D2L_PC34,
                                                   0,
                                                   0,
                                                   0,
                                                   dest,
                                                   W,
                                                   H,
                                                   STRIDE,
                                                   source,
                                                   80,
                                                   5);
    ok &= expect_int("unknown graphic no-op writes", writes, 0);
    ok &= expect_int("unknown graphic leaves dest", dest[19 * STRIDE], 0xEE);

    return ok;
}

static int test_explosion_lookup(void)
{
    int ok = 1;
    int byte_width = 0;
    int height = 0;
    const unsigned char *bitmap;

    printf("explosionBitmapF0114=DUNVIEW.C:4476-4530\n");

    bitmap = dm1_v1_explosion_bitmap_lookup_pc34(DM1_V1_EXPLOSION_ASPECT_FIRE_PC34,
                                                 16,
                                                 &byte_width,
                                                 &height,
                                                 0);
    ok &= expect_ptr_nonnull("fire scale16 bitmap", bitmap);
    ok &= expect_int("fire scale16 byte width", byte_width, 40);
    ok &= expect_int("fire scale16 height", height, 55);
    ok &= expect_int("fire scale16 derived index",
                     dm1_v1_explosion_bitmap_derived_index_pc34(DM1_V1_EXPLOSION_ASPECT_FIRE_PC34, 16),
                     6);

    bitmap = dm1_v1_explosion_bitmap_lookup_pc34(DM1_V1_EXPLOSION_ASPECT_FIRE_PC34,
                                                 16,
                                                 &byte_width,
                                                 &height,
                                                 1);
    ok &= expect_ptr_nonnull("fire scale16 flipped bitmap", bitmap);
    ok &= expect_int("fire scale16 flipped byte width", byte_width, 40);
    ok &= expect_int("fire scale16 flipped height", height, 55);

    bitmap = dm1_v1_explosion_bitmap_lookup_pc34(DM1_V1_EXPLOSION_ASPECT_SPELL_PC34,
                                                 64,
                                                 &byte_width,
                                                 &height,
                                                 0);
    ok &= expect_ptr_nonnull("spell scale clamped native bitmap", bitmap);
    ok &= expect_int("spell scale clamped byte width", byte_width, 64);
    ok &= expect_int("spell scale clamped height", height, 97);

    bitmap = dm1_v1_explosion_bitmap_lookup_pc34(99, 16, &byte_width, &height, 0);
    ok &= expect_ptr_null("out of range aspect", bitmap);
    ok &= expect_int("out of range byte width reset", byte_width, 0);
    ok &= expect_int("out of range height reset", height, 0);

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=dm1_v1_ceiling_pit_and_explosion_bitmap_viewport_pc34_compat\n");
    ok &= test_ceiling_pit_draws();
    ok &= test_explosion_lookup();

    printf("ceilingPitExplosionBitmapInvariantOk=%d\n", ok ? 1 : 0);
    return ok ? 0 : 1;
}
