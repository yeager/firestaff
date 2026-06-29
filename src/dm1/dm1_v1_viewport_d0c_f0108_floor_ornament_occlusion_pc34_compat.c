/*
 * ReDMCSB anchors: DUNVIEW.C F0127:8184-8310 D0C dispatch body,
 * F0128:8318-8542 view-sweep order, F0098:2962-3002 floor/ceiling base,
 * F0104 floor-pit/stairs bitmap, F0112 ceiling pit, F0113 teleporter field,
 * F0115 thing pass, F0108:3940-4011 floor-ornament routine, DEFS.H C10,
 * M550/M554/M555/M558/M603/M609, C0x0021, C862, C871, and C715.
 */
#include "firestaff/dm1/v1/viewport/dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_pc34_compat.h"

#include <string.h>

enum {
    DM1_D0C_VIEW_SQUARE = 0,
    DM1_D0C_VIEW_FLOOR = 9,
    DM1_D0C_WALL_ZONE = 715,
    DM1_C10_COLOR_FLESH = 10,
    DM1_M550_FIRST_THING_SLOT = 2,
    DM1_M554_PIT_OR_TELEPORTER_VISIBLE_SLOT = 3,
    DM1_M555_STAIRS_UP_SLOT = 4,
    DM1_M556_DOOR_STATE_SLOT = 7,
    DM1_M557_DOOR_THING_INDEX_SLOT = 8,
    DM1_M558_FLOOR_ORNAMENT_ORDINAL_SLOT = 5,
    DM1_D0C_F0128_DISPATCH_ORDER = 17,
    DM1_D0C_FLOOR_ZONE_BASE = 1500,
    DM1_D0C_FLOOR_ZONE_STRIDE_PC34 = 11,
    DM1_D0C_FLOOR_COORDINATE_SET = 1,
    DM1_D0C_FLOOR_ZONE = 1520,
    DM1_D0C_CELL_ORDER_BACKLEFT_BACKRIGHT = 0x0021,
    DM1_D0C_FLOOR_PIT_GRAPHIC = 57,
    DM1_D0C_FLOOR_PIT_INVISIBLE_GRAPHIC = 63,
    DM1_D0C_FLOOR_PIT_ZONE = 862,
    DM1_D0C_CEILING_PIT_GRAPHIC = 69,
    DM1_D0C_CEILING_PIT_ZONE = 871,
    DM1_D0C_KEEPOUT_ZONE_LEFT = 705,
    DM1_D0C_KEEPOUT_ZONE_RIGHT = 706
};

static const char s_source_evidence[] =
    "ReDMCSB source-lock: DUNVIEW.C F0127:8184-8310 is the D0C dispatch "
    "body. Its element switch covers C16 door-side, C19 stairs-front, "
    "C02 pit, and C05 teleporter, then an unconditional tail calls F0112 "
    "at 8284-8292 and F0115 at 8294 with M609_VIEW_SQUARE_D0C and "
    "C0x0021_CELL_ORDER_BACKLEFT_BACKRIGHT. The F0127 body contains no "
    "F0108_DUNGEONVIEW_DrawFloorOrnament call, unlike D1C F0124:7874/7926. "
    "C02 pit reaches F0104 floor-pit bitmap at 8265-8275; C19 stairs-front "
    "reaches F0104/F0105 at 8221-8254; C05 teleporter reaches F0113 field "
    "at 8302-8308. DUNVIEW.C F0128:8542 dispatches D0C after D0L and D0R "
    "as the last square in the view sweep, after F0098 floor/ceiling base "
    "has been painted at 8338/8443/8615. DUNVIEW.C F0108:3940-4011 remains "
    "the source for floor-ornament ordinal decode, C10 transparency, and "
    "C1500 + CoordinateSet*11 + ViewFloor zone math, but F0127 does not "
    "call it for D0C. DEFS.H anchors: C10_COLOR_FLESH, M550/M554/M555/"
    "M556/M557/M558 aspect slots, M603_VIEW_FLOOR_D0C=9, "
    "M609_VIEW_SQUARE_D0C=0, C0x0021 cell order, C862 floor-pit zone, "
    "C871 ceiling-pit zone, and C715 wall zone.";

static const char s_disjointness_note[] =
    "Disjoint DM1 V1 D0C F0108 no-call occlusion source-lock gate. It "
    "only pins the D0C F0127 dispatch invariant that no F0108 call is made "
    "between the F0104 pit/stairs branch, F0112 ceiling pit, F0115 thing "
    "pass, and optional F0113 teleporter field. It does not replace the "
    "positive D0C F0108 floor-ornament composition gate, the D0C "
    "floor/ceiling/ornament gate, the D0C stairs/pit dispatch gate, the "
    "D1C BUG0_64 floor-ornament occlusion gate, any real GRAPHICS.DAT load, "
    "or original-DOS pixel parity evidence.";

static const DM1_V1_D0CF0108FloorOrnamentOcclusionStepPc34
s_steps[DM1_V1_D0C_F0108_FOCCL_STEP_F0108_ABSENT_FROM_D0C_PC34 + 1] = {
    {
        DM1_V1_D0C_F0108_FOCCL_CONTEXT_CORRIDOR_PC34,
        0,
        1, 0, 0, 0, 0, 0, 0, 0,
        DM1_D0C_FLOOR_ZONE_BASE, DM1_D0C_FLOOR_COORDINATE_SET,
        DM1_D0C_VIEW_FLOOR, DM1_D0C_CELL_ORDER_BACKLEFT_BACKRIGHT,
        DM1_D0C_VIEW_SQUARE, 0, 0, 0, 0, 0, DM1_D0C_WALL_ZONE, 1,
        "F0128 base floor/ceiling precedes D0C dispatch",
        "DUNVIEW.C:8338/8443/8615 F0098 before F0127"
    },
    {
        DM1_V1_D0C_F0108_FOCCL_CONTEXT_CORRIDOR_PC34,
        1,
        1, 0, 0, 0, 0, 0, 0, 0,
        DM1_D0C_FLOOR_ZONE_BASE, DM1_D0C_FLOOR_COORDINATE_SET,
        DM1_D0C_VIEW_FLOOR, DM1_D0C_CELL_ORDER_BACKLEFT_BACKRIGHT,
        DM1_D0C_VIEW_SQUARE, 0, 0, 0, 0, 0, DM1_D0C_WALL_ZONE, 1,
        "F0127 receives an already-painted D0C base surface",
        "DUNVIEW.C:8184-8310 F0127 body"
    },
    {
        DM1_V1_D0C_F0108_FOCCL_CONTEXT_DOOR_SIDE_PC34,
        2,
        0, 1, 0, 1, 0, 0, 0, 0,
        DM1_D0C_FLOOR_ZONE_BASE, DM1_D0C_FLOOR_COORDINATE_SET,
        DM1_D0C_VIEW_FLOOR, DM1_D0C_CELL_ORDER_BACKLEFT_BACKRIGHT,
        DM1_D0C_VIEW_SQUARE, 0, 0, 0, 0, 0, DM1_D0C_WALL_ZONE, 1,
        "C16 door-side uses wallset / door-frame path, not F0108",
        "DUNVIEW.C:8188-8210 C16_ELEMENT_DOOR_SIDE"
    },
    {
        DM1_V1_D0C_F0108_FOCCL_CONTEXT_STAIRS_FRONT_PC34,
        3,
        0, 1, 1, 0, 0, 0, 0, 0,
        DM1_D0C_FLOOR_ZONE_BASE, DM1_D0C_FLOOR_COORDINATE_SET,
        DM1_D0C_VIEW_FLOOR, DM1_D0C_CELL_ORDER_BACKLEFT_BACKRIGHT,
        DM1_D0C_VIEW_SQUARE, 0, 0, 0, 0, 0, DM1_D0C_WALL_ZONE, 1,
        "C19 stairs-front uses F0104/F0105 split lanes, not F0108",
        "DUNVIEW.C:8221-8254 C19_ELEMENT_STAIRS_FRONT"
    },
    {
        DM1_V1_D0C_F0108_FOCCL_CONTEXT_OPEN_PIT_PC34,
        4,
        0, 1, 0, 0, 0, 0, 0, 0,
        DM1_D0C_FLOOR_ZONE_BASE, DM1_D0C_FLOOR_COORDINATE_SET,
        DM1_D0C_VIEW_FLOOR, DM1_D0C_CELL_ORDER_BACKLEFT_BACKRIGHT,
        DM1_D0C_VIEW_SQUARE, DM1_D0C_FLOOR_PIT_GRAPHIC,
        DM1_D0C_FLOOR_PIT_INVISIBLE_GRAPHIC, DM1_D0C_FLOOR_PIT_ZONE,
        0, 0, DM1_D0C_WALL_ZONE, 1,
        "C02 pit uses D0C pit bitmap and keeps F0108 absent",
        "DUNVIEW.C:8265-8275 C02_ELEMENT_PIT"
    },
    {
        DM1_V1_D0C_F0108_FOCCL_CONTEXT_TELEPORTER_PC34,
        5,
        0, 0, 0, 0, 0, 0, 1, 0,
        DM1_D0C_FLOOR_ZONE_BASE, DM1_D0C_FLOOR_COORDINATE_SET,
        DM1_D0C_VIEW_FLOOR, DM1_D0C_CELL_ORDER_BACKLEFT_BACKRIGHT,
        DM1_D0C_VIEW_SQUARE, 0, 0, 0, 0, 0, DM1_D0C_WALL_ZONE, 1,
        "C05 teleporter field happens after F0115, still without F0108",
        "DUNVIEW.C:8302-8308 C05_ELEMENT_TELEPORTER field"
    },
    {
        DM1_V1_D0C_F0108_FOCCL_CONTEXT_OPEN_PIT_PC34,
        6,
        0, 0, 0, 0, 0, 1, 0, 0,
        DM1_D0C_FLOOR_ZONE_BASE, DM1_D0C_FLOOR_COORDINATE_SET,
        DM1_D0C_VIEW_FLOOR, DM1_D0C_CELL_ORDER_BACKLEFT_BACKRIGHT,
        DM1_D0C_VIEW_SQUARE, 0, 0, 0, DM1_D0C_CEILING_PIT_GRAPHIC,
        DM1_D0C_CEILING_PIT_ZONE, DM1_D0C_WALL_ZONE, 1,
        "F0112 D0C ceiling pit follows element branch, not F0108",
        "DUNVIEW.C:8284-8292 F0112_DUNGEONVIEW_DrawCeilingPit"
    },
    {
        DM1_V1_D0C_F0108_FOCCL_CONTEXT_OPEN_PIT_PC34,
        7,
        0, 0, 0, 0, 0, 0, 0, 1,
        DM1_D0C_FLOOR_ZONE_BASE, DM1_D0C_FLOOR_COORDINATE_SET,
        DM1_D0C_VIEW_FLOOR, DM1_D0C_CELL_ORDER_BACKLEFT_BACKRIGHT,
        DM1_D0C_VIEW_SQUARE, 0, 0, 0, 0, 0, DM1_D0C_WALL_ZONE, 1,
        "F0115 thing pass uses M609/C0x0021 with no F0108 predecessor",
        "DUNVIEW.C:8294 F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF"
    },
    {
        DM1_V1_D0C_F0108_FOCCL_CONTEXT_OPEN_PIT_PC34,
        8,
        0, 0, 0, 0, 0, 0, 0, 0,
        DM1_D0C_FLOOR_ZONE_BASE, DM1_D0C_FLOOR_COORDINATE_SET,
        DM1_D0C_VIEW_FLOOR, DM1_D0C_CELL_ORDER_BACKLEFT_BACKRIGHT,
        DM1_D0C_VIEW_SQUARE, 0, 0, 0, 0, 0, DM1_D0C_WALL_ZONE, 1,
        "F0108 is absent from D0C F0127, so BUG0_64 is inapplicable",
        "DUNVIEW.C:8184-8310 contains no F0108 call"
    }
};

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

uint8_t dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_blend_c10_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel)
{
    return source_pixel == DM1_C10_COLOR_FLESH ? destination_pixel : source_pixel;
}

bool dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_context_occludes_pc34(
    DM1_V1_D0CF0108FloorOrnamentOcclusionContextPc34 context)
{
    switch (context) {
    case DM1_V1_D0C_F0108_FOCCL_CONTEXT_DOOR_SIDE_PC34:
    case DM1_V1_D0C_F0108_FOCCL_CONTEXT_STAIRS_FRONT_PC34:
    case DM1_V1_D0C_F0108_FOCCL_CONTEXT_OPEN_PIT_PC34:
    case DM1_V1_D0C_F0108_FOCCL_CONTEXT_TELEPORTER_PC34:
    case DM1_V1_D0C_F0108_FOCCL_CONTEXT_CORRIDOR_PC34:
        return false;
    }
    return false;
}

int dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_zone_d0c_pc34(
    int coordinate_set,
    int view_floor)
{
    if (coordinate_set < 0) coordinate_set = 0;
    if (view_floor < 0) view_floor = 0;
    return DM1_D0C_FLOOR_ZONE_BASE +
        coordinate_set * DM1_D0C_FLOOR_ZONE_STRIDE_PC34 + view_floor;
}

static void fill_model(DM1_V1_D0CF0108FloorOrnamentOcclusionModelPc34 *m)
{
    m->view_square_d0c = DM1_D0C_VIEW_SQUARE;
    m->view_floor_d0c = DM1_D0C_VIEW_FLOOR;
    m->wall_zone_d0c = DM1_D0C_WALL_ZONE;
    m->c10_transparent_color = DM1_C10_COLOR_FLESH;
    m->floor_ornament_ordinal_slot = DM1_M558_FLOOR_ORNAMENT_ORDINAL_SLOT;
    m->first_thing_slot = DM1_M550_FIRST_THING_SLOT;
    m->pit_or_teleporter_visible_slot = DM1_M554_PIT_OR_TELEPORTER_VISIBLE_SLOT;
    m->stairs_up_slot = DM1_M555_STAIRS_UP_SLOT;
    m->door_state_slot = DM1_M556_DOOR_STATE_SLOT;
    m->door_thing_index_slot = DM1_M557_DOOR_THING_INDEX_SLOT;
    m->f0098_base_order = 0;
    m->f0128_dispatch_order_d0c = DM1_D0C_F0128_DISPATCH_ORDER;
    m->f0128_dispatch_after_d0l = 1;
    m->f0128_dispatch_after_d0r = 1;
    m->f0128_dispatch_last_in_sweep = 1;
    m->f0127_door_side_calls_f0100 = 1;
    m->f0127_door_side_calls_f0104 = 1;
    m->f0127_stairs_front_calls_f0104 = 1;
    m->f0127_stairs_front_calls_f0105 = 1;
    m->f0127_open_pit_calls_f0104 = 1;
    m->f0127_open_pit_uses_pit_visible_slot = 1;
    m->f0127_teleporter_calls_f0113_field = 1;
    m->f0127_teleporter_uses_pit_visible_slot = 1;
    m->f0127_calls_f0112_ceiling_pit = 1;
    m->f0127_calls_f0115_thing_pass = 1;
    m->f0127_d0c_cell_order = DM1_D0C_CELL_ORDER_BACKLEFT_BACKRIGHT;
    m->f0127_d0c_dispatch_no_f0108 = 1;
    m->f0098_base_writes_before_f0127 = 1;
    m->f0108_zone_d0c = DM1_D0C_FLOOR_ZONE;
    m->no_f0108_call_in_d0c_dispatch = 1;
    m->bug0_64_inapplicable_to_d0c = 1;
    m->f0112_ceiling_pit_graphic = DM1_D0C_CEILING_PIT_GRAPHIC;
    m->f0112_ceiling_pit_zone_d0c = DM1_D0C_CEILING_PIT_ZONE;
    m->no_graphics_dat_reads = 1;
    m->source_locked_contract_only = 1;
    m->no_real_asset_bitmap_parity = 1;
    m->f0108_d0c_call_site_absent = 1;
    m->f0107_keepout_zone_left = DM1_D0C_KEEPOUT_ZONE_LEFT;
    m->f0107_keepout_zone_right = DM1_D0C_KEEPOUT_ZONE_RIGHT;
    m->source_evidence = s_source_evidence;
    m->disjointness_note = s_disjointness_note;
}

bool dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_default_model_builder_pc34(
    DM1_V1_D0CF0108FloorOrnamentOcclusionModelPc34 *out_model)
{
    if (!out_model) return false;
    memset(out_model, 0, sizeof(*out_model));
    fill_model(out_model);
    out_model->deterministic_hash =
        dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_hash_model_pc34(
            out_model);
    return true;
}

uint32_t dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_hash_model_pc34(
    const DM1_V1_D0CF0108FloorOrnamentOcclusionModelPc34 *model)
{
    uint32_t h = 2166136261u;
    size_t i;

    if (!model) return 0u;
    h = fnv1a_u32(h, (uint32_t)model->view_square_d0c);
    h = fnv1a_u32(h, (uint32_t)model->view_floor_d0c);
    h = fnv1a_u32(h, (uint32_t)model->wall_zone_d0c);
    h = fnv1a_u32(h, (uint32_t)model->c10_transparent_color);
    h = fnv1a_u32(h, (uint32_t)model->floor_ornament_ordinal_slot);
    h = fnv1a_u32(h, (uint32_t)model->first_thing_slot);
    h = fnv1a_u32(h, (uint32_t)model->pit_or_teleporter_visible_slot);
    h = fnv1a_u32(h, (uint32_t)model->stairs_up_slot);
    h = fnv1a_u32(h, (uint32_t)model->door_state_slot);
    h = fnv1a_u32(h, (uint32_t)model->door_thing_index_slot);
    h = fnv1a_u32(h, (uint32_t)model->f0128_dispatch_order_d0c);
    h = fnv1a_u32(h, (uint32_t)model->f0128_dispatch_after_d0l);
    h = fnv1a_u32(h, (uint32_t)model->f0128_dispatch_after_d0r);
    h = fnv1a_u32(h, (uint32_t)model->f0128_dispatch_last_in_sweep);
    h = fnv1a_u32(h, (uint32_t)model->f0127_door_side_calls_f0100);
    h = fnv1a_u32(h, (uint32_t)model->f0127_door_side_calls_f0104);
    h = fnv1a_u32(h, (uint32_t)model->f0127_stairs_front_calls_f0104);
    h = fnv1a_u32(h, (uint32_t)model->f0127_stairs_front_calls_f0105);
    h = fnv1a_u32(h, (uint32_t)model->f0127_open_pit_calls_f0104);
    h = fnv1a_u32(h, (uint32_t)model->f0127_open_pit_uses_pit_visible_slot);
    h = fnv1a_u32(h, (uint32_t)model->f0127_teleporter_calls_f0113_field);
    h = fnv1a_u32(h, (uint32_t)model->f0127_teleporter_uses_pit_visible_slot);
    h = fnv1a_u32(h, (uint32_t)model->f0127_calls_f0112_ceiling_pit);
    h = fnv1a_u32(h, (uint32_t)model->f0127_calls_f0115_thing_pass);
    h = fnv1a_u32(h, (uint32_t)model->f0127_d0c_cell_order);
    h = fnv1a_u32(h, (uint32_t)model->f0127_d0c_dispatch_no_f0108);
    h = fnv1a_u32(h, (uint32_t)model->f0098_base_writes_before_f0127);
    h = fnv1a_u32(h, (uint32_t)model->f0108_zone_d0c);
    h = fnv1a_u32(h, (uint32_t)model->no_f0108_call_in_d0c_dispatch);
    h = fnv1a_u32(h, (uint32_t)model->bug0_64_inapplicable_to_d0c);
    h = fnv1a_u32(h, (uint32_t)model->f0112_ceiling_pit_graphic);
    h = fnv1a_u32(h, (uint32_t)model->f0112_ceiling_pit_zone_d0c);
    h = fnv1a_u32(h, (uint32_t)model->f0108_d0c_call_site_absent);
    h = fnv1a_u32(h, (uint32_t)model->f0107_keepout_zone_left);
    h = fnv1a_u32(h, (uint32_t)model->f0107_keepout_zone_right);
    for (i = 0; i < sizeof(s_steps) / sizeof(s_steps[0]); ++i) {
        h = fnv1a_u32(h, (uint32_t)s_steps[i].context);
        h = fnv1a_u32(h, (uint32_t)s_steps[i].calls_f0098_ceiling_floor);
        h = fnv1a_u32(h, (uint32_t)s_steps[i].calls_f0104_floor_pit_stairs);
        h = fnv1a_u32(h, (uint32_t)s_steps[i].calls_f0105_floor_pit_stairs_flipped);
        h = fnv1a_u32(h, (uint32_t)s_steps[i].calls_f0100_wallset_door_frame);
        h = fnv1a_u32(h, (uint32_t)s_steps[i].calls_f0108_floor_ornament);
        h = fnv1a_u32(h, (uint32_t)s_steps[i].calls_f0112_ceiling_pit);
        h = fnv1a_u32(h, (uint32_t)s_steps[i].calls_f0113_field);
        h = fnv1a_u32(h, (uint32_t)s_steps[i].calls_f0115_thing_pass);
        h = fnv1a_u32(h, (uint32_t)s_steps[i].expected_cell_order);
        h = fnv1a_u32(h, (uint32_t)s_steps[i].expected_view_square);
        h = fnv1a_u32(h, (uint32_t)s_steps[i].expected_d0c_no_f0108_contract);
    }
    return h;
}

const DM1_V1_D0CF0108FloorOrnamentOcclusionModelPc34 *
dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_default_model_pc34(void)
{
    static DM1_V1_D0CF0108FloorOrnamentOcclusionModelPc34 s_model;
    static int s_init;

    if (!s_init) {
        dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_default_model_builder_pc34(
            &s_model);
        s_init = 1;
    }
    return &s_model;
}

uint32_t dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_deterministic_hash_pc34(void)
{
    const DM1_V1_D0CF0108FloorOrnamentOcclusionModelPc34 *model =
        dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_default_model_pc34();
    return model ? model->deterministic_hash : 0u;
}

unsigned int dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_context_count_pc34(void)
{
    return (unsigned int)(sizeof(s_steps) / sizeof(s_steps[0]));
}

const DM1_V1_D0CF0108FloorOrnamentOcclusionStepPc34 *
dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_step_at_pc34(size_t index)
{
    if (index >= sizeof(s_steps) / sizeof(s_steps[0])) return NULL;
    return &s_steps[index];
}

const char *dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const char *dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_disjointness_note_pc34(void)
{
    return s_disjointness_note;
}
