#include "firestaff/dm1/v1/viewport/dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_pc34_compat.h"

#include <string.h>

enum {
    DM1_D3L_VIEW_SQUARE = 12,
    DM1_D3R_VIEW_SQUARE = 13,
    DM1_D3L_WALL_ZONE = 705,
    DM1_D3R_WALL_ZONE = 706,
    DM1_D3L_FLOOR_VIEW = 2,
    DM1_D3R_FLOOR_VIEW = 4,
    DM1_D3L_FIELD_ASPECT = 3,
    DM1_D3R_FIELD_ASPECT = 4,
    DM1_C1004_ZONE_WALL_ORNAMENT = 1004,
    DM1_WALL_ORNAMENT_ZONE_STRIDE = 15,
    DM1_C1500_ZONE_FLOOR_ORNAMENT = 1500,
    DM1_M550_FIRST_THING_SLOT = 2,
    DM1_M551_RIGHT_WALL_ORNAMENT_SLOT = 4,
    DM1_M552_FRONT_WALL_ORNAMENT_SLOT = 5,
    DM1_M553_LEFT_WALL_ORNAMENT_SLOT = 6,
    DM1_M575_VIEW_WALL_D3L_RIGHT = 2,
    DM1_M576_VIEW_WALL_D3R_LEFT = 3,
    DM1_M577_VIEW_WALL_D3L_FRONT = 4,
    DM1_M578_VIEW_WALL_D3C_FRONT = 5,
    DM1_M579_VIEW_WALL_D3R_FRONT = 6,
    DM1_D3L_DOOR_ZONE = 3720,
    DM1_D3R_DOOR_ZONE = 3740,
    DM1_ALCOVE_ORDER = 0x0000,
    DM1_D3L_CORRIDOR_ORDER = 0x3421,
    DM1_D3R_CORRIDOR_ORDER = 0x4312,
    DM1_D3L_DOOR_SIDE_ORDER = 0x0321,
    DM1_D3R_DOOR_SIDE_ORDER = 0x0412,
    DM1_D3L_DOOR_PASS1_ORDER = 0x0218,
    DM1_D3R_DOOR_PASS1_ORDER = 0x0128,
    DM1_D3L_DOOR_PASS2_ORDER = 0x0349,
    DM1_D3R_DOOR_PASS2_ORDER = 0x0439
};

/* ReDMCSB: DUNVIEW.C F0107:3502-3938; F0116:6361-6498; F0117:6500-6640;
 * F0128:8491-8499 and 8503-8517; F0108:3940-4011; F0115:4547-4581.
 * This contract pins only the D3L/D3R F0107 route and deliberately keeps
 * the D0L/D0R, D1C, and D2L/D2R F0107 sibling files untouched. */
static const char s_source_evidence[] =
    "ReDMCSB source-lock: DUNVIEW.C F0107:3502-3938 is the wall-ornament "
    "dispatch body. It decrements non-zero ordinals, resolves "
    "G0205_aaauc_Graphic558_WallOrnamentCoordinateSets, computes "
    "C1004_ZONE_WALL_ORNAMENT + CoordinateSet*C15 + ViewWall at "
    "3586-3587, queries F0149 at 3589, and draws with C10_COLOR_FLESH "
    "at 3922 before returning the alcove boolean at 3933. DUNVIEW.C "
    "F0116:6361-6498 is the D3L body: wall case draws C705 at 6427, "
    "calls F0107 with M551/M575 at 6432 and M552/M577 at 6433, and "
    "uses C0x0000 alcove F0115 at 6434-6480. DUNVIEW.C F0117:6500-6640 "
    "is the D3R partner: wall case draws C706 at 6563, calls F0107 with "
    "M553/M576 at 6568 and M552/M579 at 6569, and uses C0x0000 alcove "
    "F0115 at 6570-6622. DUNVIEW.C F0128:8491-8499 dispatches D3L, "
    "then D3R, then D3C; D2L/D2R follow in the F0128:8503-8517 "
    "tail and D2C follows at 8521, so D3L/D3R "
    "are the terminal-depth side pair, not a later draw-order pair. "
    "DUNVIEW.C F0108:3940-4011 is the floor+ceiling baseline for D3L/"
    "D3R open and door-front paths at 6443/6478 and 6579/6620. "
    "DUNVIEW.C F0112 ceiling-pit routing is absent from D3L/D3R, while "
    "F0113 teleporter fields run after F0115 at 6488-6496 and 6630-6638. "
    "DUNVIEW.C F0115:4547-4581 defines the nibble-walk used by "
    "C0x3421/C0x4312 and the D3L/D3R door-pass orders. DUNGEON.C "
    "F0163:1769-1838 and F0164:1840-1905 define thing-list mutation "
    "boundaries; F0172:2466-2523 populates sensor-provided M551/M552/"
    "M553 ordinals. DEFS.H:2088 defines C10_COLOR_FLESH; DEFS.H:"
    "2538-2554 defines M550/M551/M552/M553; DEFS.H:2596-2611 defines "
    "M601/M602; DEFS.H:2696-2711 defines C0/C1/M575..M579 wall "
    "positions; DEFS.H:4045-4046 defines C705/C706; DEFS.H:4221-4225 "
    "defines G0205/G0206/G0207/G0208 zone bases.";

static const char s_disjointness_note[] =
    "D3L/D3R F0107 wall-ornament contract only. It covers the far-depth "
    "side-lane pair F0116/F0117 and its four direct F0107 calls: D3L "
    "side/front and D3R side/front. It does not touch or duplicate "
    "D0L/D0R, D1C, or D2L/D2R F0107 gates, and does not claim original "
    "DOS pixel parity or read GRAPHICS.DAT.";

static uint32_t fnv1a_u32(uint32_t hash, uint32_t value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (value >> (i * 8)) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

bool dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_returns_alcove_pc34(
    int wall_ornament_ordinal,
    bool dungeon_classifies_alcove)
{
    return wall_ornament_ordinal != 0 && dungeon_classifies_alcove;
}

int dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_zone_pc34(
    int coordinate_set,
    int view_wall)
{
    if (coordinate_set < 0 || view_wall < 0) return -1;
    return DM1_C1004_ZONE_WALL_ORNAMENT +
           coordinate_set * DM1_WALL_ORNAMENT_ZONE_STRIDE + view_wall;
}

uint8_t dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color)
{
    return source_pixel == transparent_color ? destination_pixel : source_pixel;
}

int dm1_v1_viewport_d3l_d3r_f0107_decode_cell_order_pc34(
    unsigned int cell_order,
    int ordinal_index)
{
    unsigned int nibble;

    if (ordinal_index < 0 || ordinal_index >= 4) return -1;
    if ((cell_order & 0x0fu) == 0x08u || (cell_order & 0x0fu) == 0x09u) {
        cell_order >>= 4;
    }
    nibble = (cell_order >> ((unsigned int)ordinal_index * 4u)) & 0x0fu;
    return nibble ? (int)nibble - 1 : -1;
}

static void fill_lanes(DM1_V1_D3LD3RF0107WallOrnamentModelPc34 *m)
{
    m->lanes[0].side = DM1_V1_D3L_D3R_F0107_SIDE_D3L_PC34;
    m->lanes[0].side_name = "D3L";
    m->lanes[0].view_square = DM1_D3L_VIEW_SQUARE;
    m->lanes[0].relative_depth = 3;
    m->lanes[0].relative_lateral = -1;
    m->lanes[0].wall_zone = DM1_D3L_WALL_ZONE;
    m->lanes[0].floor_view = DM1_D3L_FLOOR_VIEW;
    m->lanes[0].field_aspect_index = DM1_D3L_FIELD_ASPECT;
    m->lanes[0].f0128_update_line = 8490;
    m->lanes[0].f0128_draw_line = 8491;
    m->lanes[0].dispatcher_line_start = 6361;
    m->lanes[0].dispatcher_line_end = 6498;
    m->lanes[0].wall_case_line = 6406;
    m->lanes[0].wall_zone_draw_line = 6427;
    m->lanes[0].side_f0107_line = 6432;
    m->lanes[0].side_ornament_slot = DM1_M551_RIGHT_WALL_ORNAMENT_SLOT;
    m->lanes[0].side_view_wall = DM1_M575_VIEW_WALL_D3L_RIGHT;
    m->lanes[0].front_f0107_line = 6433;
    m->lanes[0].front_ornament_slot = DM1_M552_FRONT_WALL_ORNAMENT_SLOT;
    m->lanes[0].front_view_wall = DM1_M577_VIEW_WALL_D3L_FRONT;
    m->lanes[0].alcove_order = DM1_ALCOVE_ORDER;
    m->lanes[0].corridor_order = DM1_D3L_CORRIDOR_ORDER;
    m->lanes[0].door_side_order = DM1_D3L_DOOR_SIDE_ORDER;
    m->lanes[0].door_pass1_order = DM1_D3L_DOOR_PASS1_ORDER;
    m->lanes[0].door_pass2_order = DM1_D3L_DOOR_PASS2_ORDER;
    m->lanes[0].f0108_door_front_line = 6443;
    m->lanes[0].f0108_open_path_line = 6478;
    m->lanes[0].f0112_ceiling_line = 0;
    m->lanes[0].f0115_line = 6480;
    m->lanes[0].f0113_field_line = 6495;
    m->lanes[0].f0108_before_f0112 = 1;
    m->lanes[0].f0112_before_f0115 = 0;
    m->lanes[0].f0115_before_f0113 = 1;
    m->lanes[0].f0111_door_zone = DM1_D3L_DOOR_ZONE;
    m->lanes[0].f0111_line = 6457;
    m->lanes[0].redmcsb_anchor =
        "DUNVIEW.C F0116:6361-6498; F0107 at 6432/6433; F0108 at 6443/6478; F0115 at 6480; F0113 at 6495";

    m->lanes[1].side = DM1_V1_D3L_D3R_F0107_SIDE_D3R_PC34;
    m->lanes[1].side_name = "D3R";
    m->lanes[1].view_square = DM1_D3R_VIEW_SQUARE;
    m->lanes[1].relative_depth = 3;
    m->lanes[1].relative_lateral = 1;
    m->lanes[1].wall_zone = DM1_D3R_WALL_ZONE;
    m->lanes[1].floor_view = DM1_D3R_FLOOR_VIEW;
    m->lanes[1].field_aspect_index = DM1_D3R_FIELD_ASPECT;
    m->lanes[1].f0128_update_line = 8494;
    m->lanes[1].f0128_draw_line = 8495;
    m->lanes[1].dispatcher_line_start = 6500;
    m->lanes[1].dispatcher_line_end = 6640;
    m->lanes[1].wall_case_line = 6545;
    m->lanes[1].wall_zone_draw_line = 6563;
    m->lanes[1].side_f0107_line = 6568;
    m->lanes[1].side_ornament_slot = DM1_M553_LEFT_WALL_ORNAMENT_SLOT;
    m->lanes[1].side_view_wall = DM1_M576_VIEW_WALL_D3R_LEFT;
    m->lanes[1].front_f0107_line = 6569;
    m->lanes[1].front_ornament_slot = DM1_M552_FRONT_WALL_ORNAMENT_SLOT;
    m->lanes[1].front_view_wall = DM1_M579_VIEW_WALL_D3R_FRONT;
    m->lanes[1].alcove_order = DM1_ALCOVE_ORDER;
    m->lanes[1].corridor_order = DM1_D3R_CORRIDOR_ORDER;
    m->lanes[1].door_side_order = DM1_D3R_DOOR_SIDE_ORDER;
    m->lanes[1].door_pass1_order = DM1_D3R_DOOR_PASS1_ORDER;
    m->lanes[1].door_pass2_order = DM1_D3R_DOOR_PASS2_ORDER;
    m->lanes[1].f0108_door_front_line = 6579;
    m->lanes[1].f0108_open_path_line = 6620;
    m->lanes[1].f0112_ceiling_line = 0;
    m->lanes[1].f0115_line = 6622;
    m->lanes[1].f0113_field_line = 6637;
    m->lanes[1].f0108_before_f0112 = 1;
    m->lanes[1].f0112_before_f0115 = 0;
    m->lanes[1].f0115_before_f0113 = 1;
    m->lanes[1].f0111_door_zone = DM1_D3R_DOOR_ZONE;
    m->lanes[1].f0111_line = 6599;
    m->lanes[1].redmcsb_anchor =
        "DUNVIEW.C F0117:6500-6640; F0107 at 6568/6569; F0108 at 6579/6620; F0115 at 6622; F0113 at 6637";
}

static void fill_ordinals(DM1_V1_D3LD3RF0107WallOrnamentModelPc34 *m)
{
    static const DM1_V1_D3LD3RF0107OrdinalFlowPc34 flows[] = {
        { 0, "C0 D3L2 right-side wall position", DM1_M551_RIGHT_WALL_ORNAMENT_SLOT,
          0, 0, 0, 0, 1, "DEFS.H:2696 C00; D3L/D3R direct gate excludes D3L2" },
        { 1, "C1 D3R2 left-side wall position", DM1_M553_LEFT_WALL_ORNAMENT_SLOT,
          1, 0, 0, 0, 1, "DEFS.H:2697 C01; D3L/D3R direct gate excludes D3R2" },
        { 2, "C2 M575 D3L right wall ornament", DM1_M551_RIGHT_WALL_ORNAMENT_SLOT,
          DM1_M575_VIEW_WALL_D3L_RIGHT, 1, DM1_V1_D3L_D3R_F0107_SIDE_D3L_PC34,
          0, 1, "DUNVIEW.C:6432; DEFS.H:2698 M575" },
        { 3, "C3 M576 D3R left wall ornament", DM1_M553_LEFT_WALL_ORNAMENT_SLOT,
          DM1_M576_VIEW_WALL_D3R_LEFT, 1, DM1_V1_D3L_D3R_F0107_SIDE_D3R_PC34,
          0, 1, "DUNVIEW.C:6568; DEFS.H:2699 M576" },
        { 4, "C4 M577 D3L front wall ornament", DM1_M552_FRONT_WALL_ORNAMENT_SLOT,
          DM1_M577_VIEW_WALL_D3L_FRONT, 1, DM1_V1_D3L_D3R_F0107_SIDE_D3L_PC34,
          1, 1, "DUNVIEW.C:6433; DEFS.H:2700 M577" },
        { 5, "C5 M578 D3C front wall position", DM1_M552_FRONT_WALL_ORNAMENT_SLOT,
          DM1_M578_VIEW_WALL_D3C_FRONT, 0, 0, 1, 1,
          "DEFS.H:2701 M578; D3C sibling owns this position" },
        { 6, "C6 M579 D3R front wall ornament", DM1_M552_FRONT_WALL_ORNAMENT_SLOT,
          DM1_M579_VIEW_WALL_D3R_FRONT, 1, DM1_V1_D3L_D3R_F0107_SIDE_D3R_PC34,
          1, 1, "DUNVIEW.C:6569; DEFS.H:2702 M579" }
    };

    memcpy(m->ordinals, flows, sizeof(flows));
}

static void fill_steps(DM1_V1_D3LD3RF0107WallOrnamentModelPc34 *m)
{
    static const DM1_V1_D3LD3RF0107StepPc34 steps[] = {
        { DM1_V1_D3L_D3R_F0107_STEP_F0128_DISPATCH_D3L_PC34, 0, 1,
          "F0128 updates and draws D3L", "DUNVIEW.C:8490-8491" },
        { DM1_V1_D3L_D3R_F0107_STEP_F0128_DISPATCH_D3R_PC34, 1, 1,
          "F0128 updates and draws D3R after D3L", "DUNVIEW.C:8494-8495" },
        { DM1_V1_D3L_D3R_F0107_STEP_F0116_D3L_WALL_BODY_PC34, 2, 1,
          "F0116 D3L wall body", "DUNVIEW.C F0116:6361-6498" },
        { DM1_V1_D3L_D3R_F0107_STEP_F0117_D3R_WALL_BODY_PC34, 3, 1,
          "F0117 D3R wall body", "DUNVIEW.C F0117:6500-6640" },
        { DM1_V1_D3L_D3R_F0107_STEP_D3L_SIDE_F0107_PC34, 4, 1,
          "D3L side ornament M551/M575", "DUNVIEW.C:6432" },
        { DM1_V1_D3L_D3R_F0107_STEP_D3L_FRONT_F0107_PC34, 5, 1,
          "D3L front ornament M552/M577", "DUNVIEW.C:6433" },
        { DM1_V1_D3L_D3R_F0107_STEP_D3R_SIDE_F0107_PC34, 6, 1,
          "D3R side ornament M553/M576", "DUNVIEW.C:6568" },
        { DM1_V1_D3L_D3R_F0107_STEP_D3R_FRONT_F0107_PC34, 7, 1,
          "D3R front ornament M552/M579", "DUNVIEW.C:6569" },
        { DM1_V1_D3L_D3R_F0107_STEP_F0108_BASELINE_PC34, 8, 1,
          "F0108 floor+ceiling baseline before F0115 on open paths",
          "DUNVIEW.C F0108:3940-4011; D3L 6478; D3R 6620" },
        { DM1_V1_D3L_D3R_F0107_STEP_F0112_BEFORE_F0115_PC34, 9, 0,
          "F0112 ceiling-pit is absent on D3L/D3R open paths",
          "DUNVIEW.C F0112:4341-4425; no call in F0116/F0117" },
        { DM1_V1_D3L_D3R_F0107_STEP_F0113_AFTER_F0115_PC34, 10, 1,
          "F0113 teleporter fields run after F0115",
          "DUNVIEW.C:6480 before 6495; 6622 before 6637" },
        { DM1_V1_D3L_D3R_F0107_STEP_TERMINAL_DEPTH_SIDE_PAIR_PC34, 11, 1,
          "D3L/D3R are the terminal-depth side-lane F0107 pair",
          "DUNVIEW.C:8491/8495; D2 pair at 8513/8517 is shallower" }
    };

    memcpy(m->steps, steps, sizeof(steps));
}

static void fill_pixels(DM1_V1_D3LD3RF0107WallOrnamentModelPc34 *m)
{
    static const int ordinals[] = { 2, 2, 3, 3, 4, 4, 6, 6 };
    static const uint8_t before[] = { 0x20u, 0x21u, 0x30u, 0x31u, 0x40u, 0x41u, 0x60u, 0x61u };
    static const uint8_t source[] = { 10u, 0x71u, 0x72u, 10u, 0x73u, 10u, 10u, 0x74u };
    size_t i;

    for (i = 0; i < DM1_V1_D3L_D3R_F0107_WALL_ORNAMENT_PIXEL_COUNT_PC34; ++i) {
        m->pixels[i].ordinal_position = ordinals[i];
        m->pixels[i].before = before[i];
        m->pixels[i].source = source[i];
        m->pixels[i].after =
            dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_blend_pixel_pc34(
                before[i], source[i], DM1_V1_D3L_D3R_F0107_C10_COLOR_FLESH_PC34);
        m->pixels[i].transparent_skip =
            source[i] == DM1_V1_D3L_D3R_F0107_C10_COLOR_FLESH_PC34;
        m->pixels[i].writes_pixel =
            source[i] != DM1_V1_D3L_D3R_F0107_C10_COLOR_FLESH_PC34;
        m->pixels[i].anchor = "DUNVIEW.C F0107:3922 C10_COLOR_FLESH transparent blit";
    }
}

bool dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_default_model_builder_pc34(
    DM1_V1_D3LD3RF0107WallOrnamentModelPc34 *out_model)
{
    if (!out_model) return false;
    memset(out_model, 0, sizeof(*out_model));

    out_model->view_square_d3l = DM1_D3L_VIEW_SQUARE;
    out_model->view_square_d3r = DM1_D3R_VIEW_SQUARE;
    out_model->wall_zone_d3l = DM1_D3L_WALL_ZONE;
    out_model->wall_zone_d3r = DM1_D3R_WALL_ZONE;
    out_model->floor_view_d3l = DM1_D3L_FLOOR_VIEW;
    out_model->floor_view_d3r = DM1_D3R_FLOOR_VIEW;
    out_model->field_aspect_d3l = DM1_D3L_FIELD_ASPECT;
    out_model->field_aspect_d3r = DM1_D3R_FIELD_ASPECT;
    out_model->c10_transparent_color = DM1_V1_D3L_D3R_F0107_C10_COLOR_FLESH_PC34;
    out_model->c1004_wall_ornament_zone_base = DM1_C1004_ZONE_WALL_ORNAMENT;
    out_model->wall_ornament_zone_stride = DM1_WALL_ORNAMENT_ZONE_STRIDE;
    out_model->c1500_floor_ornament_zone_base = DM1_C1500_ZONE_FLOOR_ORNAMENT;
    out_model->g0205_wall_ornament_coordinate_sets = 1;
    out_model->g0206_floor_ornament_coordinate_sets = 1;
    out_model->g0207_door_ornament_coordinate_sets = 1;
    out_model->g0208_door_button_coordinate_sets = 1;
    out_model->f0128_d3l_then_d3r = 1;
    out_model->f0128_d3_pair_before_d3c = 1;
    out_model->f0128_d3_pair_before_d2_pair = 1;
    out_model->spatially_deeper_than_d2_pair = 1;
    out_model->direct_f0107_call_count = 4;
    out_model->sensor_position_count = 4;
    out_model->side_ornament_call_count = 2;
    out_model->front_ornament_call_count = 2;
    out_model->f0107_zero_ordinal_returns_false =
        dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_returns_alcove_pc34(0, true) ? 0 : 1;
    out_model->f0107_non_alcove_returns_false =
        dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_returns_alcove_pc34(3, false) ? 0 : 1;
    out_model->f0107_alcove_returns_true =
        dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_returns_alcove_pc34(3, true) ? 1 : 0;
    out_model->f0107_blit_uses_c10 = 1;
    out_model->c10_transparent_preserves_destination =
        dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_blend_pixel_pc34(
            0x9au, DM1_V1_D3L_D3R_F0107_C10_COLOR_FLESH_PC34,
            DM1_V1_D3L_D3R_F0107_C10_COLOR_FLESH_PC34) == 0x9au;
    out_model->f0108_floor_baseline_before_f0115 = 1;
    out_model->f0112_ceiling_pit_before_f0115 = 0;
    out_model->f0113_teleporter_field_after_f0115 = 1;
    out_model->d3l_cell_order_terminal_depth = DM1_D3L_CORRIDOR_ORDER;
    out_model->d3r_cell_order_terminal_depth = DM1_D3R_CORRIDOR_ORDER;
    out_model->no_graphics_dat_reads = 1;
    out_model->source_locked_contract_only = 1;
    out_model->no_original_dos_pixel_parity = 1;
    out_model->source_evidence = s_source_evidence;
    out_model->disjointness_note = s_disjointness_note;

    fill_lanes(out_model);
    fill_ordinals(out_model);
    fill_steps(out_model);
    fill_pixels(out_model);
    out_model->deterministic_hash =
        dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_hash_model_pc34(out_model);
    return true;
}

uint32_t dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_hash_model_pc34(
    const DM1_V1_D3LD3RF0107WallOrnamentModelPc34 *model)
{
    uint32_t h = 2166136261u;
    size_t i;

    if (!model) return 0u;
    h = fnv1a_u32(h, (uint32_t)model->view_square_d3l);
    h = fnv1a_u32(h, (uint32_t)model->view_square_d3r);
    h = fnv1a_u32(h, (uint32_t)model->wall_zone_d3l);
    h = fnv1a_u32(h, (uint32_t)model->wall_zone_d3r);
    h = fnv1a_u32(h, (uint32_t)model->floor_view_d3l);
    h = fnv1a_u32(h, (uint32_t)model->floor_view_d3r);
    h = fnv1a_u32(h, (uint32_t)model->field_aspect_d3l);
    h = fnv1a_u32(h, (uint32_t)model->field_aspect_d3r);
    h = fnv1a_u32(h, (uint32_t)model->direct_f0107_call_count);
    h = fnv1a_u32(h, (uint32_t)model->sensor_position_count);
    h = fnv1a_u32(h, (uint32_t)model->f0128_d3l_then_d3r);
    h = fnv1a_u32(h, (uint32_t)model->f0128_d3_pair_before_d3c);
    h = fnv1a_u32(h, (uint32_t)model->f0128_d3_pair_before_d2_pair);
    h = fnv1a_u32(h, (uint32_t)model->spatially_deeper_than_d2_pair);
    h = fnv1a_u32(h, (uint32_t)model->f0107_alcove_returns_true);
    h = fnv1a_u32(h, (uint32_t)model->f0113_teleporter_field_after_f0115);
    for (i = 0; i < DM1_V1_D3L_D3R_F0107_WALL_ORNAMENT_LANE_COUNT_PC34; ++i) {
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].side);
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].view_square);
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].wall_zone);
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].side_view_wall);
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].front_view_wall);
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].corridor_order);
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].door_pass1_order);
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].door_pass2_order);
    }
    for (i = 0; i < DM1_V1_D3L_D3R_F0107_WALL_ORNAMENT_FLOW_COUNT_PC34; ++i) {
        h = fnv1a_u32(h, (uint32_t)model->ordinals[i].ordinal_position);
        h = fnv1a_u32(h, (uint32_t)model->ordinals[i].aspect_slot);
        h = fnv1a_u32(h, (uint32_t)model->ordinals[i].view_wall);
        h = fnv1a_u32(h, (uint32_t)model->ordinals[i].reaches_d3l_d3r_f0107);
    }
    for (i = 0; i < DM1_V1_D3L_D3R_F0107_WALL_ORNAMENT_STEP_COUNT_PC34; ++i) {
        h = fnv1a_u32(h, (uint32_t)model->steps[i].step);
        h = fnv1a_u32(h, (uint32_t)model->steps[i].expected_present);
    }
    for (i = 0; i < DM1_V1_D3L_D3R_F0107_WALL_ORNAMENT_PIXEL_COUNT_PC34; ++i) {
        h = fnv1a_u32(h, (uint32_t)model->pixels[i].ordinal_position);
        h = fnv1a_u32(h, (uint32_t)model->pixels[i].before);
        h = fnv1a_u32(h, (uint32_t)model->pixels[i].source);
        h = fnv1a_u32(h, (uint32_t)model->pixels[i].after);
    }
    return h;
}

const DM1_V1_D3LD3RF0107WallOrnamentModelPc34 *
dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_default_model_pc34(void)
{
    static DM1_V1_D3LD3RF0107WallOrnamentModelPc34 s_model;
    static int s_init;

    if (!s_init) {
        dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_default_model_builder_pc34(
            &s_model);
        s_init = 1;
    }
    return &s_model;
}

uint32_t dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_deterministic_hash_pc34(void)
{
    const DM1_V1_D3LD3RF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_default_model_pc34();
    return model ? model->deterministic_hash : 0u;
}

const DM1_V1_D3LD3RF0107LanePc34 *
dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_lane_at_pc34(size_t index)
{
    const DM1_V1_D3LD3RF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_default_model_pc34();
    if (!model || index >= DM1_V1_D3L_D3R_F0107_WALL_ORNAMENT_LANE_COUNT_PC34) {
        return NULL;
    }
    return &model->lanes[index];
}

const DM1_V1_D3LD3RF0107StepPc34 *
dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_step_at_pc34(size_t index)
{
    const DM1_V1_D3LD3RF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_default_model_pc34();
    if (!model || index >= DM1_V1_D3L_D3R_F0107_WALL_ORNAMENT_STEP_COUNT_PC34) {
        return NULL;
    }
    return &model->steps[index];
}

const DM1_V1_D3LD3RF0107OrdinalFlowPc34 *
dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_ordinal_at_pc34(size_t index)
{
    const DM1_V1_D3LD3RF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_default_model_pc34();
    if (!model || index >= DM1_V1_D3L_D3R_F0107_WALL_ORNAMENT_FLOW_COUNT_PC34) {
        return NULL;
    }
    return &model->ordinals[index];
}

const char *dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const char *dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_disjointness_note_pc34(void)
{
    return s_disjointness_note;
}
