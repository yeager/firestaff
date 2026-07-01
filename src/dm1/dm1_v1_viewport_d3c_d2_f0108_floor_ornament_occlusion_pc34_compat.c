#include "firestaff/dm1/v1/viewport/dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_pc34_compat.h"

#include <string.h>

enum {
    DM1_M550_FIRST_THING_SLOT = 2,
    DM1_M558_FLOOR_ORNAMENT_ORDINAL_SLOT = 5,
    DM1_D3C_VIEW_SQUARE = 11,
    DM1_D2L_VIEW_SQUARE = 7,
    DM1_D2R_VIEW_SQUARE = 8,
    DM1_D2C_VIEW_SQUARE = 6,
    DM1_D3C_VIEW_FLOOR = 3,
    DM1_D2L_VIEW_FLOOR = 5,
    DM1_D2R_VIEW_FLOOR = 7,
    DM1_D2C_VIEW_FLOOR = 6,
    DM1_CELL_ORDER_BLBR_FLFR = 0x3421,
    DM1_CELL_ORDER_BRBL_FRFL = 0x4312
};

static const char s_source_evidence[] =
    "ReDMCSB source-lock: DUNVIEW.C F0118:6642-6832 D3C, F0119:6900-7049 "
    "D2L, F0120:7051-7242 D2R, and F0121:7244-7388 D2C each draw the "
    "open-pit floor bitmap first, then call F0108 with M558 floor ornament "
    "and the view floor (D3C line 6814, D2L line 7020, D2R line 7213, "
    "D2C line 7357). Those calls carry BUG0_64: floor ornaments are drawn "
    "over open pits because there is no occlusion guard. F0108:3940-4011 "
    "owns ordinal zero skip, MASK0x8000 footprint recursion, C10_COLOR_FLESH "
    "transparent blit, and C1500 + coordinateSet * 11 + viewFloor zone math. "
    "F0128:8318-8542 provides the far-to-near dispatch order.";

static const char s_disjointness_note[] =
    "Disjoint DM1 V1 D3C/D2 F0108 floor-ornament occlusion gate. Existing "
    "D1C, D3L2/D3R2, and D3L/D3R occlusion gates already cover their "
    "source lines; this module covers only the remaining D3C/D2L/D2R/D2C "
    "BUG0_64 anchors. Contract-only, asset-free, no GRAPHICS.DAT reads, "
    "and no original-DOS pixel-parity or real-asset screenshot claim.";

static DM1_V1_D3CD2F0108FloorOrnamentOcclusionSelfTestResultPc34 s_last_self_test;

static uint32_t fnv1a_u32(uint32_t hash, uint32_t value)
{
    int i;
    for (i = 0; i < 4; ++i) {
        hash ^= (value >> ((unsigned int)i * 8u)) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

static void fill_anchors(DM1_V1_D3CD2F0108FloorOrnamentOcclusionModelPc34 *m)
{
    static const DM1_V1_D3CD2F0108FloorOrnamentOcclusionAnchorPc34 anchors[] = {
        { DM1_V1_D3C_D2_FOCCL_LANE_D3C_PC34, "D3C",
          "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF", DM1_D3C_VIEW_SQUARE,
          DM1_D3C_VIEW_FLOOR, 6814, DM1_CELL_ORDER_BLBR_FLFR,
          DM1_V1_D3C_D2_FOCCL_FLOOR_ZONE_BASE_PC34 + DM1_D3C_VIEW_FLOOR,
          "DUNVIEW.C:6748-6762", "DUNVIEW.C:6811-6814 BUG0_64",
          "DUNVIEW.C:6816" },
        { DM1_V1_D3C_D2_FOCCL_LANE_D2L_PC34, "D2L",
          "F0119_DUNGEONVIEW_DrawSquareD2L", DM1_D2L_VIEW_SQUARE,
          DM1_D2L_VIEW_FLOOR, 7020, DM1_CELL_ORDER_BLBR_FLFR,
          DM1_V1_D3C_D2_FOCCL_FLOOR_ZONE_BASE_PC34 + DM1_D2L_VIEW_FLOOR,
          "DUNVIEW.C:7005-7015", "DUNVIEW.C:7017-7020 BUG0_64",
          "DUNVIEW.C:7031" },
        { DM1_V1_D3C_D2_FOCCL_LANE_D2R_PC34, "D2R",
          "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF", DM1_D2R_VIEW_SQUARE,
          DM1_D2R_VIEW_FLOOR, 7213, DM1_CELL_ORDER_BRBL_FRFL,
          DM1_V1_D3C_D2_FOCCL_FLOOR_ZONE_BASE_PC34 + DM1_D2R_VIEW_FLOOR,
          "DUNVIEW.C:7198-7208", "DUNVIEW.C:7210-7213 BUG0_64",
          "DUNVIEW.C:7224" },
        { DM1_V1_D3C_D2_FOCCL_LANE_D2C_PC34, "D2C",
          "F0121_DUNGEONVIEW_DrawSquareD2C", DM1_D2C_VIEW_SQUARE,
          DM1_D2C_VIEW_FLOOR, 7357, DM1_CELL_ORDER_BLBR_FLFR,
          DM1_V1_D3C_D2_FOCCL_FLOOR_ZONE_BASE_PC34 + DM1_D2C_VIEW_FLOOR,
          "DUNVIEW.C:7343-7353", "DUNVIEW.C:7355-7357 BUG0_64",
          "DUNVIEW.C:7367-7368" }
    };

    memcpy(m->anchors, anchors, sizeof(anchors));
}

static void fill_model(DM1_V1_D3CD2F0108FloorOrnamentOcclusionModelPc34 *m)
{
    fill_anchors(m);
    m->c10_transparent_color = DM1_V1_D3C_D2_FOCCL_C10_COLOR_FLESH_PC34;
    m->floor_zone_base = DM1_V1_D3C_D2_FOCCL_FLOOR_ZONE_BASE_PC34;
    m->floor_zone_stride = DM1_V1_D3C_D2_FOCCL_FLOOR_ZONE_STRIDE_PC34;
    m->floor_ornament_ordinal_slot = DM1_M558_FLOOR_ORNAMENT_ORDINAL_SLOT;
    m->first_thing_slot = DM1_M550_FIRST_THING_SLOT;
    m->bug0_64_anchor_count = DM1_V1_D3C_D2_FOCCL_ANCHOR_COUNT_PC34;
    m->f0108_ordinal_zero_skips_blit = 1;
    m->f0108_footprint_mask_recurses = 1;
    m->f0108_blit_uses_c10_transparent = 1;
    m->no_graphics_dat_reads = 1;
    m->source_locked_contract_only = 1;
    m->no_original_dos_pixel_parity = 1;
    m->source_evidence = s_source_evidence;
    m->disjointness_note = s_disjointness_note;
}

bool dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_default_model_builder_pc34(
    DM1_V1_D3CD2F0108FloorOrnamentOcclusionModelPc34 *out_model)
{
    if (!out_model) return false;
    memset(out_model, 0, sizeof(*out_model));
    fill_model(out_model);
    out_model->deterministic_hash =
        dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_hash_model_pc34(out_model);
    return true;
}

const DM1_V1_D3CD2F0108FloorOrnamentOcclusionModelPc34 *
dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_default_model_pc34(void)
{
    static DM1_V1_D3CD2F0108FloorOrnamentOcclusionModelPc34 s_model;
    static int s_init;
    if (!s_init) {
        dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_default_model_builder_pc34(&s_model);
        s_init = 1;
    }
    return &s_model;
}

uint32_t dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_hash_model_pc34(
    const DM1_V1_D3CD2F0108FloorOrnamentOcclusionModelPc34 *model)
{
    uint32_t h = 2166136261u;
    size_t i;
    if (!model) return 0u;
    h = fnv1a_u32(h, (uint32_t)model->c10_transparent_color);
    h = fnv1a_u32(h, (uint32_t)model->floor_zone_base);
    h = fnv1a_u32(h, (uint32_t)model->floor_zone_stride);
    h = fnv1a_u32(h, (uint32_t)model->bug0_64_anchor_count);
    for (i = 0; i < DM1_V1_D3C_D2_FOCCL_ANCHOR_COUNT_PC34; ++i) {
        h = fnv1a_u32(h, (uint32_t)model->anchors[i].lane);
        h = fnv1a_u32(h, (uint32_t)model->anchors[i].view_square);
        h = fnv1a_u32(h, (uint32_t)model->anchors[i].view_floor);
        h = fnv1a_u32(h, (uint32_t)model->anchors[i].source_line);
        h = fnv1a_u32(h, (uint32_t)model->anchors[i].cell_order);
        h = fnv1a_u32(h, (uint32_t)model->anchors[i].floor_zone_at_coordinate_zero);
    }
    return h;
}

uint32_t dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_deterministic_hash_pc34(void)
{
    const DM1_V1_D3CD2F0108FloorOrnamentOcclusionModelPc34 *model =
        dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_default_model_pc34();
    return model ? model->deterministic_hash : 0u;
}

unsigned int dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_anchor_count_pc34(void)
{
    return DM1_V1_D3C_D2_FOCCL_ANCHOR_COUNT_PC34;
}

const DM1_V1_D3CD2F0108FloorOrnamentOcclusionAnchorPc34 *
dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_anchor_at_pc34(size_t index)
{
    const DM1_V1_D3CD2F0108FloorOrnamentOcclusionModelPc34 *model =
        dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_default_model_pc34();
    if (!model || index >= DM1_V1_D3C_D2_FOCCL_ANCHOR_COUNT_PC34) return NULL;
    return &model->anchors[index];
}

int dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_zone_pc34(
    int coordinate_set,
    int view_floor)
{
    if (coordinate_set < 0) coordinate_set = 0;
    if (view_floor < 0) view_floor = 0;
    return DM1_V1_D3C_D2_FOCCL_FLOOR_ZONE_BASE_PC34 +
        coordinate_set * DM1_V1_D3C_D2_FOCCL_FLOOR_ZONE_STRIDE_PC34 + view_floor;
}

bool dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_lane_occludes_pc34(
    DM1_V1_D3CD2F0108FloorOrnamentOcclusionLanePc34 lane,
    unsigned int floor_ornament_ordinal)
{
    if (floor_ornament_ordinal == 0u) return false;
    return lane >= DM1_V1_D3C_D2_FOCCL_LANE_D3C_PC34 &&
        lane <= DM1_V1_D3C_D2_FOCCL_LANE_D2C_PC34;
}

const char *dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const char *dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_disjointness_note_pc34(void)
{
    return s_disjointness_note;
}

static void check(int condition, DM1_V1_D3CD2F0108FloorOrnamentOcclusionSelfTestResultPc34 *r)
{
    ++r->assertions;
    if (!condition) ++r->failures;
}

int dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_self_test_pc34(void)
{
    DM1_V1_D3CD2F0108FloorOrnamentOcclusionModelPc34 built;
    const DM1_V1_D3CD2F0108FloorOrnamentOcclusionModelPc34 *model;
    size_t i;

    memset(&s_last_self_test, 0, sizeof(s_last_self_test));
    s_last_self_test.model_builder_ok =
        dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_default_model_builder_pc34(&built) ? 1 : 0;
    model = dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_default_model_pc34();
    check(s_last_self_test.model_builder_ok == 1, &s_last_self_test);
    check(model != NULL, &s_last_self_test);
    if (!model) return 0;
    s_last_self_test.hash_stable =
        built.deterministic_hash == model->deterministic_hash &&
        dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_hash_model_pc34(model) ==
            model->deterministic_hash;
    s_last_self_test.anchor_count_four =
        dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_anchor_count_pc34() == 4u;
    s_last_self_test.bug0_64_count_four = model->bug0_64_anchor_count == 4;
    s_last_self_test.zones_match_stride = 1;
    s_last_self_test.occlusion_accepts_nonzero_ordinals = 1;
    for (i = 0; i < DM1_V1_D3C_D2_FOCCL_ANCHOR_COUNT_PC34; ++i) {
        const DM1_V1_D3CD2F0108FloorOrnamentOcclusionAnchorPc34 *a = &model->anchors[i];
        if (dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_zone_pc34(
                0, a->view_floor) != a->floor_zone_at_coordinate_zero) {
            s_last_self_test.zones_match_stride = 0;
        }
        if (!dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_lane_occludes_pc34(
                a->lane, 1u)) {
            s_last_self_test.occlusion_accepts_nonzero_ordinals = 0;
        }
    }
    s_last_self_test.occlusion_rejects_zero_ordinal =
        !dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_lane_occludes_pc34(
            DM1_V1_D3C_D2_FOCCL_LANE_D3C_PC34, 0u);
    s_last_self_test.source_evidence_present =
        strstr(s_source_evidence, "BUG0_64") != NULL &&
        strstr(s_source_evidence, "6814") != NULL &&
        strstr(s_source_evidence, "7020") != NULL &&
        strstr(s_source_evidence, "7213") != NULL &&
        strstr(s_source_evidence, "7357") != NULL;
    s_last_self_test.disjointness_note_present =
        strstr(s_disjointness_note, "D1C") != NULL &&
        strstr(s_disjointness_note, "D3L/D3R") != NULL &&
        strstr(s_disjointness_note, "no original-DOS") != NULL;

    check(s_last_self_test.hash_stable, &s_last_self_test);
    check(s_last_self_test.anchor_count_four, &s_last_self_test);
    check(s_last_self_test.bug0_64_count_four, &s_last_self_test);
    check(s_last_self_test.zones_match_stride, &s_last_self_test);
    check(s_last_self_test.occlusion_accepts_nonzero_ordinals, &s_last_self_test);
    check(s_last_self_test.occlusion_rejects_zero_ordinal, &s_last_self_test);
    check(s_last_self_test.source_evidence_present, &s_last_self_test);
    check(s_last_self_test.disjointness_note_present, &s_last_self_test);
    s_last_self_test.deterministic_hash = model->deterministic_hash;
    s_last_self_test.ok = s_last_self_test.failures == 0;
    return s_last_self_test.ok;
}

const DM1_V1_D3CD2F0108FloorOrnamentOcclusionSelfTestResultPc34 *
dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_last_self_test_result_pc34(void)
{
    return &s_last_self_test;
}
