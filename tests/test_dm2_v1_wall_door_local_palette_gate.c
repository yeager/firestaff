/* Source: skproject SKWIN/SkWinCore.cpp DRAW_WALL, DRAW_DOOR, and
 * QUERY_GDAT_IMAGE_LOCALPAL. A decoded dungeon image is only drawable with
 * the IMG3 palette selected by the same GDAT lookup.
 *
 * Provenance: introduced against the callback-provider route.  Re-anchored
 * 2026-07-21 after 5c21e5561 ("Fix DM2 scene local palette ownership") and
 * 50a939491 ("dm2: bind G1 wall plans to M10 scene transactions"): under
 * source_materials_required the wall and door materials are owned by
 * boot-owned M11 command plans whose commands each carry the palette of
 * their own GDAT lookup; the provider callback is no longer consulted.
 * The fixture below builds minimal synthetic plans with the same receipt
 * discipline the real-data builders prove against canonical GRAPHICS.DAT
 * (test_dm2_v1_gdat_wall_plan_viewport_real_data.c,
 * test_dm2_v1_gdat_door_overlay_plan_real_data.c). */
#include "dm2_v1_gdat_door_overlay_m11_command.h"
#include "dm2_v1_gdat_wall_m11_command.h"
#include "dm2_v1_viewport_renderer.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    int asset_fetches;
    int palette_fetches;
} MaterialTrace;

static int checks;
static int passed;

#define CHECK(label, condition) do { \
    ++checks; \
    if (condition) { ++passed; } \
    else { printf("FAIL: %s\n", label); } \
} while (0)

static uint32_t fnv1a_bytes(uint32_t hash, const uint8_t *bytes, size_t count)
{
    for (size_t i = 0; i < count; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

static uint32_t fnv1a_u32(uint32_t hash, uint32_t value)
{
    return fnv1a_bytes(hash, (const uint8_t *)&value, sizeof(value));
}

static uint32_t palette_fnv(const uint8_t palette[16])
{
    return fnv1a_bytes(2166136261u, palette, 16);
}

static uint32_t pixels_fnv(const uint8_t *pixels, int width, int height,
                           int stride)
{
    uint32_t hash = 2166136261u;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            hash ^= pixels[y * stride + x];
            hash *= 16777619u;
        }
    }
    return hash;
}

/* Mirror of the renderer's dm2_v1_wall_command_geometry_hash: FNV-1a over
 * the source/destination rect, rect number, flip/movement bytes and the
 * receipt hashes, in the command's own field order. */
static uint32_t wall_geometry_hash(const DM2_V1_GdatWallM11Command *command)
{
    uint32_t hash = 2166136261u;
    hash = fnv1a_bytes(hash, (const uint8_t *)&command->source_x,
                       sizeof(command->source_x) + sizeof(command->source_y) +
                       sizeof(command->source_width) +
                       sizeof(command->source_height) +
                       sizeof(command->destination_x) +
                       sizeof(command->destination_y) +
                       sizeof(command->destination_width) +
                       sizeof(command->destination_height));
    hash = fnv1a_u32(hash, command->rect_number);
    hash = fnv1a_u32(hash, command->mirror_flip);
    hash = fnv1a_u32(hash, command->movement_active);
    hash = fnv1a_u32(hash, (uint8_t)command->movement_query_offset_y);
    hash = fnv1a_u32(hash, command->rect_table_hash);
    hash = fnv1a_u32(hash, command->rect_row_hash);
    hash = fnv1a_u32(hash, command->metadata_hash);
    hash = fnv1a_u32(hash, command->material_receipt_hash);
    return hash;
}

/* Mirror of the door overlay builder's command_plan_hash. */
static uint32_t door_plan_hash(const DM2_V1_GdatDoorOverlayM11CommandPlan *plan)
{
    uint32_t hash = 2166136261u;
    for (int i = 0; i < plan->command_count; ++i) {
        const DM2_V1_GdatDoorOverlayM11Command *command = &plan->commands[i];
        hash = fnv1a_u32(hash, command->raw_hash);
        hash = fnv1a_u32(hash, command->decoded_hash);
        hash = fnv1a_u32(hash, command->palette_hash);
        hash = fnv1a_u32(hash, command->material_receipt_hash);
        hash = fnv1a_u32(hash, command->selection_hash);
        hash = fnv1a_u32(hash, command->geometry_hash);
        hash = fnv1a_u32(hash, command->palette_transform_hash);
    }
    return hash ? hash : 1u;
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

    (void)gdat_index;
    ++trace->palette_fetches;
    memset(out_palette16, 0, 16);
    if (out_hash) *out_hash = 0u;
    return 0;
}

static int framebuffer_contains(const uint8_t *framebuffer, uint8_t value)
{
    for (size_t i = 0; i < (size_t)DM2_VP_WIDTH * DM2_VP_HEIGHT; ++i) {
        if (framebuffer[i] == value) return 1;
    }
    return 0;
}

static uint8_t wall_pixels[4] = { 1, 0, 2, 3 };
static uint8_t door_pixels[4] = { 1, 0, 2, 3 };
static const uint8_t wall_raw_bytes[4] = { 0x11u, 0x22u, 0x33u, 0x44u };
static const uint8_t door_raw_bytes[4] = { 0x55u, 0x66u, 0x77u, 0x88u };

/* One visible D1C wall command whose palette comes from its own GDAT
 * field lookup (0xa0-based), bound through the boot-owned wall plan. */
static void build_wall_plan(DM2_V1_GdatWallM11CommandPlan *plan)
{
    DM2_V1_GdatWallM11Command *command;

    memset(plan, 0, sizeof(*plan));
    plan->valid = 1;
    plan->graphicsset = 0x2au;
    plan->command_count = 1;
    plan->command_hash = 0x57414c4cu;
    command = &plan->commands[0];
    command->view_square = DM2_SQ_D1C;
    command->field =
        (uint8_t)dm2_v1_viewport_wall_field_for_square(DM2_SQ_D1C);
    command->pixels = wall_pixels;
    command->width = 2;
    command->height = 2;
    for (int i = 0; i < 16; ++i) command->palette16[i] = (uint8_t)(0xa0u + i);
    command->raw_hash = 0x72617701u;
    command->decoded_hash = 0xdec0de01u;
    command->palette_hash = palette_fnv(command->palette16);
    command->material_raw_index = 1;
    command->material_source_bytes = wall_raw_bytes;
    command->material_source_byte_count = sizeof(wall_raw_bytes);
    command->material_receipt_hash = 0x52454350u;
    command->source_x = 0;
    command->source_y = 0;
    command->source_width = 2;
    command->source_height = 2;
    command->destination_x = 100;
    command->destination_y = 40;
    command->destination_width = 2;
    command->destination_height = 2;
    command->rect_number =
        (uint16_t)(0x2beu + (command->field - 0x22u));
    command->mirror_flip = 0;
    command->movement_active = 0;
    command->movement_query_offset_y = 0;
    command->rect_table_hash = 0x7ab1e001u;
    command->rect_row_hash = 0x70ad0001u;
    command->metadata_hash = 0x9e7a0001u;
    command->geometry_hash = wall_geometry_hash(command);
}

/* One closed D0C door panel command with its own GDAT palette (0xb0-based).
 * DRAW_DOOR's D0 distance-0 branch selects image zero with the mandatory
 * 0x71 stretch and light zero (SkWinCore.cpp:46431-46441). */
static void build_door_plan(DM2_V1_GdatDoorOverlayM11CommandPlan *plan)
{
    DM2_V1_GdatDoorOverlayM11Command *command;

    memset(plan, 0, sizeof(*plan));
    plan->valid = 1;
    plan->command_count = 1;
    command = &plan->commands[0];
    command->gdat_index =
        dm2_v1_viewport_door_panel_graphic_index_for_square(DM2_SQ_D0C);
    command->view_square = DM2_SQ_D0C;
    command->kind = DM2_V1_GDAT_DOOR_PANEL;
    command->category = DM2_GDAT_CATEGORY_DOORS;
    command->entry_index = 0;
    command->field = 0;
    command->pixels = door_pixels;
    command->width = 2;
    command->height = 2;
    command->door_opening_dir = 0;
    command->door_state = 0;
    command->door_open_pct = 0;
    command->mirror_flip = 0;
    command->draw_distance = 0;
    command->stretch_dual = 0x71u;
    command->light_palette = 0;
    command->movement_active = 0;
    command->palette_darkness = 0;
    command->palette_light_receipt_hash = 0u;
    command->palette_transform_hash = 0u;
    command->color_key = 0xffffu;
    command->no_frames = 1;
    command->rect_number = 1;
    command->rect_x = 100;
    command->rect_y = 140;
    command->rect_width = 2;
    command->rect_height = 2;
    command->source_x = 0;
    command->source_y = 0;
    command->source_width = 2;
    command->source_height = 2;
    for (int i = 0; i < 16; ++i) command->palette16[i] = (uint8_t)(0xb0u + i);
    command->material_raw_index = 1;
    command->material_source_bytes = door_raw_bytes;
    command->material_source_byte_count = sizeof(door_raw_bytes);
    command->raw_hash = 0x72617702u;
    command->decoded_hash = pixels_fnv(door_pixels, 2, 2, 2);
    command->palette_hash = palette_fnv(command->palette16);
    command->material_receipt_hash = 0x52454351u;
    command->rect_table_hash = 0x7ab1e002u;
    command->rect_row_hash = 0x70ad0002u;
    command->geometry_hash = 0x9e090002u;
    command->selection_hash = 0x5e1ec702u;
    plan->command_hash = door_plan_hash(plan);
}

static void setup_dungeon_materials(DM2_V1_ViewportState *viewport,
                                    uint8_t *framebuffer,
                                    MaterialTrace *trace,
                                    const DM2_V1_GdatWallM11CommandPlan *wall_plan,
                                    const DM2_V1_GdatDoorOverlayM11CommandPlan *door_plan)
{
    dm2_v1_viewport_init(viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_asset_provider(viewport, fetch_material, trace);
    dm2_v1_viewport_set_asset_palette_provider(
        viewport, fetch_material_palette, trace);
    dm2_v1_viewport_set_source_materials_required(viewport, 1);
    dm2_v1_viewport_set_gdat_scene_control(
        viewport, 1, 0x2a, 0x53434e45u,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    if (wall_plan) {
        dm2_v1_viewport_set_gdat_wall_material_plan(viewport, wall_plan);
        viewport->squares[DM2_SQ_D1C].flags |= DM2_SQF_HAS_WALL;
    }
    if (door_plan) {
        dm2_v1_viewport_set_gdat_door_overlay_material_plan(viewport, door_plan);
    }
    viewport->squares[DM2_SQ_D0C].flags = DM2_SQF_HAS_DOOR;
    viewport->squares[DM2_SQ_D0C].door_gfx_admitted = 1;
    viewport->squares[DM2_SQ_D0C].door_direct_g1_root = 1;
}

int main(void)
{
    DM2_V1_ViewportState viewport;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    MaterialTrace trace;
    DM2_V1_GdatWallM11CommandPlan wall_plan;
    DM2_V1_GdatDoorOverlayM11CommandPlan door_plan;

    build_wall_plan(&wall_plan);
    build_door_plan(&door_plan);

    memset(framebuffer, 0, sizeof(framebuffer));
    memset(&trace, 0, sizeof(trace));
    setup_dungeon_materials(&viewport, framebuffer, &trace, &wall_plan,
                            &door_plan);
    dm2_v1_render_walls(&viewport);
    {
        int wall_pixel_seen = framebuffer_contains(framebuffer, 0xa1u);
        dm2_v1_render_doors(&viewport);
        CHECK("wall and door pixels keep their own source IMG3 palettes",
              trace.asset_fetches == 0 && trace.palette_fetches == 0 &&
                  wall_pixel_seen &&
                  framebuffer_contains(framebuffer, 0xb1u) &&
                  viewport.asset_wall_drawn_count == 1 &&
                  viewport.gdat_wall_material_plan_consumed_count == 1 &&
                  viewport.gdat_door_overlay_material_plan_consumed_count > 0 &&
                  viewport.asset_door_panel_drawn_count > 0 &&
                  viewport.blocked_material_draw_count == 0);
    }

    /* Fail-closed re-anchor: without the boot-owned plans the source-required
     * frame blocks at the UPDATE_GFXSET transaction gate (wall, before any
     * fetch) or at the per-material receipt gate (door panel fetch cannot
     * produce a palette receipt without a bound provider plan) — never a
     * drawn pixel. */
    memset(framebuffer, 0x7e, sizeof(framebuffer));
    memset(&trace, 0, sizeof(trace));
    setup_dungeon_materials(&viewport, framebuffer, &trace, NULL, NULL);
    dm2_v1_render_walls(&viewport);
    dm2_v1_render_doors(&viewport);
    CHECK("source-required wall and door materials fail closed without IMG3 palettes",
          viewport.asset_wall_drawn_count == 0 &&
              viewport.gdat_wall_material_plan_consumed_count == 0 &&
              viewport.asset_door_panel_drawn_count == 0 &&
              viewport.asset_door_frame_drawn_count == 0 &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL) != 0u &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR) != 0u &&
              framebuffer[0] == 0x7eu &&
              framebuffer[60 * DM2_VP_WIDTH + 160] == 0x7eu);

    printf("DM2 wall/door local palette gate: %d/%d passed\n",
           passed, checks);
    return passed == checks ? 0 : 1;
}
