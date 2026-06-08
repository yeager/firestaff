#include "csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_pc34_compat.h"

enum {
    CSB_PRESENT = 1,
    CSB_ABSENT = 0,
    CSB_D3L2_VIEW_SQUARE = 14,         /* ReDMCSB DEFS.H:2610 C14_VIEW_SQUARE_D3L2. */
    CSB_D3R2_VIEW_SQUARE = 15,         /* ReDMCSB DEFS.H:2611 C15_VIEW_SQUARE_D3R2. */
    CSB_D3_VIEW_DEPTH = 3,             /* ReDMCSB DUNVIEW.C:372 G2027[14/15]. */
    CSB_D3L2_VIEW_LANE = 254,          /* ReDMCSB DUNVIEW.C:371 G2026[14] == -2. */
    CSB_D3R2_VIEW_LANE = 2,            /* ReDMCSB DUNVIEW.C:371 G2026[15]. */
    CSB_D3L2_G2028_ROW = 3,            /* ReDMCSB DUNVIEW.C:373 G2028[14]. */
    CSB_D3R2_G2028_ROW = 4,            /* ReDMCSB DUNVIEW.C:373 G2028[15]. */
    CSB_D3L2_G2033_ROW = 3,            /* ReDMCSB DUNVIEW.C:375 G2033[14]. */
    CSB_D3R2_G2033_ROW = 4,            /* ReDMCSB DUNVIEW.C:375 G2033[15]. */
    CSB_D3L2_G2034_ROW = 6,            /* ReDMCSB DUNVIEW.C:376 G2034[14]. */
    CSB_D3R2_G2034_ROW = 7,            /* ReDMCSB DUNVIEW.C:376 G2034[15]. */
    CSB_D3L2_FIELD_ASPECT = 0,         /* ReDMCSB DUNVIEW.C:377 G2035[14]. */
    CSB_D3R2_FIELD_ASPECT = 1,         /* ReDMCSB DUNVIEW.C:377 G2035[15]. */
    CSB_D3L2_WALL_ZONE = 702,          /* ReDMCSB DEFS.H:4042 C702_ZONE_WALL_D3L2. */
    CSB_D3R2_WALL_ZONE = 703,          /* ReDMCSB DEFS.H:4043 C703_ZONE_WALL_D3R2. */
    CSB_PROJECTILE_ZONE_BASE = 2900,   /* ReDMCSB DEFS.H:4230 C2900_ZONE_. */
    CSB_OBJECT_ZONE_BASE = 2500,       /* ReDMCSB DEFS.H:4228 C2500_ZONE_. */
    CSB_CREATURE_ZONE_BASE = 3200,     /* ReDMCSB DEFS.H:4236 C3200_ZONE_. */
    CSB_EXPLOSION_REBIRTH1 = 3000,     /* ReDMCSB DEFS.H:4232 C3000_ZONE_. */
    CSB_EXPLOSION_REBIRTH2 = 3007,     /* ReDMCSB DEFS.H:4233 C3007_ZONE_. */
    CSB_EXPLOSION_CENTER = 3014,       /* ReDMCSB DEFS.H:4234 C3014_ZONE_. */
    CSB_EXPLOSION_SIDE = 3031,         /* ReDMCSB DEFS.H:4235 C3031_ZONE_. */
    CSB_CELL_STRIDE_4 = 4,             /* ReDMCSB DUNVIEW.C:5075/5683 row*4+cell. */
    CSB_CREATURE_COORD_STRIDE = 65,    /* ReDMCSB DUNVIEW.C:5616 CoordinateSet*65. */
    CSB_CREATURE_CELL_STRIDE = 5,      /* ReDMCSB DUNVIEW.C:5616 row*5+cell. */
    CSB_EXPLOSION_SIDE_STRIDE = 2,     /* ReDMCSB DUNVIEW.C:6122 row*2+cell. */
    CSB_OBJECT_CREATURE_SHIFT = 0x8000,
    CSB_C10_COLOR_FLESH = 10,          /* ReDMCSB DEFS.H:2088 C10_COLOR_FLESH. */
    CSB_DERIVED_BITMAP_NONE = -1,      /* ReDMCSB DUNVIEW.C:5859/5885. */
    CSB_COORD_PARENT_RECORD = 4,       /* ReDMCSB COORD.C layout records parent index. */
    CSB_LINEAGE_RF3L2 = 2,             /* CSB Viewport.cpp:330 RF3L2. */
    CSB_LINEAGE_RF3R2 = 10,            /* CSB Viewport.cpp:335 RF3R2. */
    CSB_LINEAGE_F3L2_CONTENTS = 60117, /* CSB Viewport.cpp:503. */
    CSB_LINEAGE_F3R2_CONTENTS = 60121, /* CSB Viewport.cpp:507. */
    CSB_LINEAGE_F3L2_XY = 60100,       /* CSB Viewport.cpp:485. */
    CSB_LINEAGE_F3R2_XY = 60104,       /* CSB Viewport.cpp:489. */
    CSB_LINEAGE_ROOM_OBJECTS = 60006,  /* CSB Viewport.cpp:379. */
    CSB_LINEAGE_DRAWORDER3421 = 60278, /* CSB Viewport.cpp:680/2628. */
    CSB_LINEAGE_DRAWORDER4312 = 60281, /* CSB Viewport.cpp:683/2635. */
    CSB_LINEAGE_DRAWORDER218 = 60279,  /* CSB Viewport.cpp:681/1906. */
    CSB_LINEAGE_DRAWORDER128 = 60283,  /* CSB Viewport.cpp:685. */
    CSB_D3L2_OPEN_ORDER = 0x3421,      /* ReDMCSB DUNVIEW.C:6282/6286. */
    CSB_D3R2_OPEN_ORDER = 0x4312,      /* ReDMCSB DUNVIEW.C:6349/6353. */
    CSB_D3L2_DOOR_REAR = 0x0218,       /* ReDMCSB DUNVIEW.C:6271. */
    CSB_D3R2_DOOR_REAR = 0x0128,       /* ReDMCSB DUNVIEW.C:6338. */
    CSB_D3L2_DOOR_FRONT = 0x0349,      /* ReDMCSB DUNVIEW.C:6273/6286. */
    CSB_D3R2_DOOR_FRONT = 0x0439       /* ReDMCSB DUNVIEW.C:6340/6353. */
};

static int s_initialized;

static const char s_source_evidence[] =
    "CSB-only source-locked contract-only gate; no real-asset bitmap parity "
    "and no CSB game-data load. ReDMCSB DUNVIEW.C:6226-6291 "
    "F0676_DrawD3L2 and DUNVIEW.C:6293-6358 F0677_DrawD3R2 route D3L2/D3R2 "
    "corridor, pit, teleporter, door-side, and door-front squares through "
    "F0115, with D3L2 open order 0x3421 and D3R2 open order 0x4312. "
    "DUNVIEW.C:6361-6480 F0116 and DUNVIEW.C:6500-6622 F0117 are neighboring D3L/"
    "D3R inner-square routes and are anchor-table context, not owners of "
    "this D3L2/D3R2 slice. DUNVIEW.C:4547-4581 "
    "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF "
    "orders objects, creatures, projectiles, and explosions; 4806-4811 loads "
    "G2026/G2027/G2028; 4923 and 5075 bind C2500|MASK0x8000 item rows; "
    "5201-5214 and 5615-5627 bind C3200|MASK0x8000 creature rows; "
    "5668-5671 is the projectile G2028 row guard and 5672-5683 suppresses "
    "depth-3 front cells, restarts the list, requires C14 projectile plus "
    "cell match, and binds C2900 + row*4 + ViewCell; 5915-5933 restarts "
    "explosions using G2034/G2035; 5998-5999/6094-6096/6106-6107/"
    "6121-6122 bind C3000/C3007/C3014/C3031; 6192-6193 uses F0791 with "
    "C10_COLOR_FLESH; 6202-6219 defers fluxcages through F0113. "
    "DUNVIEW.C:8478-8508 F0128 dispatches D3L2 at relative 3,-2 before "
    "D3R2 at 3,+2, then D3L/D3R and D2L2/D2R2. DEFS.H:2088 "
    "C10_COLOR_FLESH; DEFS.H:4042 C702_ZONE_WALL_D3L2; DEFS.H:4043 "
    "C703_ZONE_WALL_D3R2; DEFS.H:2610 C14_VIEW_SQUARE_D3L2; DEFS.H:2611 "
    "C15_VIEW_SQUARE_D3R2. COORD.C:1129-1193,1058-1123,1243-1252 parent "
    "records anchor item, explosion, and creature layout ranges. "
    "CSB Viewport.cpp D3L2/D3R2 thing-pass path: RF3L2/RF3R2 at 330/335, "
    "F3L2/F3R2 contents at 503/507, StdDrawRoomObjects at 379, draw-order "
    "opcodes at 680-685 and 2628-2637. CSB Viewport.cpp:1903-1906 records "
    "the room-object overlay binding with DrawOrder218/StdDrawRoomObjects. "
    "NO-DOOR-PANEL: F0111 is explicitly excluded from this thing-pass "
    "contract even when F0676/F0677 bracket door-front panels with two F0115 "
    "passes.";

static const CSB_V1_D3L2D3R2F0115ThingPassEvidencePc34 s_evidence = {
    "CSB V1 D3L2/D3R2 F0115 thing-pass source-lock gate; contract-only, "
    "synthetic pixel fixtures only.",
    "ReDMCSB DUNVIEW.C:6226-6291 F0676_DrawD3L2",
    "ReDMCSB DUNVIEW.C:6293-6358 F0677_DrawD3R2",
    "ReDMCSB DUNVIEW.C:6361-6480 F0116_DUNGEONVIEW_DrawSquareD3L",
    "ReDMCSB DUNVIEW.C:6500-6622 F0117_DUNGEONVIEW_DrawSquareD3R",
    "ReDMCSB DUNVIEW.C:4547-4581 F0115 thing-pass order",
    "ReDMCSB DUNVIEW.C:5668-5671 F0115 projectile row guard",
    "ReDMCSB DUNVIEW.C:8478-8508 F0128 D3L2 before D3R2 dispatch",
    "ReDMCSB DEFS.H:2088 C10, 4042 C702, 4043 C703, 2610 C14, 2611 C15",
    "ReDMCSB COORD.C:1129-1193 item, 1058-1123 explosion, 1243-1252 creature parent records",
    "CSB Viewport.cpp:330/335,503/507,680-685,2628-2637 D3L2/D3R2 thing-pass path",
    "CSB Viewport.cpp:1903-1906 room-object overlay binding"
};

#define SPEC_COMMON(square, route_id, name, index, order, lateral, lane, wall, field, \
                    g2028, g2033, g2034, open_order, rear_order, front_order, \
                    lineage_cell, contents, xy, draw_order, func, lines) \
    { \
        .source_locked_contract_only = CSB_PRESENT, \
        .no_real_asset_bitmap_parity = CSB_PRESENT, \
        .no_game_data_load = CSB_PRESENT, \
        .view_square = square, \
        .route = route_id, \
        .route_name = name, \
        .spec_table_index = index, \
        .f0128_dispatch_order = order, \
        .f0128_relative_depth = 3, \
        .f0128_relative_lateral = lateral, \
        .view_depth = CSB_D3_VIEW_DEPTH, \
        .view_lane = lane, \
        .wall_zone = wall, \
        .field_aspect_index = field, \
        .f0676_f0677_has_f0115_route = CSB_PRESENT, \
        .open_cell_order = open_order, \
        .door_front_rear_f0115_order = rear_order, \
        .door_front_f0111_order = CSB_PRESENT, \
        .door_front_front_f0115_order = front_order, \
        .no_door_panel_marker = CSB_PRESENT, \
        .f0111_excluded_from_thing_pass = CSB_PRESENT, \
        .f0116_f0117_inner_square_non_owner = CSB_PRESENT, \
        .f0128_left_then_right_dispatch = CSB_PRESENT, \
        .thing_pass_route_enabled = CSB_PRESENT, \
        .g2028_row = g2028, \
        .g2033_row = g2033, \
        .g2034_row = g2034, \
        .projectile_row_selection_uses_g2028 = CSB_PRESENT, \
        .projectile_row_guard_5668_5671 = CSB_PRESENT, \
        .projectile_restarts_thing_list = CSB_PRESENT, \
        .projectile_requires_type_c14 = CSB_PRESENT, \
        .projectile_requires_cell_match = CSB_PRESENT, \
        .projectile_suppresses_depth3_front_cells = CSB_PRESENT, \
        .projectile_suppresses_depth0_back_cells = CSB_ABSENT, \
        .projectile_zone_base = CSB_PROJECTILE_ZONE_BASE, \
        .projectile_zone_cell_stride = CSB_CELL_STRIDE_4, \
        .item_requires_weapon_to_junk = CSB_PRESENT, \
        .item_requires_cell_match = CSB_PRESENT, \
        .item_suppresses_depth3_front_cells = CSB_PRESENT, \
        .item_suppresses_depth0_back_cells = CSB_ABSENT, \
        .item_zone_base = CSB_OBJECT_ZONE_BASE, \
        .item_zone_cell_stride = CSB_CELL_STRIDE_4, \
        .item_shift_mask = CSB_OBJECT_CREATURE_SHIFT, \
        .item_pile_shift_advances = CSB_PRESENT, \
        .creature_requires_group_marker = CSB_PRESENT, \
        .creature_zone_base = CSB_CREATURE_ZONE_BASE, \
        .creature_coordinate_set_stride = CSB_CREATURE_COORD_STRIDE, \
        .creature_zone_cell_stride = CSB_CREATURE_CELL_STRIDE, \
        .creature_shift_mask = CSB_OBJECT_CREATURE_SHIFT, \
        .explosion_restarts_thing_list_after_cells = CSB_PRESENT, \
        .explosion_restart_uses_g2034_g2035 = CSB_PRESENT, \
        .explosion_rebirth_step1_zone_base = CSB_EXPLOSION_REBIRTH1, \
        .explosion_rebirth_step2_zone_base = CSB_EXPLOSION_REBIRTH2, \
        .explosion_centered_zone_base = CSB_EXPLOSION_CENTER, \
        .explosion_side_zone_base = CSB_EXPLOSION_SIDE, \
        .explosion_side_zone_cell_stride = CSB_EXPLOSION_SIDE_STRIDE, \
        .fluxcage_defers_to_f0113 = CSB_PRESENT, \
        .fluxcage_field_zone = wall, \
        .transparent_color = CSB_C10_COLOR_FLESH, \
        .c10_transparency_skip = CSB_PRESENT, \
        .derived_bitmap_none = CSB_DERIVED_BITMAP_NONE, \
        .scaled_path_uses_cm1_derived_bitmap_none = CSB_PRESENT, \
        .dynamic_horizontal_flip_flag = (index & 1), \
        .dynamic_vertical_flip_flag = ((index >> 1) & 1), \
        .coord_item_parent_record = CSB_COORD_PARENT_RECORD, \
        .coord_explosion_parent_record = CSB_COORD_PARENT_RECORD, \
        .coord_creature_parent_record = CSB_COORD_PARENT_RECORD, \
        .csb_lineage_relative_cell = lineage_cell, \
        .csb_lineage_contents_opcode = contents, \
        .csb_lineage_xy_opcode = xy, \
        .csb_lineage_draw_order_opcode = draw_order, \
        .csb_lineage_std_draw_room_objects_opcode = CSB_LINEAGE_ROOM_OBJECTS, \
        .redmcsb_function = func, \
        .source_lines = lines \
    }

static const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 s_specs[] = {
    SPEC_COMMON(CSB_D3L2_VIEW_SQUARE,
                CSB_V1_D3L2_D3R2_F0115_ROUTE_PROJECTILE_PC34,
                "D3L2 projectile thing pass", 0, 0, -2, CSB_D3L2_VIEW_LANE,
                CSB_D3L2_WALL_ZONE, CSB_D3L2_FIELD_ASPECT, CSB_D3L2_G2028_ROW,
                CSB_D3L2_G2033_ROW, CSB_D3L2_G2034_ROW, CSB_D3L2_OPEN_ORDER,
                CSB_D3L2_DOOR_REAR, CSB_D3L2_DOOR_FRONT, CSB_LINEAGE_RF3L2,
                CSB_LINEAGE_F3L2_CONTENTS, CSB_LINEAGE_F3L2_XY,
                CSB_LINEAGE_DRAWORDER3421,
                "DUNVIEW.C F0676_DrawD3L2 / F0115 projectile route",
                "ReDMCSB DUNVIEW.C:6226-6291,4547-4581,5668-5683"),
    SPEC_COMMON(CSB_D3L2_VIEW_SQUARE,
                CSB_V1_D3L2_D3R2_F0115_ROUTE_CREATURE_PC34,
                "D3L2 creature thing pass", 1, 0, -2, CSB_D3L2_VIEW_LANE,
                CSB_D3L2_WALL_ZONE, CSB_D3L2_FIELD_ASPECT, CSB_D3L2_G2028_ROW,
                CSB_D3L2_G2033_ROW, CSB_D3L2_G2034_ROW, CSB_D3L2_OPEN_ORDER,
                CSB_D3L2_DOOR_REAR, CSB_D3L2_DOOR_FRONT, CSB_LINEAGE_RF3L2,
                CSB_LINEAGE_F3L2_CONTENTS, CSB_LINEAGE_F3L2_XY,
                CSB_LINEAGE_DRAWORDER3421,
                "DUNVIEW.C F0676_DrawD3L2 / F0115 creature route",
                "ReDMCSB DUNVIEW.C:6226-6291,4547-4581,5201-5214,5615-5627"),
    SPEC_COMMON(CSB_D3L2_VIEW_SQUARE,
                CSB_V1_D3L2_D3R2_F0115_ROUTE_ITEM_PC34,
                "D3L2 item thing pass", 2, 0, -2, CSB_D3L2_VIEW_LANE,
                CSB_D3L2_WALL_ZONE, CSB_D3L2_FIELD_ASPECT, CSB_D3L2_G2028_ROW,
                CSB_D3L2_G2033_ROW, CSB_D3L2_G2034_ROW, CSB_D3L2_OPEN_ORDER,
                CSB_D3L2_DOOR_REAR, CSB_D3L2_DOOR_FRONT, CSB_LINEAGE_RF3L2,
                CSB_LINEAGE_F3L2_CONTENTS, CSB_LINEAGE_F3L2_XY,
                CSB_LINEAGE_DRAWORDER3421,
                "DUNVIEW.C F0676_DrawD3L2 / F0115 item route",
                "ReDMCSB DUNVIEW.C:6226-6291,4547-4581,4923,5075"),
    SPEC_COMMON(CSB_D3L2_VIEW_SQUARE,
                CSB_V1_D3L2_D3R2_F0115_ROUTE_EXPLOSION_PC34,
                "D3L2 explosion thing pass", 3, 0, -2, CSB_D3L2_VIEW_LANE,
                CSB_D3L2_WALL_ZONE, CSB_D3L2_FIELD_ASPECT, CSB_D3L2_G2028_ROW,
                CSB_D3L2_G2033_ROW, CSB_D3L2_G2034_ROW, CSB_D3L2_OPEN_ORDER,
                CSB_D3L2_DOOR_REAR, CSB_D3L2_DOOR_FRONT, CSB_LINEAGE_RF3L2,
                CSB_LINEAGE_F3L2_CONTENTS, CSB_LINEAGE_F3L2_XY,
                CSB_LINEAGE_DRAWORDER218,
                "DUNVIEW.C F0676_DrawD3L2 / F0115 explosion route",
                "ReDMCSB DUNVIEW.C:6226-6291,4547-4581,5915-5933,5998-6122"),
    SPEC_COMMON(CSB_D3R2_VIEW_SQUARE,
                CSB_V1_D3L2_D3R2_F0115_ROUTE_PROJECTILE_PC34,
                "D3R2 projectile thing pass", 4, 1, 2, CSB_D3R2_VIEW_LANE,
                CSB_D3R2_WALL_ZONE, CSB_D3R2_FIELD_ASPECT, CSB_D3R2_G2028_ROW,
                CSB_D3R2_G2033_ROW, CSB_D3R2_G2034_ROW, CSB_D3R2_OPEN_ORDER,
                CSB_D3R2_DOOR_REAR, CSB_D3R2_DOOR_FRONT, CSB_LINEAGE_RF3R2,
                CSB_LINEAGE_F3R2_CONTENTS, CSB_LINEAGE_F3R2_XY,
                CSB_LINEAGE_DRAWORDER4312,
                "DUNVIEW.C F0677_DrawD3R2 / F0115 projectile route",
                "ReDMCSB DUNVIEW.C:6293-6358,4547-4581,5668-5683"),
    SPEC_COMMON(CSB_D3R2_VIEW_SQUARE,
                CSB_V1_D3L2_D3R2_F0115_ROUTE_CREATURE_PC34,
                "D3R2 creature thing pass", 5, 1, 2, CSB_D3R2_VIEW_LANE,
                CSB_D3R2_WALL_ZONE, CSB_D3R2_FIELD_ASPECT, CSB_D3R2_G2028_ROW,
                CSB_D3R2_G2033_ROW, CSB_D3R2_G2034_ROW, CSB_D3R2_OPEN_ORDER,
                CSB_D3R2_DOOR_REAR, CSB_D3R2_DOOR_FRONT, CSB_LINEAGE_RF3R2,
                CSB_LINEAGE_F3R2_CONTENTS, CSB_LINEAGE_F3R2_XY,
                CSB_LINEAGE_DRAWORDER4312,
                "DUNVIEW.C F0677_DrawD3R2 / F0115 creature route",
                "ReDMCSB DUNVIEW.C:6293-6358,4547-4581,5201-5214,5615-5627"),
    SPEC_COMMON(CSB_D3R2_VIEW_SQUARE,
                CSB_V1_D3L2_D3R2_F0115_ROUTE_ITEM_PC34,
                "D3R2 item thing pass", 6, 1, 2, CSB_D3R2_VIEW_LANE,
                CSB_D3R2_WALL_ZONE, CSB_D3R2_FIELD_ASPECT, CSB_D3R2_G2028_ROW,
                CSB_D3R2_G2033_ROW, CSB_D3R2_G2034_ROW, CSB_D3R2_OPEN_ORDER,
                CSB_D3R2_DOOR_REAR, CSB_D3R2_DOOR_FRONT, CSB_LINEAGE_RF3R2,
                CSB_LINEAGE_F3R2_CONTENTS, CSB_LINEAGE_F3R2_XY,
                CSB_LINEAGE_DRAWORDER4312,
                "DUNVIEW.C F0677_DrawD3R2 / F0115 item route",
                "ReDMCSB DUNVIEW.C:6293-6358,4547-4581,4923,5075"),
    SPEC_COMMON(CSB_D3R2_VIEW_SQUARE,
                CSB_V1_D3L2_D3R2_F0115_ROUTE_EXPLOSION_PC34,
                "D3R2 explosion thing pass", 7, 1, 2, CSB_D3R2_VIEW_LANE,
                CSB_D3R2_WALL_ZONE, CSB_D3R2_FIELD_ASPECT, CSB_D3R2_G2028_ROW,
                CSB_D3R2_G2033_ROW, CSB_D3R2_G2034_ROW, CSB_D3R2_OPEN_ORDER,
                CSB_D3R2_DOOR_REAR, CSB_D3R2_DOOR_FRONT, CSB_LINEAGE_RF3R2,
                CSB_LINEAGE_F3R2_CONTENTS, CSB_LINEAGE_F3R2_XY,
                CSB_LINEAGE_DRAWORDER128,
                "DUNVIEW.C F0677_DrawD3R2 / F0115 explosion route",
                "ReDMCSB DUNVIEW.C:6293-6358,4547-4581,5915-5933,5998-6122")
};

#undef SPEC_COMMON

int csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_init_pc34(void)
{
    s_initialized = CSB_PRESENT;
    return s_initialized;
}

size_t csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_count_pc34(void)
{
    if (!s_initialized) csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_init_pc34();
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 *
csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_at_pc34(size_t index)
{
    if (index >= csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_count_pc34()) {
        return 0;
    }
    return &s_specs[index];
}

const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 *
csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_for_route_pc34(
    int view_square,
    int route)
{
    size_t i;
    for (i = 0;
         i < csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_count_pc34();
         ++i) {
        if (s_specs[i].view_square == view_square && s_specs[i].route == route) {
            return &s_specs[i];
        }
    }
    return 0;
}

const CSB_V1_D3L2D3R2F0115ThingPassEvidencePc34 *
csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_evidence_pc34(void)
{
    return &s_evidence;
}

static int d3_visible_thing_cell(
    const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 *spec,
    int view_cell)
{
    if (!spec || view_cell < 0 || view_cell > 3) return 0;
    if (spec->view_depth == 3 && view_cell <= 1) return 0;
    if (spec->view_depth == 0 && view_cell >= 2) return 0;
    return 1;
}

int csb_v1_viewport_d3l2_d3r2_f0115_projectile_zone_pc34(
    const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 *spec,
    int view_cell)
{
    if (!d3_visible_thing_cell(spec, view_cell) || spec->g2028_row < 0) return -1;
    /* ReDMCSB: DUNVIEW.C:5668-5683 row guard plus C2900 + row*4 + ViewCell. */
    return spec->projectile_zone_base +
           (spec->g2028_row * spec->projectile_zone_cell_stride) +
           view_cell;
}

int csb_v1_viewport_d3l2_d3r2_f0115_item_layout_zone_pc34(
    const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 *spec,
    int view_cell)
{
    if (!d3_visible_thing_cell(spec, view_cell) || spec->g2028_row < 0) return -1;
    /* ReDMCSB: DUNVIEW.C:4923/5075 C2500 + row*4 + ViewCell. */
    return spec->item_zone_base +
           (spec->g2028_row * spec->item_zone_cell_stride) +
           view_cell;
}

int csb_v1_viewport_d3l2_d3r2_f0115_item_zone_pc34(
    const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 *spec,
    int view_cell)
{
    const int zone =
        csb_v1_viewport_d3l2_d3r2_f0115_item_layout_zone_pc34(spec, view_cell);
    if (!spec || zone < 0) return -1;
    return zone | spec->item_shift_mask;
}

int csb_v1_viewport_d3l2_d3r2_f0115_creature_zone_pc34(
    const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 *spec,
    int coordinate_set,
    int view_cell)
{
    if (!d3_visible_thing_cell(spec, view_cell) ||
        coordinate_set < 0 || spec->g2033_row < 0) {
        return -1;
    }
    /* ReDMCSB: DUNVIEW.C:5615-5627 C3200 + CoordinateSet*65 + row*5 + cell. */
    return (spec->creature_zone_base | spec->creature_shift_mask) +
           (coordinate_set * spec->creature_coordinate_set_stride) +
           (spec->g2033_row * spec->creature_zone_cell_stride) +
           view_cell;
}

int csb_v1_viewport_d3l2_d3r2_f0115_explosion_rebirth_step1_zone_pc34(
    const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 *spec)
{
    if (!spec || spec->g2034_row < 0) return -1;
    return spec->explosion_rebirth_step1_zone_base + spec->g2034_row;
}

int csb_v1_viewport_d3l2_d3r2_f0115_explosion_rebirth_step2_zone_pc34(
    const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 *spec)
{
    if (!spec || spec->g2034_row < 0) return -1;
    return spec->explosion_rebirth_step2_zone_base + spec->g2034_row;
}

int csb_v1_viewport_d3l2_d3r2_f0115_explosion_centered_zone_pc34(
    const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 *spec)
{
    if (!spec || spec->g2034_row < 0) return -1;
    return spec->explosion_centered_zone_base + spec->g2034_row;
}

int csb_v1_viewport_d3l2_d3r2_f0115_explosion_side_zone_pc34(
    const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 *spec,
    int view_cell)
{
    if (!spec || view_cell < 0 || view_cell > 1 || spec->g2034_row < 0) {
        return -1;
    }
    return spec->explosion_side_zone_base +
           (spec->g2034_row * spec->explosion_side_zone_cell_stride) +
           view_cell;
}

int csb_v1_viewport_d3l2_d3r2_f0115_apply_c10_blit_pc34(
    const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 *spec,
    const uint8_t *source,
    int source_stride,
    uint8_t *destination,
    int destination_stride,
    int width,
    int height,
    int flip_horizontal,
    int flip_vertical)
{
    int copied = 0;
    if (!spec || !source || !destination ||
        width <= 0 || height <= 0 ||
        source_stride < width || destination_stride < width) {
        return -1;
    }

    /* ReDMCSB: DUNVIEW.C:5881-5882 and 6192-6193 pass dynamic flip flags
     * to F0791 with C10_COLOR_FLESH transparency. */
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const int sx = flip_horizontal ? (width - 1 - x) : x;
            const int sy = flip_vertical ? (height - 1 - y) : y;
            const uint8_t pixel = source[(sy * source_stride) + sx];
            if (pixel == (uint8_t)spec->transparent_color) continue;
            destination[(y * destination_stride) + x] = pixel;
            ++copied;
        }
    }
    return copied;
}

const char *csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_source_evidence_pc34(void)
{
    return s_source_evidence;
}
