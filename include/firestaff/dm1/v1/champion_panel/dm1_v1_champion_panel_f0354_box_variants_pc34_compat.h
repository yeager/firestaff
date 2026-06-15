#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_F0354_BOX_VARIANTS_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_F0354_BOX_VARIANTS_PC34_COMPAT_H

/*
 * DM1 V1 champion panel F0354 box-variants gate.
 *
 * Contract-only, no-asset fixture. This pins the *box geometry and
 * variant selection* of PANEL.C F0354_INVENTORY_DrawStatusBoxPortrait,
 * which is the narrow slice left over once the F0292 -> F0354 dispatch
 * predicate is already pinned by the existing portrait-box blit
 * dispatch gate.
 *
 * ReDMCSB source anchors:
 * - PANEL.C F0354:2195-2242 (the entire F0354 function) is the call
 *   site. The function resolves the destination box in one of two
 *   shapes:
 *     1) PC 3.4 byte-coordinate box at lines 2208-2213:
 *        M770_BOX_TOP = 0;
 *        M771_BOX_BOTTOM = 28;
 *        M768_BOX_LEFT = (championIndex * C69_CHAMPION_STATUS_BOX_SPACING)
 *                        + 7;
 *        M769_BOX_RIGHT = M768_BOX_LEFT + 31;
 *        F0021_MAIN_BlitToScreen(..., C016_BYTE_WIDTH,
 *                                CM1_COLOR_NO_TRANSPARENCY);
 *     2) Zone-based box at lines 2222-2226:
 *        F0638_GetZone(C175_ZONE_FIRST_CHAMPION_STATUS_BOX +
 *                      championIndex, L2868_ai_XYZ);
 *   The two shapes are mutually exclusive at build time (the byte-
 *   coordinate shape lives inside MEDIA008/MEDIA413 and the zone shape
 *   lives inside MEDIA529/MEDIA463/MEDIA543/MEDIA746).
 * - PANEL.C F0354:2237-2241 owns the post-blit invisibility hatch:
 *        if (G0407_s_Party.Event71Count_Invisibility) {
 *            F0136_VIDEO_HatchScreenBox(
 *                C175_ZONE_FIRST_CHAMPION_STATUS_BOX + championIndex,
 *                C12_COLOR_DARKEST_GRAY);
 *        }
 *   The hatch is *additive* to either box shape: it is drawn over the
 *   blit, never instead of it. MEDIA720 gates this block.
 * - DEFS.H:825-826 (M027_PORTRAIT_X / M028_PORTRAIT_Y) plus
 *   DEFS.H:6391-6392 (G2078_C32_PortraitWidth / G2079_C29_PortraitHeight)
 *   define the 32x29 portrait that fills the byte-coordinate box and
 *   the zone box.
 * - DEFS.H:2157 (C69_CHAMPION_STATUS_BOX_SPACING = 69) and
 *   DEFS.H:2471 (C016_BYTE_WIDTH = 16) plus
 *   DEFS.H:3793 (C175_ZONE_FIRST_CHAMPION_STATUS_BOX = 175) anchor the
 *   byte-coordinate stride, the screen-bytes-per-row, and the
 *   zone-table base.
 *
 * This fixture does NOT pin the F0292 -> F0354 dispatch predicate
 * (CHAMDRAW.C F0292:757-1110 + TIMELINE.C F0254:1614-1637 +
 * CHAMDRAW.C F0293:1117-1143). That predicate is pinned by
 * `dm1_v1_champion_panel_portrait_box_blit_gate_pc34_compat`. The
 * present gate is the *post-dispatch* geometry/variant contract that
 * has to hold for the inventory portrait blit to land in the right
 * zone, in the right box shape, with the right optional hatch.
 */

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_CPFBV_CHAMPION_COUNT_PC34 4
#define DM1_V1_CPFBV_PORTRAIT_WIDTH_PC34 32
#define DM1_V1_CPFBV_PORTRAIT_HEIGHT_PC34 29
#define DM1_V1_CPFBV_BYTE_COORD_LEFT_MARGIN_PC34 7
#define DM1_V1_CPFBV_BYTE_COORD_TOP_PC34 0
#define DM1_V1_CPFBV_BYTE_COORD_BOTTOM_PC34 28
#define DM1_V1_CPFBV_BYTE_COORD_RIGHT_INSET_PC34 31

/* ReDMCSB DEFS.H:2157, DEFS.H:2471, DEFS.H:3793. */
#define DM1_V1_CPFBV_STATUS_BOX_SPACING_PC34 69
#define DM1_V1_CPFBV_BYTE_WIDTH_PC34 16
#define DM1_V1_CPFBV_PORTRAIT_ZONE_BASE_PC34 175

/* ReDMCSB DEFS.H:2471 / DEFS.H:3783-3793. */
#define DM1_V1_CPFBV_HATCH_COLOR_PC34 12

typedef enum {
    DM1_V1_CPFBV_VARIANT_PC34_BYTE_COORD_PC34 = 0,
    DM1_V1_CPFBV_VARIANT_PC34_ZONE_TABLE_PC34 = 1,
    DM1_V1_CPFBV_VARIANT_PC34_HATCH_PC34 = 2,
    DM1_V1_CPFBV_VARIANT_PC34_NOT_REACHED_PC34 = 3
} DM1_V1_CPFBV_VariantPc34Compat;

typedef struct {
    int champion_index;

    /* Per-champion byte-coordinate box, computed as
     *   left   = champion_index * STATUS_BOX_SPACING + LEFT_MARGIN
     *   top    = BYTE_COORD_TOP (0)
     *   right  = left + BYTE_COORD_RIGHT_INSET
     *   bottom = BYTE_COORD_BOTTOM
     * The 32x29 size matches M027/M028 + G2078_C32 + G2079_C29.
     */
    int byte_left;
    int byte_top;
    int byte_right;
    int byte_bottom;
    int byte_width;
    int byte_height;

    /* Per-champion zone index (C175 + champion_index). */
    int portrait_zone;

    /* Per-champion zone table box: synthesized by
     * F0638_GetZone(C175 + champion_index, ...). The fixture uses
     * the byte-coordinate left as a deterministic zone-box left
     * anchor; this is the contract-only stand-in for the zone
     * table that the Amiga/console ports read at runtime.
     */
    int zone_left;
    int zone_top;
    int zone_right;
    int zone_bottom;

    /* Invisibility hatch state and zone target. */
    int event71_count_invisibility;
    bool hatch_applies;
    int hatch_zone;
    int hatch_color;

    /* Selected variant for this champion. */
    DM1_V1_CPFBV_VariantPc34Compat variant;

    /* Stability / dispatch correctness. */
    bool reached_f0354;
    bool portrait_zone_in_range;
    bool byte_box_in_range;
    bool hatch_zone_in_range;
    uint32_t hash;
} DM1_V1_CPFBV_ChampionBoxPc34Compat;

typedef struct {
    bool contract_only;
    bool disjoint_from_portrait_box_blit_dispatch_gate;
    bool disjoint_from_portrait_state_redraw_pc34_compat;
    bool disjoint_from_mouth_eye_release_pc34_compat;
    bool disjoint_from_food_water_status_box_pc34_compat;
    bool disjoint_from_hud_recompute_pc34_compat;
    bool disjoint_from_action_hand_slot_priority_pc34_compat;
    bool disjoint_from_champion_panel_pixels_runtime_probe;
    bool disjoint_from_champion_panel_status_states_runtime_probe;
    bool disjoint_from_champion_panel_status_hand_slot_pixels_source_lock;
    bool disjoint_from_champion_panel_partial_party_pixel_probe;
    bool disjoint_from_champion_panel_status_box_asset_slice_probe;
    bool disjoint_from_champion_panel_recompute_runtime;
    bool disjoint_from_champion_panel_leader_rotation_pixel_slice;
    bool disjoint_from_champion_panel_action_cell_slotbox_runtime;
    bool disjoint_from_champion_panel_hand_slot_priority_source_lock;
    bool disjoint_from_champion_panel_shield_border_pixel;
    bool disjoint_from_champion_panel_pressing_mouth_eye_statusbox_pc34_compat;
    bool disjoint_from_inventory_champion_switch_hand_carry_pc34_compat;
    DM1_V1_CPFBV_ChampionBoxPc34Compat
        champions[DM1_V1_CPFBV_CHAMPION_COUNT_PC34];
    uint32_t deterministic_hash;
} DM1_V1_CPFBV_ModelPc34Compat;

void dm1_v1_cpfbv_default_pc34(DM1_V1_CPFBV_ModelPc34Compat *out_model);

void dm1_v1_cpfbv_build_byte_coord_pc34(
    int event71_count_invisibility,
    int champion_index,
    DM1_V1_CPFBV_ChampionBoxPc34Compat *out_box);

void dm1_v1_cpfbv_build_zone_table_pc34(
    int event71_count_invisibility,
    int champion_index,
    int zone_left,
    int zone_top,
    int zone_right,
    int zone_bottom,
    DM1_V1_CPFBV_ChampionBoxPc34Compat *out_box);

void dm1_v1_cpfbv_build_hatch_pc34(
    int event71_count_invisibility,
    int champion_index,
    DM1_V1_CPFBV_ChampionBoxPc34Compat *out_box);

void dm1_v1_cpfbv_build_model_pc34(
    int event71_count_invisibility,
    DM1_V1_CPFBV_ModelPc34Compat *out_model);

const char *dm1_v1_cpfbv_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHAMPION_PANEL_F0354_BOX_VARIANTS_PC34_COMPAT_H */
