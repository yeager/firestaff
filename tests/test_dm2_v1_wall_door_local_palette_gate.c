/* Source: skproject SKWIN/SkWinCore.cpp DRAW_WALL, DRAW_DOOR, and
 * QUERY_GDAT_IMAGE_LOCALPAL. A decoded dungeon image is only drawable with
 * the IMG3 palette selected by the same GDAT lookup. */
#include "dm2_v1_viewport_renderer.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    int asset_fetches;
    int palette_fetches;
    int wall_palette_fetches;
    int door_palette_fetches;
} MaterialTrace;

static int checks;
static int passed;

#define CHECK(label, condition) do { \
    ++checks; \
    if (condition) { ++passed; } \
    else { printf("FAIL: %s\n", label); } \
} while (0)

static int is_wall_gdat(int gdat_index)
{
    int graphicsset;
    int field;
    return dm2_v1_viewport_wall_graphic_address(
        gdat_index, &graphicsset, &field);
}

static int fetch_material(void *user,
                          int gdat_index,
                          const uint8_t **out_pixels,
                          int *out_w,
                          int *out_h,
                          int *out_stride)
{
    static const uint8_t pixels[4] = { 1, 0, 2, 3 };
    MaterialTrace *trace = (MaterialTrace *)user;

    (void)gdat_index;
    ++trace->asset_fetches;
    *out_pixels = pixels;
    *out_w = 2;
    *out_h = 2;
    *out_stride = 2;
    return 0;
}

static int fetch_material_palette(void *user,
                                  int gdat_index,
                                  uint8_t out_palette16[16],
                                  uint32_t *out_hash)
{
    MaterialTrace *trace = (MaterialTrace *)user;
    uint8_t base = is_wall_gdat(gdat_index) ? 0xa0u : 0xb0u;

    ++trace->palette_fetches;
    if (base == 0xa0u) ++trace->wall_palette_fetches;
    else ++trace->door_palette_fetches;
    for (int i = 0; i < 16; ++i) out_palette16[i] = (uint8_t)(base + i);
    if (out_hash) {
        *out_hash = 0x4d415450u ^ (uint32_t)gdat_index;
        if (*out_hash == 0u) *out_hash = 1u;
    }
    return 0;
}

static int framebuffer_contains(const uint8_t *framebuffer, uint8_t value)
{
    for (size_t i = 0; i < (size_t)DM2_VP_WIDTH * DM2_VP_HEIGHT; ++i) {
        if (framebuffer[i] == value) return 1;
    }
    return 0;
}

static void setup_dungeon_materials(DM2_V1_ViewportState *viewport,
                                    uint8_t *framebuffer,
                                    MaterialTrace *trace,
                                    int bind_palette)
{
    dm2_v1_viewport_init(viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_asset_provider(viewport, fetch_material, trace);
    if (bind_palette) {
        dm2_v1_viewport_set_asset_palette_provider(
            viewport, fetch_material_palette, trace);
    }
    dm2_v1_viewport_set_source_materials_required(viewport, 1);
    dm2_v1_viewport_set_gdat_scene_control(
        viewport, 1, 0x2a, 0x53434e45u,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    viewport->squares[DM2_SQ_D0C].flags = DM2_SQF_HAS_DOOR;
}

int main(void)
{
    DM2_V1_ViewportState viewport;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    MaterialTrace trace;

    memset(framebuffer, 0, sizeof(framebuffer));
    memset(&trace, 0, sizeof(trace));
    setup_dungeon_materials(&viewport, framebuffer, &trace, 1);
    dm2_v1_render_walls(&viewport);
    {
        int wall_pixel_seen = framebuffer_contains(framebuffer, 0xa1u);
    dm2_v1_render_doors(&viewport);
        CHECK("wall and door pixels keep their own source IMG3 palettes",
              trace.asset_fetches > 0 &&
                  trace.palette_fetches == trace.asset_fetches &&
                  trace.wall_palette_fetches > 0 &&
                  trace.door_palette_fetches > 0 &&
                  wall_pixel_seen &&
                  framebuffer_contains(framebuffer, 0xb1u) &&
                  viewport.gdat_local_palette_consumed_count > 0 &&
                  viewport.asset_wall_drawn_count > 0 &&
                  viewport.asset_door_panel_drawn_count > 0 &&
                  viewport.blocked_material_draw_count == 0);
    }

    memset(framebuffer, 0x7e, sizeof(framebuffer));
    memset(&trace, 0, sizeof(trace));
    setup_dungeon_materials(&viewport, framebuffer, &trace, 0);
    dm2_v1_render_walls(&viewport);
    dm2_v1_render_doors(&viewport);
    CHECK("source-required wall and door materials fail closed without IMG3 palettes",
          trace.asset_fetches > 0 &&
              viewport.asset_wall_drawn_count == 0 &&
              viewport.asset_door_panel_drawn_count == 0 &&
              viewport.asset_door_frame_drawn_count == 0 &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL) != 0u &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR) != 0u &&
              framebuffer[0] == 0x7eu &&
              framebuffer[60 * DM2_VP_WIDTH + 160] == 0x7eu);

    printf("DM2 wall/door local palette gate: %d/%d passed\n", passed, checks);
    return passed == checks ? 0 : 1;
}
