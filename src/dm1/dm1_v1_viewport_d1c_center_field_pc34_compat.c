#include "dm1_v1_viewport_d1c_center_field_pc34_compat.h"

#include "dm1_v1_viewport_d0c_center_field_pc34_compat.h"
#include "dm1_v1_viewport_floor_ceiling_items_pc34_compat.h"

#include <stddef.h>

/*
 * ReDMCSB anchors: DUNGEON.C F0151 lines 1423-1479, F0152 lines 1481-1492,
 * F0153 lines 1495-1506; DUNVIEW.C F0124 lines 7727-7958, F0128 lines
 * 8530-8533; DUNVIEW.C F0098 line 2962, F0113 lines 7922-7956, F0115
 * lines 4547-6137; dm1_v1_viewport_3d_pc34_compat anchors DUNVIEW.C
 * F0098/F0124/F0128 and d0c_center_field anchors DUNVIEW.C F0127 lines
 * 8164-8310 for the adjacent no-wall center-field pattern.
 */

static const char s_source_evidence[] =
    "Source-locked contract gate only; not full real-asset floor, ceiling, item, "
    "creature, projectile, explosion, pit, stairs, door, or field bitmap parity. "
    "DUNGEON.C:1423-1479 F0151_DUNGEON_GetSquare supplies the square byte and "
    "wall fallback; DUNGEON.C:1481-1492 F0152_DUNGEON_GetRelativeSquare resolves "
    "depth 1 lane 0 before F0151; DUNGEON.C:1495-1506 "
    "F0153_DUNGEON_GetRelativeSquareType applies M034_SQUARE_TYPE. "
    "DUNVIEW.C:8530-8533 F0128 dispatches D1C into "
    "F0124_DUNGEONVIEW_DrawSquareD1C; DUNVIEW.C:7727-7958 is the D1C body. "
    "DUNVIEW.C:7922-7937 is the no-wall corridor/pit/teleporter common route: "
    "DUNVIEW.C:7925 uses C0x3421, DUNVIEW.C:7926 calls F0108, "
    "DUNVIEW.C:7931-7935 calls F0112, and DUNVIEW.C:7937 calls F0115. "
    "DUNVIEW.C:7942-7956 gates F0113 teleporter field to C712_ZONE_WALL_D1C. "
    "DUNVIEW.C:7784-7872 is explicitly excluded: no F0100 wall bitmap, no F0107 "
    "wall ornament, and no wall-route return are part of this center-field slice. "
    "DUNVIEW.C:7873-7911 door-front bitmap route is excluded for closed doors; "
    "open-door fake injection is treated as the same no-wall field handoff. "
    "DUNVIEW.C:2962 F0098 floor/ceiling is delegated through "
    "dm1_viewport_3d_draw_floor_ceiling; dm1_v1_viewport_d0c_center_field "
    "anchors DUNVIEW.C:8164-8310 as the adjacent no-wall center-field reference.";

static uint8_t square_at(const DM1_V1_D1CCenterFieldDungeonPc34 *dungeon,
                         int16_t map_x,
                         int16_t map_y,
                         bool *in_bounds)
{
    if (!dungeon || !dungeon->cells ||
        map_x < 0 || map_y < 0 ||
        map_x >= dungeon->width || map_y >= dungeon->height) {
        if (in_bounds) *in_bounds = false;
        return 0;
    }
    if (in_bounds) *in_bounds = true;
    return dungeon->cells[(size_t)map_y * (size_t)dungeon->width + (size_t)map_x];
}

static void mark_target(DM1_V1_D1CCenterFieldTargetPc34 *target,
                        int x,
                        int y,
                        uint8_t color)
{
    if (!target || !target->pixels || target->width <= 0 ||
        target->height <= 0 || target->stride < target->width) {
        return;
    }
    if (x < 0 || y < 0 || x >= target->width || y >= target->height) {
        return;
    }
    target->pixels[(size_t)y * (size_t)target->stride + (size_t)x] = color;
}

static void mark_floor_ceiling(DM1_V1_D1CCenterFieldTargetPc34 *target)
{
    mark_target(target, 0, 0, 1);
    mark_target(target, target ? target->width / 2 : 0,
                target ? target->height - 1 : 0, 2);
}

static void mark_thing_pass(DM1_V1_D1CCenterFieldTargetPc34 *target,
                            const DM1_V1_D1CCenterFieldDungeonPc34 *dungeon,
                            DM1_V1_D1CCenterFieldRenderPc34 *out)
{
    if (!dungeon || !out) return;
    out->drew_item = dungeon->has_item;
    out->drew_creature = dungeon->has_creature;
    out->drew_projectile = dungeon->has_projectile;
    out->drew_explosion = dungeon->has_explosion;
    if (dungeon->has_item) mark_target(target, 4, 4, 3);
    if (dungeon->has_creature) mark_target(target, 5, 4, 4);
    if (dungeon->has_projectile) mark_target(target, 6, 4, 5);
    if (dungeon->has_explosion) mark_target(target, 7, 4, 6);
}

DM1_V1_D1CCenterFieldRenderPc34
dm1_v1_viewport_d1c_center_field_pc34_compat_render_square(
    const DM1_V1_D1CCenterFieldDungeonPc34 *dungeon,
    const DM1_V1_D1CCenterFieldPartyPc34 *party,
    DM1_Viewport3DState *viewport,
    DM1_V1_D1CCenterFieldTargetPc34 *target)
{
    DM1_V1_D1CCenterFieldRenderPc34 out = {0};
    bool in_bounds = false;

    out.route = DM1_V1_D1C_CENTER_FIELD_PC34_ROUTE_INVALID;
    out.square_type = DM1_V1_D1C_CENTER_FIELD_PC34_ELEMENT_WALL;
    out.view_square_index = DM1_V1_D1C_CENTER_FIELD_PC34_VIEW_SQUARE_INDEX;
    out.field_aspect = DM1_V1_D1C_CENTER_FIELD_PC34_FIELD_ASPECT;
    out.field_zone = DM1_V1_D1C_CENTER_FIELD_PC34_FIELD_ZONE;
    out.source_evidence = s_source_evidence;

    if (!dungeon || !party) return out;

    out.called_relative_square =
        dm1_viewport_3d_resolve_relative_map_xy(
            party->direction,
            DM1_V1_D1C_CENTER_FIELD_PC34_DEPTH,
            DM1_V1_D1C_CENTER_FIELD_PC34_LATERAL,
            party->map_x,
            party->map_y,
            &out.target_map_x,
            &out.target_map_y) ? true : false;
    if (!out.called_relative_square) return out;

    out.square = square_at(dungeon, out.target_map_x, out.target_map_y,
                           &in_bounds);
    out.called_get_square = true;
    out.square_type = (out.square >> 5) & 0x07;
    out.called_relative_square_type = true;
    if (!in_bounds) {
        out.square = 0;
        out.square_type = DM1_V1_D1C_CENTER_FIELD_PC34_ELEMENT_WALL;
    }

    out.called_d0c_reference_helper =
        dm1_v1_viewport_d0c_center_field_pc34_compat_source_evidence() != NULL;
    (void)dm1_floor_set_floor_graphic(0);
    (void)dm1_floor_set_ceiling_graphic(0);

    if (viewport && viewport->viewport_pixels &&
        viewport->viewport_stride >= DM1_VIEWPORT_WIDTH) {
        dm1_viewport_3d_draw_floor_ceiling(viewport);
        out.called_floor_ceiling_helper = true;
    }
    mark_floor_ceiling(target);

    switch (out.square_type) {
        case DM1_V1_D1C_CENTER_FIELD_PC34_ELEMENT_CORRIDOR:
        case DM1_V1_D1C_CENTER_FIELD_PC34_ELEMENT_FAKEWALL:
            out.route = DM1_V1_D1C_CENTER_FIELD_PC34_ROUTE_OPEN_FIELD;
            out.cell_order = DM1_V1_D1C_CENTER_FIELD_PC34_CELL_ORDER_OPEN;
            out.called_f0108_floor_ornament = true;
            out.called_f0112_ceiling_pit = true;
            out.called_f0115_thing_pass = true;
            mark_thing_pass(target, dungeon, &out);
            break;
        case DM1_V1_D1C_CENTER_FIELD_PC34_ELEMENT_TELEPORTER:
            out.route = DM1_V1_D1C_CENTER_FIELD_PC34_ROUTE_TELEPORTER_FIELD;
            out.cell_order = DM1_V1_D1C_CENTER_FIELD_PC34_CELL_ORDER_OPEN;
            out.called_f0108_floor_ornament = true;
            out.called_f0112_ceiling_pit = true;
            out.called_f0115_thing_pass = true;
            out.called_f0113_field = true;
            mark_thing_pass(target, dungeon, &out);
            mark_target(target, target ? target->width / 2 : 0,
                        target ? target->height / 2 : 0, 7);
            break;
        case DM1_V1_D1C_CENTER_FIELD_PC34_ELEMENT_PIT:
            out.route = DM1_V1_D1C_CENTER_FIELD_PC34_ROUTE_PIT_FIELD;
            out.cell_order = DM1_V1_D1C_CENTER_FIELD_PC34_CELL_ORDER_OPEN;
            out.called_f0104_pit_or_stairs = true;
            out.called_f0108_floor_ornament = true;
            out.called_f0112_ceiling_pit = true;
            out.called_f0115_thing_pass = true;
            mark_thing_pass(target, dungeon, &out);
            mark_target(target, target ? target->width / 2 : 0,
                        target ? target->height - 2 : 0, 8);
            break;
        case DM1_V1_D1C_CENTER_FIELD_PC34_ELEMENT_STAIRS:
            out.route = DM1_V1_D1C_CENTER_FIELD_PC34_ROUTE_STAIRS_FIELD;
            out.cell_order = DM1_V1_D1C_CENTER_FIELD_PC34_CELL_ORDER_OPEN;
            out.called_f0104_pit_or_stairs = true;
            out.called_f0108_floor_ornament = true;
            out.called_f0112_ceiling_pit = true;
            out.called_f0115_thing_pass = true;
            mark_thing_pass(target, dungeon, &out);
            mark_target(target, target ? target->width / 2 : 0, 1, 9);
            break;
        case DM1_V1_D1C_CENTER_FIELD_PC34_ELEMENT_DOOR:
            if (dungeon->door_is_open) {
                out.route = DM1_V1_D1C_CENTER_FIELD_PC34_ROUTE_OPEN_DOOR_FIELD;
                out.cell_order =
                    DM1_V1_D1C_CENTER_FIELD_PC34_CELL_ORDER_OPEN_DOOR;
                out.called_f0108_floor_ornament = true;
                out.called_f0112_ceiling_pit = true;
                out.called_f0115_thing_pass = true;
                mark_thing_pass(target, dungeon, &out);
            } else {
                out.route =
                    DM1_V1_D1C_CENTER_FIELD_PC34_ROUTE_CLOSED_DOOR_BLOCKED;
            }
            break;
        case DM1_V1_D1C_CENTER_FIELD_PC34_ELEMENT_WALL:
        default:
            out.route = DM1_V1_D1C_CENTER_FIELD_PC34_ROUTE_WALL_BLOCKED;
            break;
    }

    return out;
}
