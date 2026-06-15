#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_NAME_BOX_CLIP_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_NAME_BOX_CLIP_PC34_COMPAT_H

/*
 * DM1 V1 champion-panel name-box clip contract gate.
 *
 * Contract-only, no-asset fixture. This pins the *clip geometry* of
 * the champion name strip on the 67x29 status box (CHAMDRAW.C F0292)
 * and the *clip behavior* of the champion name string buffer that
 * feeds the F0053_TEXT_PrintToLogicalScreen / F0052_TEXT_PrintToViewport
 * text calls in PC 3.4. It is intentionally disjoint from the
 * F0292 -> F0354 dispatch predicate (covered by
 * `dm1_v1_champion_panel_portrait_box_blit_gate_pc34_compat`), the
 * portrait-box geometry (covered by
 * `dm1_v1_champion_panel_f0354_box_variants_pc34_compat`), the
 * portrait-state redraw matrix (covered by
 * `dm1_v1_champion_panel_portrait_state_redraw_pc34_compat` +
 * `dm1_v1_champion_panel_portrait_box_redraw_states_pc34_compat`),
 * the mouth/eye press release gate, the food/water status-box gate,
 * the HUD recompute gate, the action-hand slot-priority gate, the
 * action-cell slotbox gate, the status-hand slot-pixels gate, the
 * hand-slot priority source lock, and any chest/inventory/mirror
 * runtime regression.
 *
 * The lane name "name_box_clip" refers to two coupled clip
 * contracts that the existing champion-panel gates do not cover
 * together:
 *
 *   1. The status-box top-strip name-box clip. The 7-row-tall name
 *      box at the top of each champion's status box is exactly
 *      43 pixels wide (M770_BOX_TOP=0, M771_BOX_BOTTOM=6,
 *      M768_BOX_LEFT=L0868_i_ChampionStatusBoxX, M769_BOX_RIGHT=
 *      L0868+42) and the name text is printed at column
 *      L0868+1 inside that 43-pixel box. The 6-pixel-wide PC 3.4
 *      font means the name string is *clipped* to 7 glyphs at
 *      most (7 * 6 = 42, plus 1 pixel of left padding = 43 pixels
 *      total). The lane pins that the C_NAME_FIELD bytes in the
 *      champion struct (DEFS.H:623 Name[8]) bound the visible name
 *      to 7 chars, exactly matching the 43-pixel box right edge.
 *
 *   2. The inventory viewport name/title clip. When the
 *      L0863_B_IsInventoryChampion branch is taken, the F0052
 *      viewport call at line 855 prints the name at column 3
 *      (an 8x8 font baseline), the L0869_i_ChampionTitleX for the
 *      title is computed as `6 * strlen(name) + 3` (PC 3.4
 *      glyph width 6), and the title receives an additional
 *      `+6` (one extra glyph) when the title's first character is
 *      not one of {',', ';', '-'}. The lane pins the column 3
 *      name print + the title-X = 6*strlen(name)+3 + 6-when-
 *      non-punctuation contract, which is the second clip rule
 *      the existing f0354_box_variants / portrait-state / mouth-eye
 *      / HUD gates leave uncovered.
 *
 *   3. The dead-champion name-strip clip. When
 *      L0865_ps_Champion->CurrentHealth is 0, the F0292 dead
 *      branch at CHAMDRAW.C F0292:818-833 prints the name
 *      through three sibling code paths (the MEDIA001 x=1+L0868
 *      path, the MEDIA224 x=L0868+1 path, and the MEDIA529
 *      F0650 zone-centered path) and the lane pins the dead-
 *      champion x-offset (L0868+1 on PC 3.4) + the dead-champion
 *      color cascade (C13_COLOR_LIGHTEST_GRAY foreground,
 *      C01_COLOR_DARK_GRAY background) + the C163 zone id.
 *
 * ReDMCSB source anchors:
 *  - CHAMDRAW.C F0292:750 L0868_i_ChampionStatusBoxX =
 *    P0615_ui_ChampionIndex * C69_CHAMPION_STATUS_BOX_SPACING.
 *  - CHAMDRAW.C F0292:818-833 dead-champion name strip
 *    (F0053 at column 1 + L0868 / L0868+1, F0053 at column
 *    L0868+1 with C13/C01 color cascade, F0650 zone centered
 *    on C163_ZONE_FIRST_CHAMPION_NAME + championIndex).
 *  - CHAMDRAW.C F0292:843-895 live non-inventory NAME_TITLE
 *    branch:
 *      M770_BOX_TOP(L0871_ai_Box) = 0;
 *      M771_BOX_BOTTOM(L0871_ai_Box) = 6;
 *      M769_BOX_RIGHT(L0871_ai_Box) = (M768_BOX_LEFT =
 *          L0868_i_ChampionStatusBoxX) + 42;
 *      M524_FillScreenBox(L0871_ai_Box, C01_COLOR_DARK_GRAY);
 *      F0053_TEXT_PrintToLogicalScreen(L0868 + 1, 5,
 *          AL0864_i_ColorIndex, C01_COLOR_DARK_GRAY, Name);
 *  - CHAMDRAW.C F0292:855-871 inventory viewport name/title
 *    branch:
 *      F0052_TEXT_PrintToViewport(3, 7, AL0864_i_ColorIndex, Name);
 *      L0869_i_ChampionTitleX = 6 * strlen(Name) + 3;
 *      if (Title[0] not in {',', ';', '-'})
 *          L0869_i_ChampionTitleX += 6;
 *      F0052_TEXT_PrintToViewport(L0869_i_ChampionTitleX, 7,
 *          AL0864_i_ColorIndex, Title);
 *  - CHAMDRAW.C F0292:845 color cascade (MEDIA049 PC): leader
 *    C09_COLOR_GOLD, non-leader C13_COLOR_LIGHTEST_GRAY.
 *  - DEFS.H:2157 C69_CHAMPION_STATUS_BOX_SPACING = 69.
 *  - DEFS.H:2079 C01_COLOR_DARK_GRAY = 1.
 *  - DEFS.H:2087 C09_COLOR_GOLD = 9.
 *  - DEFS.H:2091 C13_COLOR_LIGHTEST_GRAY = 13.
 *  - DEFS.H:2471 C016_BYTE_WIDTH = 16 (logical screen bytes/row).
 *  - DEFS.H:3787 C159_ZONE_CHAMPION_0_STATUS_BOX_NAME = 159.
 *  - DEFS.H:3791 C163_ZONE_FIRST_CHAMPION_NAME = 163.
 *  - DEFS.H:623 CHAMPION_INCLUDING_PORTRAIT.Name[8] (and
 *    DEFS.H:660 CHAMPION.Name[8] for the no-portrait version) -
 *    the 8-byte field caps the visible name to 7 chars + NUL.
 *
 * The gate is intentionally non-duplicative with the F0292
 * NAME_TITLE strip x-anchor edge (covered by
 * `test_dm1_v1_champion_panel_hud_pc34_compat`'s slot3 name-strip
 * left/right check at lines 422-440), which only pins the byte-
 * coordinate left/right endpoints on slot 3, not the 6px/char
 * glyph width, the L0869 title-X = 6*strlen+3 formula, the dead-
 * champion x=L0868+1 path, the punctuation-passthrough title +
 * 6 increment, the C_NAME_FIELD 7-char byte cap, or the leader
 * color cascade.
 *
 * No bitmap sampling, no GRAPHICS.DAT / DUNGEON.DAT load, no
 * real-asset parity claim. This is a contract-only clip gate.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_CPNBC_CHAMPION_COUNT_PC34 4
#define DM1_V1_CPNBC_GLYPH_WIDTH_PC34 6
#define DM1_V1_CPNBC_NAME_BOX_HEIGHT_PC34 7
#define DM1_V1_CPNBC_NAME_BOX_LEFT_PAD_PC34 1
#define DM1_V1_CPNBC_NAME_BOX_RIGHT_INSET_PC34 42
#define DM1_V1_CPNBC_NAME_BOX_PRINT_Y_PC34 5
#define DM1_V1_CPNBC_NAME_BOX_LEFT_TO_RIGHT_PC34 \
    (DM1_V1_CPNBC_NAME_BOX_RIGHT_INSET_PC34 + 1)
#define DM1_V1_CPNBC_NAME_FIELD_BYTES_PC34 8
#define DM1_V1_CPNBC_NAME_FIELD_VISIBLE_CHARS_PC34 \
    (DM1_V1_CPNBC_NAME_FIELD_BYTES_PC34 - 1)

#define DM1_V1_CPNBC_INVENTORY_NAME_PRINT_X_PC34 3
#define DM1_V1_CPNBC_INVENTORY_NAME_PRINT_Y_PC34 7
#define DM1_V1_CPNBC_INVENTORY_TITLE_X_BASELINE_PC34 3
#define DM1_V1_CPNBC_INVENTORY_TITLE_X_STRIDE_PC34 \
    DM1_V1_CPNBC_GLYPH_WIDTH_PC34
#define DM1_V1_CPNBC_INVENTORY_TITLE_PUNCT_INC_PC34 \
    DM1_V1_CPNBC_GLYPH_WIDTH_PC34

#define DM1_V1_CPNBC_STATUS_BOX_SPACING_PC34 69
#define DM1_V1_CPNBC_STATUS_BOX_NAME_ZONE_BASE_PC34 159
#define DM1_V1_CPNBC_FIRST_CHAMPION_NAME_ZONE_PC34 163
#define DM1_V1_CPNBC_COLOR_NONLEADER_NAME_PC34 13
#define DM1_V1_CPNBC_COLOR_LEADER_NAME_PC34 9
#define DM1_V1_CPNBC_COLOR_NAME_FILL_PC34 1
#define DM1_V1_CPNBC_DEAD_NAME_FG_PC34 13
#define DM1_V1_CPNBC_DEAD_NAME_BG_PC34 1

typedef enum {
    DM1_V1_CPNBC_VARIANT_PC34_STATUS_BOX_LIVE_PC34 = 0,
    DM1_V1_CPNBC_VARIANT_PC34_STATUS_BOX_DEAD_PC34 = 1,
    DM1_V1_CPNBC_VARIANT_PC34_INVENTORY_VIEWPORT_PC34 = 2,
    DM1_V1_CPNBC_VARIANT_PC34_NAME_BOX_NOT_REACHED_PC34 = 3
} DM1_V1_CPNBC_VariantPc34Compat;

typedef struct {
    int champion_index;
    bool is_inventory_champion;
    bool is_dead;
    bool leader_index_match;
    int leader_index;

    /* Status-box top-strip name box (CHAMDRAW.C F0292:879-884). */
    int name_box_left;
    int name_box_top;
    int name_box_right;
    int name_box_bottom;
    int name_box_width;
    int name_box_height;
    int name_print_x;
    int name_print_y;

    /* Status-box top-strip name zone (CHAMDRAW.C F0292:893). */
    int name_zone_index;

    /* Inventory viewport column 3 / column 6*strlen+3 (F0292:855-871). */
    int inventory_name_print_x;
    int inventory_name_print_y;
    int inventory_title_x;
    int inventory_title_x_after_punct;

    /* Color cascade (CHAMDRAW.C F0292:845 MEDIA049 PC). */
    int color_fg;
    int color_bg;

    /* Title punctuation-passthrough flag. */
    int title_first_char;
    bool title_passthrough_no_increment;

    /* Name field clip bookkeeping. */
    int name_field_bytes;
    int name_visible_chars;
    int name_clip_text_pixels;

    /* Computed values for assertion surface. */
    int glyphs_that_fit_in_name_box;
    int status_box_x_anchor;
    bool reached_f0292_name_box;
    bool box_in_range;
    bool zone_in_range;
    bool color_cascade_correct;
    bool name_field_clip_holds;

    DM1_V1_CPNBC_VariantPc34Compat variant;
    uint32_t hash;
} DM1_V1_CPNBC_ChampionNameBoxPc34Compat;

typedef struct {
    bool contract_only;

    /* Disjoint markers - the lane is intentionally not covered by: */
    bool disjoint_from_f0354_box_variants_pc34_compat;
    bool disjoint_from_portrait_box_blit_dispatch_pc34_compat;
    bool disjoint_from_portrait_box_redraw_states_pc34_compat;
    bool disjoint_from_portrait_state_redraw_pc34_compat;
    bool disjoint_from_mouth_eye_release_pc34_compat;
    bool disjoint_from_food_water_status_box_pc34_compat;
    bool disjoint_from_hud_food_water_recompute_pc34_compat;
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
    bool disjoint_from_spell_area_overlay_pc34_compat;
    bool disjoint_from_hud_pc34_compat_name_strip_x_anchor_only;

    DM1_V1_CPNBC_ChampionNameBoxPc34Compat
        champions[DM1_V1_CPNBC_CHAMPION_COUNT_PC34];

    /* Reference name strings used in the assertion surface. */
    const char *seven_char_name;
    const char *punct_title;
    const char *non_punct_title;

    uint32_t deterministic_hash;
} DM1_V1_CPNBC_ModelPc34Compat;

/* --- Build helpers --- */

/* Build the status-box top-strip name box (CHAMDRAW.C F0292:879-884).
 * is_inventory_champion=false, is_dead=false. leader_index_match
 * controls the color cascade (CHAMDRAW.C F0292:845). */
void dm1_v1_cpnbc_build_status_box_live_pc34(
    int champion_index,
    int leader_index,
    const char *name,
    DM1_V1_CPNBC_ChampionNameBoxPc34Compat *out_box);

/* Build the dead-champion name strip (CHAMDRAW.C F0292:818-833). */
void dm1_v1_cpnbc_build_status_box_dead_pc34(
    int champion_index,
    const char *name,
    DM1_V1_CPNBC_ChampionNameBoxPc34Compat *out_box);

/* Build the inventory viewport name/title branch
 * (CHAMDRAW.C F0292:855-871). The title_first_char is the first
 * character of the champion's title string and drives the
 * punctuation-passthrough +6 increment decision. */
void dm1_v1_cpnbc_build_inventory_viewport_pc34(
    int champion_index,
    int leader_index,
    const char *name,
    int title_first_char,
    DM1_V1_CPNBC_ChampionNameBoxPc34Compat *out_box);

/* Build the full four-champion model (the four status-box name
 * strips in PC 3.4). The leader_index argument drives the
 * F0292:845 color cascade. */
void dm1_v1_cpnbc_build_model_pc34(
    int leader_index,
    const char *name,
    DM1_V1_CPNBC_ModelPc34Compat *out_model);

/* Compute the inventory title X = 6 * strlen(name) + 3, with the
 * optional +6 increment when title_first_char is not in
 * {',', ';', '-'}. This is a pure-function stand-in for the
 * L0869_i_ChampionTitleX = 6*strlen+3 [+ 6] formula in
 * CHAMDRAW.C F0292:856-866. */
int dm1_v1_cpnbc_compute_inventory_title_x_pc34(
    const char *name, int title_first_char);

/* Returns true when the title_first_char is one of ',', ';', '-'
 * (CHAMDRAW.C F0292:859-866 punctuation passthrough). */
bool dm1_v1_cpnbc_title_passthrough_punctuation_pc34(int title_first_char);

/* Compute the visible name-text pixel width for a name string
 * clipped to the Name[8] struct field, using the 6-px/char
 * PC 3.4 font width. */
int dm1_v1_cpnbc_name_field_text_pixels_pc34(const char *name);

/* Defaults / evidence. */
void dm1_v1_cpnbc_default_pc34(DM1_V1_CPNBC_ModelPc34Compat *out_model);

const char *dm1_v1_cpnbc_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHAMPION_PANEL_NAME_BOX_CLIP_PC34_COMPAT_H */
