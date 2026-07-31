/*
 * dm2_v1_viewport_renderer.c — DM2 V1 Viewport Rendering Pipeline
 *
 * Phase 3: DM2 viewport, UI chrome, items, outdoor/indoor presentation.
 *
 * Architecture:
 *   DM2 viewport is 320×200 game pixels, same as DM1/CSB.
 *   Status bar: top 28px  (champion health/magic/conditions)
 *   Dungeon view: 320×144px (walls, floor, creatures, items)
 *   Action strip: bottom 28px (action icons: Attack/Cast/Use/Drop/Move)
 *   Portrait panel: right 80×144px (champion portraits)
 *
 * DM2 differs from DM1:
 *   - Rooms vs corridors (DM2 is not a dungeon-corridor game)
 *   - Different wall set indices (G2107/G3060 variants)
 *   - Outdoor mode (sky gradient, weather, buildings)
 *   - Different UI chrome (gold counter, no champion portrait panel)
 *
 * Source: SKULL.ASM T560  — dungeon viewport rendering
 *         SKULL.ASM T600  — outdoor viewport rendering
 *         SKULL.ASM T520  — party/movement tick
 *         ReDMCSB DUNGEON.C — draw order, wall bitmap selection
 *         ReDMCSB DUNVIEW.C:575-586 — G0163 wall frame table
 *         ReDMCSB DUNVIEW.C:148-165  — wall set indices
 *         ReDMCSB DUNVIEW.C:2962-3047 — F0098 DrawFloorAndCeiling
 *         ReDMCSB DUNVIEW.C:3048-3070 — F0100 DrawWallSetBitmap
 *         ReDMCSB DUNVIEW.C:3082-3095 — F0102 DrawDoorBitmap
 *         ReDMCSB DUNVIEW.C:3940-4015 — F0108 DrawFloorOrnament
 *         ReDMCSB DUNVIEW.C:4016-4050 — F0109 DrawDoorOrnament
 *         ReDMCSB DUNVIEW.C:4119-4270 — F0110 DrawDoorButton, F0111 DrawDoor
 *         ReDMCSB DUNVIEW.C:4351-4382 — F0112 DrawCeilingPit
 *         SKULLWIN/SKWIN/c_gui_vp.cpp — viewport blit order
 *         docs/dm2_graphics.md — drawing pipeline audit
 *         docs/dm2_walls.md — wall/door/floor rendering specifics
 *         docs/dm2_palette.md — DM2 palette system
 */

#include "dm2_v1_viewport_renderer.h"
#include "dm2_v1_runtime.h"
#include "dm2_v1_gdat_hud_m11_command.h"
#include "dm2_v1_gdat_wall_m11_command.h"
#include "dm2_v1_gdat_door_overlay_m11_command.h"
#include "dm2_v1_world_model.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

static uint32_t dm2_v1_viewport_indexed_pixel_hash(const uint8_t *pixels,
                                                    int width,
                                                    int height,
                                                    int stride);

/* Exact SKProject dm2data.cpp::table1d7029, read by
 * c_gui_vp.cpp::DM2_DRAW_DUNGEON_TILES. */
static const uint8_t s_dm2_draw_dungeon_tiles_cells[20] = {
    0x13u, 0x14u, 0x11u, 0x12u, 0x10u,
    0x0eu, 0x0fu, 0x0cu, 0x0du, 0x0bu,
    0x09u, 0x0au, 0x07u, 0x08u, 0x06u,
    0x04u, 0x05u, 0x03u, 0x01u, 0x02u
};

static uint32_t dm2_v1_wall_hash_bytes(uint32_t hash, const uint8_t *bytes,
                                        size_t size)
{
    for (size_t i = 0u; i < size; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

static uint32_t dm2_v1_wall_command_geometry_hash(
    const DM2_V1_GdatWallM11Command *command)
{
    uint32_t hash;

    if (!command) return 0u;
    hash = dm2_v1_wall_hash_bytes(2166136261u,
                                  (const uint8_t *)&command->source_x,
                                  sizeof(command->source_x) +
                                  sizeof(command->source_y) +
                                  sizeof(command->source_width) +
                                  sizeof(command->source_height) +
                                  sizeof(command->destination_x) +
                                  sizeof(command->destination_y) +
                                  sizeof(command->destination_width) +
                                  sizeof(command->destination_height));
    {
        uint32_t rect_number = command->rect_number;
        hash = dm2_v1_wall_hash_bytes(hash, (const uint8_t *)&rect_number,
                                      sizeof(rect_number));
    }
    {
        uint32_t mirror_flip = command->mirror_flip;
        hash = dm2_v1_wall_hash_bytes(hash, (const uint8_t *)&mirror_flip,
                                      sizeof(mirror_flip));
    }
    {
        uint32_t movement_active = command->movement_active;
        hash = dm2_v1_wall_hash_bytes(hash, (const uint8_t *)&movement_active,
                                      sizeof(movement_active));
    }
    {
        uint32_t movement_query_offset_y =
            (uint8_t)command->movement_query_offset_y;
        hash = dm2_v1_wall_hash_bytes(hash,
                                      (const uint8_t *)&movement_query_offset_y,
                                      sizeof(movement_query_offset_y));
    }
    hash = dm2_v1_wall_hash_bytes(hash,
                                  (const uint8_t *)&command->rect_table_hash,
                                  sizeof(command->rect_table_hash));
    hash = dm2_v1_wall_hash_bytes(hash,
                                  (const uint8_t *)&command->rect_row_hash,
                                  sizeof(command->rect_row_hash));
    hash = dm2_v1_wall_hash_bytes(hash,
                                  (const uint8_t *)&command->metadata_hash,
                                  sizeof(command->metadata_hash));
    hash = dm2_v1_wall_hash_bytes(hash,
                                  (const uint8_t *)&command->material_receipt_hash,
                                  sizeof(command->material_receipt_hash));
    return hash;
}

/* ── Lighting helper: DM2 object illumination decay ──────────────── */
/* ReDMCSB DUNVIEW.C F0115:4960-5037 uses a per-view depth scale table for
 * object sprites; outside the valid depth window objects are not drawn. We
 * model the same boundary as a hard light-radius clip, where distance values
 * at or beyond the source radius extinguish to zero. */
uint8_t dm2_v1_viewport_object_light_level(uint8_t base_light_level,
                                           int distance_tiles,
                                           const DM2_CreatureSprite *source)
{
    if (!source) return base_light_level;
    if (source->light_radius == 0) return 0;
    if (distance_tiles < 0) return base_light_level;
    if (distance_tiles >= (int)source->light_radius) return 0;

    return (uint8_t)((int)base_light_level *
                     ((int)source->light_radius - distance_tiles) /
                     (int)source->light_radius);
}

int dm2_v1_viewport_g1_tile_class_to_square_type(uint8_t tile_class)
{
    /* skproject/SKWIN/DME.h::tileTypeIndex: these are G1 byte-square
     * classes, not DM2_SquareType enum values. */
    switch (tile_class) {
    case 0u: return DM2_SQUARE_WALL;
    case 1u: return DM2_SQUARE_FLOOR;
    case 4u: return DM2_SQUARE_DOOR;
    default: return -1;
    }
}

int dm2_v1_viewport_project_map_to_sprite(
    int map_x,
    int map_y,
    int party_dir,
    int party_x,
    int party_y,
    DM2_V1_ViewportSpritePlacement *out)
{
    static const int dx[4] = { 0, 1, 0, -1 };
    static const int dy[4] = { -1, 0, 1, 0 };
    static const int y_by_depth[4] = { 98, 84, 72, 62 };
    static const int lateral_step_by_depth[4] = { 48, 40, 30, 22 };
    const int center_x = 112;
    int dir;
    int right;
    int rel_x;
    int rel_y;
    int forward;
    int lateral;
    int depth;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    dir = party_dir & 3;
    right = (dir + 1) & 3;
    rel_x = map_x - party_x;
    rel_y = map_y - party_y;
    forward = rel_x * dx[dir] + rel_y * dy[dir];
    lateral = rel_x * dx[right] + rel_y * dy[right];

    if (forward < 1 || forward > 4 || lateral < -2 || lateral > 2) {
        return 0;
    }

    /* skproject/SKWIN renders missiles and creature-carried map chips through
     * the visible cell order before DRAW_MAP_CHIP scales the selected sprite.
     * This helper owns Firestaff's bounded first-person placement contract for
     * those runtime-produced map-coordinate overlays. */
    depth = forward - 1;
    out->visible = 1;
    out->depth = depth;
    out->screen_x = center_x + lateral * lateral_step_by_depth[depth];
    out->screen_y = y_by_depth[depth];
    return 1;
}

int dm2_v1_viewport_static_object_cell_for_map(
    int map_x,
    int map_y,
    int party_dir,
    int party_x,
    int party_y,
    int *out_cell,
    int *out_pass)
{
    static const int dx[4] = { 0, 1, 0, -1 };
    static const int dy[4] = { -1, 0, 1, 0 };
    /* SKProject dm2data.cpp::table1d7029 cell order for the visible 4×3 grid,
     * indexed by [forward-1][lateral+1].  Forward runs D0..D3 away from the
     * party; lateral runs L(-1)/C(0)/R(+1).  Cell 0 (D0C) and the side/deep
     * rows are carried here so the runtime can apply the same source pass
     * lookup; downstream DRAW_ITEM placement remains blocked until the
     * visibility mask, record ordinal and Rect14 tables for that cell are
     * source-owned. */
    static const int8_t cell_by_forward_lateral[4][3] = {
        /* forward 1: D0L/D0C/D0R */ {  1,  0,  2 },
        /* forward 2: D1L/D1C/D1R */ {  9,  3, 10 },
        /* forward 3: D2L/D2C/D2R */ {  7,  6,  8 },
        /* forward 4: D3L/D3C/D3R */ {  4, 11,  5 },
    };
    int dir;
    int right;
    int rel_x;
    int rel_y;
    int forward;
    int lateral;
    int cell;
    int pass;

    if (out_cell) *out_cell = -1;
    if (out_pass) *out_pass = -1;
    dir = party_dir & 3;
    right = (dir + 1) & 3;
    rel_x = map_x - party_x;
    rel_y = map_y - party_y;
    forward = rel_x * dx[dir] + rel_y * dy[dir];
    lateral = rel_x * dx[right] + rel_y * dy[right];

    if (forward < 1 || forward > 4 || lateral < -1 || lateral > 1) {
        return 0;
    }
    cell = (int)cell_by_forward_lateral[forward - 1][lateral + 1];
    pass = dm2_v1_viewport_draw_dungeon_tiles_pass_for_cell(cell);
    /* D0C (cell 0) has no table1d7029 pass and is not promoted to a generic
     * object draw.  Side/deep cells have passes but are still blocked by
     * dm2_v1_viewport_static_object_source_plan until their placement tables
     * are recovered. */
    if (pass < 0) return 0;
    if (out_cell) *out_cell = cell;
    if (out_pass) *out_pass = pass;
    return 1;
}

int dm2_v1_viewport_static_object_source_plan(
    int source_cell, int source_pass, int item_category,
    int object_direction, int container_open, int draw_slot,
    int view_dir,
    uint16_t record_list_ordinal, uint32_t visibility_mask_5x5,
    DM2_V1_StaticObjectSourcePlan *out)
{
    /* skproject SkGlobal.cpp glbTabYAxisDistance (_4976_412d) for cells
     * 0..15.  DRAW_PUT_DOWN_ITEM returns early when the distance exceeds 3,
     * so D4 cells (16..22) never draw static objects. */
    static const int8_t y_distance_by_cell[16] = {
        0, 0, 0, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3
    };
    /* skproject SkGlobal.cpp glbTabXAxisDistance (_4976_4116) for cells
     * 0..15; DRAW_ITEM consumes it for the chest mirror rule (bp08). */
    static const int8_t x_distance_by_cell[16] = {
        0, -1, 1, 0, -1, 1, 0, -1, 1, -2, 2, 0, -1, 1, -2, 2
    };
    static const uint8_t position_5x5_by_direction[4] = { 6, 8, 18, 16 };
    /* skproject SkGlobal.cpp _4976_418e rows 0..3 (distance stretch). */
    static const uint8_t stretch_by_distance[4][4] = {
        { 0x60, 0x57, 0x4e, 0x47 },
        { 0x40, 0x3a, 0x34, 0x2f },
        { 0x2b, 0x27, 0x23, 0x1f },
        { 0x1c, 0x1a, 0x17, 0x15 }
    };
    static const int8_t slot_delta[8] = { 0, 1, 2, 3, 0, -3, -2, -1 };
    static const uint8_t slot_axis[16][2] = {
        { 2, 5 }, { 0, 6 }, { 5, 7 }, { 3, 0 },
        { 7, 1 }, { 1, 2 }, { 6, 3 }, { 3, 3 },
        { 5, 5 }, { 2, 6 }, { 7, 7 }, { 1, 0 },
        { 3, 1 }, { 6, 2 }, { 1, 3 }, { 5, 3 }
    };
    int position_5x5;
    int y_distance;
    int vertical_row;
    int relative_direction;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    /* Cell 0 has no table1d7029 pass (the party square is drawn through a
     * separate source call) and D4 cells (16..22) are rejected by
     * DRAW_PUT_DOWN_ITEM's distance guard; side/deep cells 1..15 are admitted
     * because glbTabYAxisDistance, _4976_418e and the display-order tables
     * prove their placement. */
    if (source_cell < 1 || source_cell > 15 ||
        source_pass != dm2_v1_viewport_draw_dungeon_tiles_pass_for_cell(
            source_cell) ||
        (item_category != 0x10 && item_category != 0x14) ||
        draw_slot < 0 || draw_slot >= 16) {
        return 0;
    }

    /* SKWIN/SkWinCore.cpp DRAW_ITEM (tt == 0) anchors the object at
     * QUERY_OBJECT_5x5_POS(rl, _4976_5aa0), the record direction rotated into
     * view space.  For the four corner anchors of _4976_4a04 that rotation is
     * exactly _4976_4a04[(object_direction - view_dir) & 3]. */
    relative_direction = (object_direction - view_dir) & 3;
    position_5x5 = position_5x5_by_direction[relative_direction];
    y_distance = y_distance_by_cell[source_cell];
    vertical_row = 4 - position_5x5 / 5;
    if (vertical_row < 1 || vertical_row > 3) {
        return 0;
    }

    /* DRAW_STATIC_OBJECT filters by its source-owned 5x5 visibility mask before
     * calling DRAW_PUT_DOWN_ITEM.  The caller supplies the mask from the runtime
     * record scan; a zero mask or a missing position bit keeps the plan
     * evidence-only and fail-closed until real game data is available. */
    out->source_cell = source_cell;
    out->source_pass = source_pass;
    out->position_5x5 = position_5x5;
    out->clip_rect_id = 0x8000 | (5000 + source_cell * 25 + position_5x5);
    out->y_distance = y_distance;
    /* The source's RCJ(4, 1 + bp0a) indexes the four-byte row modulo 4. */
    out->stretch_factor64 =
        stretch_by_distance[y_distance][(1 + vertical_row) & 3];
    out->image_field = (item_category == 0x14 && container_open) ? 4 : 0;
    /* SKWIN/SkWinCore.cpp DRAW_ITEM bp08: a chest mirrors when the cell's
     * x-axis distance is 0 and the anchor sits right of centre (bp16 > 2), or
     * unconditionally when the cell is on the right side (bp14 == 1).  Left
     * and far-side cells never mirror through this rule. */
    out->flip_mirror = item_category == 0x14 &&
        ((x_distance_by_cell[source_cell] == 0 && position_5x5 % 5 > 2) ||
         x_distance_by_cell[source_cell] == 1);
    /* SKWIN/SkWinCore.cpp DRAW_ITEM lines 23961-23966 and QUERY_TEMP_PICST:
     * yy accumulates _4976_41de[_4976_41b0[vv][0]] and becomes the offx added
     * to the GDAT image x-offset (ExtendedPicture.w28); si accumulates
     * _4976_41de[_4976_41b0[vv][1]] and becomes the offy added to the
     * y-offset (w30).  DME.h:1829-1830 names w28 x-offset and w30 y-offset. */
    out->slot_x_offset = slot_delta[slot_axis[draw_slot][0]];
    out->slot_y_offset = slot_delta[slot_axis[draw_slot][1]];
    out->object_direction = object_direction & 3;
    out->record_list_ordinal = record_list_ordinal;
    out->visibility_mask_5x5 = visibility_mask_5x5;
    return 1;
}

int dm2_v1_viewport_possession_slot_placement(
    const DM2_V1_ViewportSpritePlacement *base,
    int possession_slot,
    DM2_V1_ViewportSpritePlacement *out)
{
    int slot;

    if (!base || !out || !base->visible) {
        if (out) memset(out, 0, sizeof(*out));
        return 0;
    }
    slot = possession_slot;
    if (slot < 0) {
        slot = 0;
    }
    *out = *base;
    /* skproject walks Creature::possession in chain order and draws each
     * dbWeapon..dbMiscellaneous_item as a separate map-chip overlay.  The
     * bounded Firestaff bridge keeps stable per-slot separation until the
     * exact source placement table is fully decoded. */
    out->screen_x = base->screen_x + slot * 6;
    out->screen_y = base->screen_y + slot * 4;
    return 1;
}

int dm2_v1_viewport_calc_stretched_size(int value, int factor64)
{
    /* skproject/SKWIN/SkWinCore.cpp CALC_STRETCHED_SIZE:
     * (i16(val * fact64) + (fact64 >> 1)) >> 6. */
    return (int)(((int16_t)(value * factor64) + (factor64 >> 1)) >> 6);
}

int dm2_v1_viewport_rotate_5x5_pos(int pos5x5, int dir)
{
    int x;
    int y;
    int tmp;

    if (pos5x5 < 0 || pos5x5 > 24) {
        return -1;
    }
    x = (pos5x5 % 5) - 2;
    y = (pos5x5 / 5) - 2;
    switch (dir & 3) {
    case 1:
        tmp = x;
        x = y;
        y = -tmp;
        break;
    case 2:
        x = -x;
        y = -y;
        break;
    case 3:
        tmp = x;
        x = -y;
        y = tmp;
        break;
    default:
        break;
    }
    return x + ((y + 2) * 5) + 2;
}

int dm2_v1_viewport_creature_blit_rect_id(int cell_pos,
                                          int pos5x5,
                                          int dir)
{
    int rotated = dm2_v1_viewport_rotate_5x5_pos(pos5x5, dir);

    if (cell_pos < 0 || cell_pos > 3 || rotated < 0) {
        return -1;
    }
    /* skproject/SKWIN/SkWinCore.cpp QUERY_CREATURE_BLIT_RECTI returns
     * ROTATE_5x5_POS(pos, dir) + cellPos * 25 + 5000. */
    return rotated + (cell_pos * 25) + 5000;
}

int dm2_v1_viewport_object_5x5_pos(int object_direction, int view_dir)
{
    /* skproject/SKWIN/SkWinCore.cpp QUERY_OBJECT_5x5_POS: dbWeapon ..
     * dbMiscellaneous_item records anchor at _4976_4a04[Dir()] rotated by the
     * view direction (_4976_5aa0).  _4976_4a04 = {6, 8, 18, 16} (n,e,s,w). */
    static const uint8_t anchor_by_direction[4] = { 6, 8, 18, 16 };

    if (object_direction < 0 || view_dir < 0) {
        return -1;
    }
    return dm2_v1_viewport_rotate_5x5_pos(
        anchor_by_direction[object_direction & 3], view_dir & 3);
}

uint32_t dm2_v1_viewport_static_object_visibility_bit(int object_direction,
                                                      int view_dir)
{
    /* skproject/SKWIN/SkWinCore.cpp line 45370:
     * (*_4976_5be2)[cellPos] |= U32(1) << QUERY_OBJECT_5x5_POS(record, view);
     * DRAW_STATIC_OBJECT tests this bit before DRAW_PUT_DOWN_ITEM. */
    int pos = dm2_v1_viewport_object_5x5_pos(object_direction, view_dir);

    if (pos < 0 || pos > 24) {
        return 0u;
    }
    return 1u << (unsigned)pos;
}

int dm2_v1_viewport_dir_from_5x5_pos(int pos5x5)
{
    /* skproject/SKWIN/SkWinCore.cpp DIR_FROM_5x5_POS. */
    switch (pos5x5) {
    case 6:  return 0; /* north west */
    case 8:  return 1; /* north east */
    case 18: return 2; /* south east */
    case 16: return 3; /* south west */
    case 12: return 4; /* center */
    default: return -1;
    }
}

int dm2_v1_viewport_static_object_display_order(int cell_pos,
                                                uint8_t out_order[25])
{
    /* skproject/SKWIN/SkWinCore.cpp DRAW_STATIC_OBJECT lines 47160-47174 and
     * SkGlobal.cpp tlbDisplayOrderLeft/_4976_439a, tlbDisplayOrderRight/
     * _4976_43b3, tlbDisplayOrderCenter/_4976_43cc.  The order is selected by
     * the sign of glbTabXAxisDistance[cell_pos]; cell 0 iterates only the
     * first 15 entries. */
    static const uint8_t order_left[25] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
        13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24
    };
    static const uint8_t order_right[25] = {
        4, 3, 2, 1, 0, 9, 8, 7, 6, 5, 14, 13, 12,
        11, 10, 19, 18, 17, 16, 15, 24, 23, 22, 21, 20
    };
    static const uint8_t order_center[25] = {
        0, 4, 1, 3, 2, 5, 9, 6, 8, 7, 10, 14, 11,
        13, 12, 15, 19, 16, 18, 17, 20, 24, 21, 23, 22
    };
    /* skproject SkGlobal.cpp glbTabXAxisDistance (_4976_4116) cells 0..15;
     * DRAW_STATIC_OBJECT rejects cells above 15 before any order lookup. */
    static const int8_t x_axis_by_cell[16] = {
        0, -1, 1, 0, -1, 1, 0, -1, 1, -2, 2, 0, -1, 1, -2, 2
    };
    const uint8_t *order;
    int count;

    if (!out_order || cell_pos < 0 || cell_pos > 15) {
        return 0;
    }
    if (x_axis_by_cell[cell_pos] < 0) {
        order = order_left;
    } else if (x_axis_by_cell[cell_pos] > 0) {
        order = order_right;
    } else {
        order = order_center;
    }
    count = (cell_pos == 0) ? 15 : 25;
    memcpy(out_order, order, (size_t)count);
    return count;
}

int dm2_v1_viewport_static_object_draw_positions(
    int cell_pos, uint32_t visibility_mask_5x5, uint8_t out_positions[25])
{
    /* skproject/SKWIN/SkWinCore.cpp DRAW_STATIC_OBJECT lines 47174-47190:
     * walk the source display order and keep the positions whose
     * visibility-mask bit is set; each survivor fires DRAW_PUT_DOWN_ITEM. */
    uint8_t order[25];
    int count;
    int kept = 0;
    int i;

    if (!out_positions) {
        return 0;
    }
    count = dm2_v1_viewport_static_object_display_order(cell_pos, order);
    for (i = 0; i < count; ++i) {
        if ((visibility_mask_5x5 & (1u << order[i])) != 0u) {
            out_positions[kept++] = order[i];
        }
    }
    return kept;
}

int dm2_v1_viewport_creature_occupancy_5x5(int anchor5x5,
                                           int creature_dir,
                                           int party_dir)
{
    /* skproject/SKWIN/SkWinCore.cpp QUERY_CREATURE_5x5_POS: a creature whose
     * info slot is 0xff centres at 12; otherwise its 5x5 anchor rotates by
     * (party_dir - creature_dir) & 3. */
    if (anchor5x5 == 0xff) {
        return 12;
    }
    if (anchor5x5 < 0 || anchor5x5 > 24 || creature_dir < 0 ||
        party_dir < 0) {
        return -1;
    }
    return dm2_v1_viewport_rotate_5x5_pos(
        anchor5x5, (party_dir - creature_dir) & 3);
}

int dm2_v1_viewport_occupancy_grid_coords(int cell_pos,
                                          int pos5x5,
                                          int *out_x,
                                          int *out_y)
{
    /* skproject/SKWIN/SkWinCore.cpp DRAW_STATIC_OBJECT lines 47179-47185:
     * the _4976_5aa4 grid coordinate of display position bp08 is
     * (_4976_43f5[cell][0] + _4976_4415[bp08][0],
     *  _4976_43f5[cell][1] - _4976_4415[bp08][1]) and _4976_4415 is the
     * identity (pos % 5, pos / 5) split.  SkGlobal.cpp _4976_43f5 cells 0-15. */
    static const uint8_t grid_base[16][2] = {
        { 8, 4 }, { 4, 4 }, { 12, 4 }, { 8, 8 },
        { 4, 8 }, { 12, 8 }, { 8, 12 }, { 4, 12 },
        { 12, 12 }, { 0, 12 }, { 16, 12 }, { 8, 16 },
        { 4, 16 }, { 12, 16 }, { 0, 16 }, { 16, 16 }
    };

    if (!out_x || !out_y || cell_pos < 0 || cell_pos > 15 ||
        pos5x5 < 0 || pos5x5 > 24) {
        return 0;
    }
    *out_x = grid_base[cell_pos][0] + pos5x5 % 5;
    *out_y = grid_base[cell_pos][1] - pos5x5 / 5;
    return 1;
}

int dm2_v1_viewport_static_object_display_index(int cell_pos,
                                                int pos5x5)
{
    uint8_t order[25];
    int count = dm2_v1_viewport_static_object_display_order(cell_pos, order);

    for (int i = 0; i < count; ++i) {
        if (order[i] == pos5x5) {
            return i;
        }
    }
    return -1;
}

int dm2_v1_viewport_flying_item_scale64(int y_distance,
                                        int dir_from_5x5)
{
    /* skproject/SKWIN/SkWinCore.cpp DRAW_FLYING_ITEM line 47013 and
     * SkGlobal.cpp _4976_41a9 (7 entries).  A negative table index blocks
     * the source draw (`if (bp16 < 0) continue`). */
    static const uint8_t scale_by_band[7] = {
        0x40, 0x34, 0x2b, 0x23, 0x1c, 0x17, 0x13
    };
    int band = (y_distance << 1) - (dir_from_5x5 >> 1);

    if (band < 0 || band > 6) {
        return -1;
    }
    return scale_by_band[band];
}

int dm2_v1_viewport_flying_item_image_field(int frame_class,
                                            int timer_direction,
                                            int view_dir,
                                            int tile_x,
                                            int tile_y,
                                            int x_distance,
                                            int dir_from_5x5,
                                            int cls1_is_spell,
                                            int *out_flip)
{
    /* skproject/SKWIN/SkWinCore.cpp DRAW_FLYING_ITEM lines 47024-47096: the
     * _48ae_011a frame class (bp0c) selects the base field; the missile
     * timer direction parity (bp0a) picks side-on 0x0c vs front frames; the
     * tile parity (bp1e+bp20), the cell x distance (bp1c) and the direction
     * drive the mirror bits si (masked by bp06 by the caller). */
    int di = dir_from_5x5;
    int bp13;
    int si = 0;

    if (out_flip) *out_flip = 0;
    if (di < 0) return -1;
    if (frame_class == 3) {
        bp13 = 8;
    } else if ((timer_direction & 1) != (view_dir & 1)) {
        bp13 = 0xc;
        if (frame_class == 0) {
            if (di == 0 || di == 3) {
                si |= 1;
            }
            if (((tile_x + tile_y) & 1) == 0) {
                si ^= 1;
            }
        } else if (((view_dir + 1) & 3) == timer_direction) {
            si |= 1;
        }
    } else {
        if (frame_class == 0) {
            if (((tile_x + tile_y) & 1) != 0) {
                si |= 2;
                bp13 = (di < 2) ? 8 : 9;
            } else {
                bp13 = (di >= 2) ? 8 : 9;
            }
        } else if (frame_class == 2 ||
                   (frame_class == 1 && timer_direction != view_dir)) {
            bp13 = 8;
        } else {
            bp13 = 10;
        }
        if (x_distance < 1 &&
            (x_distance != 0 || (di != 1 && di != 2))) {
            si |= 1;
        }
        if ((di & 1) != 0 && cls1_is_spell) {
            si |= 2;
        }
    }
    if (out_flip) *out_flip = si;
    return bp13;
}

int dm2_v1_viewport_interface_rect14_placement(
    const uint8_t row14[14],
    int cell_pos,
    int distance_stretch_factor64,
    DM2_V1_InterfaceRect14Placement *out)
{
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!row14 || cell_pos < 0 || cell_pos > 3 ||
        row14[0] > 24u || distance_stretch_factor64 <= 0) {
        return 0;
    }

    /* skproject/SKWIN/SkWinCore.cpp QUERY_CREATURE_PICST consumes
     * _4976_5a98[row][0] as the 5x5 anchor, [1] as a signed side offset,
     * [2..5] as direction image fields, [6..9] via CALC_STRETCHED_SIZE,
     * and [10..13] as per-direction flags. */
    out->valid = 1;
    out->base_5x5 = row14[0];
    out->lateral_offset = (int8_t)row14[1];
    out->cell_pos = (uint16_t)cell_pos;
    for (int dir = 0; dir < 4; ++dir) {
        int rect_id = dm2_v1_viewport_creature_blit_rect_id(
            cell_pos, row14[0], dir);
        if (rect_id < 0) {
            memset(out, 0, sizeof(*out));
            return 0;
        }
        out->blit_rect_id[dir] = (uint16_t)rect_id;
        out->image_field[dir] = row14[2 + dir];
        out->stretch_source[dir] = row14[6 + dir];
        out->stretched_size[dir] = (uint16_t)
            dm2_v1_viewport_calc_stretched_size(row14[6 + dir],
                                                distance_stretch_factor64);
        out->flags[dir] = row14[10 + dir];
    }
    return 1;
}

int dm2_v1_viewport_enrich_static_object_source_plan_with_rect14(
    const uint8_t *rows,
    uint32_t row_count,
    uint32_t table_hash,
    int object_direction,
    int party_dir,
    DM2_V1_StaticObjectSourcePlan *plan)
{
    int expected_rect_id;
    int view_dir;
    uint32_t row_hash;
    uint32_t placement_hash;
    const uint8_t *matched;

    if (!plan || !rows || row_count == 0u || table_hash == 0u ||
        plan->source_cell < 0 || plan->source_cell > 3 ||
        plan->position_5x5 < 0 || plan->position_5x5 > 24 ||
        plan->clip_rect_id == 0) {
        return 0;
    }

    expected_rect_id = plan->clip_rect_id & 0x7fff;
    view_dir = (party_dir - object_direction) & 3;
    matched = NULL;
    for (uint32_t r = 0; r < row_count; ++r) {
        const uint8_t *row = rows + (size_t)r * 14u;
        int rect_id;

        if (row[0] > 24u) continue;
        /* The source plan stores the view-relative 5x5 anchor (already
         * rotated by object direction).  Match the Rect14 row that shares
         * that anchor; the per-direction fields are then indexed by the
         * view-relative direction. */
        if ((int)row[0] != plan->position_5x5) continue;
        rect_id = dm2_v1_viewport_creature_blit_rect_id(
            plan->source_cell, (int)row[0], 0);
        if (rect_id >= 0 && rect_id == expected_rect_id) {
            matched = row;
            break;
        }
    }
    if (!matched) return 0;

    /* SKWIN/SkWinCore.cpp DRAW_ITEM uses _4976_5a98[row][2+reldir] as the
     * image field index, [6+reldir] as the CALC_STRETCHED_SIZE source, and
     * [10+reldir] as per-direction flags (bit 0 = horizontal mirror). */
    plan->rect14_applied = 1;
    plan->rect14_image_field = matched[2 + view_dir];
    plan->rect14_scale64 = (int)matched[6 + view_dir];
    plan->rect14_lateral_offset = (int8_t)matched[1];
    plan->rect14_flip_mirror = (int)(matched[10 + view_dir] & 1u);

    row_hash = dm2_v1_wall_hash_bytes(2166136261u, matched, 14u);
    placement_hash = row_hash;
    placement_hash = dm2_v1_wall_hash_bytes(
        placement_hash, (const uint8_t *)&expected_rect_id,
        sizeof(expected_rect_id));
    placement_hash = dm2_v1_wall_hash_bytes(
        placement_hash, (const uint8_t *)&plan->rect14_scale64,
        sizeof(plan->rect14_scale64));
    placement_hash = dm2_v1_wall_hash_bytes(
        placement_hash, (const uint8_t *)&plan->rect14_flip_mirror,
        sizeof(plan->rect14_flip_mirror));
    placement_hash = dm2_v1_wall_hash_bytes(
        placement_hash, (const uint8_t *)&plan->rect14_lateral_offset,
        sizeof(plan->rect14_lateral_offset));

    plan->rect14_row_hash = row_hash ? row_hash : 1u;
    plan->rect14_placement_hash = placement_hash ? placement_hash : 1u;
    (void)table_hash; /* table provenance is carried by the caller's receipt. */
    return 1;
}

int dm2_v1_viewport_build_hud_chrome_plan(
    int is_outdoor,
    DM2_V1_HudChromeRenderPlan *out_plan)
{
    static const uint8_t icon_x[DM2_V1_HUD_ACTION_ICON_COUNT] =
        { 20, 70, 120, 170, 220 };
    const int action_y = DM2_VP_HEIGHT - DM2_VP_CHROME_BOT;
    const int panel_x = 240;

    if (!out_plan) {
        return 0;
    }
    memset(out_plan, 0, sizeof(*out_plan));
    out_plan->outdoor = is_outdoor ? 1 : 0;
    out_plan->top_bar_rect =
        (DM2_V1_ViewportRect){ 0, 0, DM2_VP_WIDTH, DM2_VP_CHROME_TOP };
    out_plan->top_divider_rect =
        (DM2_V1_ViewportRect){ 0, DM2_VP_CHROME_TOP, DM2_VP_WIDTH, 1 };
    out_plan->action_strip_rect =
        (DM2_V1_ViewportRect){ 0, action_y, DM2_VP_WIDTH, DM2_VP_CHROME_BOT };
    out_plan->action_divider_rect =
        (DM2_V1_ViewportRect){ 0, action_y - 1, DM2_VP_WIDTH, 1 };
    out_plan->gold_box_rect =
        (DM2_V1_ViewportRect){ DM2_VP_WIDTH - 40, action_y + 4, 36, 16 };
    out_plan->gold_coin_rect =
        (DM2_V1_ViewportRect){ out_plan->gold_box_rect.x + 2,
                               out_plan->gold_box_rect.y + 2, 12, 12 };
    out_plan->gold_label_rect =
        (DM2_V1_ViewportRect){ out_plan->gold_box_rect.x + 16,
                               out_plan->gold_box_rect.y + 3, 14, 7 };
    /* skproject SKULLWIN/c_gdatfile.cpp DM2_LOAD_GDAT_INTERFACE_00_02
     * loads interface category 1/image 0/field 7 before the runtime HUD
     * is drawn.  Firestaff keeps these as renderer-private GDAT keys so
     * the asset provider can resolve real GRAPHICS.DAT surfaces. */
    out_plan->top_bar_gdat_index =
        dm2_v1_viewport_hud_core_graphic_index(
            DM2_V1_VIEWPORT_GFX_HUD_CORE_TOP_BAR);
    out_plan->action_strip_gdat_index =
        dm2_v1_viewport_hud_core_graphic_index(
            DM2_V1_VIEWPORT_GFX_HUD_CORE_ACTION_STRIP);
    out_plan->gold_box_gdat_index =
        dm2_v1_viewport_hud_core_graphic_index(
            DM2_V1_VIEWPORT_GFX_HUD_CORE_GOLD_BOX);
    out_plan->action_icon_count = DM2_V1_HUD_ACTION_ICON_COUNT;
    for (int i = 0; i < out_plan->action_icon_count; ++i) {
        DM2_V1_HudIconRender *icon = &out_plan->action_icons[i];
        icon->frame_rect =
            (DM2_V1_ViewportRect){ icon_x[i], action_y + 6, 20, 16 };
        icon->fill_rect =
            (DM2_V1_ViewportRect){ icon->frame_rect.x + 2,
                                   icon->frame_rect.y + 2, 16, 12 };
        icon->gdat_index = dm2_v1_viewport_hud_action_icon_graphic_index(i);
    }
    if (!out_plan->outdoor) {
        out_plan->portrait_separator_dark_rect =
            (DM2_V1_ViewportRect){ panel_x, DM2_VP_CHROME_TOP, 1,
                                   DM2_VP_HEIGHT - DM2_VP_CHROME_TOP -
                                       DM2_VP_CHROME_BOT };
        out_plan->portrait_separator_light_rect =
            (DM2_V1_ViewportRect){ panel_x + 1, DM2_VP_CHROME_TOP, 1,
                                   DM2_VP_HEIGHT - DM2_VP_CHROME_TOP -
                                       DM2_VP_CHROME_BOT };
        out_plan->portrait_panel_rect =
            (DM2_V1_ViewportRect){ panel_x + 2, DM2_VP_CHROME_TOP,
                                   DM2_VP_WIDTH - (panel_x + 2),
                                   DM2_VP_HEIGHT - DM2_VP_CHROME_TOP -
                                       DM2_VP_CHROME_BOT };
        out_plan->portrait_panel_gdat_index =
            dm2_v1_viewport_hud_core_graphic_index(
                DM2_V1_VIEWPORT_GFX_HUD_CORE_PORTRAIT_PANEL);
        out_plan->champion_slot_count = DM2_V1_HUD_CHAMPION_SLOT_COUNT;
        for (int slot = 0; slot < out_plan->champion_slot_count; ++slot) {
            int py = DM2_VP_CHROME_TOP + 2 + slot * 36;
            DM2_V1_HudChampionSlotRender *champ =
                &out_plan->champion_slots[slot];
            champ->frame_rect =
                (DM2_V1_ViewportRect){ panel_x + 4, py,
                                       DM2_VP_WIDTH - 8 - (panel_x + 4),
                                       28 };
            champ->fill_rect =
                (DM2_V1_ViewportRect){ panel_x + 6, py + 2,
                                       DM2_VP_WIDTH - 6 - (panel_x + 6),
                                       22 };
        }
    }
    return 1;
}

static uint8_t dm2_v1_hud_clamp_pct(int pct)
{
    if (pct < 0) {
        return 0;
    }
    if (pct > 100) {
        return 100;
    }
    return (uint8_t)pct;
}

static int dm2_v1_hud_name_marker_width(const char *name)
{
    int len = 0;
    if (!name) {
        return 0;
    }
    while (len < DM2_V1_HUD_CHAMPION_NAME_MAX && name[len]) {
        ++len;
    }
    return len * 3;
}

static DM2_V1_ViewportRect dm2_v1_hud_bar_fill(
    const DM2_V1_ViewportRect *bar,
    uint8_t pct)
{
    DM2_V1_ViewportRect fill = { 0, 0, 0, 0 };
    if (!bar || bar->w <= 0 || bar->h <= 0) {
        return fill;
    }
    fill = *bar;
    fill.w = (bar->w * (int)dm2_v1_hud_clamp_pct((int)pct)) / 100;
    return fill;
}

int dm2_v1_viewport_build_hud_chrome_plan_for_party(
    int is_outdoor,
    const DM2_V1_HudPartyState *party,
    DM2_V1_HudChromeRenderPlan *out_plan)
{
    if (!dm2_v1_viewport_build_hud_chrome_plan(is_outdoor, out_plan)) {
        return 0;
    }
    if (!party || out_plan->outdoor) {
        return 1;
    }
    for (int slot = 0; slot < out_plan->champion_slot_count; ++slot) {
        DM2_V1_HudChampionSlotRender *dst =
            &out_plan->champion_slots[slot];
        const DM2_V1_HudChampionState *src = NULL;
        int py = dst->frame_rect.y;
        int marker_w;

        if (slot < party->champion_count &&
            slot < DM2_V1_HUD_CHAMPION_SLOT_COUNT) {
            src = &party->champions[slot];
        }
        if (!src || !src->occupied) {
            continue;
        }

        dst->occupied = 1;
        dst->leader = src->leader || slot == party->leader_index;
        dst->hp_pct = dm2_v1_hud_clamp_pct((int)src->hp_pct);
        dst->stamina_pct = dm2_v1_hud_clamp_pct((int)src->stamina_pct);
        dst->mana_pct = dm2_v1_hud_clamp_pct((int)src->mana_pct);
        if (src->stat_bar_color_source_bound && src->stat_bar_color < 16u) {
            dst->stat_bar_color = src->stat_bar_color;
            dst->stat_bar_color_source_bound = 1;
        }
        /* skproject passes Champion::HeroType directly to
         * DRAW_CHAMPION_PICTURE. Do not fold it into a local ordinal. */
        dst->portrait_index = src->portrait_index;
        dst->portrait_type_source_bound = src->portrait_type_source_bound;
        dst->state_source_bound = src->state_source_bound;
        memcpy(dst->name, src->name, sizeof(dst->name));
        dst->name[DM2_V1_HUD_CHAMPION_NAME_MAX] = '\0';
        dst->leader_mark_rect =
            (DM2_V1_ViewportRect){ dst->frame_rect.x + 2, py + 3, 3, 3 };
        dst->portrait_rect =
            (DM2_V1_ViewportRect){ dst->frame_rect.x + 4, py + 4, 18, 18 };
        marker_w = dm2_v1_hud_name_marker_width(src->name);
        dst->name_marker_rect =
            (DM2_V1_ViewportRect){ dst->frame_rect.x + 26, py + 2,
                                   marker_w, marker_w > 0 ? 6 : 0 };
        dst->hp_bar_rect =
            (DM2_V1_ViewportRect){ dst->frame_rect.x + 26, py + 9, 34, 3 };
        dst->stamina_bar_rect =
            (DM2_V1_ViewportRect){ dst->frame_rect.x + 26, py + 14, 34, 3 };
        dst->mana_bar_rect =
            (DM2_V1_ViewportRect){ dst->frame_rect.x + 26, py + 19, 34, 3 };
        dst->hp_fill_rect = dm2_v1_hud_bar_fill(&dst->hp_bar_rect,
                                                dst->hp_pct);
        dst->stamina_fill_rect = dm2_v1_hud_bar_fill(&dst->stamina_bar_rect,
                                                     dst->stamina_pct);
        dst->mana_fill_rect = dm2_v1_hud_bar_fill(&dst->mana_bar_rect,
                                                  dst->mana_pct);
    }
    return 1;
}

/* ── Transparency color (ReDMCSB DEFS.H C10_COLOR_FLESH = 10)
 * Used as skip color in wall blits. ── */
#define DM2_COLOR_TRANSPARENT  10

/* ── Viewport geometry ────────────────────────────────────────────── */
#define DM2_BLACK_AREA_TOP    0
#define DM2_BLACK_AREA_H     37
#define DM2_CEILING_Y         0
#define DM2_CEILING_H        29
#define DM2_FLOOR_Y          66
#define DM2_FLOOR_H          70
#define DM2_WALL_ZONE_D3_Y   25
#define DM2_WALL_ZONE_D2_Y   20
#define DM2_WALL_ZONE_D1_Y    9
#define DM2_WALL_ZONE_D0_Y    0

/* ── Wall frame table (12 entries, D3C..D0R) ─────────────────────────
 * Derived from ReDMCSB DUNVIEW.C G0163_aauc_Graphic558_Frame_Walls[12][8]
 * (lines 575-586), same as DM1. DM2 uses the same geometry constants.
 *
 * Index mapping (DUNVIEW.C:581-594):
 *   D3C=0, D3L=1, D3R=2, D2C=3, D2L=4, D2R=5,
 *   D1C=6, D1L=7, D1R=8, D0C=9, D0L=10, D0R=11
 *
 * Frame format: { X1, X2, Y1, Y2, ByteWidth, Height, X, Y }
 * Source: DUNVIEW.C:581-594 (G0163)
 * ─────────────────────────────────────────────────────────────────── */

const DM2_WallFrame g_dm2_wall_frames[DM2_SQ_COUNT] = {
    /* D3C */ {  74, 149, 25,  75,  64,  51,  18, 0 },
    /* D3L */ {   0,  83, 25,  75,  64,  51,  32, 0 },
    /* D3R */ { 139, 223, 25,  75,  64,  51,   0, 0 },
    /* D2C */ {  60, 163, 20,  90,  72,  71,  16, 0 },
    /* D2L */ {   0,  74, 20,  90,  72,  71,  61, 0 },
    /* D2R */ { 149, 223, 20,  90,  72,  71,   0, 0 },
    /* D1C */ {  32, 191,  9, 119, 128, 111,  48, 0 },
    /* D1L */ {   0,  63,  9, 119, 128, 111, 192, 0 },
    /* D1R */ { 160, 223,  9, 119, 128, 111,   0, 0 },
    /* D0C */ {   0, 223,  0, 135, 224, 136,   0, 0 },
    /* D0L */ {   0,  31,  0, 135,  16, 136,   0, 0 },
    /* D0R */ { 192, 223,  0, 135,  16, 136,   0, 0 },
};

/* DM2 wall set index table — negative = derived offset from wall set base.
 * Source: DUNVIEW.C:140-144, G3011-G3015 (I34E section).
 * DM2 uses different set indices than DM1 (G3060 variant, lines 170-175). */
static const int16_t __attribute__((unused)) s_dm2_wall_set [12] = {
    /* D3C */ -7,   /* G3060_i_WallSet_Wall_D3C */
    /* D3L */ -8,   /* G3061_i_WallSet_Wall_D3L */
    /* D3R */ -9,   /* G3062_i_WallSet_Wall_D3R */
    /* D2C */ -10,  /* G3063_i_WallSet_Wall_D2C */
    /* D2L */ -11,  /* G3064_i_WallSet_Wall_D2L */
    /* D2R */ -12,  /* G3065_i_WallSet_Wall_D2R */
    /* D1C */ -13,  /* G3066_i_WallSet_Wall_D1C */
    /* D1L */ -14,  /* G3067_i_WallSet_Wall_D1L (DM2-specific) */
    /* D1R */ -15,  /* G3068_i_WallSet_Wall_D1R (DM2-specific) */
    /* D0C */   0,
    /* D0L */ -16,  /* G3014_i_WallSet_Wall_D0L */
    /* D0R */ -17,  /* G3015_i_WallSet_Wall_D0R */
};

/* DM2 flipped wall set — horizontally mirrored L↔R per depth group.
 * Source: DUNVIEW.C:159-168, G3049-G3059 (WallSetFlipped). */
static const int16_t __attribute__((unused)) s_dm2_wall_set_flipped [12] = {
    /* D3C */ -18,  /* G3049_i_WallSetFlipped_Wall_D3C */
    /* D3L */ -19,  /* G3050_i_WallSetFlipped_Wall_D3L */
    /* D3R */ -20,  /* G3051_i_WallSetFlipped_Wall_D3R */
    /* D2C */ -21,  /* G3052_i_WallSetFlipped_Wall_D2C */
    /* D2L */ -22,  /* G3053_i_WallSetFlipped_Wall_D2L */
    /* D2R */ -23,  /* G3054_i_WallSetFlipped_Wall_D2R */
    /* D1C */ -24,  /* G3055_i_WallSetFlipped_Wall_D1C */
    /* D1L */ -25,  /* G3056_i_WallSetFlipped_Wall_D1L */
    /* D1R */ -26,  /* G3057_i_WallSetFlipped_Wall_D1R */
    /* D0C */   0,
    /* D0L */ -27,  /* G3058_i_WallSetFlipped_Wall_D0L */
    /* D0R */ -28,  /* G3059_i_WallSetFlipped_Wall_D0R */
};

/* DM2 door frame indices.
 * Source: DUNVIEW.C:148-157, G2116-G2119, G2196.
 * Different from DM1: DM2 door frames are larger/more ornate. */
static const int16_t __attribute__((unused)) s_dm2_door_frames [6] = {
    /* Top row (D1R,D1L,D1LCR,D2R,D2L,D2LCR) */
    /* DM2 door frame indices differ from DM1 (G2116=front D0C, etc.) */
    -35,  /* G2116_DoorFrameFrontD0C (DM2: larger door frames) */
    -33,  /* G2196_DoorFrameRightD1C */
    -34,  /* G2117_DoorFrameLeftD1C */
    -32,  /* G2118_DoorFrameLeftD2C */
    -30,  /* G2119_DoorFrameLeftD3C */
    -31,  /* G21xx_DoorFrameRightD2C (DM2 extension) */
};

/* ── Internal state ───────────────────────────────────────────────── */

/* Cached wall/floor/ceiling graphic index pairs (DM2 uses -1/-2 like DM1).
 * Source: DUNVIEW.C:126-127, G2108_Floor=-1, G2109_Ceiling=-2 */
#define DM2_GRAPHIC_FLOOR   DM2_V1_VIEWPORT_GFX_FLOOR
#define DM2_GRAPHIC_CEILING DM2_V1_VIEWPORT_GFX_CEILING

int dm2_v1_viewport_scene_material_graphic_index(int graphicsset_index,
                                                  int material_field)
{
    if (graphicsset_index < 0 || graphicsset_index > 0xff ||
        (material_field != DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_FLOOR &&
         material_field != DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_CEILING)) {
        return 0;
    }
    return DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_BASE -
           (graphicsset_index << 8) - material_field;
}

int dm2_v1_viewport_scene_material_graphic_address(int gdat_index,
                                                    int *out_graphicsset_index,
                                                    int *out_material_field)
{
    int packed = DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_BASE - gdat_index;
    int material_field = packed & 0xff;
    int graphicsset_index = (packed >> 8) & 0xff;

    if (!out_graphicsset_index || !out_material_field || packed < 0 ||
        packed > 0x0f01 ||
        gdat_index > DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_BASE ||
        (material_field != DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_FLOOR &&
         material_field != DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_CEILING)) {
        return 0;
    }
    *out_graphicsset_index = graphicsset_index;
    *out_material_field = material_field;
    return 1;
}

int dm2_v1_viewport_weather_environment_graphic_index(int graphicsset_index,
                                                       int environment_field)
{
    if (graphicsset_index < 0 || graphicsset_index > 0xff ||
        environment_field < (int)DM2_V1_WEATHER_BOLT_CMD_BASE ||
        environment_field > (int)DM2_V1_WEATHER_RAIN_STORM_CMD) {
        return 0;
    }
    return DM2_V1_VIEWPORT_GFX_WEATHER_ENVIRONMENT_BASE -
           (graphicsset_index << 8) - environment_field;
}

int dm2_v1_viewport_weather_environment_graphic_address(
    int gdat_index, int *out_graphicsset_index, int *out_environment_field)
{
    int packed = DM2_V1_VIEWPORT_GFX_WEATHER_ENVIRONMENT_BASE - gdat_index;
    int environment_field = packed & 0xff;
    int graphicsset_index = (packed >> 8) & 0xff;

    if (!out_graphicsset_index || !out_environment_field || packed < 0 ||
        packed > 0xff6c ||
        gdat_index > DM2_V1_VIEWPORT_GFX_WEATHER_ENVIRONMENT_BASE ||
        environment_field < (int)DM2_V1_WEATHER_BOLT_CMD_BASE ||
        environment_field > (int)DM2_V1_WEATHER_RAIN_STORM_CMD) {
        return 0;
    }
    *out_graphicsset_index = graphicsset_index;
    *out_environment_field = environment_field;
    return 1;
}

int dm2_v1_viewport_teleporter_map_chip_graphic_index(void)
{
    return DM2_V1_VIEWPORT_GFX_TELEPORTER_MAP_CHIP;
}

int dm2_v1_viewport_floor_gfx_map_chip_graphic_index(int floor_gfx_index)
{
    if (floor_gfx_index < 0 || floor_gfx_index > 0xff) return 0;
    return DM2_V1_VIEWPORT_GFX_FLOOR_GFX_MAP_CHIP_BASE - floor_gfx_index;
}

int dm2_v1_viewport_floor_gfx_map_chip_graphic_address(
    int gdat_index, int *out_floor_gfx_index)
{
    int index = DM2_V1_VIEWPORT_GFX_FLOOR_GFX_MAP_CHIP_BASE - gdat_index;

    if (!out_floor_gfx_index || index < 0 || index > 0xff ||
        gdat_index > DM2_V1_VIEWPORT_GFX_FLOOR_GFX_MAP_CHIP_BASE) {
        return 0;
    }
    *out_floor_gfx_index = index;
    return 1;
}

int dm2_v1_viewport_wall_gfx_map_chip_graphic_index(int wall_gfx_index)
{
    if (wall_gfx_index < 0 || wall_gfx_index > 0xff) return 0;
    return DM2_V1_VIEWPORT_GFX_WALL_GFX_MAP_CHIP_BASE - wall_gfx_index;
}

int dm2_v1_viewport_wall_gfx_map_chip_graphic_address(
    int gdat_index, int *out_wall_gfx_index)
{
    int index = DM2_V1_VIEWPORT_GFX_WALL_GFX_MAP_CHIP_BASE - gdat_index;

    if (!out_wall_gfx_index || index < 0 || index > 0xff ||
        gdat_index > DM2_V1_VIEWPORT_GFX_WALL_GFX_MAP_CHIP_BASE) {
        return 0;
    }
    *out_wall_gfx_index = index;
    return 1;
}

int dm2_v1_viewport_door_map_chip_graphic_index(int door_gfx_index)
{
    if (door_gfx_index < 0 || door_gfx_index > 0xff) return 0;
    return DM2_V1_VIEWPORT_GFX_DOOR_MAP_CHIP_BASE - door_gfx_index;
}

int dm2_v1_viewport_door_map_chip_graphic_address(
    int gdat_index, int *out_door_gfx_index)
{
    int index = DM2_V1_VIEWPORT_GFX_DOOR_MAP_CHIP_BASE - gdat_index;

    if (index < 0 || index > 0xff) return 0;
    if (out_door_gfx_index) *out_door_gfx_index = index;
    return 1;
}

int dm2_v1_viewport_dialogue_box_graphic_index(void)
{
    return DM2_V1_VIEWPORT_GFX_DIALOGUE_BOX;
}

/* ── Helper: resolve blit clipping gate ─────────────────────────── */

/* Clip gate for blit operations — prevents out-of-bounds writes.
 * Source: dm1_v1_viewport_3d_pc34_compat.c resolve_wall_blit_clip_gate */
typedef struct {
    int visible;
    int src_x, src_y;
    int dst_x, dst_y;
    int width, height;
} DM2_BlitClipGate;

static DM2_BlitClipGate dm2_resolve_blit_clip(
    const DM2_WallFrame *frame,
    int bitmap_w, int bitmap_h,
    int vp_w, int vp_h)
{
    DM2_BlitClipGate gate = {0};
    if (!frame || frame->byte_width == 0 || frame->height == 0) return gate;

    /* Frame source rect */
    int src_x = frame->blit_x;
    int src_y = frame->blit_y;
    int bw = frame->byte_width;
    int bh = frame->height;
    (void)bitmap_w; (void)bitmap_h; /* reserved for future full bitmap clip */

    /* Frame dest rect */
    int dst_x = frame->left_x;
    int dst_y = frame->top_y;
    int fw = frame->right_x - frame->left_x + 1;
    int fh = frame->bottom_y - frame->top_y + 1;
    (void)fw; (void)fh;

    /* Clip against viewport bounds */
    int clip_left   = (dst_x < 0) ? -dst_x : 0;
    int clip_top    = (dst_y < 0) ? -dst_y : 0;
    int clip_right  = (dst_x + bw > vp_w) ? (vp_w - (dst_x + bw)) : 0;
    int clip_bottom = (dst_y + bh > vp_h) ? (vp_h - (dst_y + bh)) : 0;

    if (clip_left >= bw || clip_top >= bh || clip_right >= bw || clip_bottom >= bh)
        return gate;

    gate.visible = 1;
    gate.src_x = src_x + clip_left;
    gate.src_y = src_y + clip_top;
    gate.dst_x = dst_x + clip_left;
    gate.dst_y = dst_y + clip_top;
    gate.width  = bw - clip_left + clip_right;
    gate.height = bh - clip_top  + clip_bottom;
    return gate;
}

/* ── Palette color constants (ReDMCSB DEFS.H color indices) ───────── */
enum DM2_ColorIndex {
    DM2_COL_BLACK   = 0,
    DM2_COL_DKGRAY  = 1,
    DM2_COL_MIDGRAY = 7,
    DM2_COL_LTGRAY  = 8,
    DM2_COL_FLESH   = 10,  /* C10_COLOR_FLESH = transparency */
    DM2_COL_WHITE   = 15,
    /* DM2 outdoor sky gradient */
    DM2_COL_SKY_DEEP = 9,
    DM2_COL_SKY_CYAN = 3,
    DM2_COL_GROUND   = 6,
};

static void dm2_v1_fill_rect(uint8_t *fb,
                             int stride,
                             const DM2_V1_ViewportRect *rect,
                             uint8_t color)
{
    int x0;
    int y0;
    int x1;
    int y1;

    if (!fb || !rect || stride <= 0 || rect->w <= 0 || rect->h <= 0) {
        return;
    }
    x0 = rect->x < 0 ? 0 : rect->x;
    y0 = rect->y < 0 ? 0 : rect->y;
    x1 = rect->x + rect->w;
    y1 = rect->y + rect->h;
    if (x1 > DM2_VP_WIDTH) x1 = DM2_VP_WIDTH;
    if (y1 > DM2_VP_HEIGHT) y1 = DM2_VP_HEIGHT;
    if (x0 >= x1 || y0 >= y1) {
        return;
    }
    for (int y = y0; y < y1; ++y) {
        memset(fb + y * stride + x0, color, (size_t)(x1 - x0));
    }
}

/* ── Initialization ───────────────────────────────────────────────── */

void dm2_v1_viewport_init(DM2_V1_ViewportState *s,
                          uint8_t *framebuffer,
                          int      fb_stride)
{
    if (!s) return;
    memset(s, 0, sizeof(*s));
    s->framebuffer = framebuffer;
    s->fb_stride   = fb_stride > 0 ? fb_stride : DM2_VP_WIDTH;
    s->surface_snapshot.framebuffer = framebuffer;
    s->surface_snapshot.width = DM2_VP_WIDTH;
    s->surface_snapshot.height = DM2_VP_HEIGHT;
    s->surface_snapshot.stride = (uint16_t)s->fb_stride;
    s->surface_snapshot.resolution = 8u;
    s->surface_snapshot.generation = 1u;
    /* Position, level, weather, clock and RNG are supplied by the live
     * source-owned runtime immediately after this allocation.  Keep the
     * zeroed state unavailable until that handoff instead of carrying the
     * former Hall-of-Champions/noon fixture values. */
    s->dirty        = 1;
    s->last_hud_core_gdat_hash = 2166136261u;

    /* Initialize all view squares to empty */
    for (int i = 0; i < DM2_SQ_COUNT; i++) {
        s->squares[i].square_type   = DM2_SQUARE_FLOOR;
        s->squares[i].flags        = DM2_SQF_NONE;
        s->squares[i].light_level  = 15;  /* full light */
        s->squares[i].door_open_pct = 0;
        s->squares[i].sprite_depth = i;   /* depth = square index */
    }

    /* Initialize sprite pools */
    s->creature_count  = 0;
    s->item_count      = 0;
    s->carried_item_present = 0;
    s->projectile_count = 0;

}

int dm2_v1_viewport_bind_surface(DM2_V1_ViewportState *s, uint8_t *framebuffer,
                                 int stride)
{
    if (!s || !framebuffer || stride < DM2_VP_WIDTH) return 0;
    s->framebuffer = framebuffer;
    s->fb_stride = stride;
    s->surface_snapshot.framebuffer = framebuffer;
    s->surface_snapshot.width = DM2_VP_WIDTH;
    s->surface_snapshot.height = DM2_VP_HEIGHT;
    s->surface_snapshot.stride = (uint16_t)stride;
    s->surface_snapshot.resolution = 8u;
    ++s->surface_snapshot.generation;
    if (!s->surface_snapshot.generation) ++s->surface_snapshot.generation;
    return 1;
}

int dm2_v1_viewport_surface_snapshot(const DM2_V1_ViewportState *s,
                                     DM2_V1_ViewportSurfaceSnapshot *out)
{
    if (!s || !out || !s->surface_snapshot.framebuffer ||
        !s->surface_snapshot.generation) return 0;
    *out = s->surface_snapshot;
    return 1;
}

void dm2_v1_viewport_set_party(DM2_V1_ViewportState *s,
                                int dir, int x, int y)
{
    if (!s) return;
    s->party_dir = (dir & 3);
    s->party_x   = x;
    s->party_y   = y;
    s->dirty     = 1;
}

void dm2_v1_viewport_set_outdoor(DM2_V1_ViewportState *s, int is_outdoor)
{
    if (!s) return;
    if (s->is_outdoor != (is_outdoor ? 1 : 0)) {
        s->is_outdoor = is_outdoor ? 1 : 0;
        s->dirty = 1;
    }
}

void dm2_v1_viewport_set_g1_first_map_runtime(
    DM2_V1_ViewportState *s,
    const DM2_V1_G1FirstMapRuntimeReceipt *receipt)
{
    if (!s || !receipt || !receipt->committed ||
        !receipt->incomplete_world || receipt->object_count != 0 ||
        receipt->blocked_record_reads != 0) {
        return;
    }
    s->g1_first_map_runtime = *receipt;
    s->dirty = 1;
}

void dm2_v1_viewport_set_g1_map0_teleporter_transition(
    DM2_V1_ViewportState *s,
    const DM2_V1_G1TeleporterTransitionReceipt *receipt)
{
    if (!s || !receipt || !receipt->committed ||
        !receipt->incomplete_world || receipt->source_map != 0 ||
        receipt->generic_record_reads != 0 ||
        receipt->blocked_record_reads != 0) {
        return;
    }
    s->g1_map0_teleporter_transition = *receipt;
    s->dirty = 1;
}

void dm2_v1_viewport_set_level(DM2_V1_ViewportState *s, int level)
{
    if (!s) return;
    s->dungeon_level = level;
    s->dirty = 1;
}

void dm2_v1_viewport_set_weather(DM2_V1_ViewportState *s,
                                   int weather,
                                   int rain_intensity)
{
    if (!s) return;
    s->weather = weather;
    s->rain_intensity = rain_intensity;
    s->dirty = 1;
}

void dm2_v1_viewport_set_time(DM2_V1_ViewportState *s, float time_of_day)
{
    if (!s) return;
    s->time_of_day = (time_of_day < 0) ? 0 : (time_of_day > 1 ? 1 : time_of_day);
    s->dirty = 1;
}

void dm2_v1_viewport_set_hud_party(DM2_V1_ViewportState *s,
                                   const DM2_V1_HudPartyState *party)
{
    if (!s) {
        return;
    }
    memset(&s->hud_party, 0, sizeof(s->hud_party));
    s->hud_party_valid = 0;
    if (party) {
        s->hud_party = *party;
        if (s->hud_party.champion_count < 0) {
            s->hud_party.champion_count = 0;
        }
        if (s->hud_party.champion_count > DM2_V1_HUD_CHAMPION_SLOT_COUNT) {
            s->hud_party.champion_count = DM2_V1_HUD_CHAMPION_SLOT_COUNT;
        }
        if (s->hud_party.leader_index < 0 ||
            s->hud_party.leader_index >= s->hud_party.champion_count) {
            s->hud_party.leader_index = 0;
        }
        s->hud_party_valid = 1;
    }
    s->dirty = 1;
}

void dm2_v1_viewport_set_hud_hand_action_source(
    DM2_V1_ViewportState *s,
    const DM2_V1_HudHandActionSource *source)
{
    DM2_V1_HudHandActionSource accepted;
    int expected_entry;
    int expected_rectno;

    if (!s) return;
    memset(&s->hud_hand_action_source, 0, sizeof(s->hud_hand_action_source));
    if (!source || !source->valid ||
        source->player_index >= DM2_V1_HUD_CHAMPION_SLOT_COUNT ||
        source->possession_index > 1u || source->left_or_right > 1u ||
        source->player_position > 3u || source->party_direction > 3u ||
        source->map_load_token == 0u || source->scene_control_hash == 0u ||
        source->palette_hash == 0u ||
        source->destination_rect.x < 0 || source->destination_rect.y < 0 ||
        source->destination_rect.w <= 0 || source->destination_rect.h <= 0 ||
        source->destination_rect.x + source->destination_rect.w >
            DM2_VP_WIDTH ||
        source->destination_rect.y + source->destination_rect.h >
            DM2_VP_HEIGHT) {
        s->dirty = 1;
        return;
    }
    expected_entry = ((int)source->possession_index << 1) +
        (int)source->left_or_right + 2;
    expected_rectno = (source->possession_index == 1u ? 0x46 : 0x4a) +
        (((int)source->player_position + 4 -
          (int)source->party_direction) & 3);
    if (source->gdat_category != DM2_GDAT_CATEGORY_INTERFACE_GENERAL ||
        source->gdat_subcategory != 4u ||
        source->gdat_entry != (uint8_t)expected_entry ||
        source->rectno != (uint8_t)expected_rectno) {
        s->dirty = 1;
        return;
    }
    accepted = *source;
    s->hud_hand_action_source = accepted;
    s->dirty = 1;
}

void dm2_v1_viewport_set_asset_provider(DM2_V1_ViewportState *s,
                                        DM2_V1_ViewportAssetFetch fetch,
                                        void *user)
{
    if (!s) return;
    s->asset_fetch = fetch;
    s->asset_user = user;
    s->dirty = 1;
}

void dm2_v1_viewport_set_gdat_door_overlay_material_plan(
    DM2_V1_ViewportState *s,
    const DM2_V1_GdatDoorOverlayM11CommandPlan *plan)
{
    if (!s) return;
    s->gdat_door_overlay_material_plan = plan;
    s->gdat_door_overlay_material_plan_consumed_count = 0;
}

void dm2_v1_viewport_set_asset_palette_provider(
    DM2_V1_ViewportState *s,
    DM2_V1_ViewportAssetPaletteFetch fetch,
    void *user)
{
    if (!s) return;
    s->asset_palette_fetch = fetch;
    s->asset_palette_user = user;
    s->active_asset_palette_ready = 0;
    s->active_asset_palette_hash = 0u;
    memset(s->active_asset_palette16, 0,
           sizeof(s->active_asset_palette16));
    s->dirty = 1;
}

void dm2_v1_viewport_set_door_surface_view_provider(
    DM2_V1_ViewportState *s,
    DM2_V1_ViewportDoorSurfaceViewFetch fetch,
    void *user)
{
    if (!s) return;
    s->door_surface_view_fetch = fetch;
    s->door_surface_view_user = user;
    s->dirty = 1;
}

void dm2_v1_viewport_set_asset_loader(
    DM2_V1_ViewportState *s,
    const DM2_V1_AssetLoader *loader)
{
    if (!s) return;
    s->asset_loader = loader;
    s->dirty = 1;
}

void dm2_v1_viewport_set_source_materials_required(
    DM2_V1_ViewportState *s, int required)
{
    if (!s) return;
    s->source_materials_required = required ? 1 : 0;
    if (s->source_materials_required) {
        const uint8_t light = s->gdat_c_light_receipt_ready
            ? s->gdat_c_light_level : 0u;
        for (int i = 0; i < DM2_SQ_COUNT; ++i) {
            s->squares[i].light_level = light;
        }
    }
    s->dirty = 1;
}

void dm2_v1_viewport_set_gdat_scene_control(
    DM2_V1_ViewportState *s,
    int ready,
    int graphicsset_index,
    uint32_t hash,
    uint16_t scene_colorkey,
    uint16_t scene_flags,
    uint16_t ambient_light,
    uint16_t highest_light_level,
    uint16_t void_random_fall,
    uint16_t animated_floor,
    uint16_t scene_rain,
    uint16_t misty_map,
    uint16_t thunder_position,
    uint16_t ambient_darkness)
{
    if (!s) return;
    s->gdat_scene_control_ready = ready ? 1 : 0;
    s->gdat_scene_material_index = ready && graphicsset_index >= 0 &&
        graphicsset_index <= 0xff ? graphicsset_index : 0;
    s->gdat_scene_control_hash = ready ? hash : 0u;
    s->gdat_scene_colorkey = ready ? scene_colorkey : 0u;
    s->gdat_scene_flags = ready ? scene_flags : 0u;
    s->gdat_ambient_light = ready ? ambient_light : 0u;
    s->gdat_highest_light_level = ready ? highest_light_level : 0u;
    s->gdat_void_random_fall = ready ? void_random_fall : 0u;
    s->gdat_animated_floor = ready ? animated_floor : 0u;
    s->gdat_scene_rain = ready ? scene_rain : 0u;
    s->gdat_misty_map = ready ? misty_map : 0u;
    s->gdat_thunder_position = ready ? thunder_position : 0u;
    s->gdat_ambient_darkness = ready ? ambient_darkness : 0u;
    /* The live light plan is derived from this same UPDATE_GFXSET
     * transaction (RECALC_LIGHT_LEVEL).  When the receipt goes missing the
     * plan must not linger from the previous scene owner. */
    if (!ready) {
        s->gdat_scene_light_floor = 0u;
        s->gdat_scene_light_search_depth = 0u;
        s->gdat_scene_light_recompute_enabled = 0;
    }
    /* UPDATE_GFXSET replaces the active scene as one transaction. A retained
     * plane plan from an earlier map/style cannot remain drawable after its
     * G1 control owner changes. */
    if (s->gdat_scene_material_plan &&
        (!s->gdat_scene_control_ready ||
         s->gdat_scene_material_plan->graphicsset !=
             (uint8_t)s->gdat_scene_material_index ||
         s->gdat_scene_material_plan->command_hash !=
             s->gdat_scene_control_hash)) {
        s->gdat_scene_material_plan = NULL;
    }
    s->gdat_scene_material_plan_rejected = 0;
    if (s->gdat_wall_material_plan &&
        (!s->gdat_scene_control_ready ||
         s->gdat_wall_material_plan->graphicsset !=
             (uint8_t)s->gdat_scene_material_index ||
         s->gdat_wall_material_plan_scene_control_hash !=
             s->gdat_scene_control_hash)) {
        s->gdat_wall_material_plan = NULL;
        s->gdat_wall_material_plan_scene_control_hash = 0u;
    }
    /* A c_light result has no meaning after UPDATE_GFXSET changes its owning
     * source transaction.  Do not retain it across a map/style handoff. */
    if (!s->gdat_scene_control_ready ||
        s->gdat_c_light_scene_control_hash != s->gdat_scene_control_hash) {
        s->gdat_c_light_receipt_ready = 0;
        s->gdat_c_light_level = 0u;
        s->gdat_c_light_scene_control_hash = 0u;
        s->gdat_c_light_source_state_hash = 0u;
        s->gdat_c_light_receipt_hash = 0u;
    }
    s->dirty = 1;
}

void dm2_v1_viewport_set_c_light_receipt(
    DM2_V1_ViewportState *s,
    const DM2_V1_CLightM11Receipt *receipt)
{
    int valid;

    if (!s) return;
    valid = receipt && receipt->valid && receipt->receipt_hash != 0u &&
        receipt->source_state_hash != 0u &&
        s->gdat_scene_control_ready &&
        receipt->graphicsset == (uint8_t)s->gdat_scene_material_index &&
        receipt->scene_control_hash == s->gdat_scene_control_hash;
    s->gdat_c_light_receipt_ready = valid ? 1 : 0;
    s->gdat_c_light_level = valid ? receipt->light_level : 0u;
    s->gdat_c_light_scene_control_hash = valid
        ? receipt->scene_control_hash : 0u;
    s->gdat_c_light_source_state_hash = valid
        ? receipt->source_state_hash : 0u;
    s->gdat_c_light_receipt_hash = valid ? receipt->receipt_hash : 0u;
    if (s->source_materials_required) {
        for (int i = 0; i < DM2_SQ_COUNT; ++i) {
            s->squares[i].light_level = s->gdat_c_light_level;
        }
    }
    s->dirty = 1;
}

void dm2_v1_viewport_set_gdat_scene_material_plan(
    DM2_V1_ViewportState *s,
    const DM2_V1_GdatSceneM11CommandPlan *plan)
{
    int attached;
    int sidecar_bound;

    if (!s) return;
    /* The material pair is only meaningful as the same source transaction
     * that produced the current G1 MapGraphicsStyle control receipt.  The
     * QUERY_BLIT_RECT and c_gui_vp draw-order sidecars are part of that
     * transaction, so reject stale or edited sidecars before M11 can retain
     * the plan pointer. */
    sidecar_bound = plan && plan->query_blit_rect.valid &&
        plan->query_blit_rect_hash != 0u &&
        plan->query_blit_rect_hash ==
            dm2_v1_gdat_scene_query_blit_rect_hash(&plan->query_blit_rect) &&
        dm2_v1_gdat_scene_m11_command_plan_draw_order_valid(plan);
    attached = plan && plan->valid &&
        s->gdat_scene_control_ready &&
        plan->graphicsset == (uint8_t)s->gdat_scene_material_index &&
        plan->command_hash == s->gdat_scene_control_hash &&
        sidecar_bound;
    s->gdat_scene_material_plan = attached ? plan : NULL;
    s->gdat_scene_material_plan_rejected = plan && !attached ? 1 : 0;
    s->dirty = 1;
}

void dm2_v1_viewport_set_gdat_scene_movement_active(
    DM2_V1_ViewportState *s, int active)
{
    if (!s) return;
    s->gdat_scene_movement_active = active ? 1 : 0;
    s->dirty = 1;
}

void dm2_v1_viewport_set_gdat_scene_map_origin(
    DM2_V1_ViewportState *s, int map_offset_x, int map_offset_y)
{
    if (!s) return;
    s->gdat_scene_map_offset_x = map_offset_x;
    s->gdat_scene_map_offset_y = map_offset_y;
    s->dirty = 1;
}

void dm2_v1_viewport_set_gdat_wall_material_plan(
    DM2_V1_ViewportState *s, const DM2_V1_GdatWallM11CommandPlan *plan)
{
    if (!s) return;
    /* DM2_DRAW_WALL consumes the GRAPHICSSET chosen by UPDATE_GFXSET.  A
     * standalone plan has no live G1 owner and must not become a substitute
     * for the active map's source transaction. */
    s->gdat_wall_material_plan = plan && plan->valid &&
        plan->command_count > 0 && plan->command_hash != 0u &&
        s->gdat_scene_control_ready &&
        plan->graphicsset == (uint8_t)s->gdat_scene_material_index ? plan : NULL;
    s->gdat_wall_material_plan_scene_control_hash =
        s->gdat_wall_material_plan ? s->gdat_scene_control_hash : 0u;
    s->gdat_wall_material_plan_consumed_count = 0;
    s->dirty = 1;
}

void dm2_v1_viewport_set_gdat_wall_ornament_material_plan(
    DM2_V1_ViewportState *s,
    const DM2_V1_WallOrnamentRenderPlan *plan)
{
    if (!s) return;
    /* DM2 DRAW_WALL_ORNATE placement is owned by the runtime/loader; the
     * renderer must not invent a destination rectangle.  Accept only a
     * validated plan and clear it on the next source state change. */
    s->gdat_wall_ornament_material_plan =
        plan && plan->valid && plan->ornament_count > 0 ? plan : NULL;
    s->dirty = 1;
}

void dm2_v1_viewport_set_gdat_hud_material_plan(
    DM2_V1_ViewportState *s,
    const DM2_V1_GdatHudM11CommandPlan *plan)
{
    if (!s) return;
    s->gdat_hud_material_plan = plan;
    s->gdat_hud_material_plan_consumed_count = 0;
    s->dirty = 1;
}

void dm2_v1_viewport_set_gdat_dialogue_box_host_command(
    DM2_V1_ViewportState *s,
    const DM2_V1_DialogueBoxHostCommand *command,
    int active)
{
    if (!s) return;
    memset(&s->gdat_dialogue_box_command, 0,
           sizeof(s->gdat_dialogue_box_command));
    s->gdat_dialogue_box_active = 0;
    s->gdat_dialogue_box_consumed_count = 0;
    s->gdat_dialogue_box_consumed_hash = 0u;
    if (!active || !command || !command->valid || !command->draw.valid ||
        command->draw.gdat_category != DM2_GDAT_CATEGORY_DIALOG_BOXES ||
        command->draw.gdat_index != DM2_V1_DIALOGUE_BOX_INDEX ||
        command->draw.gdat_field != DM2_V1_DIALOGUE_BOX_FIELD ||
        command->draw.expanded_rect_index != DM2_V1_DIALOGUE_BOX_RECT_INDEX ||
        command->draw.plan_hash == 0u || command->command_hash == 0u ||
        command->rect.w <= 0 || command->rect.h <= 0) return;
    s->gdat_dialogue_box_command = *command;
    s->gdat_dialogue_box_active = 1;
    s->dirty = 1;
}

void dm2_v1_viewport_set_gdat_dialogue_open_panel_host_command(
    DM2_V1_ViewportState *s,
    const DM2_V1_DialogueOpenPanelHostCommand *command,
    int active)
{
    if (!s) return;
    memset(&s->gdat_dialogue_open_panel_command, 0,
           sizeof(s->gdat_dialogue_open_panel_command));
    s->gdat_dialogue_open_panel_active = 0;
    s->gdat_dialogue_open_panel_consumed_count = 0;
    s->gdat_dialogue_open_panel_consumed_hash = 0u;
    if (!active || !command || !command->valid || !command->draw.valid ||
        !command->draw.material.valid ||
        command->draw.material.metadata.bits_per_pixel != 4u ||
        command->draw.material.palette_hash == 0u ||
        command->draw.version_text_size !=
            DM2_V1_DIALOGUE_OPEN_PANEL_VERSION_TEXT_SIZE ||
        command->draw.version_text_hash == 0u ||
        memcmp(command->draw.version_text,
               DM2_V1_DIALOGUE_OPEN_PANEL_VERSION_TEXT,
               DM2_V1_DIALOGUE_OPEN_PANEL_VERSION_TEXT_SIZE) != 0 ||
        !command->draw.text[0] || !command->draw.text[1] ||
        command->draw.text_size[0] == 0u || command->draw.text_size[1] == 0u ||
        command->draw.text_hash[0] == 0u || command->draw.text_hash[1] == 0u ||
        command->draw.panel_rect_index != DM2_V1_DIALOGUE_OPEN_PANEL_RECT_INDEX ||
        command->panel_rect.w <= 0 || command->panel_rect.h <= 0 ||
        command->version_text_rect.w <= 0 || command->version_text_rect.h <= 0 ||
        command->primary_text_rect.w <= 0 || command->primary_text_rect.h <= 0 ||
        command->secondary_text_rect.w <= 0 || command->secondary_text_rect.h <= 0 ||
        command->command_hash == 0u) return;
    s->gdat_dialogue_open_panel_command = *command;
    for (unsigned int i = 0u; i < DM2_V1_DIALOGUE_OPEN_PANEL_TEXT_COUNT; ++i) {
        s->gdat_dialogue_open_panel_command.draw.text[i] =
            s->gdat_dialogue_open_panel_command.draw.decoded_text[i];
    }
    s->gdat_dialogue_open_panel_active = 1;
    s->dirty = 1;
}

void dm2_v1_viewport_set_scene_map_load_token(
    DM2_V1_ViewportState *s, uint32_t source_map_load_token)
{
    if (!s) return;
    s->gdat_scene_map_load_token = source_map_load_token;
    s->dirty = 1;
}

static int dm2_v1_viewport_scene_bind_matches(
    const DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash)
{
    return s && s->gdat_scene_control_ready &&
        source_map_load_token != 0u &&
        source_scene_control_hash != 0u &&
        s->gdat_scene_map_load_token == source_map_load_token &&
        s->gdat_scene_control_hash == source_scene_control_hash;
}

int dm2_v1_viewport_bind_static_graphicsset_scene_record(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash)
{
    DM2_V1_GraphicsSetStaticSceneReceipt *scene;
    if (!dm2_v1_viewport_scene_bind_matches(
            s, source_map_load_token, source_scene_control_hash)) {
        return 0;
    }
    scene = &s->gdat_static_scene_record;
    memset(scene, 0, sizeof(*scene));
    scene->valid = 1;
    scene->map_load_token = source_map_load_token;
    scene->scene_control_hash = source_scene_control_hash;
    scene->graphicsset = (uint8_t)s->gdat_scene_material_index;
    scene->scene_colorkey = s->gdat_scene_colorkey;
    scene->scene_flags = s->gdat_scene_flags;
    scene->ambient_light = s->gdat_ambient_light;
    scene->highest_light_level = s->gdat_highest_light_level;
    scene->ambient_darkness = s->gdat_ambient_darkness;
    scene->material_category = DM2_GDAT_CATEGORY_GRAPHICSSET;
    scene->floor_field = DM2_GDAT_GFXSET_FLOOR;
    scene->ceiling_field = DM2_GDAT_GFXSET_CEIL;
    scene->door_frame_front_d1_field = DM2_GDAT_GFXSET_DOOR_FRAME_FRONT_D1;
    scene->door_frame_d1c_field = DM2_GDAT_GFXSET_DOOR_FRAME_D1C;
    scene->door_frame_d2c_field = DM2_GDAT_GFXSET_DOOR_FRAME_D2C;
    s->dirty = 1;
    return 1;
}

static int dm2_v1_viewport_bind_static_scene_flag(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash,
    uint32_t *token_slot,
    uint32_t *hash_slot,
    int *owned_slot)
{
    if (!token_slot || !hash_slot || !owned_slot ||
        !dm2_v1_viewport_scene_bind_matches(
            s, source_map_load_token, source_scene_control_hash)) {
        return 0;
    }
    *token_slot = source_map_load_token;
    *hash_slot = source_scene_control_hash;
    *owned_slot = 1;
    s->dirty = 1;
    return 1;
}

int dm2_v1_viewport_bind_static_scene_light_control(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash)
{
    return dm2_v1_viewport_bind_static_scene_flag(
        s, source_map_load_token, source_scene_control_hash,
        &s->gdat_static_light_map_load_token,
        &s->gdat_static_light_scene_control_hash,
        &s->gdat_static_light_control_owned);
}

int dm2_v1_viewport_bind_static_scene_ambient_light_control(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash)
{
    return dm2_v1_viewport_bind_static_scene_flag(
        s, source_map_load_token, source_scene_control_hash,
        &s->gdat_static_ambient_light_map_load_token,
        &s->gdat_static_ambient_light_scene_control_hash,
        &s->gdat_static_ambient_light_control_owned);
}

int dm2_v1_viewport_bind_static_scene_ambient_darkness_control(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash)
{
    return dm2_v1_viewport_bind_static_scene_flag(
        s, source_map_load_token, source_scene_control_hash,
        &s->gdat_static_ambient_darkness_map_load_token,
        &s->gdat_static_ambient_darkness_scene_control_hash,
        &s->gdat_static_ambient_darkness_control_owned);
}

int dm2_v1_viewport_bind_static_scene_flags_control(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash)
{
    return dm2_v1_viewport_bind_static_scene_flag(
        s, source_map_load_token, source_scene_control_hash,
        &s->gdat_static_scene_flags_map_load_token,
        &s->gdat_static_scene_flags_scene_control_hash,
        &s->gdat_static_scene_flags_control_owned);
}

int dm2_v1_viewport_bind_static_scene_colorkey_control(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash)
{
    return dm2_v1_viewport_bind_static_scene_flag(
        s, source_map_load_token, source_scene_control_hash,
        &s->gdat_static_scene_colorkey_map_load_token,
        &s->gdat_static_scene_colorkey_scene_control_hash,
        &s->gdat_static_scene_colorkey_control_owned);
}

int dm2_v1_viewport_bind_static_scene_floor_material(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash)
{
    return dm2_v1_viewport_bind_static_scene_flag(
        s, source_map_load_token, source_scene_control_hash,
        &s->gdat_static_scene_floor_material_map_load_token,
        &s->gdat_static_scene_floor_material_scene_control_hash,
        &s->gdat_static_scene_floor_material_owned);
}

int dm2_v1_viewport_bind_static_scene_ceiling_material(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash)
{
    return dm2_v1_viewport_bind_static_scene_flag(
        s, source_map_load_token, source_scene_control_hash,
        &s->gdat_static_scene_ceiling_material_map_load_token,
        &s->gdat_static_scene_ceiling_material_scene_control_hash,
        &s->gdat_static_scene_ceiling_material_owned);
}

int dm2_v1_viewport_bind_static_scene_wall_material(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash,
    int view_square)
{
    const int field = dm2_v1_viewport_wall_field_for_square(view_square);

    if (field < 0 ||
        !dm2_v1_viewport_bind_static_scene_flag(
            s, source_map_load_token, source_scene_control_hash,
            &s->gdat_static_scene_wall_material_map_load_token,
            &s->gdat_static_scene_wall_material_scene_control_hash,
            &s->gdat_static_scene_wall_material_owned)) {
        return 0;
    }
    s->gdat_static_scene_wall_material_mask |=
        (uint16_t)(1u << (unsigned)view_square);
    s->gdat_static_scene_wall_material_view_square = (uint8_t)view_square;
    s->gdat_static_scene_wall_material_field = (uint8_t)field;
    return 1;
}

int dm2_v1_viewport_bind_static_scene_all_wall_materials(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash)
{
    uint16_t mask = 0u;

    if (!dm2_v1_viewport_scene_bind_matches(
            s, source_map_load_token, source_scene_control_hash)) {
        return 0;
    }
    for (int view_square = 0; view_square < DM2_SQ_COUNT; ++view_square) {
        if (dm2_v1_viewport_wall_field_for_square(view_square) >= 0) {
            mask |= (uint16_t)(1u << (unsigned)view_square);
        }
    }
    if (!mask ||
        !dm2_v1_viewport_bind_static_scene_flag(
            s, source_map_load_token, source_scene_control_hash,
            &s->gdat_static_scene_wall_material_map_load_token,
            &s->gdat_static_scene_wall_material_scene_control_hash,
            &s->gdat_static_scene_wall_material_owned)) {
        return 0;
    }
    s->gdat_static_scene_wall_material_mask = mask;
    s->gdat_static_scene_wall_material_view_square = 0u;
    s->gdat_static_scene_wall_material_field = 0u;
    return 1;
}

int dm2_v1_viewport_bind_static_scene_door_frame_material(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash)
{
    return dm2_v1_viewport_bind_static_scene_flag(
        s, source_map_load_token, source_scene_control_hash,
        &s->gdat_static_scene_door_frame_material_map_load_token,
        &s->gdat_static_scene_door_frame_material_scene_control_hash,
        &s->gdat_static_scene_door_frame_material_owned);
}

int dm2_v1_viewport_bind_static_scene_door_frame_d1c_material(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash)
{
    return dm2_v1_viewport_bind_static_scene_flag(
        s, source_map_load_token, source_scene_control_hash,
        &s->gdat_static_scene_door_frame_d1c_material_map_load_token,
        &s->gdat_static_scene_door_frame_d1c_material_scene_control_hash,
        &s->gdat_static_scene_door_frame_d1c_material_owned);
}

int dm2_v1_viewport_bind_static_scene_door_frame_d2c_material(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash)
{
    return dm2_v1_viewport_bind_static_scene_flag(
        s, source_map_load_token, source_scene_control_hash,
        &s->gdat_static_scene_door_frame_d2c_material_map_load_token,
        &s->gdat_static_scene_door_frame_d2c_material_scene_control_hash,
        &s->gdat_static_scene_door_frame_d2c_material_owned);
}

int dm2_v1_viewport_set_floor_gfx_viewport_ownership(
    DM2_V1_ViewportState *s,
    const DM2_V1_FloorGfxViewportOwnershipReceipt *ownership)
{
    if (!s || !ownership || !ownership->valid || !ownership->viewport_owned ||
        ownership->map_load_token != s->gdat_scene_map_load_token ||
        ownership->gdat_category != DM2_GDAT_CATEGORY_FLOOR_GFX) {
        return 0;
    }
    s->floor_gfx_viewport_ownership = *ownership;
    s->dirty = 1;
    return 1;
}

int dm2_v1_viewport_floor_gfx_render_plan_receipt(
    const DM2_V1_ViewportState *s,
    DM2_V1_ViewportFloorGfxRenderPlanReceipt *out_receipt)
{
    const DM2_V1_GraphicsSetStaticSceneReceipt *scene;
    const DM2_V1_FloorGfxViewportOwnershipReceipt *floor;
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!s || !s->gdat_scene_control_ready) return 0;
    scene = &s->gdat_static_scene_record;
    floor = &s->floor_gfx_viewport_ownership;
    if (!scene->valid ||
        scene->map_load_token != s->gdat_scene_map_load_token ||
        scene->scene_control_hash != s->gdat_scene_control_hash ||
        scene->graphicsset != (uint8_t)s->gdat_scene_material_index ||
        !floor->valid || !floor->viewport_owned ||
        floor->map_load_token != s->gdat_scene_map_load_token ||
        floor->gdat_category != DM2_GDAT_CATEGORY_FLOOR_GFX) {
        return 0;
    }
    out_receipt->valid = 1;
    out_receipt->static_scene_control_owned = 1;
    out_receipt->static_light_control_owned = 1;
    out_receipt->static_ambient_light_control_owned = 1;
    out_receipt->static_ambient_darkness_control_owned = 1;
    out_receipt->static_scene_flags_control_owned = 1;
    out_receipt->static_scene_colorkey_control_owned = 1;
    out_receipt->static_scene_floor_material_owned = 1;
    out_receipt->static_scene_ceiling_material_owned = 1;
    out_receipt->static_scene_door_frame_material_owned = 1;
    out_receipt->static_scene_door_frame_d1c_material_owned = 1;
    out_receipt->static_scene_door_frame_d2c_material_owned = 1;
    out_receipt->map_load_token = s->gdat_scene_map_load_token;
    out_receipt->gdat_category = floor->gdat_category;
    out_receipt->floor_ornate_source_index = floor->floor_ornate_source_index;
    out_receipt->animated_frame_route = floor->animated_frame_route;
    out_receipt->scene_control_hash = s->gdat_scene_control_hash;
    out_receipt->scene_colorkey = s->gdat_scene_colorkey;
    out_receipt->scene_flags = s->gdat_scene_flags;
    out_receipt->outdoor_scene =
        (s->gdat_scene_flags & DM2_V1_GDAT_SCENE_FLAG_OUTDOOR) != 0u;
    out_receipt->scene_floor_material_category = scene->material_category;
    out_receipt->scene_floor_material_graphicsset = scene->graphicsset;
    out_receipt->scene_floor_material_field = scene->floor_field;
    out_receipt->scene_ceiling_material_category = scene->material_category;
    out_receipt->scene_ceiling_material_graphicsset = scene->graphicsset;
    out_receipt->scene_ceiling_material_field = scene->ceiling_field;
    out_receipt->scene_door_frame_material_category = scene->material_category;
    out_receipt->scene_door_frame_material_graphicsset = scene->graphicsset;
    out_receipt->scene_door_frame_material_field =
        scene->door_frame_front_d1_field;
    out_receipt->scene_door_frame_d1c_material_category =
        scene->material_category;
    out_receipt->scene_door_frame_d1c_material_graphicsset =
        scene->graphicsset;
    out_receipt->scene_door_frame_d1c_material_field =
        scene->door_frame_d1c_field;
    out_receipt->scene_door_frame_d2c_material_category =
        scene->material_category;
    out_receipt->scene_door_frame_d2c_material_graphicsset =
        scene->graphicsset;
    out_receipt->scene_door_frame_d2c_material_field =
        scene->door_frame_d2c_field;
    out_receipt->ambient_light = s->gdat_ambient_light;
    out_receipt->highest_light_level = s->gdat_highest_light_level;
    out_receipt->ambient_darkness = s->gdat_ambient_darkness;
    return 1;
}

void dm2_v1_viewport_scene_light_control(uint16_t highest_light_level,
                                         uint16_t ambient_darkness,
                                         uint8_t *out_light_floor,
                                         uint8_t *out_search_depth,
                                         int *out_recompute_enabled)
{
    uint16_t light_floor = highest_light_level;
    uint16_t search_depth = ambient_darkness;
    if (light_floor > 5u) light_floor = 5u;
    if (search_depth > 8u) search_depth = 8u;
    if (out_light_floor) *out_light_floor = (uint8_t)light_floor;
    if (out_search_depth) *out_search_depth = (uint8_t)search_depth;
    if (out_recompute_enabled) {
        *out_recompute_enabled = (light_floor != 0u || search_depth != 0u) ? 1 : 0;
    }
}

void dm2_v1_viewport_set_gdat_weather_renderer_receipt(
    DM2_V1_ViewportState *s,
    uint8_t graphicsset_index,
    const DM2_V1_WeatherRendererReceipt *receipt)
{
    if (!s) return;
    s->gdat_weather_renderer_receipt = receipt && receipt->valid &&
        receipt->renderer_hash != 0u && receipt->command_count <= 2u
        ? receipt : NULL;
    s->gdat_weather_renderer_graphicsset =
        s->gdat_weather_renderer_receipt ? graphicsset_index : 0u;
    s->gdat_weather_renderer_consumed_hash = 0u;
    s->gdat_weather_renderer_consumed_command_count = 0u;
    s->asset_weather_drawn_count = 0;
    s->dirty = 1;
}

void dm2_v1_viewport_set_gdat_interface_palette(
    DM2_V1_ViewportState *s,
    int ready,
    uint32_t hash,
    const uint8_t palette16[16])
{
    if (!s) return;
    s->gdat_interface_palette_ready = ready && hash != 0u && palette16;
    s->gdat_interface_palette_hash =
        s->gdat_interface_palette_ready ? hash : 0u;
    if (s->gdat_interface_palette_ready) {
        memcpy(s->gdat_interface_palette16, palette16,
               sizeof(s->gdat_interface_palette16));
    } else {
        memset(s->gdat_interface_palette16, 0,
               sizeof(s->gdat_interface_palette16));
    }
    s->dirty = 1;
}

void dm2_v1_viewport_set_gdat_interface_text_palette(
    DM2_V1_ViewportState *s,
    int ready,
    uint32_t hash,
    const uint8_t palette16[16])
{
    if (!s) return;
    s->gdat_interface_text_palette_ready = ready && hash != 0u && palette16;
    s->gdat_interface_text_palette_hash =
        s->gdat_interface_text_palette_ready ? hash : 0u;
    if (s->gdat_interface_text_palette_ready) {
        memcpy(s->gdat_interface_text_palette16, palette16,
               sizeof(s->gdat_interface_text_palette16));
    } else {
        memset(s->gdat_interface_text_palette16, 0,
               sizeof(s->gdat_interface_text_palette16));
    }
    s->dirty = 1;
}

void dm2_v1_viewport_set_gdat_interface_font(
    DM2_V1_ViewportState *s,
    const uint8_t *rows,
    uint32_t hash)
{
    if (!s) return;
    s->gdat_interface_font_rows = rows;
    s->gdat_interface_font_hash = rows && hash != 0u ? hash : 0u;
    s->dirty = 1;
}

void dm2_v1_viewport_set_g1_creature_map_chip_materials(
    DM2_V1_ViewportState *s,
    const DM2_V1_G1CreatureMapChipRuntimeReceipt *receipt)
{
    if (!s) return;
    s->g1_creature_map_chip_materials =
        receipt && receipt->valid ? receipt : NULL;
    s->dirty = 1;
}

void dm2_v1_viewport_set_g1_creature_v5_materials(
    DM2_V1_ViewportState *s,
    const DM2_V1_G1CreatureV5RuntimeReceipt *receipt)
{
    if (!s) return;
    s->g1_creature_v5_materials =
        receipt && receipt->valid ? receipt : NULL;
    s->dirty = 1;
}

int dm2_v1_g1_creature_v5_material_matches(
    const DM2_V1_G1CreatureV5RuntimeReceipt *receipt,
    uint16_t object_id, int map_x, int map_y, int creature_type,
    int image_field, int width, int height,
    uint32_t palette_hash, uint32_t decoded_hash)
{
    if (!receipt || !receipt->valid || object_id == 0xfffeu ||
        creature_type < 0 || creature_type > 0xff || image_field < 0 ||
        image_field > 0xff || width <= 0 || height <= 0 ||
        palette_hash == 0u || decoded_hash == 0u) {
        return 0;
    }
    for (int i = 0; i < receipt->count && i < DM2_V1_G1_CREATURE_V5_MAX;
         ++i) {
        const DM2_V1_G1CreatureV5Material *material =
            &receipt->materials[i];
        if (material->object_id == object_id &&
            material->map_x == map_x && material->map_y == map_y &&
            material->creature_type == (uint8_t)creature_type &&
            material->image_field == (uint8_t)image_field &&
            material->width == width && material->height == height &&
            material->palette_hash == palette_hash &&
            material->decoded_hash == decoded_hash) {
            return 1;
        }
    }
    return 0;
}

void dm2_v1_viewport_set_g1_weapon_map_chip_materials(
    DM2_V1_ViewportState *s,
    const DM2_V1_G1WeaponMapChipRuntimeReceipt *receipt)
{
    if (!s) return;
    s->g1_weapon_map_chip_materials = receipt && receipt->valid ? receipt : NULL;
    s->dirty = 1;
}

void dm2_v1_viewport_set_g1_container_map_chip_materials(
    DM2_V1_ViewportState *s,
    const DM2_V1_G1ContainerMapChipRuntimeReceipt *receipt)
{
    if (!s) return;
    s->g1_container_map_chip_materials =
        receipt && receipt->valid ? receipt : NULL;
    s->dirty = 1;
}

void dm2_v1_viewport_set_g1_scene_creature_material(
    DM2_V1_ViewportState *s, int ready, int map_x, int map_y,
    int creature_type, int gdat_index, int width, int height, int stride,
    uint32_t palette_hash)
{
    if (!s) return;
    s->g1_scene_creature_material_ready =
        ready && creature_type >= 0 && creature_type <= 0xff &&
        gdat_index != 0 && width > 0 && height > 0 && stride >= width &&
        palette_hash != 0u;
    s->g1_scene_creature_material_map_x = map_x;
    s->g1_scene_creature_material_map_y = map_y;
    s->g1_scene_creature_material_type = creature_type;
    s->g1_scene_creature_material_gdat_index = gdat_index;
    s->g1_scene_creature_material_width = width;
    s->g1_scene_creature_material_height = height;
    s->g1_scene_creature_material_stride = stride;
    s->g1_scene_creature_material_pixels = NULL;
    s->g1_scene_creature_material_pixel_hash = 0u;
    memset(s->g1_scene_creature_material_palette16, 0,
           sizeof(s->g1_scene_creature_material_palette16));
    s->g1_scene_creature_material_palette_hash = palette_hash;
    s->g1_scene_creature_material_consumed_count = 0;
    s->dirty = 1;
}

void dm2_v1_viewport_set_g1_scene_creature_material_direct(
    DM2_V1_ViewportState *s, int ready, int map_x, int map_y,
    int creature_type, int gdat_index, const uint8_t *pixels,
    int width, int height, int stride, const uint8_t palette16[16],
    uint32_t palette_hash, uint32_t expected_pixel_hash)
{
    dm2_v1_viewport_set_g1_scene_creature_material(
        s, ready, map_x, map_y, creature_type, gdat_index, width, height,
        stride, palette_hash);
    if (!s) return;
    if (!s->g1_scene_creature_material_ready || !pixels || !palette16) {
        s->g1_scene_creature_material_ready = 0;
        return;
    }
    s->g1_scene_creature_material_pixels = pixels;
    s->g1_scene_creature_material_pixel_hash =
        dm2_v1_viewport_indexed_pixel_hash(pixels, width, height, stride);
    if (s->g1_scene_creature_material_pixel_hash == 0u ||
        (expected_pixel_hash != 0u &&
         s->g1_scene_creature_material_pixel_hash != expected_pixel_hash)) {
        s->g1_scene_creature_material_ready = 0;
        return;
    }
    memcpy(s->g1_scene_creature_material_palette16, palette16,
           sizeof(s->g1_scene_creature_material_palette16));
}

void dm2_v1_viewport_set_g1_scene_item_material_direct(
    DM2_V1_ViewportState *s, int ready, int item_category, int item_type,
    int gdat_index, uint16_t object_id, int map_x, int map_y,
    const uint8_t *pixels, int width, int height, int stride,
    const uint8_t palette16[16], uint32_t palette_hash,
    uint32_t expected_pixel_hash)
{
    if (!s) return;
    s->g1_scene_item_material_ready =
        ready && (item_category == 0x10 || item_category == 0x14) &&
        item_type >= 0 && item_type <= 0xff && gdat_index != 0 &&
        object_id != 0xfffeu && pixels && width > 0 && height > 0 &&
        stride >= width && palette16 && palette_hash != 0u &&
        expected_pixel_hash != 0u;
    s->g1_scene_item_material_category = item_category;
    s->g1_scene_item_material_type = item_type;
    s->g1_scene_item_material_gdat_index = gdat_index;
    s->g1_scene_item_material_object_id = object_id;
    s->g1_scene_item_material_map_x = map_x;
    s->g1_scene_item_material_map_y = map_y;
    s->g1_scene_item_material_width = width;
    s->g1_scene_item_material_height = height;
    s->g1_scene_item_material_stride = stride;
    s->g1_scene_item_material_pixels = NULL;
    s->g1_scene_item_material_pixel_hash = 0u;
    memset(s->g1_scene_item_material_palette16, 0,
           sizeof(s->g1_scene_item_material_palette16));
    s->g1_scene_item_material_palette_hash = 0u;
    s->g1_scene_item_material_raw_gfx256_hash = 0u;
    s->g1_scene_item_material_raw_gfx256_receipt_hash = 0u;
    s->g1_scene_item_material_raw4_hash = 0u;
    s->g1_scene_item_material_raw4_receipt_hash = 0u;
    s->g1_scene_item_material_consumed_count = 0;
    if (!s->g1_scene_item_material_ready) return;
    s->g1_scene_item_material_pixel_hash =
        dm2_v1_viewport_indexed_pixel_hash(pixels, width, height, stride);
    if (s->g1_scene_item_material_pixel_hash != expected_pixel_hash) {
        s->g1_scene_item_material_ready = 0;
        return;
    }
    s->g1_scene_item_material_pixels = pixels;
    memcpy(s->g1_scene_item_material_palette16, palette16,
           sizeof(s->g1_scene_item_material_palette16));
    s->g1_scene_item_material_palette_hash = palette_hash;
    s->dirty = 1;
}

void dm2_v1_viewport_set_g1_scene_static_item_material_direct(
    DM2_V1_ViewportState *s, int ready, int item_category, int item_type,
    int gdat_index, uint16_t object_id, int map_x, int map_y,
    const uint8_t *pixels, int width, int height, int stride,
    const uint8_t palette16[16], uint32_t palette_hash,
    uint32_t expected_pixel_hash, uint32_t raw_gfx256_hash,
    uint32_t raw_gfx256_receipt_hash, uint32_t raw4_hash,
    uint32_t raw4_receipt_hash)
{
    dm2_v1_viewport_set_g1_scene_item_material_direct(s, ready, item_category,
        item_type, gdat_index, object_id, map_x, map_y, pixels, width, height,
        stride, palette16, palette_hash, expected_pixel_hash);
    if (!s || !s->g1_scene_item_material_ready || !raw_gfx256_hash ||
        !raw_gfx256_receipt_hash || !raw4_hash || !raw4_receipt_hash) {
        if (s) s->g1_scene_item_material_ready = 0;
        return;
    }
    s->g1_scene_item_material_raw_gfx256_hash = raw_gfx256_hash;
    s->g1_scene_item_material_raw_gfx256_receipt_hash = raw_gfx256_receipt_hash;
    s->g1_scene_item_material_raw4_hash = raw4_hash;
    s->g1_scene_item_material_raw4_receipt_hash = raw4_receipt_hash;
}

void dm2_v1_viewport_set_g1_scene_static_item_materials_direct(
    DM2_V1_ViewportState *s,
    const DM2_V1_G1SceneStaticItemMaterial *materials,
    int material_count)
{
    int i;

    if (!s) return;
    memset(s->g1_scene_static_item_materials, 0,
           sizeof(s->g1_scene_static_item_materials));
    s->g1_scene_static_item_material_count = 0;
    s->g1_scene_static_item_material_consumed_count = 0;
    if (!materials || material_count <= 0 ||
        material_count > DM2_V1_G1_SCENE_STATIC_ITEM_MATERIAL_MAX) {
        return;
    }
    for (i = 0; i < material_count; ++i) {
        const DM2_V1_G1SceneStaticItemMaterial *source = &materials[i];
        DM2_V1_G1SceneStaticItemMaterial *target =
            &s->g1_scene_static_item_materials[i];
        uint32_t pixel_hash;

        if (!source->ready ||
            (source->item_category != 0x10 && source->item_category != 0x14) ||
            source->item_type < 0 || source->item_type > 0xff ||
            source->gdat_index == 0 || source->object_id == 0xfffeu ||
            !source->pixels || source->width <= 0 || source->height <= 0 ||
            source->stride < source->width || source->palette_hash == 0u ||
            source->pixel_hash == 0u || source->raw_gfx256_hash == 0u ||
            source->raw_gfx256_receipt_hash == 0u || source->raw4_hash == 0u ||
            source->raw4_receipt_hash == 0u) {
            memset(s->g1_scene_static_item_materials, 0,
                   sizeof(s->g1_scene_static_item_materials));
            return;
        }
        pixel_hash = dm2_v1_viewport_indexed_pixel_hash(
            source->pixels, source->width, source->height, source->stride);
        if (pixel_hash == 0u || pixel_hash != source->pixel_hash) {
            memset(s->g1_scene_static_item_materials, 0,
                   sizeof(s->g1_scene_static_item_materials));
            return;
        }
        *target = *source;
    }
    s->g1_scene_static_item_material_count = material_count;
    s->dirty = 1;
}

static const DM2_V1_G1SceneStaticItemMaterial *
dm2_v1_viewport_find_g1_scene_static_item_material(
    const DM2_V1_ViewportState *s, const DM2_V1_ItemRender *item)
{
    int i;

    if (!s || !item || !item->source_static_object_admitted ||
        item->source_gdat_field == 0xf9u) return NULL;
    for (i = 0; i < s->g1_scene_static_item_material_count; ++i) {
        const DM2_V1_G1SceneStaticItemMaterial *material =
            &s->g1_scene_static_item_materials[i];
        if (material->ready &&
            material->item_category == item->item_category &&
            material->item_type == item->item_type &&
            material->gdat_index == item->gdat_index &&
            material->object_id == item->object_id &&
            material->map_x == item->map_x && material->map_y == item->map_y &&
            material->raw_gfx256_hash == item->source_static_object_raw_gfx256_hash &&
            material->raw_gfx256_receipt_hash ==
                item->source_static_object_raw_gfx256_receipt_hash &&
            material->raw4_hash == item->source_static_object_raw4_hash &&
            material->raw4_receipt_hash ==
                item->source_static_object_raw4_receipt_hash) {
            return material;
        }
    }
    return NULL;
}

void dm2_v1_viewport_set_g1_scene_wall_button_material_direct(
    DM2_V1_ViewportState *s, int ready, int gdat_index,
    int wall_gfx_index, int field, int map_x, int map_y,
    uint16_t object_id, const uint8_t *pixels, int width, int height,
    int stride, const uint8_t palette16[16], uint32_t palette_hash,
    uint32_t expected_pixel_hash, uint16_t raw_index,
    const uint8_t *raw_bytes, size_t raw_byte_count, uint32_t raw_hash,
    uint32_t raw_receipt_hash)
{
    if (!s) return;
    s->g1_scene_wall_button_material_ready =
        ready && gdat_index != 0 && wall_gfx_index >= 0 &&
        wall_gfx_index <= 0xff && field == 1 && object_id != 0xfffeu &&
        pixels && width > 0 && height > 0 && stride >= width && palette16 &&
        palette_hash != 0u && expected_pixel_hash != 0u && raw_bytes &&
        raw_byte_count != 0u && raw_byte_count <= (size_t)INT_MAX &&
        raw_hash != 0u && raw_receipt_hash != 0u &&
        dm2_v1_viewport_indexed_pixel_hash(raw_bytes, (int)raw_byte_count,
                                           1, (int)raw_byte_count) == raw_hash;
    s->g1_scene_wall_button_material_gdat_index = gdat_index;
    s->g1_scene_wall_button_material_wall_gfx_index = wall_gfx_index;
    s->g1_scene_wall_button_material_field = field;
    s->g1_scene_wall_button_material_map_x = map_x;
    s->g1_scene_wall_button_material_map_y = map_y;
    s->g1_scene_wall_button_material_object_id = object_id;
    s->g1_scene_wall_button_material_width = width;
    s->g1_scene_wall_button_material_height = height;
    s->g1_scene_wall_button_material_stride = stride;
    s->g1_scene_wall_button_material_pixels = NULL;
    s->g1_scene_wall_button_material_pixel_hash = 0u;
    memset(s->g1_scene_wall_button_material_palette16, 0,
           sizeof(s->g1_scene_wall_button_material_palette16));
    s->g1_scene_wall_button_material_palette_hash = 0u;
    s->g1_scene_wall_button_material_raw_index = 0u;
    s->g1_scene_wall_button_material_raw_bytes = NULL;
    s->g1_scene_wall_button_material_raw_byte_count = 0u;
    s->g1_scene_wall_button_material_raw_hash = 0u;
    s->g1_scene_wall_button_material_receipt_hash = 0u;
    s->g1_scene_wall_button_material_consumed_count = 0;
    if (!s->g1_scene_wall_button_material_ready) return;
    s->g1_scene_wall_button_material_pixel_hash =
        dm2_v1_viewport_indexed_pixel_hash(pixels, width, height, stride);
    if (s->g1_scene_wall_button_material_pixel_hash != expected_pixel_hash) {
        s->g1_scene_wall_button_material_ready = 0;
        return;
    }
    s->g1_scene_wall_button_material_pixels = pixels;
    memcpy(s->g1_scene_wall_button_material_palette16, palette16,
           sizeof(s->g1_scene_wall_button_material_palette16));
    s->g1_scene_wall_button_material_palette_hash = palette_hash;
    s->g1_scene_wall_button_material_raw_index = raw_index;
    s->g1_scene_wall_button_material_raw_bytes = raw_bytes;
    s->g1_scene_wall_button_material_raw_byte_count = raw_byte_count;
    s->g1_scene_wall_button_material_raw_hash = raw_hash;
    s->g1_scene_wall_button_material_receipt_hash = raw_receipt_hash;
    s->dirty = 1;
}

void dm2_v1_viewport_set_g1_wall_gfx_materials(
    DM2_V1_ViewportState *s,
    const DM2_V1_G1TextWallGfxRuntimeReceipt *text_receipt,
    const DM2_V1_G1ActuatorWallGfxRuntimeReceipt *actuator_receipt)
{
    if (!s) return;
    s->g1_text_wall_gfx_materials =
        text_receipt && text_receipt->valid ? text_receipt : NULL;
    s->g1_actuator_wall_gfx_materials =
        actuator_receipt && actuator_receipt->valid ? actuator_receipt : NULL;
    s->dirty = 1;
}

void dm2_v1_viewport_set_gdat_interface_hud_layout(
    DM2_V1_ViewportState *s,
    const DM2_V1_InterfaceHudLayout *layout)
{
    if (!s) return;
    s->gdat_interface_hud_layout = layout && layout->valid ? layout : NULL;
    s->dirty = 1;
}

int dm2_v1_viewport_hud_dynamic_overlay_ready(
    const DM2_V1_ViewportState *s,
    const DM2_V1_HudChampionSlotRender *champion)
{
    /* SKWIN/SkWinCore.cpp draws the active champion data through the expanded
     * dt04 rectangles, the interface palette, and QUERY_FONT's dt07 rows.
     * All four inputs must stay boot/session-owned in a source-only frame. */
    return s && champion && champion->state_source_bound &&
        champion->stat_bar_color_source_bound &&
        champion->stat_bar_color < 16u &&
        s->gdat_interface_hud_layout &&
        s->gdat_interface_palette_ready &&
        s->gdat_interface_palette_hash != 0u &&
        s->gdat_interface_font_rows &&
        s->gdat_interface_font_hash != 0u;
}

void dm2_v1_viewport_set_gdat_interface_rect14(
    DM2_V1_ViewportState *s,
    const uint8_t *rows,
    uint32_t row_count,
    uint32_t hash)
{
    if (!s) return;
    /* skproject/SKWIN/SkWinCore.cpp QUERY_CREATURE_PICST consumes the
     * LOAD_GDAT_INTERFACE_00_0A table only after runtime has checked its
     * host receipt. An empty or unhashed buffer is never a drawable owner. */
    s->gdat_interface_rect14_rows = rows && row_count > 0u && hash != 0u
        ? rows : NULL;
    s->gdat_interface_rect14_row_count =
        s->gdat_interface_rect14_rows ? row_count : 0u;
    s->gdat_interface_rect14_hash =
        s->gdat_interface_rect14_rows ? hash : 0u;
    s->dirty = 1;
}

/* ── Wall frame lookup ────────────────────────────────────────────── */

const DM2_WallFrame *dm2_v1_get_wall_frame(int view_square)
{
    if (view_square < 0 || view_square >= DM2_SQ_COUNT) return NULL;
    return &g_dm2_wall_frames[view_square];
}

int dm2_v1_viewport_wall_field_for_square(int view_square)
{
    if (view_square < 0 || view_square >= DM2_SQ_COUNT) return -1;
    if (g_dm2_wall_frames[view_square].byte_width == 0 ||
        g_dm2_wall_frames[view_square].height == 0) {
        return -1;
    }
    if (view_square == DM2_SQ_D3C) return -1;
    /* skproject SKWIN/SkWinCore.cpp DRAW_WALL/QUERY_TEMP_PICST
     * lines ~47373-47474 maps normal wall cells through
     * `iViewportCell + 0x22`.  The admitted startup route retains the
     * original ten drawable cells in command order. */
    return DM2_V1_VIEWPORT_GFX_WALL_FIELD_FIRST + view_square;
}

int dm2_v1_viewport_draw_dungeon_tiles_pass_for_square(int view_square)
{
    /* D3C has no DRAW_WALL GRAPHICSSET field.  D0C is the front-player tile
     * and is drawn outside table1d7029 (via the door/player-tile path), so it
     * must not be promoted to a generic DRAW_WALL pass either.  The remaining
     * fields use iViewportCell + 0x22, so their source cell is the square
     * ordinal. */
    if (view_square == DM2_SQ_D3C || view_square == DM2_SQ_D0C ||
        dm2_v1_viewport_wall_field_for_square(view_square) <
            DM2_V1_VIEWPORT_GFX_WALL_FIELD_FIRST) {
        return -1;
    }
    return dm2_v1_viewport_draw_dungeon_tiles_pass_for_cell(view_square);
}

int dm2_v1_viewport_draw_dungeon_tiles_pass_for_cell(int skproject_cell)
{
    if (skproject_cell < 0 || skproject_cell > 0xff) return -1;
    for (int pass = 0; pass < (int)(sizeof(s_dm2_draw_dungeon_tiles_cells) /
                                    sizeof(s_dm2_draw_dungeon_tiles_cells[0]));
         ++pass) {
        if (s_dm2_draw_dungeon_tiles_cells[pass] == (uint8_t)skproject_cell) {
            return pass;
        }
    }
    return -1;
}

int dm2_v1_viewport_wall_graphic_index_for_square(int view_square)
{
    return dm2_v1_viewport_wall_graphic_index_for_graphicsset(
        DM2_V1_VIEWPORT_GFX_WALL_DEFAULT_GRAPHICSSET, view_square);
}

int dm2_v1_viewport_wall_graphic_index_for_graphicsset(int graphicsset_index,
                                                        int view_square)
{
    int field = dm2_v1_viewport_wall_field_for_square(view_square);

    if (field < 0 || graphicsset_index < 0 || graphicsset_index > 0xff) {
        return 0;
    }
    /* ReDMCSB lineage: SkWinCore.cpp DRAW_WALL lines 47466-47474 queries
     * GDAT_CATEGORY_GRAPHICSSET with the live iMapGfx for each wall cell. */
    if (graphicsset_index == DM2_V1_VIEWPORT_GFX_WALL_DEFAULT_GRAPHICSSET) {
        return DM2_V1_VIEWPORT_GFX_WALL_FIELD_BASE - field;
    }
    return DM2_V1_VIEWPORT_GFX_WALL_GRAPHICSSET_BASE -
           (graphicsset_index << 8) - field;
}

int dm2_v1_viewport_wall_graphic_address(int gdat_index,
                                         int *out_graphicsset_index,
                                         int *out_field)
{
    int packed;
    int graphicsset_index;
    int field;

    if (!out_graphicsset_index || !out_field) return 0;
    if (gdat_index <= DM2_V1_VIEWPORT_GFX_WALL_FIELD_BASE -
                          DM2_V1_VIEWPORT_GFX_WALL_FIELD_FIRST &&
        gdat_index > DM2_V1_VIEWPORT_GFX_DOOR_FRAME_FIELD_BASE) {
        field = DM2_V1_VIEWPORT_GFX_WALL_FIELD_BASE - gdat_index;
        *out_graphicsset_index = DM2_V1_VIEWPORT_GFX_WALL_DEFAULT_GRAPHICSSET;
        *out_field = field;
        return 1;
    }
    packed = DM2_V1_VIEWPORT_GFX_WALL_GRAPHICSSET_BASE - gdat_index;
    graphicsset_index = (packed >> 8) & 0xff;
    field = packed & 0xff;
    if (packed < 0 || packed > 0xff3f ||
        gdat_index > DM2_V1_VIEWPORT_GFX_WALL_GRAPHICSSET_BASE ||
        field < DM2_V1_VIEWPORT_GFX_WALL_FIELD_FIRST || field >= 0x40) {
        return 0;
    }
    *out_graphicsset_index = graphicsset_index;
    *out_field = field;
    return 1;
}

int dm2_v1_viewport_build_wall_panel_render_plan(
    const DM2_V1_ViewportState *s,
    DM2_V1_WallPanelRenderPlan *out_plan)
{
    if (!out_plan) {
        return 0;
    }
    memset(out_plan, 0, sizeof(*out_plan));
    /* DRAW_WALL reads glbMapGraphicsSet. The planner is also consumed by
     * M11 before viewport drawing, so it must reject an absent G1 owner here
     * rather than encode the former renderer-default graphics set. */
    if (s && s->source_materials_required && !s->gdat_scene_control_ready) {
        return 0;
    }
    out_plan->party_direction = s ? (s->party_dir & 3) : 0;
    /* The source scheduler owns the wall traversal. Do not reuse the DM1
     * depth ordering: DM2_DRAW_DUNGEON_TILES walks table1d7029 and invokes
     * DRAW_WALL_TILE for a cell only at that pass. */
    for (int step = 0; step < (int)(sizeof(s_dm2_draw_dungeon_tiles_cells) /
                                    sizeof(s_dm2_draw_dungeon_tiles_cells[0]));
         ++step) {
        int square = s_dm2_draw_dungeon_tiles_cells[step];
        const DM2_WallFrame *frame = dm2_v1_get_wall_frame(square);
        int graphicsset_index = s && s->gdat_scene_control_ready
            ? s->gdat_scene_material_index
            : DM2_V1_VIEWPORT_GFX_WALL_DEFAULT_GRAPHICSSET;
        int gdat_index = dm2_v1_viewport_wall_graphic_index_for_graphicsset(
            graphicsset_index, square);
        DM2_V1_WallPanelRender *row;

        /* G1/c_map has already projected the actual dungeon tile into this
         * view square for the current party direction.  In source-required
         * M10 mode, an absent wall fact is not permission to draw the generic
         * GRAPHICSSET panel.  The cell value from table1d7029 is the Firestaff
         * view-square index for wall panels; the step itself is the source
         * draw order (DUNVIEW.C:8466-8542). */
        if (square < 0 || square >= DM2_SQ_COUNT ||
            s_dm2_draw_dungeon_tiles_cells[step] != (uint8_t)square ||
            (s && s->source_materials_required &&
             (s->squares[square].flags & DM2_SQF_HAS_WALL) == 0u)) {
            continue;
        }
        /* D0C is the front-player tile; it is scheduled by the door/player-tile
         * path, not by the generic table1d7029 wall scheduler.  Keep it out of
         * the source GDAT wall plan and of non-source previews.  The bounded
         * asset-fallback path (G1 unit tests with no pre-built wall plan) is
         * allowed to draw it as a wall when the square is explicitly flagged. */
        if (square == DM2_SQ_D0C &&
            !(s && s->source_materials_required && !s->gdat_wall_material_plan)) {
            continue;
        }
        if (!frame || frame->byte_width == 0 || frame->height == 0 ||
            gdat_index == 0 ||
            out_plan->panel_count >= DM2_V1_WALL_PANEL_RENDER_MAX) {
            continue;
        }
        row = &out_plan->panels[out_plan->panel_count++];
        row->render_step = step;
        row->view_square = square;
        row->skproject_cell = dm2_v1_viewport_skproject_cell_for_square(square);
        row->gdat_index = gdat_index;
        row->src_rect = (DM2_V1_ViewportRect){
            frame->blit_x,
            frame->blit_y,
            frame->byte_width,
            frame->height
        };
        row->dst_rect = (DM2_V1_ViewportRect){
            frame->left_x,
            frame->top_y,
            frame->right_x - frame->left_x + 1,
            frame->bottom_y - frame->top_y + 1
        };
        /* SKWIN c_gui_vp.cpp::DM2_DRAW_WALL resolves this cell's GDAT
         * image before blitting. The plan intentionally has no colour
         * fallback: an unresolved source image is a no-draw condition. */
        out_plan->selected_square_mask |= (uint16_t)(1u << (unsigned)square);
    }
    return 1;
}

int dm2_v1_viewport_door_frame_field_for_square(int view_square)
{
    switch (view_square) {
    case DM2_SQ_D0C:
        return DM2_V1_VIEWPORT_GFX_DOOR_FRAME_FRONT;
    case DM2_SQ_D1C:
        return DM2_V1_VIEWPORT_GFX_DOOR_FRAME_D1C;
    case DM2_SQ_D2C:
        return DM2_V1_VIEWPORT_GFX_DOOR_FRAME_D2C;
    default:
        return -1;
    }
}

int dm2_v1_viewport_door_frame_graphic_index_for_square(int view_square)
{
    int field = dm2_v1_viewport_door_frame_field_for_square(view_square);
    if (field < 0) return 0;
    return DM2_V1_VIEWPORT_GFX_DOOR_FRAME_FIELD_BASE - field;
}

int dm2_v1_viewport_door_frame_graphic_index_for_graphicsset(
    int graphicsset_index, int view_square)
{
    int field = dm2_v1_viewport_door_frame_field_for_square(view_square);

    if (field < 0 || graphicsset_index < 0 || graphicsset_index > 0xff) {
        return 0;
    }
    if (graphicsset_index == DM2_V1_VIEWPORT_GFX_WALL_DEFAULT_GRAPHICSSET) {
        return DM2_V1_VIEWPORT_GFX_DOOR_FRAME_FIELD_BASE - field;
    }
    return DM2_V1_VIEWPORT_GFX_DOOR_FRAME_GRAPHICSSET_BASE -
        (graphicsset_index << 8) - field;
}

int dm2_v1_viewport_door_side_frame_source_for_movement(
    int view_square, int side, int movement_active,
    int *out_graphicsset_field, int *out_rect_number, int *out_mirror_flip,
    int *out_offset_x, int *out_offset_y)
{
    /* SKWIN/skval1.h: tlbGraphicsDoorSideFrames[14][2].  The active centre
     * cells are 0, 3, 6 and 11 (D0..D3) in DRAW_DOOR_FRAMES. */
    static const uint8_t side_frames[14][2] = {
        { 0xd3u, 0xd4u }, { 0xffu, 0xffu }, { 0xffu, 0xffu },
        { 0x07u, 0x08u }, { 0xffu, 0xd5u }, { 0xd6u, 0xffu },
        { 0x09u, 0x0au }, { 0xffu, 0xd7u }, { 0xd8u, 0xffu },
        { 0xffu, 0xffu }, { 0xffu, 0xffu }, { 0x0bu, 0x0cu },
        { 0x0du, 0x0eu }, { 0x0fu, 0x10u }
    };
    static const uint8_t movement_cells[14] = {
        0x00u, 0x02u, 0x01u, 0x03u, 0x05u, 0x04u, 0x06u,
        0x08u, 0x07u, 0x0au, 0x09u, 0x0bu, 0x0du, 0x0cu
    };
    int cell;
    int material_cell;
    int material_side;
    uint8_t field;

    if (!out_graphicsset_field || !out_rect_number || !out_mirror_flip ||
        !out_offset_x || !out_offset_y || side < 0 || side > 1) return 0;
    switch (view_square) {
    case DM2_SQ_D0C: cell = 0; break;
    case DM2_SQ_D1C: cell = 3; break;
    case DM2_SQ_D2C: cell = 6; break;
    case DM2_SQ_D3C: cell = 11; break;
    default: return 0;
    }
    /* SKULLWIN c_gui_vp.cpp:2390-2404: v1e12d0 selects table1d6b2c[cell],
     * then left reads table1d6ee1[row][1] and right reads [row][0]. */
    material_cell = movement_active ? movement_cells[cell] : cell;
    material_side = movement_active ? 1 - side : side;
    field = side_frames[material_cell][material_side];
    if (field == 0xffu) return 0;
    *out_graphicsset_field = field;
    *out_rect_number = 5000 + cell * 25 + (side == 0 ? 10 : 14);
    /* DRAW_DOOR_FRAMES: left uses QUERY_TEMP_PICST(0,...,rect,4), right
     * uses QUERY_TEMP_PICST(1,...,rect,3), then applies these offsets. */
    *out_mirror_flip = side == 0 ? 0 : 1;
    *out_offset_x = side == 0 ? -2 : 2;
    *out_offset_y = 4;
    return 1;
}

int dm2_v1_viewport_door_side_frame_source(int view_square, int side,
                                           int *out_graphicsset_field,
                                           int *out_rect_number,
                                           int *out_mirror_flip,
                                           int *out_offset_x,
                                           int *out_offset_y)
{
    return dm2_v1_viewport_door_side_frame_source_for_movement(
        view_square, side, 0, out_graphicsset_field, out_rect_number,
        out_mirror_flip, out_offset_x, out_offset_y);
}

int dm2_v1_viewport_door_panel_field_for_square(int view_square)
{
    switch (view_square) {
    case DM2_SQ_D0C:
        return DM2_V1_VIEWPORT_GFX_DOOR_PANEL_FRONT;
    case DM2_SQ_D1C:
        return DM2_V1_VIEWPORT_GFX_DOOR_PANEL_D1C;
    case DM2_SQ_D2C:
        return DM2_V1_VIEWPORT_GFX_DOOR_PANEL_D2C;
    case DM2_SQ_D3C:
        return 2;
    default:
        return -1;
    }
}

int dm2_v1_viewport_door_panel_graphic_index_for_square(int view_square)
{
    int field = dm2_v1_viewport_door_panel_field_for_square(view_square);
    if (field < 0) return 0;
    return DM2_V1_VIEWPORT_GFX_DOOR_PANEL_FIELD_BASE - field;
}

int dm2_v1_viewport_door_panel_graphic_index_for_record(int view_square,
                                                        int door_gfx_index,
                                                        int opening_dir)
{
    int field = dm2_v1_viewport_door_panel_field_for_square(view_square);
    int record_field;
    if (field < 0) return 0;
    if (door_gfx_index < 0) door_gfx_index = 0;
    if (door_gfx_index > 0xff) door_gfx_index = 0xff;
    /* skproject SKWIN/SkWinCore.cpp lines 46405-46431 fetch the panel from
     * GDAT_CATEGORY_DOORS using glbMapDoorType[DoorType()]. Lines 46580-46606
     * use OpeningDir() to select the split/position path for moving panels. */
    record_field = field & DM2_V1_VIEWPORT_GFX_DOOR_PANEL_FIELD_MASK;
    record_field |= (opening_dir & 1) <<
        DM2_V1_VIEWPORT_GFX_DOOR_PANEL_OPENING_SHIFT;
    record_field |= (door_gfx_index & 0xff) <<
        DM2_V1_VIEWPORT_GFX_DOOR_PANEL_INDEX_SHIFT;
    return DM2_V1_VIEWPORT_GFX_DOOR_RECORD_PANEL_FIELD_BASE - record_field;
}

int dm2_v1_viewport_door_ornate_graphic_index(int door_ornate_index,
                                              int view_square)
{
    int field = dm2_v1_viewport_door_panel_field_for_square(view_square);
    int packed;
    if (field < 0 || door_ornate_index <= 0) return 0;
    if (door_ornate_index > 0xff) door_ornate_index = 0xff;
    /* skproject SKWIN/SkWinCore.cpp lines 46477-46510 draws Door::OrnateIndex()
     * through GDAT_CATEGORY_DOOR_GFX after the base door panel. */
    packed = ((door_ornate_index & 0xff) <<
              DM2_V1_VIEWPORT_GFX_DOOR_OVERLAY_INDEX_SHIFT) |
             (field & DM2_V1_VIEWPORT_GFX_DOOR_OVERLAY_FIELD_MASK);
    return DM2_V1_VIEWPORT_GFX_DOOR_ORNATE_FIELD_BASE - packed;
}

int dm2_v1_viewport_door_destroyed_mask_graphic_index(int door_gfx_index,
                                                      int view_square)
{
    int field = dm2_v1_viewport_door_panel_field_for_square(view_square);
    int packed;
    if (field < 0) return 0;
    if (door_gfx_index < 0) door_gfx_index = 0;
    if (door_gfx_index > 0xff) door_gfx_index = 0xff;
    /* skproject SKWIN/SkWinCore.cpp lines 46513-46535 overlays the destroyed
     * mask from GDAT_CATEGORY_DOORS when tile door state is 5. */
    packed = ((door_gfx_index & 0xff) <<
              DM2_V1_VIEWPORT_GFX_DOOR_OVERLAY_INDEX_SHIFT) |
             (field & DM2_V1_VIEWPORT_GFX_DOOR_OVERLAY_FIELD_MASK);
    return DM2_V1_VIEWPORT_GFX_DOOR_DESTROYED_MASK_FIELD_BASE - packed;
}

int dm2_v1_viewport_door_button_field_for_state(int pushed)
{
    /* skproject SKWIN/SkWinCore.cpp DRAW_DOOR_FRAMES line ~46342 calls
     * DRAW_DEFAULT_DOOR_BUTTON(GDAT_CATEGORY_DOOR_BUTTONS, 0,
     * door->ButtonState() * 5, iViewportCell). */
    return pushed ? DM2_V1_VIEWPORT_GFX_DOOR_BUTTON_PUSHED
                  : DM2_V1_VIEWPORT_GFX_DOOR_BUTTON_RELEASED;
}

int dm2_v1_viewport_door_button_graphic_index_for_state(int pushed)
{
    int field = dm2_v1_viewport_door_button_field_for_state(pushed);
    return DM2_V1_VIEWPORT_GFX_DOOR_BUTTON_FIELD_BASE - field;
}

int dm2_v1_viewport_door_open_pct_from_state(int door_state,
                                             int explicit_open_pct)
{
    int source_pct;

    if (door_state < 0 || door_state > 5) {
        door_state = 4; /* unknown state -> closed, fail-closed */
    }
    source_pct = dm2_v1_creature_door_open_pct_from_state(door_state);
    /* An explicit runtime percentage is authoritative for animation frames.
     * When the caller leaves it at zero, derive the percentage from the
     * source state table so OPEN (0), partial states (1..3), CLOSED (4) and
     * DESTROYED (5) all carry the correct panel visibility. */
    if (explicit_open_pct > 0 && explicit_open_pct <= 100) {
        return explicit_open_pct;
    }
    return source_pct;
}

static const int8_t s_dm2_square_to_skproject_cell[DM2_SQ_COUNT] = {
    /* Firestaff D3/D2/D1/D0 center rows do not have the same ordinal as
     * skproject's tblCellTilesRoom viewport cells.  skproject SKWINSPX
     * kskval1.h line 62 defines tlbRectnoDoorButton for cells 0,3,6,11,13;
     * SkWinCore.cpp DRAW_DOOR_TILE lines ~46650-46700 dispatches center-door
     * cells 0,3,6 through DRAW_DOOR for the D0/D1/D2 startup path. */
    /* D3C */ 11, /* D3L */ -1, /* D3R */ -1,
    /* D2C */  6, /* D2L */ -1, /* D2R */ -1,
    /* D1C */  3, /* D1L */ -1, /* D1R */ -1,
    /* D0C */  0, /* D0L */ -1, /* D0R */ -1,
};

static const int8_t s_dm2_skproject_rectno_door_button[14] = {
    /* skproject SKWINSPX/src/v4/kskval1.h line 62:
     * tlbRectnoDoorButton =
     *   {4,-1,-1,3,-1,-1,2,-1,-1,-1,-1,1,-1,0}. */
     4, -1, -1,  3, -1, -1,  2,
    -1, -1, -1, -1,  1, -1,  0,
};

int dm2_v1_viewport_skproject_cell_for_square(int view_square)
{
    if (view_square < 0 || view_square >= DM2_SQ_COUNT) return -1;
    return s_dm2_square_to_skproject_cell[view_square];
}

int dm2_v1_viewport_door_button_rectno_for_square(int view_square)
{
    int cell = dm2_v1_viewport_skproject_cell_for_square(view_square);
    if (cell < 0 || cell >= (int)(sizeof s_dm2_skproject_rectno_door_button /
                                  sizeof s_dm2_skproject_rectno_door_button[0])) {
        return -1;
    }
    return s_dm2_skproject_rectno_door_button[cell];
}

int dm2_v1_viewport_door_button_clickable_for_square(int view_square)
{
    int rectno = dm2_v1_viewport_door_button_rectno_for_square(view_square);
    /* skproject SKWIN/SkWinCore.cpp DRAW_DEFAULT_DOOR_BUTTON lines
     * ~46261-46264 calls MAKE_BUTTON_CLICKABLE only for rectno 3 and 4. */
    return rectno == 3 || rectno == 4;
}

int dm2_v1_viewport_wall_button_graphic_index(int wall_gfx_index,
                                              int wall_gfx_field)
{
    int packed;
    if (wall_gfx_index < 0 || wall_gfx_index > 0xFF ||
        wall_gfx_field < 0 || wall_gfx_field > 0xFF) {
        return 0;
    }
    packed = (wall_gfx_index << DM2_V1_VIEWPORT_GFX_WALL_BUTTON_INDEX_SHIFT) |
             (wall_gfx_field & DM2_V1_VIEWPORT_GFX_WALL_BUTTON_FIELD_MASK);
    return DM2_V1_VIEWPORT_GFX_WALL_BUTTON_FIELD_BASE - packed;
}

int dm2_v1_viewport_creature_graphic_index(int creature_type,
                                           int frame_index)
{
    int packed;
    if (creature_type < 0 || creature_type > 0xFF ||
        frame_index < 0 || frame_index > 0xFF) {
        return 0;
    }
    packed = (creature_type << DM2_V1_VIEWPORT_GFX_CREATURE_INDEX_SHIFT) |
             (frame_index & DM2_V1_VIEWPORT_GFX_CREATURE_FIELD_MASK);
    return DM2_V1_VIEWPORT_GFX_CREATURE_FIELD_BASE - packed;
}

int dm2_v1_viewport_creature_field_graphic_index(int creature_type,
                                                 int image_field)
{
    int packed;
    if (creature_type < 0 || creature_type > 0xff ||
        image_field < 0 || image_field > 0xff) return 0;
    packed = (creature_type << DM2_V1_VIEWPORT_GFX_CREATURE_INDEX_SHIFT) |
             (image_field & DM2_V1_VIEWPORT_GFX_CREATURE_FIELD_MASK);
    return DM2_V1_VIEWPORT_GFX_CREATURE_DIRECT_FIELD_BASE - packed;
}

int dm2_v1_viewport_item_graphic_index(int item_category,
                                       int item_type,
                                       int frame_index)
{
    int packed;
    if (item_category <= 0 || item_category > 0xFF ||
        item_type < 0 || item_type > 0xFF ||
        frame_index < 0 || frame_index > 0xFF) {
        return 0;
    }
    packed = (item_category << DM2_V1_VIEWPORT_GFX_ITEM_CATEGORY_SHIFT) |
             (item_type << DM2_V1_VIEWPORT_GFX_ITEM_INDEX_SHIFT) |
             (frame_index & DM2_V1_VIEWPORT_GFX_ITEM_FIELD_MASK);
    return DM2_V1_VIEWPORT_GFX_ITEM_FIELD_BASE - packed;
}

int dm2_v1_viewport_select_carried_item_image_field(uint16_t selector,
                                                    uint32_t object_index,
                                                    uint32_t game_tick,
                                                    int party_direction,
                                                    uint8_t *out_image_field)
{
    uint16_t frame_count;
    uint16_t mode;
    uint8_t field = 0x18u;

    if (!out_image_field || party_direction < 0 || party_direction > 3) {
        return 0;
    }
    frame_count = selector & 0x000fu;
    if (frame_count == 0u) {
        *out_image_field = field;
        return 1;
    }
    /* _2405_014a delegates this to IS_ITEM_FIT_FOR_EQUIP, whose record and
     * action context is not yet owned by the leader cursor. */
    if ((selector & 0x8000u) != 0u) {
        return 0;
    }
    mode = (selector >> 8) & 0x001fu;
    switch (mode) {
    case 0u:
        field = (uint8_t)(field + (game_tick % frame_count));
        break;
    case 2u:
        field = (uint8_t)(field + party_direction);
        break;
    case 5u:
        field = (uint8_t)(field +
                          ((game_tick + object_index) % frame_count));
        break;
    default:
        /* Modes 1/3/4/6 read random or record charge state. */
        return 0;
    }
    *out_image_field = field;
    return 1;
}

int dm2_v1_viewport_item_category_for_db_pool(int db_pool)
{
    switch (db_pool) {
    case 5:  return 0x10; /* WEAPON */
    case 6:  return 0x11; /* CLOTH */
    case 7:  return 0x12; /* SCROLL */
    case 10: return 0x15; /* MISC */
    default: return 0;
    }
}

int dm2_v1_viewport_projectile_graphic_index(int projectile_category,
                                             int projectile_type,
                                             int frame_index)
{
    int packed;
    if (projectile_category <= 0 || projectile_category > 0xFF ||
        projectile_type < 0 || projectile_type > 0xFF ||
        frame_index < 0 || frame_index > 0xFF) {
        return 0;
    }
    packed = (projectile_category << DM2_V1_VIEWPORT_GFX_PROJECTILE_CATEGORY_SHIFT) |
             (projectile_type << DM2_V1_VIEWPORT_GFX_PROJECTILE_INDEX_SHIFT) |
             (frame_index & DM2_V1_VIEWPORT_GFX_PROJECTILE_FIELD_MASK);
    return DM2_V1_VIEWPORT_GFX_PROJECTILE_FIELD_BASE - packed;
}

int dm2_v1_viewport_hud_portrait_graphic_index(int portrait_index)
{
    int packed;
    if (portrait_index < 0 || portrait_index > 255) {
        return 0;
    }
    /* skproject SKWIN/SkWinCore.cpp T560 draws the right-side status
     * portraits through UI GDAT image queries.  Firestaff packs the
     * source HeroType into a renderer-private index. The source gate owns
     * validation; this packing must not narrow the 8-bit record field. */
    packed = (portrait_index << DM2_V1_VIEWPORT_GFX_HUD_PORTRAIT_INDEX_SHIFT) |
             DM2_V1_VIEWPORT_GFX_HUD_PORTRAIT_FIELD;
    return DM2_V1_VIEWPORT_GFX_HUD_PORTRAIT_FIELD_BASE - packed;
}

int dm2_v1_viewport_hud_core_graphic_index(int field)
{
    if (field < 0 || field > DM2_V1_VIEWPORT_GFX_HUD_CORE_FIELD_MASK) {
        return 0;
    }
    return DM2_V1_VIEWPORT_GFX_HUD_CORE_FIELD_BASE - field;
}

int dm2_v1_viewport_hud_hand_action_graphic_index(int possession_index,
                                                   int left_or_right)
{
    int entry;
    if (possession_index < 0 || possession_index > 1 ||
        left_or_right < 0 || left_or_right > 1) {
        return 0;
    }
    entry = (possession_index << 1) + left_or_right + 2;
    return DM2_V1_VIEWPORT_GFX_HUD_HAND_ACTION_BASE - entry;
}

int dm2_v1_viewport_hud_hand_action_graphic_address(
    int gdat_index,
    int *out_possession_index,
    int *out_left_or_right,
    int *out_entry)
{
    int entry = DM2_V1_VIEWPORT_GFX_HUD_HAND_ACTION_BASE - gdat_index;
    if (!out_possession_index || !out_left_or_right || !out_entry ||
        entry < 2 || entry > 5) {
        return 0;
    }
    *out_entry = entry;
    *out_possession_index = (entry - 2) >> 1;
    *out_left_or_right = (entry - 2) & 1;
    return 1;
}

int dm2_v1_viewport_hud_action_icon_graphic_index(int icon_index)
{
    if (icon_index < 0 || icon_index >= DM2_V1_HUD_ACTION_ICON_COUNT) {
        return 0;
    }
    return dm2_v1_viewport_hud_core_graphic_index(
        DM2_V1_VIEWPORT_GFX_HUD_CORE_ACTION_ICON_BASE + icon_index);
}

int dm2_v1_viewport_map_chip_frame_width(int src_w, int src_h)
{
    if (src_w <= 0 || src_h <= 0) return 0;
    /* skproject SKWIN/SkWinCore.cpp DRAW_CHIP_OF_MAGIC_MAP lines
     * 1001-1037 selects source X as glbMagicMapWidth * frame. DM2's
     * startup constants set glbMagicMapWidth/glbMagicMapHeight to 7, and
     * QUERY_DUNGEON_MAP_CHIP_PICT returns atlas_width / glbMagicMapWidth.
     * Use square tiles for decoded atlases and keep single bitmap fixtures
     * unchanged. */
    if (src_w > src_h && (src_w % src_h) == 0) return src_h;
    return src_w;
}

int dm2_v1_viewport_map_chip_frame_count(int src_w, int src_h)
{
    int frame_w = dm2_v1_viewport_map_chip_frame_width(src_w, src_h);
    if (frame_w <= 0 || src_w <= 0) return 0;
    return src_w / frame_w;
}

int dm2_v1_viewport_map_chip_frame_index(int requested_frame,
                                         int frame_count)
{
    if (frame_count <= 0) return 0;
    if (requested_frame < 0) return 0;
    return requested_frame % frame_count;
}

int dm2_v1_viewport_projectile_frame_for_direction(int requested_frame,
                                                   int projectile_direction,
                                                   int party_direction,
                                                   int frame_count)
{
    int rel;
    if (frame_count <= 0) return 0;
    /* skproject SKWIN/SkWinCore.cpp DRAW_MAP_CHIP lines 1525-1575 uses
     * directional missile frames when QUERY_DUNGEON_MAP_CHIP_PICT reports
     * more than three 7px frames. The source chooses a base missile frame
     * near 3 plus a view-relative direction term before DRAW_CHIP_OF_MAGIC_MAP
     * slices the atlas. Keep short atlases on their requested animation frame. */
    if (frame_count <= 3) {
        return dm2_v1_viewport_map_chip_frame_index(requested_frame,
                                                   frame_count);
    }
    rel = ((projectile_direction & 3) - (party_direction & 3)) & 3;
    return dm2_v1_viewport_map_chip_frame_index(3 + rel, frame_count);
}

int dm2_v1_viewport_projectile_frame_for_map_chip(int requested_frame,
                                                  int projectile_direction,
                                                  int object_direction,
                                                  int party_direction,
                                                  int frame_count,
                                                  int frame_class)
{
    static const int8_t s_skproject_missile_frame_adjust[16] = {
        0, 0, 2, 2,
        0, 2, 2, 0,
        0, 0, -2, -2,
        0, -2, -2, 0
    };
    int motion_rel;
    int object_rel;
    int frame;

    if (frame_count <= 0) return 0;
    if (frame_count <= 3) {
        return dm2_v1_viewport_map_chip_frame_index(requested_frame,
                                                   frame_count);
    }

    /* skproject SKWIN/SkWinCore.cpp `_48ae_011a` lines 10168-10198
     * classifies missile map-chip image coverage. DRAW_MAP_CHIP lines
     * 10691-10718 applies `_4976_3fa8` only for class 1; the other source
     * cases collapse to frame 0 or the base-front frame 3. */
    switch ((uint8_t)frame_class) {
    case DM2_V1_PROJECTILE_FRAME_CLASS_DIRECTIONAL:
        motion_rel = ((projectile_direction & 3) - (party_direction & 3)) & 3;
        object_rel = ((object_direction & 3) - (party_direction & 3)) & 3;
        frame = 3 + motion_rel;
        frame += s_skproject_missile_frame_adjust[
            ((frame - 3) << 2) + object_rel];
        return dm2_v1_viewport_map_chip_frame_index(frame, frame_count);
    case DM2_V1_PROJECTILE_FRAME_CLASS_BASE_FRONT:
        return dm2_v1_viewport_map_chip_frame_index(3, frame_count);
    case DM2_V1_PROJECTILE_FRAME_CLASS_MISSING:
    case DM2_V1_PROJECTILE_FRAME_CLASS_FRONT_ONLY:
    case DM2_V1_PROJECTILE_FRAME_CLASS_FLAT:
    default:
        return 0;
    }
}

int dm2_v1_viewport_projectile_flip_for_direction(int projectile_direction,
                                                  int party_direction)
{
    /* skproject SKWIN/SkGlobal.cpp `_4976_3fa4 = {0,1,3,2}`; DRAW_MAP_CHIP
     * lines 10720-10725 passes it to DRAW_CHIP_OF_MAGIC_MAP for dbMissile. */
    return dm2_v1_viewport_map_chip_flip_for_object_direction(
        projectile_direction, party_direction);
}

int dm2_v1_viewport_map_chip_flip_for_object_direction(int object_direction,
                                                       int party_direction)
{
    static const uint8_t s_skproject_object_flip[4] = { 0, 1, 3, 2 };
    int rel = ((object_direction & 3) - (party_direction & 3)) & 3;
    /* skproject SKWIN/SkGlobal.cpp `_4976_3fa4 = {0,1,3,2}` is also used
     * for creature possession overlays in SkWinCore.cpp DRAW_MAP_CHIP
     * lines 10798-10815, where `(si.Dir() - viewDir) & 3` selects the
     * mirror passed to DRAW_CHIP_OF_MAGIC_MAP. */
    return s_skproject_object_flip[rel];
}

int dm2_v1_viewport_cloud_frame_for_tick(int tick_count,
                                         int frame_count)
{
    if (frame_count <= 0) return 0;
    /* skproject SKWIN/SkWinCore.cpp DRAW_MAP_CHIP lines 10672-10743:
     * dbCloud objects use `(glbGameTick & 1) + 1` before
     * DRAW_CHIP_OF_MAGIC_MAP instead of the missile directional frame path. */
    return dm2_v1_viewport_map_chip_frame_index((tick_count & 1) + 1,
                                                frame_count);
}

int dm2_v1_viewport_cloud_flip_for_seed(uint32_t *seed)
{
    uint32_t next_seed;

    if (!seed) return 0;
    next_seed = (*seed * 0xbb40e62du) + 11u;
    *seed = next_seed;
    /* skproject SKWIN/SkWinCore.cpp DRAW_MAP_CHIP lines 10743-10749
     * passes RAND02() to DRAW_CHIP_OF_MAGIC_MAP for dbCloud. RAND02
     * lines 33533-33541 advances glbRandomSeed with the same LCG and
     * returns `(glbRandomSeed >> 8) & 3`. */
    return (int)((next_seed >> 8) & 3u);
}

int dm2_v1_viewport_creature_frame_for_direction(int requested_frame,
                                                 int creature_direction,
                                                 int party_direction,
                                                 int frame_count)
{
    int rel;
    int base;

    if (frame_count <= 0) return 0;
    if (frame_count <= 3) {
        return dm2_v1_viewport_map_chip_frame_index(requested_frame,
                                                   frame_count);
    }

    /* skproject SKWIN/SkWinCore.cpp DRAW_MAP_CHIP lines 10588-10618:
     * creatures are fetched as one IMG_MAP_CHIP atlas and the drawn frame is
     * `(viewDir - creatureDir) & 1` added to an even animation base before
     * DRAW_CHIP_OF_MAGIC_MAP slices the 7px chip. */
    base = requested_frame & ~1;
    if (base + 1 >= frame_count) base = 0;
    rel = ((party_direction & 3) - (creature_direction & 3)) & 3;
    return dm2_v1_viewport_map_chip_frame_index(base + (rel & 1),
                                                frame_count);
}

static void dm2_v1_viewport_clear_rect(DM2_V1_ViewportRect *out_rect)
{
    if (!out_rect) return;
    out_rect->x = 0;
    out_rect->y = 0;
    out_rect->w = 0;
    out_rect->h = 0;
}

int dm2_v1_viewport_door_panel_rect_for_square(int view_square,
                                               DM2_V1_ViewportRect *out_rect)
{
    if (!out_rect) return 0;
    dm2_v1_viewport_clear_rect(out_rect);

    /* skproject SKWIN/SkWinCore.cpp DRAW_DOOR routes D0C/D1C/D2C through
     * viewport-cell door graphics. These are the current bounded startup
     * rectangles; exact tlbRectnoDoorButton/panel-table replacement stays
     * isolated behind this API. */
    switch (view_square) {
    case DM2_SQ_D0C:
        out_rect->x = 80;
        out_rect->y = 0;
        out_rect->w = 160;
        out_rect->h = 135;
        return 1;
    case DM2_SQ_D1C:
        out_rect->x = 60;
        out_rect->y = 9;
        out_rect->w = 104;
        out_rect->h = 110;
        return 1;
    case DM2_SQ_D2C:
        out_rect->x = 60;
        out_rect->y = 20;
        out_rect->w = 103;
        out_rect->h = 71;
        return 1;
    case DM2_SQ_D3C:
        /* SKWIN c_gui_vp.cpp G0163: D3C is the source table's
         * [74..149] x [25..75] panel. It has no DRAW_DOOR_FRAMES route. */
        out_rect->x = 74;
        out_rect->y = 25;
        out_rect->w = 76;
        out_rect->h = 51;
        return 1;
    default:
        return 0;
    }
}

int dm2_v1_viewport_door_button_rect_for_square(int view_square,
                                                DM2_V1_ViewportRect *out_rect)
{
    DM2_V1_ViewportRect panel;
    int rectno;

    if (!out_rect) return 0;
    dm2_v1_viewport_clear_rect(out_rect);
    if (!dm2_v1_viewport_door_panel_rect_for_square(view_square, &panel)) {
        return 0;
    }

    rectno = dm2_v1_viewport_door_button_rectno_for_square(view_square);
    switch (rectno) {
    case 4:
        out_rect->w = 16;
        out_rect->h = 18;
        out_rect->x = panel.x + panel.w - 28;
        out_rect->y = panel.y + (panel.h / 2) - 9;
        return 1;
    case 3:
        out_rect->w = 12;
        out_rect->h = 14;
        out_rect->x = panel.x + panel.w - 22;
        out_rect->y = panel.y + (panel.h / 2) - 7;
        return 1;
    case 2:
        out_rect->w = 8;
        out_rect->h = 9;
        out_rect->x = panel.x + panel.w - 16;
        out_rect->y = panel.y + (panel.h / 2) - 4;
        return 1;
    default:
        return 0;
    }
}

/* Resolve a panel_gdat_index back to the DOORS category/index/field it
 * represents. This mirrors the M11 command builder's material address resolver
 * so the renderer can query the source image size for RAW4 placement. */
static int dm2_v1_viewport_door_panel_material_address(
    int panel_gdat_index,
    int *out_category,
    int *out_index,
    int *out_field)
{
    int packed;
    if (!out_category || !out_index || !out_field) return 0;
    *out_category = DM2_GDAT_CATEGORY_DOORS;
    if (panel_gdat_index <= DM2_V1_VIEWPORT_GFX_DOOR_RECORD_PANEL_FIELD_BASE &&
        panel_gdat_index > DM2_V1_VIEWPORT_GFX_DOOR_ORNATE_FIELD_BASE) {
        packed = DM2_V1_VIEWPORT_GFX_DOOR_RECORD_PANEL_FIELD_BASE -
                 panel_gdat_index;
        *out_index = (packed >> DM2_V1_VIEWPORT_GFX_DOOR_PANEL_INDEX_SHIFT) & 0xff;
        *out_field = packed & DM2_V1_VIEWPORT_GFX_DOOR_PANEL_FIELD_MASK;
    } else {
        *out_index = 0;
        *out_field = DM2_V1_VIEWPORT_GFX_DOOR_PANEL_FIELD_BASE - panel_gdat_index;
    }
    return *out_field >= 0 && *out_field <= 0xff;
}

/* Source-locked closed-panel rectangle from INTERFACE_GENERAL/0/RAW4/0
 * tlbRectnoDoorPosition. Falls back to the hard-coded compatibility rectangle
 * when the source route is unavailable. */
static int dm2_v1_viewport_door_panel_rect_for_square_from_source(
    const DM2_V1_ViewportState *s,
    int view_square,
    int panel_gdat_index,
    DM2_V1_ViewportRect *out_rect)
{
    uint16_t rect_number;
    int category, index, field;
    int width = 0, height = 0;
    uint8_t *pixels;
    int result;

    if (!s || !out_rect) return 0;
    if (!s->source_materials_required || !s->asset_loader) return 0;
    if (!dm2_v1_gdat_door_overlay_panel_rect_number(view_square, &rect_number)) {
        return 0;
    }
    if (!dm2_v1_viewport_door_panel_material_address(
            panel_gdat_index, &category, &index, &field)) {
        return 0;
    }
    pixels = dm2_v1_asset_load_image_field(
        s->asset_loader, category, index, field, &width, &height, NULL);
    if (!pixels || width <= 0 || height <= 0) {
        dm2_v1_asset_free_pixels(pixels);
        return 0;
    }
    result = dm2_v1_gdat_door_overlay_query_raw4_destination_rect(
        s->asset_loader, rect_number, width, height, out_rect);
    dm2_v1_asset_free_pixels(pixels);
    return result;
}

/* Source-locked default door-button rectangle from INTERFACE_GENERAL/0/RAW4/0
 * tlbRectnoDoorButton. Falls back to the hard-coded compatibility rectangle
 * when the source route is unavailable. */
static int dm2_v1_viewport_door_button_rect_for_square_from_source(
    const DM2_V1_ViewportState *s,
    int view_square,
    DM2_V1_ViewportRect *out_rect)
{
    uint16_t rect_number;
    int width = 0, height = 0;
    uint8_t *pixels;
    int result;

    if (!s || !out_rect) return 0;
    if (!s->source_materials_required || !s->asset_loader) return 0;
    if (!dm2_v1_gdat_door_overlay_button_rect_number(view_square, &rect_number)) {
        return 0;
    }
    /* Button placement is the same for released (field 0) and pushed
     * (field 5); use the released image to resolve the source size. */
    pixels = dm2_v1_asset_load_image_field(
        s->asset_loader, DM2_GDAT_CATEGORY_DOOR_BUTTONS, 0, 0,
        &width, &height, NULL);
    if (!pixels || width <= 0 || height <= 0) {
        dm2_v1_asset_free_pixels(pixels);
        return 0;
    }
    result = dm2_v1_gdat_door_overlay_query_raw4_destination_rect(
        s->asset_loader, rect_number, width, height, out_rect);
    dm2_v1_asset_free_pixels(pixels);
    return result;
}

static DM2_V1_ViewportRect dm2_v1_viewport_wall_frame_rect(int view_square)
{
    const DM2_WallFrame *frame = dm2_v1_get_wall_frame(view_square);
    DM2_V1_ViewportRect rect = { 0, 0, 0, 0 };
    if (!frame) {
        return rect;
    }
    rect.x = frame->left_x;
    rect.y = frame->top_y;
    rect.w = frame->right_x - frame->left_x + 1;
    rect.h = frame->bottom_y - frame->top_y + 1;
    return rect;
}

static DM2_V1_ViewportRect dm2_v1_viewport_door_visible_panel_rect(
    const DM2_V1_ViewportRect *panel_rect,
    int door_open_pct)
{
    DM2_V1_ViewportRect rect = { 0, 0, 0, 0 };
    int visible_pct;
    int visible_h;

    if (!panel_rect || panel_rect->w <= 0 || panel_rect->h <= 0) {
        return rect;
    }
    if (door_open_pct < 0) {
        door_open_pct = 0;
    } else if (door_open_pct > 100) {
        door_open_pct = 100;
    }
    visible_pct = 100 - door_open_pct;
    if (visible_pct <= 0) {
        return rect;
    }
    visible_h = (panel_rect->h * visible_pct + 99) / 100;
    if (visible_h <= 0) {
        visible_h = 1;
    } else if (visible_h > panel_rect->h) {
        visible_h = panel_rect->h;
    }
    rect = *panel_rect;
    rect.y = panel_rect->y + (panel_rect->h - visible_h);
    rect.h = visible_h;
    return rect;
}

/* skproject DRAW_DOOR_FRAMES consults the DOORS no-frames word before
 * requesting either side frame. The M11 panel receipt is the sole source for
 * that per-door-type gate; an absent receipt cannot make a frame disappear. */
static int dm2_v1_viewport_door_m11_has_no_frames(
    const DM2_V1_GdatDoorOverlayM11CommandPlan *plan,
    const DM2_V1_DoorRender *door)
{
    if (!plan || !plan->valid || !door) return 0;
    for (int i = 0; i < plan->command_count; ++i) {
        const DM2_V1_GdatDoorOverlayM11Command *command =
            &plan->commands[i];
        if (command->kind == DM2_V1_GDAT_DOOR_PANEL &&
            command->gdat_index == door->panel_gdat_index &&
            command->view_square == door->view_square &&
            command->door_opening_dir == door->door_opening_dir &&
            command->door_state == door->door_state &&
            command->door_open_pct == door->door_open_pct) {
            return command->no_frames != 0u;
        }
    }
    return 0;
}

int dm2_v1_viewport_build_door_render_plan(
    const DM2_V1_ViewportState *s,
    DM2_V1_DoorRenderPlan *out_plan)
{
    if (!out_plan) {
        return 0;
    }
    memset(out_plan, 0, sizeof(*out_plan));
    if (!s) {
        return 1;
    }

    /* Source frames follow DM2_DRAW_DUNGEON_TILES plus the front-player tile.
     * table1d7029 does not list cell zero, but skproject still draws an
     * admitted directly-facing DB0 door through the later player-tile/
     * DRAW_DOOR path. Admit D0C only after runtime has attached that G1 root;
     * all other cells remain bound to the table1d7029 scheduler below. */
    const int source_scheduler =
        s->source_materials_required && s->gdat_scene_control_ready;
    const int iteration_count = source_scheduler
        ? 1 + (int)(sizeof(s_dm2_draw_dungeon_tiles_cells) /
                    sizeof(s_dm2_draw_dungeon_tiles_cells[0]))
        : DM2_SQ_COUNT;
    for (int iteration = 0; iteration < iteration_count; ++iteration) {
        int square = source_scheduler ? -1 : iteration;
        int source_pass = source_scheduler ? iteration : -1;
        const DM2_ViewSquare *vs = NULL;
        DM2_V1_DoorRender *row;
        DM2_V1_ViewportRect panel_rect;

        if (source_scheduler && iteration == 0) {
            square = DM2_SQ_D0C;
            source_pass = -1;
            vs = &s->squares[square];
            if (!vs->door_direct_g1_root) {
                continue;
            }
        } else if (source_scheduler) {
            const int cell = s_dm2_draw_dungeon_tiles_cells[iteration - 1];
            source_pass = iteration - 1;
            for (int candidate = 0; candidate < DM2_SQ_COUNT; ++candidate) {
                if (dm2_v1_viewport_skproject_cell_for_square(candidate) == cell) {
                    square = candidate;
                    break;
                }
            }
            if (square < 0) continue;
            vs = &s->squares[square];
        } else {
            vs = &s->squares[square];
        }

        if (!(vs->flags & DM2_SQF_HAS_DOOR) ||
            out_plan->door_count >= DM2_V1_DOOR_RENDER_MAX) {
            continue;
        }
        /* LOAD_LOCALLEVEL_DYN only admits the two map DoorType slots when
         * Map_definitions::UseDoor0/UseDoor1 says that type is live. A DB0
         * payload alone must not select a same-numbered DOORS image. */
        if (s->source_materials_required && !vs->door_gfx_admitted &&
            !vs->door_wall_button) {
            continue;
        }
        row = &out_plan->doors[out_plan->door_count++];
        row->view_square = square;
        row->skproject_cell = dm2_v1_viewport_skproject_cell_for_square(square);
        row->source_pass = source_pass;
        row->door_record_type = vs->door_record_type;
        row->door_gfx_index = vs->door_gfx_index;
        row->door_gfx_admitted = vs->door_gfx_admitted;
        row->door_opening_dir = vs->door_opening_dir;
        row->ornament_index = vs->ornament_index;
        row->door_ornate_gfx_index = vs->door_ornate_gfx_index;
        row->door_button = vs->door_button;
        row->door_button_state = vs->door_button_state;
        /* skproject DRAW_DOOR/DRAW_DOOR_FRAMES select the map-local DOORS
         * image for every state except destroyed (state 5), where only the
         * frame and destroyed mask are drawn.  States 0..4 use the recorded
         * DoorType/OpeningDir-specific panel only when the G1 root actually
         * admitted a door record (door_record_type != 0).  An unrecorded door
         * falls back to the square panel, regardless of its opening direction.
         * Source: SKWIN/SkWinCore.cpp DM2_DRAW_DOOR uses glbMapDoorType[DoorType()]
         * only after a valid DB0 door record is present. */
        if (vs->door_state <= 4u && vs->door_record_type != 0) {
            row->panel_gdat_index =
                dm2_v1_viewport_door_panel_graphic_index_for_record(
                    square,
                    vs->door_gfx_index,
                    vs->door_opening_dir);
        } else {
            row->panel_gdat_index =
                dm2_v1_viewport_door_panel_graphic_index_for_square(square);
        }
        row->frame_gdat_index =
            dm2_v1_viewport_door_frame_graphic_index_for_square(square);
        for (int side = 0; side < 2; ++side) {
            int field, rect_number, mirror_flip, offset_x, offset_y;
            if (dm2_v1_viewport_door_side_frame_source_for_movement(
                    square, side, s->gdat_scene_movement_active, &field,
                    &rect_number, &mirror_flip, &offset_x, &offset_y)) {
                row->side_frame_graphicsset_field[side] = field;
                row->side_frame_gdat_index[side] =
                    DM2_V1_VIEWPORT_GFX_DOOR_FRAME_FIELD_BASE - field;
                row->side_frame_rect_number[side] = rect_number;
                row->side_frame_mirror_flip[side] = mirror_flip;
                row->side_frame_offset_x[side] = offset_x;
                row->side_frame_offset_y[side] = offset_y;
            }
        }
        /* DRAW_DOOR_FRAMES reads the live MAP graphics set.  A source frame
         * cannot borrow the renderer's historical set-one convenience. */
        row->graphicsset_index = s->gdat_scene_control_ready
            ? s->gdat_scene_material_index : -1;
        row->door_open_pct = vs->door_open_pct;
        row->door_state = vs->door_state;
        row->ornate_gdat_index =
            dm2_v1_viewport_door_ornate_graphic_index(vs->door_ornate_gfx_index,
                                                      square);
        if (vs->door_state == 5) {
            row->destroyed_mask_gdat_index =
                dm2_v1_viewport_door_destroyed_mask_graphic_index(
                    vs->door_gfx_index,
                    square);
        }
        /* skproject SKWINSPX/src/v4/c_gui_vp.cpp::DM2_DRAW_DOOR_FRAMES
         * resolves the panel from its GDAT owner.  It has no generated-colour
         * substitute, so an unresolved panel is removed from the plan below. */
        if (!dm2_v1_viewport_door_panel_rect_for_square_from_source(
                s, square, row->panel_gdat_index, &panel_rect) &&
            !dm2_v1_viewport_door_panel_rect_for_square(square, &panel_rect)) {
            memset(row, 0, sizeof(*row));
            --out_plan->door_count;
            continue;
        }
        row->panel_rect = panel_rect;
        row->panel_visible_rect =
            dm2_v1_viewport_door_visible_panel_rect(&panel_rect,
                                                    row->door_open_pct);
        row->frame_rect = dm2_v1_viewport_wall_frame_rect(square);
        if (vs->door_button || vs->door_wall_button) {
            if (!dm2_v1_viewport_door_button_rect_for_square_from_source(
                    s, square, &row->button_rect)) {
                (void)dm2_v1_viewport_door_button_rect_for_square(
                    square,
                    &row->button_rect);
            }
            if (vs->door_button) {
                row->button_gdat_index =
                    dm2_v1_viewport_door_button_graphic_index_for_state(
                        vs->door_button_state != 0);
                row->button_source_kind = 1;
            } else {
                /* skproject DRAW_DEFAULT_DOOR_BUTTON advances the WALL_GFX field
                 * by one for the pushed variant when the actuator/text record
                 * owns that field.  Without a source receipt for the pushed
                 * field the renderer stays fail-closed instead of inventing a
                 * second button image. */
                int wall_button_field = (int)vs->door_wall_button_field +
                    (int)vs->door_wall_button_state;
                if (wall_button_field > 0xff) wall_button_field = 0xff;
                row->button_gdat_index =
                    dm2_v1_viewport_wall_button_graphic_index(
                        vs->door_wall_button_index,
                        wall_button_field);
                row->button_source_kind = 2;
                row->wall_button_index = vs->door_wall_button_index;
                row->wall_button_field = wall_button_field;
                row->wall_button_x = vs->door_wall_button_x;
                row->wall_button_y = vs->door_wall_button_y;
                row->wall_button_object_id = vs->door_wall_button_object_id;
            }
        }
    }
    return 1;
}

static int dm2_v1_viewport_full_rect_asset_blit(
    int gdat_index,
    const DM2_V1_ViewportRect *dst_rect,
    int src_w,
    int src_h,
    int src_stride,
    DM2_V1_DoorAssetBlit *out_blit)
{
    DM2_V1_DoorAssetBlit blit;

    if (!out_blit) {
        return 0;
    }
    memset(&blit, 0, sizeof(blit));
    blit.gdat_index = -1;
    blit.transparent_color = DM2_COLOR_TRANSPARENT;
    if (gdat_index == 0 || !dst_rect || dst_rect->w <= 0 ||
        dst_rect->h <= 0 || src_w <= 0 || src_h <= 0) {
        *out_blit = blit;
        return 0;
    }
    blit.gdat_index = gdat_index;
    blit.src_rect = (DM2_V1_ViewportRect){ 0, 0, src_w, src_h };
    blit.dst_rect = *dst_rect;
    blit.src_stride = src_stride > 0 ? src_stride : src_w;
    blit.transparent_color = DM2_COLOR_TRANSPARENT;
    *out_blit = blit;
    return 1;
}

int dm2_v1_viewport_door_panel_asset_blit(
    const DM2_V1_DoorRender *render,
    int src_w,
    int src_h,
    int src_stride,
    DM2_V1_DoorAssetBlit *out_blit)
{
    DM2_V1_DoorAssetBlit blit;
    int source_y;
    int source_h;

    if (!out_blit) {
        return 0;
    }
    memset(&blit, 0, sizeof(blit));
    blit.gdat_index = -1;
    blit.transparent_color = DM2_COLOR_TRANSPARENT;
    if (!render || render->panel_gdat_index == 0 ||
        render->panel_rect.w <= 0 || render->panel_rect.h <= 0 ||
        render->panel_visible_rect.w <= 0 ||
        render->panel_visible_rect.h <= 0 ||
        src_w <= 0 || src_h <= 0) {
        *out_blit = blit;
        return 0;
    }

    source_y =
        ((render->panel_visible_rect.y - render->panel_rect.y) * src_h) /
        render->panel_rect.h;
    source_h =
        (render->panel_visible_rect.h * src_h + render->panel_rect.h - 1) /
        render->panel_rect.h;
    if (source_y < 0) source_y = 0;
    if (source_y > src_h) source_y = src_h;
    if (source_h < 1) source_h = 1;
    if (source_y + source_h > src_h) {
        source_h = src_h - source_y;
    }

    blit.gdat_index = render->panel_gdat_index;
    blit.src_rect = (DM2_V1_ViewportRect){ 0, source_y, src_w, source_h };
    blit.dst_rect = render->panel_visible_rect;
    blit.src_stride = src_stride > 0 ? src_stride : src_w;
    blit.transparent_color = DM2_COLOR_TRANSPARENT;
    *out_blit = blit;
    return source_h > 0;
}

int dm2_v1_viewport_door_frame_asset_blit(
    const DM2_V1_DoorRender *render,
    int src_w,
    int src_h,
    int src_stride,
    DM2_V1_DoorAssetBlit *out_blit)
{
    if (!render) {
        if (out_blit) {
            memset(out_blit, 0, sizeof(*out_blit));
            out_blit->gdat_index = -1;
            out_blit->transparent_color = DM2_COLOR_TRANSPARENT;
        }
        return 0;
    }
    return dm2_v1_viewport_full_rect_asset_blit(render->frame_gdat_index,
                                                &render->frame_rect,
                                                src_w,
                                                src_h,
                                                src_stride,
                                                out_blit);
}

int dm2_v1_viewport_door_button_asset_blit(
    const DM2_V1_DoorRender *render,
    int src_w,
    int src_h,
    int src_stride,
    DM2_V1_DoorAssetBlit *out_blit)
{
    if (!render) {
        if (out_blit) {
            memset(out_blit, 0, sizeof(*out_blit));
            out_blit->gdat_index = -1;
            out_blit->transparent_color = DM2_COLOR_TRANSPARENT;
        }
        return 0;
    }
    return dm2_v1_viewport_full_rect_asset_blit(render->button_gdat_index,
                                                &render->button_rect,
                                                src_w,
                                                src_h,
                                                src_stride,
                                                out_blit);
}

/* ── Internal blit helper ─────────────────────────────────────────── */

static void __attribute__((unused)) dm2_blit_bitmap (
    uint8_t *vp,
    int vp_stride,
    const uint8_t *bitmap,
    const DM2_WallFrame *frame,
    int bitmap_stride,
    int flip_horizontal,
    int parity_flip)
{
    if (!vp || !bitmap || !frame) return;
    if (frame->byte_width == 0 || frame->height == 0) return;

    DM2_BlitClipGate gate = dm2_resolve_blit_clip(
        frame, frame->byte_width, frame->height,
        DM2_VP_WIDTH, DM2_VP_HEIGHT);
    if (!gate.visible) return;

    for (int y = 0; y < gate.height; y++) {
        const uint8_t *src_row = bitmap + (gate.src_y + y) * bitmap_stride;
        uint8_t *dst_row = vp + (gate.dst_y + y) * vp_stride;

        for (int x = 0; x < gate.width; x++) {
            int sx = flip_horizontal
                       ? (frame->byte_width - 1 - (gate.src_x + x))
                       : (gate.src_x + x);
            uint8_t pixel = src_row[sx];
            if (pixel != DM2_COLOR_TRANSPARENT) {
                dst_row[gate.dst_x + x] = pixel;
            }
        }
        (void)parity_flip;
    }
}

static int dm2_v1_fetch_viewport_asset(DM2_V1_ViewportState *s,
                                       int gdat_index,
                                       const uint8_t **out_pixels,
                                       int *out_w,
                                       int *out_h,
                                       int *out_stride)
{
    if (out_pixels) *out_pixels = NULL;
    if (out_w) *out_w = 0;
    if (out_h) *out_h = 0;
    if (out_stride) *out_stride = 0;
    if (s) {
        s->active_asset_palette_ready = 0;
        s->active_asset_palette_hash = 0u;
        memset(s->active_asset_palette16, 0,
               sizeof(s->active_asset_palette16));
    }
    if (s && s->asset_fetch &&
        s->asset_fetch(s->asset_user, gdat_index, out_pixels, out_w, out_h,
                       out_stride) == 0) {
        if (s->asset_palette_fetch) {
            if (s->asset_palette_fetch(s->asset_palette_user, gdat_index,
                                       s->active_asset_palette16,
                                       &s->active_asset_palette_hash) != 0 ||
                s->active_asset_palette_hash == 0u) {
                memset(s->active_asset_palette16, 0,
                       sizeof(s->active_asset_palette16));
                s->active_asset_palette_hash = 0u;
                if (s->source_materials_required) return -1;
            } else {
                s->active_asset_palette_ready = 1;
            }
        }
        return 0;
    }
    return dm2_v1_gfx_fetch(gdat_index, out_pixels, out_w, out_h, out_stride);
}

/* skproject SKWIN/SkWinCore.cpp QUERY_DUNGEON_MAP_CHIP_PICT resolves the
 * decoded map-chip image together with QUERY_GDAT_IMAGE_LOCALPAL before
 * DRAW_CHIP_OF_MAGIC_MAP. It is part of material lookup, not an optional
 * presentation transform: source-owned walls, doors, creatures, floor objects,
 * possession and carried items, projectiles, and CHAMPIONS portraits must not
 * borrow INTERFACE_GENERAL when their per-IMG3 receipt is unavailable. */
static int dm2_v1_fetch_viewport_local_material(
    DM2_V1_ViewportState *s,
    int gdat_index,
    const uint8_t **out_pixels,
    int *out_w,
    int *out_h,
    int *out_stride)
{
    if (dm2_v1_fetch_viewport_asset(s, gdat_index, out_pixels, out_w, out_h,
                                    out_stride) != 0) {
        return -1;
    }
    if (s && s->source_materials_required &&
        !s->active_asset_palette_ready) {
        if (out_pixels) *out_pixels = NULL;
        if (out_w) *out_w = 0;
        if (out_h) *out_h = 0;
        if (out_stride) *out_stride = 0;
        return -1;
    }
    return 0;
}

static uint32_t dm2_v1_weather_pixels_hash(const uint8_t *pixels, int width,
                                           int height, int stride)
{
    uint32_t hash = 2166136261u;
    int y;

    if (!pixels || width <= 0 || height <= 0 || stride < width) return 0u;
    for (y = 0; y < height; ++y) {
        int x;
        for (x = 0; x < width; ++x) {
            hash ^= pixels[y * stride + x];
            hash *= 16777619u;
        }
    }
    return hash;
}

static void __attribute__((unused)) dm2_v1_blit_tiled_bitmap(uint8_t *dst,
                                     int dst_stride,
                                     int dst_x,
                                     int dst_y,
                                     int dst_w,
                                     int dst_h,
                                     const uint8_t *src,
                                     int src_w,
                                     int src_h,
                                     int src_stride,
                                     int transparent_color)
{
    int y;

    if (!dst || !src || dst_stride <= 0 || dst_w <= 0 || dst_h <= 0 ||
        src_w <= 0 || src_h <= 0 || src_stride < src_w) {
        return;
    }
    for (y = 0; y < dst_h; ++y) {
        int sy = y % src_h;
        int fy = dst_y + y;
        int x;
        if ((unsigned)fy >= (unsigned)DM2_VP_HEIGHT) continue;
        for (x = 0; x < dst_w; ++x) {
            int sx = x % src_w;
            int fx = dst_x + x;
            uint8_t pixel;
            if ((unsigned)fx >= (unsigned)DM2_VP_WIDTH) continue;
            pixel = src[sy * src_stride + sx];
            if (transparent_color >= 0 &&
                pixel == (uint8_t)transparent_color) {
                continue;
            }
            dst[fy * dst_stride + fx] = pixel;
        }
    }
}

static void __attribute__((unused)) dm2_v1_blit_scaled_bitmap(uint8_t *dst,
                                      int dst_stride,
                                      int dst_x,
                                      int dst_y,
                                      int dst_w,
                                      int dst_h,
                                      const uint8_t *src,
                                      int src_w,
                                      int src_h,
                                      int src_stride,
                                      int transparent_color)
{
    int y;

    if (!dst || !src || dst_stride <= 0 || dst_w <= 0 || dst_h <= 0 ||
        src_w <= 0 || src_h <= 0 || src_stride < src_w) {
        return;
    }
    for (y = 0; y < dst_h; ++y) {
        int sy = (y * src_h) / dst_h;
        int fy = dst_y + y;
        int x;
        if ((unsigned)fy >= (unsigned)DM2_VP_HEIGHT) continue;
        for (x = 0; x < dst_w; ++x) {
            int sx = (x * src_w) / dst_w;
            int fx = dst_x + x;
            uint8_t pixel;
            if ((unsigned)fx >= (unsigned)DM2_VP_WIDTH) continue;
            pixel = src[sy * src_stride + sx];
            if (transparent_color >= 0 &&
                pixel == (uint8_t)transparent_color) {
                continue;
            }
            dst[fy * dst_stride + fx] = pixel;
        }
    }
}

/* skproject QUERY_GDAT_IMAGE_LOCALPAL supplies the per-IMG3 16-byte palette
 * to every dungeon map-chip blit.  The global interface table is only the
 * compatibility fallback used by non-source test providers. */
static uint8_t dm2_v1_material_palette_color(DM2_V1_ViewportState *s,
                                             uint8_t logical_color,
                                             int *consumed_count)
{
    if (!s) {
        return logical_color;
    }
    if (consumed_count) ++*consumed_count;
    if (logical_color >= 16u) {
        return logical_color;
    }
    if (s->gdat_scene_control_ready &&
        (s->gdat_ambient_light != 0u ||
         s->gdat_highest_light_level != 0u ||
         s->gdat_ambient_darkness != 0u)) {
        ++s->gdat_scene_light_consumed_count;
    }
    if (s->active_asset_palette_ready) {
        int is_identity = 1;
        int i;
        for (i = 0; i < 16; ++i) {
            if (s->active_asset_palette16[i] != (uint8_t)i) {
                is_identity = 0;
                break;
            }
        }
        /* 8bpp IMG9 has no 16-color local palette; SUMMARY_IMAGE installs the
         * 256-entry identity translation, so pixel bytes index the global
         * palette directly.  Do not count this as a local-palette consumption. */
        if (is_identity) {
            return logical_color;
        }
        ++s->gdat_local_palette_consumed_count;
        return s->active_asset_palette16[logical_color];
    }
    if (!s->gdat_interface_palette_ready) {
        return logical_color;
    }
    return s->gdat_interface_palette16[logical_color];
}

static uint32_t dm2_v1_viewport_indexed_pixel_hash(const uint8_t *pixels,
                                                    int width,
                                                    int height,
                                                    int stride)
{
    uint32_t hash = 2166136261u;
    int y;

    if (!pixels || width <= 0 || height <= 0 || stride < width) return 0u;
    for (y = 0; y < height; ++y) {
        int x;
        for (x = 0; x < width; ++x) {
            hash ^= pixels[y * stride + x];
            hash *= 16777619u;
        }
    }
    return hash ? hash : 1u;
}

static void dm2_v1_block_source_material(DM2_V1_ViewportState *s,
                                         uint32_t material_bit);

static void dm2_v1_blit_scaled_material_bitmap(DM2_V1_ViewportState *s,
                                                uint8_t *dst,
                                                int dst_stride,
                                                int dst_x,
                                                int dst_y,
                                                int dst_w,
                                                int dst_h,
                                                const uint8_t *src,
                                                int src_w,
                                                int src_h,
                                                int src_stride,
                                                int transparent_color,
                                                int *consumed_count)
{
    int y;
    if (!s || !dst || !src || dst_stride <= 0 || dst_w <= 0 || dst_h <= 0 ||
        src_w <= 0 || src_h <= 0 || src_stride < src_w) return;
    for (y = 0; y < dst_h; ++y) {
        int fy = dst_y + y;
        if ((unsigned)fy >= (unsigned)DM2_VP_HEIGHT) continue;
        for (int x = 0; x < dst_w; ++x) {
            int fx = dst_x + x;
            uint8_t pixel;
            if ((unsigned)fx >= (unsigned)DM2_VP_WIDTH) continue;
            pixel = src[((y * src_h) / dst_h) * src_stride +
                        ((x * src_w) / dst_w)];
            if (transparent_color >= 0 && pixel == (uint8_t)transparent_color) {
                continue;
            }
            dst[fy * dst_stride + fx] =
                dm2_v1_material_palette_color(s, pixel, consumed_count);
        }
    }
}

void dm2_v1_render_teleporter_fields(DM2_V1_ViewportState *s)
{
    static const struct { int x, y, w, h; } placements[DM2_SQ_COUNT] = {
        { 102, 63, 16, 10 }, {  68, 65, 20, 12 }, { 136, 65, 20, 12 },
        {  88, 70, 48, 24 }, {  45, 72, 30, 18 }, { 179, 72, 30, 18 },
        {  74, 78, 76, 38 }, {  34, 82, 42, 25 }, { 192, 82, 42, 25 },
        {  53, 90,118, 58 }, {  13, 96, 54, 34 }, { 203, 96, 54, 34 }
    };
    const uint8_t *pixels = NULL;
    int width = 0, height = 0, stride = 0;
    int frame_width, frame_count, frame_index;
    int found = 0;
    const int gdat_index = dm2_v1_viewport_teleporter_map_chip_graphic_index();

    if (!s || !s->framebuffer || s->is_outdoor) return;
    for (int i = 0; i < DM2_SQ_COUNT; ++i) {
        if (s->squares[i].square_type == DM2_SQUARE_TELEPORTER) {
            found = 1;
            break;
        }
    }
    if (!found) return;

    /* SKProject DRAW_MAP_CHIP selects TELEPORTERS/0/F9 and advances its
     * horizontal map-chip frame with the live game tick. */
    if (dm2_v1_fetch_viewport_local_material(
            s, gdat_index, &pixels, &width, &height, &stride) != 0 ||
        !pixels || width <= 0 || height <= 0 || stride < width ||
        (frame_count = dm2_v1_viewport_map_chip_frame_count(width, height)) <= 0 ||
        (frame_width = dm2_v1_viewport_map_chip_frame_width(width, height)) <= 0) {
        dm2_v1_block_source_material(
            s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_TELEPORTER);
        return;
    }
    frame_index = dm2_v1_viewport_map_chip_frame_index(
        s->tick_count, frame_count);
    for (int i = 0; i < DM2_SQ_COUNT; ++i) {
        if (s->squares[i].square_type != DM2_SQUARE_TELEPORTER) continue;
        dm2_v1_blit_scaled_material_bitmap(
            s, s->framebuffer, s->fb_stride,
            placements[i].x, placements[i].y, placements[i].w, placements[i].h,
            pixels + frame_index * frame_width, frame_width, height, stride,
            DM2_COLOR_TRANSPARENT, &s->gdat_sprite_palette_consumed_count);
        ++s->asset_teleporter_drawn_count;
    }
}

static void dm2_v1_blit_tiled_material_bitmap(DM2_V1_ViewportState *s,
                                               uint8_t *dst,
                                               int dst_stride,
                                               int dst_x,
                                               int dst_y,
                                               int dst_w,
                                               int dst_h,
                                               const uint8_t *src,
                                               int src_w,
                                               int src_h,
                                               int src_stride,
                                               int transparent_color,
                                               int mirror_flip,
                                               int *consumed_count)
{
    int y;
    if (!s || !dst || !src || dst_stride <= 0 || dst_w <= 0 || dst_h <= 0 ||
        src_w <= 0 || src_h <= 0 || src_stride < src_w) return;
    for (y = 0; y < dst_h; ++y) {
        int fy = dst_y + y;
        if ((unsigned)fy >= (unsigned)DM2_VP_HEIGHT) continue;
        for (int x = 0; x < dst_w; ++x) {
            int fx = dst_x + x;
            uint8_t pixel;
            if ((unsigned)fx >= (unsigned)DM2_VP_WIDTH) continue;
            {
                int sx = x % src_w;
                if (mirror_flip) sx = src_w - 1 - sx;
                pixel = src[(y % src_h) * src_stride + sx];
            }
            if (transparent_color >= 0 && pixel == (uint8_t)transparent_color) {
                continue;
            }
            dst[fy * dst_stride + fx] =
                dm2_v1_material_palette_color(s, pixel, consumed_count);
        }
    }
}

static int dm2_v1_scene_plane_flip_from_position(
    const DM2_V1_ViewportState *s, uint8_t kind)
{
    int64_t parity;

    if (!s) return 0;
    /* SKProject SkWinCore.cpp SET_GRAPHICS_FLIP_FROM_POSITION (32CB:59CA).
     * DISPLAY_VIEWPORT uses kind 0x20 for ceiling rect 700 and 1 for floor
     * rect 701; no other caller is admitted by this plane route. */
    parity = (int64_t)s->party_x + s->party_y + s->party_dir +
        s->gdat_scene_map_offset_x + s->gdat_scene_map_offset_y +
        s->dungeon_level;
    parity &= 1;
    if (kind == 1u) {
        if ((s->gdat_scene_flags & 8u) != 0u) {
            if ((s->gdat_scene_flags & 0x10u) != 0u)
                return (s->tick_count & 7) > 3;
            return (int)parity;
        }
        if ((s->gdat_scene_flags & 0x40u) != 0u)
            return (s->party_dir & 1) != 0;
        return 0;
    }
    if (kind == 0x20u) {
        if ((s->gdat_scene_flags & 2u) != 0u) {
            if ((s->gdat_scene_flags & 4u) != 0u)
                return (s->tick_count & 7) <= 3;
            return !parity;
        }
        if ((s->gdat_scene_flags & 0x20u) != 0u)
            return (s->party_dir & 1) != 0;
        return 0;
    }
    return (int)parity;
}

static void dm2_v1_blit_scaled_material_bitmap_region(
    DM2_V1_ViewportState *s, uint8_t *dst, int dst_stride, int dst_x,
    int dst_y, int dst_w, int dst_h, const uint8_t *src, int src_x,
    int src_y, int src_w, int src_h, int src_stride, int transparent_color,
    int *consumed_count)
{
    int y;
    if (!s || !dst || !src || dst_stride <= 0 || dst_w <= 0 || dst_h <= 0 ||
        src_w <= 0 || src_h <= 0 || src_stride <= 0) return;
    for (y = 0; y < dst_h; ++y) {
        int fy = dst_y + y;
        int sy = src_y + (y * src_h) / dst_h;
        if ((unsigned)fy >= (unsigned)DM2_VP_HEIGHT) continue;
        for (int x = 0; x < dst_w; ++x) {
            int fx = dst_x + x;
            int sx = src_x + (x * src_w) / dst_w;
            uint8_t pixel;
            if ((unsigned)fx >= (unsigned)DM2_VP_WIDTH || sx < 0 || sy < 0 ||
                sx >= src_stride) continue;
            pixel = src[sy * src_stride + sx];
            if (transparent_color >= 0 && pixel == (uint8_t)transparent_color) {
                continue;
            }
            dst[fy * dst_stride + fx] =
                dm2_v1_material_palette_color(s, pixel, consumed_count);
        }
    }
}

/* skproject/SKWIN/SkWinCore.cpp routes map-chip and HUD sprites through the
 * same dtPalIRGB/dtPalette16 binding as dungeon materials. Keep this as a
 * distinct primitive so ordinary fallback pixels cannot be counted as GDAT
 * palette consumption. */
static void dm2_v1_blit_scaled_material_bitmap_region_ex(
    DM2_V1_ViewportState *s, uint8_t *dst, int dst_stride, int dst_x,
    int dst_y, int dst_w, int dst_h, const uint8_t *src, int src_x,
    int src_y, int src_w, int src_h, int src_stride, int transparent_color,
    int flip_mirror, int *consumed_count)
{
    if (!s || !dst || !src || dst_stride <= 0 || dst_w <= 0 || dst_h <= 0 ||
        src_w <= 0 || src_h <= 0 || src_stride <= 0) return;
    for (int y = 0; y < dst_h; ++y) {
        int fy = dst_y + y;
        int sy = src_y + ((flip_mirror & 2)
            ? src_h - 1 - ((y * src_h) / dst_h) : (y * src_h) / dst_h);
        if ((unsigned)fy >= (unsigned)DM2_VP_HEIGHT) continue;
        for (int x = 0; x < dst_w; ++x) {
            int rx = (x * src_w) / dst_w;
            int sx = src_x + ((flip_mirror & 1) ? src_w - 1 - rx : rx);
            int fx = dst_x + x;
            uint8_t pixel;
            if ((unsigned)fx >= (unsigned)DM2_VP_WIDTH || sx < 0 || sy < 0 ||
                sx >= src_stride) continue;
            pixel = src[sy * src_stride + sx];
            if (transparent_color >= 0 && pixel == (uint8_t)transparent_color) {
                continue;
            }
            dst[fy * dst_stride + fx] =
                dm2_v1_material_palette_color(s, pixel, consumed_count);
        }
    }
}

static int dm2_v1_prepare_map_chip_frame(int src_w,
                                         int src_h,
                                         int requested_frame,
                                         int *out_frame_x,
                                         int *out_frame_w)
{
    int frame_w = dm2_v1_viewport_map_chip_frame_width(src_w, src_h);
    int frame_count = dm2_v1_viewport_map_chip_frame_count(src_w, src_h);
    int frame_index = dm2_v1_viewport_map_chip_frame_index(requested_frame,
                                                          frame_count);
    if (frame_w <= 0 || frame_count <= 0) return 0;
    if (out_frame_x) *out_frame_x = frame_index * frame_w;
    if (out_frame_w) *out_frame_w = frame_w;
    return 1;
}

static int dm2_v1_prepare_projectile_map_chip_frame(int src_w,
                                                    int src_h,
                                                    int requested_frame,
                                                    int projectile_direction,
                                                    int object_direction,
                                                    int party_direction,
                                                    int frame_class,
                                                    int *out_frame_x,
                                                    int *out_frame_w)
{
    int frame_w = dm2_v1_viewport_map_chip_frame_width(src_w, src_h);
    int frame_count = dm2_v1_viewport_map_chip_frame_count(src_w, src_h);
    int frame_index = dm2_v1_viewport_projectile_frame_for_map_chip(
        requested_frame,
        projectile_direction,
        object_direction,
        party_direction,
        frame_count,
        frame_class);
    if (frame_w <= 0 || frame_count <= 0) return 0;
    if (out_frame_x) *out_frame_x = frame_index * frame_w;
    if (out_frame_w) *out_frame_w = frame_w;
    return 1;
}

static int dm2_v1_viewport_scaled_sprite_extent(int src_extent,
                                                int depth,
                                                int min_extent,
                                                int max_extent)
{
    int scale_pct;
    int extent = src_extent;
    if (extent <= 0) return 0;
    if (depth <= 0) {
        scale_pct = 100;
    } else if (depth == 1) {
        scale_pct = 80;
    } else if (depth == 2) {
        scale_pct = 62;
    } else if (depth == 3) {
        scale_pct = 48;
    } else {
        scale_pct = 36;
    }
    extent = (extent * scale_pct + 50) / 100;
    if (extent < min_extent) extent = min_extent;
    if (extent > max_extent) extent = max_extent;
    return extent;
}

int dm2_v1_viewport_build_creature_render_plan(
    const DM2_V1_ViewportState *s,
    DM2_V1_CreatureRenderPlan *out_plan)
{
    if (!out_plan) {
        return 0;
    }
    memset(out_plan, 0, sizeof(*out_plan));
    if (!s) {
        return 1;
    }

    /* skproject SKWIN/SkWinCore.cpp DRAW_TEMP_PICST/QUERY_DUNGEON_MAP_CHIP_PICT
     * selects a creature map-chip image before DRAW_CHIP_OF_MAGIC_MAP scales it
     * at the visible-cell center. Keep only source-owned image identity and
     * Rect14 geometry in the renderer-owned plan. */
    for (int i = 0; i < s->creature_count &&
                    i < DM2_MAX_CREATURES_PER_SQ; ++i) {
        const DM2_CreatureSprite *src = &s->creatures[i];
        DM2_V1_CreatureRender *row;

        if (src->screen_x < 0 || src->screen_x >= DM2_VP_WIDTH ||
            src->screen_y < 0 || src->screen_y >= DM2_VP_HEIGHT ||
            out_plan->creature_count >= DM2_MAX_CREATURES_PER_SQ) {
            continue;
        }

        row = &out_plan->creatures[out_plan->creature_count++];
        row->creature_index = i;
        row->creature_type = src->creature_type;
        row->source_kind = src->source_kind;
        row->object_id = src->object_id;
        row->frame_index = src->frame_index;
        row->material_frame_index = src->frame_index;
        row->direction = src->direction;
        row->depth = src->depth;
        row->map_x = src->map_x;
        row->map_y = src->map_y;
        row->center_x = src->screen_x;
        row->center_y = src->screen_y;
        if (src->source_kind == 1) {
            /* QUERY_CREATURE_PICST consumes the FD-selected dtImage field
             * directly. A live creature without that original selection
             * gets no drawable key, so the source-material gate blocks it. */
            if (src->source_material_proven) {
                row->gdat_index = dm2_v1_viewport_creature_field_graphic_index(
                    src->creature_type, src->gdat_image_field);
                row->material_frame_index = 0;
            }
        } else if (src->source_kind == 2) {
            if (src->source_v5_field && src->source_material_proven) {
                /* The real FB/FC/FD V5 chain selected this record's dtImage
                 * field: draw CREATURES/type/field, never the F9 map chip. */
                row->gdat_index = dm2_v1_viewport_creature_field_graphic_index(
                    src->creature_type, src->gdat_image_field);
                row->material_frame_index = 0;
                row->source_v5_field = 1;
            } else {
                row->gdat_index = dm2_v1_viewport_creature_graphic_index(
                    src->creature_type, src->frame_index);
            }
        }
        /* _4976_5aa4 occupancy evidence: when a Rect14 row governs this
         * creature, its [0] anchor rotated by (party_dir - dir) gives the
         * occupancy 5x5 position; the display-order index follows the source
         * DRAW_STATIC_OBJECT walk.  The runtime pass reorders by that index
         * only when every row is proven. */
        row->occupancy_5x5 = -1;
        row->occupancy_display_index = -1;
        row->source_pass = -1;
        if (src->source_kind != 0 &&
            (src->source_kind != 1 || src->source_material_proven) &&
            s->gdat_interface_rect14_rows &&
            src->frame_index < s->gdat_interface_rect14_row_count) {
            const uint8_t *rect14 = s->gdat_interface_rect14_rows +
                ((size_t)src->frame_index * 14u);
            int relative_direction = (s->party_dir - src->direction) & 3;
            uint8_t image_field = rect14[2 + relative_direction];

            /* skproject SkWinCore.cpp QUERY_CREATURE_PICST (32CB:28C7)
             * selects the creature dtImage field from this row. */
            if (rect14[0] <= 24u && image_field != 0xffu) {
                int occ_cell;
                int occ_pass;

                row->gdat_index = dm2_v1_viewport_creature_field_graphic_index(
                    src->creature_type, image_field);
                row->rect14_applied = 1;
                row->rect14_scale64 = rect14[6 + relative_direction];
                row->rect14_lateral_offset = (int8_t)rect14[1];
                row->rect14_flip_mirror = rect14[10 + relative_direction] & 1u;
                /* QUERY_CREATURE_5x5_POS: the row anchor rotated into view
                 * space is the creature's occupancy position. */
                row->occupancy_5x5 =
                    dm2_v1_viewport_creature_occupancy_5x5(
                        rect14[0], src->direction, s->party_dir);
                if (row->occupancy_5x5 >= 0 &&
                    dm2_v1_viewport_static_object_cell_for_map(
                        src->map_x, src->map_y, s->party_dir, s->party_x,
                        s->party_y, &occ_cell, &occ_pass)) {
                    row->occupancy_display_index =
                        dm2_v1_viewport_static_object_display_index(
                            occ_cell, row->occupancy_5x5);
                    row->source_pass = occ_pass;
                }
            }
        }
    }
    /* DRAW_STATIC_OBJECT interleaves creatures into its tlbDisplayOrder*
     * walk at their _4976_5aa4 occupancy slot.  Reorder the pass by
     * (source pass, occupancy display index) only when every creature row
     * carries proven occupancy evidence; otherwise keep the existing order
     * fail-closed. */
    if (out_plan->creature_count > 1) {
        int all_proven = 1;
        for (int i = 0; i < out_plan->creature_count; ++i) {
            if (out_plan->creatures[i].occupancy_5x5 < 0 ||
                out_plan->creatures[i].occupancy_display_index < 0 ||
                out_plan->creatures[i].source_pass < 0) {
                all_proven = 0;
                break;
            }
        }
        if (all_proven) {
            for (int i = 0; i < out_plan->creature_count; ++i) {
                for (int j = i + 1; j < out_plan->creature_count; ++j) {
                    DM2_V1_CreatureRender *a = &out_plan->creatures[i];
                    DM2_V1_CreatureRender *b = &out_plan->creatures[j];
                    if (b->source_pass < a->source_pass ||
                        (b->source_pass == a->source_pass &&
                         b->occupancy_display_index <
                             a->occupancy_display_index)) {
                        DM2_V1_CreatureRender swap = *a;
                        *a = *b;
                        *b = swap;
                    }
                }
            }
        }
    }
    return 1;
}

int dm2_v1_viewport_creature_asset_blit(
    const DM2_V1_CreatureRender *render,
    int src_w,
    int src_h,
    int src_stride,
    int party_direction,
    DM2_V1_CreatureAssetBlit *out_blit)
{
    DM2_V1_CreatureAssetBlit blit;
    int frame_x = 0;
    int frame_w = src_w;
    int frame_count;
    int render_frame = 0;
    int dst_w;
    int dst_h;

    if (!out_blit) {
        return 0;
    }
    memset(&blit, 0, sizeof(blit));
    blit.gdat_index = -1;
    blit.transparent_color = DM2_COLOR_TRANSPARENT;
    if (!render || render->gdat_index == 0 || src_w <= 0 || src_h <= 0) {
        *out_blit = blit;
        return 0;
    }

    frame_count = dm2_v1_viewport_map_chip_frame_count(src_w, src_h);
    if (render->rect14_applied) {
        int scale64 = render->rect14_scale64;
        if (scale64 <= 0) scale64 = 64;
        frame_count = 1;
        dst_w = dm2_v1_viewport_calc_stretched_size(src_w, scale64);
        dst_h = dm2_v1_viewport_calc_stretched_size(src_h, scale64);
    } else {
        render_frame = dm2_v1_viewport_creature_frame_for_direction(
            render->material_frame_index,
            render->direction,
            party_direction,
            frame_count);
        if (!dm2_v1_prepare_map_chip_frame(src_w, src_h,
                                           render_frame,
                                           &frame_x,
                                           &frame_w)) {
            frame_x = 0;
            frame_w = src_w;
            render_frame = 0;
        }
        dst_w = dm2_v1_viewport_scaled_sprite_extent(frame_w,
                                                      render->depth,
                                                      8,
                                                      64);
        dst_h = dm2_v1_viewport_scaled_sprite_extent(src_h,
                                                      render->depth,
                                                      8,
                                                      64);
    }

    blit.gdat_index = render->gdat_index;
    blit.frame_x = frame_x;
    blit.frame_y = 0;
    blit.frame_w = frame_w;
    blit.frame_h = src_h;
    blit.dst_rect.x = render->center_x - (dst_w / 2);
    blit.dst_rect.y = render->center_y - (dst_h / 2);
    if (render->rect14_applied && render->rect14_lateral_offset != 0) {
        int offset = render->rect14_lateral_offset;
        int relative_direction = (party_direction - render->direction) & 3;
        if (relative_direction == 0) {
            blit.dst_rect.x += dm2_v1_viewport_calc_stretched_size(-7, offset);
        } else if (relative_direction == 2) {
            blit.dst_rect.x += dm2_v1_viewport_calc_stretched_size(7, offset);
        } else {
            blit.dst_rect.y += dm2_v1_viewport_calc_stretched_size(-64, offset);
        }
    }
    blit.dst_rect.w = dst_w;
    blit.dst_rect.h = dst_h;
    blit.src_stride = src_stride > 0 ? src_stride : src_w;
    blit.transparent_color = DM2_COLOR_TRANSPARENT;
    blit.flip_mirror = render->rect14_applied ? render->rect14_flip_mirror : 0;
    blit.render_frame = render_frame;
    blit.draw_order = render->creature_index;
    *out_blit = blit;
    return frame_w > 0 && dst_w > 0 && dst_h > 0;
}

int dm2_v1_viewport_build_item_render_plan(
    const DM2_V1_ViewportState *s,
    DM2_V1_ItemRenderPlan *out_plan)
{
    if (!out_plan) {
        return 0;
    }
    memset(out_plan, 0, sizeof(*out_plan));
    if (!s) {
        return 1;
    }

    /* skproject SKWIN/SkWinCore.cpp DRAW_MAP_CHIP/DRAW_TEMP_PICST routes
     * floor possessions through QUERY_DUNGEON_MAP_CHIP_PICT before the
     * scaled draw call. The record-owned category is mandatory; source-frame
     * clipping still depends on the fetched bitmap dimensions and is resolved
     * in the blit pass. */
    for (int i = 0; i < s->item_count && i < DM2_MAX_ITEMS_PER_SQ; ++i) {
        const DM2_ItemSprite *src = &s->items[i];
        DM2_V1_ItemRender *row;
        int category;

        if (src->screen_x < 0 || src->screen_x >= DM2_VP_WIDTH ||
            src->screen_y < 0 || src->screen_y >= DM2_VP_HEIGHT ||
            out_plan->item_count >= DM2_MAX_ITEMS_PER_SQ) {
            continue;
        }

        category = src->item_category;
        row = &out_plan->items[out_plan->item_count++];
        row->item_index = i;
        row->item_category = category;
        row->item_type = src->item_type;
        row->frame_index = src->frame_index;
        row->direction = src->direction;
        row->depth = src->depth;
        row->center_x = src->screen_x;
        row->center_y = src->screen_y;
        row->gdat_index = dm2_v1_viewport_item_graphic_index(
            category,
            src->item_type,
            src->source_gdat_field ? src->source_gdat_field : src->frame_index);
        row->object_id = src->object_id;
        row->map_x = src->map_x;
        row->map_y = src->map_y;
        row->source_gdat_field = src->source_gdat_field;
        row->source_g1_weapon = src->source_g1_weapon;
        row->source_g1_container = src->source_g1_container;
        row->source_static_object_admitted =
            src->source_static_object_admitted;
        row->source_static_object_cell = src->source_static_object_cell;
        row->source_static_object_pass = src->source_static_object_pass;
        row->source_static_object_clip_rect_id = src->source_static_object_clip_rect_id;
        row->source_static_object_raw_gfx256_hash = src->source_static_object_raw_gfx256_hash;
        row->source_static_object_raw_gfx256_receipt_hash = src->source_static_object_raw_gfx256_receipt_hash;
        row->source_static_object_raw4_hash = src->source_static_object_raw4_hash;
        row->source_static_object_raw4_receipt_hash = src->source_static_object_raw4_receipt_hash;
        /* skproject SKWINSPX/src/v4/skcore.cpp::DRAW_ITEM draws only the
         * resolved GDAT map-chip.  This plan deliberately carries no
         * generated colour or radius substitute. */

        /* skproject SKWIN/SkWinCore.cpp DRAW_ITEM/QUERY_CREATURE_PICST consumes
         * INTERFACE_GENERAL dt07/0x0A Rect14 rows for source placement.  Static
         * objects already carry Rect14 through their own delivery plan; do not
         * duplicate that route here. */
        if (!src->source_static_object_admitted &&
            s->gdat_interface_rect14_rows &&
            src->frame_index < s->gdat_interface_rect14_row_count) {
            const uint8_t *rect14 = s->gdat_interface_rect14_rows +
                ((size_t)src->frame_index * 14u);
            int relative_direction = (s->party_dir - src->direction) & 3;
            uint8_t image_field = rect14[2 + relative_direction];

            if (rect14[0] <= 24u && image_field != 0xffu) {
                uint32_t placement_hash;

                row->gdat_index = dm2_v1_viewport_item_graphic_index(
                    category, src->item_type, image_field);
                row->rect14_applied = 1;
                row->rect14_scale64 = (int)rect14[6 + relative_direction];
                row->rect14_lateral_offset = (int8_t)rect14[1];
                row->rect14_flip_mirror = (int)(rect14[10 + relative_direction] & 1u);
                row->rect14_row_hash = dm2_v1_wall_hash_bytes(
                    2166136261u, rect14, 14u);
                placement_hash = row->rect14_row_hash;
                placement_hash = dm2_v1_wall_hash_bytes(
                    placement_hash,
                    (const uint8_t *)&row->rect14_scale64,
                    sizeof(row->rect14_scale64));
                placement_hash = dm2_v1_wall_hash_bytes(
                    placement_hash,
                    (const uint8_t *)&row->rect14_flip_mirror,
                    sizeof(row->rect14_flip_mirror));
                placement_hash = dm2_v1_wall_hash_bytes(
                    placement_hash,
                    (const uint8_t *)&row->rect14_lateral_offset,
                    sizeof(row->rect14_lateral_offset));
                row->rect14_placement_hash = placement_hash ? placement_hash : 1u;
            }
        }

        /* Source-owned DRAW_ITEM placement for admitted DB5/DB9 static
         * objects when no INTERFACE_GENERAL Rect14 row governs the row.  The
         * placement is re-derived from the admitted cell/pass/clip route with
         * the same source tables the runtime delivery plan used; a clip-rect
         * mismatch keeps the row fail-closed.  The bound image field owns the
         * chest open state (F4 = open) and draw_slot stays 0: the runtime only
         * admits tile chain heads, and the source chain walk draws the head of
         * a matching direction group first (DRAW_PUT_DOWN_ITEM si == 0). */
        if (src->source_static_object_admitted && !row->rect14_applied &&
            row->source_static_object_pass >= 0 &&
            row->source_static_object_clip_rect_id != 0) {
            DM2_V1_StaticObjectSourcePlan source_plan;

            if (dm2_v1_viewport_static_object_source_plan(
                    row->source_static_object_cell,
                    row->source_static_object_pass,
                    row->item_category, row->direction,
                    row->source_gdat_field == 4, 0,
                    s->party_dir,
                    (uint16_t)(row->item_index + 1),
                    dm2_v1_viewport_static_object_visibility_bit(
                        row->direction, s->party_dir),
                    &source_plan) &&
                (int)(source_plan.clip_rect_id & 0x7fffu) ==
                    row->source_static_object_clip_rect_id) {
                row->source_static_object_placement_valid = 1;
                row->source_static_object_stretch_factor64 =
                    source_plan.stretch_factor64;
                row->source_static_object_slot_x_offset =
                    source_plan.slot_x_offset;
                row->source_static_object_slot_y_offset =
                    source_plan.slot_y_offset;
                row->source_static_object_position_5x5 =
                    source_plan.position_5x5;
                row->source_static_object_image_field =
                    source_plan.image_field;
                row->source_static_object_flip_mirror =
                    source_plan.flip_mirror;
                row->source_static_object_image_offset =
                    src->source_static_object_image_offset;
            }
        }
    }

    /* DM2_DRAW_DUNGEON_TILES runs static objects inside table1d7029's
     * source pass.  Preserve that ordering for the exact centre-line cells
     * that the runtime can prove; unrelated item kinds retain their own
     * source call order. */
    for (int i = 0; i < out_plan->item_count; ++i) {
        for (int j = i + 1; j < out_plan->item_count; ++j) {
            DM2_V1_ItemRender *a = &out_plan->items[i];
            DM2_V1_ItemRender *b = &out_plan->items[j];
            if (a->source_static_object_admitted &&
                b->source_static_object_admitted &&
                b->source_static_object_pass < a->source_static_object_pass) {
                DM2_V1_ItemRender swap = *a;
                *a = *b;
                *b = swap;
            }
        }
    }
    return 1;
}

int dm2_v1_viewport_build_carried_item_render_plan(
    const DM2_V1_ViewportState *s,
    DM2_V1_CarriedItemRenderPlan *out_plan)
{
    const DM2_ItemSprite *src;
    DM2_V1_ItemRender *row;
    int category;

    if (!out_plan) {
        return 0;
    }
    memset(out_plan, 0, sizeof(*out_plan));
    if (!s || !s->carried_item_present) {
        return 1;
    }
    src = &s->carried_item;
    if (src->screen_x < 0 || src->screen_x >= DM2_VP_WIDTH ||
        src->screen_y < 0 || src->screen_y >= DM2_VP_HEIGHT) {
        return 1;
    }

    /* skproject SKWIN/SkWinCore.cpp DRAW_ITEM_IN_HAND selects the carried
     * object's GDAT identity before drawing the cursor buffer. A missing
     * original category cannot be normalized to a miscellaneous icon. */
    category = src->item_category;
    row = &out_plan->item;
    out_plan->item_present = 1;
    row->item_index = 0;
    row->item_category = category;
    row->item_type = src->item_type;
    row->frame_index = src->frame_index;
    row->direction = src->direction;
    row->depth = src->depth;
    row->center_x = src->screen_x;
    row->center_y = src->screen_y;
    row->gdat_index = dm2_v1_viewport_item_graphic_index(
        category,
        src->item_type,
        src->frame_index);
    return 1;
}

int dm2_v1_viewport_build_creature_possession_item_render_plan(
    const DM2_V1_ViewportState *s,
    DM2_V1_CreaturePossessionItemRenderPlan *out_plan)
{
    if (!out_plan) {
        return 0;
    }
    memset(out_plan, 0, sizeof(*out_plan));
    if (!s) {
        return 1;
    }

    /* skproject SKWIN/SkWinCore.cpp DRAW_MAP_CHIP lines 10782-10817 draws
     * source-visible creature possessions after the creature with frame 0
     * and the _4976_3fa4 object-direction flip table. */
    for (int i = 0;
         i < s->creature_possession_item_count &&
             i < DM2_MAX_CREATURE_POSSESSION_ITEMS;
         ++i) {
        const DM2_ItemSprite *src = &s->creature_possession_items[i];
        DM2_V1_ItemRender *row;
        int category;

        if (src->screen_x < 0 || src->screen_x >= DM2_VP_WIDTH ||
            src->screen_y < 0 || src->screen_y >= DM2_VP_HEIGHT ||
            out_plan->item_count >= DM2_MAX_CREATURE_POSSESSION_ITEMS) {
            continue;
        }

        category = src->item_category;
        row = &out_plan->items[out_plan->item_count++];
        row->item_index = i;
        row->item_category = category;
        row->item_type = src->item_type;
        row->frame_index = 0;
        row->direction = src->direction;
        row->depth = src->depth;
        row->center_x = src->screen_x;
        row->center_y = src->screen_y;
        row->gdat_index = dm2_v1_viewport_item_graphic_index(
            category,
            src->item_type,
            src->source_gdat_field ? src->source_gdat_field : 0);
        row->object_id = src->object_id;
        row->map_x = src->map_x;
        row->map_y = src->map_y;
        row->source_g1_weapon = src->source_g1_weapon;
        row->source_g1_container = src->source_g1_container;
        row->flip_mirror =
            dm2_v1_viewport_map_chip_flip_for_object_direction(
                src->direction,
                s->party_dir);
    }
    return 1;
}

int dm2_v1_viewport_item_asset_blit(
    const DM2_V1_ItemRender *render,
    int src_w,
    int src_h,
    int src_stride,
    int party_direction,
    int scale_base,
    int scale_max,
    DM2_V1_ItemAssetBlit *out_blit)
{
    DM2_V1_ItemAssetBlit blit;
    int frame_x = 0;
    int frame_w = src_w;
    int frame_count;
    int render_frame = 0;
    int dst_w;
    int dst_h;

    if (!out_blit) {
        return 0;
    }
    memset(&blit, 0, sizeof(blit));
    blit.gdat_index = -1;
    blit.transparent_color = DM2_COLOR_TRANSPARENT;
    if (!render || render->gdat_index == 0 ||
        src_w <= 0 || src_h <= 0 ||
        scale_base <= 0 || scale_max <= 0) {
        *out_blit = blit;
        return 0;
    }

    frame_count = dm2_v1_viewport_map_chip_frame_count(src_w, src_h);
    if (render->rect14_applied) {
        int scale64 = render->rect14_scale64;
        if (scale64 <= 0) scale64 = 64;
        frame_count = 1;
        dst_w = dm2_v1_viewport_calc_stretched_size(src_w, scale64);
        dst_h = dm2_v1_viewport_calc_stretched_size(src_h, scale64);
    } else if (render->source_static_object_placement_valid) {
        /* skproject SKWIN/SkWinCore.cpp DRAW_ITEM scales the selected image
         * by _4976_418e[y_distance][1 + vertical_row] via QUERY_TEMP_PICST;
         * CALC_STRETCHED_SIZE gives the destination extent. */
        int scale64 = render->source_static_object_stretch_factor64;
        if (scale64 <= 0) scale64 = 64;
        frame_count = 1;
        dst_w = dm2_v1_viewport_calc_stretched_size(frame_w, scale64);
        dst_h = dm2_v1_viewport_calc_stretched_size(src_h, scale64);
    } else {
        render_frame = dm2_v1_viewport_map_chip_frame_index(render->frame_index,
                                                            frame_count);
        if (!dm2_v1_prepare_map_chip_frame(src_w, src_h,
                                           render_frame,
                                           &frame_x,
                                           &frame_w)) {
            frame_x = 0;
            frame_w = src_w;
            render_frame = 0;
        }

        dst_w = dm2_v1_viewport_scaled_sprite_extent(frame_w,
                                                      render->depth,
                                                      scale_base,
                                                      scale_max);
        dst_h = dm2_v1_viewport_scaled_sprite_extent(src_h,
                                                      render->depth,
                                                      scale_base,
                                                      scale_max);
    }

    blit.gdat_index = render->gdat_index;
    blit.frame_x = frame_x;
    blit.frame_y = 0;
    blit.frame_w = frame_w;
    blit.frame_h = src_h;
    blit.dst_rect.x = render->center_x - (dst_w / 2);
    blit.dst_rect.y = render->center_y - (dst_h / 2);
    if (render->rect14_applied && render->rect14_lateral_offset != 0) {
        int offset = render->rect14_lateral_offset;
        int relative_direction = (party_direction - render->direction) & 3;
        /* skproject QUERY_CREATURE_PICST/DRAW_ITEM apply a signed lateral
         * offset to the destination before blitting. */
        if (relative_direction == 0) {
            blit.dst_rect.x += dm2_v1_viewport_calc_stretched_size(-7, offset);
        } else if (relative_direction == 2) {
            blit.dst_rect.x += dm2_v1_viewport_calc_stretched_size(7, offset);
        } else {
            blit.dst_rect.y += dm2_v1_viewport_calc_stretched_size(-64, offset);
        }
    } else if (render->source_static_object_placement_valid) {
        /* skproject SKWIN/SkWinCore.cpp DRAW_ITEM lines 23961-23977: the draw
         * slot adds _4976_41de[_4976_41b0[slot][0]] to the x anchor (offx ->
         * ExtendedPicture.w28) and _4976_41de[_4976_41b0[slot][1]] to the y
         * anchor (offy -> w30); the record's dtImageOffset then adds its
         * signed high byte to x and its signed low byte to y.  These are
         * direct pixel deltas, not stretched values. */
        blit.dst_rect.x += render->source_static_object_slot_x_offset +
            (int)(int8_t)(render->source_static_object_image_offset >> 8);
        blit.dst_rect.y += render->source_static_object_slot_y_offset +
            (int)(int8_t)(render->source_static_object_image_offset & 0xff);
    }
    blit.dst_rect.w = dst_w;
    blit.dst_rect.h = dst_h;
    blit.src_stride = src_stride > 0 ? src_stride : src_w;
    blit.transparent_color = DM2_COLOR_TRANSPARENT;
    blit.flip_mirror = render->rect14_applied
        ? render->rect14_flip_mirror
        : render->source_static_object_placement_valid
            ? render->source_static_object_flip_mirror
            : render->flip_mirror;
    blit.render_frame = render_frame;
    blit.draw_order = render->item_index;
    *out_blit = blit;
    return frame_w > 0 && dst_w > 0 && dst_h > 0;
}

int dm2_v1_viewport_build_projectile_render_plan(
    const DM2_V1_ViewportState *s,
    DM2_V1_ProjectileRenderPlan *out_plan)
{
    if (!out_plan) {
        return 0;
    }
    memset(out_plan, 0, sizeof(*out_plan));
    if (!s) {
        return 1;
    }

    /* skproject SKWIN/SkWinCore.cpp DRAW_TEMP_PICST and DRAW_MAP_CHIP first
     * resolve missile/cloud map-chip identity, direction class, and mirror
     * before DRAW_CHIP_OF_MAGIC_MAP handles bitmap dimensions. Clouds still
     * draw their random mirror in the blit pass so the runtime seed advances
     * exactly when an asset-backed cloud is actually rendered. */
    for (int i = 0; i < s->projectile_count &&
                    i < DM2_MAX_PROJECTILES; ++i) {
        const DM2_Projectile *src = &s->projectiles[i];
        DM2_V1_ProjectileRender *row;
        int category;

        if (src->screen_x < 0 || src->screen_x >= DM2_VP_WIDTH ||
            src->screen_y < 0 || src->screen_y >= DM2_VP_HEIGHT ||
            out_plan->projectile_count >= DM2_MAX_PROJECTILES) {
            continue;
        }

        category = src->projectile_category;
        row = &out_plan->projectiles[out_plan->projectile_count++];
        row->projectile_index = i;
        row->projectile_category = category;
        row->projectile_type = src->projectile_type;
        row->frame_index = src->frame_index;
        row->render_frame = src->frame_index;
        row->direction = src->direction;
        row->object_direction = src->object_direction;
        row->frame_class = src->frame_class;
        row->render_kind = src->render_kind;
        row->depth = src->depth;
        row->center_x = src->screen_x;
        row->center_y = src->screen_y;
        row->gdat_index = dm2_v1_viewport_projectile_graphic_index(
            category,
            src->projectile_type,
            src->frame_index);
        row->flip_mirror = dm2_v1_viewport_projectile_flip_for_direction(
            src->direction,
            s->party_dir);
        row->cloud_flip_from_seed =
            (src->render_kind == DM2_V1_PROJECTILE_RENDER_CLOUD);
        /* skproject SKWINSPX/src/v4/skcore.cpp::DRAW_TEMP_PICST resolves a
         * source map-chip and placement; missing material is no-draw. */

        /* skproject SKWIN/SkWinCore.cpp DRAW_TEMP_PICST may consume
         * INTERFACE_GENERAL dt07/0x0A Rect14 rows for source placement when the
         * runtime has bound the table.  Clouds keep their random mirror path. */
        if (s->gdat_interface_rect14_rows &&
            src->frame_index < s->gdat_interface_rect14_row_count) {
            const uint8_t *rect14 = s->gdat_interface_rect14_rows +
                ((size_t)src->frame_index * 14u);
            int relative_direction = (s->party_dir - src->direction) & 3;
            uint8_t image_field = rect14[2 + relative_direction];

            if (rect14[0] <= 24u && image_field != 0xffu) {
                uint32_t placement_hash;

                row->gdat_index = dm2_v1_viewport_projectile_graphic_index(
                    category, src->projectile_type, image_field);
                row->rect14_applied = 1;
                row->rect14_scale64 = (int)rect14[6 + relative_direction];
                row->rect14_lateral_offset = (int8_t)rect14[1];
                row->rect14_flip_mirror = (int)(rect14[10 + relative_direction] & 1u);
                row->rect14_row_hash = dm2_v1_wall_hash_bytes(
                    2166136261u, rect14, 14u);
                placement_hash = row->rect14_row_hash;
                placement_hash = dm2_v1_wall_hash_bytes(
                    placement_hash,
                    (const uint8_t *)&row->rect14_scale64,
                    sizeof(row->rect14_scale64));
                placement_hash = dm2_v1_wall_hash_bytes(
                    placement_hash,
                    (const uint8_t *)&row->rect14_flip_mirror,
                    sizeof(row->rect14_flip_mirror));
                placement_hash = dm2_v1_wall_hash_bytes(
                    placement_hash,
                    (const uint8_t *)&row->rect14_lateral_offset,
                    sizeof(row->rect14_lateral_offset));
                row->rect14_placement_hash = placement_hash ? placement_hash : 1u;
            }
        }

    }
    return 1;
}

int dm2_v1_viewport_projectile_asset_blit(
    const DM2_V1_ProjectileRender *render,
    int src_w,
    int src_h,
    int src_stride,
    int party_direction,
    int tick_count,
    uint32_t *random_seed,
    DM2_V1_ProjectileAssetBlit *out_blit)
{
    DM2_V1_ProjectileAssetBlit blit;
    int frame_x = 0;
    int frame_w = src_w;
    int render_frame;
    int flip_mirror;
    int dst_w;
    int dst_h;

    if (!out_blit) {
        return 0;
    }
    memset(&blit, 0, sizeof(blit));
    blit.gdat_index = -1;
    blit.transparent_color = DM2_COLOR_TRANSPARENT;
    if (!render || render->gdat_index == 0 ||
        src_w <= 0 || src_h <= 0) {
        *out_blit = blit;
        return 0;
    }

    render_frame = render->frame_index;
    flip_mirror = render->flip_mirror;
    if (random_seed) {
        blit.random_seed_before = *random_seed;
        blit.random_seed_after = *random_seed;
    }
    if (!dm2_v1_prepare_projectile_map_chip_frame(src_w,
                                                  src_h,
                                                  render->frame_index,
                                                  render->direction,
                                                  render->object_direction,
                                                  party_direction,
                                                  render->frame_class,
                                                  &frame_x,
                                                  &frame_w)) {
        frame_x = 0;
        frame_w = src_w;
    }

    if (render->render_kind == DM2_V1_PROJECTILE_RENDER_CLOUD) {
        int frame_count = dm2_v1_viewport_map_chip_frame_count(src_w, src_h);
        if (render->cloud_flip_from_seed && !render->rect14_applied) {
            flip_mirror = dm2_v1_viewport_cloud_flip_for_seed(random_seed);
            if (random_seed) {
                blit.random_seed_after = *random_seed;
            }
        }
        render_frame = dm2_v1_viewport_cloud_frame_for_tick(tick_count,
                                                            frame_count);
        if (!dm2_v1_prepare_map_chip_frame(src_w, src_h, render_frame,
                                           &frame_x, &frame_w)) {
            frame_x = 0;
            frame_w = src_w;
        }
    }

    if (render->rect14_applied) {
        int scale64 = render->rect14_scale64;
        if (scale64 <= 0) scale64 = 64;
        dst_w = dm2_v1_viewport_calc_stretched_size(frame_w, scale64);
        dst_h = dm2_v1_viewport_calc_stretched_size(src_h, scale64);
    } else {
        dst_w = dm2_v1_viewport_scaled_sprite_extent(frame_w,
                                                      render->depth,
                                                      3,
                                                      32);
        dst_h = dm2_v1_viewport_scaled_sprite_extent(src_h,
                                                      render->depth,
                                                      3,
                                                      32);
    }

    blit.gdat_index = render->gdat_index;
    blit.frame_x = frame_x;
    blit.frame_y = 0;
    blit.frame_w = frame_w;
    blit.frame_h = src_h;
    blit.dst_rect.x = render->center_x - (dst_w / 2);
    blit.dst_rect.y = render->center_y - (dst_h / 2);
    if (render->rect14_applied && render->rect14_lateral_offset != 0) {
        int offset = render->rect14_lateral_offset;
        int relative_direction = (party_direction - render->direction) & 3;
        /* skproject QUERY_CREATURE_PICST/DRAW_TEMP_PICST apply a signed lateral
         * offset to the destination before blitting. */
        if (relative_direction == 0) {
            blit.dst_rect.x += dm2_v1_viewport_calc_stretched_size(-7, offset);
        } else if (relative_direction == 2) {
            blit.dst_rect.x += dm2_v1_viewport_calc_stretched_size(7, offset);
        } else {
            blit.dst_rect.y += dm2_v1_viewport_calc_stretched_size(-64, offset);
        }
    }
    blit.dst_rect.w = dst_w;
    blit.dst_rect.h = dst_h;
    blit.src_stride = src_stride > 0 ? src_stride : src_w;
    blit.transparent_color = DM2_COLOR_TRANSPARENT;
    blit.flip_mirror = render->rect14_applied
        ? render->rect14_flip_mirror
        : flip_mirror;
    blit.render_frame = render_frame;
    blit.draw_order = render->projectile_index;
    *out_blit = blit;
    return frame_w > 0 && dst_w > 0 && dst_h > 0;
}

int dm2_v1_viewport_build_weather_overlay_render_plan(
    const DM2_V1_ViewportState *s,
    DM2_V1_WeatherOverlayRenderPlan *out_plan)
{
    int stride2;

    if (!out_plan) {
        return 0;
    }
    memset(out_plan, 0, sizeof(*out_plan));
    if (!s || s->weather <= 0 || s->rain_intensity <= 0) {
        return 1;
    }

    out_plan->kind = (DM2_V1_WeatherOverlayKind)s->weather;
    out_plan->intensity = s->rain_intensity;
    out_plan->streak_step = 3;
    out_plan->rain_color = DM2_COL_WHITE;
    out_plan->fog_target_color = DM2_COL_BLACK;
    out_plan->lightning_color = DM2_COL_WHITE;

    /* skproject SKWIN outdoor weather resolves overlay density and animated
     * scroll from the weather/tick state before the blitline_48-style pass.
     * Keep those render decisions in a DM2-owned plan; the pass below only
     * applies the already-bound overlay command. */
    if (s->weather == DM2_V1_WEATHER_OVERLAY_RAIN) {
        out_plan->density = (s->rain_intensity + 9) / 10;
        stride2 = s->rain_intensity / 5;
        out_plan->scroll = (s->tick_count * stride2) & 7;
    } else if (s->weather == DM2_V1_WEATHER_OVERLAY_FOG) {
        out_plan->alpha = (s->rain_intensity + 7) / 8;
    } else if (s->weather == DM2_V1_WEATHER_OVERLAY_STORM) {
        out_plan->density = (s->rain_intensity + 5) / 10;
        stride2 = s->rain_intensity / 4;
        out_plan->scroll = (s->tick_count * stride2) & 7;
        out_plan->lightning_flash = ((s->tick_count % 120) < 2);
    } else {
        out_plan->kind = DM2_V1_WEATHER_OVERLAY_NONE;
    }
    return 1;
}

int dm2_v1_viewport_build_weather_overlay_commands(
    const DM2_V1_WeatherOverlayRenderPlan *plan,
    DM2_V1_WeatherOverlayCommandPlan *out_commands)
{
    DM2_V1_WeatherOverlayCommand *cmd;

    if (!out_commands) {
        return 0;
    }
    memset(out_commands, 0, sizeof(*out_commands));
    if (!plan || plan->kind == DM2_V1_WEATHER_OVERLAY_NONE) {
        return 1;
    }

    if (plan->kind == DM2_V1_WEATHER_OVERLAY_RAIN ||
        plan->kind == DM2_V1_WEATHER_OVERLAY_STORM) {
        cmd = &out_commands->commands[out_commands->command_count++];
        cmd->kind = DM2_V1_WEATHER_COMMAND_RAIN_STREAKS;
        cmd->density = plan->density;
        cmd->scroll = plan->scroll;
        cmd->streak_step = plan->streak_step;
        cmd->color = plan->rain_color;
    }

    if (plan->kind == DM2_V1_WEATHER_OVERLAY_FOG) {
        cmd = &out_commands->commands[out_commands->command_count++];
        cmd->kind = DM2_V1_WEATHER_COMMAND_FOG_BLEND;
        cmd->alpha = plan->alpha;
        cmd->target_color = plan->fog_target_color;
    }

    if (plan->kind == DM2_V1_WEATHER_OVERLAY_STORM &&
        plan->lightning_flash &&
        out_commands->command_count < DM2_V1_WEATHER_OVERLAY_COMMAND_MAX) {
        cmd = &out_commands->commands[out_commands->command_count++];
        cmd->kind = DM2_V1_WEATHER_COMMAND_LIGHTNING_FILL;
        cmd->color = plan->lightning_color;
    }
    return 1;
}

/* ── Populate view squares from world model ─────────────────────── */

/*
 * dm2_populate_view_squares —
 *   Fill the 12 view squares from world model given party position/direction.
 *
 * For each of the 12 view squares (D3L, D3R, D3C, D2L, D2R, D2C,
 * D1L, D1R, D1C, D0L, D0R, D0C), compute the dungeon grid coordinate
 * and fetch tile data from the world model.
 *
 * DM2 has an outdoor mode where the view is fundamentally different.
 * For indoor dungeon mode, we use the same 3×4 grid projection as DM1.
 *
 * Source: SKULL.ASM T560 (dungeon viewport projection)
 *         DUNGEON.C:1371-1421 (map coordinate resolution)
 *         DM2 uses: 16-byte map descriptor with width/height override fields
 */
static void __attribute__((unused)) dm2_populate_view_squares (
    DM2_V1_ViewportState *s,
    const dm2_dungeon_world_t *world)
{
    if (!s) return;

    /* Direction vectors: N=0, E=1, S=2, W=3 */
    static const int dx[4] = {  0,  1,  0, -1 };
    static const int dy[4] = { -1,  0,  1,  0 };

    int dir = s->party_dir & 3;
    int px  = s->party_x;
    int py  = s->party_y;

    /* Per-square relative offsets (lateral = left, right of facing dir).
     * Depth 3 (D3): 4 squares ahead + 1 ahead = 5 ahead, ±2 lateral
     * Depth 2 (D2): 3 squares ahead, ±2 lateral
     * Depth 1 (D1): 2 squares ahead, ±1 lateral
     * Depth 0 (D0): 1 square ahead, ±1 lateral
     *
     * The lateral offset uses the perpendicular direction.
     * Source: DUNGEON.C:1371-1421, DUNVIEW.C:8318-8542 */
    static const struct {
        int depth;
        int lateral;  /* -2 = far-left, -1 = left, 0 = center, 1 = right, 2 = far-right */
        int fwd;      /* forward steps from party */
    } s_square_rel[DM2_SQ_COUNT] = {
        /* D3L */ { 3, -2, 5 },  /* far-left back row */
        /* D3R */ { 3,  2, 5 },  /* far-right back row */
        /* D3C */ { 3,  0, 5 },  /* center back row */
        /* D2L */ { 2, -2, 3 },  /* left mid row */
        /* D2R */ { 2,  2, 3 },  /* right mid row */
        /* D2C */ { 2,  0, 3 },  /* center mid row */
        /* D1L */ { 1, -1, 2 },  /* left near row */
        /* D1R */ { 1,  1, 2 },  /* right near row */
        /* D1C */ { 1,  0, 2 },  /* center near row */
        /* D0L */ { 0, -1, 1 },  /* immediate left */
        /* D0R */ { 0,  1, 1 },  /* immediate right */
        /* D0C */ { 0,  0, 1 },  /* immediate front */
    };

    /* Perpendicular direction index: (dir + 1) % 4 for left, (dir + 3) % 4 for right */
    int perp_dir[5] = { (dir + 1) & 3, (dir + 3) & 3, dir, dir, dir };

    for (int i = 0; i < DM2_SQ_COUNT; i++) {
        const int sq = s_square_rel[i].depth;
        const int lat = s_square_rel[i].lateral;
        const int fwd = s_square_rel[i].fwd;

        /* Resolve grid coordinate: party_pos + fwd*forward_dir + lat*perp_dir */
        int lat_idx = (lat < 0) ? (2 + lat) : lat; /* -2→0, -1→1, 0→2, 1→3, 2→4 */
        int gx = px + dx[dir] * fwd + dx[perp_dir[lat_idx]] * (lat < 0 ? -lat : lat);
        int gy = py + dy[dir] * fwd + dy[perp_dir[lat_idx]] * (lat < 0 ? -lat : lat);

        DM2_ViewSquare *vs = &s->squares[i];
        memset(vs, 0, sizeof(*vs));
        vs->square_type = DM2_SQUARE_FLOOR;
        vs->flags = DM2_SQF_NONE;
        vs->sprite_depth = sq;

        /* Fetch tile from world model if available */
        if (world && s->dungeon_level < world->map_count) {
            int tt = dm2_world_get_tile_type(world, s->dungeon_level, gx, gy);
            vs->square_type = (uint8_t)(tt < DM2_SQUARE_COUNT ? tt : DM2_SQUARE_FLOOR);
            vs->wall_parity = (sq & 1);  /* alternate wall sets for visual variety */

            /* Populate square flags based on tile type */
            if (vs->square_type == DM2_SQUARE_WALL)
                vs->flags |= DM2_SQF_HAS_WALL;
            else if (vs->square_type == DM2_SQUARE_DOOR)
                vs->flags |= (DM2_SQF_HAS_DOOR | DM2_SQF_HAS_WALL);
            else if (vs->square_type == DM2_SQUARE_FLOOR_ORNATE)
                vs->flags |= DM2_SQF_HAS_FLOOR_ORNAMENT;
            else if (vs->square_type == DM2_SQUARE_SECRET_DOOR)
                vs->flags |= (DM2_SQF_HAS_DOOR | DM2_SQF_HAS_WALL | DM2_SQF_TRANSPARENT_WALL);
            else if (vs->square_type == DM2_SQUARE_FLOOR)
                vs->flags |= DM2_SQF_NONE;

            /* DM2 uses a dynamic per-square light value. The active G1
             * GRAPHICSSET receipt owns only ambient control words, not the
             * later c_light per-square result. A source-required frame must
             * leave it unavailable rather than invent fully-lit tiles. */
            vs->light_level = s->source_materials_required ? 0u : 15u;
        }

        (void)gx; (void)gy; /* reserved for world model integration */
    }
}

/* ── Background ─────────────────────────────────────────────────── */

void dm2_v1_render_background(DM2_V1_ViewportState *s)
{
    if (!s || !s->framebuffer) return;
    uint8_t *vp = s->framebuffer;
    int stride = s->fb_stride;

    /* DM2 black area: top 37 lines, all black.
     * Source: DUNVIEW.C F0098 (line 2968), DM1 black area same height. */
    for (int y = DM2_BLACK_AREA_TOP; y < DM2_BLACK_AREA_TOP + DM2_BLACK_AREA_H; y++) {
        memset(vp + y * stride, DM2_COL_BLACK, (size_t)DM2_VP_WIDTH);
    }
}

/* ── Floor and ceiling ───────────────────────────────────────────── */

static void dm2_v1_block_source_material(DM2_V1_ViewportState *s,
                                         uint32_t material_mask);

void dm2_v1_render_floor_ceiling(DM2_V1_ViewportState *s)
{
    typedef struct {
        const uint8_t *pixels;
        int width;
        int height;
        int stride;
        uint8_t palette16[16];
        uint32_t palette_hash;
        int ready;
    } DM2_V1_ScenePlaneMaterial;
    enum {
        DM2_SCENE_PLANE_FLOOR = 1u << 0,
        DM2_SCENE_PLANE_CEILING = 1u << 1
    };
    if (!s || !s->framebuffer) return;
    uint8_t *vp = s->framebuffer;
    int stride = s->fb_stride;
    const uint8_t *ceiling_pixels = NULL;
    const uint8_t *floor_pixels = NULL;
    int ceiling_w = 0;
    int ceiling_h_src = 0;
    int ceiling_stride = 0;
    int floor_w = 0;
    int floor_h_src = 0;
    int floor_stride = 0;
    int ceiling_gdat_index = dm2_v1_viewport_scene_material_graphic_index(
        s->gdat_scene_material_index,
        DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_CEILING);
    int floor_gdat_index = dm2_v1_viewport_scene_material_graphic_index(
        s->gdat_scene_material_index,
        DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_FLOOR);
    DM2_V1_ScenePlaneMaterial ceiling_material = { 0 };
    DM2_V1_ScenePlaneMaterial floor_material = { 0 };
    int ceiling_asset;
    int floor_asset;
    /* SKProject SkWinCore.cpp DRAW_DUNGEON_GRAPHIC applies these initialized
     * _4976_00fa/_4976_00fc displacements only when glbIsPlayerMoving is
     * nonzero, and only for rect 700/701. Do not admit arbitrary offsets. */
    const int movement_ceiling_y = s->gdat_scene_movement_active ? -2 : 0;
    const int movement_floor_y = s->gdat_scene_movement_active ? 3 : 0;
    const int ceiling_mirror = dm2_v1_scene_plane_flip_from_position(s, 0x20u);
    const int floor_mirror = dm2_v1_scene_plane_flip_from_position(s, 1u);
    uint8_t ceiling_trim = 0u;
    uint8_t floor_trim = 0u;

    s->last_floor_ceiling_material_required_mask = 0u;
    s->last_floor_ceiling_material_consumed_mask = 0u;
    s->gdat_scene_draw_order_consumed_count = 0;
    if (s->source_materials_required) {
        const DM2_V1_GdatSceneM11CommandPlan *plan =
            s->gdat_scene_material_plan;
        int graphicsset_index = 0;
        int material_field = 0;

        s->last_floor_ceiling_material_required_mask =
            DM2_SCENE_PLANE_FLOOR | DM2_SCENE_PLANE_CEILING;
        if (!s->gdat_scene_control_ready ||
            s->gdat_scene_material_plan_rejected ||
            (!plan && (!s->asset_fetch || !s->asset_palette_fetch))) {
            dm2_v1_block_source_material(
                s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING);
            return;
        }
        /* skproject c_gui_vp consumes the decoded UPDATE_GFXSET result. When
         * M11 has that exact boot-owned plan, do not re-query GDAT or permit a
         * callback to substitute another graphics set. */
        if (plan) {
            const DM2_V1_GdatSceneM11Command *floor = &plan->commands[0];
            const DM2_V1_GdatSceneM11Command *ceiling = &plan->commands[1];
            const DM2_V1_GdatSceneBlitRect *floor_rect = &plan->rects[0];
            const DM2_V1_GdatSceneBlitRect *ceiling_rect = &plan->rects[1];
            if (!s->gdat_scene_control_ready ||
                plan->graphicsset != (uint8_t)s->gdat_scene_material_index ||
                plan->command_hash != s->gdat_scene_control_hash ||
                plan->highest_light_level != s->gdat_highest_light_level ||
                plan->ambient_darkness != s->gdat_ambient_darkness ||
                floor->field != DM2_GDAT_GFXSET_FLOOR ||
                ceiling->field != DM2_GDAT_GFXSET_CEIL ||
                !floor->pixels || !ceiling->pixels ||
                floor->width == 0u || floor->height == 0u ||
                ceiling->width == 0u || ceiling->height == 0u ||
                floor->decoded_hash == 0u || ceiling->decoded_hash == 0u ||
                floor->palette_hash == 0u || ceiling->palette_hash == 0u) {
                dm2_v1_block_source_material(
                    s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING);
                return;
            }
            if ((floor->palette_light_receipt_hash != 0u ||
                 ceiling->palette_light_receipt_hash != 0u) &&
                (!s->gdat_c_light_receipt_ready ||
                 floor->palette_light_receipt_hash == 0u ||
                 ceiling->palette_light_receipt_hash == 0u ||
                 floor->palette_light_receipt_hash !=
                     s->gdat_c_light_receipt_hash ||
                 ceiling->palette_light_receipt_hash !=
                     s->gdat_c_light_receipt_hash ||
                 floor->palette_transform_hash == 0u ||
                 ceiling->palette_transform_hash == 0u)) {
                dm2_v1_block_source_material(
                    s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING);
                return;
            }
            if (floor->decoded_hash !=
                    dm2_v1_gdat_scene_m11_command_pixel_hash(floor) ||
                ceiling->decoded_hash !=
                    dm2_v1_gdat_scene_m11_command_pixel_hash(ceiling) ||
                floor->palette_hash !=
                    dm2_v1_weather_pixels_hash(floor->palette16, 16, 1, 16) ||
                ceiling->palette_hash !=
                    dm2_v1_weather_pixels_hash(ceiling->palette16, 16, 1, 16) ||
                floor->geometry_hash == 0u || ceiling->geometry_hash == 0u ||
                floor->geometry_hash !=
                    dm2_v1_gdat_scene_m11_command_geometry_hash(floor, floor_rect) ||
                ceiling->geometry_hash !=
                    dm2_v1_gdat_scene_m11_command_geometry_hash(
                        ceiling, ceiling_rect)) {
                dm2_v1_block_source_material(
                    s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING);
                return;
            }
            /* SKProject c_gui_vp.cpp DRAW_DUNGEON_GRAPHIC submits rect 700
             * for ceiling then 701 for floor through QUERY_BLIT_RECT. The
             * decoded planes are usable only while the source table and both
             * program rows remain bound to this plan. */
            if (!plan->query_blit_rect.valid ||
                plan->query_blit_rect.floor_rect_number !=
                    DM2_V1_GDAT_SCENE_FLOOR_RECT_NUMBER ||
                plan->query_blit_rect.ceiling_rect_number !=
                    DM2_V1_GDAT_SCENE_CEILING_RECT_NUMBER ||
                plan->query_blit_rect.table_hash == 0u ||
                plan->query_blit_rect.floor_row_hash == 0u ||
                plan->query_blit_rect.ceiling_row_hash == 0u ||
                plan->query_blit_rect_hash == 0u ||
                plan->query_blit_rect_hash !=
                    dm2_v1_gdat_scene_query_blit_rect_hash(
                        &plan->query_blit_rect)) {
                dm2_v1_block_source_material(
                    s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING);
                return;
            }
            if (!dm2_v1_gdat_scene_m11_command_plan_draw_order_valid(plan)) {
                dm2_v1_block_source_material(
                    s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING);
                return;
            }
            /* c_gui_vp.cpp::DM2_DISPLAY_VIEWPORT: a fully blocked D1
             * cluster overrides D2 and submits GRAPHICSSET 0x70; otherwise a
             * blocked D2 cluster submits 0x71. TRIM_BLIT_RECT consumes the
             * low byte before ceiling and high byte before floor. */
            if ((s->squares[DM2_SQ_D1L].flags & DM2_SQF_HAS_WALL) &&
                (s->squares[DM2_SQ_D1C].flags & DM2_SQF_HAS_WALL) &&
                (s->squares[DM2_SQ_D1R].flags & DM2_SQF_HAS_WALL)) {
                /* QUERY_GDAT_ENTRY_DATA_INDEX yields source zero when the
                 * optional trim word is absent; DISPLAY_VIEWPORT passes that
                 * zero straight to TRIM_BLIT_RECT. */
                ceiling_trim = (uint8_t)plan->trim_wall_d1;
                floor_trim = (uint8_t)(plan->trim_wall_d1 >> 8);
            } else if ((s->squares[DM2_SQ_D2L].flags & DM2_SQF_HAS_WALL) &&
                       (s->squares[DM2_SQ_D2C].flags & DM2_SQF_HAS_WALL) &&
                       (s->squares[DM2_SQ_D2R].flags & DM2_SQF_HAS_WALL)) {
                ceiling_trim = (uint8_t)plan->trim_wall_d2;
                floor_trim = (uint8_t)(plan->trim_wall_d2 >> 8);
            }
            if (floor_rect->rect_number != DM2_V1_GDAT_SCENE_FLOOR_RECT_NUMBER ||
                ceiling_rect->rect_number != DM2_V1_GDAT_SCENE_CEILING_RECT_NUMBER ||
                floor_rect->width != floor->width || floor_rect->height != floor->height ||
                ceiling_rect->width != ceiling->width ||
                ceiling_rect->height != ceiling->height ||
                floor_rect->x < 0 || floor_rect->y < 0 ||
                ceiling_rect->x < 0 || ceiling_rect->y < 0 ||
                (unsigned)floor_rect->x + floor_rect->width > DM2_VP_WIDTH ||
                (unsigned)floor_rect->y + floor_rect->height > DM2_VP_HEIGHT ||
                (unsigned)ceiling_rect->x + ceiling_rect->width > DM2_VP_WIDTH ||
                (unsigned)ceiling_rect->y + ceiling_rect->height > DM2_VP_HEIGHT) {
                dm2_v1_block_source_material(
                    s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING);
                return;
            }
            floor_material.pixels = floor->pixels;
            floor_material.width = floor->width;
            floor_material.height = floor->height;
            floor_material.stride = floor->width;
            memcpy(floor_material.palette16, floor->palette16,
                   sizeof(floor_material.palette16));
            floor_material.palette_hash = floor->palette_hash;
            floor_material.ready = 1;
            ceiling_material.pixels = ceiling->pixels;
            ceiling_material.width = ceiling->width;
            ceiling_material.height = ceiling->height;
            ceiling_material.stride = ceiling->width;
            memcpy(ceiling_material.palette16, ceiling->palette16,
                   sizeof(ceiling_material.palette16));
            ceiling_material.palette_hash = ceiling->palette_hash;
            ceiling_material.ready = 1;
            ceiling_pixels = ceiling_material.pixels;
            ceiling_w = ceiling_material.width;
            ceiling_h_src = ceiling_material.height;
            ceiling_stride = ceiling_material.stride;
            memcpy(s->active_asset_palette16, ceiling_material.palette16,
                   sizeof(s->active_asset_palette16));
            s->active_asset_palette_hash = ceiling_material.palette_hash;
            s->active_asset_palette_ready = 1;
            ceiling_asset = 1;
        } else {
            /* Legacy callback route for focused renderer tests. The live M11
             * runtime supplies the validated UPDATE_GFXSET plan above. */
        if (!dm2_v1_viewport_scene_material_graphic_address(
                ceiling_gdat_index, &graphicsset_index, &material_field) ||
            graphicsset_index != s->gdat_scene_material_index ||
            material_field != DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_CEILING ||
            dm2_v1_fetch_viewport_local_material(
                s, ceiling_gdat_index, &ceiling_material.pixels,
                &ceiling_material.width, &ceiling_material.height,
                &ceiling_material.stride) != 0 ||
            !ceiling_material.pixels || ceiling_material.width <= 0 ||
            ceiling_material.height <= 0 || !s->active_asset_palette_ready ||
            s->active_asset_palette_hash == 0u) {
            dm2_v1_block_source_material(
                s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING);
            return;
        }
        memcpy(ceiling_material.palette16, s->active_asset_palette16,
               sizeof(ceiling_material.palette16));
        ceiling_material.palette_hash = s->active_asset_palette_hash;
        ceiling_material.ready = 1;

        if (!dm2_v1_viewport_scene_material_graphic_address(
                floor_gdat_index, &graphicsset_index, &material_field) ||
            graphicsset_index != s->gdat_scene_material_index ||
            material_field != DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_FLOOR ||
            dm2_v1_fetch_viewport_local_material(
                s, floor_gdat_index, &floor_material.pixels,
                &floor_material.width, &floor_material.height,
                &floor_material.stride) != 0 ||
            !floor_material.pixels || floor_material.width <= 0 ||
            floor_material.height <= 0 || !s->active_asset_palette_ready ||
            s->active_asset_palette_hash == 0u) {
            dm2_v1_block_source_material(
                s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING);
            return;
        }
        memcpy(floor_material.palette16, s->active_asset_palette16,
               sizeof(floor_material.palette16));
        floor_material.palette_hash = s->active_asset_palette_hash;
        floor_material.ready = 1;
        ceiling_pixels = ceiling_material.pixels;
        ceiling_w = ceiling_material.width;
        ceiling_h_src = ceiling_material.height;
        ceiling_stride = ceiling_material.stride;
        memcpy(s->active_asset_palette16, ceiling_material.palette16,
               sizeof(s->active_asset_palette16));
        s->active_asset_palette_hash = ceiling_material.palette_hash;
        s->active_asset_palette_ready = ceiling_material.ready;
        ceiling_asset = 1;
        }
    } else {
        ceiling_asset = dm2_v1_fetch_viewport_asset(
                            s, ceiling_gdat_index, &ceiling_pixels,
                            &ceiling_w, &ceiling_h_src, &ceiling_stride) == 0 &&
            ceiling_pixels && ceiling_w > 0 && ceiling_h_src > 0;
    }

    /* DM2 uses the same floor (G2108=-1) and ceiling (G2109=-2) indices as
     * DM1. Source: ReDMCSB DUNVIEW.C:126-127. */

    int ceiling_x = 0;
    int ceiling_y = movement_ceiling_y;
    int ceiling_h = DM2_CEILING_H;
    int ceiling_w_dst = DM2_VP_WIDTH;
    if (s->source_materials_required && s->gdat_scene_material_plan) {
        const DM2_V1_GdatSceneBlitRect *rect =
            &s->gdat_scene_material_plan->rects[1];
        ceiling_x = rect->x;
        ceiling_y = rect->y + movement_ceiling_y;
        ceiling_w_dst = rect->width;
        ceiling_h = rect->height;
    }
    if (ceiling_trim >= ceiling_h) {
        dm2_v1_block_source_material(s,
            DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING);
        return;
    }
    ceiling_h -= ceiling_trim;
    if (ceiling_asset) {
        dm2_v1_blit_tiled_material_bitmap(s,
                                 vp,
                                 stride,
                                 ceiling_x,
                                 ceiling_y,
                                 ceiling_w_dst,
                                 ceiling_h,
                                 ceiling_pixels,
                                 ceiling_w,
                                 ceiling_h_src,
                                 ceiling_stride > 0 ? ceiling_stride : ceiling_w,
                                 -1,
                                 ceiling_mirror,
                                 &s->gdat_material_palette_floor_ceiling_consumed_count);
        ++s->asset_floor_ceiling_drawn_count;
        ++s->gdat_scene_material_consumed_count;
        if (s->source_materials_required) {
            ++s->gdat_scene_draw_order_consumed_count;
        }
        if (s->source_materials_required) {
            s->last_floor_ceiling_material_consumed_mask |=
                DM2_SCENE_PLANE_CEILING;
        }
    } else {
        dm2_v1_block_source_material(
            s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING);
    }

    int floor_x = 0;
    int floor_y = DM2_FLOOR_Y + movement_floor_y;
    int floor_h = DM2_FLOOR_H;
    int floor_w_dst = DM2_VP_WIDTH;
    if (s->source_materials_required && s->gdat_scene_material_plan) {
        const DM2_V1_GdatSceneBlitRect *rect =
            &s->gdat_scene_material_plan->rects[0];
        floor_x = rect->x;
        floor_y = rect->y + movement_floor_y;
        floor_w_dst = rect->width;
        floor_h = rect->height;
    }
    if (floor_trim >= floor_h) {
        dm2_v1_block_source_material(s,
            DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING);
        return;
    }
    floor_y += floor_trim;
    floor_h -= floor_trim;
    /* QUERY_GDAT_IMAGE_LOCALPAL belongs to the decoded IMG3 being drawn.
     * Fetch the floor only after the ceiling blit: the viewport intentionally
     * keeps one active palette binding, so prefetching both would present the
     * ceiling through the floor's palette.  skproject DRAW_DUNGEON executes
     * the image/local-palette query immediately before each material blit. */
    if (s->source_materials_required) {
        floor_pixels = floor_material.pixels;
        floor_w = floor_material.width;
        floor_h_src = floor_material.height;
        floor_stride = floor_material.stride;
        memcpy(s->active_asset_palette16, floor_material.palette16,
               sizeof(s->active_asset_palette16));
        s->active_asset_palette_hash = floor_material.palette_hash;
        s->active_asset_palette_ready = floor_material.ready;
        floor_asset = 1;
    } else {
        floor_asset = dm2_v1_fetch_viewport_asset(
                          s, floor_gdat_index, &floor_pixels, &floor_w,
                          &floor_h_src, &floor_stride) == 0 &&
            floor_pixels && floor_w > 0 && floor_h_src > 0;
    }
    if (floor_asset) {
        dm2_v1_blit_tiled_material_bitmap(s,
                                 vp,
                                 stride,
                                 floor_x,
                                 floor_y,
                                 floor_w_dst,
                                 floor_h,
                                 floor_pixels,
                                 floor_w,
                                 floor_h_src,
                                 floor_stride > 0 ? floor_stride : floor_w,
                                 -1,
                                 floor_mirror,
                                 &s->gdat_material_palette_floor_ceiling_consumed_count);
        ++s->asset_floor_ceiling_drawn_count;
        ++s->gdat_scene_material_consumed_count;
        if (s->source_materials_required) {
            ++s->gdat_scene_draw_order_consumed_count;
        }
        if (s->source_materials_required) {
            s->last_floor_ceiling_material_consumed_mask |=
                DM2_SCENE_PLANE_FLOOR;
        }
    } else {
        dm2_v1_block_source_material(
            s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING);
    }

    if (s->source_materials_required &&
        s->last_floor_ceiling_material_required_mask !=
            s->last_floor_ceiling_material_consumed_mask) {
        dm2_v1_block_source_material(
            s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING);
    }

    /* DM2 distinctive: vertical wall frame area between ceiling and floor.
     * Source: DUNVIEW.C:2962-2967 (black area fill with 37 lines).
     * DM2 rooms: walls are drawn in the middle zone (lines ~25-135). */
}

/* ── Walls ───────────────────────────────────────────────────────── */

static void dm2_v1_block_source_material(DM2_V1_ViewportState *s,
                                         uint32_t material_mask)
{
    if (!s || !s->source_materials_required) {
        return;
    }
    ++s->blocked_material_draw_count;
    s->blocked_material_mask |= material_mask;
}

static int dm2_v1_wall_button_receipt_matches(
    const DM2_V1_ViewportState *s,
    const DM2_V1_DoorRender *door,
    int image_width,
    int image_height)
{
    int i;

    if (!s || !door || door->button_source_kind != 2 ||
        door->wall_button_field != 1 || image_width <= 0 ||
        image_height <= 0) {
        return 0;
    }
    if (s->g1_text_wall_gfx_materials &&
        s->g1_text_wall_gfx_materials->map == s->dungeon_level) {
        const DM2_V1_G1TextWallGfxRuntimeReceipt *receipt =
            s->g1_text_wall_gfx_materials;
        for (i = 0; i < receipt->material_count; ++i) {
            const DM2_V1_G1TextWallGfxMaterial *material =
                &receipt->materials[i];
            if (material->x == door->wall_button_x &&
                material->y == door->wall_button_y &&
                material->object_id == door->wall_button_object_id &&
                material->wall_gfx_index == (uint8_t)door->wall_button_index &&
                (!s->source_materials_required ||
                 (material->front_image_ready &&
                 material->front_image_width == (uint16_t)image_width &&
                 material->front_image_height == (uint16_t)image_height &&
                 material->local_palette_hash != 0u &&
                 material->raw_material_bytes &&
                 material->raw_material_byte_count != 0u &&
                 material->raw_material_hash != 0u &&
                 material->raw_material_receipt_hash != 0u &&
                 material->local_palette_hash ==
                      s->active_asset_palette_hash))) {
                return 1;
            }
        }
    }
    if (s->g1_actuator_wall_gfx_materials &&
        s->g1_actuator_wall_gfx_materials->map == s->dungeon_level) {
        const DM2_V1_G1ActuatorWallGfxRuntimeReceipt *receipt =
            s->g1_actuator_wall_gfx_materials;
        for (i = 0; i < receipt->material_count; ++i) {
            const DM2_V1_G1ActuatorWallGfxMaterial *material =
                &receipt->materials[i];
            if (material->x == door->wall_button_x &&
                material->y == door->wall_button_y &&
                material->object_id == door->wall_button_object_id &&
                material->wall_gfx_index == (uint8_t)door->wall_button_index &&
                (!s->source_materials_required ||
                 (material->front_image_ready &&
                 material->front_image_width == (uint16_t)image_width &&
                 material->front_image_height == (uint16_t)image_height &&
                 material->local_palette_hash != 0u &&
                 material->raw_material_bytes &&
                 material->raw_material_byte_count != 0u &&
                 material->raw_material_hash != 0u &&
                 material->raw_material_receipt_hash != 0u &&
                 material->local_palette_hash ==
                      s->active_asset_palette_hash))) {
                return 1;
            }
        }
    }
    return 0;
}

void dm2_v1_render_walls(DM2_V1_ViewportState *s)
{
    typedef struct {
        const uint8_t *pixels;
        int width;
        int height;
        int stride;
        uint8_t palette16[16];
        uint32_t palette_hash;
        DM2_V1_ViewportRect destination_rect;
        DM2_V1_ViewportRect source_rect;
        int mirror_flip;
        int ready;
    } DM2_V1_WallMaterial;
    if (!s || !s->framebuffer) return;
    uint8_t *vp = s->framebuffer;
    int stride = s->fb_stride;
    int wall_asset_count = 0;
    DM2_V1_WallPanelRenderPlan plan;
    DM2_V1_WallMaterial materials[DM2_V1_WALL_PANEL_RENDER_MAX];

    memset(materials, 0, sizeof(materials));
    s->last_dungeon_wall_material_required_mask = 0u;
    s->last_dungeon_wall_material_consumed_mask = 0u;

    /* DM2 wall rendering: draw back-to-front (D3→D2→D1→D0).
     * For each depth level, draw side walls first (L,R), then center (C).
     * Source: DUNVIEW.C:8466-8542 (draw order), DUNGEON.C:1371-1421.
     *
     * Wall set selection: for odd parity (wall_parity=1), use flipped set.
     * DM2 uses G3060 variant wall set (different from DM1's G2107).
     * Source: DUNVIEW.C:170-175, G3060_i_WallSet_Wall_D3C etc.
     *
     * Each visible wall cell is a source GDAT material; missing material must
     * stay absent rather than become a deterministic placeholder.
     */
    if (!s->source_materials_required && !s->asset_fetch) {
        dm2_v1_block_source_material(
            s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL);
        return;
    }

    /* skproject DRAW_WALL queries GRAPHICSSET with the live MapGraphicsStyle.
     * The default set is only a data-free renderer convenience; it must not
     * substitute for a missing source-owned scene-control receipt.  Source
     * material may arrive either as a pre-built GDAT wall plan or, for bounded
     * unit tests and direct M11 consumers, through the registered asset/palette
     * providers. */
    if (s->source_materials_required &&
        (!s->gdat_scene_control_ready ||
         (!s->gdat_wall_material_plan && !s->asset_fetch) ||
         (s->gdat_wall_material_plan &&
          s->gdat_wall_material_plan_scene_control_hash !=
              s->gdat_scene_control_hash))) {
        dm2_v1_block_source_material(
            s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL);
        return;
    }

    if (!dm2_v1_viewport_build_wall_panel_render_plan(s, &plan)) {
        if (s->source_materials_required) {
            dm2_v1_block_source_material(
                s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL);
        }
        return;
    }
    /* In M11 source mode a scene-control receipt without a pre-built wall plan
     * is incomplete unless the caller has explicitly flagged visible wall
     * squares (the bounded G1 asset-fallback path).  An empty plan here means
     * no wall material was required, so the frame must fail closed rather than
     * pretend a missing canonical plan is acceptable. */
    if (s->source_materials_required &&
        s->gdat_scene_control_ready &&
        !s->gdat_wall_material_plan &&
        plan.panel_count == 0) {
        dm2_v1_block_source_material(
            s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL);
        return;
    }
    if (s->source_materials_required &&
        plan.party_direction != (s->party_dir & 3)) {
        dm2_v1_block_source_material(
            s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL);
        return;
    }

    /* skproject/SKULLWIN/c_gui_vp.cpp DM2_DRAW_WALL resolves
     * GRAPHICSSET[viewportCell + 0x22] for every visible panel before its
     * blits.  Cache the complete source set before drawing the first pixel:
     * a missing later cell must block the frame, never leave an invented or
     * partially materialized dungeon wall behind. */
    if (s->source_materials_required) {
        for (int i = 0; i < plan.panel_count; ++i) {
            const DM2_V1_WallPanelRender *panel = &plan.panels[i];
            DM2_V1_WallMaterial *material = &materials[i];
            const DM2_V1_GdatWallM11Command *command = NULL;

            s->last_dungeon_wall_material_required_mask |=
                (uint16_t)(1u << (unsigned)panel->view_square);
            if (s->gdat_wall_material_plan) {
                const DM2_V1_GdatWallM11CommandPlan *wall_plan =
                    s->gdat_wall_material_plan;
                if (!wall_plan->valid || !wall_plan->command_hash ||
                    wall_plan->graphicsset != (uint8_t)s->gdat_scene_material_index) {
                    dm2_v1_block_source_material(s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL);
                    return;
                }
                for (int j = 0; j < wall_plan->command_count; ++j)
                    if (wall_plan->commands[j].view_square == panel->view_square) {
                        command = &wall_plan->commands[j]; break;
                    }
                if (!command || command->field !=
                        dm2_v1_viewport_wall_field_for_square(panel->view_square) ||
                    !command->pixels || !command->width || !command->height ||
                    !command->decoded_hash || !command->palette_hash ||
                    !command->material_source_bytes ||
                    command->material_source_byte_count == 0u ||
                    !command->material_receipt_hash ||
                    !command->rect_table_hash || !command->rect_row_hash ||
                    !command->metadata_hash || !command->geometry_hash ||
                    command->movement_active !=
                        (uint8_t)(s->gdat_scene_movement_active ? 1u : 0u) ||
                    dm2_v1_wall_command_geometry_hash(command) !=
                        command->geometry_hash) {
                    dm2_v1_block_source_material(s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL);
                    return;
                }
                material->pixels = command->pixels;
                material->width = command->width;
                material->height = command->height;
                material->stride = command->width;
                memcpy(material->palette16, command->palette16, sizeof(material->palette16));
                material->palette_hash = command->palette_hash;
                material->destination_rect = (DM2_V1_ViewportRect){
                    command->destination_x, command->destination_y,
                    command->destination_width, command->destination_height
                };
                material->source_rect = (DM2_V1_ViewportRect){
                    command->source_x, command->source_y,
                    command->source_width, command->source_height
                };
                material->mirror_flip = command->mirror_flip;
                material->ready = 1;
                continue;
            }
            /* Bounded source path for direct M11 consumers and unit tests:
             * when no pre-built GDAT wall plan is present, fetch each panel's
             * material through the registered asset/palette providers.  The
             * panel geometry still comes from the source-ordered wall frame
             * table (DUNVIEW.C:8466-8542). */
            {
                const uint8_t *pixels = NULL;
                int w = 0, h = 0, stride = 0;
                if (dm2_v1_fetch_viewport_local_material(
                        s, panel->gdat_index,
                        &pixels, &w, &h, &stride) != 0 ||
                    !pixels || w <= 0 || h <= 0 ||
                    !s->active_asset_palette_ready) {
                    dm2_v1_block_source_material(
                        s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL);
                    return;
                }
                material->pixels = pixels;
                material->width = w;
                material->height = h;
                material->stride = stride;
                memcpy(material->palette16, s->active_asset_palette16,
                       sizeof(material->palette16));
                material->palette_hash = s->active_asset_palette_hash;
                material->destination_rect = panel->dst_rect;
                /* The registered asset provider owns the decoded pixel buffer;
                 * consume its full extent as the source rectangle.  This keeps
                 * bounded unit-test stand-ins (e.g. 2x2) source-backed without
                 * guessing real GDAT dimensions. */
                material->source_rect = (DM2_V1_ViewportRect){0, 0, w, h};
                material->mirror_flip = 0;
                material->ready = 1;
                continue;
            }
        }
    }

    for (int i = 0; i < plan.panel_count; ++i) {
        const DM2_V1_WallPanelRender *panel = &plan.panels[i];
        const uint8_t *wall_pixels = NULL;
        int wall_w = 0;
        int wall_h = 0;
        int wall_stride = 0;
        const DM2_V1_ViewportRect *destination_rect = &panel->dst_rect;
        DM2_V1_ViewportRect source_rect = panel->src_rect;
        int mirror_flip = 0;

        if (s->source_materials_required) {
            const DM2_V1_WallMaterial *material = &materials[i];
            wall_pixels = material->pixels;
            wall_w = material->width;
            wall_h = material->height;
            wall_stride = material->stride;
            destination_rect = &material->destination_rect;
            source_rect = material->source_rect;
            mirror_flip = material->mirror_flip;
            memcpy(s->active_asset_palette16, material->palette16,
                   sizeof(s->active_asset_palette16));
            s->active_asset_palette_hash = material->palette_hash;
            s->active_asset_palette_ready = material->ready;
        }
        if ((!s->source_materials_required &&
             dm2_v1_fetch_viewport_local_material(s,
                                                   panel->gdat_index,
                                                   &wall_pixels,
                                                   &wall_w,
                                                   &wall_h,
                                                   &wall_stride) != 0) ||
            !wall_pixels || wall_w <= 0 || wall_h <= 0) {
            dm2_v1_block_source_material(
                s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL);
            continue;
        }

        if (source_rect.x < 0 || source_rect.y < 0 || source_rect.w <= 0 ||
            source_rect.h <= 0 || source_rect.x + source_rect.w > wall_w ||
            source_rect.y + source_rect.h > wall_h) {
            dm2_v1_block_source_material(
                s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL);
            continue;
        }
        dm2_v1_blit_scaled_material_bitmap_region_ex(
            s, vp, stride, destination_rect->x, destination_rect->y,
            destination_rect->w, destination_rect->h, wall_pixels,
            source_rect.x, source_rect.y, source_rect.w, source_rect.h,
            wall_stride > 0 ? wall_stride : wall_w,
            s->gdat_scene_control_ready ? (int)s->gdat_scene_colorkey
                                        : DM2_COLOR_TRANSPARENT,
            mirror_flip, &s->gdat_material_palette_wall_consumed_count);
        if (s->gdat_scene_control_ready) {
            ++s->gdat_scene_control_consumed_count;
        }
        if (s->source_materials_required) {
            s->last_dungeon_wall_material_consumed_mask |=
                (uint16_t)(1u << (unsigned)panel->view_square);
            if (s->gdat_wall_material_plan) {
                ++s->gdat_wall_material_plan_consumed_count;
            }
        }
        ++wall_asset_count;
    }

    if (s->source_materials_required &&
        s->last_dungeon_wall_material_required_mask !=
            s->last_dungeon_wall_material_consumed_mask) {
        dm2_v1_block_source_material(
            s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL);
        return;
    }

    if (wall_asset_count > 0) {
        s->asset_wall_drawn_count += wall_asset_count;
    }
}

/* ── Wall ornaments ─────────────────────────────────────────────────
 * skproject/SKWIN/SkWinCore.cpp DRAW_WALL_ORNATE (^32CB:15B8) selects a
 * WALL_GFX image from tblCellTilesRoom and materializes it through
 * QUERY_TEMP_PICST / DRAW_TEMP_PICST.  Firestaff treats every visible
 * wall-ornament square as a distinct dungeon material class: a missing
 * source image blocks the frame rather than leaving a blank wall or a
 * procedural substitute.  Exact placement mirrors the source rect table
 * and is applied only when a source-owned material plan is bound.
 */

void dm2_v1_render_wall_ornaments(DM2_V1_ViewportState *s)
{
    const DM2_V1_WallOrnamentRenderPlan *plan = NULL;

    if (!s || !s->framebuffer || s->is_outdoor) return;

    s->last_wall_ornament_material_required_mask = 0u;
    s->last_wall_ornament_material_consumed_mask = 0u;

    /* skproject/SKWIN/SkWinCore.cpp DRAW_WALL_ORNATE (^32CB:15B8) fetches the
     * WALL_GFX image from tblCellTilesRoom and draws it through
     * QUERY_TEMP_PICST / DRAW_TEMP_PICST.  Firestaff consumes a runtime-bound
     * source plan so the destination rectangle is owned by the loader, not
     * invented by the renderer. */
    if (s->source_materials_required) {
        plan = s->gdat_wall_ornament_material_plan;
    }

    for (int i = 0; i < DM2_SQ_COUNT; ++i) {
        const DM2_ViewSquare *sq = &s->squares[i];
        const uint8_t *pixels = NULL;
        int width = 0, height = 0, stride = 0;
        int gdat_index;
        const DM2_V1_WallOrnamentRender *ornament = NULL;

        if (sq->square_type != DM2_SQUARE_WALL ||
            sq->wall_ornate_gfx_index == 0u) {
            continue;
        }

        s->last_wall_ornament_material_required_mask |=
            (uint16_t)(1u << (unsigned)i);

        if (s->source_materials_required) {
            if (!plan || !plan->valid) {
                dm2_v1_block_source_material(
                    s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL_ORNAMENT);
                continue;
            }
            for (int j = 0; j < plan->ornament_count; ++j) {
                if (plan->ornaments[j].view_square == i &&
                    plan->ornaments[j].gdat_index != 0) {
                    ornament = &plan->ornaments[j];
                    break;
                }
            }
            if (!ornament) {
                dm2_v1_block_source_material(
                    s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL_ORNAMENT);
                continue;
            }
        }

        gdat_index = dm2_v1_viewport_wall_gfx_map_chip_graphic_index(
            sq->wall_ornate_gfx_index);
        /* The runtime plan identifies both the source WALL_GFX material and
         * its source-owned placement. Do not let a plan for a different GDAT
         * image borrow this square's derived map-chip pixels. */
        if (ornament && ornament->gdat_index != gdat_index) {
            dm2_v1_block_source_material(
                s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL_ORNAMENT);
            continue;
        }
        if (gdat_index == 0 ||
            dm2_v1_fetch_viewport_local_material(
                s, gdat_index, &pixels, &width, &height, &stride) != 0 ||
            !pixels || width <= 0 || height <= 0 || stride < width) {
            dm2_v1_block_source_material(
                s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL_ORNAMENT);
            continue;
        }

        if (ornament) {
            const DM2_V1_ViewportRect *dst = &ornament->dst_rect;
            if (dst->w <= 0 || dst->h <= 0 ||
                dst->x < 0 || dst->y < 0 ||
                (unsigned)dst->x + (unsigned)dst->w > DM2_VP_WIDTH ||
                (unsigned)dst->y + (unsigned)dst->h > DM2_VP_HEIGHT) {
                dm2_v1_block_source_material(
                    s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL_ORNAMENT);
                continue;
            }
            dm2_v1_blit_scaled_material_bitmap(
                s, s->framebuffer, s->fb_stride,
                dst->x, dst->y, dst->w, dst->h,
                pixels, width, height, stride,
                DM2_COLOR_TRANSPARENT,
                &s->gdat_material_palette_wall_consumed_count);
        }
        ++s->asset_wall_ornament_drawn_count;
        s->last_wall_ornament_material_consumed_mask |=
            (uint16_t)(1u << (unsigned)i);
    }

    if (s->source_materials_required &&
        s->last_wall_ornament_material_required_mask !=
            s->last_wall_ornament_material_consumed_mask) {
        dm2_v1_block_source_material(
            s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL_ORNAMENT);
    }
}

/* ── Doors ────────────────────────────────────────────────────────── */

void dm2_v1_render_doors(DM2_V1_ViewportState *s)
{
    typedef struct {
        const uint8_t *pixels;
        int width;
        int height;
        int stride;
        uint8_t palette16[16];
        uint32_t palette_hash;
        uint32_t decoded_hash;
        uint8_t palette_darkness;
        uint32_t palette_light_receipt_hash;
        uint32_t palette_transform_hash;
        int transparent_color;
        uint8_t light_palette;
        uint16_t rect_number;
        DM2_V1_ViewportRect source_rect;
        uint32_t geometry_hash;
        const DM2_V1_GdatDoorOverlayM11Command *panel_commands[2];
        int panel_command_count;
        int required;
        int consumed;
    } DM2_V1_DoorMaterial;
    enum {
        DM2_DOOR_MATERIAL_PANEL = 0,
        DM2_DOOR_MATERIAL_ORNATE,
        DM2_DOOR_MATERIAL_DESTROYED_MASK,
        DM2_DOOR_MATERIAL_FRAME,
        DM2_DOOR_MATERIAL_BUTTON,
        DM2_DOOR_MATERIAL_COUNT
    };
    if (!s || !s->framebuffer) return;
    uint8_t *vp = s->framebuffer;
    int stride = s->fb_stride;
    int door_panel_asset_count = 0;
    int door_overlay_asset_count = 0;
    int door_asset_count = 0;
    int door_button_asset_count = 0;
    DM2_V1_DoorRenderPlan plan;
    DM2_V1_DoorMaterial materials[DM2_V1_DOOR_RENDER_MAX]
                                   [DM2_DOOR_MATERIAL_COUNT];

    /* DM2 door rendering: overlays on wall squares.
     * Source: DUNVIEW.C:3082-3095 F0102_DrawDoorBitmap,
     *         DUNVIEW.C:3096-3112 F0103_DrawDoorFrameBitmapFlippedHorizontally,
     *         DUNVIEW.C:4119-4270 F0110_DrawDoorButton, F0111_DrawDoor.
     * DM2 door frames: larger/more ornate than DM1 (G2116-G2119 + G2196).
     * Source: DUNVIEW.C:148-157 (door frame indices).
     *
     * Door panels are admitted from the source GRAPHICS.DAT plan. */

    if (!dm2_v1_viewport_build_door_render_plan(s, &plan)) {
        return;
    }
    /* skproject DRAW_DOOR_FRAMES captures glbMapGraphicsSet before resolving
     * GRAPHICSSET side frames.  Without the G1 scene receipt, a same-shaped
     * default door would be a wrong material route rather than a valid draw. */
    if (s->source_materials_required && !s->gdat_scene_control_ready) {
        int source_custom_button_only = 1;
        for (int square = 0; square < DM2_SQ_COUNT; ++square) {
            const DM2_ViewSquare *vs = &s->squares[square];
            if ((vs->flags & DM2_SQF_HAS_DOOR) && !vs->door_wall_button) {
                source_custom_button_only = 0;
                break;
            }
        }
        if (!source_custom_button_only) {
            dm2_v1_block_source_material(
                s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR);
            return;
        }
    }
    if (s->source_materials_required &&
        s->gdat_door_overlay_material_plan &&
        !dm2_v1_gdat_door_overlay_m11_command_plan_draw_controls_valid(
            s->gdat_door_overlay_material_plan)) {
        /* DRAW_DOOR chooses its image, stretch and light tuple before any
         * panel/frame draw. A mismatched M11 receipt is not source material. */
        dm2_v1_block_source_material(
            s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR);
        return;
    }
    if (s->source_materials_required &&
        s->gdat_door_overlay_material_plan) {
        const DM2_V1_GdatDoorOverlayM11CommandPlan *overlay_plan =
            s->gdat_door_overlay_material_plan;
        for (int i = 0; i < overlay_plan->command_count; ++i) {
            if (overlay_plan->commands[i].movement_active !=
                (uint8_t)(s->gdat_scene_movement_active ? 1u : 0u)) {
                dm2_v1_block_source_material(
                    s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR);
                return;
            }
        }
    }
    if (s->source_materials_required) {
        for (int square = 0; square < DM2_SQ_COUNT; ++square) {
            const DM2_ViewSquare *vs = &s->squares[square];
            if ((vs->flags & DM2_SQF_HAS_DOOR) && !vs->door_gfx_admitted &&
                !vs->door_wall_button) {
                /* A map can contain a DB0-shaped tile while LOAD_LOCALLEVEL_DYN
                 * did not admit its DoorType. Keep the whole source frame
                 * blocked instead of replacing it with a generic panel. */
                dm2_v1_block_source_material(
                    s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR);
                return;
            }
        }
    }
    memset(materials, 0, sizeof(materials));
    s->last_door_material_required_mask = 0u;
    s->last_door_material_consumed_mask = 0u;

    /* skproject/SKWIN/SkWinCore.cpp DM2_DRAW_DOOR resolves the selected
     * panel, overlays, frame, and button IMG3s before submitting its door
     * blits. Keep the complete transaction renderer-owned: a later missing
     * local palette must not leave a partly invented door on the scene. */
    if (s->source_materials_required) {
        for (int i = 0; i < plan.door_count; ++i) {
            const DM2_V1_DoorRender *door = &plan.doors[i];
            const int custom_wall_button_only =
                door->button_source_kind == 2 && !door->door_gfx_admitted;
            const int source_no_frames =
                dm2_v1_viewport_door_m11_has_no_frames(
                    s->gdat_door_overlay_material_plan, door);
            const int gdat_indices[DM2_DOOR_MATERIAL_COUNT] = {
                door->panel_gdat_index,
                door->ornate_gdat_index,
                door->destroyed_mask_gdat_index,
                door->frame_gdat_index,
                door->button_gdat_index
            };
            const int required[DM2_DOOR_MATERIAL_COUNT] = {
                !custom_wall_button_only && door->panel_visible_rect.w > 0 &&
                    door->panel_visible_rect.h > 0,
                !custom_wall_button_only && door->ornate_gdat_index != 0 &&
                    door->panel_rect.w > 0 && door->panel_rect.h > 0,
                !custom_wall_button_only && door->destroyed_mask_gdat_index != 0 &&
                    door->panel_rect.w > 0 && door->panel_rect.h > 0,
                !custom_wall_button_only && !source_no_frames &&
                    door->frame_gdat_index != 0 &&
                    door->frame_rect.w > 0 &&
                    door->frame_rect.h > 0,
                door->button_gdat_index != 0 &&
                    door->button_rect.w > 0 && door->button_rect.h > 0
            };

            for (int kind = 0; kind < DM2_DOOR_MATERIAL_COUNT; ++kind) {
                DM2_V1_DoorMaterial *material = &materials[i][kind];

                if (!required[kind]) {
                    continue;
                }
                material->required = 1;
                s->last_door_material_required_mask |=
                    (uint8_t)(1u << (unsigned)kind);
                if (s->gdat_door_overlay_material_plan) {
                    const DM2_V1_GdatDoorOverlayM11CommandPlan *overlay_plan =
                        s->gdat_door_overlay_material_plan;
                    const int wanted_kind = kind == DM2_DOOR_MATERIAL_PANEL
                        ? DM2_V1_GDAT_DOOR_PANEL
                        : kind == DM2_DOOR_MATERIAL_ORNATE
                            ? DM2_V1_GDAT_DOOR_OVERLAY_ORNATE
                            : kind == DM2_DOOR_MATERIAL_DESTROYED_MASK
                                ? DM2_V1_GDAT_DOOR_OVERLAY_DESTROYED_MASK
                                : kind == DM2_DOOR_MATERIAL_FRAME
                                    ? DM2_V1_GDAT_DOOR_FRAME
                                    : DM2_V1_GDAT_DOOR_BUTTON;
                    const DM2_V1_GdatDoorOverlayM11Command *command = NULL;
                    for (int j = 0; overlay_plan->valid &&
                         j < overlay_plan->command_count; ++j) {
                        const DM2_V1_GdatDoorOverlayM11Command *candidate =
                            &overlay_plan->commands[j];
                        if (candidate->gdat_index == gdat_indices[kind] &&
                            candidate->view_square == door->view_square &&
                            candidate->kind == wanted_kind &&
                            candidate->door_opening_dir ==
                                door->door_opening_dir &&
                            candidate->door_state == door->door_state &&
                            candidate->door_open_pct == door->door_open_pct) {
                            if (!command) command = candidate;
                            if (kind == DM2_DOOR_MATERIAL_PANEL &&
                                material->panel_command_count < 2) {
                                material->panel_commands[
                                    material->panel_command_count++] = candidate;
                            }
                        }
                    }
                    if (command) {
                        material->pixels = command->pixels;
                        material->width = command->width;
                        material->height = command->height;
                        material->stride = command->width;
                        memcpy(material->palette16, command->palette16,
                               sizeof(material->palette16));
                        material->palette_hash = command->palette_hash;
                        material->decoded_hash = command->decoded_hash;
                        material->palette_darkness = command->palette_darkness;
                        material->palette_light_receipt_hash =
                            command->palette_light_receipt_hash;
                        material->palette_transform_hash =
                            command->palette_transform_hash;
                        material->light_palette = command->light_palette;
                        material->rect_number = command->rect_number;
                        material->source_rect = (DM2_V1_ViewportRect){
                            command->rect_x, command->rect_y,
                            command->rect_width, command->rect_height };
                        material->geometry_hash = command->geometry_hash;
                        /* SKProject DRAW_DOOR obtains DOORS/entry/
                         * GDAT_IMG_COLORKEY_1 before its panel blit. Only
                         * the selected panel owns that datum. */
                        material->transparent_color =
                            kind == DM2_DOOR_MATERIAL_PANEL
                                ? (int)command->color_key
                                : DM2_COLOR_TRANSPARENT;
                        ++s->gdat_door_overlay_material_plan_consumed_count;
                    }
                    if (kind == DM2_DOOR_MATERIAL_PANEL) {
                        const int horizontal_split = door->door_state > 0u &&
                            door->door_state < 4u &&
                            door->door_opening_dir == 0u;
                        if ((horizontal_split && material->panel_command_count != 2) ||
                            (!horizontal_split && material->panel_command_count != 1)) {
                            dm2_v1_block_source_material(
                                s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR);
                            return;
                        }
                    }
                }
                if (kind == DM2_DOOR_MATERIAL_BUTTON &&
                    door->button_source_kind == 2 &&
                    (!material->pixels || material->width <= 0 ||
                     material->height <= 0)) {
                    (void)dm2_v1_fetch_viewport_local_material(
                        s, gdat_indices[kind], &material->pixels,
                        &material->width, &material->height,
                        &material->stride);
                }
                if (kind == DM2_DOOR_MATERIAL_BUTTON &&
                    door->button_source_kind == 2 && material->pixels &&
                    material->width > 0 && material->height > 0 &&
                    material->stride >= material->width) {
                    material->decoded_hash = dm2_v1_weather_pixels_hash(
                        material->pixels, material->width, material->height,
                        material->stride);
                }
                if (gdat_indices[kind] == 0 ||
                    ((!material->pixels || material->width <= 0 ||
                      material->height <= 0) &&
                     dm2_v1_fetch_viewport_local_material(
                        s, gdat_indices[kind], &material->pixels,
                        &material->width, &material->height,
                        &material->stride) != 0) ||
                    !material->pixels || material->width <= 0 ||
                    material->height <= 0 ||
                    !material->decoded_hash ||
                    dm2_v1_weather_pixels_hash(material->pixels,
                                               material->width,
                                               material->height,
                                               material->stride) !=
                        material->decoded_hash ||
                    (!material->palette_hash &&
                     (!s->active_asset_palette_ready ||
                      s->active_asset_palette_hash == 0u)) ||
                    (material->palette_hash &&
                     dm2_v1_weather_pixels_hash(material->palette16, 16, 1,
                                                16) != material->palette_hash) ||
                    (kind == DM2_DOOR_MATERIAL_BUTTON &&
                     door->button_source_kind == 2 &&
                     !dm2_v1_wall_button_receipt_matches(
                         s, door, material->width, material->height))) {
                    dm2_v1_block_source_material(
                        s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR);
                    return;
                }
                /* c_gui_vp.cpp::DM2_DRAW_DOOR may retry field zero with a
                 * nonzero source light palette. It is legal only after the
                 * plan's dt07/2 remap is bound to this exact c_light result. */
                if (kind == DM2_DOOR_MATERIAL_PANEL &&
                    material->light_palette != 0u &&
                    (!s->gdat_c_light_receipt_ready ||
                     material->palette_darkness > 64u ||
                     material->palette_light_receipt_hash !=
                         s->gdat_c_light_receipt_hash ||
                     material->palette_transform_hash == 0u)) {
                        dm2_v1_block_source_material(
                            s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR);
                        return;
                }
                if (kind == DM2_DOOR_MATERIAL_PANEL &&
                    door->panel_visible_rect.w > 0 &&
                    s->gdat_door_overlay_material_plan &&
                    (!material->geometry_hash || !material->rect_number ||
                     material->source_rect.w <= 0 ||
                     material->source_rect.h <= 0)) {
                    /* DRAW_DOOR's panel destination is a RAW4
                     * QUERY_BLIT_RECT result.  Do not retain the old
                     * bounded viewport rectangle when source M11 is active. */
                    dm2_v1_block_source_material(
                        s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR);
                    return;
                }
                if (!material->palette_hash) {
                    memcpy(material->palette16, s->active_asset_palette16,
                           sizeof(material->palette16));
                    material->palette_hash = s->active_asset_palette_hash;
                }
            }
        }
    }
    s->last_door_panel_asset_blit_valid = 0;
    s->last_door_ornate_asset_blit_valid = 0;
    s->last_door_destroyed_mask_asset_blit_valid = 0;
    s->last_door_frame_asset_blit_valid = 0;
    s->last_door_button_asset_blit_valid = 0;
    s->last_door_panel_asset_src_w = 0;
    s->last_door_panel_asset_src_h = 0;
    s->last_door_panel_asset_src_stride = 0;
    s->last_door_ornate_asset_src_w = 0;
    s->last_door_ornate_asset_src_h = 0;
    s->last_door_ornate_asset_src_stride = 0;
    s->last_door_destroyed_mask_asset_src_w = 0;
    s->last_door_destroyed_mask_asset_src_h = 0;
    s->last_door_destroyed_mask_asset_src_stride = 0;
    s->last_door_frame_asset_src_w = 0;
    s->last_door_frame_asset_src_h = 0;
    s->last_door_frame_asset_src_stride = 0;
    s->last_door_button_asset_src_w = 0;
    s->last_door_button_asset_src_h = 0;
    s->last_door_button_asset_src_stride = 0;
    memset(&s->last_door_panel_asset_blit, 0,
           sizeof(s->last_door_panel_asset_blit));
    memset(&s->last_door_ornate_asset_blit, 0,
           sizeof(s->last_door_ornate_asset_blit));
    memset(&s->last_door_destroyed_mask_asset_blit, 0,
           sizeof(s->last_door_destroyed_mask_asset_blit));
    memset(&s->last_door_frame_asset_blit, 0,
           sizeof(s->last_door_frame_asset_blit));
    memset(&s->last_door_button_asset_blit, 0,
           sizeof(s->last_door_button_asset_blit));

    for (int i = 0; i < plan.door_count; i++) {
        const DM2_V1_DoorRender *door = &plan.doors[i];
        const int custom_wall_button_only =
            s->source_materials_required &&
            door->button_source_kind == 2 && !door->door_gfx_admitted;
        DM2_V1_DoorRender source_door;
        const DM2_V1_DoorRender *render_door = door;
        const int source_no_frames = s->source_materials_required &&
            dm2_v1_viewport_door_m11_has_no_frames(
                s->gdat_door_overlay_material_plan, door);
        int ornate_drawn_asset = 0;
        int destroyed_mask_drawn_asset = 0;
        int frame_drawn_asset = 0;
        int button_drawn_asset = 0;

        if (!custom_wall_button_only && s->source_materials_required &&
            door->panel_visible_rect.w > 0 &&
            door->panel_visible_rect.h > 0) {
            const DM2_V1_DoorMaterial *panel =
                &materials[i][DM2_DOOR_MATERIAL_PANEL];
            source_door = *door;
            source_door.panel_rect = panel->source_rect;
            /* The only admitted current geometry route is the closed panel;
             * partial opening/split-panel placement remains fail-closed. */
            source_door.panel_visible_rect = panel->source_rect;
            render_door = &source_door;
        }

        if (!custom_wall_button_only &&
            render_door->panel_visible_rect.w > 0 &&
            render_door->panel_visible_rect.h > 0) {
            const uint8_t *panel_pixels = NULL;
            int panel_w = 0;
            int panel_h = 0;
            int panel_stride = 0;
            int panel_drawn_asset = 0;
            const DM2_V1_DoorMaterial *source_panel =
                &materials[i][DM2_DOOR_MATERIAL_PANEL];
            const int horizontal_split = s->source_materials_required &&
                source_panel->panel_command_count == 2;
            if (horizontal_split) {
                int split_drawn = 0;
                for (int part = 0; part < source_panel->panel_command_count;
                     ++part) {
                    const DM2_V1_GdatDoorOverlayM11Command *command =
                        source_panel->panel_commands[part];
                    if (!command || !command->pixels || !command->decoded_hash ||
                        !command->palette_hash ||
                        (command->light_palette != 0u &&
                         (!s->gdat_c_light_receipt_ready ||
                          command->palette_darkness > 64u ||
                          command->palette_light_receipt_hash !=
                              s->gdat_c_light_receipt_hash ||
                          command->palette_transform_hash == 0u)) ||
                        command->source_width == 0u || command->source_height == 0u ||
                        command->source_x + command->source_width > command->width ||
                        command->source_y + command->source_height > command->height ||
                        command->rect_width == 0u || command->rect_height == 0u ||
                        dm2_v1_weather_pixels_hash(command->pixels,
                                                   command->width,
                                                   command->height,
                                                   command->width) !=
                            command->decoded_hash ||
                        dm2_v1_weather_pixels_hash(command->palette16, 16, 1,
                                                   16) != command->palette_hash) {
                        dm2_v1_block_source_material(
                            s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR);
                        return;
                    }
                    memcpy(s->active_asset_palette16, command->palette16,
                           sizeof(s->active_asset_palette16));
                    s->active_asset_palette_hash = command->palette_hash;
                    s->active_asset_palette_ready = 1;
                    dm2_v1_blit_scaled_material_bitmap_region_ex(
                        s, vp, stride, command->rect_x, command->rect_y,
                        command->rect_width, command->rect_height,
                        command->pixels, command->source_x, command->source_y,
                        command->source_width, command->source_height,
                        command->width, command->color_key, 0,
                        &s->gdat_sprite_palette_consumed_count);
                    ++split_drawn;
                    ++door_panel_asset_count;
                    s->last_door_panel_asset_blit_valid = 1;
                    s->last_door_panel_asset_blit = (DM2_V1_DoorAssetBlit){
                        command->gdat_index,
                        { (int)command->source_x, (int)command->source_y,
                          (int)command->source_width, (int)command->source_height },
                        { command->rect_x, command->rect_y,
                          command->rect_width, command->rect_height },
                        command->width, command->color_key };
                    s->last_door_panel_asset_src_w = command->width;
                    s->last_door_panel_asset_src_h = command->height;
                    s->last_door_panel_asset_src_stride = command->width;
                }
                if (split_drawn != 2) {
                    dm2_v1_block_source_material(
                        s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR);
                    return;
                }
                materials[i][DM2_DOOR_MATERIAL_PANEL].consumed = 1;
                panel_drawn_asset = 1;
            }
            if (!horizontal_split && s->source_materials_required) {
                const DM2_V1_DoorMaterial *material =
                    &materials[i][DM2_DOOR_MATERIAL_PANEL];
                panel_pixels = material->pixels;
                panel_w = material->width;
                panel_h = material->height;
                panel_stride = material->stride;
                memcpy(s->active_asset_palette16, material->palette16,
                       sizeof(s->active_asset_palette16));
                s->active_asset_palette_hash = material->palette_hash;
                s->active_asset_palette_ready = material->palette_hash != 0u;
            }
            if (!horizontal_split && ((s->source_materials_required && panel_pixels &&
                  panel_w > 0 && panel_h > 0) ||
                 (!s->source_materials_required && render_door->panel_gdat_index != 0 &&
                dm2_v1_fetch_viewport_local_material(s,
                                                      render_door->panel_gdat_index,
                                                      &panel_pixels,
                                                      &panel_w,
                                                      &panel_h,
                                                      &panel_stride) == 0 &&
                  panel_pixels && panel_w > 0 && panel_h > 0))) {
                DM2_V1_DoorAssetBlit blit;
                /* skproject SKWIN/SkWinCore.cpp DRAW_DOOR lines
                 * ~46402-46457 draws the panel through GDAT_CATEGORY_DOORS
                 * with image 0 for D0/D1 and image 1 for D2. Door type
                 * decoding is still boot-defaulted to index 0 here. */
                if (dm2_v1_viewport_door_panel_asset_blit(render_door,
                                                          panel_w,
                                                          panel_h,
                                                          panel_stride,
                                                          &blit)) {
                    if (s->source_materials_required) {
                        blit.transparent_color =
                            materials[i][DM2_DOOR_MATERIAL_PANEL]
                                .transparent_color;
                    }
                    dm2_v1_blit_scaled_material_bitmap_region_ex(
                        s, vp,
                        stride,
                        blit.dst_rect.x,
                        blit.dst_rect.y,
                        blit.dst_rect.w,
                        blit.dst_rect.h,
                        panel_pixels,
                        blit.src_rect.x,
                        blit.src_rect.y,
                        blit.src_rect.w,
                        blit.src_rect.h,
                        blit.src_stride,
                        blit.transparent_color,
                        0,
                        &s->gdat_sprite_palette_consumed_count);
                    ++door_panel_asset_count;
                    s->last_door_panel_asset_blit_valid = 1;
                    s->last_door_panel_asset_blit = blit;
                    s->last_door_panel_asset_src_w = panel_w;
                    s->last_door_panel_asset_src_h = panel_h;
                    s->last_door_panel_asset_src_stride =
                        panel_stride > 0 ? panel_stride : panel_w;
                    panel_drawn_asset = 1;
                    if (s->source_materials_required) {
                        materials[i][DM2_DOOR_MATERIAL_PANEL].consumed = 1;
                    }
                }
            }
            if (!panel_drawn_asset) {
                /* DRAW_DOOR resolves the selected DOORS image before it
                 * paints a panel.  Never replace a missing GDAT panel with
                 * a generated rectangle. */
                dm2_v1_block_source_material(
                    s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR);
            }
        }
        if (!custom_wall_button_only) {
            const int overlay_indices[2] = {
                door->ornate_gdat_index,
                door->destroyed_mask_gdat_index
            };
            for (int overlay_i = 0; overlay_i < 2; ++overlay_i) {
                const uint8_t *overlay_pixels = NULL;
                int overlay_w = 0;
                int overlay_h = 0;
                int overlay_stride = 0;
                const int material_kind = overlay_i == 0
                    ? DM2_DOOR_MATERIAL_ORNATE
                    : DM2_DOOR_MATERIAL_DESTROYED_MASK;
                if (s->source_materials_required &&
                    materials[i][material_kind].required) {
                    const DM2_V1_DoorMaterial *material =
                        &materials[i][material_kind];
                    overlay_pixels = material->pixels;
                    overlay_w = material->width;
                    overlay_h = material->height;
                    overlay_stride = material->stride;
                    memcpy(s->active_asset_palette16, material->palette16,
                           sizeof(s->active_asset_palette16));
                    s->active_asset_palette_hash = material->palette_hash;
                    s->active_asset_palette_ready = material->palette_hash != 0u;
                }
                if (((s->source_materials_required && overlay_pixels &&
                      overlay_w > 0 && overlay_h > 0) ||
                     (!s->source_materials_required && overlay_indices[overlay_i] != 0 &&
                    render_door->panel_rect.w > 0 && render_door->panel_rect.h > 0 &&
                    dm2_v1_fetch_viewport_local_material(
                        s, overlay_indices[overlay_i], &overlay_pixels,
                        &overlay_w, &overlay_h, &overlay_stride) == 0 &&
                      overlay_pixels && overlay_w > 0 && overlay_h > 0))) {
                    DM2_V1_DoorAssetBlit blit;
                    if (dm2_v1_viewport_full_rect_asset_blit(
                            overlay_indices[overlay_i],
                            &render_door->panel_rect,
                            overlay_w,
                            overlay_h,
                            overlay_stride,
                            &blit)) {
                        /* skproject/SKULLWIN/c_gui_vp.cpp DM2_DRAW_DOOR
                         * draws ornate and destroyed-door overlays through
                         * the active GDAT palette just like the base door
                         * panel.  Keeping these pixels on the raw blitter
                         * made real logical indices bypass dtPalette16. */
                        dm2_v1_blit_scaled_material_bitmap_region(
                            s, vp,
                            stride,
                            blit.dst_rect.x,
                            blit.dst_rect.y,
                            blit.dst_rect.w,
                            blit.dst_rect.h,
                            overlay_pixels,
                            blit.src_rect.x,
                            blit.src_rect.y,
                            blit.src_rect.w,
                            blit.src_rect.h,
                            blit.src_stride,
                            blit.transparent_color,
                            &s->gdat_sprite_palette_consumed_count);
                        ++door_overlay_asset_count;
                        if (s->source_materials_required) {
                            materials[i][material_kind].consumed = 1;
                        }
                        if (overlay_i == 0) {
                            ornate_drawn_asset = 1;
                            s->last_door_ornate_asset_blit_valid = 1;
                            s->last_door_ornate_asset_blit = blit;
                            s->last_door_ornate_asset_src_w = overlay_w;
                            s->last_door_ornate_asset_src_h = overlay_h;
                            s->last_door_ornate_asset_src_stride =
                                overlay_stride > 0 ? overlay_stride :
                                                     overlay_w;
                        } else {
                            destroyed_mask_drawn_asset = 1;
                            s->last_door_destroyed_mask_asset_blit_valid = 1;
                            s->last_door_destroyed_mask_asset_blit = blit;
                            s->last_door_destroyed_mask_asset_src_w =
                                overlay_w;
                            s->last_door_destroyed_mask_asset_src_h =
                                overlay_h;
                            s->last_door_destroyed_mask_asset_src_stride =
                                overlay_stride > 0 ? overlay_stride :
                                                     overlay_w;
                        }
                    }
                }
                if (overlay_indices[overlay_i] != 0 &&
                    s->source_materials_required &&
                    ((overlay_i == 0 &&
                      !ornate_drawn_asset) ||
                     (overlay_i == 1 &&
                      !destroyed_mask_drawn_asset))) {
                    dm2_v1_block_source_material(
                        s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR);
                }
            }
        }
        if (!custom_wall_button_only && !source_no_frames &&
            door->frame_gdat_index != 0 &&
            door->frame_rect.w > 0 &&
            door->frame_rect.h > 0) {
            const uint8_t *door_pixels = NULL;
            int door_w = 0;
            int door_h = 0;
            int door_stride = 0;

            if (s->source_materials_required) {
                const DM2_V1_DoorMaterial *material =
                    &materials[i][DM2_DOOR_MATERIAL_FRAME];
                door_pixels = material->pixels;
                door_w = material->width;
                door_h = material->height;
                door_stride = material->stride;
                memcpy(s->active_asset_palette16, material->palette16,
                       sizeof(s->active_asset_palette16));
                s->active_asset_palette_hash = material->palette_hash;
                s->active_asset_palette_ready = material->palette_hash != 0u;
            }
            if (((s->source_materials_required && door_pixels && door_w > 0 &&
                  door_h > 0) ||
                 (!s->source_materials_required && door->frame_gdat_index != 0 &&
                dm2_v1_fetch_viewport_local_material(s,
                                                      door->frame_gdat_index,
                                                      &door_pixels,
                                                      &door_w,
                                                      &door_h,
                                                      &door_stride) == 0 &&
                  door_pixels && door_w > 0 && door_h > 0))) {
                DM2_V1_DoorAssetBlit blit;
                /* skproject GRAPHICSSET fields 0x06/0x07/0x09 are the
                 * first boot-bound door-frame images for front, D1C and D2C.
                 * This pass scales them into the current bounded DM2 frame
                 * rectangles; exact DRAW_DUNGEON_GRAPHIC offsets remain open. */
                if (dm2_v1_viewport_door_frame_asset_blit(door,
                                                          door_w,
                                                          door_h,
                                                          door_stride,
                                                          &blit)) {
                    dm2_v1_blit_scaled_material_bitmap_region(
                        s, vp,
                        stride,
                        blit.dst_rect.x,
                        blit.dst_rect.y,
                        blit.dst_rect.w,
                        blit.dst_rect.h,
                        door_pixels,
                        blit.src_rect.x,
                        blit.src_rect.y,
                        blit.src_rect.w,
                        blit.src_rect.h,
                        blit.src_stride,
                        blit.transparent_color,
                        &s->gdat_material_palette_door_frame_consumed_count);
                    ++door_asset_count;
                    s->last_door_frame_asset_blit_valid = 1;
                    s->last_door_frame_asset_blit = blit;
                    s->last_door_frame_asset_src_w = door_w;
                    s->last_door_frame_asset_src_h = door_h;
                    s->last_door_frame_asset_src_stride =
                        door_stride > 0 ? door_stride : door_w;
                    frame_drawn_asset = 1;
                    if (s->source_materials_required) {
                        materials[i][DM2_DOOR_MATERIAL_FRAME].consumed = 1;
                    }
                }
            }
            if (s->source_materials_required &&
                !frame_drawn_asset) {
                dm2_v1_block_source_material(
                    s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR);
            }
        }
        /* SKProject DRAW_DOOR_FRAMES draws its left and right GRAPHICSSET
         * jambs through distinct QUERY_TEMP_PICST calls.  Their RAW4
         * destinations, image offsets, mirror state and local palettes are
         * carried by the M11 door command; never reuse the bounded wall-frame
         * rectangle for this route. */
        if (!custom_wall_button_only && s->source_materials_required &&
            !source_no_frames) {
            for (int side = 0; side < 2; ++side) {
                const int kind = side == 0
                    ? DM2_V1_GDAT_DOOR_SIDE_FRAME_LEFT
                    : DM2_V1_GDAT_DOOR_SIDE_FRAME_RIGHT;
                const DM2_V1_GdatDoorOverlayM11Command *command = NULL;
                const int wanted_gdat = door->side_frame_gdat_index[side];

                if (wanted_gdat == 0) continue;
                if (!s->gdat_door_overlay_material_plan ||
                    s->gdat_scene_colorkey > 0xffu) {
                    dm2_v1_block_source_material(
                        s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR);
                    return;
                }
                for (int j = 0;
                     j < s->gdat_door_overlay_material_plan->command_count;
                     ++j) {
                    const DM2_V1_GdatDoorOverlayM11Command *candidate =
                        &s->gdat_door_overlay_material_plan->commands[j];
                    if (candidate->kind == kind &&
                        candidate->view_square == door->view_square &&
                        candidate->gdat_index == wanted_gdat &&
                        candidate->door_opening_dir == door->door_opening_dir &&
                        candidate->door_state == door->door_state &&
                        candidate->door_open_pct == door->door_open_pct) {
                        command = candidate;
                        break;
                    }
                }
                if (!command || !command->pixels || command->width == 0u ||
                    command->height == 0u || command->source_width == 0u ||
                    command->source_height == 0u || command->rect_width == 0u ||
                    command->rect_height == 0u || !command->geometry_hash ||
                    !command->decoded_hash || !command->palette_hash ||
                    command->source_x + command->source_width > command->width ||
                    command->source_y + command->source_height > command->height ||
                    dm2_v1_weather_pixels_hash(command->pixels,
                                               command->width, command->height,
                                               command->width) !=
                        command->decoded_hash ||
                    dm2_v1_weather_pixels_hash(command->palette16, 16, 1, 16) !=
                        command->palette_hash) {
                    dm2_v1_block_source_material(
                        s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR);
                    return;
                }
                memcpy(s->active_asset_palette16, command->palette16,
                       sizeof(s->active_asset_palette16));
                s->active_asset_palette_hash = command->palette_hash;
                s->active_asset_palette_ready = 1;
                dm2_v1_blit_scaled_material_bitmap_region_ex(
                    s, vp, stride, command->rect_x, command->rect_y,
                    command->rect_width, command->rect_height,
                    command->pixels, command->source_x, command->source_y,
                    command->source_width, command->source_height,
                    command->width, (int)s->gdat_scene_colorkey,
                    command->mirror_flip,
                    &s->gdat_material_palette_door_frame_consumed_count);
                ++door_asset_count;
            }
        }
        if (door->button_gdat_index != 0 &&
            door->button_rect.w > 0 && door->button_rect.h > 0) {
            const uint8_t *button_pixels = NULL;
            int button_w = 0;
            int button_h = 0;
            int button_stride = 0;
            int wall_button_material_bound =
                door->button_source_kind != 2;
            const int direct_g1_wall_button_material =
                door->button_source_kind == 2 &&
                s->g1_scene_wall_button_material_ready &&
                s->g1_scene_wall_button_material_pixels &&
                s->g1_scene_wall_button_material_pixel_hash != 0u &&
                s->g1_scene_wall_button_material_raw_bytes &&
                s->g1_scene_wall_button_material_raw_byte_count != 0u &&
                s->g1_scene_wall_button_material_raw_hash != 0u &&
                s->g1_scene_wall_button_material_receipt_hash != 0u &&
                door->button_gdat_index ==
                    s->g1_scene_wall_button_material_gdat_index &&
                door->wall_button_index ==
                    s->g1_scene_wall_button_material_wall_gfx_index &&
                door->wall_button_field ==
                    s->g1_scene_wall_button_material_field &&
                door->wall_button_x ==
                    s->g1_scene_wall_button_material_map_x &&
                door->wall_button_y ==
                    s->g1_scene_wall_button_material_map_y &&
                door->wall_button_object_id ==
                    s->g1_scene_wall_button_material_object_id;

            /* skproject DRAW_DEFAULT_DOOR_BUTTON reaches the custom button
             * through the current WALL_GFX owner. Do not let the generic
             * view-square helper pick a same-numbered GDAT image unless the
             * direct DB2/DB3 receipt proves that ownership. */
            if (direct_g1_wall_button_material) {
                button_pixels = s->g1_scene_wall_button_material_pixels;
                button_w = s->g1_scene_wall_button_material_width;
                button_h = s->g1_scene_wall_button_material_height;
                button_stride = s->g1_scene_wall_button_material_stride;
                memcpy(s->active_asset_palette16,
                       s->g1_scene_wall_button_material_palette16,
                       sizeof(s->active_asset_palette16));
                s->active_asset_palette_hash =
                    s->g1_scene_wall_button_material_palette_hash;
                s->active_asset_palette_ready = 1;
            } else if (s->source_materials_required) {
                /* Custom buttons are preloaded above from their exact packed
                 * WALL_GFX field-1 key. They share this source-owned local
                 * material slot with normal buttons so M11 never queries it
                 * a second time. */
                const DM2_V1_DoorMaterial *material =
                    &materials[i][DM2_DOOR_MATERIAL_BUTTON];
                button_pixels = material->pixels;
                button_w = material->width;
                button_h = material->height;
                button_stride = material->stride;
                memcpy(s->active_asset_palette16, material->palette16,
                       sizeof(s->active_asset_palette16));
                s->active_asset_palette_hash = material->palette_hash;
                s->active_asset_palette_ready = material->palette_hash != 0u;
            }
            if (direct_g1_wall_button_material ||
                (s->source_materials_required && button_pixels &&
                 button_w > 0 && button_h > 0) ||
                (!s->source_materials_required &&
                 dm2_v1_fetch_viewport_local_material(
                     s, door->button_gdat_index, &button_pixels, &button_w,
                     &button_h, &button_stride) == 0 &&
                 button_pixels && button_w > 0 && button_h > 0)) {
                DM2_V1_DoorAssetBlit blit;
                if (door->button_source_kind == 2) {
                    /* The palette query belongs to this exact image fetch,
                     * so match only after it has populated the active IMG3
                     * palette receipt. */
                    wall_button_material_bound =
                        dm2_v1_wall_button_receipt_matches(
                            s, door, button_w, button_h);
                }
                /* skproject SKWIN/SkWinCore.cpp DRAW_DEFAULT_DOOR_BUTTON
                 * lines ~46243-46264 renders both default door buttons and
                 * custom wall-gfx buttons through the same rectno path. Exact
                 * viewport-cell placement is isolated in
                 * dm2_v1_viewport_door_button_rect_for_square(). */
                if ((!s->source_materials_required ||
                     wall_button_material_bound) &&
                    dm2_v1_viewport_door_button_asset_blit(door,
                                                           button_w,
                                                           button_h,
                                                           button_stride,
                                                           &blit)) {
                    dm2_v1_blit_scaled_material_bitmap_region_ex(
                        s, vp,
                        stride,
                        blit.dst_rect.x,
                        blit.dst_rect.y,
                        blit.dst_rect.w,
                        blit.dst_rect.h,
                        button_pixels,
                        blit.src_rect.x,
                        blit.src_rect.y,
                        blit.src_rect.w,
                        blit.src_rect.h,
                        blit.src_stride,
                        blit.transparent_color,
                        0,
                        &s->gdat_sprite_palette_consumed_count);
                    ++door_button_asset_count;
                    s->last_door_button_asset_blit_valid = 1;
                    s->last_door_button_asset_blit = blit;
                    s->last_door_button_asset_src_w = button_w;
                    s->last_door_button_asset_src_h = button_h;
                    s->last_door_button_asset_src_stride =
                        button_stride > 0 ? button_stride : button_w;
                    button_drawn_asset = 1;
                    if (s->source_materials_required) {
                        materials[i][DM2_DOOR_MATERIAL_BUTTON].consumed = 1;
                    }
                    if (direct_g1_wall_button_material) {
                        ++s->g1_scene_wall_button_material_consumed_count;
                    }
                }
            }
            if (s->source_materials_required &&
                (!wall_button_material_bound || !button_drawn_asset)) {
                dm2_v1_block_source_material(
                    s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR);
            }
        }
    }
    if (s->source_materials_required) {
        for (int kind = 0; kind < DM2_DOOR_MATERIAL_COUNT; ++kind) {
            int complete = 1;
            for (int i = 0; i < plan.door_count; ++i) {
                const DM2_V1_DoorMaterial *material = &materials[i][kind];
                if (material->required && !material->consumed) {
                    complete = 0;
                    break;
                }
            }
            if ((s->last_door_material_required_mask &
                 (uint8_t)(1u << (unsigned)kind)) != 0u && complete) {
                s->last_door_material_consumed_mask |=
                    (uint8_t)(1u << (unsigned)kind);
            }
        }
        if (s->last_door_material_required_mask !=
            s->last_door_material_consumed_mask) {
            dm2_v1_block_source_material(
                s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR);
            return;
        }
    }
    s->asset_door_panel_drawn_count += door_panel_asset_count;
    s->asset_door_overlay_drawn_count += door_overlay_asset_count;
    s->asset_door_frame_drawn_count += door_asset_count;
    s->asset_door_button_drawn_count += door_button_asset_count;
}

/* ── Creatures ───────────────────────────────────────────────────── */

void dm2_v1_render_creatures(DM2_V1_ViewportState *s)
{
    if (!s || !s->framebuffer) return;
    uint8_t *vp = s->framebuffer;
    int stride = s->fb_stride;
    DM2_V1_CreatureRenderPlan plan;

    /* DM2 creature rendering:
     * skproject SKWIN/SkWinCore.cpp lines 10557-10619 routes creature
     * records through QUERY_DUNGEON_MAP_CHIP_PICT(cls1, cls2) before
     * DRAW_CHIP_OF_MAGIC_MAP. This pass asks the boot-owned asset provider
     * for that map-chip bitmap; missing GDAT material remains no-draw. */

    if (!dm2_v1_viewport_build_creature_render_plan(s, &plan)) {
        return;
    }
    s->last_creature_asset_blit_valid = 0;
    s->last_creature_render_valid = 0;
    s->last_creature_draw_order = -1;
    memset(&s->last_creature_asset_render, 0,
           sizeof(s->last_creature_asset_render));
    memset(&s->last_creature_asset_blit, 0,
           sizeof(s->last_creature_asset_blit));
    memset(&s->last_creature_render, 0,
           sizeof(s->last_creature_render));
    s->last_creature_asset_src_w = 0;
    s->last_creature_asset_src_h = 0;
    s->last_creature_asset_src_stride = 0;
    s->creature_material_drawn_count = 0;
    memset(s->creature_material_gdat_indices, 0,
           sizeof(s->creature_material_gdat_indices));

    for (int i = 0; i < plan.creature_count; i++) {
        const DM2_V1_CreatureRender *c = &plan.creatures[i];
        int drawn_asset = 0;

        s->last_creature_render_valid = 1;
        s->last_creature_render = *c;
        s->last_creature_draw_order = i;

        {
            const uint8_t *pixels = NULL;
            int src_w = 0;
            int src_h = 0;
            int src_stride = 0;
            const int direct_g1_scene_material =
                c->source_kind == 2 &&
                s->g1_scene_creature_material_ready &&
                s->g1_scene_creature_material_pixels &&
                s->g1_scene_creature_material_pixel_hash != 0u &&
                c->map_x == s->g1_scene_creature_material_map_x &&
                c->map_y == s->g1_scene_creature_material_map_y &&
                c->creature_type == s->g1_scene_creature_material_type &&
                c->gdat_index == s->g1_scene_creature_material_gdat_index;
            if (direct_g1_scene_material) {
                /* c_map owns QUERY_DUNGEON_MAP_CHIP_PICT before c_gui_vp
                 * draws it.  M10 consumes that bounded handoff directly;
                 * querying the provider again could swap GDAT bytes mid-frame. */
                pixels = s->g1_scene_creature_material_pixels;
                src_w = s->g1_scene_creature_material_width;
                src_h = s->g1_scene_creature_material_height;
                src_stride = s->g1_scene_creature_material_stride;
                memcpy(s->active_asset_palette16,
                       s->g1_scene_creature_material_palette16,
                       sizeof(s->active_asset_palette16));
                s->active_asset_palette_hash =
                    s->g1_scene_creature_material_palette_hash;
                s->active_asset_palette_ready = 1;
            }
            if ((direct_g1_scene_material ||
                 (c->gdat_index != 0 &&
                  dm2_v1_fetch_viewport_local_material(
                      s, c->gdat_index, &pixels, &src_w, &src_h,
                      &src_stride) == 0)) &&
                pixels && src_w > 0 && src_h > 0) {
                DM2_V1_CreatureAssetBlit blit;
                if (c->source_v5_field) {
                    /* The FB/FC/FD V5 field has its own decoded-image and
                     * palette evidence; the F9 map-chip instance receipt
                     * does not apply to it. */
                    if (!s->g1_creature_v5_materials ||
                        !dm2_v1_g1_creature_v5_material_matches(
                            s->g1_creature_v5_materials,
                            c->object_id, c->map_x, c->map_y,
                            c->creature_type,
                            (int)(DM2_V1_VIEWPORT_GFX_CREATURE_DIRECT_FIELD_BASE -
                                  c->gdat_index) &
                                DM2_V1_VIEWPORT_GFX_CREATURE_FIELD_MASK,
                            src_w, src_h,
                            s->active_asset_palette_hash,
                            dm2_v1_viewport_indexed_pixel_hash(
                                pixels, src_w, src_h, src_stride))) {
                        dm2_v1_block_source_material(
                            s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_CREATURE);
                        continue;
                    }
                } else if (c->source_kind == 2 &&
                    (!s->g1_creature_map_chip_materials ||
                     !dm2_v1_g1_creature_map_chip_matches_decoded_instance(
                        s->g1_creature_map_chip_materials,
                        c->object_id, c->map_x, c->map_y,
                        c->direction,
                        c->creature_type, src_w, src_h,
                        s->active_asset_palette_hash))) {
                    dm2_v1_block_source_material(
                        s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_CREATURE);
                    continue;
                }
                if (c->source_kind == 2 &&
                    s->g1_scene_creature_material_ready &&
                    c->map_x == s->g1_scene_creature_material_map_x &&
                    c->map_y == s->g1_scene_creature_material_map_y &&
                    (c->creature_type != s->g1_scene_creature_material_type ||
                     c->gdat_index != s->g1_scene_creature_material_gdat_index ||
                     src_w != s->g1_scene_creature_material_width ||
                     src_h != s->g1_scene_creature_material_height ||
                     src_stride != s->g1_scene_creature_material_stride ||
                     (s->g1_scene_creature_material_pixels &&
                      dm2_v1_viewport_indexed_pixel_hash(
                          pixels, src_w, src_h, src_stride) !=
                          s->g1_scene_creature_material_pixel_hash) ||
                     s->active_asset_palette_hash !=
                         s->g1_scene_creature_material_palette_hash)) {
                    dm2_v1_block_source_material(
                        s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_CREATURE);
                    continue;
                }
                if (dm2_v1_viewport_creature_asset_blit(c,
                                                        src_w,
                                                        src_h,
                                                        src_stride,
                                                        s->party_dir,
                                                        &blit)) {
                    dm2_v1_blit_scaled_material_bitmap_region_ex(
                        s, vp,
                        stride,
                        blit.dst_rect.x,
                        blit.dst_rect.y,
                        blit.dst_rect.w,
                        blit.dst_rect.h,
                        pixels,
                        blit.frame_x,
                        blit.frame_y,
                        blit.frame_w,
                        blit.frame_h,
                        blit.src_stride,
                        blit.transparent_color,
                        blit.flip_mirror,
                        &s->gdat_sprite_palette_consumed_count);
                    ++s->asset_creature_drawn_count;
                    if (c->source_kind == 2 &&
                        s->g1_scene_creature_material_ready &&
                        c->map_x == s->g1_scene_creature_material_map_x &&
                        c->map_y == s->g1_scene_creature_material_map_y) {
                        ++s->g1_scene_creature_material_consumed_count;
                    }
                    s->last_creature_asset_blit_valid = 1;
                    s->last_creature_asset_render = *c;
                    s->last_creature_asset_blit = blit;
                    s->last_creature_asset_blit.draw_order = i;
                    s->last_creature_asset_src_w = src_w;
                    s->last_creature_asset_src_h = src_h;
                    s->last_creature_asset_src_stride =
                        src_stride > 0 ? src_stride : src_w;
                    if (s->creature_material_drawn_count <
                        DM2_MAX_CREATURES_PER_SQ) {
                        s->creature_material_gdat_indices[
                            s->creature_material_drawn_count++] =
                            c->gdat_index;
                    }
                    drawn_asset = 1;
                    if (c->rect14_applied) {
                        /* Count only a rendered source-selected row, not a
                         * parsed table or a speculative creature plan. */
                        ++s->gdat_interface_rect14_consumed_count;
                    }
                }
            }
        }
        if (!drawn_asset) {
            /* skproject DRAW_MAP_CHIP selects a GDAT bitmap.  A coloured
             * substitute would hide a missing source material and produce
             * graphics that the original cannot render. */
            dm2_v1_block_source_material(
                s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_CREATURE);
            continue;
        }
    }
}

/* ── Items ─────────────────────────────────────────────────────────── */

static void dm2_v1_viewport_note_item_material(DM2_V1_ViewportState *s,
                                                int source_kind,
                                                int gdat_index)
{
    int i;

    if (!s || gdat_index == 0 || source_kind < 1 || source_kind > 3) return;
    i = s->item_material_drawn_count;
    if (i < 0 || i >= DM2_MAX_PRESENTED_ITEM_MATERIALS) return;
    s->item_material_gdat_indices[i] = gdat_index;
    s->item_material_source_kinds[i] = (uint8_t)source_kind;
    s->item_material_drawn_count = i + 1;
}

void dm2_v1_render_items(DM2_V1_ViewportState *s)
{
    if (!s || !s->framebuffer) return;
    uint8_t *vp = s->framebuffer;
    int stride = s->fb_stride;
    DM2_V1_ItemRenderPlan plan;

    /* DM2 item rendering:
     * skproject SKWIN/SkWinCore.cpp lines 10523-10549 draws floor items
     * through QUERY_DUNGEON_MAP_CHIP_PICT(cls1, cls2) and
     * DRAW_CHIP_OF_MAGIC_MAP. The category must be carried by the original
     * record; an absent category has no source-owned GDAT image. */

    if (!dm2_v1_viewport_build_item_render_plan(s, &plan)) {
        return;
    }

    for (int i = 0; i < plan.item_count; i++) {
        const DM2_V1_ItemRender *it = &plan.items[i];
        const DM2_V1_G1SceneStaticItemMaterial *direct_static_material =
            dm2_v1_viewport_find_g1_scene_static_item_material(s, it);
        int drawn_asset = 0;
        const int direct_g1_scene_material =
            direct_static_material != NULL ||
            (s->g1_scene_item_material_ready &&
             s->g1_scene_item_material_pixels &&
             s->g1_scene_item_material_pixel_hash != 0u &&
             it->item_category == s->g1_scene_item_material_category &&
             it->item_type == s->g1_scene_item_material_type &&
             it->gdat_index == s->g1_scene_item_material_gdat_index &&
             it->object_id == s->g1_scene_item_material_object_id &&
             it->map_x == s->g1_scene_item_material_map_x &&
             it->map_y == s->g1_scene_item_material_map_y);

        s->last_item_render_valid = 1;
        s->last_item_asset_blit_valid = 0;
        s->last_item_source_kind = 1;
        s->last_item_draw_order = i;
        s->last_item_render = *it;
        memset(&s->last_item_asset_blit, 0, sizeof(s->last_item_asset_blit));
        s->last_item_asset_src_w = 0;
        s->last_item_asset_src_h = 0;
        s->last_item_asset_src_stride = 0;

        if ((it->source_g1_weapon || it->source_g1_container) &&
            (!it->source_static_object_admitted ||
             it->source_static_object_pass < 0 ||
             dm2_v1_viewport_draw_dungeon_tiles_pass_for_cell(
                 it->source_static_object_cell) !=
                 it->source_static_object_pass ||
             /* F9 is DRAW_MAP_CHIP material. DRAW_ITEM selects F0 for DB5
              * and F0/F4 for DB9, and additionally needs the source 5x5
              * visibility mask, slot ordinal, expanded clip rectangle and
              * dtImageOffset. None is owned by this F9 receipt. */
             it->source_gdat_field == 0xf9)) {
            dm2_v1_block_source_material(
                s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_ITEM);
            continue;
        }
        if ((it->source_g1_weapon || it->source_g1_container) &&
            it->source_gdat_field != 0xf9 &&
            (!direct_g1_scene_material ||
             it->source_static_object_clip_rect_id == 0 ||
             it->source_static_object_raw_gfx256_hash == 0u ||
             it->source_static_object_raw_gfx256_receipt_hash == 0u ||
             it->source_static_object_raw4_hash == 0u ||
             it->source_static_object_raw4_receipt_hash == 0u ||
             (!direct_static_material &&
              (it->source_static_object_raw_gfx256_hash !=
                   s->g1_scene_item_material_raw_gfx256_hash ||
               it->source_static_object_raw_gfx256_receipt_hash !=
                   s->g1_scene_item_material_raw_gfx256_receipt_hash ||
               it->source_static_object_raw4_hash !=
                   s->g1_scene_item_material_raw4_hash ||
               it->source_static_object_raw4_receipt_hash !=
                   s->g1_scene_item_material_raw4_receipt_hash)))) {
            dm2_v1_block_source_material(
                s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_ITEM);
            continue;
        }

        {
            const uint8_t *pixels = NULL;
            int src_w = 0;
            int src_h = 0;
            int src_stride = 0;
            if (direct_g1_scene_material) {
                if (direct_static_material) {
                    pixels = direct_static_material->pixels;
                    src_w = direct_static_material->width;
                    src_h = direct_static_material->height;
                    src_stride = direct_static_material->stride;
                    memcpy(s->active_asset_palette16,
                           direct_static_material->palette16,
                           sizeof(s->active_asset_palette16));
                    s->active_asset_palette_hash =
                        direct_static_material->palette_hash;
                } else {
                    pixels = s->g1_scene_item_material_pixels;
                    src_w = s->g1_scene_item_material_width;
                    src_h = s->g1_scene_item_material_height;
                    src_stride = s->g1_scene_item_material_stride;
                    memcpy(s->active_asset_palette16,
                           s->g1_scene_item_material_palette16,
                           sizeof(s->active_asset_palette16));
                    s->active_asset_palette_hash =
                        s->g1_scene_item_material_palette_hash;
                }
                s->active_asset_palette_ready = 1;
            }
            if ((direct_g1_scene_material ||
                 (it->gdat_index != 0 &&
                  dm2_v1_fetch_viewport_local_material(
                      s, it->gdat_index, &pixels, &src_w, &src_h,
                      &src_stride) == 0)) &&
                pixels && src_w > 0 && src_h > 0) {
                DM2_V1_ItemAssetBlit blit;
                if (it->source_g1_weapon && it->source_gdat_field == 0xf9 &&
                    (!s->g1_weapon_map_chip_materials ||
                     !dm2_v1_g1_weapon_map_chip_matches_decoded_instance(
                         s->g1_weapon_map_chip_materials,
                         it->object_id, it->map_x, it->map_y,
                         it->item_type, src_w, src_h,
                         s->active_asset_palette_hash,
                         dm2_v1_viewport_indexed_pixel_hash(
                             pixels, src_w, src_h, src_stride)))) {
                    dm2_v1_block_source_material(
                        s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_ITEM);
                    continue;
                }
                if (it->source_g1_container && it->source_gdat_field == 0xf9 &&
                    (!s->g1_container_map_chip_materials ||
                     !dm2_v1_g1_container_map_chip_matches_decoded_instance(
                         s->g1_container_map_chip_materials,
                         it->object_id, it->map_x, it->map_y,
                         it->item_type, src_w, src_h,
                         s->active_asset_palette_hash,
                         dm2_v1_viewport_indexed_pixel_hash(
                             pixels, src_w, src_h, src_stride)))) {
                    dm2_v1_block_source_material(
                        s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_ITEM);
                    continue;
                }
                if (dm2_v1_viewport_item_asset_blit(it,
                                                    src_w,
                                                    src_h,
                                                    src_stride,
                                                    s->party_dir,
                                                    4,
                                                    32,
                                                    &blit)) {
                    dm2_v1_blit_scaled_material_bitmap_region_ex(
                        s, vp,
                        stride,
                        blit.dst_rect.x,
                        blit.dst_rect.y,
                        blit.dst_rect.w,
                        blit.dst_rect.h,
                        pixels,
                        blit.frame_x,
                        blit.frame_y,
                        blit.frame_w,
                        blit.frame_h,
                        blit.src_stride,
                        blit.transparent_color,
                        blit.flip_mirror,
                        &s->gdat_sprite_palette_consumed_count);
                    ++s->asset_item_drawn_count;
                    dm2_v1_viewport_note_item_material(s, 1, it->gdat_index);
                    s->last_item_asset_blit_valid = 1;
                    s->last_item_asset_blit = blit;
                    s->last_item_asset_blit.draw_order = i;
                    s->last_item_asset_src_w = src_w;
                    s->last_item_asset_src_h = src_h;
                    s->last_item_asset_src_stride =
                        src_stride > 0 ? src_stride : src_w;
                    if (direct_g1_scene_material) {
                        if (direct_static_material) {
                            ++s->g1_scene_static_item_material_consumed_count;
                        } else {
                            ++s->g1_scene_item_material_consumed_count;
                        }
                    }
                    drawn_asset = 1;
                }
            }
        }
        if (!drawn_asset) {
            /* DRAW_MAP_CHIP draws only the queried GDAT image. */
            dm2_v1_block_source_material(
                s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_ITEM);
            continue;
        }
    }
}

void dm2_v1_render_creature_possession_items(DM2_V1_ViewportState *s)
{
    DM2_V1_CreaturePossessionItemRenderPlan plan;
    uint8_t *vp;
    int stride;

    if (!s || !s->framebuffer) return;
    vp = s->framebuffer;
    stride = s->fb_stride;

    /* skproject SKWIN/SkWinCore.cpp DRAW_MAP_CHIP lines 10782-10817:
     * after drawing a creature, source-visible carried/embedded weapon..misc
     * records are rendered with QUERY_DUNGEON_MAP_CHIP_PICT and
     * DRAW_CHIP_OF_MAGIC_MAP(frame=0, flip=_4976_3fa4[(Dir-viewDir)&3]).
     * Runtime possession-chain extraction is a separate bridge; this pass is
     * the renderer-owned hook that keeps those overlays in source order. */
    if (!dm2_v1_viewport_build_creature_possession_item_render_plan(s,
                                                                    &plan)) {
        return;
    }
    for (int i = 0; i < plan.item_count; ++i) {
        const DM2_V1_ItemRender *it = &plan.items[i];
        int drawn_asset = 0;

        s->last_item_render_valid = 1;
        s->last_item_asset_blit_valid = 0;
        s->last_item_source_kind = 2;
        s->last_item_draw_order = i;
        s->last_item_render = *it;
        memset(&s->last_item_asset_blit, 0, sizeof(s->last_item_asset_blit));
        s->last_item_asset_src_w = 0;
        s->last_item_asset_src_h = 0;
        s->last_item_asset_src_stride = 0;

        {
            const uint8_t *pixels = NULL;
            int src_w = 0;
            int src_h = 0;
            int src_stride = 0;
            if (it->gdat_index != 0 &&
                dm2_v1_fetch_viewport_local_material(
                    s, it->gdat_index, &pixels, &src_w, &src_h,
                    &src_stride) == 0 &&
                pixels && src_w > 0 && src_h > 0) {
                DM2_V1_ItemAssetBlit blit;
                if (dm2_v1_viewport_item_asset_blit(it,
                                                    src_w,
                                                    src_h,
                                                    src_stride,
                                                    s->party_dir,
                                                    4,
                                                    32,
                                                    &blit)) {
                    dm2_v1_blit_scaled_material_bitmap_region_ex(
                        s, vp,
                        stride,
                        blit.dst_rect.x,
                        blit.dst_rect.y,
                        blit.dst_rect.w,
                        blit.dst_rect.h,
                        pixels,
                        blit.frame_x,
                        blit.frame_y,
                        blit.frame_w,
                        blit.frame_h,
                        blit.src_stride,
                        blit.transparent_color,
                        blit.flip_mirror,
                        &s->gdat_sprite_palette_consumed_count);
                    ++s->asset_creature_possession_item_drawn_count;
                    dm2_v1_viewport_note_item_material(s, 2, it->gdat_index);
                    s->last_item_asset_blit_valid = 1;
                    s->last_item_asset_blit = blit;
                    s->last_item_asset_blit.draw_order = i;
                    s->last_item_asset_src_w = src_w;
                    s->last_item_asset_src_h = src_h;
                    s->last_item_asset_src_stride =
                        src_stride > 0 ? src_stride : src_w;
                    drawn_asset = 1;
                }
            }
        }

        if (!drawn_asset) {
            /* Creature possession uses the same source-owned map-chip
             * route.  Do not invent a diamond when the GDAT material fails. */
            dm2_v1_block_source_material(
                s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_POSSESSION);
            continue;
        }
    }
}

void dm2_v1_render_carried_item(DM2_V1_ViewportState *s)
{
    DM2_V1_CarriedItemRenderPlan plan;
    const DM2_V1_ItemRender *it;
    int drawn_asset = 0;
    uint8_t *vp;
    int stride;

    if (!s || !s->framebuffer) return;

    /* skproject SKWIN/SkWinCore.cpp DRAW_ITEM_IN_HAND lines 15753-15814
     * renders glbLeaderHandPossession from the object's GDAT class/type and
     * selected image into the leader-hand cursor buffer. Firestaff does not
     * expose that cursor buffer in this renderer yet, so the runtime binds
     * the carried object as a bounded viewport overlay using the same
     * item-map-chip asset path as floor objects. */
    if (!dm2_v1_viewport_build_carried_item_render_plan(s, &plan) ||
        !plan.item_present) {
        return;
    }

    it = &plan.item;
    vp = s->framebuffer;
    stride = s->fb_stride;
    s->last_item_render_valid = 1;
    s->last_item_asset_blit_valid = 0;
    s->last_item_source_kind = 3;
    s->last_item_draw_order = 0;
    s->last_item_render = *it;
    memset(&s->last_item_asset_blit, 0, sizeof(s->last_item_asset_blit));
    s->last_item_asset_src_w = 0;
    s->last_item_asset_src_h = 0;
    s->last_item_asset_src_stride = 0;

    {
        const uint8_t *pixels = NULL;
        int src_w = 0;
        int src_h = 0;
        int src_stride = 0;
        if (it->gdat_index != 0 &&
            dm2_v1_fetch_viewport_local_material(
                s, it->gdat_index, &pixels, &src_w, &src_h,
                &src_stride) == 0 &&
            pixels && src_w > 0 && src_h > 0) {
            DM2_V1_ItemAssetBlit blit;
            if (dm2_v1_viewport_item_asset_blit(it,
                                                src_w,
                                                src_h,
                                                src_stride,
                                                s->party_dir,
                                                8,
                                                40,
                                                &blit)) {
                dm2_v1_blit_scaled_material_bitmap_region_ex(
                    s, vp,
                    stride,
                    blit.dst_rect.x,
                    blit.dst_rect.y,
                    blit.dst_rect.w,
                    blit.dst_rect.h,
                    pixels,
                    blit.frame_x,
                    blit.frame_y,
                    blit.frame_w,
                    blit.frame_h,
                    blit.src_stride,
                    blit.transparent_color,
                    blit.flip_mirror,
                    &s->gdat_sprite_palette_consumed_count);
                ++s->asset_carried_item_drawn_count;
                dm2_v1_viewport_note_item_material(s, 3, it->gdat_index);
                s->last_item_asset_blit_valid = 1;
                s->last_item_asset_blit = blit;
                s->last_item_asset_blit.draw_order = 0;
                s->last_item_asset_src_w = src_w;
                s->last_item_asset_src_h = src_h;
                s->last_item_asset_src_stride =
                    src_stride > 0 ? src_stride : src_w;
                drawn_asset = 1;
            }
        }
    }

    if (!drawn_asset) {
        /* DRAW_ITEM_IN_HAND uses the object's selected GDAT image. */
        dm2_v1_block_source_material(
            s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_CARRIED_ITEM);
        return;
    }
}

/* ── Projectiles ──────────────────────────────────────────────────── */

void dm2_v1_render_projectiles(DM2_V1_ViewportState *s)
{
    if (!s || !s->framebuffer) return;
    uint8_t *vp = s->framebuffer;
    int stride = s->fb_stride;
    DM2_V1_ProjectileRenderPlan plan;

    /* DM2 projectile rendering:
     * skproject SKWIN/SkWinCore.cpp lines 10672-10750 routes missiles and
     * clouds through QUERY_DUNGEON_MAP_CHIP_PICT before DRAW_CHIP_OF_MAGIC_MAP.
     * The runtime drain must supply that original GDAT category/type pair;
     * unmapped graphics have no source-owned draw. */

    if (!dm2_v1_viewport_build_projectile_render_plan(s, &plan)) {
        return;
    }

    for (int i = 0; i < plan.projectile_count; i++) {
        const DM2_V1_ProjectileRender *p = &plan.projectiles[i];
        int drawn_asset = 0;

        s->last_projectile_render_valid = 1;
        s->last_projectile_asset_blit_valid = 0;
        s->last_projectile_draw_order = i;
        s->last_projectile_render = *p;
        memset(&s->last_projectile_asset_blit, 0,
               sizeof(s->last_projectile_asset_blit));
        s->last_projectile_asset_src_w = 0;
        s->last_projectile_asset_src_h = 0;
        s->last_projectile_asset_src_stride = 0;

        {
            const uint8_t *pixels = NULL;
            int src_w = 0;
            int src_h = 0;
            int src_stride = 0;
            if (p->gdat_index != 0 &&
                dm2_v1_fetch_viewport_local_material(
                    s, p->gdat_index, &pixels, &src_w, &src_h,
                    &src_stride) == 0 &&
                pixels && src_w > 0 && src_h > 0) {
                DM2_V1_ProjectileAssetBlit blit;
                if (dm2_v1_viewport_projectile_asset_blit(
                        p,
                        src_w,
                        src_h,
                        src_stride,
                        s->party_dir,
                        s->tick_count,
                        &s->random_seed,
                        &blit)) {
                    dm2_v1_blit_scaled_material_bitmap_region_ex(
                        s, vp,
                        stride,
                        blit.dst_rect.x,
                        blit.dst_rect.y,
                        blit.dst_rect.w,
                        blit.dst_rect.h,
                        pixels,
                        blit.frame_x,
                        blit.frame_y,
                        blit.frame_w,
                        blit.frame_h,
                        blit.src_stride,
                        blit.transparent_color,
                        blit.flip_mirror,
                        &s->gdat_sprite_palette_consumed_count);
                    ++s->asset_projectile_drawn_count;
                    if (s->projectile_material_drawn_count <
                        DM2_MAX_PROJECTILES) {
                        s->projectile_material_gdat_indices[
                            s->projectile_material_drawn_count++] =
                            p->gdat_index;
                    }
                    s->last_projectile_asset_blit_valid = 1;
                    s->last_projectile_asset_blit = blit;
                    s->last_projectile_asset_blit.draw_order = i;
                    s->last_projectile_asset_src_w = src_w;
                    s->last_projectile_asset_src_h = src_h;
                    s->last_projectile_asset_src_stride =
                        src_stride > 0 ? src_stride : src_w;
                    drawn_asset = 1;
                }
            }
        }
        if (!drawn_asset) {
            /* Missile/cloud map chips have no source-independent visual. */
            dm2_v1_block_source_material(
                s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_PROJECTILE);
            continue;
        }
    }
}

/* ── Weather overlay ──────────────────────────────────────────────── */

void dm2_v1_render_weather_overlay(DM2_V1_ViewportState *s)
{
    const DM2_V1_WeatherRendererReceipt *receipt;
    const uint8_t *pixels[DM2_V1_WEATHER_MAX_SLOTS] = { NULL, NULL, NULL };
    int widths[DM2_V1_WEATHER_MAX_SLOTS] = { 0, 0, 0 };
    int heights[DM2_V1_WEATHER_MAX_SLOTS] = { 0, 0, 0 };
    int strides[DM2_V1_WEATHER_MAX_SLOTS] = { 0, 0, 0 };
    unsigned int i;

    if (!s) return;
    s->gdat_weather_renderer_consumed_hash = 0u;
    s->gdat_weather_renderer_consumed_command_count = 0u;
    if (!s->is_outdoor || !s->gdat_weather_renderer_receipt) return;
    receipt = s->gdat_weather_renderer_receipt;
    if (receipt->command_count == 0u) return;

    /* skproject c_weather.cpp:221-266 fills cloud then rain slots; each is
     * realized through c_querydb.cpp DM2_QUERY_TEMP_PICST:2381. Preflight
     * the complete source transaction so a later missing ENVIRONMENT image
     * cannot leave an earlier cloud layer on the frame. */
    for (i = 0u; i < receipt->command_count; ++i) {
        const DM2_V1_WeatherDrawPlan *draw = &receipt->draws[i];
        const DM2_V1_WeatherDestinationClip *clip = &receipt->clips[i];
        int gdat_index;
        if (!draw->valid || !clip->valid || draw->material_hash == 0u ||
            draw->image_field < DM2_V1_WEATHER_BOLT_CMD_BASE ||
            draw->image_field > DM2_V1_WEATHER_RAIN_STORM_CMD ||
            draw->source_right <= draw->source_left ||
            draw->source_bottom <= draw->source_top ||
            draw->scale_x == 0u || draw->scale_x > 0x40u ||
            draw->scale_y == 0u || draw->scale_y > 0x40u ||
            clip->w <= 0 || clip->h <= 0 ||
            dm2_v1_viewport_calc_stretched_size(clip->w, draw->scale_x) <= 0 ||
            dm2_v1_viewport_calc_stretched_size(clip->h, draw->scale_y) <= 0) {
            dm2_v1_block_source_material(
                s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WEATHER);
            return;
        }
        gdat_index = dm2_v1_viewport_weather_environment_graphic_index(
            s->gdat_weather_renderer_graphicsset, draw->image_field);
        if (gdat_index == 0 || dm2_v1_fetch_viewport_local_material(
                s, gdat_index, &pixels[i], &widths[i], &heights[i],
                &strides[i]) != 0 || !pixels[i] ||
            widths[i] != draw->source_right - draw->source_left ||
            heights[i] != draw->source_bottom - draw->source_top ||
            strides[i] < widths[i] ||
            draw->decoded_pixel_count !=
                (uint32_t)((size_t)widths[i] * (size_t)heights[i]) ||
            dm2_v1_weather_pixels_hash(pixels[i], widths[i], heights[i],
                                       strides[i]) !=
                draw->decoded_pixels_hash ||
            s->active_asset_palette_hash != draw->local_palette_hash) {
            dm2_v1_block_source_material(
                s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WEATHER);
            return;
        }
    }
    for (i = 0u; i < receipt->command_count; ++i) {
        const DM2_V1_WeatherDrawPlan *draw = &receipt->draws[i];
        const DM2_V1_WeatherDestinationClip *clip = &receipt->clips[i];
        int dst_w = dm2_v1_viewport_calc_stretched_size(
            clip->w, draw->scale_x);
        int dst_h = dm2_v1_viewport_calc_stretched_size(
            clip->h, draw->scale_y);

        /* skproject c_bkgrnd.cpp::ENVIRONMENT_DRAW_DISTANT_ELEMENT passes
         * the source slot's offset, stretch and FW-selected mirror through
         * DRAW_TEMP_PICST.  The receipt has already bound its original CD
         * clip and IMG3 bounds; consume those exact transform values here
         * instead of silently treating every layer as an unmirrored 1:1 blit.
         */
        dm2_v1_blit_scaled_material_bitmap_region_ex(
            s, s->framebuffer, s->fb_stride,
            clip->x + draw->draw_offset_x, clip->y + draw->draw_offset_y,
            dst_w, dst_h, pixels[i], 0, 0, widths[i], heights[i], strides[i],
            DM2_COLOR_TRANSPARENT, draw->mirror_flip ? 1 : 0,
            &s->gdat_scene_weather_consumed_count);
        ++s->asset_weather_drawn_count;
    }
    s->gdat_weather_renderer_consumed_hash = receipt->renderer_hash;
    s->gdat_weather_renderer_consumed_command_count = receipt->command_count;
}

/* ── UI Chrome ────────────────────────────────────────────────────── */

static uint32_t dm2_v1_viewport_hash_gdat_asset(uint32_t hash,
                                                int gdat_index,
                                                int w,
                                                int h)
{
    hash ^= (uint32_t)gdat_index;
    hash *= 16777619u;
    hash ^= (uint32_t)w;
    hash *= 16777619u;
    hash ^= (uint32_t)h;
    hash *= 16777619u;
    return hash;
}

static uint8_t dm2_v1_hud_palette_color(DM2_V1_ViewportState *s,
                                        uint8_t logical_color)
{
    if (!s || !s->gdat_interface_palette_ready || logical_color >= 16u) {
        return logical_color;
    }
    ++s->gdat_interface_palette_consumed_count;
    return s->gdat_interface_palette16[logical_color];
}

static uint8_t dm2_v1_hud_text_palette_color(DM2_V1_ViewportState *s,
                                             uint8_t logical_color)
{
    if (!s || logical_color >= 16u) return logical_color;
    if (s->gdat_interface_text_palette_ready) {
        ++s->gdat_interface_action_palette_consumed_count;
        return s->gdat_interface_text_palette16[logical_color];
    }
    return dm2_v1_hud_palette_color(s, logical_color);
}

static int dm2_v1_render_hud_source_font(
    DM2_V1_ViewportState *s,
    const DM2_V1_ViewportRect *rect,
    const char *text,
    uint8_t foreground,
    uint8_t background,
    int transparent_background)
{
    int glyph_count = 0;

    if (!s || !s->framebuffer || !rect || !text || !text[0] ||
        !s->gdat_interface_font_rows || s->gdat_interface_font_hash == 0u) {
        return 0;
    }
    /* skproject DRAW_STRING dispatches high-bit text through DRAW_MBCS_STR.
     * The canonical PC G1 graphics corpus has no category-0x1c font entry,
     * so QUERY_CHAR_METRICS returns NULL and the original consumes the byte
     * without blitting a substitute glyph.  Keep that source-owned no-draw
     * outcome: the ASCII dt07/0 table is not a valid replacement font. */
    for (int glyph = 0; text[glyph] && glyph < 80; ++glyph) {
        if ((unsigned char)text[glyph] & 0x80u) return 1;
    }
    /* skproject/SKWIN/SkWinCore.cpp QUERY_FONT expands each dt07/0 byte
     * into three pixels in the order 0x10, 0x04, 0x01 for six rows. */
    for (int glyph = 0; text[glyph] && glyph < 80; ++glyph) {
        unsigned char character = (unsigned char)text[glyph];
        if (character >= 128u || rect->x + glyph * 3 >= rect->x + rect->w) {
            break;
        }
        for (int row = 0; row < 6; ++row) {
            uint8_t bits = s->gdat_interface_font_rows[row * 128 + character];
            for (int column = 0; column < 3; ++column) {
                DM2_V1_ViewportRect pixel = {
                    rect->x + glyph * 3 + column, rect->y + row, 1, 1
                };
                if (bits & (0x10u >> (column * 2))) {
                    dm2_v1_fill_rect(s->framebuffer, s->fb_stride, &pixel,
                        dm2_v1_hud_text_palette_color(s, foreground));
                } else if (!transparent_background) {
                    dm2_v1_fill_rect(s->framebuffer, s->fb_stride, &pixel,
                        dm2_v1_hud_text_palette_color(s, background));
                }
            }
        }
        ++glyph_count;
    }
    s->gdat_interface_font_consumed_count += glyph_count;
    return glyph_count > 0;
}

static const DM2_V1_GdatHudM11Command *dm2_v1_hud_plan_command(
    DM2_V1_ViewportState *s, int gdat_index,
    const DM2_V1_ViewportRect *rect, int kind)
{
    const DM2_V1_GdatHudM11CommandPlan *plan;

    if (!s || !rect || !(plan = s->gdat_hud_material_plan) ||
        !plan->valid ||
        plan->command_count < DM2_V1_GDAT_HUD_M11_COMMAND_MAX -
                                  DM2_V1_HUD_CHAMPION_SLOT_COUNT - 1 ||
        plan->command_count > DM2_V1_GDAT_HUD_M11_COMMAND_MAX ||
        plan->command_hash == 0u ||
        plan->command_hash !=
            dm2_v1_gdat_hud_m11_command_plan_hash(plan)) {
        return NULL;
    }
    for (int i = 0; i < plan->command_count; ++i) {
        const DM2_V1_GdatHudM11Command *command = &plan->commands[i];
        int dest_match = kind == DM2_V1_GDAT_HUD_M11_COMMAND_CHAMPION_PORTRAIT ||
            (command->destination.x == rect->x && command->destination.y == rect->y &&
             command->destination.w == rect->w && command->destination.h == rect->h);
        int pixel_ok = command->pixels && command->width > 0 && command->height > 0 &&
            command->palette_hash != 0u && command->decoded_hash != 0u &&
            command->material_source_bytes &&
            command->material_source_byte_count == command->raw_byte_count &&
            command->material_receipt_hash != 0u;
        int hash_ok = command->decoded_hash ==
                dm2_v1_gdat_hud_m11_command_pixel_hash(command) &&
            command->palette_hash ==
                dm2_v1_weather_pixels_hash(command->palette16, 16, 1, 16);
        if (command->kind == kind && command->viewport_gdat_index == gdat_index &&
            dest_match && pixel_ok && hash_ok) {
            return command;
        }
    }
    return NULL;
}

static int dm2_v1_render_hud_plan_command(DM2_V1_ViewportState *s,
                                           int gdat_index,
                                           const DM2_V1_ViewportRect *rect,
                                           int kind)
{
    const DM2_V1_GdatHudM11Command *command =
        dm2_v1_hud_plan_command(s, gdat_index, rect, kind);
    if (!command) return 0;
    memcpy(s->active_asset_palette16, command->palette16,
           sizeof(s->active_asset_palette16));
    s->active_asset_palette_hash = command->palette_hash;
    s->active_asset_palette_ready = 1;
    dm2_v1_blit_scaled_material_bitmap(s, s->framebuffer, s->fb_stride,
        rect->x, rect->y, rect->w, rect->h, command->pixels,
        command->width, command->height, command->width,
        DM2_COLOR_TRANSPARENT, &s->gdat_interface_palette_consumed_count);
    ++s->gdat_hud_material_plan_consumed_count;
    return 1;
}

static int dm2_v1_render_hud_core_asset(DM2_V1_ViewportState *s,
                                        const DM2_V1_ViewportRect *rect,
                                        int gdat_index)
{
    const uint8_t *pixels = NULL;
    DM2_V1_ViewportHudMaterialRequest request;
    DM2_V1_ViewportHudPresentationCommand command;
    int w = 0;
    int h = 0;
    int stride = 0;
    int field;
    if (!s || !s->framebuffer || !rect || rect->w <= 0 || rect->h <= 0 ||
        gdat_index == 0) return 0;
    if (s->source_materials_required && s->gdat_hud_material_plan) {
        if (!dm2_v1_render_hud_plan_command(s, gdat_index, rect,
                                             gdat_index == dm2_v1_viewport_hud_core_graphic_index(DM2_V1_VIEWPORT_GFX_HUD_CORE_TOP_BAR) ? DM2_V1_GDAT_HUD_M11_COMMAND_TOP_BAR :
                                             gdat_index == dm2_v1_viewport_hud_core_graphic_index(DM2_V1_VIEWPORT_GFX_HUD_CORE_ACTION_STRIP) ? DM2_V1_GDAT_HUD_M11_COMMAND_ACTION_STRIP :
                                             gdat_index == dm2_v1_viewport_hud_core_graphic_index(DM2_V1_VIEWPORT_GFX_HUD_CORE_GOLD_BOX) ? DM2_V1_GDAT_HUD_M11_COMMAND_GOLD_BOX :
                                             gdat_index == dm2_v1_viewport_hud_core_graphic_index(DM2_V1_VIEWPORT_GFX_HUD_CORE_PORTRAIT_PANEL) ? DM2_V1_GDAT_HUD_M11_COMMAND_PORTRAIT_PANEL : DM2_V1_GDAT_HUD_M11_COMMAND_ACTION_ICON)) return 0;
    } else if (
        dm2_v1_fetch_viewport_asset(s,
                                    gdat_index,
                                    &pixels,
                                    &w,
                                    &h,
                                    &stride) != 0 ||
        !pixels || w <= 0 || h <= 0) {
        return 0;
    }
    if (!s->source_materials_required || !s->gdat_hud_material_plan) dm2_v1_blit_scaled_material_bitmap(s,
                              s->framebuffer,
                              s->fb_stride,
                              rect->x,
                              rect->y,
                              rect->w,
                              rect->h,
                              pixels,
                              w,
                              h,
                              stride > 0 ? stride : w,
                              DM2_COLOR_TRANSPARENT,
                              /* skproject LOAD_GDAT_INTERFACE_00_02
                               * initializes the INTERFACE_GENERAL palette
                               * used by these chrome images.  Keep its
                               * consumption distinct from map-chip sprite
                               * palettes in the frame ownership receipt. */
                              &s->gdat_interface_palette_consumed_count);
    /* skproject binds dtPalIRGB/dtPalette16 before selecting the chrome
     * bitmap.  Count that source palette binding even if this particular
     * image contains no logical index below 16 (where the pixel loop above
     * would otherwise have no individual remap to count). */
    if (s->gdat_interface_palette_ready) {
        ++s->gdat_interface_palette_consumed_count;
    }
    ++s->asset_hud_core_drawn_count;
    s->last_hud_core_gdat_hash =
        dm2_v1_viewport_hash_gdat_asset(s->last_hud_core_gdat_hash,
                                        gdat_index,
                                        w,
                                        h);
    s->last_hud_core_pixel_count += (uint32_t)(rect->w * rect->h);
    field = DM2_V1_VIEWPORT_GFX_HUD_CORE_FIELD_BASE - gdat_index;
    if (s->source_materials_required &&
        (field == DM2_V1_VIEWPORT_GFX_HUD_CORE_TOP_BAR ||
         field == DM2_V1_VIEWPORT_GFX_HUD_CORE_PORTRAIT_PANEL)) {
        memset(&request, 0, sizeof(request));
        request.valid = s->gdat_interface_palette_ready &&
            s->gdat_interface_palette_hash != 0u &&
            field >= 0 && field <= DM2_V1_VIEWPORT_GFX_HUD_CORE_FIELD_MASK;
        request.gdat_index = gdat_index;
        request.gdat_category = DM2_GDAT_CATEGORY_INTERFACE_GENERAL;
        request.gdat_subcategory = 0u;
        request.gdat_entry = (uint8_t)field;
        request.field = (uint8_t)field;
        request.indexed_pixels = pixels;
        request.palette16 = s->gdat_interface_palette16;
        request.palette_hash = s->gdat_interface_palette_hash;
        request.palette_entry_count = 16;
        request.width = w;
        request.height = h;
        request.stride = stride > 0 ? stride : w;
        request.transparent_color = DM2_COLOR_TRANSPARENT;
        request.colorkey_palette_index =
            request.palette16[request.transparent_color];
        request.source_rect = (DM2_V1_ViewportRect){ 0, 0, w, h };
        request.destination_rect = *rect;
        memset(&command, 0, sizeof(command));
        command.valid = request.valid && request.indexed_pixels &&
            request.palette16 && request.width > 0 && request.height > 0 &&
            request.stride >= request.width;
        command.material = request;
        command.indexed_pixels = request.indexed_pixels;
        command.palette16 = request.palette16;
        command.transparent_color = request.transparent_color;
        command.source_rect = request.source_rect;
        command.destination_rect = request.destination_rect;
        if (field == DM2_V1_VIEWPORT_GFX_HUD_CORE_TOP_BAR) {
            s->last_hud_top_bar_material_request = request;
            s->last_hud_top_bar_presentation_command = command;
        } else {
            s->last_hud_status_panel_material_request = request;
            s->last_hud_status_panel_presentation_command = command;
        }
    }
    return 1;
}

/* ReDMCSB/skproject SKWINSPX/src/v4/skguidrw.cpp DRAW_HAND_ACTION_ICONS
 * selects INTERFACE_GENERAL/4 entry (possession<<1)+side+2 and expands the
 * matching 0x46/0x4a rectangle before DRAW_ICON_PICT_ENTRY. */
static int dm2_v1_render_hud_hand_action_asset(DM2_V1_ViewportState *s)
{
    const DM2_V1_HudHandActionSource *source;
    DM2_V1_ViewportHudMaterialRequest request;
    DM2_V1_ViewportHudPresentationCommand command;
    const uint8_t *pixels = NULL;
    int width = 0;
    int height = 0;
    int stride = 0;
    int gdat_index;

    if (!s || !s->framebuffer) return 0;
    source = &s->hud_hand_action_source;
    if (!source->valid || !s->gdat_interface_palette_ready ||
        s->gdat_interface_palette_hash == 0u || !s->hud_party_valid ||
        source->player_index >= (uint8_t)s->hud_party.champion_count ||
        !s->hud_party.champions[source->player_index].occupied ||
        source->map_load_token != s->gdat_scene_map_load_token ||
        source->scene_control_hash != s->gdat_scene_control_hash ||
        source->palette_hash != s->gdat_interface_palette_hash) {
        return 0;
    }
    gdat_index = dm2_v1_viewport_hud_hand_action_graphic_index(
        source->possession_index, source->left_or_right);
    if (gdat_index == 0 ||
        dm2_v1_fetch_viewport_asset(s, gdat_index, &pixels, &width, &height,
                                    &stride) != 0 ||
        !pixels || width <= 0 || height <= 0 || stride < width) {
        return 0;
    }
    dm2_v1_blit_scaled_material_bitmap(
        s, s->framebuffer, s->fb_stride, source->destination_rect.x,
        source->destination_rect.y, source->destination_rect.w,
        source->destination_rect.h, pixels, width, height, stride,
        DM2_COLOR_TRANSPARENT, &s->gdat_interface_palette_consumed_count);
    memset(&request, 0, sizeof(request));
    request.valid = 1;
    request.gdat_index = gdat_index;
    request.gdat_category = source->gdat_category;
    request.gdat_subcategory = source->gdat_subcategory;
    request.gdat_entry = source->gdat_entry;
    request.field = source->gdat_entry;
    request.indexed_pixels = pixels;
    request.palette16 = s->gdat_interface_palette16;
    request.palette_hash = s->gdat_interface_palette_hash;
    request.palette_entry_count = 16;
    request.width = width;
    request.height = height;
    request.stride = stride;
    request.transparent_color = DM2_COLOR_TRANSPARENT;
    request.colorkey_palette_index =
        request.palette16[request.transparent_color];
    request.source_rect = (DM2_V1_ViewportRect){ 0, 0, width, height };
    request.destination_rect = source->destination_rect;
    memset(&command, 0, sizeof(command));
    command.valid = 1;
    command.material = request;
    command.indexed_pixels = request.indexed_pixels;
    command.palette16 = request.palette16;
    command.transparent_color = request.transparent_color;
    command.source_rect = request.source_rect;
    command.destination_rect = request.destination_rect;
    s->last_hud_hand_action_material_request = request;
    s->last_hud_hand_action_presentation_command = command;
    return 1;
}

/* skproject/SKWIN/SkWinCore.cpp CHECK_RECOMPUTE_LIGHT binds map-local
 * GRAPHICSSET light before later dungeon draws, including door frames. */
static int dm2_v1_viewport_frame_light_palette_owned(
    const DM2_V1_ViewportState *s)
{
    const DM2_V1_GraphicsSetStaticSceneReceipt *scene;

    if (!s || !s->gdat_scene_control_ready ||
        s->gdat_scene_map_load_token == 0u ||
        s->gdat_scene_control_hash == 0u ||
        !s->gdat_interface_palette_ready ||
        s->gdat_interface_palette_hash == 0u) {
        return 0;
    }
    scene = &s->gdat_static_scene_record;
    return scene->valid &&
        scene->map_load_token == s->gdat_scene_map_load_token &&
        scene->scene_control_hash == s->gdat_scene_control_hash &&
        s->gdat_static_light_control_owned &&
        s->gdat_static_light_map_load_token == s->gdat_scene_map_load_token &&
        s->gdat_static_light_scene_control_hash ==
            s->gdat_scene_control_hash;
}

static int dm2_v1_viewport_scene_control_command(
    const DM2_V1_ViewportState *s,
    DM2_V1_ViewportSceneControlCommand *out_command)
{
    const DM2_V1_GraphicsSetStaticSceneReceipt *scene;

    if (!out_command) return 0;
    memset(out_command, 0, sizeof(*out_command));
    if (!s || !s->source_materials_required ||
        !dm2_v1_viewport_frame_light_palette_owned(s) ||
        s->gdat_scene_colorkey > 15u ||
        !s->gdat_static_ambient_darkness_control_owned ||
        s->gdat_static_ambient_darkness_map_load_token !=
            s->gdat_scene_map_load_token ||
        s->gdat_static_ambient_darkness_scene_control_hash !=
            s->gdat_scene_control_hash ||
        !s->gdat_static_scene_colorkey_control_owned ||
        s->gdat_static_scene_colorkey_map_load_token !=
            s->gdat_scene_map_load_token ||
        s->gdat_static_scene_colorkey_scene_control_hash !=
            s->gdat_scene_control_hash) {
        return 0;
    }
    scene = &s->gdat_static_scene_record;
    if (!scene->valid || scene->map_load_token != s->gdat_scene_map_load_token ||
        scene->scene_control_hash != s->gdat_scene_control_hash ||
        scene->graphicsset != (uint8_t)s->gdat_scene_material_index ||
        scene->material_category != DM2_GDAT_CATEGORY_GRAPHICSSET ||
        scene->ambient_darkness != s->gdat_ambient_darkness) {
        return 0;
    }
    out_command->gdat_category = DM2_GDAT_CATEGORY_GRAPHICSSET;
    out_command->graphicsset = scene->graphicsset;
    out_command->field = DM2_GDAT_GFXSET_AMBIANT_DARKNESS;
    out_command->map_load_token = s->gdat_scene_map_load_token;
    out_command->scene_control_hash = s->gdat_scene_control_hash;
    out_command->palette_hash = s->gdat_interface_palette_hash;
    out_command->scene_colorkey = s->gdat_scene_colorkey;
    out_command->colorkey_palette_index =
        s->gdat_interface_palette16[out_command->scene_colorkey];
    out_command->ambient_darkness = s->gdat_ambient_darkness;
    out_command->light_floor = s->gdat_scene_light_floor;
    out_command->walk_path_depth = s->gdat_scene_light_search_depth;
    out_command->light_check_enabled = s->gdat_scene_light_recompute_enabled;
    out_command->valid = out_command->palette_hash != 0u &&
        out_command->colorkey_palette_index ==
            s->gdat_interface_palette16[out_command->scene_colorkey];
    return out_command->valid;
}

void dm2_v1_render_ui_chrome(DM2_V1_ViewportState *s)
{
    DM2_V1_HudChromeRenderPlan plan;
    if (!s || !s->framebuffer) return;
    memset(&s->last_hud_top_bar_material_request, 0,
           sizeof(s->last_hud_top_bar_material_request));
    memset(&s->last_hud_top_bar_presentation_command, 0,
           sizeof(s->last_hud_top_bar_presentation_command));
    memset(&s->last_hud_status_panel_material_request, 0,
           sizeof(s->last_hud_status_panel_material_request));
    memset(&s->last_hud_status_panel_presentation_command, 0,
           sizeof(s->last_hud_status_panel_presentation_command));
    memset(&s->last_hud_hand_action_material_request, 0,
           sizeof(s->last_hud_hand_action_material_request));
    memset(&s->last_hud_hand_action_presentation_command, 0,
           sizeof(s->last_hud_hand_action_presentation_command));
    uint8_t *vp = s->framebuffer;
    int stride = s->fb_stride;

    /* DM2 UI chrome:
     *   Top status bar: 28px (champion health/magic/conditions)
     *   Bottom action strip: 28px (Attack/Cast/Use/Drop/Move icons)
     *   Right portrait panel: 80px wide × 144px (champion portraits)
     *   Gold counter in top bar (DM2 specific — DM1 doesn't have gold display)
     *
     * DM2 portrait panel uses portrait graphics from GRAPHICS.DAT.
     * Source: SKULL.ASM T560 (status bar rendering)
     *         DM2_V1_CompanionUI via dm2_v2_companion_ui.c
     *
     * skproject/SKWIN/SkWinCore.cpp DRAW_CHAMPION_PICTURE (12866-12880)
     * draws decoded CHAMPIONS pixels, then DRAW_PLAYER_3STAT_HEALTH_BAR
     * (12885-12947) overlays live champion state. A real GDAT profile must
     * therefore leave missing static HUD material untouched while retaining
     * the dynamic state overlays below.
     */
    if (!dm2_v1_viewport_build_hud_chrome_plan_for_party(
            s->is_outdoor, s->hud_party_valid ? &s->hud_party : NULL,
            &plan)) {
        return;
    }
    if (!plan.outdoor && s->gdat_interface_hud_layout) {
        /* skproject _098d_1208 expands the original 640-wide dt04/0 rect
         * table. Firestaff's indexed game surface is 320 wide, so consume
         * the source rectangles through the matching half-resolution path. */
        for (int slot = 0; slot < plan.champion_slot_count; ++slot) {
            const DM2_V1_InterfaceHudLayout *layout = s->gdat_interface_hud_layout;
            DM2_V1_HudChampionSlotRender *champ = &plan.champion_slots[slot];
            champ->portrait_rect = (DM2_V1_ViewportRect){
                layout->portrait[slot].x / 2, layout->portrait[slot].y / 2,
                layout->portrait[slot].w / 2, layout->portrait[slot].h / 2 };
            champ->name_marker_rect = (DM2_V1_ViewportRect){
                layout->name[slot].x / 2, layout->name[slot].y / 2,
                layout->name[slot].w / 2, layout->name[slot].h / 2 };
            champ->hp_bar_rect = (DM2_V1_ViewportRect){
                layout->status[slot][0].x / 2, layout->status[slot][0].y / 2,
                layout->status[slot][0].w / 2, layout->status[slot][0].h / 2 };
            champ->stamina_bar_rect = (DM2_V1_ViewportRect){
                layout->status[slot][1].x / 2, layout->status[slot][1].y / 2,
                layout->status[slot][1].w / 2, layout->status[slot][1].h / 2 };
            champ->mana_bar_rect = (DM2_V1_ViewportRect){
                layout->status[slot][2].x / 2, layout->status[slot][2].y / 2,
                layout->status[slot][2].w / 2, layout->status[slot][2].h / 2 };
            champ->hp_fill_rect = dm2_v1_hud_bar_fill(&champ->hp_bar_rect, champ->hp_pct);
            champ->stamina_fill_rect = dm2_v1_hud_bar_fill(&champ->stamina_bar_rect, champ->stamina_pct);
            champ->mana_fill_rect = dm2_v1_hud_bar_fill(&champ->mana_bar_rect, champ->mana_pct);
        }
    }

    if (!dm2_v1_render_hud_core_asset(s,
                                      &plan.top_bar_rect,
                                      plan.top_bar_gdat_index)) {
        dm2_v1_block_source_material(
            s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_HUD_CORE);
    }
    if (!dm2_v1_render_hud_core_asset(s,
                                      &plan.action_strip_rect,
                                      plan.action_strip_gdat_index)) {
        dm2_v1_block_source_material(
            s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_HUD_CORE);
    }
    if (!dm2_v1_render_hud_core_asset(s,
                                      &plan.gold_box_rect,
                                      plan.gold_box_gdat_index)) {
        dm2_v1_block_source_material(
            s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_HUD_CORE);
    }
    for (int i = 0; i < plan.action_icon_count; ++i) {
        if (!dm2_v1_render_hud_core_asset(s,
                                          &plan.action_icons[i].fill_rect,
                                          plan.action_icons[i].gdat_index)) {
            dm2_v1_block_source_material(
                s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_HUD_CORE);
        }
    }

    if (!plan.outdoor) {
        if (!dm2_v1_render_hud_core_asset(s,
                                          &plan.portrait_panel_rect,
                                          plan.portrait_panel_gdat_index)) {
            dm2_v1_block_source_material(
                s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_HUD_CORE);
        }
        for (int slot = 0; slot < plan.champion_slot_count; ++slot) {
            if (plan.champion_slots[slot].occupied) {
                if (s->source_materials_required &&
                    !dm2_v1_viewport_hud_dynamic_overlay_ready(
                        s, &plan.champion_slots[slot])) {
                    /* A live state value without its source dt04/dt07/palette
                     * contract is not drawable HUD material.  In particular,
                     * do not turn health percentages into host-colour bars. */
                    dm2_v1_block_source_material(
                        s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_HUD_CORE);
                    continue;
                }
                const uint8_t *portrait_pixels = NULL;
                int portrait_w = 0;
                int portrait_h = 0;
                int portrait_stride = 0;
                int portrait_gdat =
                    dm2_v1_viewport_hud_portrait_graphic_index(
                        plan.champion_slots[slot].portrait_index);
                const DM2_V1_GdatHudM11Command *portrait_command = NULL;
                /* DRAW_CHAMPION_PICTURE uses glbChampionSquad.HeroType(),
                 * not Firestaff's session-tail portrait ordinal. Until the
                 * original save/session parser binds that field, a real-data
                 * profile must not select a CHAMPIONS image by inference. */
                if (s->source_materials_required &&
                    !plan.champion_slots[slot].portrait_type_source_bound) {
                    dm2_v1_block_source_material(
                        s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_HUD_PORTRAIT);
                } else if (s->source_materials_required &&
                    s->gdat_hud_material_plan &&
                    (portrait_command = dm2_v1_hud_plan_command(
                        s, portrait_gdat,
                        &plan.champion_slots[slot].portrait_rect,
                        DM2_V1_GDAT_HUD_M11_COMMAND_CHAMPION_PORTRAIT)) != NULL) {
                    memcpy(s->active_asset_palette16, portrait_command->palette16,
                           sizeof(s->active_asset_palette16));
                    s->active_asset_palette_hash = portrait_command->palette_hash;
                    s->active_asset_palette_ready = 1;
                    dm2_v1_blit_scaled_material_bitmap(s, vp, stride,
                        portrait_command->destination.x,
                        portrait_command->destination.y,
                        portrait_command->destination.w,
                        portrait_command->destination.h,
                        portrait_command->pixels, portrait_command->width,
                        portrait_command->height, portrait_command->width,
                        DM2_COLOR_TRANSPARENT,
                        &s->gdat_interface_palette_consumed_count);
                    ++s->gdat_hud_material_plan_consumed_count;
                    ++s->asset_hud_portrait_drawn_count;
                } else if (!s->source_materials_required &&
                    portrait_gdat != 0 &&
                    dm2_v1_fetch_viewport_local_material(
                        s, portrait_gdat, &portrait_pixels, &portrait_w,
                        &portrait_h, &portrait_stride) == 0 &&
                    portrait_pixels && portrait_w > 0 && portrait_h > 0 &&
                    portrait_stride >= portrait_w) {
                    dm2_v1_blit_scaled_material_bitmap(s,
                                              vp,
                                              stride,
                                              plan.champion_slots[slot].portrait_rect.x,
                                              plan.champion_slots[slot].portrait_rect.y,
                                              plan.champion_slots[slot].portrait_rect.w,
                                              plan.champion_slots[slot].portrait_rect.h,
                                              portrait_pixels,
                                              portrait_w,
                                              portrait_h,
                                              portrait_stride,
                                              DM2_COLOR_TRANSPARENT,
                                              &s->gdat_interface_palette_consumed_count);
                    if (s->gdat_interface_palette_ready) {
                        ++s->gdat_interface_palette_consumed_count;
                    }
                    ++s->asset_hud_portrait_drawn_count;
                } else {
                    /* DRAW_CHAMPION_PICTURE has only the selected CHAMPIONS
                     * GDAT bitmap route. Do not invent a portrait fill. */
                    dm2_v1_block_source_material(
                        s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_HUD_PORTRAIT);
                }
                if (!dm2_v1_render_hud_source_font(
                        s, &plan.champion_slots[slot].name_marker_rect,
                        plan.champion_slots[slot].name,
                        DM2_COL_WHITE, DM2_COL_BLACK, 0)) {
                    dm2_v1_block_source_material(
                        s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_HUD_CORE);
                }
                dm2_v1_fill_rect(vp, stride,
                                 &plan.champion_slots[slot].hp_bar_rect,
                                 dm2_v1_hud_palette_color(
                                     s, DM2_COL_BLACK));
                dm2_v1_fill_rect(vp, stride,
                                 &plan.champion_slots[slot].hp_fill_rect,
                                 dm2_v1_hud_palette_color(
                                     s, plan.champion_slots[slot]
                                            .stat_bar_color));
                dm2_v1_fill_rect(vp, stride,
                                 &plan.champion_slots[slot].stamina_bar_rect,
                                 dm2_v1_hud_palette_color(
                                     s, DM2_COL_BLACK));
                dm2_v1_fill_rect(vp, stride,
                                 &plan.champion_slots[slot].stamina_fill_rect,
                                 dm2_v1_hud_palette_color(
                                     s, plan.champion_slots[slot]
                                            .stat_bar_color));
                dm2_v1_fill_rect(vp, stride,
                                 &plan.champion_slots[slot].mana_bar_rect,
                                 dm2_v1_hud_palette_color(
                                     s, DM2_COL_BLACK));
                dm2_v1_fill_rect(vp, stride,
                                 &plan.champion_slots[slot].mana_fill_rect,
                                 dm2_v1_hud_palette_color(
                                     s, plan.champion_slots[slot]
                                            .stat_bar_color));
                if (plan.champion_slots[slot].leader) {
                    dm2_v1_fill_rect(
                        vp, stride,
                        &plan.champion_slots[slot].leader_mark_rect,
                        dm2_v1_hud_palette_color(s, DM2_COL_WHITE));
                }
            }
        }
        if (s->hud_hand_action_source.valid &&
            !dm2_v1_render_hud_hand_action_asset(s) &&
            s->source_materials_required) {
            dm2_v1_block_source_material(
                s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_HUD_CORE);
        }
    }
}

/* skproject/SKULLWIN/c_dialog.cpp::DM2_dialog_2066_3820 expands RECT_453
 * and blits DIALOG_BOXES/0x81/0 over the already drawn interface. The M11
 * owner alone supplies the active command, so an admitted material cannot
 * leak into normal dungeon frames. */
void dm2_v1_render_dialogue_box(DM2_V1_ViewportState *s)
{
    const DM2_V1_DialogueBoxHostCommand *command;
    const uint8_t *pixels = NULL;
    int width = 0;
    int height = 0;
    int stride = 0;
    int gdat_index;

    if (!s || !s->framebuffer || !s->gdat_dialogue_box_active) return;
    command = &s->gdat_dialogue_box_command;
    if (!command->valid || !command->draw.valid ||
        command->draw.gdat_category != DM2_GDAT_CATEGORY_DIALOG_BOXES ||
        command->draw.gdat_index != DM2_V1_DIALOGUE_BOX_INDEX ||
        command->draw.gdat_field != DM2_V1_DIALOGUE_BOX_FIELD ||
        command->draw.expanded_rect_index != DM2_V1_DIALOGUE_BOX_RECT_INDEX ||
        command->draw.plan_hash == 0u || command->command_hash == 0u ||
        command->rect.w <= 0 || command->rect.h <= 0) {
        dm2_v1_block_source_material(
            s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_HUD_CORE);
        return;
    }
    gdat_index = dm2_v1_viewport_dialogue_box_graphic_index();
    if (dm2_v1_fetch_viewport_local_material(s, gdat_index, &pixels, &width,
                                             &height, &stride) != 0 ||
        !pixels || width <= 0 || height <= 0 || stride < width ||
        !s->active_asset_palette_ready || s->active_asset_palette_hash == 0u) {
        dm2_v1_block_source_material(
            s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_HUD_CORE);
        return;
    }
    dm2_v1_blit_scaled_material_bitmap(
        s, s->framebuffer, s->fb_stride,
        command->rect.x / 2, command->rect.y / 2,
        command->rect.w / 2, command->rect.h / 2,
        pixels, width, height, stride, DM2_COLOR_TRANSPARENT,
        &s->gdat_interface_palette_consumed_count);
    s->gdat_dialogue_box_consumed_count = 1;
    s->gdat_dialogue_box_consumed_hash = command->command_hash;
}

static DM2_V1_ViewportRect dm2_v1_dialogue_text_rect(
    const DM2_V1_InterfaceRect *source_rect)
{
    DM2_V1_ViewportRect rect = { 0, 0, 0, 0 };

    if (!source_rect) return rect;
    rect.x = source_rect->x;
    rect.y = source_rect->y;
    rect.w = source_rect->w;
    rect.h = source_rect->h;
    if (rect.w <= 0 || rect.h < 6) {
        rect.w = 0;
        rect.h = 0;
    }
    return rect;
}

/* skproject/SKULLWIN/c_dialog.cpp::DM2_dialog_OPEN_DIALOG_PANEL loads the
 * exact DIALOG_BOXES image, then its source-owned V1.0 heading and two GDAT
 * text fields at their original destination rectangles. */
void dm2_v1_render_dialogue_open_panel(DM2_V1_ViewportState *s)
{
    const DM2_V1_DialogueOpenPanelHostCommand *command;
    const uint8_t *pixels = NULL;
    DM2_V1_ViewportRect panel;
    DM2_V1_ViewportRect version_text;
    DM2_V1_ViewportRect primary_text;
    DM2_V1_ViewportRect secondary_text;
    int width = 0;
    int height = 0;
    int stride = 0;
    int gdat_index;

    if (!s || !s->framebuffer || !s->gdat_dialogue_open_panel_active) return;
    command = &s->gdat_dialogue_open_panel_command;
    if (!command->valid || !command->draw.valid ||
        !command->draw.material.valid ||
        command->draw.material.metadata.bits_per_pixel != 4u ||
        command->draw.material.palette_hash == 0u ||
        command->draw.panel_rect_index != DM2_V1_DIALOGUE_OPEN_PANEL_RECT_INDEX ||
        command->command_hash == 0u || command->draw.version_text_size !=
            DM2_V1_DIALOGUE_OPEN_PANEL_VERSION_TEXT_SIZE ||
        command->draw.version_text_hash == 0u || !command->draw.text[0] ||
        memcmp(command->draw.version_text,
               DM2_V1_DIALOGUE_OPEN_PANEL_VERSION_TEXT,
               DM2_V1_DIALOGUE_OPEN_PANEL_VERSION_TEXT_SIZE) != 0 ||
        !command->draw.text[1] || command->draw.text_size[0] == 0u ||
        command->draw.text_size[1] == 0u || command->panel_rect.w <= 0 ||
        command->panel_rect.h <= 0 || command->version_text_rect.w <= 0 ||
        command->version_text_rect.h <= 0 || command->primary_text_rect.w <= 0 ||
        command->primary_text_rect.h <= 0 || command->secondary_text_rect.w <= 0 ||
        command->secondary_text_rect.h <= 0) {
        dm2_v1_block_source_material(
            s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_HUD_CORE);
        return;
    }
    gdat_index = dm2_v1_viewport_dialogue_box_graphic_index();
    if (dm2_v1_fetch_viewport_local_material(s, gdat_index, &pixels, &width,
                                             &height, &stride) != 0 ||
        !pixels || width <= 0 || height <= 0 || stride < width ||
        !s->active_asset_palette_ready || s->active_asset_palette_hash == 0u) {
        dm2_v1_block_source_material(
            s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_HUD_CORE);
        return;
    }
    panel = (DM2_V1_ViewportRect){ command->panel_rect.x,
        command->panel_rect.y, command->panel_rect.w,
        command->panel_rect.h };
    version_text = dm2_v1_dialogue_text_rect(&command->version_text_rect);
    primary_text = dm2_v1_dialogue_text_rect(&command->primary_text_rect);
    secondary_text = dm2_v1_dialogue_text_rect(&command->secondary_text_rect);
    if (panel.w <= 0 || panel.h <= 0 || version_text.w <= 0 ||
        primary_text.w <= 0 ||
        secondary_text.w <= 0 || !s->gdat_interface_font_rows ||
        s->gdat_interface_font_hash == 0u ||
        !s->gdat_interface_text_palette_ready ||
        s->gdat_interface_text_palette_hash == 0u) {
        dm2_v1_block_source_material(
            s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_HUD_CORE);
        return;
    }
    /* c_dialog draws the panel before its compiled heading and GDAT labels. */
    dm2_v1_blit_scaled_material_bitmap(
        s, s->framebuffer, s->fb_stride, panel.x, panel.y, panel.w, panel.h,
        pixels, width, height, stride, DM2_COLOR_TRANSPARENT,
        &s->gdat_interface_palette_consumed_count);
    if (!dm2_v1_render_hud_source_font(s, &version_text,
            (const char *)command->draw.version_text,
            command->draw.version_palette_slot, 0u, 1) ||
        !dm2_v1_render_hud_source_font(s, &primary_text,
            (const char *)command->draw.text[0],
            command->draw.button_palette_slot, 0u, 1) ||
        !dm2_v1_render_hud_source_font(s, &secondary_text,
            (const char *)command->draw.text[1],
            command->draw.button_palette_slot, 0u, 1)) {
        dm2_v1_block_source_material(
            s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_HUD_CORE);
        return;
    }
    s->gdat_dialogue_open_panel_consumed_count = 4;
    s->gdat_dialogue_open_panel_consumed_hash = command->command_hash;
}

/* ── Main render entry ─────────────────────────────────────────────── */

static int dm2_v1_viewport_build_m11_frame_receipt(
    const DM2_V1_ViewportState *s,
    DM2_V1_ViewportM11FrameReceipt *out_receipt);

void dm2_v1_viewport_render(DM2_V1_ViewportState *s)
{
    if (!s) return;

    /* If not dirty and no pending world update, skip full redraw.
     * For Phase 3, always render when called (dirty flag tracking
     * is wired but full optimization deferred to Phase 4). */
    if (!s->dirty && !s->framebuffer) return;
    s->asset_floor_ceiling_drawn_count = 0;
    s->fallback_floor_ceiling_drawn_count = 0;
    s->asset_teleporter_drawn_count = 0;
    s->blocked_material_draw_count = 0;
    s->blocked_material_mask = 0u;
    s->asset_outdoor_sky_drawn_count = 0;
    s->asset_outdoor_ground_drawn_count = 0;
    s->asset_wall_drawn_count = 0;
    s->fallback_wall_drawn_count = 0;
    s->asset_wall_ornament_drawn_count = 0;
    s->fallback_wall_ornament_drawn_count = 0;
    s->asset_door_panel_drawn_count = 0;
    s->asset_door_overlay_drawn_count = 0;
    s->asset_door_frame_drawn_count = 0;
    s->asset_door_button_drawn_count = 0;
    s->fallback_door_drawn_count = 0;
    memset(&s->last_original_door_presentation_command, 0,
           sizeof(s->last_original_door_presentation_command));
    memset(&s->last_dungeon_floor_presentation_command, 0,
           sizeof(s->last_dungeon_floor_presentation_command));
    memset(&s->last_dungeon_ceiling_presentation_command, 0,
           sizeof(s->last_dungeon_ceiling_presentation_command));
    memset(&s->last_dungeon_wall_presentation_command, 0,
           sizeof(s->last_dungeon_wall_presentation_command));
    memset(&s->last_wall_ornament_presentation_command, 0,
           sizeof(s->last_wall_ornament_presentation_command));
    s->last_dungeon_wall_material_required_mask = 0u;
    s->last_dungeon_wall_material_consumed_mask = 0u;
    s->last_wall_ornament_material_required_mask = 0u;
    s->last_wall_ornament_material_consumed_mask = 0u;
    memset(&s->last_scene_control_presentation_command, 0,
           sizeof(s->last_scene_control_presentation_command));
    memset(&s->last_creature_presentation_command, 0,
           sizeof(s->last_creature_presentation_command));
    memset(&s->last_item_presentation_command, 0,
           sizeof(s->last_item_presentation_command));
    memset(&s->last_frame_composition, 0, sizeof(s->last_frame_composition));
    memset(&s->last_m11_frame_receipt, 0, sizeof(s->last_m11_frame_receipt));
    s->asset_creature_drawn_count = 0;
    s->fallback_creature_drawn_count = 0;
    s->asset_item_drawn_count = 0;
    s->fallback_item_drawn_count = 0;
    s->asset_creature_possession_item_drawn_count = 0;
    s->fallback_creature_possession_item_drawn_count = 0;
    s->asset_carried_item_drawn_count = 0;
    s->fallback_carried_item_drawn_count = 0;
    s->item_material_drawn_count = 0;
    memset(s->item_material_gdat_indices, 0,
           sizeof(s->item_material_gdat_indices));
    memset(s->item_material_source_kinds, 0,
           sizeof(s->item_material_source_kinds));
    s->last_item_render_valid = 0;
    s->last_item_asset_blit_valid = 0;
    s->last_item_source_kind = 0;
    s->last_item_draw_order = -1;
    s->last_item_asset_src_w = 0;
    s->last_item_asset_src_h = 0;
    s->last_item_asset_src_stride = 0;
    memset(&s->last_item_render, 0, sizeof(s->last_item_render));
    memset(&s->last_item_asset_blit, 0, sizeof(s->last_item_asset_blit));
    s->asset_projectile_drawn_count = 0;
    s->fallback_projectile_drawn_count = 0;
    s->projectile_material_drawn_count = 0;
    memset(s->projectile_material_gdat_indices, 0,
           sizeof(s->projectile_material_gdat_indices));
    s->last_projectile_render_valid = 0;
    s->last_projectile_asset_blit_valid = 0;
    s->last_projectile_draw_order = -1;
    s->last_projectile_asset_src_w = 0;
    s->last_projectile_asset_src_h = 0;
    s->last_projectile_asset_src_stride = 0;
    memset(&s->last_projectile_render, 0,
           sizeof(s->last_projectile_render));
    memset(&s->last_projectile_asset_blit, 0,
           sizeof(s->last_projectile_asset_blit));
    s->asset_hud_core_drawn_count = 0;
    s->fallback_hud_core_drawn_count = 0;
    s->gdat_interface_palette_consumed_count = 0;
    s->gdat_interface_action_palette_consumed_count = 0;
    s->gdat_interface_font_consumed_count = 0;
    s->gdat_material_palette_floor_ceiling_consumed_count = 0;
    s->gdat_material_palette_wall_consumed_count = 0;
    s->gdat_material_palette_door_frame_consumed_count = 0;
    s->gdat_scene_light_consumed_count = 0;
    s->gdat_c_light_consumed_count = 0;
    s->gdat_scene_material_consumed_count = 0;
    s->gdat_scene_weather_consumed_count = 0;
    s->gdat_sprite_palette_consumed_count = 0;
    s->gdat_local_palette_consumed_count = 0;
    s->last_outdoor_scene_material_required_mask = 0u;
    s->last_outdoor_scene_material_consumed_mask = 0u;
    s->last_hud_core_gdat_hash = 2166136261u;
    s->last_hud_core_pixel_count = 0u;
    s->asset_hud_portrait_drawn_count = 0;
    s->fallback_hud_portrait_drawn_count = 0;
    if (s->gdat_scene_control_ready) {
        dm2_v1_viewport_scene_light_control(
            s->gdat_highest_light_level,
            s->gdat_ambient_darkness,
            &s->gdat_scene_light_floor,
            &s->gdat_scene_light_search_depth,
            &s->gdat_scene_light_recompute_enabled);
        if (s->gdat_ambient_light != 0u ||
            s->gdat_scene_light_floor != 0u ||
            s->gdat_scene_light_search_depth != 0u) {
            ++s->gdat_scene_light_consumed_count;
        }
    }
    s->last_frame_composition.indoor_viewport = !s->is_outdoor;
    s->last_frame_composition.scene_record_owned =
        s->gdat_static_scene_record.valid &&
        s->gdat_static_scene_record.map_load_token ==
            s->gdat_scene_map_load_token &&
        s->gdat_static_scene_record.scene_control_hash ==
            s->gdat_scene_control_hash;
    s->last_frame_composition.scene_light_owned =
        dm2_v1_viewport_frame_light_palette_owned(s);
    s->last_frame_composition.palette_owned =
        s->gdat_interface_palette_ready &&
        s->gdat_interface_palette_hash != 0u;
    s->last_frame_composition.map_load_token = s->gdat_scene_map_load_token;
    s->last_frame_composition.scene_control_hash = s->gdat_scene_control_hash;
    s->last_frame_composition.palette_hash = s->gdat_interface_palette_hash;
    s->last_frame_composition.light_floor = s->gdat_scene_light_floor;
    s->last_frame_composition.light_search_depth =
        s->gdat_scene_light_search_depth;

    /* c_gui_vp consumes the already recomputed light state as part of the
     * dungeon frame setup. This is deliberately metadata-only until the
     * source palette branch is recovered; counting it must not imply a host
     * palette transform or a generated brightness pixel. */
    if (s->source_materials_required && s->gdat_c_light_receipt_ready &&
        s->gdat_c_light_receipt_hash != 0u &&
        s->gdat_c_light_scene_control_hash == s->gdat_scene_control_hash) {
        ++s->gdat_c_light_consumed_count;
    }

    /* When source materials are required and the action text palette is
     * source-bound, count it as consumed even if no HUD text path runs this
     * frame. The palette is already part of the frame's source contract; this
     * prevents M11 from rejecting no-party outdoor frames where no text is
     * drawn. */
    if (s->source_materials_required &&
        s->gdat_interface_text_palette_ready &&
        s->gdat_interface_text_palette_hash != 0u) {
        s->gdat_interface_action_palette_consumed_count = 1;
    }

    /* DM2 has two fundamentally different render paths:
     *   1. Indoor dungeon (is_outdoor=0): first-person 3D dungeon view
     *   2. Outdoor (is_outdoor=1): sky gradient + ground + buildings
     *
     * Source: SKULL.ASM T560 (dungeon), SKULL.ASM T600 (outdoor) */

    if (s->is_outdoor) {
        typedef struct {
            const uint8_t *pixels;
            int width;
            int height;
            int stride;
            uint8_t palette16[16];
            uint32_t palette_hash;
            int ready;
        } DM2_V1_OutdoorSceneMaterial;
        enum {
            DM2_OUTDOOR_SCENE_SKY = 1u << 0,
            DM2_OUTDOOR_SCENE_GROUND = 1u << 1
        };
        /* DM2 outdoor rendering:
         * Source: SKULL.ASM T600 (outdoor tick, sky and ground draw)
         *         skproject/SKWIN/SkWinCore.cpp GRAPHICSSET material route
         *
         * Do not substitute a generated gradient for source material.  The
         * active map GRAPHICSSET already supplies the ceiling/floor GDAT
         * records used by the scene; route them through the same palette
         * binding as the indoor viewport before weather and HUD are layered. */
        uint8_t *vp = s->framebuffer;
        int stride = s->fb_stride;
        const uint8_t *sky_pixels = NULL;
        int sky_w = 0;
        int sky_h_src = 0;
        int sky_stride = 0;
        int sky_h = DM2_VP_HEIGHT / 2;
        int sky_gdat_index = dm2_v1_viewport_scene_material_graphic_index(
            s->gdat_scene_material_index,
            DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_CEILING);
        int ground_gdat_index = dm2_v1_viewport_scene_material_graphic_index(
            s->gdat_scene_material_index,
            DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_FLOOR);
        DM2_V1_OutdoorSceneMaterial sky_material = { 0 };
        DM2_V1_OutdoorSceneMaterial ground_material = { 0 };
        int sky_asset;
        int ground_asset;
        /* T600 resolves each GRAPHICSSET IMG3 with its own local palette;
         * source-required outdoor frames must not borrow an interface palette. */
        if (s->source_materials_required) {
            const DM2_V1_GdatSceneM11CommandPlan *plan =
                s->gdat_scene_material_plan;
            int graphicsset_index = 0;
            int material_field = 0;

            s->last_outdoor_scene_material_required_mask =
                DM2_OUTDOOR_SCENE_SKY | DM2_OUTDOOR_SCENE_GROUND;
            if (!s->gdat_scene_control_ready ||
                (!plan && (!s->asset_fetch || !s->asset_palette_fetch))) {
                dm2_v1_block_source_material(
                    s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING);
                return;
            }
            if (plan) {
                const DM2_V1_GdatSceneM11Command *ground =
                    &plan->commands[0];
                const DM2_V1_GdatSceneM11Command *sky = &plan->commands[1];

                if (plan->graphicsset != (uint8_t)s->gdat_scene_material_index ||
                    plan->command_hash != s->gdat_scene_control_hash ||
                    plan->highest_light_level != s->gdat_highest_light_level ||
                    plan->ambient_darkness != s->gdat_ambient_darkness ||
                    ground->field != DM2_GDAT_GFXSET_FLOOR ||
                    sky->field != DM2_GDAT_GFXSET_CEIL ||
                    !ground->pixels || !sky->pixels ||
                    ground->width == 0u || ground->height == 0u ||
                    sky->width == 0u || sky->height == 0u ||
                    ground->decoded_hash == 0u || sky->decoded_hash == 0u ||
                    ground->palette_hash == 0u || sky->palette_hash == 0u ||
                    ground->decoded_hash !=
                        dm2_v1_gdat_scene_m11_command_pixel_hash(ground) ||
                    sky->decoded_hash !=
                        dm2_v1_gdat_scene_m11_command_pixel_hash(sky) ||
                    ground->palette_hash !=
                        dm2_v1_weather_pixels_hash(ground->palette16, 16, 1,
                                                   16) ||
                    sky->palette_hash !=
                        dm2_v1_weather_pixels_hash(sky->palette16, 16, 1,
                                                   16)) {
                    dm2_v1_block_source_material(
                        s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING);
                    return;
                }
                if ((ground->palette_light_receipt_hash != 0u ||
                     sky->palette_light_receipt_hash != 0u) &&
                    (!s->gdat_c_light_receipt_ready ||
                     ground->palette_light_receipt_hash == 0u ||
                     sky->palette_light_receipt_hash == 0u ||
                     ground->palette_light_receipt_hash !=
                         s->gdat_c_light_receipt_hash ||
                     sky->palette_light_receipt_hash !=
                         s->gdat_c_light_receipt_hash ||
                     ground->palette_transform_hash == 0u ||
                     sky->palette_transform_hash == 0u)) {
                    dm2_v1_block_source_material(
                        s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING);
                    return;
                }

                sky_material.pixels = sky->pixels;
                sky_material.width = sky->width;
                sky_material.height = sky->height;
                sky_material.stride = sky->width;
                memcpy(sky_material.palette16, sky->palette16,
                       sizeof(sky_material.palette16));
                sky_material.palette_hash = sky->palette_hash;
                sky_material.ready = 1;
                ground_material.pixels = ground->pixels;
                ground_material.width = ground->width;
                ground_material.height = ground->height;
                ground_material.stride = ground->width;
                memcpy(ground_material.palette16, ground->palette16,
                       sizeof(ground_material.palette16));
                ground_material.palette_hash = ground->palette_hash;
                ground_material.ready = 1;
            } else {
            /* skproject T600 resolves both active GRAPHICSSET materials
             * before it presents the outdoor scene. Cache the real IMG3 plus
             * local-palette pairs, so a rejected ground cannot leave a sky
             * rendered through an invented or stale palette. */
            if (!dm2_v1_viewport_scene_material_graphic_address(
                    sky_gdat_index, &graphicsset_index, &material_field) ||
                graphicsset_index != s->gdat_scene_material_index ||
                material_field != DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_CEILING ||
                dm2_v1_fetch_viewport_local_material(
                    s, sky_gdat_index, &sky_material.pixels,
                    &sky_material.width, &sky_material.height,
                    &sky_material.stride) != 0 ||
                !sky_material.pixels || sky_material.width <= 0 ||
                sky_material.height <= 0 || !s->active_asset_palette_ready ||
                s->active_asset_palette_hash == 0u) {
                dm2_v1_block_source_material(
                    s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING);
                return;
            }
            memcpy(sky_material.palette16, s->active_asset_palette16,
                   sizeof(sky_material.palette16));
            sky_material.palette_hash = s->active_asset_palette_hash;
            sky_material.ready = 1;

            if (!dm2_v1_viewport_scene_material_graphic_address(
                    ground_gdat_index, &graphicsset_index, &material_field) ||
                graphicsset_index != s->gdat_scene_material_index ||
                material_field != DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_FLOOR ||
                dm2_v1_fetch_viewport_local_material(
                    s, ground_gdat_index, &ground_material.pixels,
                    &ground_material.width, &ground_material.height,
                    &ground_material.stride) != 0 ||
                !ground_material.pixels || ground_material.width <= 0 ||
                ground_material.height <= 0 || !s->active_asset_palette_ready ||
                s->active_asset_palette_hash == 0u) {
                dm2_v1_block_source_material(
                    s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING);
                return;
            }
            memcpy(ground_material.palette16, s->active_asset_palette16,
                   sizeof(ground_material.palette16));
            ground_material.palette_hash = s->active_asset_palette_hash;
            ground_material.ready = 1;
            }
            /* T600 binds each GRAPHICSSET IMG3 to its own local palette
             * before either scene plane is presented. The palette receipt hash
             * is owned by the source decoder/provider, so the viewport only
             * requires that each plane carries a nonzero local-palette
             * receipt; it must not reinterpret that receipt as an FNV of the
             * already-decoded bytes. */
            if (sky_material.palette_hash == 0u ||
                ground_material.palette_hash == 0u) {
                dm2_v1_block_source_material(
                    s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING);
                return;
            }
            sky_pixels = sky_material.pixels;
            sky_w = sky_material.width;
            sky_h_src = sky_material.height;
            sky_stride = sky_material.stride;
            memcpy(s->active_asset_palette16, sky_material.palette16,
                   sizeof(s->active_asset_palette16));
            s->active_asset_palette_hash = sky_material.palette_hash;
            s->active_asset_palette_ready = sky_material.ready;
            sky_asset = 1;
        } else {
            sky_asset = dm2_v1_fetch_viewport_local_material(
                            s, sky_gdat_index, &sky_pixels, &sky_w,
                            &sky_h_src, &sky_stride) == 0 &&
                sky_pixels && sky_w > 0 && sky_h_src > 0;
        }
        if (sky_asset) {
            dm2_v1_blit_tiled_material_bitmap(
                s, vp, stride, 0, 0, DM2_VP_WIDTH, sky_h, sky_pixels,
                sky_w, sky_h_src,
                sky_stride > 0 ? sky_stride : sky_w, -1,
                0,
                &s->gdat_material_palette_floor_ceiling_consumed_count);
            ++s->asset_floor_ceiling_drawn_count;
            ++s->asset_outdoor_sky_drawn_count;
            ++s->gdat_scene_material_consumed_count;
            if (s->gdat_scene_control_ready) {
                ++s->gdat_scene_control_consumed_count;
            }
            if (s->source_materials_required) {
                s->last_outdoor_scene_material_consumed_mask |=
                    DM2_OUTDOOR_SCENE_SKY;
            }
        } else if (s->source_materials_required) {
            dm2_v1_block_source_material(
                s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING);
        }
        /* Keep the ground query after the sky draw for the same reason as
         * indoor ceiling/floor: its local IMG3 palette must not replace the
         * palette paired with already-decoded sky pixels. */
        const uint8_t *ground_pixels = NULL;
        int ground_w = 0;
        int ground_h_src = 0;
        int ground_stride = 0;
        if (s->source_materials_required) {
            ground_pixels = ground_material.pixels;
            ground_w = ground_material.width;
            ground_h_src = ground_material.height;
            ground_stride = ground_material.stride;
            memcpy(s->active_asset_palette16, ground_material.palette16,
                   sizeof(s->active_asset_palette16));
            s->active_asset_palette_hash = ground_material.palette_hash;
            s->active_asset_palette_ready = ground_material.ready;
            ground_asset = 1;
        } else {
            ground_asset = dm2_v1_fetch_viewport_local_material(
                               s, ground_gdat_index, &ground_pixels,
                               &ground_w, &ground_h_src, &ground_stride) == 0 &&
                ground_pixels && ground_w > 0 && ground_h_src > 0;
        }
        if (ground_asset) {
            dm2_v1_blit_tiled_material_bitmap(
                s, vp, stride, 0, sky_h, DM2_VP_WIDTH,
                DM2_VP_HEIGHT - sky_h, ground_pixels, ground_w,
                ground_h_src,
                ground_stride > 0 ? ground_stride : ground_w, -1,
                0,
                &s->gdat_material_palette_floor_ceiling_consumed_count);
            ++s->asset_floor_ceiling_drawn_count;
            ++s->asset_outdoor_ground_drawn_count;
            ++s->gdat_scene_material_consumed_count;
            if (s->gdat_scene_control_ready) {
                ++s->gdat_scene_control_consumed_count;
            }
            if (s->source_materials_required) {
                s->last_outdoor_scene_material_consumed_mask |=
                    DM2_OUTDOOR_SCENE_GROUND;
            }
        } else if (s->source_materials_required) {
            dm2_v1_block_source_material(
                s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING);
        }
        if (s->source_materials_required &&
            s->last_outdoor_scene_material_required_mask !=
                s->last_outdoor_scene_material_consumed_mask) {
            dm2_v1_block_source_material(
                s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING);
            return;
        }
    } else {
        /* DM2 indoor dungeon rendering:
         * Draw order (same as DM1): D3→D2→D1→D0 per depth.
         * Source: DUNGEON.C:1371-1421; DUNVIEW.C:8466-8542 */

        /* 1. Background (black) */
        dm2_v1_render_background(s);

        /* 2. Floor and ceiling */
        dm2_v1_render_floor_ceiling(s);
        if (s->source_materials_required &&
            s->last_dungeon_ceiling_presentation_command.valid) {
            s->last_frame_composition.dungeon_ceiling_presentation_stage = 1;
            s->last_frame_composition.dungeon_ceiling_command_consumed = 1;
            s->last_frame_composition.dungeon_ceiling_command =
                s->last_dungeon_ceiling_presentation_command;
        }
        if (s->source_materials_required &&
            s->last_dungeon_floor_presentation_command.valid) {
            s->last_frame_composition.dungeon_floor_presentation_stage = 2;
            s->last_frame_composition.dungeon_floor_command_consumed = 1;
            s->last_frame_composition.dungeon_floor_command =
                s->last_dungeon_floor_presentation_command;
        }

        /* 2b. Source-owned TELEPORTERS/0/F9 map-chip fields. */
        dm2_v1_render_teleporter_fields(s);

        /* 3. Walls — source GRAPHICS.DAT material pass. */
        dm2_v1_render_walls(s);
        if (s->source_materials_required &&
            s->last_dungeon_wall_presentation_command.valid) {
            s->last_frame_composition.dungeon_wall_presentation_stage = 3;
            s->last_frame_composition.dungeon_wall_command_consumed = 1;
            s->last_frame_composition.dungeon_wall_command =
                s->last_dungeon_wall_presentation_command;
            s->last_frame_composition.dungeon_wall_material_required_mask =
                s->last_dungeon_wall_material_required_mask;
            s->last_frame_composition.dungeon_wall_material_consumed_mask =
                s->last_dungeon_wall_material_consumed_mask;
        }

        /* 3b. Wall ornaments (alcoves, wall features). */
        dm2_v1_render_wall_ornaments(s);
        if (s->source_materials_required &&
            s->last_wall_ornament_presentation_command.valid) {
            s->last_frame_composition.wall_ornament_presentation_stage = 4;
            s->last_frame_composition.wall_ornament_command_consumed = 1;
            s->last_frame_composition.wall_ornament_command =
                s->last_wall_ornament_presentation_command;
            s->last_frame_composition.wall_ornament_material_required_mask =
                s->last_wall_ornament_material_required_mask;
            s->last_frame_composition.wall_ornament_material_consumed_mask =
                s->last_wall_ornament_material_consumed_mask;
        }

        /* 4. Doors */
        if (s->source_materials_required &&
            !s->last_frame_composition.scene_light_owned) {
            dm2_v1_block_source_material(
                s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR);
        } else {
            dm2_v1_render_doors(s);
            if (s->source_materials_required &&
                s->last_original_door_presentation_command.valid) {
                s->last_frame_composition.door_presentation_stage = 4;
                s->last_frame_composition.door_command_consumed = 1;
                s->last_frame_composition.door_command =
                    s->last_original_door_presentation_command;
            }
        }

        if (s->source_materials_required) {
            DM2_V1_ViewportSceneControlCommand command;
            if (dm2_v1_viewport_scene_control_command(s, &command)) {
                s->last_scene_control_presentation_command = command;
                s->last_frame_composition.scene_control_presentation_stage = 5;
                s->last_frame_composition.scene_control_command_consumed = 1;
                s->last_frame_composition.scene_control_command = command;
            } else {
                dm2_v1_block_source_material(
                    s, DM2_V1_VIEWPORT_BLOCKED_MATERIAL_SCENE_CONTROL);
            }
        }

        /* 5. Floor items */
        dm2_v1_render_items(s);
        if (s->source_materials_required &&
            s->last_item_presentation_command.valid) {
            s->last_frame_composition.item_presentation_stage = 7;
            s->last_frame_composition.item_command_consumed = 1;
            s->last_frame_composition.item_command =
                s->last_item_presentation_command;
        }

        /* 6. Creatures */
        dm2_v1_render_creatures(s);
        if (s->source_materials_required &&
            s->last_creature_presentation_command.valid) {
            s->last_frame_composition.creature_presentation_stage = 6;
            s->last_frame_composition.creature_command_consumed = 1;
            s->last_frame_composition.creature_command =
                s->last_creature_presentation_command;
        }

        /* 7. Creature possession/item overlays */
        dm2_v1_render_creature_possession_items(s);

        /* 8. Projectiles */
        dm2_v1_render_projectiles(s);
    }

    /* 9. Carried leader-hand item overlay */
    dm2_v1_render_carried_item(s);

    /* 10. Weather overlay (applies to both indoor and outdoor) */
    dm2_v1_render_weather_overlay(s);

    /* 11. UI chrome (always on top) */
    dm2_v1_render_ui_chrome(s);
    if (!s->is_outdoor) {
        s->last_frame_composition.hud_presentation_stage = 11;
        if (s->source_materials_required &&
            s->last_hud_top_bar_material_request.valid) {
            s->last_frame_composition.hud_top_bar_material_consumed = 1;
            s->last_frame_composition.hud_top_bar_request =
                s->last_hud_top_bar_material_request;
        }
        if (s->source_materials_required &&
            s->last_hud_top_bar_presentation_command.valid) {
            s->last_frame_composition.hud_top_bar_command_consumed = 1;
            s->last_frame_composition.hud_top_bar_command =
                s->last_hud_top_bar_presentation_command;
            s->last_frame_composition.hud_top_bar_order = 1;
        }
        if (s->source_materials_required &&
            s->last_hud_status_panel_material_request.valid) {
            s->last_frame_composition.hud_status_panel_material_consumed = 1;
            s->last_frame_composition.hud_status_panel_request =
                s->last_hud_status_panel_material_request;
        }
        if (s->source_materials_required &&
            s->last_hud_status_panel_presentation_command.valid) {
            s->last_frame_composition.hud_status_panel_command_consumed = 1;
            s->last_frame_composition.hud_status_panel_command =
                s->last_hud_status_panel_presentation_command;
            s->last_frame_composition.hud_status_panel_order = 2;
        }
        if (s->source_materials_required &&
            s->hud_hand_action_source.valid &&
            s->last_hud_hand_action_material_request.valid) {
            s->last_frame_composition.hud_hand_action_command_consumed = 1;
            s->last_frame_composition.hud_hand_action_request =
                s->last_hud_hand_action_material_request;
            s->last_frame_composition.hud_hand_action_order = 3;
        }
        if (s->source_materials_required &&
            s->hud_hand_action_source.valid &&
            s->last_hud_hand_action_presentation_command.valid) {
            s->last_frame_composition.hud_hand_action_command =
                s->last_hud_hand_action_presentation_command;
        }
        s->last_frame_composition.valid =
            !s->source_materials_required ||
            (s->last_frame_composition.scene_light_owned &&
             s->last_frame_composition.dungeon_ceiling_command_consumed &&
             s->last_frame_composition.dungeon_floor_command_consumed &&
             s->last_frame_composition.dungeon_wall_command_consumed &&
             s->last_frame_composition.scene_control_command_consumed &&
             s->last_frame_composition.hud_top_bar_material_consumed &&
             s->last_frame_composition.hud_top_bar_command_consumed &&
             s->last_frame_composition.hud_status_panel_material_consumed &&
             s->last_frame_composition.hud_status_panel_command_consumed &&
             (!s->hud_hand_action_source.valid ||
              (s->last_frame_composition.hud_hand_action_command_consumed &&
               s->last_frame_composition.hud_hand_action_command.valid)));
    }
    (void)dm2_v1_viewport_build_m11_frame_receipt(
        s, &s->last_m11_frame_receipt);

    /* 12. Original M11-owned dialogue surfaces, when source state is active. */
    dm2_v1_render_dialogue_open_panel(s);
    dm2_v1_render_dialogue_box(s);

    s->dirty = 0;
}

/* ── GDAT-backed graphic fetch ───────────────────────────────────── */

static int dm2_v1_viewport_build_m11_frame_receipt(
    const DM2_V1_ViewportState *s,
    DM2_V1_ViewportM11FrameReceipt *out_receipt)
{
    const DM2_V1_ViewportFrameCompositionReceipt *c;
    int door_required = 0;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!s) return 0;
    for (int i = 0; i < DM2_SQ_COUNT; ++i) {
        if (s->squares[i].flags & DM2_SQF_HAS_DOOR) {
            door_required = 1;
            break;
        }
    }

    c = &s->last_frame_composition;
    out_receipt->source_materials_required = s->source_materials_required;
    out_receipt->map_load_token = c->map_load_token;
    out_receipt->scene_control_hash = c->scene_control_hash;
    out_receipt->palette_hash = c->palette_hash;
    out_receipt->composition = *c;
    out_receipt->valid =
        s->source_materials_required && c->valid && c->indoor_viewport &&
        c->scene_record_owned && c->scene_light_owned && c->palette_owned &&
        c->map_load_token != 0u && c->scene_control_hash != 0u &&
        c->palette_hash != 0u &&
        c->dungeon_ceiling_command_consumed &&
        c->dungeon_ceiling_command.valid &&
        c->dungeon_floor_command_consumed &&
        c->dungeon_floor_command.valid &&
        c->dungeon_wall_command_consumed && c->dungeon_wall_command.valid &&
        c->dungeon_wall_material_required_mask != 0u &&
        c->dungeon_wall_material_required_mask ==
            c->dungeon_wall_material_consumed_mask &&
        c->scene_control_command_consumed && c->scene_control_command.valid &&
        (!door_required ||
         (c->door_command_consumed && c->door_command.valid)) &&
        (!s->creature_count ||
         (c->creature_command_consumed && c->creature_command.valid)) &&
        (!s->item_count ||
         (c->item_command_consumed && c->item_command.valid)) &&
        c->hud_top_bar_material_consumed && c->hud_top_bar_request.valid &&
        c->hud_top_bar_command_consumed && c->hud_top_bar_command.valid &&
        c->hud_status_panel_material_consumed &&
        c->hud_status_panel_request.valid &&
        c->hud_status_panel_command_consumed &&
        c->hud_status_panel_command.valid &&
        (!s->hud_hand_action_source.valid ||
         (c->hud_hand_action_command_consumed &&
          c->hud_hand_action_request.valid &&
          c->hud_hand_action_command.valid));
    out_receipt->m11_consume_frame = out_receipt->valid;
    return out_receipt->valid;
}

int dm2_v1_gfx_fetch(int gdat_index,
                     const uint8_t **out_pixels,
                     int *out_w, int *out_h,
                     int *out_stride)
{
    /* DM2 GRAPHICS.DAT asset loading.
     * gdat_index: category<<8 | entry (see dm2_v1_gfx_asset_loader.h)
     *
     * DM2 graphics categories:
     *   Wall graphics:    negative indices (G2107 wall set base)
     *   Floor graphics:   -1 (floor), -2 (ceiling)
     *   Door graphics:    G2116-G2119 + G2196
     *   Ornament:         G0103_as_CurrentMapDoorOrnamentsInfo[17]
     *   Creature:         SKULL.ASM creature graphic indices
     *   Item:             SKULL.ASM object graphic indices
     *   Projectile:       G0075_apuc_PaletteChanges_Projectile
     *
     * Phase 3: returns NULL/0 (no asset system yet).
     * Full GDAT loading deferred to Phase 3 asset system integration.
     *
     * Source: SKULL.ASM T560 (GDAT loading)
     *         DUNVIEW.C F0096 (LoadCurrentMapGraphics)
     *         asset_loader_m11.c (shared asset system)
     */
    (void)gdat_index;
    if (out_pixels) *out_pixels = NULL;
    if (out_w) *out_w = 0;
    if (out_h) *out_h = 0;
    if (out_stride) *out_stride = 0;
    return -1;
}

int dm2_v1_viewport_build_flying_item_m11_delivery_plan(
    const DM2_V1_RuntimeFlyingItemReceipt *receipt,
    DM2_V1_FlyingItemM11DeliveryPlan *out_plan)
{
    uint32_t hash = 2166136261u;
    const DM2_V1_G1FlyingItemSourceReceipt *source;

    if (!out_plan) return 0;
    memset(out_plan, 0, sizeof(*out_plan));
    if (!receipt || !receipt->valid || !receipt->no_draw ||
        !receipt->raw_gfx256_hash || !receipt->raw_gfx256_receipt_hash ||
        !receipt->palette_hash || !receipt->raw4_hash ||
        !receipt->raw4_receipt_hash || !receipt->timer_receipt_hash ||
        !receipt->identity_hash) return 0;
    source = &receipt->source;
    if (!source->valid || source->missile_object_id == 0xfffeu ||
        (source->category != 0x0du && source->category != 0x0eu) ||
        (source->image_field != 8u && source->image_field != 9u &&
         source->image_field != 10u && source->image_field != 12u) ||
        source->flip_flags > 3u || source->cell_pos > 22u ||
        source->position_5x5 > 24u || !source->clip_rect_id ||
        !source->stretch_factor64 || !source->identity_hash) return 0;
    out_plan->missile_object_id = source->missile_object_id;
    out_plan->category = source->category;
    out_plan->item_type = source->item_type;
    out_plan->image_field = source->image_field;
    out_plan->flip_flags = source->flip_flags;
    out_plan->cell_pos = source->cell_pos;
    out_plan->position_5x5 = source->position_5x5;
    out_plan->clip_rect_id = source->clip_rect_id;
    out_plan->stretch_factor64 = source->stretch_factor64;
    out_plan->raw_gfx256_hash = receipt->raw_gfx256_hash;
    out_plan->raw_gfx256_receipt_hash = receipt->raw_gfx256_receipt_hash;
    out_plan->palette_hash = receipt->palette_hash;
    out_plan->raw4_hash = receipt->raw4_hash;
    out_plan->raw4_receipt_hash = receipt->raw4_receipt_hash;
    out_plan->timer_receipt_hash = receipt->timer_receipt_hash;
    hash ^= source->identity_hash; hash *= 16777619u;
    hash ^= receipt->raw_gfx256_hash; hash *= 16777619u;
    hash ^= receipt->raw_gfx256_receipt_hash; hash *= 16777619u;
    hash ^= receipt->palette_hash; hash *= 16777619u;
    hash ^= receipt->raw4_hash; hash *= 16777619u;
    hash ^= receipt->raw4_receipt_hash; hash *= 16777619u;
    hash ^= receipt->timer_receipt_hash; hash *= 16777619u;
    hash ^= receipt->identity_hash; hash *= 16777619u;
    out_plan->identity_hash = hash ? hash : 1u;
    /* Exact material delivery is complete, but no original pixel decode has
     * been evidenced. M11 receives a no-draw plan only. */
    out_plan->valid = 1;
    out_plan->no_draw = 1;
    out_plan->pixel_decoder_ready = 0;
    out_plan->m11_delivery_ready = 1;
    return 1;
}

int dm2_v1_viewport_build_flying_item_m11_delivery_plan_from_viewport_evidence(
    const DM2_V1_RuntimeFlyingItemReceipt *receipt,
    const DM2_V1_RuntimeFlyingItemViewportEvidence *evidence,
    DM2_V1_FlyingItemM11DeliveryPlan *out_plan)
{
    if (!evidence || !evidence->valid || !evidence->no_draw ||
        !evidence->identity_hash || !evidence->session_identity ||
        !evidence->map_load_token || !receipt ||
        evidence->timer_receipt_hash != receipt->timer_receipt_hash ||
        evidence->gdat_identity_hash != receipt->identity_hash ||
        !dm2_v1_viewport_build_flying_item_m11_delivery_plan(receipt, out_plan)) return 0;
    out_plan->viewport_evidence_hash = evidence->identity_hash;
    out_plan->viewport_session_identity = evidence->session_identity;
    out_plan->viewport_map_load_token = evidence->map_load_token;
    return 1;
}

int dm2_v1_viewport_build_static_object_m11_delivery_plan(
    const DM2_V1_G1StaticObjectMaterialReceipt *material,
    const DM2_V1_StaticObjectSourcePlan *source_plan,
    uint32_t session_identity,
    DM2_V1_StaticObjectM11DeliveryPlan *out_plan)
{
    const DM2_V1_G1StaticObjectMaterialSelector *selector;
    uint32_t hash = 2166136261u;

    if (!out_plan) return 0;
    memset(out_plan, 0, sizeof(*out_plan));
    if (!material || !source_plan || !session_identity ||
        !material->raw_gfx256_bytes || !material->raw_gfx256_byte_count ||
        !material->raw_gfx256_hash || !material->raw_gfx256_receipt_hash ||
        !material->local_palette_hash || !material->raw4_hash ||
        !material->raw4_receipt_hash) return 0;
    selector = &material->selector;
    if (!selector->valid || !selector->object_id || !selector->identity_hash ||
        (selector->category != 0x10u && selector->category != 0x14u) ||
        (selector->category == 0x10u &&
         (selector->image_field != 0u || selector->container_open)) ||
        (selector->category == 0x14u &&
         (selector->image_field != 0u && selector->image_field != 4u)) ||
        selector->image_field == 0xf9u || source_plan->source_cell < 0 ||
        source_plan->source_pass !=
            dm2_v1_viewport_draw_dungeon_tiles_pass_for_cell(
                source_plan->source_cell) || source_plan->position_5x5 < 0 ||
        source_plan->position_5x5 > 24 || source_plan->clip_rect_id == 0 ||
        (uint16_t)(source_plan->clip_rect_id & 0x7fffu) !=
            material->clip_rect_id || source_plan->image_field !=
            selector->image_field || source_plan->stretch_factor64 <= 0 ||
        source_plan->record_list_ordinal == 0u ||
        source_plan->visibility_mask_5x5 == 0u ||
        (source_plan->visibility_mask_5x5 &
         (1u << (unsigned)source_plan->position_5x5)) == 0u) return 0;
    out_plan->session_identity = session_identity;
    out_plan->object_id = selector->object_id;
    out_plan->category = selector->category;
    out_plan->item_type = selector->item_type;
    out_plan->image_field = selector->image_field;
    out_plan->direction = selector->direction;
    out_plan->container_open = selector->container_open;
    out_plan->image_offset = selector->image_offset;
    out_plan->source_cell = source_plan->source_cell;
    out_plan->source_pass = source_plan->source_pass;
    out_plan->position_5x5 = source_plan->position_5x5;
    out_plan->clip_rect_id = material->clip_rect_id;
    out_plan->stretch_factor64 = source_plan->stretch_factor64;
    out_plan->flip_mirror = source_plan->rect14_applied
        ? source_plan->rect14_flip_mirror
        : source_plan->flip_mirror;
    out_plan->selector_identity_hash = selector->identity_hash;
    out_plan->raw_gfx256_hash = material->raw_gfx256_hash;
    out_plan->raw_gfx256_receipt_hash = material->raw_gfx256_receipt_hash;
    out_plan->palette_hash = material->local_palette_hash;
    out_plan->raw4_hash = material->raw4_hash;
    out_plan->raw4_receipt_hash = material->raw4_receipt_hash;
    out_plan->rect14_row_hash = source_plan->rect14_applied
        ? source_plan->rect14_row_hash : 0u;
    out_plan->rect14_placement_hash = source_plan->rect14_applied
        ? source_plan->rect14_placement_hash : 0u;
    out_plan->visibility_mask_5x5 = source_plan->visibility_mask_5x5;
    out_plan->record_list_ordinal = source_plan->record_list_ordinal;
    hash ^= session_identity; hash *= 16777619u;
    hash ^= selector->identity_hash; hash *= 16777619u;
    hash ^= material->raw_gfx256_hash; hash *= 16777619u;
    hash ^= material->raw_gfx256_receipt_hash; hash *= 16777619u;
    hash ^= material->local_palette_hash; hash *= 16777619u;
    hash ^= material->raw4_hash; hash *= 16777619u;
    hash ^= material->raw4_receipt_hash; hash *= 16777619u;
    hash ^= (uint32_t)source_plan->clip_rect_id; hash *= 16777619u;
    hash ^= (uint32_t)source_plan->stretch_factor64; hash *= 16777619u;
    hash ^= source_plan->visibility_mask_5x5; hash *= 16777619u;
    hash ^= (uint32_t)source_plan->record_list_ordinal; hash *= 16777619u;
    if (source_plan->rect14_applied) {
        hash ^= source_plan->rect14_row_hash; hash *= 16777619u;
        hash ^= source_plan->rect14_placement_hash; hash *= 16777619u;
    }
    out_plan->identity_hash = hash ? hash : 1u;
    out_plan->valid = 1;
    out_plan->no_draw = 1;
    out_plan->pixel_decoder_ready = 0;
    out_plan->m11_delivery_ready = 1;
    return 1;
}

int dm2_v1_viewport_static_object_m11_delivery_plan_matches(
    const DM2_V1_StaticObjectM11DeliveryPlan *plan,
    const DM2_V1_G1StaticObjectMaterialReceipt *material,
    const DM2_V1_StaticObjectSourcePlan *source_plan,
    uint32_t session_identity)
{
    DM2_V1_StaticObjectM11DeliveryPlan candidate;
    return plan && plan->valid && plan->no_draw && plan->m11_delivery_ready &&
        !plan->pixel_decoder_ready &&
        dm2_v1_viewport_build_static_object_m11_delivery_plan(
            material, source_plan, session_identity, &candidate) &&
        candidate.identity_hash == plan->identity_hash;
}

/* ── Source evidence ─────────────────────────────────────────────── */

const char *dm2_v1_viewport_source_evidence(void)
{
    return
        "DM2 V1 Viewport Renderer — Phase 3\n"
        "Source: SKULL.ASM T560  — dungeon viewport rendering pipeline\n"
        "Source: SKULL.ASM T600  — outdoor viewport rendering (sky gradient, buildings)\n"
        "Source: SKULL.ASM T520  — party/movement tick, map coordinate resolution\n"
        "Source: ReDMCSB DUNGEON.C:1371-1421 — draw order, map coordinate resolution\n"
        "Source: ReDMCSB DUNVIEW.C:575-586  — G0163 wall frame table (12 entries)\n"
        "Source: ReDMCSB DUNVIEW.C:140-175  — wall set indices (G3011-G3066)\n"
        "Source: ReDMCSB DUNVIEW.C:126-127  — G2108_Floor=-1, G2109_Ceiling=-2\n"
        "Source: ReDMCSB DUNVIEW.C:148-157  — door frame indices (G2116-G2119, G2196)\n"
        "Source: ReDMCSB DUNVIEW.C:2962-3070 — F0098 DrawFloorAndCeiling, F0100 DrawWallSetBitmap\n"
        "Source: ReDMCSB DUNVIEW.C:3082-3112 — F0102 DrawDoorBitmap, F0103 DrawDoorFrameBitmapFlipped\n"
        "Source: ReDMCSB DUNVIEW.C:3940-4015 — F0108 DrawFloorOrnament, F0109 DrawDoorOrnament\n"
        "Source: ReDMCSB DUNVIEW.C:4119-4270 — F0110 DrawDoorButton, F0111 DrawDoor\n"
        "Source: ReDMCSB DUNVIEW.C:4351-4382 — F0112 DrawCeilingPit (outdoor ceiling)\n"
        "Source: ReDMCSB DUNVIEW.C:4960-5039 — object depth scale and palette changes\n"
        "Source: ReDMCSB DUNVIEW.C:4567-4581 — creature/object/projectile layer specs\n"
        "Source: ReDMCSB DUNVIEW.C:5681-5883 — projectile occlusion specs\n"
        "Source: ReDMCSB DUNVIEW.C:361        — G0103_as_CurrentMapDoorOrnamentsInfo[17]\n"
        "Source: ReDMCSB DUNVIEW.C:8466-8542 — draw order (D4L→D4R→D4C→D3L→...→D0C)\n"
        "Source: skproject/SKWIN/SkWinCore.cpp:1001-1037 — DRAW_CHIP_OF_MAGIC_MAP frame atlas offset\n"
        "Source: skproject/SKWIN/SkWinCore.cpp:10782-10817 — DRAW_MAP_CHIP creature possession item overlays\n"
        "Source: SKULLWIN/SKWIN/c_gui_vp.cpp  — viewport blit order (reference)\n"
        "Source: docs/dm2_graphics.md         — drawing pipeline audit\n"
        "Source: docs/dm2_walls.md            — wall/door/floor rendering specifics\n"
        "Source: docs/dm2_palette.md          — DM2 palette system\n"
        "Reference: dm1_v1_viewport_3d_pc34_compat.c (DM1 draw order, wall blit patterns)\n";
}
