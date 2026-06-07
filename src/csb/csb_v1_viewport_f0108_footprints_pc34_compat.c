#include "csb/csb_v1_viewport_f0108_footprints_pc34_compat.h"

enum {
    CSB_PRESENT = 1,
    CSB_ABSENT = 0,
    CSB_FOOTPRINT_MASK = 0x8000,       /* ReDMCSB DEFS.H:2561 MASK0x8000_FOOTPRINTS. */
    CSB_FOOTPRINT_INDEX = 15,          /* ReDMCSB DEFS.H:2465 C15_FLOOR_ORNAMENT_FOOTPRINTS. */
    CSB_FOOTPRINT_ORDINAL = 16,        /* ReDMCSB COMPILE.H:1038 M000_INDEX_TO_ORDINAL. */
    CSB_VIEW_FLOOR_D3L2 = 0,           /* ReDMCSB DEFS.H:2750 C00_VIEW_FLOOR_D3L2. */
    CSB_VIEW_FLOOR_D3R2 = 1,           /* ReDMCSB DEFS.H:2751 C01_VIEW_FLOOR_D3R2. */
    CSB_VIEW_FLOOR_D3C = 3,            /* ReDMCSB DEFS.H:2753 M589_VIEW_FLOOR_D3C. */
    CSB_VIEW_FLOOR_D2C = 6,            /* ReDMCSB DEFS.H:2756 M592_VIEW_FLOOR_D2C. */
    CSB_VIEW_FLOOR_D1C = 9,            /* ReDMCSB DEFS.H:2759 M595_VIEW_FLOOR_D1C. */
    CSB_VIEW_FLOOR_D3R = 4,            /* ReDMCSB DEFS.H:2754 M590_VIEW_FLOOR_D3R. */
    CSB_VIEW_FLOOR_D2R = 7,            /* ReDMCSB DEFS.H:2757 M593_VIEW_FLOOR_D2R. */
    CSB_VIEW_FLOOR_D1R = 10,           /* ReDMCSB DEFS.H:2760 M596_VIEW_FLOOR_D1R. */
    CSB_ZONE_FLOOR_ORNAMENT = 1500,    /* ReDMCSB DEFS.H:4223 C1500_ZONE_FLOOR_ORNAMENT. */
    CSB_COORDINATE_SET_STRIDE = 11,    /* ReDMCSB DUNVIEW.C:3998 PC34/I34 zone stride. */
    CSB_COORDINATE_SET_INDEX = 0,      /* ReDMCSB DUNVIEW.C:1008-1017 CSB/I34 G0195 entries. */
    CSB_COLOR_FLESH = 10,              /* ReDMCSB DEFS.H:2088 C10_COLOR_FLESH. */
    CSB_FLIP_HORIZONTAL = 1            /* ReDMCSB DUNVIEW.C:3983 MASK0x0001_FLIP_HORIZONTAL. */
};

static const char s_source_evidence[] =
    "Contract-only synthetic gate; no real-asset pixel parity is claimed. "
    "ReDMCSB DUNVIEW.C:F0108_DUNGEONVIEW_DrawFloorOrnament:3940-4011 "
    "checks ordinal zero at 3959, detects MASK0x8000_FOOTPRINTS at 3960, "
    "clears the mask before the base floor-ornament draw at 3961, skips "
    "that base draw when the cleared ordinal is zero at 3961-3962, draws "
    "the base floor ornament through ordinal--/G0191/G0206 at 3965-3966, "
    "uses the PC34/I34 C1500 + CoordinateSet*11 + ViewFloor F0791 path at "
    "3998 with C10 transparency, then recursively draws "
    "M000_INDEX_TO_ORDINAL(C15_FLOOR_ORNAMENT_FOOTPRINTS) at 4007-4008. "
    "DUNVIEW.C:3980-3983 flips right floor views and, for footprint index "
    "15, flipped center D1C/D2C/D3C views. ReDMCSB DUNGEON.C:"
    "F0172_DUNGEON_SetSquareAspect:2666-2718 sources M558 floor ornaments "
    "from random/sensor/scent data and sets MASK0x8000 footprints; "
    "DUNGEON.C:F0174:2755-2760 copies current-map floor ornament metadata. "
    "CSB/I34 adds C00/C01 D3L2/D3R2 floor-view indices and the 11-slot "
    "floor-ornament zone namespace (DEFS.H:2749-2760,4223), while the "
    "Firestaff CSB dungeon loader keeps CSB DUNGEON.DAT profile data under "
    "src/csb/csb_v1_dungeon_loader_pc34_compat.c instead of a DM1 path.";

static const CSB_V1_ViewportF0108FootprintsPc34Contract s_contract = {
    CSB_PRESENT,
    CSB_FOOTPRINT_MASK,
    CSB_FOOTPRINT_INDEX,
    CSB_FOOTPRINT_ORDINAL,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_VIEW_FLOOR_D3L2,
    CSB_VIEW_FLOOR_D3R2,
    CSB_VIEW_FLOOR_D1C,
    CSB_VIEW_FLOOR_D2C,
    CSB_VIEW_FLOOR_D3C,
    CSB_ZONE_FLOOR_ORNAMENT,
    CSB_COORDINATE_SET_STRIDE,
    CSB_COORDINATE_SET_INDEX,
    CSB_COLOR_FLESH,
    CSB_FLIP_HORIZONTAL,
    "ReDMCSB DUNVIEW.C:F0108_DUNGEONVIEW_DrawFloorOrnament:3940-4011",
    "ReDMCSB DUNGEON.C:F0172_DUNGEON_SetSquareAspect:2666-2718; "
    "F0174_DUNGEON_SetCurrentMapAndPartyMap:2755-2760",
    "ReDMCSB DEFS.H:2465 C15_FLOOR_ORNAMENT_FOOTPRINTS; 2561 "
    "MASK0x8000_FOOTPRINTS; 2749-2760 CSB/I34 floor views; 4223 C1500",
    "CSB/I34 D3L2/D3R2 floor views use C00/C01 and the F0108 11-zone "
    "floor-ornament namespace; Firestaff implementation path is src/csb/",
    s_source_evidence
};

static int is_right_floor_view(int view_floor_index)
{
    return view_floor_index == CSB_VIEW_FLOOR_D3R2 ||
           view_floor_index == CSB_VIEW_FLOOR_D3R ||
           view_floor_index == CSB_VIEW_FLOOR_D2R ||
           view_floor_index == CSB_VIEW_FLOOR_D1R;
}

static int is_center_floor_view(int view_floor_index)
{
    return view_floor_index == CSB_VIEW_FLOOR_D3C ||
           view_floor_index == CSB_VIEW_FLOOR_D2C ||
           view_floor_index == CSB_VIEW_FLOOR_D1C;
}

static int zone_for_view(int view_floor_index)
{
    if (view_floor_index < 0) return -1;
    return CSB_ZONE_FLOOR_ORNAMENT +
           (CSB_COORDINATE_SET_INDEX * CSB_COORDINATE_SET_STRIDE) +
           view_floor_index;
}

static int flip_for_draw(int ornament_index, int view_floor_index, int use_flipped)
{
    if (is_right_floor_view(view_floor_index)) return CSB_FLIP_HORIZONTAL;
    if (ornament_index == CSB_FOOTPRINT_INDEX &&
        use_flipped &&
        is_center_floor_view(view_floor_index)) {
        return CSB_FLIP_HORIZONTAL;
    }
    return 0;
}

const CSB_V1_ViewportF0108FootprintsPc34Contract *
csb_v1_viewport_f0108_footprints_contract_pc34(void)
{
    return &s_contract;
}

const char *
csb_v1_viewport_f0108_footprints_source_evidence_pc34(void)
{
    return s_source_evidence;
}

int csb_v1_viewport_f0108_footprints_plan_pc34(
    uint16_t floor_ornament_ordinal,
    int view_floor_index,
    int use_flipped_wall_and_footprints_bitmaps,
    CSB_V1_ViewportF0108FootprintsPc34Plan *out_plan)
{
    CSB_V1_ViewportF0108FootprintsPc34Plan plan;
    const int has_footprints =
        (floor_ornament_ordinal & (uint16_t)CSB_FOOTPRINT_MASK) != 0;
    const int cleared_ordinal =
        (int)(floor_ornament_ordinal & (uint16_t)~CSB_FOOTPRINT_MASK);

    if (!out_plan || view_floor_index < 0) return -1;

    plan.ok = 0;
    plan.draw_count = 0;
    plan.base_drawn = 0;
    plan.footprints_drawn = 0;
    plan.base_ornament_index = -1;
    plan.footprints_ornament_index = -1;
    plan.cleared_base_ordinal = cleared_ordinal;
    plan.recursive_ordinal = 0;
    plan.recursive_view_floor = -1;
    plan.base_zone = -1;
    plan.footprints_zone = -1;
    plan.base_flip = 0;
    plan.footprints_flip = 0;
    plan.base_transparent_color = CSB_COLOR_FLESH;
    plan.footprints_transparent_color = CSB_COLOR_FLESH;
    plan.recursion_stops = 1;
    plan.source_evidence = s_source_evidence;

    /* ReDMCSB: DUNVIEW.C F0108 lines 3959-3962; ordinal 0 never reaches
     * bitmap lookup, zone calculation, or the recursive footprints call. */
    if (floor_ornament_ordinal == 0) {
        plan.ok = 1;
        *out_plan = plan;
        return 0;
    }

    /* ReDMCSB: DUNVIEW.C F0108 lines 3960-3966 clear MASK0x8000 before
     * pre-decrementing the base ordinal into a current-map ornament index. */
    if (cleared_ordinal != 0) {
        plan.base_drawn = 1;
        plan.base_ornament_index = cleared_ordinal - 1;
        plan.base_zone = zone_for_view(view_floor_index);
        plan.base_flip = flip_for_draw(plan.base_ornament_index,
                                       view_floor_index,
                                       use_flipped_wall_and_footprints_bitmaps);
        ++plan.draw_count;
    }

    if (has_footprints) {
        /* ReDMCSB: DUNVIEW.C F0108 lines 4007-4008 recurse after the base
         * draw, preserving P0119_ui_ViewFloorIndex and using ordinal 16. */
        plan.footprints_drawn = 1;
        plan.footprints_ornament_index = CSB_FOOTPRINT_INDEX;
        plan.recursive_ordinal = CSB_FOOTPRINT_ORDINAL;
        plan.recursive_view_floor = view_floor_index;
        plan.footprints_zone = zone_for_view(view_floor_index);
        plan.footprints_flip = flip_for_draw(CSB_FOOTPRINT_INDEX,
                                             view_floor_index,
                                             use_flipped_wall_and_footprints_bitmaps);
        ++plan.draw_count;
    }

    plan.ok = (plan.draw_count > 0) &&
              (!has_footprints ||
               (plan.footprints_drawn &&
                plan.recursive_ordinal == CSB_FOOTPRINT_ORDINAL &&
                plan.recursive_view_floor == view_floor_index &&
                plan.recursion_stops));

    *out_plan = plan;
    return plan.ok ? 0 : 1;
}
