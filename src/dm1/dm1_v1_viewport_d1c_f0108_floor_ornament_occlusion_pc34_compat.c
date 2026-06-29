/*
 * ReDMCSB anchors: DUNVIEW.C F0124:7727-7924 D1C dispatch body, F0108:3940-4011
 * floor-ornament ordinal / MASK0x8000_FOOTPRINTS recursion / C10 transparent
 * blit / PC 3.4 I34E MEDIA709 C1500 + CoordinateSet*11 + ViewFloor zone
 * math, F0128:8318-8542
 * D1L/D1R/D1C/D0L/D0R/D0C dispatch order, F0112 ceiling-pit, F0115 thing pass
 * with C0x0218_DOORPASS1 / C0x3421_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT
 * cell orders, F0104 floor-pit/stairs bitmap (C0x0218 door-front, C0x3421
 * pit/teleporter/corridor path), DEFS.H:2088 C10_COLOR_FLESH, 2533-2559
 * M550/M558/M554/M555/M556/M557, 2596-2611 M606_VIEW_SQUARE_D1C=3,
 * 2668-2677 cell orders, 2696-2711 M587_VIEW_WALL_D1C_FRONT=14, 2746
 * M595_VIEW_FLOOR_D1C=7, 4045-4046 C705/C706, 4052 C712_ZONE_WALL_D1C.
 */
#include "firestaff/dm1/v1/viewport/dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_pc34_compat.h"

#include <string.h>

enum {
    DM1_D1C_VIEW_SQUARE = 3,
    DM1_D1C_VIEW_FLOOR = 7,
    DM1_D1C_VIEW_WALL_FRONT = 14,
    DM1_D1C_WALL_ZONE = 712,
    DM1_C10_COLOR_FLESH = 10,
    DM1_M550_FIRST_THING_SLOT = 2,
    DM1_M552_FRONT_WALL_ORNAMENT_SLOT = 5,
    DM1_M554_PIT_OR_TELEPORTER_VISIBLE_SLOT = 3,
    DM1_M555_STAIRS_UP_SLOT = 4,
    DM1_M556_DOOR_STATE_SLOT = 7,
    DM1_M557_DOOR_THING_INDEX_SLOT = 8,
    DM1_M558_FLOOR_ORNAMENT_ORDINAL_SLOT = 5,
    DM1_D1C_F0128_DISPATCH_ORDER = 14,
    DM1_D1C_FLOOR_ZONE_BASE = 1500,
    DM1_D1C_FLOOR_ZONE_STRIDE_PC34 = 11,
    DM1_D1C_FLOOR_COORDINATE_SET = 1,
    DM1_D1C_FLOOR_ZONE = 1518,
    DM1_D1C_CEILING_PIT_GRAPHIC = 67,
    DM1_D1C_CEILING_PIT_ZONE = 868,
    DM1_CELL_ORDER_ALCOVE = 0x0000,
    DM1_CELL_ORDER_DOORPASS1 = 0x0218,
    DM1_CELL_ORDER_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT = 0x3421,
    DM1_CELL_ORDER_DOORPASS2 = 0x0349,
    DM1_FLOOR_ORNAMENT_FOOTPRINTS = 15
};

static const char s_source_evidence[] =
    "ReDMCSB source-lock: DUNVIEW.C F0124:7727-7924 is the D1C dispatch "
    "body. The C19_ELEMENT_STAIRS_FRONT case at 7868 fires F0104 stairs "
    "bitmap (M555_STAIRS_UP routes 7825-7866), then `goto T0124017` to "
    "share the F0108/F0112/F0115 corridor/pit/teleporter tail. The "
    "C02_ELEMENT_PIT case at 7906 fires F0104 floor-pit bitmap "
    "(M765/M759 graphic, C857/C859 zone) and falls through to "
    "C05_ELEMENT_TELEPORTER / C01_ELEMENT_CORRIDOR. The C17_ELEMENT_DOOR_FRONT "
    "case at 7873 fires F0108 once at 7874 then F0115 with "
    "C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT. T0124017 fires F0108 "
    "at 7926 with the source comment BUG0_64 (no occlusion guard over "
    "open pits), F0112 ceiling-pit at 7929 (C067/C866/C868 zone), and "
    "F0115 with C0x3421_CELL_ORDER_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT at "
    "7937. DUNVIEW.C F0108:3940-4011 anchors the "
    "M558_FLOOR_ORNAMENT_ORDINAL handler, MASK0x8000_FOOTPRINTS recursion at "
    "T0108005, C10_COLOR_FLESH transparency, and PC 3.4 I34E MEDIA709 "
    "zone math C1500 + CoordinateSet*11 + ViewFloor. The neighboring "
    "MEDIA458/MEDIA506 C1500 + CoordinateSet*9 branches are not the PC "
    "3.4 path pinned by this gate. The D1C column-center square "
    "is M595_VIEW_FLOOR_D1C=7, M606_VIEW_SQUARE_D1C=3, and the D1C front "
    "wall is M587_VIEW_WALL_D1C_FRONT=14. F0128:8318-8542 dispatches D1C "
    "after D1L (8526) and D1R (8531) and before D0L (8541) at line 8536. "
    "DEFS.H:2088 C10_COLOR_FLESH; DEFS.H:2533-2559 "
    "M550_FIRST_THING / M554_PIT_OR_TELEPORTER_VISIBLE / "
    "M555_STAIRS_UP / M556_DOOR_STATE / M557_DOOR_THING_INDEX / "
    "M558_FLOOR_ORNAMENT_ORDINAL; DEFS.H:2599 M606=3; DEFS.H:2669 "
    "C0x0218_CELL_ORDER_DOORPASS1; DEFS.H:2672 C0x0349_DOORPASS2; "
    "DEFS.H:2676 C0x3421_CELL_ORDER_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT; "
    "DEFS.H:2696-2711 M587_VIEW_WALL_D1C_FRONT=14; DEFS.H:2746 "
    "M595_VIEW_FLOOR_D1C=7; DEFS.H:4045-4046 C705/C706; DEFS.H:4052 "
    "C712_ZONE_WALL_D1C.";

static const char s_disjointness_note[] =
    "Disjoint DM1 V1 D1C F0108 floor-ornament occlusion source-lock gate. "
    "It does not touch D0C/D0L/D0R/D2/D3 tests, D1C F0107 wall-ornament, "
    "D1C F0111 partly-open door, D1C F0115 / D1C stairs / D1C pit / "
    "D1C center-field / D1C wall composition, any D0C/D2C F0108 "
    "floor+ceiling+ornament test, or the D0C F0108 floor-ornament keepout "
    "test. It is asset-free and does not read GRAPHICS.DAT. It pins the "
    "BUG0_64 occlusion contract on the D1C square only (C02 pit, C05 "
    "teleporter, C19 stairs-front share the T0124017 F0108 call with the "
    "source BUG0_64 comment at DUNVIEW.C:7926). It does not modify any "
    "CSB/Nexus/Theron/DM2 source files.";

static const DM1_V1_D1CF0108FloorOrnamentOcclusionStepPc34
s_steps[DM1_V1_D1C_F0108_FOCCL_STEP_BUG0_64_OCCLUSION_GUARD_PC34 + 1] = {
    {
        DM1_V1_D1C_F0108_FOCCL_CONTEXT_CORRIDOR_PC34,
        0,
        0, 0, 0, 0, 0,
        DM1_CELL_ORDER_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT,
        0, DM1_D1C_FLOOR_ZONE_BASE, DM1_D1C_FLOOR_COORDINATE_SET, DM1_D1C_VIEW_FLOOR,
        "F0128 dispatches D1C after D1L/D1R and before D0L",
        "DUNVIEW.C:8318-8542 dispatch table at line 8536"
    },
    {
        DM1_V1_D1C_F0108_FOCCL_CONTEXT_OPEN_PIT_PC34,
        1,
        1, 1, 1, 0, 0,
        DM1_CELL_ORDER_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT,
        0, DM1_D1C_FLOOR_ZONE_BASE, DM1_D1C_FLOOR_COORDINATE_SET, DM1_D1C_VIEW_FLOOR,
        "F0124 T0124017 F0108 call at 7926 with M595_VIEW_FLOOR_D1C",
        "DUNVIEW.C:7926 F0108_DUNGEONVIEW_DrawFloorOrnament call"
    },
    {
        DM1_V1_D1C_F0108_FOCCL_CONTEXT_OPEN_PIT_PC34,
        2,
        0, 0, 1, 0, 0,
        DM1_CELL_ORDER_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT,
        0, DM1_D1C_FLOOR_ZONE_BASE, DM1_D1C_FLOOR_COORDINATE_SET, DM1_D1C_VIEW_FLOOR,
        "F0108 M558 ordinal decode at 3949-3964 with footprint mask",
        "DUNVIEW.C:3949-3964 M007_GET/P0118_ui_FloorOrnamentOrdinal"
    },
    {
        DM1_V1_D1C_F0108_FOCCL_CONTEXT_OPEN_PIT_PC34,
        3,
        0, 0, 1, 0, 0,
        DM1_CELL_ORDER_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT,
        0, DM1_D1C_FLOOR_ZONE_BASE, DM1_D1C_FLOOR_COORDINATE_SET, DM1_D1C_VIEW_FLOOR,
        "F0108 T0108005 MASK0x8000_FOOTPRINTS recursion",
        "DUNVIEW.C:4008 F0108_DUNGEONVIEW_DrawFloorOrnament self-recursion"
    },
    {
        DM1_V1_D1C_F0108_FOCCL_CONTEXT_OPEN_PIT_PC34,
        4,
        0, 0, 1, 0, 0,
        DM1_CELL_ORDER_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT,
        0, DM1_D1C_FLOOR_ZONE_BASE, DM1_D1C_FLOOR_COORDINATE_SET, DM1_D1C_VIEW_FLOOR,
        "F0108 F0791 C10 transparent blit at 3988-3993",
        "DUNVIEW.C:3988-3993 F0791_DUNGEONVIEW_DrawBitmapXX with C10"
    },
    {
        DM1_V1_D1C_F0108_FOCCL_CONTEXT_OPEN_PIT_PC34,
        5,
        0, 0, 0, 1, 0,
        DM1_CELL_ORDER_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT,
        0, DM1_D1C_FLOOR_ZONE_BASE, DM1_D1C_FLOOR_COORDINATE_SET, DM1_D1C_VIEW_FLOOR,
        "F0112 ceiling-pit at 7929 (C067 graphic / C866 or C868 zone)",
        "DUNVIEW.C:7929 F0112_DUNGEONVIEW_DrawCeilingPit"
    },
    {
        DM1_V1_D1C_F0108_FOCCL_CONTEXT_OPEN_PIT_PC34,
        6,
        0, 0, 0, 0, 1,
        DM1_CELL_ORDER_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT,
        1, DM1_D1C_FLOOR_ZONE_BASE, DM1_D1C_FLOOR_COORDINATE_SET, DM1_D1C_VIEW_FLOOR,
        "F0115 thing pass with C0x3421 cell order at 7937",
        "DUNVIEW.C:7937 F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF"
    },
    {
        DM1_V1_D1C_F0108_FOCCL_CONTEXT_OPEN_PIT_PC34,
        7,
        0, 0, 1, 0, 0,
        DM1_CELL_ORDER_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT,
        0, DM1_D1C_FLOOR_ZONE_BASE, DM1_D1C_FLOOR_COORDINATE_SET, DM1_D1C_VIEW_FLOOR,
        "BUG0_64 occlusion guard absent (no pit-open check before F0108)",
        "DUNVIEW.C:7926 source comment BUG0_64"
    }
};

static DM1_V1_D1CF0108FloorOrnamentOcclusionSelfTestResultPc34 s_last_self_test;

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

uint8_t dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_blend_c10_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel)
{
    return source_pixel == DM1_C10_COLOR_FLESH ? destination_pixel : source_pixel;
}

unsigned int dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_decode_ordinal_pc34(
    unsigned int floor_ornament_ordinal,
    bool *footprint_flag_set,
    unsigned int *cleared_ordinal,
    bool *primary_draws,
    int *primary_index,
    bool *recursive_footprints_draw,
    int *recursive_footprints_index)
{
    unsigned int cleared = 0u;
    bool has_ordinal = floor_ornament_ordinal != 0u;
    bool fp_set = false;
    bool recurse_fp = false;

    if (footprint_flag_set) *footprint_flag_set = false;
    if (cleared_ordinal) *cleared_ordinal = 0u;
    if (primary_draws) *primary_draws = false;
    if (primary_index) *primary_index = -1;
    if (recursive_footprints_draw) *recursive_footprints_draw = false;
    if (recursive_footprints_index) *recursive_footprints_index = -1;
    if (!has_ordinal) return 0u;

    fp_set = (floor_ornament_ordinal &
              DM1_V1_D1C_F0108_FOCCL_FOOTPRINT_MASK_PC34) != 0u;
    cleared = floor_ornament_ordinal &
        ~DM1_V1_D1C_F0108_FOCCL_FOOTPRINT_MASK_PC34;
    recurse_fp = fp_set && cleared == 0u;

    if (footprint_flag_set) *footprint_flag_set = fp_set;
    if (cleared_ordinal) *cleared_ordinal = cleared;
    if (primary_draws) *primary_draws = !fp_set || cleared != 0u;
    if (primary_index && (!fp_set || cleared != 0u)) {
        *primary_index = (int)((fp_set ? cleared : floor_ornament_ordinal) - 1u);
    }
    if (recursive_footprints_draw) *recursive_footprints_draw = recurse_fp;
    if (recursive_footprints_index && recurse_fp) {
        *recursive_footprints_index = DM1_FLOOR_ORNAMENT_FOOTPRINTS;
    }
    return floor_ornament_ordinal;
}

bool dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_context_occludes_pc34(
    DM1_V1_D1CF0108FloorOrnamentOcclusionContextPc34 context,
    unsigned int floor_ornament_ordinal)
{
    bool fp_set = false;
    unsigned int cleared = 0u;
    bool primary_draws = false;

    if (floor_ornament_ordinal == 0u) return false;
    if (!dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_decode_ordinal_pc34(
            floor_ornament_ordinal, &fp_set, &cleared, &primary_draws,
            NULL, NULL, NULL)) {
        return false;
    }
    if (!primary_draws && !fp_set) return false;
    switch (context) {
    case DM1_V1_D1C_F0108_FOCCL_CONTEXT_CORRIDOR_PC34:
    case DM1_V1_D1C_F0108_FOCCL_CONTEXT_DOOR_FRONT_PC34:
        return true;
    case DM1_V1_D1C_F0108_FOCCL_CONTEXT_OPEN_PIT_PC34:
    case DM1_V1_D1C_F0108_FOCCL_CONTEXT_TELEPORTER_PC34:
    case DM1_V1_D1C_F0108_FOCCL_CONTEXT_STAIRS_FRONT_PC34:
        /* BUG0_64: F0108 always blits over pit / teleporter / stairs-front
         * with no occlusion guard. The cell_order is C0x3421 for the
         * T0124017 shared tail. */
        return true;
    }
    return false;
}

bool dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_context_has_bug0_64_pc34(
    DM1_V1_D1CF0108FloorOrnamentOcclusionContextPc34 context,
    unsigned int floor_ornament_ordinal)
{
    if (!dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_context_occludes_pc34(
            context, floor_ornament_ordinal)) {
        return false;
    }

    switch (context) {
    case DM1_V1_D1C_F0108_FOCCL_CONTEXT_OPEN_PIT_PC34:
    case DM1_V1_D1C_F0108_FOCCL_CONTEXT_TELEPORTER_PC34:
    case DM1_V1_D1C_F0108_FOCCL_CONTEXT_STAIRS_FRONT_PC34:
        /* ReDMCSB: DUNVIEW.C F0124 lines 7912-7937.  C02 pit,
         * C05 teleporter, and C19 stairs-front reach T0124017, where
         * F0108 is called with the BUG0_64 no-occlusion-guard comment.
         * C01 corridor and C17 door-front also draw floor ornaments, but
         * they are not the open-surface BUG0_64 case. */
        return true;
    case DM1_V1_D1C_F0108_FOCCL_CONTEXT_CORRIDOR_PC34:
    case DM1_V1_D1C_F0108_FOCCL_CONTEXT_DOOR_FRONT_PC34:
        return false;
    }
    return false;
}

int dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_zone_d1c_pc34(
    int coordinate_set,
    int view_floor)
{
    if (coordinate_set < 0) coordinate_set = 0;
    if (view_floor < 0) view_floor = 0;
    return DM1_D1C_FLOOR_ZONE_BASE +
        coordinate_set * DM1_D1C_FLOOR_ZONE_STRIDE_PC34 + view_floor;
}

static void fill_model(DM1_V1_D1CF0108FloorOrnamentOcclusionModelPc34 *m)
{
    m->view_square_d1c = DM1_D1C_VIEW_SQUARE;
    m->view_floor_d1c = DM1_D1C_VIEW_FLOOR;
    m->view_wall_d1c_front = DM1_D1C_VIEW_WALL_FRONT;
    m->wall_zone_d1c = DM1_D1C_WALL_ZONE;
    m->c10_transparent_color = DM1_C10_COLOR_FLESH;
    m->floor_ornament_ordinal_slot = DM1_M558_FLOOR_ORNAMENT_ORDINAL_SLOT;
    m->first_thing_slot = DM1_M550_FIRST_THING_SLOT;
    m->pit_or_teleporter_visible_slot = DM1_M554_PIT_OR_TELEPORTER_VISIBLE_SLOT;
    m->stairs_up_slot = DM1_M555_STAIRS_UP_SLOT;
    m->door_thing_index_slot = DM1_M557_DOOR_THING_INDEX_SLOT;
    m->door_state_slot = DM1_M556_DOOR_STATE_SLOT;
    m->front_wall_ornament_slot = DM1_M552_FRONT_WALL_ORNAMENT_SLOT;
    m->f0128_dispatch_order_d1c = DM1_D1C_F0128_DISPATCH_ORDER;
    m->f0128_dispatch_after_d1l = 1;
    m->f0128_dispatch_after_d1r = 1;
    m->f0128_dispatch_before_d0l = 1;
    m->f0124_door_front_calls_f0108 = 1;
    m->f0124_door_front_calls_f0115 = 1;
    m->f0124_door_front_cell_order = DM1_CELL_ORDER_DOORPASS1;
    m->f0124_corridor_calls_f0108 = 1;
    m->f0124_corridor_calls_f0112 = 1;
    m->f0124_corridor_calls_f0115 = 1;
    m->f0124_corridor_cell_order =
        DM1_CELL_ORDER_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT;
    m->f0124_pit_f0104_fires_first = 1;
    m->f0124_pit_f0108_occludes_pit = 1;
    m->f0124_teleporter_f0108_occludes_teleporter = 1;
    m->f0124_stairs_front_f0104_fires_first = 1;
    m->f0124_stairs_front_f0108_occludes_stairs = 1;
    m->f0108_ordinal_zero_skips_blit = 1;
    m->f0108_footprint_mask_recurses = 1;
    m->f0108_footprint_only_skips_primary = 1;
    m->f0108_blit_uses_c10_transparent = 1;
    m->f0108_d1c_in_horizontal_flip_branch = 1;
    m->f0108_d1c_zone_uses_11_stride = 1;
    m->f0108_d1c_zone_uses_9_stride = 0;
    m->f0108_zone_d1c = DM1_D1C_FLOOR_ZONE;
    m->bug0_64_occlusion_guard = 0;
    m->f0112_ceiling_pit_graphic = DM1_D1C_CEILING_PIT_GRAPHIC;
    m->f0112_ceiling_pit_zone_d1c = DM1_D1C_CEILING_PIT_ZONE;
    m->no_graphics_dat_reads = 1;
    m->source_locked_contract_only = 1;
    m->no_real_asset_bitmap_parity = 1;
    m->source_evidence = s_source_evidence;
    m->disjointness_note = s_disjointness_note;
}

bool dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_default_model_builder_pc34(
    DM1_V1_D1CF0108FloorOrnamentOcclusionModelPc34 *out_model)
{
    if (!out_model) return false;
    memset(out_model, 0, sizeof(*out_model));
    fill_model(out_model);
    out_model->deterministic_hash =
        dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_hash_model_pc34(
            out_model);
    return true;
}

uint32_t dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_hash_model_pc34(
    const DM1_V1_D1CF0108FloorOrnamentOcclusionModelPc34 *model)
{
    uint32_t h = 2166136261u;
    size_t i;

    if (!model) return 0u;
    h = fnv1a_u32(h, (uint32_t)model->view_square_d1c);
    h = fnv1a_u32(h, (uint32_t)model->view_floor_d1c);
    h = fnv1a_u32(h, (uint32_t)model->view_wall_d1c_front);
    h = fnv1a_u32(h, (uint32_t)model->wall_zone_d1c);
    h = fnv1a_u32(h, (uint32_t)model->c10_transparent_color);
    h = fnv1a_u32(h, (uint32_t)model->floor_ornament_ordinal_slot);
    h = fnv1a_u32(h, (uint32_t)model->first_thing_slot);
    h = fnv1a_u32(h, (uint32_t)model->pit_or_teleporter_visible_slot);
    h = fnv1a_u32(h, (uint32_t)model->stairs_up_slot);
    h = fnv1a_u32(h, (uint32_t)model->door_thing_index_slot);
    h = fnv1a_u32(h, (uint32_t)model->door_state_slot);
    h = fnv1a_u32(h, (uint32_t)model->front_wall_ornament_slot);
    h = fnv1a_u32(h, (uint32_t)model->f0124_door_front_calls_f0108);
    h = fnv1a_u32(h, (uint32_t)model->f0124_door_front_calls_f0115);
    h = fnv1a_u32(h, (uint32_t)model->f0124_door_front_cell_order);
    h = fnv1a_u32(h, (uint32_t)model->f0124_corridor_calls_f0108);
    h = fnv1a_u32(h, (uint32_t)model->f0124_corridor_calls_f0112);
    h = fnv1a_u32(h, (uint32_t)model->f0124_corridor_calls_f0115);
    h = fnv1a_u32(h, (uint32_t)model->f0124_corridor_cell_order);
    h = fnv1a_u32(h, (uint32_t)model->f0124_pit_f0104_fires_first);
    h = fnv1a_u32(h, (uint32_t)model->f0124_pit_f0108_occludes_pit);
    h = fnv1a_u32(h, (uint32_t)model->f0124_teleporter_f0108_occludes_teleporter);
    h = fnv1a_u32(h, (uint32_t)model->f0124_stairs_front_f0104_fires_first);
    h = fnv1a_u32(h, (uint32_t)model->f0124_stairs_front_f0108_occludes_stairs);
    h = fnv1a_u32(h, (uint32_t)model->f0108_ordinal_zero_skips_blit);
    h = fnv1a_u32(h, (uint32_t)model->f0108_footprint_mask_recurses);
    h = fnv1a_u32(h, (uint32_t)model->f0108_footprint_only_skips_primary);
    h = fnv1a_u32(h, (uint32_t)model->f0108_blit_uses_c10_transparent);
    h = fnv1a_u32(h, (uint32_t)model->f0108_d1c_in_horizontal_flip_branch);
    h = fnv1a_u32(h, (uint32_t)model->f0108_d1c_zone_uses_11_stride);
    h = fnv1a_u32(h, (uint32_t)model->f0108_d1c_zone_uses_9_stride);
    h = fnv1a_u32(h, (uint32_t)model->f0108_zone_d1c);
    h = fnv1a_u32(h, (uint32_t)model->bug0_64_occlusion_guard);
    h = fnv1a_u32(h, (uint32_t)model->f0112_ceiling_pit_graphic);
    h = fnv1a_u32(h, (uint32_t)model->f0112_ceiling_pit_zone_d1c);
    for (i = 0; i < sizeof(s_steps) / sizeof(s_steps[0]); ++i) {
        h = fnv1a_u32(h, (uint32_t)s_steps[i].context);
        h = fnv1a_u32(h, (uint32_t)s_steps[i].calls_f0108);
        h = fnv1a_u32(h, (uint32_t)s_steps[i].f0108_occludes_cell);
        h = fnv1a_u32(h, (uint32_t)s_steps[i].bug0_64_occlusion_present);
        h = fnv1a_u32(h, (uint32_t)s_steps[i].calls_f0112);
        h = fnv1a_u32(h, (uint32_t)s_steps[i].calls_f0115);
        h = fnv1a_u32(h, (uint32_t)s_steps[i].expected_cell_order);
        h = fnv1a_u32(h, (uint32_t)s_steps[i].expected_thing_pass);
        h = fnv1a_u32(h, (uint32_t)s_steps[i].expected_view_floor);
    }
    return h;
}

const DM1_V1_D1CF0108FloorOrnamentOcclusionModelPc34 *
dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_default_model_pc34(void)
{
    static DM1_V1_D1CF0108FloorOrnamentOcclusionModelPc34 s_model;
    static int s_init;

    if (!s_init) {
        dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_default_model_builder_pc34(
            &s_model);
        s_init = 1;
    }
    return &s_model;
}

uint32_t dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_deterministic_hash_pc34(void)
{
    const DM1_V1_D1CF0108FloorOrnamentOcclusionModelPc34 *model =
        dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_default_model_pc34();
    return model ? model->deterministic_hash : 0u;
}

unsigned int dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_context_count_pc34(void)
{
    return (unsigned int)(sizeof(s_steps) / sizeof(s_steps[0]));
}

const DM1_V1_D1CF0108FloorOrnamentOcclusionStepPc34 *
dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_step_at_pc34(size_t index)
{
    if (index >= sizeof(s_steps) / sizeof(s_steps[0])) return NULL;
    return &s_steps[index];
}

const char *dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const char *
dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_disjointness_note_pc34(void)
{
    return s_disjointness_note;
}

static void self_check(
    int condition,
    DM1_V1_D1CF0108FloorOrnamentOcclusionSelfTestResultPc34 *result)
{
    ++result->assertions;
    if (!condition) ++result->failures;
}

int dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_self_test_pc34(void)
{
    DM1_V1_D1CF0108FloorOrnamentOcclusionModelPc34 built;
    const DM1_V1_D1CF0108FloorOrnamentOcclusionModelPc34 *model;
    bool fp_set;
    unsigned int cleared;
    bool primary_draws;
    bool recurse_fp;
    int primary_index;
    int recurse_index;
    unsigned int n;
    unsigned int i;
    int bug0_64_count = 0;

    memset(&s_last_self_test, 0, sizeof(s_last_self_test));
    s_last_self_test.deterministic_hash = 2166136261u;

    s_last_self_test.model_builder_ok =
        dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_default_model_builder_pc34(
            &built) ? 1 : 0;
    model = dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_default_model_pc34();
    self_check(s_last_self_test.model_builder_ok == 1, &s_last_self_test);
    self_check(model != NULL, &s_last_self_test);
    self_check(
        dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_default_model_builder_pc34(
            NULL) == 0,
        &s_last_self_test);
    self_check(
        dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_hash_model_pc34(
            NULL) == 0u,
        &s_last_self_test);
    if (model) {
        s_last_self_test.hash_stable =
            built.deterministic_hash == model->deterministic_hash &&
            dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_hash_model_pc34(
                model) == model->deterministic_hash &&
            dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_deterministic_hash_pc34() ==
                model->deterministic_hash;
        self_check(s_last_self_test.hash_stable == 1, &s_last_self_test);
        self_check(model->view_square_d1c == DM1_D1C_VIEW_SQUARE, &s_last_self_test);
        self_check(model->view_floor_d1c == DM1_D1C_VIEW_FLOOR, &s_last_self_test);
        self_check(model->view_wall_d1c_front == DM1_D1C_VIEW_WALL_FRONT,
                   &s_last_self_test);
        self_check(model->wall_zone_d1c == DM1_D1C_WALL_ZONE, &s_last_self_test);
        self_check(model->c10_transparent_color == DM1_C10_COLOR_FLESH,
                   &s_last_self_test);
        self_check(model->f0108_zone_d1c == DM1_D1C_FLOOR_ZONE, &s_last_self_test);
        self_check(model->bug0_64_occlusion_guard == 0, &s_last_self_test);
        self_check(model->no_graphics_dat_reads == 1, &s_last_self_test);
        self_check(model->source_locked_contract_only == 1, &s_last_self_test);
        self_check(model->no_real_asset_bitmap_parity == 1, &s_last_self_test);
        s_last_self_test.deterministic_hash =
            fnv1a_u32(s_last_self_test.deterministic_hash,
                      model->deterministic_hash);
    }

    fp_set = false;
    cleared = 0u;
    primary_draws = false;
    recurse_fp = false;
    primary_index = -1;
    recurse_index = -1;
    dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_decode_ordinal_pc34(
        7u, &fp_set, &cleared, &primary_draws, &primary_index, &recurse_fp,
        &recurse_index);
    s_last_self_test.decode_simple_primary =
        !fp_set && cleared == 7u && primary_draws && primary_index == 6 &&
        !recurse_fp && recurse_index == -1;
    self_check(s_last_self_test.decode_simple_primary == 1, &s_last_self_test);

    fp_set = false;
    cleared = 0xffffffffu;
    primary_draws = true;
    recurse_fp = false;
    primary_index = -1;
    recurse_index = -1;
    dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_decode_ordinal_pc34(
        0x8000u, &fp_set, &cleared, &primary_draws, &primary_index, &recurse_fp,
        &recurse_index);
    s_last_self_test.decode_fp_only_recurses =
        fp_set && cleared == 0u && !primary_draws && primary_index == -1 &&
        recurse_fp && recurse_index == DM1_FLOOR_ORNAMENT_FOOTPRINTS;
    self_check(s_last_self_test.decode_fp_only_recurses == 1, &s_last_self_test);

    fp_set = false;
    cleared = 0u;
    primary_draws = false;
    recurse_fp = true;
    primary_index = -1;
    recurse_index = -1;
    dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_decode_ordinal_pc34(
        0x8007u, &fp_set, &cleared, &primary_draws, &primary_index, &recurse_fp,
        &recurse_index);
    s_last_self_test.decode_fp_with_primary_both =
        fp_set && cleared == 7u && primary_draws && primary_index == 6 &&
        !recurse_fp && recurse_index == -1;
    self_check(s_last_self_test.decode_fp_with_primary_both == 1,
               &s_last_self_test);

    fp_set = true;
    cleared = 0xffffffffu;
    primary_draws = true;
    dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_decode_ordinal_pc34(
        0u, &fp_set, &cleared, &primary_draws, NULL, NULL, NULL);
    s_last_self_test.decode_zero_skips_blit =
        !fp_set && cleared == 0u && !primary_draws;
    self_check(s_last_self_test.decode_zero_skips_blit == 1, &s_last_self_test);

    s_last_self_test.context_occlusion_paths =
        (dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_context_occludes_pc34(
             DM1_V1_D1C_F0108_FOCCL_CONTEXT_CORRIDOR_PC34, 5u) ? 1 : 0) +
        (dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_context_occludes_pc34(
             DM1_V1_D1C_F0108_FOCCL_CONTEXT_OPEN_PIT_PC34, 5u) ? 1 : 0) +
        (dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_context_occludes_pc34(
             DM1_V1_D1C_F0108_FOCCL_CONTEXT_TELEPORTER_PC34, 5u) ? 1 : 0) +
        (dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_context_occludes_pc34(
             DM1_V1_D1C_F0108_FOCCL_CONTEXT_STAIRS_FRONT_PC34, 5u) ? 1 : 0) +
        (dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_context_occludes_pc34(
             DM1_V1_D1C_F0108_FOCCL_CONTEXT_DOOR_FRONT_PC34, 5u) ? 1 : 0);
    self_check(s_last_self_test.context_occlusion_paths == 5, &s_last_self_test);
    s_last_self_test.context_zero_ordinal_no_occlusion =
        !dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_context_occludes_pc34(
            DM1_V1_D1C_F0108_FOCCL_CONTEXT_OPEN_PIT_PC34, 0u);
    self_check(s_last_self_test.context_zero_ordinal_no_occlusion == 1,
               &s_last_self_test);
    self_check(
        !dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_context_occludes_pc34(
            (DM1_V1_D1CF0108FloorOrnamentOcclusionContextPc34)99, 5u),
        &s_last_self_test);

    s_last_self_test.blend_c10_preserves_destination =
        dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_blend_c10_pc34(
            0xaau, 10u) == 0xaau;
    self_check(s_last_self_test.blend_c10_preserves_destination == 1,
               &s_last_self_test);
    s_last_self_test.blend_opaque_writes_source =
        dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_blend_c10_pc34(
            0xaau, 0x52u) == 0x52u;
    self_check(s_last_self_test.blend_opaque_writes_source == 1,
               &s_last_self_test);
    s_last_self_test.zone_d1c_stride_11 =
        dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_zone_d1c_pc34(
            1, 7) == DM1_D1C_FLOOR_ZONE &&
        dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_zone_d1c_pc34(
            2, 7) == DM1_D1C_FLOOR_ZONE_BASE + 2 * DM1_D1C_FLOOR_ZONE_STRIDE_PC34 + 7 &&
        dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_zone_d1c_pc34(
            -1, -1) == DM1_D1C_FLOOR_ZONE_BASE;
    self_check(s_last_self_test.zone_d1c_stride_11 == 1, &s_last_self_test);

    n = dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_context_count_pc34();
    s_last_self_test.step_count_eight = (n == sizeof(s_steps) / sizeof(s_steps[0]) &&
                                         n == 8u);
    self_check(s_last_self_test.step_count_eight == 1, &s_last_self_test);
    self_check(
        dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_step_at_pc34(n) == NULL,
        &s_last_self_test);
    for (i = 0; i < n; ++i) {
        const DM1_V1_D1CF0108FloorOrnamentOcclusionStepPc34 *step =
            dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_step_at_pc34(i);
        self_check(step != NULL, &s_last_self_test);
        if (!step) continue;
        self_check(step->order_index == (int)i, &s_last_self_test);
        self_check(step->redmcsb_anchor &&
                       strstr(step->redmcsb_anchor, "DUNVIEW.C") != NULL,
                   &s_last_self_test);
        if (step->bug0_64_occlusion_present) ++bug0_64_count;
        s_last_self_test.deterministic_hash =
            fnv1a_u32(s_last_self_test.deterministic_hash,
                      (uint32_t)step->expected_cell_order);
    }
    s_last_self_test.bug0_64_marker_count = bug0_64_count;
    self_check(s_last_self_test.bug0_64_marker_count == 5, &s_last_self_test);

    s_last_self_test.source_evidence_present =
        strstr(s_source_evidence, "F0108:3940-4011") != NULL &&
        strstr(s_source_evidence, "F0124:7727-7924") != NULL &&
        strstr(s_source_evidence, "BUG0_64") != NULL &&
        strstr(s_source_evidence, "C1500") != NULL &&
        strstr(s_source_evidence, "C10_COLOR_FLESH") != NULL;
    self_check(s_last_self_test.source_evidence_present == 1, &s_last_self_test);
    s_last_self_test.disjointness_note_present =
        strstr(s_disjointness_note, "D1C F0108") != NULL &&
        strstr(s_disjointness_note, "D0C F0108") != NULL &&
        strstr(s_disjointness_note, "GRAPHICS.DAT") != NULL &&
        strstr(s_disjointness_note, "CSB/Nexus/Theron/DM2") != NULL;
    self_check(s_last_self_test.disjointness_note_present == 1,
               &s_last_self_test);
    self_check(s_last_self_test.assertions >= 40, &s_last_self_test);
    self_check(s_last_self_test.deterministic_hash != 2166136261u,
               &s_last_self_test);

    s_last_self_test.ok = s_last_self_test.failures == 0;
    return s_last_self_test.ok;
}

const DM1_V1_D1CF0108FloorOrnamentOcclusionSelfTestResultPc34 *
dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_last_self_test_result_pc34(void)
{
    return &s_last_self_test;
}
