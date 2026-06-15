#include "firestaff/dm1/v1/viewport/dm1_v1_viewport_d1c_f0107_wall_ornament_pc34_compat.h"

#include "dm1_v1_viewport_f0107_wall_ornament_alcove_pc34_compat.h"

#include <string.h>

enum {
    DM1_D1C_VIEW_SQUARE = 3,
    DM1_D1C_VIEW_WALL_FRONT = 14,
    DM1_D1C_WALL_ZONE = 712,
    DM1_C10_COLOR_FLESH = 10,
    DM1_M550_FIRST_THING_SLOT = 2,
    DM1_M551_RIGHT_WALL_ORNAMENT_SLOT = 4,
    DM1_M552_FRONT_WALL_ORNAMENT_SLOT = 5,
    DM1_M553_LEFT_WALL_ORNAMENT_SLOT = 6,
    DM1_C1004_ZONE_WALL_ORNAMENT = 1004,
    DM1_WALL_ORNAMENT_ZONE_STRIDE = 15,
    DM1_D1C_WALL_ORNAMENT_COORDINATE_SET = 2,
    DM1_D1C_F0128_ORDER = 14
};

static const char s_source_evidence[] =
    "ReDMCSB source-lock: DUNVIEW.C F0107:3502-3938 "
    "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF decrements a "
    "non-zero wall ornament ordinal, resolves NativeBitmapIndex and "
    "CoordinateSet, classifies the wall ornament through F0149, applies "
    "D1C front-facing alcove/Vi altar/fountain side effects, and blits "
    "the wall ornament with C10_COLOR_FLESH before returning the alcove "
    "boolean. DUNVIEW.C F0104:3113-3156 is the native C10 transparent "
    "bitmap blit shape used by D1C wall-adjacent overlays. DUNVIEW.C "
    "F0108:3940-4011 is intentionally kept out of this D1C wall route. "
    "DUNVIEW.C F0115:4547-4581 defines cell-order dispatch, and the D1C "
    "alcove thing pass uses C0x0000_CELL_ORDER_ALCOVE only when F0107 "
    "returns true. DUNVIEW.C F0124:7727-7924 is the D1C dispatch body: "
    "the C00_ELEMENT_WALL case draws the opaque D1C wall, calls F0107 "
    "with L0218[M552_FRONT_WALL_ORNAMENT_ORDINAL] and "
    "M587_VIEW_WALL_D1C_FRONT, then conditionally calls F0115 with "
    "L0218[M550_FIRST_THING] and M606_VIEW_SQUARE_D1C. DUNVIEW.C "
    "F0128:8318-8542 dispatches D1L, D1R, D1C, D0L, D0R, D0C in order. "
    "DEFS.H:2088 defines C10_COLOR_FLESH; DEFS.H:2596-2611 defines "
    "M606_VIEW_SQUARE_D1C=3; DEFS.H:4045-4046 are adjacent C705/C706 "
    "wall zones while the D1C wall is C712; DEFS.H:4139-4153 is the "
    "cell-order zone band; DEFS.H:2538-2554 defines M550/M551/M552/M553; "
    "DEFS.H:2696-2711 defines M587_VIEW_WALL_D1C_FRONT=14.";

static const char s_disjointness_note[] =
    "Disjoint DM1 V1 D1C F0107 wall-ornament source-lock gate. It does "
    "not touch D0C/D0L/D0R/D2/D3 tests, any F0108 floor/ceiling/ornament "
    "test, any D1C F0111/F0115/stairs/pit/center-field test, "
    "src/dm1/dm1_v1_viewport_d1c_wall_pc34_compat.*, or CSB/Nexus/"
    "Theron/DM2 files. It is asset-free and does not read GRAPHICS.DAT.";

static uint32_t fnv1a_u32(uint32_t hash, uint32_t value)
{
    hash ^= value & 0xffu;
    hash *= 16777619u;
    hash ^= (value >> 8) & 0xffu;
    hash *= 16777619u;
    hash ^= (value >> 16) & 0xffu;
    hash *= 16777619u;
    hash ^= (value >> 24) & 0xffu;
    hash *= 16777619u;
    return hash;
}

uint8_t dm1_v1_viewport_d1c_f0107_wall_ornament_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color)
{
    return source_pixel == transparent_color ? destination_pixel : source_pixel;
}

bool dm1_v1_viewport_d1c_f0107_wall_ornament_returns_alcove_pc34(
    int wall_ornament_ordinal,
    bool dungeon_classifies_alcove)
{
    return wall_ornament_ordinal != 0 && dungeon_classifies_alcove;
}

static void fill_slots(DM1_V1_D1CF0107WallOrnamentModelPc34 *m)
{
    m->slots[0].aspect_slot = DM1_M552_FRONT_WALL_ORNAMENT_SLOT;
    m->slots[0].slot_name = "M552_FRONT_WALL_ORNAMENT_ORDINAL";
    m->slots[0].view_wall = DM1_D1C_VIEW_WALL_FRONT;
    m->slots[0].reaches_d1c_f0107 = 1;
    m->slots[0].can_trigger_alcove_thing_pass = 1;
    m->slots[0].side_slot_rejected_by_d1c = 0;
    m->slots[0].expected_ord_flow_index = 0;
    m->slots[0].redmcsb_anchor =
        "DUNVIEW.C:7842; DEFS.H:2548/2554 M552; DEFS.H:2710 M587";

    m->slots[1].aspect_slot = DM1_M551_RIGHT_WALL_ORNAMENT_SLOT;
    m->slots[1].slot_name = "M551_RIGHT_WALL_ORNAMENT_ORDINAL";
    m->slots[1].view_wall = DM1_D1C_VIEW_WALL_FRONT;
    m->slots[1].reaches_d1c_f0107 = 0;
    m->slots[1].can_trigger_alcove_thing_pass = 0;
    m->slots[1].side_slot_rejected_by_d1c = 1;
    m->slots[1].expected_ord_flow_index = 1;
    m->slots[1].redmcsb_anchor =
        "DUNVIEW.C:7459 side-wall contrast; F0124:7842 reads M552 only";

    m->slots[2].aspect_slot = DM1_M553_LEFT_WALL_ORNAMENT_SLOT;
    m->slots[2].slot_name = "M553_LEFT_WALL_ORNAMENT_ORDINAL";
    m->slots[2].view_wall = DM1_D1C_VIEW_WALL_FRONT;
    m->slots[2].reaches_d1c_f0107 = 0;
    m->slots[2].can_trigger_alcove_thing_pass = 0;
    m->slots[2].side_slot_rejected_by_d1c = 1;
    m->slots[2].expected_ord_flow_index = 2;
    m->slots[2].redmcsb_anchor =
        "DUNVIEW.C:7627 side-wall contrast; F0124:7842 reads M552 only";
}

static void fill_steps(DM1_V1_D1CF0107WallOrnamentModelPc34 *m)
{
    m->steps[0].step = DM1_V1_D1C_F0107_STEP_F0128_DISPATCH_D1C_PC34;
    m->steps[0].order_index = 0;
    m->steps[0].expected_present = 1;
    m->steps[0].name = "F0128 dispatches D1C after D1L/D1R";
    m->steps[0].redmcsb_anchor = "DUNVIEW.C:8524-8536";

    m->steps[1].step = DM1_V1_D1C_F0107_STEP_F0124_WALL_CASE_PC34;
    m->steps[1].order_index = 1;
    m->steps[1].expected_present = 1;
    m->steps[1].name = "F0124 C00_ELEMENT_WALL branch";
    m->steps[1].redmcsb_anchor = "DUNVIEW.C:7790-7848";

    m->steps[2].step = DM1_V1_D1C_F0107_STEP_F0765_OPAQUE_D1C_WALL_PC34;
    m->steps[2].order_index = 2;
    m->steps[2].expected_present = 1;
    m->steps[2].name = "opaque D1C wall draw before wall ornament";
    m->steps[2].redmcsb_anchor = "DUNVIEW.C:7836-7840; DEFS.H:4049 C712";

    m->steps[3].step = DM1_V1_D1C_F0107_STEP_F0107_FRONT_WALL_ORNAMENT_PC34;
    m->steps[3].order_index = 3;
    m->steps[3].expected_present = 1;
    m->steps[3].name = "F0107 M552 front wall ornament";
    m->steps[3].redmcsb_anchor = "DUNVIEW.C:7842; F0107:3502-3938";

    m->steps[4].step = DM1_V1_D1C_F0107_STEP_F0115_ALCOVE_THING_PASS_PC34;
    m->steps[4].order_index = 4;
    m->steps[4].expected_present = 1;
    m->steps[4].name = "conditional F0115 alcove object pass";
    m->steps[4].redmcsb_anchor = "DUNVIEW.C:7843-7845; F0115:4547-4581";

    m->steps[5].step = DM1_V1_D1C_F0107_STEP_F0108_KEEP_OUT_PC34;
    m->steps[5].order_index = 5;
    m->steps[5].expected_present = 0;
    m->steps[5].name = "F0108 keepout for D1C wall route";
    m->steps[5].redmcsb_anchor = "DUNVIEW.C:3940-4011 not called by F0124 wall case";

    m->steps[6].step = DM1_V1_D1C_F0107_STEP_F0111_KEEP_OUT_PC34;
    m->steps[6].order_index = 6;
    m->steps[6].expected_present = 0;
    m->steps[6].name = "F0111 keepout for D1C wall route";
    m->steps[6].redmcsb_anchor = "DUNVIEW.C:4218-4337 not called by F0124 wall case";
}

static void fill_pixels(DM1_V1_D1CF0107WallOrnamentModelPc34 *m)
{
    static const uint8_t before[] = { 0x11u, 0x22u, 0x33u, 0x44u, 0x55u, 0x66u };
    static const uint8_t source[] = { 0x10u, 10u, 0x2au, 10u, 0x7bu, 0x01u };
    size_t i;

    for (i = 0; i < DM1_V1_D1C_F0107_WALL_ORNAMENT_PIXEL_COUNT_PC34; ++i) {
        m->pixels[i].before = before[i];
        m->pixels[i].source = source[i];
        m->pixels[i].after =
            dm1_v1_viewport_d1c_f0107_wall_ornament_blend_pixel_pc34(
                before[i], source[i], DM1_C10_COLOR_FLESH);
        m->pixels[i].transparent_skip = source[i] == DM1_C10_COLOR_FLESH;
        m->pixels[i].writes_pixel = source[i] != DM1_C10_COLOR_FLESH;
        m->pixels[i].anchor = "DUNVIEW.C F0107:3922 F0791 C10 transparent blit";
    }
}

bool dm1_v1_viewport_d1c_f0107_wall_ornament_default_model_builder_pc34(
    DM1_V1_D1CF0107WallOrnamentModelPc34 *out_model)
{
    size_t helper_count = 0;

    if (!out_model) return false;
    memset(out_model, 0, sizeof(*out_model));

    out_model->view_square_d1c = DM1_D1C_VIEW_SQUARE;
    out_model->view_wall_d1c_front = DM1_D1C_VIEW_WALL_FRONT;
    out_model->wall_zone_d1c = DM1_D1C_WALL_ZONE;
    out_model->c10_transparent_color = DM1_C10_COLOR_FLESH;
    out_model->front_wall_ornament_slot =
        DM1_V1_F0107_SLOT_M552_FRONT_WALL_ORNAMENT_PC34;
    out_model->right_wall_ornament_slot =
        DM1_V1_F0107_SLOT_M551_RIGHT_WALL_ORNAMENT_PC34;
    out_model->left_wall_ornament_slot =
        DM1_V1_F0107_SLOT_M553_LEFT_WALL_ORNAMENT_PC34;
    out_model->first_thing_slot = DM1_M550_FIRST_THING_SLOT;
    out_model->alcove_cell_order = 0x0000u;
    out_model->f0128_dispatch_order_d1c = DM1_D1C_F0128_ORDER;
    out_model->f0128_dispatch_after_d1l = 1;
    out_model->f0128_dispatch_after_d1r = 1;
    out_model->f0128_dispatch_before_d0l = 1;
    out_model->f0124_wall_case_uses_f0107 = 1;
    out_model->f0124_wall_case_uses_f0108 = 0;
    out_model->f0124_wall_case_uses_f0111 = 0;
    out_model->f0124_wall_case_uses_f0115_only_for_alcove = 1;
    out_model->f0107_zero_ordinal_returns_false =
        dm1_v1_viewport_d1c_f0107_wall_ornament_returns_alcove_pc34(0, true) ? 0 : 1;
    out_model->f0107_non_alcove_returns_false =
        dm1_v1_viewport_d1c_f0107_wall_ornament_returns_alcove_pc34(3, false) ? 0 : 1;
    out_model->f0107_alcove_returns_true =
        dm1_v1_viewport_d1c_f0107_wall_ornament_returns_alcove_pc34(3, true) ? 1 : 0;
    out_model->f0107_sets_facing_alcove = 1;
    out_model->f0107_sets_vi_altar = 1;
    out_model->f0107_sets_fountain = 1;
    out_model->f0107_blit_uses_c10 = 1;
    out_model->wall_ornament_zone_base = DM1_C1004_ZONE_WALL_ORNAMENT;
    out_model->wall_ornament_zone_stride = DM1_WALL_ORNAMENT_ZONE_STRIDE;
    out_model->wall_ornament_coordinate_set = DM1_D1C_WALL_ORNAMENT_COORDINATE_SET;
    out_model->wall_ornament_zone_d1c_front =
        DM1_C1004_ZONE_WALL_ORNAMENT +
        DM1_D1C_WALL_ORNAMENT_COORDINATE_SET * DM1_WALL_ORNAMENT_ZONE_STRIDE +
        DM1_D1C_VIEW_WALL_FRONT;
    out_model->wall_ornament_native_bitmap_incremented_for_front = 1;
    out_model->wall_ornament_palette_d1c_native = 1;
    out_model->f0115_alcove_uses_first_thing = 1;
    out_model->f0115_alcove_uses_d1c_view_square = 1;
    out_model->no_graphics_dat_reads = 1;
    out_model->source_locked_contract_only = 1;
    out_model->no_real_asset_bitmap_parity = 1;
    dm1_v1_viewport_f0107_wall_ornament_alcove_cases_pc34(&helper_count);
    out_model->helper_f0107_slot_constants_reused = helper_count > 0U ? 1 : 0;
    out_model->source_evidence = s_source_evidence;
    out_model->disjointness_note = s_disjointness_note;

    fill_slots(out_model);
    fill_steps(out_model);
    fill_pixels(out_model);
    out_model->deterministic_hash =
        dm1_v1_viewport_d1c_f0107_wall_ornament_hash_model_pc34(out_model);
    return true;
}

uint32_t dm1_v1_viewport_d1c_f0107_wall_ornament_hash_model_pc34(
    const DM1_V1_D1CF0107WallOrnamentModelPc34 *model)
{
    uint32_t h = 2166136261u;
    size_t i;

    if (!model) return 0u;
    h = fnv1a_u32(h, (uint32_t)model->view_square_d1c);
    h = fnv1a_u32(h, (uint32_t)model->view_wall_d1c_front);
    h = fnv1a_u32(h, (uint32_t)model->wall_zone_d1c);
    h = fnv1a_u32(h, (uint32_t)model->front_wall_ornament_slot);
    h = fnv1a_u32(h, (uint32_t)model->right_wall_ornament_slot);
    h = fnv1a_u32(h, (uint32_t)model->left_wall_ornament_slot);
    h = fnv1a_u32(h, (uint32_t)model->alcove_cell_order);
    h = fnv1a_u32(h, (uint32_t)model->wall_ornament_zone_d1c_front);
    h = fnv1a_u32(h, (uint32_t)model->f0124_wall_case_uses_f0107);
    h = fnv1a_u32(h, (uint32_t)model->f0124_wall_case_uses_f0108);
    h = fnv1a_u32(h, (uint32_t)model->f0124_wall_case_uses_f0111);
    h = fnv1a_u32(h, (uint32_t)model->f0107_alcove_returns_true);
    h = fnv1a_u32(h, (uint32_t)model->f0107_blit_uses_c10);
    for (i = 0; i < DM1_V1_D1C_F0107_WALL_ORNAMENT_SLOT_COUNT_PC34; ++i) {
        h = fnv1a_u32(h, (uint32_t)model->slots[i].aspect_slot);
        h = fnv1a_u32(h, (uint32_t)model->slots[i].reaches_d1c_f0107);
        h = fnv1a_u32(h, (uint32_t)model->slots[i].can_trigger_alcove_thing_pass);
    }
    for (i = 0; i < DM1_V1_D1C_F0107_WALL_ORNAMENT_STEP_COUNT_PC34; ++i) {
        h = fnv1a_u32(h, (uint32_t)model->steps[i].step);
        h = fnv1a_u32(h, (uint32_t)model->steps[i].expected_present);
    }
    for (i = 0; i < DM1_V1_D1C_F0107_WALL_ORNAMENT_PIXEL_COUNT_PC34; ++i) {
        h = fnv1a_u32(h, (uint32_t)model->pixels[i].before);
        h = fnv1a_u32(h, (uint32_t)model->pixels[i].source);
        h = fnv1a_u32(h, (uint32_t)model->pixels[i].after);
    }
    return h;
}

const DM1_V1_D1CF0107WallOrnamentModelPc34 *
dm1_v1_viewport_d1c_f0107_wall_ornament_default_model_pc34(void)
{
    static DM1_V1_D1CF0107WallOrnamentModelPc34 s_model;
    static int s_init;

    if (!s_init) {
        dm1_v1_viewport_d1c_f0107_wall_ornament_default_model_builder_pc34(&s_model);
        s_init = 1;
    }
    return &s_model;
}

uint32_t dm1_v1_viewport_d1c_f0107_wall_ornament_deterministic_hash_pc34(void)
{
    const DM1_V1_D1CF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d1c_f0107_wall_ornament_default_model_pc34();
    return model ? model->deterministic_hash : 0u;
}

const DM1_V1_D1CF0107SlotFlowPc34 *
dm1_v1_viewport_d1c_f0107_wall_ornament_slot_flow_at_pc34(size_t index)
{
    const DM1_V1_D1CF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d1c_f0107_wall_ornament_default_model_pc34();
    if (!model || index >= DM1_V1_D1C_F0107_WALL_ORNAMENT_SLOT_COUNT_PC34) {
        return NULL;
    }
    return &model->slots[index];
}

const DM1_V1_D1CF0107StepPc34 *
dm1_v1_viewport_d1c_f0107_wall_ornament_step_at_pc34(size_t index)
{
    const DM1_V1_D1CF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d1c_f0107_wall_ornament_default_model_pc34();
    if (!model || index >= DM1_V1_D1C_F0107_WALL_ORNAMENT_STEP_COUNT_PC34) {
        return NULL;
    }
    return &model->steps[index];
}

const char *dm1_v1_viewport_d1c_f0107_wall_ornament_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const char *dm1_v1_viewport_d1c_f0107_wall_ornament_disjointness_note_pc34(void)
{
    return s_disjointness_note;
}
