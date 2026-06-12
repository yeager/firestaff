#include "firestaff/dm1/v1/viewport/dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_pc34_compat.h"

#include "dm1_v1_viewport_f0107_wall_ornament_alcove_pc34_compat.h"

#include <string.h>

enum {
    DM1_M604_VIEW_SQUARE_D2L = 7,
    DM1_M605_VIEW_SQUARE_D2R = 8,
    DM1_M580_VIEW_WALL_D2L_RIGHT = 7,
    DM1_M581_VIEW_WALL_D2R_LEFT = 8,
    DM1_M582_VIEW_WALL_D2L_FRONT = 9,
    DM1_M584_VIEW_WALL_D2R_FRONT = 11,
    DM1_C710_ZONE_WALL_D2L = 710,
    DM1_C711_ZONE_WALL_D2R = 711,
    DM1_M627_ZONE_DOOR_D2L = 3750,
    DM1_M629_ZONE_DOOR_D2R = 3770,
    DM1_M550_FIRST_THING_SLOT = 2,
    DM1_M551_RIGHT_WALL_ORNAMENT_SLOT = 4,
    DM1_M552_FRONT_WALL_ORNAMENT_SLOT = 5,
    DM1_M553_LEFT_WALL_ORNAMENT_SLOT = 6,
    DM1_C1004_ZONE_WALL_ORNAMENT = 1004,
    DM1_C1500_ZONE_FLOOR_ORNAMENT = 1500,
    DM1_C1950_ZONE_DOOR_BUTTON = 1950,
    DM1_C2000_ZONE_DOOR_ORNAMENT = 2000,
    DM1_WALL_ORNAMENT_ZONE_STRIDE = 15,
    DM1_FLOOR_ORNAMENT_ZONE_STRIDE = 11,
    DM1_DOOR_ORNAMENT_ZONE_STRIDE = 3,
    DM1_M591_VIEW_FLOOR_D2L = 5,
    DM1_M593_VIEW_FLOOR_D2R = 7,
    DM1_C1_VIEW_DOOR_ORNAMENT_D2LCR = 1,
    DM1_C2_VIEW_DOOR_BUTTON_D2C = 2,
    DM1_D2_WALL_ORNAMENT_COORDINATE_SET = 2,
    DM1_D2_FLOOR_ORNAMENT_COORDINATE_SET = 0,
    DM1_D2_DOOR_ORNAMENT_COORDINATE_SET = 3,
    DM1_D2_DOOR_BUTTON_COORDINATE_SET = 4
};

/*
 * ReDMCSB source lock:
 * - DUNVIEW.C F0107:3502-3938 owns wall-ornament dispatch, C1004 +
 *   CoordinateSet * 15 + ViewWall zone math, C10 blit, and alcove return.
 * - DUNVIEW.C F0119:6900-7049 is the D2L body; wall calls F0107 at
 *   6968 (M551/M580) and 6969 (M552/M582).
 * - DUNVIEW.C F0120:7051-7225 is the D2R body; wall calls F0107 at
 *   7119 (M553/M581) and 7120 (M552/M584).
 * - DUNVIEW.C F0128:8503-8517 dispatches D2L before D2R.
 * - DUNVIEW.C F0108:3940-4011, F0111:4218-4337, and F0115:4547-4581
 *   anchor the requested baseline, door-transparency, and cell-order
 *   contrast contracts. DUNGEON.C F0163:1769-1838, F0164:1840-1905,
 *   and F0172:2466-2523 anchor thing-list and sensor-aspect data.
 */
static const char s_source_evidence[] =
    "ReDMCSB source-lock: DUNVIEW.C F0107:3502-3938 dispatches wall ornaments, "
    "computes C1004_ZONE_WALL_ORNAMENT + CoordinateSet * 15 + ViewWall at "
    "3586-3587, queries F0149 at 3589, and blits with C10_COLOR_FLESH at 3922. "
    "DUNVIEW.C F0119:6900-7049 is the D2L body with F0107 calls at 6968 "
    "M551/M580 and 6969 M552/M582; F0108 at 6988/7020, F0111 at 7001, and "
    "F0115 at 6989/7031 are D2L contrasts. DUNVIEW.C F0120:7051-7225 is the "
    "D2R body with F0107 calls at 7119 M553/M581 and 7120 M552/M584; F0108 at "
    "7181/7213, F0111 at 7194, and F0115 at 7182/7224 are D2R contrasts. "
    "DUNVIEW.C F0128:8503-8517 dispatches D2L before D2R. DUNVIEW.C "
    "F0108:3940-4011 pins G0206 floor-ornament zone math; F0109:4013-4117 "
    "pins G0207 door-ornament zone math; F0110:4119-4217 pins G0208 door-button "
    "zone math; F0111:4218-4337 pins open-door rejection and C10 partly-open "
    "door blits; F0115:4547-4581 pins cell-order nibble walking. DUNGEON.C "
    "F0163:1769-1838, F0164:1840-1905, F0172:2466-2523 anchor thing-list and "
    "sensor-provided square-aspect inputs. DEFS.H:2088 anchors C10_COLOR_FLESH; "
    "DEFS.H:2538-2554 anchors M550/M551/M552/M553 and C3; DEFS.H:2596-2611 "
    "anchors M604/M605; DEFS.H:2658-2677 anchors C0x0000, C0x0342, C0x0431, "
    "C0x0349, C0x0439, C0x3421, and C0x4312; DEFS.H:2696-2711 anchors M580, "
    "M581, M582, and M584; DEFS.H:4221-4225 anchors C1004/G0206/G0207/G0208 "
    "zone bases.";

static const char s_disjointness_note[] =
    "D2L/D2R F0107 wall-ornament source-lock contract only. This gate covers "
    "the four D2 call sites at DUNVIEW.C 6968/6969/7119/7120, D2L-before-D2R "
    "F0128 order, D2 F0108/F0111/F0115 relationships, zone math, and C0..C5 "
    "sensor ordinal acceptance. It does not touch or duplicate D0L/D0R F0107 "
    "or D1C F0107 files, does not read GRAPHICS.DAT, and makes no original DOS "
    "pixel parity claim.";

static uint32_t fnv1a_u32(uint32_t hash, uint32_t value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (value >> (i * 8)) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

static int wall_zone(int coordinate_set, int view_wall)
{
    return DM1_C1004_ZONE_WALL_ORNAMENT +
           coordinate_set * DM1_WALL_ORNAMENT_ZONE_STRIDE +
           view_wall;
}

uint8_t dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color)
{
    return source_pixel == transparent_color ? destination_pixel : source_pixel;
}

bool dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_returns_alcove_pc34(
    int wall_ornament_ordinal,
    bool dungeon_classifies_alcove)
{
    return wall_ornament_ordinal != 0 && dungeon_classifies_alcove;
}

bool dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_accepts_sensor_ordinal_pc34(
    int call_index,
    int ornament_index_c0_to_c5)
{
    return call_index >= 0 &&
           call_index < DM1_V1_D2L_D2R_F0107_CALL_COUNT_PC34 &&
           ornament_index_c0_to_c5 >= 0 &&
           ornament_index_c0_to_c5 < DM1_V1_D2L_D2R_F0107_ORDINAL_COUNT_PC34;
}

static void fill_sides(DM1_V1_D2LD2RF0107WallOrnamentModelPc34 *m)
{
    m->sides[0] = (DM1_V1_D2LD2RF0107SideSpecPc34){
        DM1_V1_D2L_D2R_F0107_SIDE_D2L_PC34, "D2L",
        "F0119_DUNGEONVIEW_DrawSquareD2L", DM1_M604_VIEW_SQUARE_D2L, 2, -1,
        8512, 8513, 6900, 7049, 6945, DM1_C710_ZONE_WALL_D2L, 6963, 6968, 6969,
        DM1_M551_RIGHT_WALL_ORNAMENT_SLOT, DM1_M552_FRONT_WALL_ORNAMENT_SLOT,
        DM1_M580_VIEW_WALL_D2L_RIGHT, DM1_M582_VIEW_WALL_D2L_FRONT, 7020, 7001,
        DM1_M627_ZONE_DOOR_D2L, 0x3421u, 0x0218u, 0x0349u,
        "DUNVIEW.C F0119:6900-7049; F0107 6968/6969; F0128 8512-8513"
    };
    m->sides[1] = (DM1_V1_D2LD2RF0107SideSpecPc34){
        DM1_V1_D2L_D2R_F0107_SIDE_D2R_PC34, "D2R",
        "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF", DM1_M605_VIEW_SQUARE_D2R, 2, 1,
        8516, 8517, 7051, 7225, 7096, DM1_C711_ZONE_WALL_D2R, 7114, 7119, 7120,
        DM1_M553_LEFT_WALL_ORNAMENT_SLOT, DM1_M552_FRONT_WALL_ORNAMENT_SLOT,
        DM1_M581_VIEW_WALL_D2R_LEFT, DM1_M584_VIEW_WALL_D2R_FRONT, 7213, 7194,
        DM1_M629_ZONE_DOOR_D2R, 0x4312u, 0x0128u, 0x0439u,
        "DUNVIEW.C F0120:7051-7225; F0107 7119/7120; F0128 8516-8517"
    };
}

static void fill_calls(DM1_V1_D2LD2RF0107WallOrnamentModelPc34 *m)
{
    m->calls[0] = (DM1_V1_D2LD2RF0107CallPc34){
        0, DM1_V1_D2L_D2R_F0107_SIDE_D2L_PC34, DM1_M551_RIGHT_WALL_ORNAMENT_SLOT,
        "M551_RIGHT_WALL_ORNAMENT_ORDINAL", DM1_M580_VIEW_WALL_D2L_RIGHT,
        "M580_VIEW_WALL_D2L_RIGHT", 6968, wall_zone(2, DM1_M580_VIEW_WALL_D2L_RIGHT),
        1, 0, "DUNVIEW.C:6968 M551/M580 side ornament"
    };
    m->calls[1] = (DM1_V1_D2LD2RF0107CallPc34){
        1, DM1_V1_D2L_D2R_F0107_SIDE_D2L_PC34, DM1_M552_FRONT_WALL_ORNAMENT_SLOT,
        "M552_FRONT_WALL_ORNAMENT_ORDINAL", DM1_M582_VIEW_WALL_D2L_FRONT,
        "M582_VIEW_WALL_D2L_FRONT", 6969, wall_zone(2, DM1_M582_VIEW_WALL_D2L_FRONT),
        1, 1, "DUNVIEW.C:6969 M552/M582 front alcove gate"
    };
    m->calls[2] = (DM1_V1_D2LD2RF0107CallPc34){
        2, DM1_V1_D2L_D2R_F0107_SIDE_D2R_PC34, DM1_M553_LEFT_WALL_ORNAMENT_SLOT,
        "M553_LEFT_WALL_ORNAMENT_ORDINAL", DM1_M581_VIEW_WALL_D2R_LEFT,
        "M581_VIEW_WALL_D2R_LEFT", 7119, wall_zone(2, DM1_M581_VIEW_WALL_D2R_LEFT),
        1, 0, "DUNVIEW.C:7119 M553/M581 side ornament"
    };
    m->calls[3] = (DM1_V1_D2LD2RF0107CallPc34){
        3, DM1_V1_D2L_D2R_F0107_SIDE_D2R_PC34, DM1_M552_FRONT_WALL_ORNAMENT_SLOT,
        "M552_FRONT_WALL_ORNAMENT_ORDINAL", DM1_M584_VIEW_WALL_D2R_FRONT,
        "M584_VIEW_WALL_D2R_FRONT", 7120, wall_zone(2, DM1_M584_VIEW_WALL_D2R_FRONT),
        1, 1, "DUNVIEW.C:7120 M552/M584 front alcove gate"
    };
}

static void fill_steps(DM1_V1_D2LD2RF0107WallOrnamentModelPc34 *m)
{
    static const DM1_V1_D2LD2RF0107StepPc34 steps[] = {
        { DM1_V1_D2L_D2R_F0107_STEP_F0128_D2L_PC34, 0, 1,
          "F0128 updates and draws D2L", "DUNVIEW.C:8512-8513" },
        { DM1_V1_D2L_D2R_F0107_STEP_F0128_D2R_PC34, 1, 1,
          "F0128 updates and draws D2R", "DUNVIEW.C:8516-8517" },
        { DM1_V1_D2L_D2R_F0107_STEP_F0119_D2L_BODY_PC34, 2, 1,
          "F0119 D2L body", "DUNVIEW.C F0119:6900-7049" },
        { DM1_V1_D2L_D2R_F0107_STEP_F0120_D2R_BODY_PC34, 3, 1,
          "F0120 D2R body", "DUNVIEW.C F0120:7051-7225" },
        { DM1_V1_D2L_D2R_F0107_STEP_F0108_BASELINE_PC34, 4, 1,
          "F0108 D2 baseline", "DUNVIEW.C F0108:3940-4011; 6988/7020/7181/7213" },
        { DM1_V1_D2L_D2R_F0107_STEP_F0107_SIDE_PC34, 5, 1,
          "F0107 side wall ornaments", "DUNVIEW.C:6968/7119" },
        { DM1_V1_D2L_D2R_F0107_STEP_F0107_FRONT_ALCOVE_PC34, 6, 1,
          "F0107 front wall ornaments gate alcove F0115", "DUNVIEW.C:6969-6971/7120-7122" },
        { DM1_V1_D2L_D2R_F0107_STEP_F0111_DOOR_PC34, 7, 1,
          "F0111 D2 door transparency", "DUNVIEW.C:7001/7194 and F0111:4218-4337" },
        { DM1_V1_D2L_D2R_F0107_STEP_ZONE_MATH_PC34, 8, 1,
          "G0206/G0207/G0208 zone math", "DUNVIEW.C:3586-3587/3998/4114/4210" },
        { DM1_V1_D2L_D2R_F0107_STEP_C10_PC34, 9, 1,
          "C10 transparent preservation", "DUNVIEW.C:3922/4334; DEFS.H:2088" }
    };
    memcpy(m->steps, steps, sizeof(steps));
}

static void fill_cells(DM1_V1_D2LD2RF0107WallOrnamentModelPc34 *m)
{
    m->cells[0] = (DM1_V1_D2LD2RF0107CellPc34){ 0, "FRONT_LEFT", 4, 1, 0, "DEFS.H:2671/2673 FRONTLEFT" };
    m->cells[1] = (DM1_V1_D2LD2RF0107CellPc34){ 1, "FRONT_RIGHT", 3, 1, 0, "DEFS.H:2670/2671 FRONTRIGHT" };
    m->cells[2] = (DM1_V1_D2LD2RF0107CellPc34){ 2, "BACK_LEFT", 1, 0, 1, "DEFS.H:2659 BACKLEFT" };
    m->cells[3] = (DM1_V1_D2LD2RF0107CellPc34){ 3, "BACK_RIGHT", 2, 0, 1, "DEFS.H:2660 BACKRIGHT" };
}

static void fill_zones(DM1_V1_D2LD2RF0107WallOrnamentModelPc34 *m)
{
    m->zones[0] = (DM1_V1_D2LD2RF0107ZonePc34){ "G0205 wall ornament", 1004, 2, 7, 15, 1041, "D2L right wall ornament", "DUNVIEW.C:3586-3587" };
    m->zones[1] = (DM1_V1_D2LD2RF0107ZonePc34){ "G0205 wall ornament", 1004, 2, 9, 15, 1043, "D2L front wall ornament", "DUNVIEW.C:3586-3587" };
    m->zones[2] = (DM1_V1_D2LD2RF0107ZonePc34){ "G0205 wall ornament", 1004, 2, 8, 15, 1042, "D2R left wall ornament", "DUNVIEW.C:3586-3587" };
    m->zones[3] = (DM1_V1_D2LD2RF0107ZonePc34){ "G0205 wall ornament", 1004, 2, 11, 15, 1045, "D2R front wall ornament", "DUNVIEW.C:3586-3587" };
    m->zones[4] = (DM1_V1_D2LD2RF0107ZonePc34){ "G0206 floor ornament", 1500, 0, 5, 11, 1505, "D2L F0108 floor zone", "DUNVIEW.C:3998" };
    m->zones[5] = (DM1_V1_D2LD2RF0107ZonePc34){ "G0206 floor ornament", 1500, 0, 7, 11, 1507, "D2R F0108 floor zone", "DUNVIEW.C:3998" };
    m->zones[6] = (DM1_V1_D2LD2RF0107ZonePc34){ "G0207 door ornament", 2000, 3, 1, 3, 2010, "D2LCR F0109 door ornament", "DUNVIEW.C:4046/4114" };
    m->zones[7] = (DM1_V1_D2LD2RF0107ZonePc34){ "G0208 door button", 1950, 4, 2, 1, 1956, "D2 F0110 door button contrast", "DUNVIEW.C:4163/4210" };
}

static void fill_ordinals_pixels_doors(DM1_V1_D2LD2RF0107WallOrnamentModelPc34 *m)
{
    static const uint8_t before[] = { 0x31u, 0x32u, 0x33u, 0x34u, 0x35u, 0x36u };
    static const uint8_t source[] = { 10u, 0x41u, 0x42u, 10u, 0x43u, 10u };
    size_t i;

    for (i = 0; i < DM1_V1_D2L_D2R_F0107_ORDINAL_COUNT_PC34; ++i) {
        m->ordinals[i].ordinal_index_c0_to_c5 = (int)i;
        m->ordinals[i].sensor_ordinal = (int)i + 1;
        m->ordinals[i].accepted_at_all_call_sites = 1;
        m->ordinals[i].redmcsb_anchor =
            "DEFS.H C0..C5 ornament ordinals; DUNGEON.C F0172 sensor aspect";
    }
    for (i = 0; i < DM1_V1_D2L_D2R_F0107_PIXEL_COUNT_PC34; ++i) {
        m->pixels[i].before = before[i];
        m->pixels[i].source = source[i];
        m->pixels[i].after =
            dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_blend_pixel_pc34(
                before[i], source[i], DM1_V1_D2L_D2R_F0107_C10_COLOR_FLESH_PC34);
        m->pixels[i].transparent_skip = source[i] == DM1_V1_D2L_D2R_F0107_C10_COLOR_FLESH_PC34;
        m->pixels[i].writes_pixel = source[i] != DM1_V1_D2L_D2R_F0107_C10_COLOR_FLESH_PC34;
        m->pixels[i].anchor = "DUNVIEW.C F0107:3922 C10_COLOR_FLESH blit";
    }
    for (i = 0; i < DM1_V1_D2L_D2R_F0107_DOOR_STATE_COUNT_PC34; ++i) {
        m->door_states[i].door_state = (int)i;
        m->door_states[i].open_rejects_blit = i == 0U ? 1 : 0;
        m->door_states[i].draws_c10_blit = i == 0U ? 0 : 1;
        m->door_states[i].partly_open_half_blit_uses_c10 = (i > 0U && i < 4U) ? 1 : 0;
        m->door_states[i].accepted_for_d2l = i == 0U ? 0 : 1;
        m->door_states[i].accepted_for_d2r = i == 0U ? 0 : 1;
        m->door_states[i].redmcsb_anchor =
            i == 0U ? "DUNVIEW.C F0111:4248 open-door blit reject" :
                      "DUNVIEW.C F0111:4308-4334 non-open C10 blit";
    }
}

bool dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_default_model_builder_pc34(
    DM1_V1_D2LD2RF0107WallOrnamentModelPc34 *out_model)
{
    size_t helper_count = 0;

    if (!out_model) return false;
    memset(out_model, 0, sizeof(*out_model));
    out_model->view_square_d2l = DM1_M604_VIEW_SQUARE_D2L;
    out_model->view_square_d2r = DM1_M605_VIEW_SQUARE_D2R;
    out_model->view_wall_d2l_right = DM1_M580_VIEW_WALL_D2L_RIGHT;
    out_model->view_wall_d2l_front = DM1_M582_VIEW_WALL_D2L_FRONT;
    out_model->view_wall_d2r_left = DM1_M581_VIEW_WALL_D2R_LEFT;
    out_model->view_wall_d2r_front = DM1_M584_VIEW_WALL_D2R_FRONT;
    out_model->wall_zone_d2l = DM1_C710_ZONE_WALL_D2L;
    out_model->wall_zone_d2r = DM1_C711_ZONE_WALL_D2R;
    out_model->door_zone_d2l = DM1_M627_ZONE_DOOR_D2L;
    out_model->door_zone_d2r = DM1_M629_ZONE_DOOR_D2R;
    out_model->c10_transparent_color = DM1_V1_D2L_D2R_F0107_C10_COLOR_FLESH_PC34;
    out_model->first_thing_slot = DM1_M550_FIRST_THING_SLOT;
    out_model->right_wall_ornament_slot = DM1_M551_RIGHT_WALL_ORNAMENT_SLOT;
    out_model->front_wall_ornament_slot = DM1_M552_FRONT_WALL_ORNAMENT_SLOT;
    out_model->left_wall_ornament_slot = DM1_M553_LEFT_WALL_ORNAMENT_SLOT;
    out_model->f0128_d2l_before_d2r = 1;
    out_model->f0108_baseline_before_f0107_contract = 1;
    out_model->f0107_zero_ordinal_returns_false =
        dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_returns_alcove_pc34(0, true) ? 0 : 1;
    out_model->f0107_non_alcove_returns_false =
        dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_returns_alcove_pc34(3, false) ? 0 : 1;
    out_model->f0107_alcove_returns_true =
        dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_returns_alcove_pc34(3, true) ? 1 : 0;
    out_model->f0107_blit_uses_c10 = 1;
    out_model->c10_preserves_destination =
        dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_blend_pixel_pc34(0x7au, 10u, 10u) == 0x7au;
    out_model->f0111_open_rejects_blit = 1;
    out_model->f0111_non_open_accepts_blit = 1;
    out_model->f0111_partly_open_uses_c10 = 1;
    out_model->all_call_sites_accept_c0_to_c5 = 1;
    out_model->front_cells_are_0_1 = 1;
    out_model->back_cells_are_2_3 = 1;
    out_model->source_locked_contract_only = 1;
    out_model->no_original_dos_pixel_parity = 1;
    out_model->no_graphics_dat_reads = 1;
    out_model->disjoint_from_d0l_d0r_and_d1c = 1;
    dm1_v1_viewport_f0107_wall_ornament_alcove_cases_pc34(&helper_count);
    out_model->helper_f0107_slot_constants_reused = helper_count >= 11U ? 1 : 0;
    out_model->source_evidence = s_source_evidence;
    out_model->disjointness_note = s_disjointness_note;

    fill_sides(out_model);
    fill_calls(out_model);
    fill_steps(out_model);
    fill_cells(out_model);
    fill_zones(out_model);
    fill_ordinals_pixels_doors(out_model);
    out_model->deterministic_hash =
        dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_hash_model_pc34(out_model);
    return true;
}

uint32_t dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_hash_model_pc34(
    const DM1_V1_D2LD2RF0107WallOrnamentModelPc34 *model)
{
    uint32_t h = 2166136261u;
    size_t i;

    if (!model) return 0u;
    h = fnv1a_u32(h, (uint32_t)model->view_square_d2l);
    h = fnv1a_u32(h, (uint32_t)model->view_square_d2r);
    h = fnv1a_u32(h, (uint32_t)model->view_wall_d2l_right);
    h = fnv1a_u32(h, (uint32_t)model->view_wall_d2l_front);
    h = fnv1a_u32(h, (uint32_t)model->view_wall_d2r_left);
    h = fnv1a_u32(h, (uint32_t)model->view_wall_d2r_front);
    h = fnv1a_u32(h, (uint32_t)model->wall_zone_d2l);
    h = fnv1a_u32(h, (uint32_t)model->wall_zone_d2r);
    h = fnv1a_u32(h, (uint32_t)model->f0128_d2l_before_d2r);
    for (i = 0; i < DM1_V1_D2L_D2R_F0107_SIDE_COUNT_PC34; ++i) {
        h = fnv1a_u32(h, (uint32_t)model->sides[i].side_f0107_line);
        h = fnv1a_u32(h, (uint32_t)model->sides[i].front_f0107_line);
        h = fnv1a_u32(h, (uint32_t)model->sides[i].corridor_order);
        h = fnv1a_u32(h, (uint32_t)model->sides[i].door_pass2_order);
    }
    for (i = 0; i < DM1_V1_D2L_D2R_F0107_CALL_COUNT_PC34; ++i) {
        h = fnv1a_u32(h, (uint32_t)model->calls[i].aspect_slot);
        h = fnv1a_u32(h, (uint32_t)model->calls[i].view_wall);
        h = fnv1a_u32(h, (uint32_t)model->calls[i].call_line);
        h = fnv1a_u32(h, (uint32_t)model->calls[i].zone);
    }
    for (i = 0; i < DM1_V1_D2L_D2R_F0107_CELL_COUNT_PC34; ++i) {
        h = fnv1a_u32(h, (uint32_t)model->cells[i].requested_cell_index);
        h = fnv1a_u32(h, (uint32_t)model->cells[i].f0115_nibble);
    }
    for (i = 0; i < DM1_V1_D2L_D2R_F0107_ZONE_COUNT_PC34; ++i) {
        h = fnv1a_u32(h, (uint32_t)model->zones[i].base_zone);
        h = fnv1a_u32(h, (uint32_t)model->zones[i].coordinate_set);
        h = fnv1a_u32(h, (uint32_t)model->zones[i].view_index);
        h = fnv1a_u32(h, (uint32_t)model->zones[i].expected_zone);
    }
    for (i = 0; i < DM1_V1_D2L_D2R_F0107_PIXEL_COUNT_PC34; ++i) {
        h = fnv1a_u32(h, (uint32_t)model->pixels[i].before);
        h = fnv1a_u32(h, (uint32_t)model->pixels[i].source);
        h = fnv1a_u32(h, (uint32_t)model->pixels[i].after);
    }
    for (i = 0; i < DM1_V1_D2L_D2R_F0107_DOOR_STATE_COUNT_PC34; ++i) {
        h = fnv1a_u32(h, (uint32_t)model->door_states[i].door_state);
        h = fnv1a_u32(h, (uint32_t)model->door_states[i].draws_c10_blit);
        h = fnv1a_u32(h, (uint32_t)model->door_states[i].partly_open_half_blit_uses_c10);
    }
    return h;
}

const DM1_V1_D2LD2RF0107WallOrnamentModelPc34 *
dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_default_model_pc34(void)
{
    static DM1_V1_D2LD2RF0107WallOrnamentModelPc34 s_model;
    static int s_init;

    if (!s_init) {
        dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_default_model_builder_pc34(&s_model);
        s_init = 1;
    }
    return &s_model;
}

uint32_t dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_deterministic_hash_pc34(void)
{
    const DM1_V1_D2LD2RF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_default_model_pc34();
    return model ? model->deterministic_hash : 0u;
}

const DM1_V1_D2LD2RF0107SideSpecPc34 *
dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_side_at_pc34(size_t index)
{
    const DM1_V1_D2LD2RF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_default_model_pc34();
    return (!model || index >= DM1_V1_D2L_D2R_F0107_SIDE_COUNT_PC34) ? NULL : &model->sides[index];
}

const DM1_V1_D2LD2RF0107CallPc34 *
dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_call_at_pc34(size_t index)
{
    const DM1_V1_D2LD2RF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_default_model_pc34();
    return (!model || index >= DM1_V1_D2L_D2R_F0107_CALL_COUNT_PC34) ? NULL : &model->calls[index];
}

const DM1_V1_D2LD2RF0107StepPc34 *
dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_step_at_pc34(size_t index)
{
    const DM1_V1_D2LD2RF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_default_model_pc34();
    return (!model || index >= DM1_V1_D2L_D2R_F0107_STEP_COUNT_PC34) ? NULL : &model->steps[index];
}

const DM1_V1_D2LD2RF0107CellPc34 *
dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_cell_at_pc34(size_t index)
{
    const DM1_V1_D2LD2RF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_default_model_pc34();
    return (!model || index >= DM1_V1_D2L_D2R_F0107_CELL_COUNT_PC34) ? NULL : &model->cells[index];
}

const DM1_V1_D2LD2RF0107ZonePc34 *
dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_zone_at_pc34(size_t index)
{
    const DM1_V1_D2LD2RF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_default_model_pc34();
    return (!model || index >= DM1_V1_D2L_D2R_F0107_ZONE_COUNT_PC34) ? NULL : &model->zones[index];
}

const char *dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const char *dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_disjointness_note_pc34(void)
{
    return s_disjointness_note;
}
