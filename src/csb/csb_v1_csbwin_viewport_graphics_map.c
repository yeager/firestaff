#include "csb_v1_csbwin_viewport_graphics_map.h"
#include "csb_v1_graphics_atari_st_loader_pc34_compat.h"

#include <stdlib.h>
#include <string.h>

int csb_v1_csbwin_floor_ceiling_graphic_index(uint16_t floor_set,
                                               int ceiling,
                                               uint16_t *out_graphic_index)
{
    if (!out_graphic_index || floor_set > 15u) return 0;
    *out_graphic_index = (uint16_t)(CSB_V1_CSBWIN_FLOORSET_FIRST_GRAPHIC +
        floor_set * CSB_V1_CSBWIN_FLOORSET_GRAPHIC_COUNT + (ceiling ? 1u : 0u));
    return 1;
}

int csb_v1_csbwin_packed_byte_width(uint16_t pixel_width,
                                    uint16_t *out_byte_width)
{
    if (!out_byte_width || pixel_width == 0u) return 0;
    *out_byte_width = (uint16_t)(((unsigned int)pixel_width + 1u) / 2u);
    return 1;
}

int csb_v1_csbwin_viewport_graphic_index(uint16_t wall_set,
                                          uint16_t slot,
                                          uint16_t *out_graphic_index)
{
    unsigned int index;

    if (!out_graphic_index || wall_set > 15u ||
        slot >= CSB_V1_CSBWIN_WALLSET_GRAPHIC_COUNT) return 0;
    index = CSB_V1_CSBWIN_WALLSET_FIRST_GRAPHIC +
        wall_set * CSB_V1_CSBWIN_WALLSET_GRAPHIC_COUNT + slot;
    if (index > UINT16_MAX) return 0;
    *out_graphic_index = (uint16_t)index;
    return 1;
}

int csb_v1_csbwin_viewport_wall_source(
    uint16_t wall_set, CSB_V1_CSBWinViewportWall wall,
    uint16_t *out_graphic_index, int *out_mirrored)
{
    static const uint8_t source_slot[CSB_V1_CSBWIN_VIEWPORT_WALL_COUNT] = {
        5u, 4u, 4u, 4u, 5u, 3u, 3u, 3u, 2u, 2u, 2u, 1u, UINT8_MAX, 0u
    };

    if (out_mirrored) *out_mirrored = 0;
    if (!out_graphic_index || !out_mirrored ||
        (unsigned int)wall >= CSB_V1_CSBWIN_VIEWPORT_WALL_COUNT) return 0;
    /* StdBitmapPointers deliberately has no BMP_StdWallBitmapF0. */
    if (source_slot[wall] == UINT8_MAX) return 0;
    if (!csb_v1_csbwin_viewport_graphic_index(
            wall_set, (uint16_t)(CSB_V1_CSBWIN_DOOR_GRAPHIC_COUNT +
                                  source_slot[wall]), out_graphic_index)) {
        return 0;
    }
    *out_mirrored = wall == CSB_V1_CSBWIN_VIEWPORT_WALL_F3R2;
    return 1;
}

int csb_v1_csbwin_viewport_wall_projection_rectangle(
    CSB_V1_CSBWinViewportWall wall, uint8_t *out_rectangle_index)
{
    static const uint8_t rectangle_index[CSB_V1_CSBWIN_VIEWPORT_WALL_COUNT] = {
        13u, 1u, 0u, 2u, 12u, 4u, 3u, 5u, 7u, 6u, 8u, 10u, 9u, 11u
    };

    if (!out_rectangle_index ||
        (unsigned int)wall >= CSB_V1_CSBWIN_VIEWPORT_WALL_COUNT) return 0;
    *out_rectangle_index = rectangle_index[wall];
    return 1;
}

int csb_v1_csbwin_viewport_projection_rectangle_is_valid(
    const CSB_V1_CSBWinViewportProjectionRectangle *rectangle)
{
    unsigned int source_width;

    if (!rectangle || rectangle->x1 > rectangle->x2 ||
        rectangle->y1 > rectangle->y2 || rectangle->y2 >= 200u) return 0;
    /* F0 is composed by CSBWin's local-cell path, not a pWallBitmaps source;
     * its source tuple is deliberately all zero in GRAPHICS.DAT item 0x22e. */
    if (rectangle->source_stride == 0u || rectangle->source_height == 0u) {
        return rectangle->source_stride == 0u && rectangle->source_height == 0u &&
            rectangle->source_x == 0u && rectangle->source_y == 0u;
    }
    source_width = (unsigned int)rectangle->source_stride * 2u;
    return rectangle->source_x < source_width &&
        rectangle->source_y < rectangle->source_height;
}

int csb_v1_csbwin_viewport_layout_022e_decode(
    const uint8_t *decoded_graphic, size_t decoded_size,
    CSB_V1_CSBWinViewportLayout022e *out_layout)
{
    size_t index;
    const CSB_V1_CSBWinViewportProjectionRectangle *door_rectangles;

    if (!out_layout) return 0;
    memset(out_layout, 0, sizeof(*out_layout));
    if (!decoded_graphic || decoded_size !=
        CSB_V1_CSBWIN_LAYOUT_022E_DECODED_SIZE) return 0;
    if (CSB_V1_CSBWIN_LAYOUT_022E_DOOR_RECTANGLE_OFFSET +
            sizeof(out_layout->door_rectangles) > decoded_size ||
        CSB_V1_CSBWIN_LAYOUT_022E_DOOR_TRACK_RECTANGLE_OFFSET +
            sizeof(out_layout->door_track_rectangles) > decoded_size ||
        CSB_V1_CSBWIN_LAYOUT_022E_DOOR_FRAME_RECTANGLE_OFFSET +
            sizeof(out_layout->door_frame_rectangles) > decoded_size ||
        CSB_V1_CSBWIN_LAYOUT_022E_WALL_RECTANGLE_OFFSET +
            sizeof(out_layout->rectangles) > decoded_size) return 0;
    memcpy(out_layout->door_rectangles,
           decoded_graphic + CSB_V1_CSBWIN_LAYOUT_022E_DOOR_RECTANGLE_OFFSET,
           sizeof(out_layout->door_rectangles));
    memcpy(out_layout->door_track_rectangles,
           decoded_graphic + CSB_V1_CSBWIN_LAYOUT_022E_DOOR_TRACK_RECTANGLE_OFFSET,
           sizeof(out_layout->door_track_rectangles));
    memcpy(out_layout->door_frame_rectangles,
           decoded_graphic + CSB_V1_CSBWIN_LAYOUT_022E_DOOR_FRAME_RECTANGLE_OFFSET,
           sizeof(out_layout->door_frame_rectangles));
    memcpy(out_layout->rectangles,
           decoded_graphic + CSB_V1_CSBWIN_LAYOUT_022E_WALL_RECTANGLE_OFFSET,
           sizeof(out_layout->rectangles));
    door_rectangles = &out_layout->door_rectangles[0][0];
    for (index = 0u; index < CSB_V1_CSBWIN_LAYOUT_022E_DOOR_RECTANGLE_FAMILY_COUNT *
            CSB_V1_CSBWIN_LAYOUT_022E_DOOR_RECTANGLE_STATE_COUNT; ++index) {
        if (!csb_v1_csbwin_viewport_projection_rectangle_is_valid(
                door_rectangles + index)) {
            memset(out_layout, 0, sizeof(*out_layout));
            return 0;
        }
    }
    for (index = 0u; index < CSB_V1_CSBWIN_LAYOUT_022E_DOOR_TRACK_RECTANGLE_COUNT;
         ++index) {
        if (!csb_v1_csbwin_viewport_projection_rectangle_is_valid(
                &out_layout->door_track_rectangles[index])) {
            memset(out_layout, 0, sizeof(*out_layout));
            return 0;
        }
    }
    for (index = 0u; index < CSB_V1_CSBWIN_LAYOUT_022E_DOOR_FRAME_RECTANGLE_COUNT;
         ++index) {
        if (!csb_v1_csbwin_viewport_projection_rectangle_is_valid(
                &out_layout->door_frame_rectangles[index])) {
            memset(out_layout, 0, sizeof(*out_layout));
            return 0;
        }
    }
    for (index = 0u; index < CSB_V1_CSBWIN_VIEWPORT_WALL_COUNT; ++index) {
        if (!csb_v1_csbwin_viewport_projection_rectangle_is_valid(
                &out_layout->rectangles[index])) {
            memset(out_layout, 0, sizeof(*out_layout));
            return 0;
        }
    }
    out_layout->valid = 1;
    return 1;
}

int csb_v1_csbwin_viewport_layout_022e_read_graphics_dat(
    const char *graphics_dat_path, CSB_V1_CSBWinViewportLayout022e *out_layout)
{
    CSB_AtariStLoader loader;
    uint8_t *decoded = NULL;
    int ok = 0;

    if (!out_layout) return 0;
    memset(out_layout, 0, sizeof(*out_layout));
    if (!graphics_dat_path || !graphics_dat_path[0]) return 0;
    csb_atari_st_graphics_loader_init(&loader);
    if (!csb_atari_st_graphics_loader_open(&loader, graphics_dat_path) ||
        loader.item_count != 563u ||
        loader.items[CSB_V1_CSBWIN_LAYOUT_022E_GRAPHIC_INDEX].decompressed_size !=
            CSB_V1_CSBWIN_LAYOUT_022E_DECODED_SIZE) goto done;
    decoded = (uint8_t *)malloc(CSB_V1_CSBWIN_LAYOUT_022E_DECODED_SIZE);
    if (!decoded || csb_atari_st_graphics_loader_read_item(
            &loader, CSB_V1_CSBWIN_LAYOUT_022E_GRAPHIC_INDEX, decoded,
            CSB_V1_CSBWIN_LAYOUT_022E_DECODED_SIZE) !=
            (int)CSB_V1_CSBWIN_LAYOUT_022E_DECODED_SIZE) goto done;
    ok = csb_v1_csbwin_viewport_layout_022e_decode(
        decoded, CSB_V1_CSBWIN_LAYOUT_022E_DECODED_SIZE, out_layout);
done:
    free(decoded);
    csb_atari_st_graphics_loader_close(&loader);
    return ok;
}

int csb_v1_csbwin_viewport_build_wall_plan(
    uint16_t wall_set, const CSB_V1_CSBWinViewportLayout022e *layout,
    CSB_V1_CSBWinViewportWallPlan *out_plan)
{
    unsigned int wall;

    if (!out_plan) return 0;
    memset(out_plan, 0, sizeof(*out_plan));
    if (!layout || !layout->valid || wall_set > 15u) return 0;
    for (wall = 0u; wall < CSB_V1_CSBWIN_VIEWPORT_WALL_COUNT; ++wall) {
        CSB_V1_CSBWinViewportWallDraw *draw;
        uint8_t rectangle_index;
        uint16_t graphic_index;
        int mirrored;

        if ((CSB_V1_CSBWinViewportWall)wall ==
            CSB_V1_CSBWIN_VIEWPORT_WALL_F0) continue;
        if (out_plan->count >= CSB_V1_CSBWIN_VIEWPORT_WALL_DRAW_COUNT ||
            !csb_v1_csbwin_viewport_wall_projection_rectangle(
                (CSB_V1_CSBWinViewportWall)wall, &rectangle_index) ||
            !csb_v1_csbwin_viewport_wall_source(
                wall_set, (CSB_V1_CSBWinViewportWall)wall,
                &graphic_index, &mirrored) ||
            rectangle_index >= CSB_V1_CSBWIN_VIEWPORT_WALL_COUNT) {
            memset(out_plan, 0, sizeof(*out_plan));
            return 0;
        }
        draw = &out_plan->draws[out_plan->count++];
        draw->wall = (CSB_V1_CSBWinViewportWall)wall;
        draw->graphic_index = graphic_index;
        draw->mirrored = mirrored;
        draw->projection = layout->rectangles[rectangle_index];
    }
    out_plan->valid = out_plan->count ==
        CSB_V1_CSBWIN_VIEWPORT_WALL_DRAW_COUNT;
    return out_plan->valid;
}
