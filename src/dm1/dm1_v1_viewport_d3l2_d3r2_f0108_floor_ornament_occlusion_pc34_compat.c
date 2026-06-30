/*
 * ReDMCSB anchors: DUNVIEW.C F0676:6226-6290 D3L2 dispatch body,
 * F0677:6293-6357 D3R2 dispatch body, F0108:3940-4011 floor-ornament
 * ordinal / MASK0x8000_FOOTPRINTS recursion / C10 transparent blit /
 * C1500 + CoordinateSet*11 + ViewFloor zone math, F0128:8318-8542
 * dispatch order with D3C follow-up F0676/F0677, F0115 thing pass with
 * C0x0218_DOORPASS1 / C0x0321_SIDE / C0x0349_DOORPASS2 / C0x0412_SIDE /
 * C0x0439_DOORPASS2 / C0x3421_OPEN / C0x4312_OPEN cell orders,
 * F0104 floor-pit / stairs bitmap (only on the C02 case behind the
 * !M554_PIT_OR_TELEPORTER_VISIBLE guard), DEFS.H:2088 C10_COLOR_FLESH,
 * 2533-2559 M550/M551/M552/M553/M554/M555/M556/M557/M558, 2596-2611
 * M610_VIEW_SQUARE_D3L2=14, M611_VIEW_SQUARE_D3R2=15, 2668-2677 cell
 * orders, 4139-4153 C702/C703 wall zones for D3L2/D3R2, 4205-4207
 * C800/C801 stairs-up-front D3L2/D3R2 zones.
 */
#include "firestaff/dm1/v1/viewport/dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_pc34_compat.h"

#include <string.h>

enum {
    DM1_M550_FIRST_THING_SLOT = 2,
    DM1_M552_FRONT_WALL_ORNAMENT_SLOT = 5,
    DM1_M554_PIT_OR_TELEPORTER_VISIBLE_SLOT = 3,
    DM1_M555_STAIRS_UP_SLOT = 4,
    DM1_M556_DOOR_STATE_SLOT = 7,
    DM1_M557_DOOR_THING_INDEX_SLOT = 8,
    DM1_M558_FLOOR_ORNAMENT_ORDINAL_SLOT = 5,
    DM1_C10_COLOR_FLESH = 10,
    DM1_D3L2_VIEW_SQUARE = 14,
    DM1_D3R2_VIEW_SQUARE = 15,
    DM1_D3L2_VIEW_FLOOR = 0,
    DM1_D3R2_VIEW_FLOOR = 1,
    DM1_D3L2_WALL_ZONE = 702,
    DM1_D3R2_WALL_ZONE = 703,
    DM1_FLOOR_ZONE_BASE = 1500,
    DM1_FLOOR_ZONE_STRIDE = 11,
    DM1_FLOOR_ZONE_D3L2 = DM1_FLOOR_ZONE_BASE + 0 * DM1_FLOOR_ZONE_STRIDE,
    DM1_FLOOR_ZONE_D3R2 = DM1_FLOOR_ZONE_BASE + 0 * DM1_FLOOR_ZONE_STRIDE + 1,
    DM1_FLOOR_ORNAMENT_FOOTPRINTS = 15
};

static const char s_source_evidence[] =
    "ReDMCSB source-lock for DM1 V1 D3L2/D3R2 F0108 floor-ornament "
    "occlusion gate. DUNVIEW.C F0676:6226-6290 dispatches D3L2. The "
    "C17_ELEMENT_DOOR_FRONT case at 6268 calls "
    "F0108_DUNGEONVIEW_DrawFloorOrnament with "
    "M552_FRONT_WALL_ORNAMENT_ORDINAL and C00_VIEW_FLOOR_D3L2 at line "
    "6270, then F0115 with C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT "
    "and C14_VIEW_SQUARE_D3L2, then F0111 with the D3L2 door pair, then "
    "drops to C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT via "
    "T0676017. The shared C02_ELEMENT_PIT (when not M554_PIT_OR_"
    "TELEPORTER_VISIBLE) draws F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap "
    "with C049_GRAPHIC_FLOOR_PIT_D3L2 / C850_ZONE_FLOORPIT_D3L2 before "
    "falling through to C05_ELEMENT_TELEPORTER and C01_ELEMENT_CORRIDOR. "
    "The shared-tail path calls F0108 with M558_FLOOR_ORNAMENT_ORDINAL "
    "and C00_VIEW_FLOOR_D3L2 at line 6284 with the source comment "
    "BUG0_64 (Floor ornaments are drawn over open pits). The C18/C16 "
    "door-side/stairs-side path uses C0x0321_CELL_ORDER_BACKLEFT_BACKRIGHT_"
    "FRONTRIGHT. DUNVIEW.C F0677:6293-6357 mirrors the D3R2 dispatch body "
    "with the same BUG0_64 comment at line 6351 and C01_VIEW_FLOOR_D3R2. "
    "F0108:3940-4011 is the shared floor-ornament ordinal handler with "
    "MASK0x8000_FOOTPRINTS recursion at T0108005, C10_COLOR_FLESH "
    "transparent blit, and PC 3.4 C1500 + CoordinateSet*11 + ViewFloor "
    "zone math at 3998/4004. F0128:8318-8542 dispatches D3C then calls "
    "F0676 (D3L2) and F0677 (D3R2) in order before the rest of the "
    "far-to-near pass. DEFS.H:2088 C10_COLOR_FLESH; DEFS.H:2550 "
    "M550_FIRST_THING; DEFS.H:2554 M554_PIT_OR_TELEPORTER_VISIBLE; "
    "DEFS.H:2555 M555_STAIRS_UP; DEFS.H:2556 M556_DOOR_STATE; DEFS.H:2557 "
    "M557_DOOR_THING_INDEX; DEFS.H:2558 M558_FLOOR_ORNAMENT_ORDINAL; "
    "DEFS.H:2668-2677 cell orders (C0x0218_DOORPASS1, C0x0321_SIDE, "
    "C0x0349_DOORPASS2, C0x0412_SIDE, C0x0439_DOORPASS2, C0x3421_OPEN, "
    "C0x4312_OPEN); DEFS.H:4045-4046 C705/C706 (D3L/D3R walls, not used "
    "here); DEFS.H:4139-4153 C702_ZONE_WALL_D3L2 / C703_ZONE_WALL_D3R2; "
    "DEFS.H:4205-4207 C800/C801 stairs-up-front zones for D3L2/D3R2.";

static const char s_disjointness_note[] =
    "Disjoint DM1 V1 D3L2/D3R2 F0108 floor-ornament occlusion gate. It "
    "does not touch the D1C F0108 floor-ornament occlusion sibling (which "
    "pins BUG0_64 on the D1C front-square column-center only), the "
    "F0676/F0677 thing-pass sibling, the F0108 floor+ceiling+ornament "
    "sibling, the F0108 wall-composition sibling, the D3L2/D3R2 wall "
    "sibling, the stairs/pit dispatch sibling, the F0111 door-front-pair "
    "sibling, the D1L/D1R door-frame sibling, the D3L/D3R sibling, the "
    "D0C floor-ornament keepout, the F0111 partly-open door family, the "
    "D2L/D2R F0098 fallback, the D2C F0108 floor+ceiling+ornament, the "
    "D0C F0108 floor-ornament, the CSB/Nexus/Theron/DM2 lanes, the F0098 "
    "floor+ceiling fallback, the F0095 floor-ornament aggregate, the "
    "F0107 wall-ornament alcove helper, or any other PC34 viewport "
    "ornament sibling. It is asset-free and does not read GRAPHICS.DAT. "
    "It pins the BUG0_64 occlusion contract on the D3L2 and D3R2 lanes "
    "only (F0676 corridor/pit/teleporter/stairs-side shared tail at line "
    "6284 and F0677 corridor/pit/teleporter/stairs-side shared tail at "
    "line 6351 both carry the source BUG0_64 comment). It does not "
    "modify any CSB/Nexus/Theron/DM2 source files.";

static const DM1_V1_D3L2D3R2F0108FloorOrnamentOcclusionStepPc34
s_steps[DM1_V1_D3L2_D3R2_FOCCL_STEP_F0108_C10_BLIT_PC34 + 1] = {
    {
        DM1_V1_D3L2_D3R2_FOCCL_CONTEXT_DOOR_FRONT_PC34,
        0,
        0, 0, 0, 0, 0,
        DM1_V1_D3L2_D3R2_FOCCL_CELL_ORDER_DOORPASS1_BLBR_PC34,
        DM1_D3L2_VIEW_FLOOR,
        DM1_D3L2_WALL_ZONE,
        "F0676/F0677 dispatch sequence after D3C and before D2L/D2R",
        "DUNVIEW.C F0128:8318-8542 F0676/F0677 dispatch hooks"
    },
    {
        DM1_V1_D3L2_D3R2_FOCCL_CONTEXT_DOOR_FRONT_PC34,
        1,
        1, 1, 0, 1,
        DM1_V1_D3L2_D3R2_FOCCL_CELL_ORDER_DOORPASS1_BLBR_PC34,
        DM1_D3L2_VIEW_FLOOR,
        DM1_D3L2_WALL_ZONE,
        "F0676 C17 door-front F0108 with M552 then F0115 with C0x0218",
        "DUNVIEW.C F0676:6268-6275 (C17_ELEMENT_DOOR_FRONT case)"
    },
    {
        DM1_V1_D3L2_D3R2_FOCCL_CONTEXT_CORRIDOR_PC34,
        2,
        1, 1, 1, 1,
        DM1_V1_D3L2_D3R2_FOCCL_CELL_ORDER_OPEN_BLBR_FLFR_D3L2_PC34,
        DM1_D3L2_VIEW_FLOOR,
        DM1_D3L2_WALL_ZONE,
        "F0676 shared tail F0108 with M558 + F0115 with C0x3421",
        "DUNVIEW.C F0676:6276-6286 T0676015 corridor shared tail (BUG0_64)"
    },
    {
        DM1_V1_D3L2_D3R2_FOCCL_CONTEXT_OPEN_PIT_PC34,
        3,
        1, 1, 1, 0,
        DM1_V1_D3L2_D3R2_FOCCL_CELL_ORDER_OPEN_BLBR_FLFR_D3L2_PC34,
        DM1_D3L2_VIEW_FLOOR,
        DM1_D3L2_WALL_ZONE,
        "F0676 BUG0_64 F0108 over open pit (D3L2)",
        "DUNVIEW.C F0676:6284 BUG0_64 F0108 over open pit"
    },
    {
        DM1_V1_D3L2_D3R2_FOCCL_CONTEXT_DOOR_FRONT_PC34,
        4,
        1, 1, 0, 1,
        DM1_V1_D3L2_D3R2_FOCCL_CELL_ORDER_DOORPASS1_BLBR_PC34,
        DM1_D3R2_VIEW_FLOOR,
        DM1_D3R2_WALL_ZONE,
        "F0677 C17 door-front F0108 with M558 then F0115 with C0x0128",
        "DUNVIEW.C F0677:6333-6341 (C17_ELEMENT_DOOR_FRONT case)"
    },
    {
        DM1_V1_D3L2_D3R2_FOCCL_CONTEXT_CORRIDOR_PC34,
        5,
        1, 1, 1, 1,
        DM1_V1_D3L2_D3R2_FOCCL_CELL_ORDER_OPEN_BRBL_FRFL_D3R2_PC34,
        DM1_D3R2_VIEW_FLOOR,
        DM1_D3R2_WALL_ZONE,
        "F0677 shared tail F0108 with M558 + F0115 with C0x4312",
        "DUNVIEW.C F0677:6342-6353 T0677016 corridor shared tail (BUG0_64)"
    },
    {
        DM1_V1_D3L2_D3R2_FOCCL_CONTEXT_OPEN_PIT_PC34,
        6,
        1, 1, 1, 0,
        DM1_V1_D3L2_D3R2_FOCCL_CELL_ORDER_OPEN_BRBL_FRFL_D3R2_PC34,
        DM1_D3R2_VIEW_FLOOR,
        DM1_D3R2_WALL_ZONE,
        "F0677 BUG0_64 F0108 over open pit (D3R2)",
        "DUNVIEW.C F0677:6351 BUG0_64 F0108 over open pit"
    },
    {
        DM1_V1_D3L2_D3R2_FOCCL_CONTEXT_CORRIDOR_PC34,
        7,
        0, 0, 0, 0,
        DM1_V1_D3L2_D3R2_FOCCL_CELL_ORDER_OPEN_BLBR_FLFR_D3L2_PC34,
        DM1_D3L2_VIEW_FLOOR,
        DM1_D3L2_WALL_ZONE,
        "F0108 M558 ordinal decode at 3949-3964 with footprint mask",
        "DUNVIEW.C F0108:3949-3964 M007_GET/P0118 ordinal handler"
    },
    {
        DM1_V1_D3L2_D3R2_FOCCL_CONTEXT_CORRIDOR_PC34,
        8,
        0, 0, 0, 0,
        DM1_V1_D3L2_D3R2_FOCCL_CELL_ORDER_OPEN_BLBR_FLFR_D3L2_PC34,
        DM1_D3L2_VIEW_FLOOR,
        DM1_D3L2_WALL_ZONE,
        "F0108 T0108005 MASK0x8000_FOOTPRINTS recursion",
        "DUNVIEW.C F0108:4008 T0108005 footprint recursion"
    },
    {
        DM1_V1_D3L2_D3R2_FOCCL_CONTEXT_CORRIDOR_PC34,
        9,
        0, 0, 1, 0,
        DM1_V1_D3L2_D3R2_FOCCL_CELL_ORDER_OPEN_BLBR_FLFR_D3L2_PC34,
        DM1_D3L2_VIEW_FLOOR,
        DM1_D3L2_WALL_ZONE,
        "F0108 F0791 C10 transparent blit at 3988-3993",
        "DUNVIEW.C F0108:3988-3993 F0791 C10 transparent blit"
    }
};

static DM1_V1_D3L2D3R2F0108FloorOrnamentOcclusionSelfTestResultPc34 s_last_self_test;

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

uint8_t dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_blend_c10_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel)
{
    return source_pixel == DM1_C10_COLOR_FLESH ? destination_pixel : source_pixel;
}

unsigned int dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_decode_ordinal_pc34(
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
              DM1_V1_D3L2_D3R2_FOCCL_FOOTPRINT_MASK_PC34) != 0u;
    cleared = floor_ornament_ordinal &
        ~DM1_V1_D3L2_D3R2_FOCCL_FOOTPRINT_MASK_PC34;
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

bool dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_context_occludes_pc34(
    DM1_V1_D3L2D3R2F0108FloorOrnamentOcclusionSidePc34 side,
    DM1_V1_D3L2D3R2F0108FloorOrnamentOcclusionContextPc34 context,
    unsigned int floor_ornament_ordinal)
{
    bool fp_set = false;
    unsigned int cleared = 0u;
    bool primary_draws = false;

    if (side != DM1_V1_D3L2_D3R2_FOCCL_SIDE_D3L2_PC34 &&
        side != DM1_V1_D3L2_D3R2_FOCCL_SIDE_D3R2_PC34) {
        return false;
    }
    if (floor_ornament_ordinal == 0u) return false;
    if (!dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_decode_ordinal_pc34(
            floor_ornament_ordinal, &fp_set, &cleared, &primary_draws,
            NULL, NULL, NULL)) {
        return false;
    }
    if (!primary_draws && !fp_set) return false;
    switch (context) {
    case DM1_V1_D3L2_D3R2_FOCCL_CONTEXT_DOOR_FRONT_PC34:
        return true;
    case DM1_V1_D3L2_D3R2_FOCCL_CONTEXT_CORRIDOR_PC34:
    case DM1_V1_D3L2_D3R2_FOCCL_CONTEXT_OPEN_PIT_PC34:
    case DM1_V1_D3L2_D3R2_FOCCL_CONTEXT_TELEPORTER_PC34:
    case DM1_V1_D3L2_D3R2_FOCCL_CONTEXT_STAIRS_SIDE_PC34:
        /* BUG0_64: F0108 always blits over corridor / pit / teleporter /
         * stairs-side on the F0676/F0677 shared tail with no occlusion
         * guard. Cell order differs: D3L2 uses C0x3421 (and C0x0321
         * for door-side), D3R2 uses C0x4312 (and C0x0412 for door-side). */
        return true;
    }
    return false;
}

int dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_zone_d3l2_pc34(
    int coordinate_set,
    int view_floor)
{
    if (coordinate_set < 0) coordinate_set = 0;
    if (view_floor < 0) view_floor = 0;
    return DM1_FLOOR_ZONE_BASE +
        coordinate_set * DM1_FLOOR_ZONE_STRIDE + DM1_D3L2_VIEW_FLOOR;
}

int dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_zone_d3r2_pc34(
    int coordinate_set,
    int view_floor)
{
    if (coordinate_set < 0) coordinate_set = 0;
    if (view_floor < 0) view_floor = 0;
    return DM1_FLOOR_ZONE_BASE +
        coordinate_set * DM1_FLOOR_ZONE_STRIDE + DM1_D3R2_VIEW_FLOOR;
}

static void fill_model(DM1_V1_D3L2D3R2F0108FloorOrnamentOcclusionModelPc34 *m)
{
    m->view_square_d3l2 = DM1_D3L2_VIEW_SQUARE;
    m->view_square_d3r2 = DM1_D3R2_VIEW_SQUARE;
    m->view_floor_d3l2 = DM1_D3L2_VIEW_FLOOR;
    m->view_floor_d3r2 = DM1_D3R2_VIEW_FLOOR;
    m->wall_zone_d3l2 = DM1_D3L2_WALL_ZONE;
    m->wall_zone_d3r2 = DM1_D3R2_WALL_ZONE;
    m->c10_transparent_color = DM1_C10_COLOR_FLESH;
    m->footprint_index = DM1_FLOOR_ORNAMENT_FOOTPRINTS;
    m->floor_ornament_ordinal_slot = DM1_M558_FLOOR_ORNAMENT_ORDINAL_SLOT;
    m->front_wall_ornament_slot = DM1_M552_FRONT_WALL_ORNAMENT_SLOT;
    m->first_thing_slot = DM1_M550_FIRST_THING_SLOT;
    m->door_state_slot = DM1_M556_DOOR_STATE_SLOT;
    m->door_thing_index_slot = DM1_M557_DOOR_THING_INDEX_SLOT;
    m->pit_or_teleporter_visible_slot = DM1_M554_PIT_OR_TELEPORTER_VISIBLE_SLOT;
    m->stairs_up_slot = DM1_M555_STAIRS_UP_SLOT;
    m->f0128_dispatches_after_d3c = 1;
    m->f0676_dispatches_before_f0677 = 1;
    m->f0677_dispatches_before_d2l = 1;
    m->f0676_door_front_calls_f0108_with_552 = 1;
    m->f0676_door_front_calls_f0115_doorpass1 = 1;
    m->f0676_shared_tail_calls_f0108_with_558 = 1;
    m->f0676_shared_tail_open_pit_uses_f0104_first = 1;
    m->f0676_corridor_cell_order_d3l2 =
        DM1_V1_D3L2_D3R2_FOCCL_CELL_ORDER_OPEN_BLBR_FLFR_D3L2_PC34;
    m->f0676_side_cell_order_d3l2 =
        DM1_V1_D3L2_D3R2_FOCCL_CELL_ORDER_SIDE_BLBR_FR_PC34;
    m->f0676_doorpass2_cell_order_d3l2 =
        DM1_V1_D3L2_D3R2_FOCCL_CELL_ORDER_DOORPASS2_FLFR_PC34;
    m->f0677_door_front_calls_f0108 = 1;
    m->f0677_door_front_calls_f0115_doorpass1 = 1;
    m->f0677_shared_tail_calls_f0108 = 1;
    m->f0677_shared_tail_open_pit_uses_f0104_first = 1;
    m->f0677_corridor_cell_order_d3r2 =
        DM1_V1_D3L2_D3R2_FOCCL_CELL_ORDER_OPEN_BRBL_FRFL_D3R2_PC34;
    m->f0677_side_cell_order_d3r2 =
        DM1_V1_D3L2_D3R2_FOCCL_CELL_ORDER_SIDE_BRBL_FL_PC34;
    m->f0677_doorpass2_cell_order_d3r2 =
        DM1_V1_D3L2_D3R2_FOCCL_CELL_ORDER_DOORPASS2_FRFL_PC34;
    m->f0108_ordinal_zero_skips_blit = 1;
    m->f0108_footprint_mask_recurses = 1;
    m->f0108_footprint_only_skips_primary = 1;
    m->f0108_blit_uses_c10_transparent = 1;
    m->f0108_zone_uses_11_stride = 1;
    m->f0108_zone_d3l2 = DM1_FLOOR_ZONE_D3L2;
    m->f0108_zone_d3r2 = DM1_FLOOR_ZONE_D3R2;
    m->bug0_64_occlusion_guard = 0;
    m->no_graphics_dat_reads = 1;
    m->source_locked_contract_only = 1;
    m->no_real_asset_bitmap_parity = 1;
    m->source_evidence = s_source_evidence;
    m->disjointness_note = s_disjointness_note;
}

bool dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_default_model_builder_pc34(
    DM1_V1_D3L2D3R2F0108FloorOrnamentOcclusionModelPc34 *out_model)
{
    if (!out_model) return false;
    memset(out_model, 0, sizeof(*out_model));
    fill_model(out_model);
    out_model->deterministic_hash =
        dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_hash_model_pc34(
            out_model);
    return true;
}

uint32_t dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_hash_model_pc34(
    const DM1_V1_D3L2D3R2F0108FloorOrnamentOcclusionModelPc34 *model)
{
    uint32_t h = 2166136261u;
    size_t i;

    if (!model) return 0u;
    h = fnv1a_u32(h, (uint32_t)model->view_square_d3l2);
    h = fnv1a_u32(h, (uint32_t)model->view_square_d3r2);
    h = fnv1a_u32(h, (uint32_t)model->view_floor_d3l2);
    h = fnv1a_u32(h, (uint32_t)model->view_floor_d3r2);
    h = fnv1a_u32(h, (uint32_t)model->wall_zone_d3l2);
    h = fnv1a_u32(h, (uint32_t)model->wall_zone_d3r2);
    h = fnv1a_u32(h, (uint32_t)model->c10_transparent_color);
    h = fnv1a_u32(h, (uint32_t)model->footprint_index);
    h = fnv1a_u32(h, (uint32_t)model->floor_ornament_ordinal_slot);
    h = fnv1a_u32(h, (uint32_t)model->front_wall_ornament_slot);
    h = fnv1a_u32(h, (uint32_t)model->first_thing_slot);
    h = fnv1a_u32(h, (uint32_t)model->door_state_slot);
    h = fnv1a_u32(h, (uint32_t)model->door_thing_index_slot);
    h = fnv1a_u32(h, (uint32_t)model->pit_or_teleporter_visible_slot);
    h = fnv1a_u32(h, (uint32_t)model->stairs_up_slot);
    h = fnv1a_u32(h, (uint32_t)model->f0128_dispatches_after_d3c);
    h = fnv1a_u32(h, (uint32_t)model->f0676_dispatches_before_f0677);
    h = fnv1a_u32(h, (uint32_t)model->f0677_dispatches_before_d2l);
    h = fnv1a_u32(h, (uint32_t)model->f0676_door_front_calls_f0108_with_552);
    h = fnv1a_u32(h, (uint32_t)model->f0676_door_front_calls_f0115_doorpass1);
    h = fnv1a_u32(h, (uint32_t)model->f0676_shared_tail_calls_f0108_with_558);
    h = fnv1a_u32(h, (uint32_t)model->f0676_shared_tail_open_pit_uses_f0104_first);
    h = fnv1a_u32(h, (uint32_t)model->f0676_corridor_cell_order_d3l2);
    h = fnv1a_u32(h, (uint32_t)model->f0676_side_cell_order_d3l2);
    h = fnv1a_u32(h, (uint32_t)model->f0676_doorpass2_cell_order_d3l2);
    h = fnv1a_u32(h, (uint32_t)model->f0677_door_front_calls_f0108);
    h = fnv1a_u32(h, (uint32_t)model->f0677_door_front_calls_f0115_doorpass1);
    h = fnv1a_u32(h, (uint32_t)model->f0677_shared_tail_calls_f0108);
    h = fnv1a_u32(h, (uint32_t)model->f0677_shared_tail_open_pit_uses_f0104_first);
    h = fnv1a_u32(h, (uint32_t)model->f0677_corridor_cell_order_d3r2);
    h = fnv1a_u32(h, (uint32_t)model->f0677_side_cell_order_d3r2);
    h = fnv1a_u32(h, (uint32_t)model->f0677_doorpass2_cell_order_d3r2);
    h = fnv1a_u32(h, (uint32_t)model->f0108_ordinal_zero_skips_blit);
    h = fnv1a_u32(h, (uint32_t)model->f0108_footprint_mask_recurses);
    h = fnv1a_u32(h, (uint32_t)model->f0108_footprint_only_skips_primary);
    h = fnv1a_u32(h, (uint32_t)model->f0108_blit_uses_c10_transparent);
    h = fnv1a_u32(h, (uint32_t)model->f0108_zone_uses_11_stride);
    h = fnv1a_u32(h, (uint32_t)model->f0108_zone_d3l2);
    h = fnv1a_u32(h, (uint32_t)model->f0108_zone_d3r2);
    h = fnv1a_u32(h, (uint32_t)model->bug0_64_occlusion_guard);
    for (i = 0; i < sizeof(s_steps) / sizeof(s_steps[0]); ++i) {
        h = fnv1a_u32(h, (uint32_t)s_steps[i].context);
        h = fnv1a_u32(h, (uint32_t)s_steps[i].calls_f0108);
        h = fnv1a_u32(h, (uint32_t)s_steps[i].f0108_occludes_cell);
        h = fnv1a_u32(h, (uint32_t)s_steps[i].bug0_64_occlusion_present);
        h = fnv1a_u32(h, (uint32_t)s_steps[i].calls_f0115);
        h = fnv1a_u32(h, (uint32_t)s_steps[i].expected_cell_order);
        h = fnv1a_u32(h, (uint32_t)s_steps[i].expected_view_floor);
        h = fnv1a_u32(h, (uint32_t)s_steps[i].expected_zone_wall);
    }
    return h;
}

const DM1_V1_D3L2D3R2F0108FloorOrnamentOcclusionModelPc34 *
dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_default_model_pc34(void)
{
    static DM1_V1_D3L2D3R2F0108FloorOrnamentOcclusionModelPc34 s_model;
    static int s_init;

    if (!s_init) {
        dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_default_model_builder_pc34(
            &s_model);
        s_init = 1;
    }
    return &s_model;
}

uint32_t dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_deterministic_hash_pc34(void)
{
    const DM1_V1_D3L2D3R2F0108FloorOrnamentOcclusionModelPc34 *model =
        dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_default_model_pc34();
    return model ? model->deterministic_hash : 0u;
}

unsigned int dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_context_count_pc34(void)
{
    return (unsigned int)(sizeof(s_steps) / sizeof(s_steps[0]));
}

const DM1_V1_D3L2D3R2F0108FloorOrnamentOcclusionStepPc34 *
dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_step_at_pc34(size_t index)
{
    if (index >= sizeof(s_steps) / sizeof(s_steps[0])) return NULL;
    return &s_steps[index];
}

const char *dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const char *
dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_disjointness_note_pc34(void)
{
    return s_disjointness_note;
}

static void self_check(
    int condition,
    DM1_V1_D3L2D3R2F0108FloorOrnamentOcclusionSelfTestResultPc34 *result)
{
    ++result->assertions;
    if (!condition) ++result->failures;
}

int dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_self_test_pc34(void)
{
    DM1_V1_D3L2D3R2F0108FloorOrnamentOcclusionModelPc34 built;
    const DM1_V1_D3L2D3R2F0108FloorOrnamentOcclusionModelPc34 *model;
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
        dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_default_model_builder_pc34(
            &built) ? 1 : 0;
    model = dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_default_model_pc34();
    self_check(s_last_self_test.model_builder_ok == 1, &s_last_self_test);
    self_check(model != NULL, &s_last_self_test);
    self_check(
        dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_default_model_builder_pc34(
            NULL) == 0,
        &s_last_self_test);
    self_check(
        dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_hash_model_pc34(
            NULL) == 0u,
        &s_last_self_test);
    if (model) {
        s_last_self_test.hash_stable =
            built.deterministic_hash == model->deterministic_hash &&
            dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_hash_model_pc34(
                model) == model->deterministic_hash &&
            dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_deterministic_hash_pc34() ==
                model->deterministic_hash;
        self_check(s_last_self_test.hash_stable == 1, &s_last_self_test);
        self_check(model->view_square_d3l2 == DM1_D3L2_VIEW_SQUARE,
                   &s_last_self_test);
        self_check(model->view_square_d3r2 == DM1_D3R2_VIEW_SQUARE,
                   &s_last_self_test);
        self_check(model->view_floor_d3l2 == DM1_D3L2_VIEW_FLOOR,
                   &s_last_self_test);
        self_check(model->view_floor_d3r2 == DM1_D3R2_VIEW_FLOOR,
                   &s_last_self_test);
        self_check(model->wall_zone_d3l2 == DM1_D3L2_WALL_ZONE,
                   &s_last_self_test);
        self_check(model->wall_zone_d3r2 == DM1_D3R2_WALL_ZONE,
                   &s_last_self_test);
        self_check(model->c10_transparent_color == DM1_C10_COLOR_FLESH,
                   &s_last_self_test);
        self_check(model->f0108_zone_d3l2 == DM1_FLOOR_ZONE_D3L2,
                   &s_last_self_test);
        self_check(model->f0108_zone_d3r2 == DM1_FLOOR_ZONE_D3R2,
                   &s_last_self_test);
        self_check(model->bug0_64_occlusion_guard == 0, &s_last_self_test);
        self_check(model->no_graphics_dat_reads == 1, &s_last_self_test);
        self_check(model->source_locked_contract_only == 1, &s_last_self_test);
        self_check(model->no_real_asset_bitmap_parity == 1, &s_last_self_test);
        self_check(model->f0108_ordinal_zero_skips_blit == 1,
                   &s_last_self_test);
        self_check(model->f0676_door_front_calls_f0108_with_552 == 1,
                   &s_last_self_test);
        self_check(model->f0677_door_front_calls_f0108 == 1,
                   &s_last_self_test);
        self_check(model->f0676_shared_tail_open_pit_uses_f0104_first == 1,
                   &s_last_self_test);
        self_check(model->f0677_shared_tail_open_pit_uses_f0104_first == 1,
                   &s_last_self_test);
        s_last_self_test.f0676_door_front_branch_present =
            model->f0676_door_front_calls_f0108_with_552 == 1 &&
            model->f0676_door_front_calls_f0115_doorpass1 == 1 &&
            model->f0676_corridor_cell_order_d3l2 ==
                DM1_V1_D3L2_D3R2_FOCCL_CELL_ORDER_OPEN_BLBR_FLFR_D3L2_PC34;
        self_check(s_last_self_test.f0676_door_front_branch_present == 1,
                   &s_last_self_test);
        s_last_self_test.f0677_door_front_branch_present =
            model->f0677_door_front_calls_f0108 == 1 &&
            model->f0677_door_front_calls_f0115_doorpass1 == 1 &&
            model->f0677_corridor_cell_order_d3r2 ==
                DM1_V1_D3L2_D3R2_FOCCL_CELL_ORDER_OPEN_BRBL_FRFL_D3R2_PC34;
        self_check(s_last_self_test.f0677_door_front_branch_present == 1,
                   &s_last_self_test);
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
    dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_decode_ordinal_pc34(
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
    dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_decode_ordinal_pc34(
        0x8000u, &fp_set, &cleared, &primary_draws, &primary_index, &recurse_fp,
        &recurse_index);
    s_last_self_test.decode_fp_only_recurses =
        fp_set && cleared == 0u && !primary_draws && primary_index == -1 &&
        recurse_fp && recurse_index == DM1_FLOOR_ORNAMENT_FOOTPRINTS;
    self_check(s_last_self_test.decode_fp_only_recurses == 1,
               &s_last_self_test);

    fp_set = false;
    cleared = 0u;
    primary_draws = false;
    recurse_fp = true;
    primary_index = -1;
    recurse_index = -1;
    dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_decode_ordinal_pc34(
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
    dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_decode_ordinal_pc34(
        0u, &fp_set, &cleared, &primary_draws, NULL, NULL, NULL);
    s_last_self_test.decode_zero_skips_blit =
        !fp_set && cleared == 0u && !primary_draws;
    self_check(s_last_self_test.decode_zero_skips_blit == 1,
               &s_last_self_test);

    s_last_self_test.context_occlusion_paths =
        (dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_context_occludes_pc34(
             DM1_V1_D3L2_D3R2_FOCCL_SIDE_D3L2_PC34,
             DM1_V1_D3L2_D3R2_FOCCL_CONTEXT_CORRIDOR_PC34, 5u) ? 1 : 0) +
        (dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_context_occludes_pc34(
             DM1_V1_D3L2_D3R2_FOCCL_SIDE_D3L2_PC34,
             DM1_V1_D3L2_D3R2_FOCCL_CONTEXT_OPEN_PIT_PC34, 5u) ? 1 : 0) +
        (dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_context_occludes_pc34(
             DM1_V1_D3L2_D3R2_FOCCL_SIDE_D3L2_PC34,
             DM1_V1_D3L2_D3R2_FOCCL_CONTEXT_TELEPORTER_PC34, 5u) ? 1 : 0) +
        (dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_context_occludes_pc34(
             DM1_V1_D3L2_D3R2_FOCCL_SIDE_D3L2_PC34,
             DM1_V1_D3L2_D3R2_FOCCL_CONTEXT_STAIRS_SIDE_PC34, 5u) ? 1 : 0) +
        (dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_context_occludes_pc34(
             DM1_V1_D3L2_D3R2_FOCCL_SIDE_D3L2_PC34,
             DM1_V1_D3L2_D3R2_FOCCL_CONTEXT_DOOR_FRONT_PC34, 5u) ? 1 : 0);
    self_check(s_last_self_test.context_occlusion_paths == 5,
               &s_last_self_test);
    s_last_self_test.context_zero_ordinal_no_occlusion =
        !dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_context_occludes_pc34(
            DM1_V1_D3L2_D3R2_FOCCL_SIDE_D3R2_PC34,
            DM1_V1_D3L2_D3R2_FOCCL_CONTEXT_OPEN_PIT_PC34, 0u);
    self_check(s_last_self_test.context_zero_ordinal_no_occlusion == 1,
               &s_last_self_test);
    self_check(
        !dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_context_occludes_pc34(
            (DM1_V1_D3L2D3R2F0108FloorOrnamentOcclusionSidePc34)99,
            DM1_V1_D3L2_D3R2_FOCCL_CONTEXT_OPEN_PIT_PC34, 5u),
        &s_last_self_test);
    self_check(
        !dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_context_occludes_pc34(
            DM1_V1_D3L2_D3R2_FOCCL_SIDE_D3L2_PC34,
            (DM1_V1_D3L2D3R2F0108FloorOrnamentOcclusionContextPc34)99, 5u),
        &s_last_self_test);

    s_last_self_test.blend_c10_preserves_destination =
        dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_blend_c10_pc34(
            0xaau, 10u) == 0xaau;
    self_check(s_last_self_test.blend_c10_preserves_destination == 1,
               &s_last_self_test);
    s_last_self_test.blend_opaque_writes_source =
        dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_blend_c10_pc34(
            0xaau, 0x52u) == 0x52u;
    self_check(s_last_self_test.blend_opaque_writes_source == 1,
               &s_last_self_test);
    s_last_self_test.zone_d3l2_stride_11 =
        dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_zone_d3l2_pc34(
            0, 0) == DM1_FLOOR_ZONE_D3L2 &&
        dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_zone_d3l2_pc34(
            1, 0) == DM1_FLOOR_ZONE_BASE + DM1_FLOOR_ZONE_STRIDE +
            DM1_D3L2_VIEW_FLOOR &&
        dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_zone_d3l2_pc34(
            -1, -1) == DM1_FLOOR_ZONE_D3L2;
    self_check(s_last_self_test.zone_d3l2_stride_11 == 1, &s_last_self_test);
    s_last_self_test.zone_d3r2_stride_11 =
        dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_zone_d3r2_pc34(
            0, 0) == DM1_FLOOR_ZONE_D3R2 &&
        dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_zone_d3r2_pc34(
            2, 0) == DM1_FLOOR_ZONE_BASE + 2 * DM1_FLOOR_ZONE_STRIDE +
            DM1_D3R2_VIEW_FLOOR &&
        dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_zone_d3r2_pc34(
            -1, -1) == DM1_FLOOR_ZONE_D3R2;
    self_check(s_last_self_test.zone_d3r2_stride_11 == 1, &s_last_self_test);

    n = dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_context_count_pc34();
    s_last_self_test.step_count_ten =
        (n == sizeof(s_steps) / sizeof(s_steps[0]) && n == 10u);
    self_check(s_last_self_test.step_count_ten == 1, &s_last_self_test);
    self_check(
        dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_step_at_pc34(n) ==
            NULL,
        &s_last_self_test);
    for (i = 0; i < n; ++i) {
        const DM1_V1_D3L2D3R2F0108FloorOrnamentOcclusionStepPc34 *step =
            dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_step_at_pc34(i);
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
    self_check(s_last_self_test.bug0_64_marker_count == 5,
               &s_last_self_test);

    s_last_self_test.source_evidence_present =
        strstr(s_source_evidence, "F0108:3940-4011") != NULL &&
        strstr(s_source_evidence, "F0676:6226-6290") != NULL &&
        strstr(s_source_evidence, "F0677:6293-6357") != NULL &&
        strstr(s_source_evidence, "BUG0_64") != NULL &&
        strstr(s_source_evidence, "C1500") != NULL &&
        strstr(s_source_evidence, "C10_COLOR_FLESH") != NULL &&
        strstr(s_source_evidence, "F0128") != NULL &&
        strstr(s_source_evidence, "M558") != NULL;
    self_check(s_last_self_test.source_evidence_present == 1,
               &s_last_self_test);
    s_last_self_test.disjointness_note_present =
        strstr(s_disjointness_note, "D1C F0108") != NULL &&
        strstr(s_disjointness_note, "D3L2/D3R2") != NULL &&
        strstr(s_disjointness_note, "GRAPHICS.DAT") != NULL &&
        strstr(s_disjointness_note, "CSB/Nexus/Theron/DM2") != NULL &&
        strstr(s_disjointness_note, "F0676") != NULL &&
        strstr(s_disjointness_note, "F0677") != NULL;
    self_check(s_last_self_test.disjointness_note_present == 1,
               &s_last_self_test);
    self_check(s_last_self_test.assertions >= 40, &s_last_self_test);
    self_check(s_last_self_test.deterministic_hash != 2166136261u,
               &s_last_self_test);

    s_last_self_test.ok = s_last_self_test.failures == 0;
    return s_last_self_test.ok;
}

const DM1_V1_D3L2D3R2F0108FloorOrnamentOcclusionSelfTestResultPc34 *
dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_last_self_test_result_pc34(void)
{
    return &s_last_self_test;
}
