#include "dm1_f0135_video_fillbox_planar_20260714_pc34_compat.h"
#include "dm1_v1_viewport_planar_fill_material_pc34_compat.h"
#include "redmcsb_f0732_fill_screen_area_pc34_compat.h"
#include "redmcsb_f0733_fill_zone_by_index_pc34_compat.h"
#include "redmcsb_f0735_fill_viewport_box_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct FillSurfacePc34Compat {
    uint8_t *bitmap;
    size_t bitmap_size;
    size_t row_bytes;
    size_t pixel_height;
    int fill_result;
    int call_count;
    int16_t last_width;
} FillSurfacePc34Compat;

static int check(int condition, const char *label)
{
    if (condition) return 1;
    fprintf(stderr, "FAIL: %s\n", label);
    return 0;
}

static unsigned int pixel_color(const uint8_t *bitmap,
                                size_t row_bytes,
                                int x,
                                int y)
{
    const uint8_t *group = bitmap + (size_t)y * row_bytes +
                           (size_t)(x / 16) * 8u;
    const unsigned int mask = 0x8000u >> (x & 15);
    unsigned int color = 0;
    unsigned int plane;

    for (plane = 0; plane < 4u; ++plane) {
        const unsigned int word = ((unsigned int)group[plane * 2u] << 8) |
                                  group[plane * 2u + 1u];
        if ((word & mask) != 0u) color |= 1u << plane;
    }
    return color;
}

static void fill_screen_callback(void *context,
                                 uint8_t *ignored_bitmap,
                                 int16_t *box,
                                 int16_t color,
                                 int16_t width)
{
    FillSurfacePc34Compat *surface = context;

    if (ignored_bitmap != NULL) surface->fill_result = 0;
    surface->call_count++;
    surface->last_width = width;
    surface->fill_result = dm1_f0135_video_fillbox_planar_20260714_pc34_compat(
        surface->bitmap, surface->bitmap_size, surface->row_bytes,
        surface->pixel_height, box, (uint16_t)color);
}

static void fill_zone_callback(void *context,
                               int16_t *box,
                               int16_t color,
                               int16_t width,
                               int16_t height)
{
    FillSurfacePc34Compat *surface = context;

    if (width != 320 || height != 200) surface->fill_result = 0;
    surface->call_count++;
    surface->last_width = width;
    surface->fill_result = dm1_f0135_video_fillbox_planar_20260714_pc34_compat(
        surface->bitmap, surface->bitmap_size, surface->row_bytes,
        surface->pixel_height, box, (uint16_t)color);
}

static void fill_viewport_callback(void *context,
                                   int16_t *box,
                                   int16_t color,
                                   int16_t width)
{
    FillSurfacePc34Compat *surface = context;

    surface->call_count++;
    surface->last_width = width;
    surface->fill_result = dm1_f0135_video_fillbox_planar_20260714_pc34_compat(
        surface->bitmap, surface->bitmap_size, surface->row_bytes,
        surface->pixel_height, box, (uint16_t)color);
}

static int16_t *get_zone_callback(void *context,
                                  int16_t zone_index,
                                  int16_t zone_xyz[4])
{
    (void)context;
    (void)zone_index;
    zone_xyz[0] = 5;
    zone_xyz[1] = 7;
    zone_xyz[2] = 3;
    zone_xyz[3] = 4;
    return zone_xyz;
}

int main(void)
{
    static uint8_t screen_a[160u * 200u];
    static uint8_t screen_b[160u * 200u];
    static uint8_t viewport[112u * 136u];
    static uint8_t material_viewport[112u * 136u];
    uint8_t material_before[112u * 136u];
    FillSurfacePc34Compat screen_surface_a = {
        screen_a, sizeof(screen_a), 160u, 200u, 0, 0, 0
    };
    FillSurfacePc34Compat screen_surface_b = {
        screen_b, sizeof(screen_b), 160u, 200u, 0, 0, 0
    };
    FillSurfacePc34Compat viewport_surface = {
        viewport, sizeof(viewport), 112u, 136u, 0, 0, 0
    };
    const redmcsb_f0732_video_driver_pc34_compat screen_driver = {
        fill_screen_callback, &screen_surface_a
    };
    const redmcsb_f0733_graphics_pc34_compat zone_graphics = {
        get_zone_callback, fill_zone_callback, &screen_surface_b
    };
    const redmcsb_f0735_graphics_pc34_compat viewport_graphics = {
        fill_viewport_callback, &viewport_surface
    };
    DM1_V1_ViewportPlanarFillMaterialPc34 material = {
        material_viewport, sizeof(material_viewport), 112u, 136u, 1
    };
    int16_t screen_zone[4] = { 1, 2, 2, 2 };
    int16_t viewport_box[4] = { 2, 4, 5, 6 };
    int ok = 1;

    redmcsb_f0732_fill_screen_area_pc34_compat(
        &screen_driver, screen_zone, UINT16_C(0x0003));
    ok &= check(screen_surface_a.call_count == 1 &&
                    screen_surface_a.last_width == 320 &&
                    screen_surface_a.fill_result == 1,
                "F0732 dispatches its 320-wide screen box to F0135");
    ok &= check(pixel_color(screen_a, 160u, 1, 2) == 3u &&
                    pixel_color(screen_a, 160u, 2, 3) == 3u &&
                    pixel_color(screen_a, 160u, 3, 3) == 0u,
                "F0732 zone expansion reaches the caller-owned screen bitmap");

    redmcsb_f0733_fill_zone_by_index_pc34_compat(
        &zone_graphics, 19, INT16_C(0x000a));
    ok &= check(screen_surface_b.call_count == 1 &&
                    screen_surface_b.last_width == 320 &&
                    screen_surface_b.fill_result == 1,
                "F0733 resolves a source zone then dispatches it to F0135");
    ok &= check(pixel_color(screen_b, 160u, 5, 3) == 10u &&
                    pixel_color(screen_b, 160u, 7, 4) == 10u &&
                    pixel_color(screen_b, 160u, 8, 4) == 0u,
                "F0733 preserves the resolved inclusive source box");

    redmcsb_f0735_fill_viewport_box_pc34_compat(
        &viewport_graphics, viewport_box, INT16_C(0x0005));
    ok &= check(viewport_surface.call_count == 1 &&
                    viewport_surface.last_width == 224 &&
                    viewport_surface.fill_result == 1,
                "F0735 dispatches its 224-wide viewport box to F0135");
    ok &= check(pixel_color(viewport, 112u, 2, 5) == 5u &&
                    pixel_color(viewport, 112u, 4, 6) == 5u &&
                    pixel_color(viewport, 112u, 5, 6) == 0u,
                "F0735 writes only the caller-owned viewport box");

    memset(material_viewport, 0, sizeof(material_viewport));
    ok &= check(dm1_v1_viewport_fill_material_f0134_pc34(&material, 12u) == 1 &&
                    pixel_color(material_viewport, 112u, 0, 0) == 12u &&
                    pixel_color(material_viewport, 112u, 223, 135) == 12u,
                "F0134 fills only an admitted original planar viewport material");
    ok &= check(dm1_v1_viewport_fill_material_box_f0135_pc34(
                    &material, viewport_box, UINT16_C(0x0003)) == 1 &&
                    pixel_color(material_viewport, 112u, 2, 5) == 3u &&
                    pixel_color(material_viewport, 112u, 4, 6) == 3u &&
                    pixel_color(material_viewport, 112u, 5, 6) == 12u,
                "F0135 applies its inclusive box after the F0134 material fill");
    memcpy(material_before, material_viewport, sizeof(material_before));
    material.original_material_verified = 0;
    ok &= check(dm1_v1_viewport_fill_material_f0134_pc34(&material, 1u) == 0 &&
                    dm1_v1_viewport_fill_material_box_f0135_pc34(
                        &material, viewport_box, UINT16_C(0x0001)) == 0 &&
                    memcmp(material_viewport, material_before,
                           sizeof(material_viewport)) == 0,
                "missing original material rejects without a synthetic fallback");

    if (!ok) return 1;
    puts("PASS redmcsb_fillbox_blitfill_f0135_integration_pc34_compat");
    return 0;
}
