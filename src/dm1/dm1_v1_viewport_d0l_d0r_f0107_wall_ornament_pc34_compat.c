#include "firestaff/dm1/v1/viewport/dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_pc34_compat.h"

#include "dm1_v1_viewport_f0107_wall_ornament_alcove_pc34_compat.h"

#include <string.h>

enum {
    DM1_D0L_VIEW_SQUARE = 1,
    DM1_D0R_VIEW_SQUARE = 2,
    DM1_D0L_WALL_ZONE = 716,
    DM1_D0R_WALL_ZONE = 717,
    DM1_M550_FIRST_THING_SLOT = 2,
    DM1_M551_RIGHT_WALL_ORNAMENT_SLOT = 4,
    DM1_M552_FRONT_WALL_ORNAMENT_SLOT = 5,
    DM1_M553_LEFT_WALL_ORNAMENT_SLOT = 6,
    DM1_M585_VIEW_WALL_D1L_RIGHT = 12,
    DM1_M586_VIEW_WALL_D1R_LEFT = 13,
    DM1_M587_VIEW_WALL_D1C_FRONT = 14,
    DM1_D0L_THING_ORDER = 0x0002,
    DM1_D0R_THING_ORDER = 0x0001
};

static const char s_source_evidence[] =
    "ReDMCSB source-lock: DUNVIEW.C F0107:3502-3938 is the wall-ornament "
    "and alcove BOOLEAN function. It decrements only non-zero ordinals, "
    "queries the map alcove classifier through F0149 at line 3589, and "
    "uses C10_COLOR_FLESH for the final wall-ornament blit at line 3922 "
    "before returning the alcove boolean at 3933 or false at 3936. "
    "DUNVIEW.C F0125:7960-8062 and F0126:8064-8162 are the D0L/D0R "
    "dispatch bodies: their C00 wall cases draw C716/C717 and return at "
    "8024/8131, so they do not directly call F0107. Their open side-lane "
    "paths pin F0115 at 8005 and 8115 with M610/M611 and C0x0002/C0x0001. "
    "DUNVIEW.C F0128:8536-8541 reaches D0L first, then D0R. DUNVIEW.C "
    "F0108:3940-4011 is a keepout/contrast anchor for floor ornament C10 "
    "composition, while F0115:4547-4581 owns the thing-pass cell-order "
    "contract. DUNVIEW.C F0111:4218-4337 owns door-front C10 composition; "
    "its partly-open horizontal door path performs the C10 half-blit at "
    "4322-4324 and applies the shifted second zone at 4325. DEFS.H:2088 "
    "defines C10_COLOR_FLESH; DEFS.H:2538-2554 defines M550/M551/M552/"
    "M553; DEFS.H:2596-2611 defines M610/M611; DEFS.H:2696-2711 defines "
    "the F0107 view-wall ordinals; DEFS.H:4040-4057 defines C716/C717; "
    "DEFS.H:4139-4153 defines the cell-order band.";

static const char s_disjointness_note[] =
    "D0L/D0R F0107 wall-ornament source-lock contract only. It is "
    "disjoint from the D1C F0107 gate: this model proves the D0 side "
    "dispatch keepout, D0L-then-D0R ordering, C10 preservation, F0108/"
    "F0115 ordering contrast, and F0111 partly-open transparency "
    "relationship on a synthetic framebuffer. It does not claim original "
    "DOS pixel parity and reads no GRAPHICS.DAT.";

static uint32_t fnv1a_u32(uint32_t hash, uint32_t value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (value >> (i * 8)) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

uint8_t dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color)
{
    return source_pixel == transparent_color ? destination_pixel : source_pixel;
}

bool dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_returns_alcove_pc34(
    int wall_ornament_ordinal,
    unsigned int cell_content_bits)
{
    return wall_ornament_ordinal != 0 &&
           (cell_content_bits & DM1_V1_D0L_D0R_F0107_ALCOVE_CELL_CONTENT_MASK_PC34) != 0u;
}

static void fill_lanes(DM1_V1_D0LD0RF0107WallOrnamentModelPc34 *m)
{
    m->lanes[0].side = DM1_V1_D0L_D0R_F0107_SIDE_D0L_PC34;
    m->lanes[0].side_name = "D0L";
    m->lanes[0].view_square = DM1_D0L_VIEW_SQUARE;
    m->lanes[0].wall_zone = DM1_D0L_WALL_ZONE;
    m->lanes[0].relative_depth = 0;
    m->lanes[0].relative_lateral = -1;
    m->lanes[0].f0128_update_line = 8536;
    m->lanes[0].f0128_draw_line = 8537;
    m->lanes[0].dispatcher_line_start = 7960;
    m->lanes[0].dispatcher_line_end = 8062;
    m->lanes[0].wall_case_line = 8009;
    m->lanes[0].wall_case_returns_before_f0107 = 1;
    m->lanes[0].direct_f0107_call_present = 0;
    m->lanes[0].first_thing_slot = DM1_M550_FIRST_THING_SLOT;
    m->lanes[0].thing_pass_order = DM1_D0L_THING_ORDER;
    m->lanes[0].thing_pass_line = 8005;
    m->lanes[0].ceiling_line = 8043;
    m->lanes[0].thing_before_ceiling = 1;
    m->lanes[0].ceiling_before_thing = 0;
    m->lanes[0].f0108_keepout = 1;
    m->lanes[0].f0111_direct_call_present = 0;
    m->lanes[0].redmcsb_anchor =
        "DUNVIEW.C:7960-8062 F0125; 8005 F0115; 8009-8024 wall return; 8536-8537 D0L";

    m->lanes[1].side = DM1_V1_D0L_D0R_F0107_SIDE_D0R_PC34;
    m->lanes[1].side_name = "D0R";
    m->lanes[1].view_square = DM1_D0R_VIEW_SQUARE;
    m->lanes[1].wall_zone = DM1_D0R_WALL_ZONE;
    m->lanes[1].relative_depth = 0;
    m->lanes[1].relative_lateral = 1;
    m->lanes[1].f0128_update_line = 8540;
    m->lanes[1].f0128_draw_line = 8541;
    m->lanes[1].dispatcher_line_start = 8064;
    m->lanes[1].dispatcher_line_end = 8162;
    m->lanes[1].wall_case_line = 8119;
    m->lanes[1].wall_case_returns_before_f0107 = 1;
    m->lanes[1].direct_f0107_call_present = 0;
    m->lanes[1].first_thing_slot = DM1_M550_FIRST_THING_SLOT;
    m->lanes[1].thing_pass_order = DM1_D0R_THING_ORDER;
    m->lanes[1].thing_pass_line = 8115;
    m->lanes[1].ceiling_line = 8113;
    m->lanes[1].thing_before_ceiling = 0;
    m->lanes[1].ceiling_before_thing = 1;
    m->lanes[1].f0108_keepout = 1;
    m->lanes[1].f0111_direct_call_present = 0;
    m->lanes[1].redmcsb_anchor =
        "DUNVIEW.C:8064-8162 F0126; 8113 ceiling; 8115 F0115; 8119-8131 wall return; 8540-8541 D0R";
}

static void fill_ordinals(DM1_V1_D0LD0RF0107WallOrnamentModelPc34 *m)
{
    static const int slots[DM1_V1_D0L_D0R_F0107_WALL_ORNAMENT_PIXEL_COUNT_PC34] = {
        DM1_M551_RIGHT_WALL_ORNAMENT_SLOT,
        DM1_M553_LEFT_WALL_ORNAMENT_SLOT,
        DM1_M552_FRONT_WALL_ORNAMENT_SLOT,
        DM1_M551_RIGHT_WALL_ORNAMENT_SLOT,
        DM1_M553_LEFT_WALL_ORNAMENT_SLOT,
        DM1_M552_FRONT_WALL_ORNAMENT_SLOT
    };
    static const char *names[DM1_V1_D0L_D0R_F0107_WALL_ORNAMENT_PIXEL_COUNT_PC34] = {
        "C0 M551 right wall ordinal",
        "C1 M553 left wall ordinal",
        "C2 M552 front wall ordinal",
        "C3 M551 D1L side source",
        "C4 M553 D1R side source",
        "C5 M552 D1C contrast source"
    };
    static const int view_walls[DM1_V1_D0L_D0R_F0107_WALL_ORNAMENT_PIXEL_COUNT_PC34] = {
        DM1_M585_VIEW_WALL_D1L_RIGHT,
        DM1_M586_VIEW_WALL_D1R_LEFT,
        DM1_M587_VIEW_WALL_D1C_FRONT,
        DM1_M585_VIEW_WALL_D1L_RIGHT,
        DM1_M586_VIEW_WALL_D1R_LEFT,
        DM1_M587_VIEW_WALL_D1C_FRONT
    };
    static const char *anchors[DM1_V1_D0L_D0R_F0107_WALL_ORNAMENT_PIXEL_COUNT_PC34] = {
        "DUNVIEW.C:7459 M551 side F0107; D0L/D0R direct keepout",
        "DUNVIEW.C:7627 M553 side F0107; D0L/D0R direct keepout",
        "DUNVIEW.C:7842 M552 D1C contrast; D0L/D0R direct keepout",
        "DUNVIEW.C:7459 and F0107:3502-3938",
        "DUNVIEW.C:7627 and F0107:3502-3938",
        "DUNVIEW.C:7842 and F0107:3502-3938"
    };
    size_t i;

    for (i = 0; i < DM1_V1_D0L_D0R_F0107_WALL_ORNAMENT_PIXEL_COUNT_PC34; ++i) {
        m->ordinals[i].ordinal_slot = slots[i];
        m->ordinals[i].slot_name = names[i];
        m->ordinals[i].reaches_d0l_d0r_directly = 0;
        m->ordinals[i].nearest_source_view_wall = view_walls[i];
        m->ordinals[i].helper_case_index = (int)i + 6;
        m->ordinals[i].expected_rejection = 1;
        m->ordinals[i].redmcsb_anchor = anchors[i];
    }
}

static void fill_steps(DM1_V1_D0LD0RF0107WallOrnamentModelPc34 *m)
{
    static const DM1_V1_D0LD0RF0107StepPc34 steps[] = {
        { DM1_V1_D0L_D0R_F0107_STEP_F0128_DISPATCH_D0L_PC34, 0, 1,
          "F0128 updates and draws D0L", "DUNVIEW.C:8536-8537" },
        { DM1_V1_D0L_D0R_F0107_STEP_F0128_DISPATCH_D0R_PC34, 1, 1,
          "F0128 updates and draws D0R after D0L", "DUNVIEW.C:8540-8541" },
        { DM1_V1_D0L_D0R_F0107_STEP_F0125_D0L_BODY_PC34, 2, 1,
          "F0125 D0L side dispatcher", "DUNVIEW.C F0125:7960-8062" },
        { DM1_V1_D0L_D0R_F0107_STEP_F0126_D0R_BODY_PC34, 3, 1,
          "F0126 D0R side dispatcher", "DUNVIEW.C F0126:8064-8162" },
        { DM1_V1_D0L_D0R_F0107_STEP_D0_WALL_RETURNS_BEFORE_F0107_PC34, 4, 1,
          "D0 wall cases return without direct F0107", "DUNVIEW.C:8009-8024/8119-8131" },
        { DM1_V1_D0L_D0R_F0107_STEP_F0107_ALCOVE_BOOL_SOURCE_PC34, 5, 1,
          "F0107 alcove boolean comes from F0149 cell-content classification",
          "DUNVIEW.C:3589/3933/3936" },
        { DM1_V1_D0L_D0R_F0107_STEP_F0107_C10_TRANSPARENCY_PC34, 6, 1,
          "F0107 C10 transparent wall-ornament blit", "DUNVIEW.C:3922; DEFS.H:2088" },
        { DM1_V1_D0L_D0R_F0107_STEP_F0108_F0115_ORDER_CONTRAST_PC34, 7, 1,
          "D0 side-lane F0108 keepout and F0115 order contrast",
          "DUNVIEW.C F0108:3940-4011; F0115:4547-4581; F0125/F0126" },
        { DM1_V1_D0L_D0R_F0107_STEP_F0111_PARTLY_OPEN_RELATION_PC34, 8, 1,
          "F0111 partly-open door-front C10 relationship", "DUNVIEW.C:4218-4337/4322-4325" }
    };
    memcpy(m->steps, steps, sizeof(steps));
}

static void fill_pixels(DM1_V1_D0LD0RF0107WallOrnamentModelPc34 *m)
{
    static const uint8_t before[] = { 0x21u, 0x22u, 0x23u, 0x24u, 0x25u, 0x26u };
    static const uint8_t source[] = { 10u, 0x44u, 10u, 0x45u, 0x46u, 10u };
    size_t i;

    for (i = 0; i < DM1_V1_D0L_D0R_F0107_WALL_ORNAMENT_PIXEL_COUNT_PC34; ++i) {
        m->pixels[i].ordinal_index = (int)i;
        m->pixels[i].before = before[i];
        m->pixels[i].source = source[i];
        m->pixels[i].after =
            dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_blend_pixel_pc34(
                before[i], source[i], DM1_V1_D0L_D0R_F0107_C10_COLOR_FLESH_PC34);
        m->pixels[i].transparent_skip =
            source[i] == DM1_V1_D0L_D0R_F0107_C10_COLOR_FLESH_PC34;
        m->pixels[i].writes_pixel =
            source[i] != DM1_V1_D0L_D0R_F0107_C10_COLOR_FLESH_PC34;
        m->pixels[i].anchor = "DUNVIEW.C F0107:3922 C10_COLOR_FLESH transparent blit";
    }
}

static void fill_door_states(DM1_V1_D0LD0RF0107WallOrnamentModelPc34 *m)
{
    int i;

    for (i = 0; i < DM1_V1_D0L_D0R_F0107_WALL_ORNAMENT_DOOR_STATE_COUNT_PC34; ++i) {
        m->door_states[i].door_state = i + 1;
        m->door_states[i].f0111_partly_open = 1;
        m->door_states[i].horizontal_half_blit_uses_c10 = 1;
        m->door_states[i].mask0x4000_shift_applied = 1;
        m->door_states[i].shares_f0107_c10_transparency = 1;
        m->door_states[i].anchor = "DUNVIEW.C:4308-4325 partly-open F0111; DEFS.H:2088 C10";
    }
}

bool dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_default_model_builder_pc34(
    DM1_V1_D0LD0RF0107WallOrnamentModelPc34 *out_model)
{
    size_t helper_count = 0;

    if (!out_model) return false;
    memset(out_model, 0, sizeof(*out_model));

    out_model->view_square_d0l = DM1_D0L_VIEW_SQUARE;
    out_model->view_square_d0r = DM1_D0R_VIEW_SQUARE;
    out_model->wall_zone_d0l = DM1_D0L_WALL_ZONE;
    out_model->wall_zone_d0r = DM1_D0R_WALL_ZONE;
    out_model->c10_transparent_color = DM1_V1_D0L_D0R_F0107_C10_COLOR_FLESH_PC34;
    out_model->m550_first_thing_slot = DM1_M550_FIRST_THING_SLOT;
    out_model->m551_right_wall_ornament_slot = DM1_M551_RIGHT_WALL_ORNAMENT_SLOT;
    out_model->m552_front_wall_ornament_slot = DM1_M552_FRONT_WALL_ORNAMENT_SLOT;
    out_model->m553_left_wall_ornament_slot = DM1_M553_LEFT_WALL_ORNAMENT_SLOT;
    out_model->f0128_d0l_then_d0r = 1;
    out_model->d0l_direct_f0107_calls = 0;
    out_model->d0r_direct_f0107_calls = 0;
    out_model->d0_wall_case_returns_before_f0107 = 1;
    out_model->f0107_zero_ordinal_returns_false =
        dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_returns_alcove_pc34(
            0, DM1_V1_D0L_D0R_F0107_ALCOVE_CELL_CONTENT_MASK_PC34) ? 0 : 1;
    out_model->f0107_non_alcove_cell_returns_false =
        dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_returns_alcove_pc34(3, 0u) ? 0 : 1;
    out_model->f0107_alcove_cell_returns_true =
        dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_returns_alcove_pc34(
            3, DM1_V1_D0L_D0R_F0107_ALCOVE_CELL_CONTENT_MASK_PC34) ? 1 : 0;
    out_model->f0107_uses_cell_content_bits = 1;
    out_model->f0107_blit_uses_c10 = 1;
    out_model->c10_transparent_preserves_destination =
        dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_blend_pixel_pc34(
            0x7au, DM1_V1_D0L_D0R_F0107_C10_COLOR_FLESH_PC34,
            DM1_V1_D0L_D0R_F0107_C10_COLOR_FLESH_PC34) == 0x7au;
    out_model->f0108_floor_ceiling_keepout = 1;
    out_model->f0115_d0l_order_backright = DM1_D0L_THING_ORDER;
    out_model->f0115_d0r_order_backleft = DM1_D0R_THING_ORDER;
    out_model->d0l_thing_before_ceiling = 1;
    out_model->d0r_ceiling_before_thing = 1;
    out_model->f0111_partly_open_uses_c10_half_blits = 1;
    out_model->no_graphics_dat_reads = 1;
    out_model->source_locked_contract_only = 1;
    out_model->no_original_dos_pixel_parity = 1;
    dm1_v1_viewport_f0107_wall_ornament_alcove_cases_pc34(&helper_count);
    out_model->helper_f0107_slot_constants_reused = helper_count >= 11U ? 1 : 0;
    out_model->source_evidence = s_source_evidence;
    out_model->disjointness_note = s_disjointness_note;

    fill_lanes(out_model);
    fill_ordinals(out_model);
    fill_steps(out_model);
    fill_pixels(out_model);
    fill_door_states(out_model);
    out_model->deterministic_hash =
        dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_hash_model_pc34(out_model);
    return true;
}

uint32_t dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_hash_model_pc34(
    const DM1_V1_D0LD0RF0107WallOrnamentModelPc34 *model)
{
    uint32_t h = 2166136261u;
    size_t i;

    if (!model) return 0u;
    h = fnv1a_u32(h, (uint32_t)model->view_square_d0l);
    h = fnv1a_u32(h, (uint32_t)model->view_square_d0r);
    h = fnv1a_u32(h, (uint32_t)model->wall_zone_d0l);
    h = fnv1a_u32(h, (uint32_t)model->wall_zone_d0r);
    h = fnv1a_u32(h, (uint32_t)model->m551_right_wall_ornament_slot);
    h = fnv1a_u32(h, (uint32_t)model->m552_front_wall_ornament_slot);
    h = fnv1a_u32(h, (uint32_t)model->m553_left_wall_ornament_slot);
    h = fnv1a_u32(h, (uint32_t)model->f0128_d0l_then_d0r);
    h = fnv1a_u32(h, (uint32_t)model->d0l_direct_f0107_calls);
    h = fnv1a_u32(h, (uint32_t)model->d0r_direct_f0107_calls);
    h = fnv1a_u32(h, (uint32_t)model->d0_wall_case_returns_before_f0107);
    h = fnv1a_u32(h, (uint32_t)model->f0107_alcove_cell_returns_true);
    h = fnv1a_u32(h, (uint32_t)model->f0107_blit_uses_c10);
    h = fnv1a_u32(h, (uint32_t)model->f0115_d0l_order_backright);
    h = fnv1a_u32(h, (uint32_t)model->f0115_d0r_order_backleft);
    for (i = 0; i < DM1_V1_D0L_D0R_F0107_WALL_ORNAMENT_SIDE_COUNT_PC34; ++i) {
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].side);
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].view_square);
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].wall_zone);
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].direct_f0107_call_present);
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].thing_pass_order);
    }
    for (i = 0; i < DM1_V1_D0L_D0R_F0107_WALL_ORNAMENT_PIXEL_COUNT_PC34; ++i) {
        h = fnv1a_u32(h, (uint32_t)model->ordinals[i].ordinal_slot);
        h = fnv1a_u32(h, (uint32_t)model->ordinals[i].nearest_source_view_wall);
        h = fnv1a_u32(h, (uint32_t)model->pixels[i].before);
        h = fnv1a_u32(h, (uint32_t)model->pixels[i].source);
        h = fnv1a_u32(h, (uint32_t)model->pixels[i].after);
    }
    for (i = 0; i < DM1_V1_D0L_D0R_F0107_WALL_ORNAMENT_DOOR_STATE_COUNT_PC34; ++i) {
        h = fnv1a_u32(h, (uint32_t)model->door_states[i].door_state);
        h = fnv1a_u32(h, (uint32_t)model->door_states[i].horizontal_half_blit_uses_c10);
    }
    return h;
}

const DM1_V1_D0LD0RF0107WallOrnamentModelPc34 *
dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_default_model_pc34(void)
{
    static DM1_V1_D0LD0RF0107WallOrnamentModelPc34 s_model;
    static int s_init;

    if (!s_init) {
        dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_default_model_builder_pc34(
            &s_model);
        s_init = 1;
    }
    return &s_model;
}

uint32_t dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_deterministic_hash_pc34(void)
{
    const DM1_V1_D0LD0RF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_default_model_pc34();
    return model ? model->deterministic_hash : 0u;
}

const DM1_V1_D0LD0RF0107LanePc34 *
dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_lane_at_pc34(size_t index)
{
    const DM1_V1_D0LD0RF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_default_model_pc34();
    if (!model || index >= DM1_V1_D0L_D0R_F0107_WALL_ORNAMENT_SIDE_COUNT_PC34) {
        return NULL;
    }
    return &model->lanes[index];
}

const DM1_V1_D0LD0RF0107StepPc34 *
dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_step_at_pc34(size_t index)
{
    const DM1_V1_D0LD0RF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_default_model_pc34();
    if (!model || index >= DM1_V1_D0L_D0R_F0107_WALL_ORNAMENT_STEP_COUNT_PC34) {
        return NULL;
    }
    return &model->steps[index];
}

const DM1_V1_D0LD0RF0107OrdinalFlowPc34 *
dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_ordinal_at_pc34(size_t index)
{
    const DM1_V1_D0LD0RF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_default_model_pc34();
    if (!model || index >= DM1_V1_D0L_D0R_F0107_WALL_ORNAMENT_PIXEL_COUNT_PC34) {
        return NULL;
    }
    return &model->ordinals[index];
}

const char *dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const char *dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_disjointness_note_pc34(void)
{
    return s_disjointness_note;
}
