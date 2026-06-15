/*
 * DM1 V1 champion panel F0354 box-variants gate implementation.
 *
 * Source-lock anchors (ReDMCSB WIP 20210206):
 *  - PANEL.C F0354:2195-2242 (the entire F0354 function) owns the
 *    box-variant selection for the inventory-portrait blit.
 *  - PANEL.C F0354:2208-2213 PC 3.4 byte-coordinate box:
 *        M770_BOX_TOP = 0;
 *        M771_BOX_BOTTOM = 28;
 *        M768_BOX_LEFT = (championIndex * C69_CHAMPION_STATUS_BOX_SPACING)
 *                        + 7;
 *        M769_BOX_RIGHT = M768_BOX_LEFT + 31;
 *    Stride: DEFS.H:2157 C69_CHAMPION_STATUS_BOX_SPACING = 69.
 *    Width: 32 pixels (M769 - M768 + 1 = 31 - 0 + 1 = 32), matching
 *    DEFS.H:6391 G2078_C32_PortraitWidth.
 *    Height: 29 pixels (M771 - M770 + 1 = 28 - 0 + 1 = 29), matching
 *    DEFS.H:6392 G2079_C29_PortraitHeight.
 *  - PANEL.C F0354:2213 blit signature:
 *        F0021_MAIN_BlitToScreen(Portrait, L1101_ai_Box, C016_BYTE_WIDTH,
 *                                CM1_COLOR_NO_TRANSPARENCY);
 *    DEFS.H:2471 C016_BYTE_WIDTH = 16.
 *  - PANEL.C F0354:2222-2226 zone-table box:
 *        F0638_GetZone(C175_ZONE_FIRST_CHAMPION_STATUS_BOX +
 *                      championIndex, L2868_ai_XYZ);
 *    DEFS.H:3793 C175_ZONE_FIRST_CHAMPION_STATUS_BOX = 175.
 *  - PANEL.C F0354:2237-2241 post-blit invisibility hatch:
 *        if (G0407_s_Party.Event71Count_Invisibility) {
 *            F0136_VIDEO_HatchScreenBox(
 *                C175_ZONE_FIRST_CHAMPION_STATUS_BOX + championIndex,
 *                C12_COLOR_DARKEST_GRAY);
 *        }
 *    DEFS.H C12_COLOR_DARKEST_GRAY = 12.
 *  - DEFS.H:825-826 (M027_PORTRAIT_X / M028_PORTRAIT_Y) plus
 *    DEFS.H:6391-6392 (G2078_C32_PortraitWidth / G2079_C29_PortraitHeight)
 *    define the 32x29 portrait that fills the byte-coordinate box and
 *    the zone box.
 *
 * No bitmap sampling, no GRAPHICS.DAT / DUNGEON.DAT load, no real-asset
 * parity claim. This gate is the post-dispatch geometry / variant
 * contract for F0354; the F0292 -> F0354 dispatch predicate is pinned
 * by `dm1_v1_champion_panel_portrait_box_blit_gate_pc34_compat`.
 */

#include "firestaff/dm1/v1/champion_panel/dm1_v1_champion_panel_f0354_box_variants_pc34_compat.h"

#include <string.h>

static const char s_source_evidence[] =
    "contract_only=1; no real-asset bitmap parity claim; no GRAPHICS.DAT or "
    "DUNGEON.DAT load. ReDMCSB PANEL.C F0354:2195-2242 is the entire "
    "F0354_INVENTORY_DrawStatusBoxPortrait function and owns the box-"
    "variant selection. ReDMCSB PANEL.C F0354:2208-2213 sets "
    "M770_BOX_TOP=0, M771_BOX_BOTTOM=28, M768_BOX_LEFT=(championIndex*"
    "C69_CHAMPION_STATUS_BOX_SPACING)+7, M769_BOX_RIGHT=M768_BOX_LEFT+31, "
    "and calls F0021_MAIN_BlitToScreen(Portrait, L1101_ai_Box, "
    "C016_BYTE_WIDTH, CM1_COLOR_NO_TRANSPARENCY). ReDMCSB DEFS.H:2157 "
    "anchors C69_CHAMPION_STATUS_BOX_SPACING=69. ReDMCSB DEFS.H:2471 "
    "anchors C016_BYTE_WIDTH=16. ReDMCSB DEFS.H:6391-6392 anchor "
    "G2078_C32_PortraitWidth=32 and G2079_C29_PortraitHeight=29, which "
    "match the 32x29 box implied by the byte-coordinate left+31/right "
    "and top=0/bottom=28 endpoints. ReDMCSB PANEL.C F0354:2222-2226 "
    "calls F0638_GetZone(C175_ZONE_FIRST_CHAMPION_STATUS_BOX + "
    "championIndex, L2868_ai_XYZ) for the zone-based box variant used "
    "by Amiga/console ports (MEDIA529/MEDIA463/MEDIA543/MEDIA746). "
    "ReDMCSB DEFS.H:3793 anchors C175_ZONE_FIRST_CHAMPION_STATUS_BOX=175. "
    "ReDMCSB PANEL.C F0354:2237-2241 fires "
    "F0136_VIDEO_HatchScreenBox(C175+championIndex, C12_COLOR_DARKEST_GRAY) "
    "when G0407_s_Party.Event71Count_Invisibility is non-zero "
    "(MEDIA720). The hatch is additive to the byte-coordinate or zone "
    "blit, never a replacement. This fixture is the post-dispatch "
    "geometry / variant contract; the F0292 -> F0354 dispatch predicate "
    "is pinned separately by the existing portrait-box blit dispatch "
    "gate.";

static uint32_t hash_step(uint32_t hash, unsigned int value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (uint32_t)((value >> (i * 8)) & 0xffu);
        hash *= UINT32_C(16777619);
    }
    return hash;
}

static bool is_valid_champion_index(int champion_index)
{
    return champion_index >= 0 &&
           champion_index < DM1_V1_CPFBV_CHAMPION_COUNT_PC34;
}

static uint32_t hash_champion_box(const DM1_V1_CPFBV_ChampionBoxPc34Compat *box)
{
    uint32_t hash = UINT32_C(2166136261);

    if (!box) {
        return hash;
    }
    hash = hash_step(hash, (unsigned int)box->champion_index);
    hash = hash_step(hash, (unsigned int)box->byte_left);
    hash = hash_step(hash, (unsigned int)box->byte_top);
    hash = hash_step(hash, (unsigned int)box->byte_right);
    hash = hash_step(hash, (unsigned int)box->byte_bottom);
    hash = hash_step(hash, (unsigned int)box->byte_width);
    hash = hash_step(hash, (unsigned int)box->byte_height);
    hash = hash_step(hash, (unsigned int)box->portrait_zone);
    hash = hash_step(hash, (unsigned int)box->zone_left);
    hash = hash_step(hash, (unsigned int)box->zone_top);
    hash = hash_step(hash, (unsigned int)box->zone_right);
    hash = hash_step(hash, (unsigned int)box->zone_bottom);
    hash = hash_step(hash, (unsigned int)box->event71_count_invisibility);
    hash = hash_step(hash, box->hatch_applies ? 1u : 0u);
    hash = hash_step(hash, (unsigned int)box->hatch_zone);
    hash = hash_step(hash, (unsigned int)box->hatch_color);
    hash = hash_step(hash, (unsigned int)box->variant);
    hash = hash_step(hash, box->reached_f0354 ? 1u : 0u);
    hash = hash_step(hash, box->portrait_zone_in_range ? 1u : 0u);
    hash = hash_step(hash, box->byte_box_in_range ? 1u : 0u);
    hash = hash_step(hash, box->hatch_zone_in_range ? 1u : 0u);
    return hash;
}

void dm1_v1_cpfbv_default_pc34(DM1_V1_CPFBV_ModelPc34Compat *out_model)
{
    if (!out_model) {
        return;
    }
    memset(out_model, 0, sizeof(*out_model));
    out_model->contract_only = true;
    out_model->disjoint_from_portrait_box_blit_dispatch_gate = true;
    out_model->disjoint_from_portrait_state_redraw_pc34_compat = true;
    out_model->disjoint_from_mouth_eye_release_pc34_compat = true;
    out_model->disjoint_from_food_water_status_box_pc34_compat = true;
    out_model->disjoint_from_hud_recompute_pc34_compat = true;
    out_model->disjoint_from_action_hand_slot_priority_pc34_compat = true;
    out_model->disjoint_from_champion_panel_pixels_runtime_probe = true;
    out_model->disjoint_from_champion_panel_status_states_runtime_probe =
        true;
    out_model->disjoint_from_champion_panel_status_hand_slot_pixels_source_lock =
        true;
    out_model->disjoint_from_champion_panel_partial_party_pixel_probe = true;
    out_model->disjoint_from_champion_panel_status_box_asset_slice_probe =
        true;
    out_model->disjoint_from_champion_panel_recompute_runtime = true;
    out_model->disjoint_from_champion_panel_leader_rotation_pixel_slice =
        true;
    out_model->disjoint_from_champion_panel_action_cell_slotbox_runtime =
        true;
    out_model->disjoint_from_champion_panel_hand_slot_priority_source_lock =
        true;
    out_model->disjoint_from_champion_panel_shield_border_pixel = true;
    out_model
        ->disjoint_from_champion_panel_pressing_mouth_eye_statusbox_pc34_compat =
        true;
    out_model->disjoint_from_inventory_champion_switch_hand_carry_pc34_compat =
        true;
    out_model->deterministic_hash = UINT32_C(2166136261);
}

void dm1_v1_cpfbv_build_byte_coord_pc34(
    int event71_count_invisibility,
    int champion_index,
    DM1_V1_CPFBV_ChampionBoxPc34Compat *out_box)
{
    if (!out_box) {
        return;
    }
    memset(out_box, 0, sizeof(*out_box));

    out_box->champion_index = champion_index;
    out_box->reached_f0354 = is_valid_champion_index(champion_index);
    out_box->portrait_zone =
        DM1_V1_CPFBV_PORTRAIT_ZONE_BASE_PC34 + champion_index;
    out_box->portrait_zone_in_range =
        is_valid_champion_index(champion_index) &&
        out_box->portrait_zone == DM1_V1_CPFBV_PORTRAIT_ZONE_BASE_PC34 +
                                       champion_index;

    /*
     * ReDMCSB PANEL.C F0354:2208-2211 byte-coordinate box formula:
     *   M770_BOX_TOP = 0
     *   M771_BOX_BOTTOM = 28
     *   M768_BOX_LEFT = (championIndex * C69) + 7
     *   M769_BOX_RIGHT = M768_BOX_LEFT + 31
     * The 7 is the "left margin" inside the 67-pixel status box
     * (the status box is C151+championIndex stride 69 wide, and the
     * portrait uses the right 32-pixel column starting 7 pixels in).
     * 32+7+30 = 69 so the right column edge of the portrait ends
     * exactly at the right edge of the status box.
     */
    out_box->byte_left =
        (champion_index * DM1_V1_CPFBV_STATUS_BOX_SPACING_PC34) +
        DM1_V1_CPFBV_BYTE_COORD_LEFT_MARGIN_PC34;
    out_box->byte_top = DM1_V1_CPFBV_BYTE_COORD_TOP_PC34;
    out_box->byte_right =
        out_box->byte_left + DM1_V1_CPFBV_BYTE_COORD_RIGHT_INSET_PC34;
    out_box->byte_bottom = DM1_V1_CPFBV_BYTE_COORD_BOTTOM_PC34;
    out_box->byte_width = DM1_V1_CPFBV_PORTRAIT_WIDTH_PC34;
    out_box->byte_height = DM1_V1_CPFBV_PORTRAIT_HEIGHT_PC34;
    out_box->byte_box_in_range =
        is_valid_champion_index(champion_index) &&
        (out_box->byte_right - out_box->byte_left +
         1) == DM1_V1_CPFBV_PORTRAIT_WIDTH_PC34 &&
        (out_box->byte_bottom - out_box->byte_top +
         1) == DM1_V1_CPFBV_PORTRAIT_HEIGHT_PC34;

    /* The byte-coordinate variant does NOT call F0638_GetZone, so
     * the zone table box is left at zero. The byte-coord and zone
     * variants are mutually exclusive at build time, not runtime.
     */
    out_box->zone_left = 0;
    out_box->zone_top = 0;
    out_box->zone_right = 0;
    out_box->zone_bottom = 0;

    out_box->event71_count_invisibility = event71_count_invisibility;
    out_box->hatch_applies = event71_count_invisibility > 0;
    out_box->hatch_zone = out_box->portrait_zone;
    out_box->hatch_color = DM1_V1_CPFBV_HATCH_COLOR_PC34;
    out_box->hatch_zone_in_range = is_valid_champion_index(champion_index);

    out_box->variant = out_box->reached_f0354
                           ? DM1_V1_CPFBV_VARIANT_PC34_BYTE_COORD_PC34
                           : DM1_V1_CPFBV_VARIANT_PC34_NOT_REACHED_PC34;

    out_box->hash = hash_champion_box(out_box);
}

void dm1_v1_cpfbv_build_zone_table_pc34(
    int event71_count_invisibility,
    int champion_index,
    int zone_left,
    int zone_top,
    int zone_right,
    int zone_bottom,
    DM1_V1_CPFBV_ChampionBoxPc34Compat *out_box)
{
    if (!out_box) {
        return;
    }
    memset(out_box, 0, sizeof(*out_box));

    out_box->champion_index = champion_index;
    out_box->reached_f0354 = is_valid_champion_index(champion_index);
    out_box->portrait_zone =
        DM1_V1_CPFBV_PORTRAIT_ZONE_BASE_PC34 + champion_index;
    out_box->portrait_zone_in_range =
        is_valid_champion_index(champion_index) &&
        out_box->portrait_zone == DM1_V1_CPFBV_PORTRAIT_ZONE_BASE_PC34 +
                                       champion_index;

    /*
     * ReDMCSB PANEL.C F0354:2222-2226 zone-table box.
     * F0638_GetZone(C175 + championIndex, L2868_ai_XYZ) writes a
     * 4-element box (X1, X2, Y1, Y2) into the caller's array. The
     * fixture takes the zone box from the caller; the byte-coordinate
     * fields are still anchored (PC 3.4 always computes them, even
     * on the zone path) but the box actually consumed by the blit is
     * the zone-table box, not the byte-coord box.
     */
    out_box->byte_left =
        (champion_index * DM1_V1_CPFBV_STATUS_BOX_SPACING_PC34) +
        DM1_V1_CPFBV_BYTE_COORD_LEFT_MARGIN_PC34;
    out_box->byte_top = DM1_V1_CPFBV_BYTE_COORD_TOP_PC34;
    out_box->byte_right =
        out_box->byte_left + DM1_V1_CPFBV_BYTE_COORD_RIGHT_INSET_PC34;
    out_box->byte_bottom = DM1_V1_CPFBV_BYTE_COORD_BOTTOM_PC34;
    out_box->byte_width = DM1_V1_CPFBV_PORTRAIT_WIDTH_PC34;
    out_box->byte_height = DM1_V1_CPFBV_PORTRAIT_HEIGHT_PC34;
    out_box->byte_box_in_range =
        is_valid_champion_index(champion_index) &&
        (out_box->byte_right - out_box->byte_left +
         1) == DM1_V1_CPFBV_PORTRAIT_WIDTH_PC34 &&
        (out_box->byte_bottom - out_box->byte_top +
         1) == DM1_V1_CPFBV_PORTRAIT_HEIGHT_PC34;

    out_box->zone_left = zone_left;
    out_box->zone_top = zone_top;
    out_box->zone_right = zone_right;
    out_box->zone_bottom = zone_bottom;

    out_box->event71_count_invisibility = event71_count_invisibility;
    out_box->hatch_applies = event71_count_invisibility > 0;
    out_box->hatch_zone = out_box->portrait_zone;
    out_box->hatch_color = DM1_V1_CPFBV_HATCH_COLOR_PC34;
    out_box->hatch_zone_in_range = is_valid_champion_index(champion_index);

    out_box->variant = out_box->reached_f0354
                           ? DM1_V1_CPFBV_VARIANT_PC34_ZONE_TABLE_PC34
                           : DM1_V1_CPFBV_VARIANT_PC34_NOT_REACHED_PC34;

    out_box->hash = hash_champion_box(out_box);
}

void dm1_v1_cpfbv_build_hatch_pc34(
    int event71_count_invisibility,
    int champion_index,
    DM1_V1_CPFBV_ChampionBoxPc34Compat *out_box)
{
    if (!out_box) {
        return;
    }
    memset(out_box, 0, sizeof(*out_box));

    out_box->champion_index = champion_index;
    out_box->reached_f0354 = is_valid_champion_index(champion_index);
    out_box->portrait_zone =
        DM1_V1_CPFBV_PORTRAIT_ZONE_BASE_PC34 + champion_index;
    out_box->portrait_zone_in_range =
        is_valid_champion_index(champion_index);

    /*
     * The hatch variant is "the byte-coord or zone blit already ran,
     * and we are now adding the post-blit invisibility hatch". The
     * byte-coord box is still recorded for completeness; the variant
     * tag marks the gate as having *selected* the hatch branch.
     */
    out_box->byte_left =
        (champion_index * DM1_V1_CPFBV_STATUS_BOX_SPACING_PC34) +
        DM1_V1_CPFBV_BYTE_COORD_LEFT_MARGIN_PC34;
    out_box->byte_top = DM1_V1_CPFBV_BYTE_COORD_TOP_PC34;
    out_box->byte_right =
        out_box->byte_left + DM1_V1_CPFBV_BYTE_COORD_RIGHT_INSET_PC34;
    out_box->byte_bottom = DM1_V1_CPFBV_BYTE_COORD_BOTTOM_PC34;
    out_box->byte_width = DM1_V1_CPFBV_PORTRAIT_WIDTH_PC34;
    out_box->byte_height = DM1_V1_CPFBV_PORTRAIT_HEIGHT_PC34;
    out_box->byte_box_in_range =
        is_valid_champion_index(champion_index) &&
        (out_box->byte_right - out_box->byte_left +
         1) == DM1_V1_CPFBV_PORTRAIT_WIDTH_PC34 &&
        (out_box->byte_bottom - out_box->byte_top +
         1) == DM1_V1_CPFBV_PORTRAIT_HEIGHT_PC34;

    out_box->event71_count_invisibility = event71_count_invisibility;
    out_box->hatch_applies =
        out_box->reached_f0354 && (event71_count_invisibility > 0);
    out_box->hatch_zone = out_box->portrait_zone;
    out_box->hatch_color = DM1_V1_CPFBV_HATCH_COLOR_PC34;
    out_box->hatch_zone_in_range = is_valid_champion_index(champion_index);

    out_box->variant = (out_box->hatch_applies)
                           ? DM1_V1_CPFBV_VARIANT_PC34_HATCH_PC34
                           : (out_box->reached_f0354
                                  ? DM1_V1_CPFBV_VARIANT_PC34_BYTE_COORD_PC34
                                  : DM1_V1_CPFBV_VARIANT_PC34_NOT_REACHED_PC34);

    out_box->hash = hash_champion_box(out_box);
}

void dm1_v1_cpfbv_build_model_pc34(
    int event71_count_invisibility,
    DM1_V1_CPFBV_ModelPc34Compat *out_model)
{
    int i;

    if (!out_model) {
        return;
    }
    dm1_v1_cpfbv_default_pc34(out_model);

    /*
     * Build the four-champion PC 3.4 byte-coordinate box set; the
     * hatch status is shared across all four champions (Event71 is
     * a party-wide spell, so all four champions get the hatch when
     * active).
     */
    for (i = 0; i < DM1_V1_CPFBV_CHAMPION_COUNT_PC34; ++i) {
        dm1_v1_cpfbv_build_byte_coord_pc34(
            event71_count_invisibility, i, &out_model->champions[i]);
    }

    out_model->deterministic_hash = UINT32_C(2166136261);
    for (i = 0; i < DM1_V1_CPFBV_CHAMPION_COUNT_PC34; ++i) {
        out_model->deterministic_hash =
            hash_step(out_model->deterministic_hash,
                      out_model->champions[i].hash);
    }
}

const char *dm1_v1_cpfbv_source_evidence_pc34(void)
{
    return s_source_evidence;
}
