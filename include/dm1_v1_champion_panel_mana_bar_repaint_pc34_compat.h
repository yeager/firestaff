#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_PC34_COMPAT_H

/*
 * DM1 V1 champion-panel mana bar repaint contract.
 *
 * Source reference: ReDMCSB CHAMDRAW.C
 *   F0287_CHAMPION_DrawBarGraphs lines 141-154:
 *     if (champion->CurrentMana > 0) {
 *         if (champion->CurrentMana > champion->MaximumMana)
 *             L0847_ai_BarGraphHeights[idx] = 25;          (overflow clamp)
 *         else
 *             height = ceil(currentMana * 25 / maximumMana);
 *     } else {
 *         L0847_ai_BarGraphHeights[idx] = 0;              (no mana -> blank)
 *     }
 *
 *   F0287 lines 307-342: PC34 bar split emits a C12 blank band on top and
 *   a G0046 champion-color fill band on the bottom, advancing +4 zones per
 *   stat so HP=C195, Stamina=C199, Mana=C203 plus champion ordinal.
 *
 *   F0292_CHAMPION_DrawState line 898-935: after stat mutation, the per-stat
 *   bar graphs are recomputed from the current champion state.
 *
 * Source reference: ReDMCSB SYMBOL.C F0399 lines 20-39:
 *   manaCost = G0485[symbolStep][symbolIndex];
 *   if (symbolStep) manaCost = (manaCost * G0486[Symbols[0] - 96]) >> 3;
 *   if (manaCost <= champion->CurrentMana) {
 *       champion->CurrentMana -= manaCost;
 *       champion->Symbols[symbolStep] = encode(symbolStep, symbolIndex);
 *       champion->SymbolStep = (symbolStep + 1) & 3;
 *   }
 *
 * Source reference: ReDMCSB DEFS.H
 *   M008_STATISTICS bit 0x0100 marks the champion attributes byte so the
 *   next F0292_DrawState pass redraws the bar graphs.
 *
 * Contract only: this slice uses synthetic champion state and reports which
 * champion-panel mana bar outputs change after a deterministic mana mutation.
 * It does not load game data, render bitmaps, or claim pixel parity.
 */

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_ATTR_STATISTICS_PC34 0x0100u
#define DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_ATTR_PANEL_PC34       0x0800u

#define DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_NONE_PC34_COMPAT (-1)

typedef enum {
    DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_KIND_SPEND_RUNE_PC34 = 0,
    DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_KIND_REFILL_PC34 = 1,
    DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_KIND_OVERFLOW_CLAMP_PC34 = 2,
    DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_KIND_DEPLETE_TO_ZERO_PC34 = 3
} dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_kind_t;

typedef struct {
    /* Champion whose mana bar repaints. */
    int champion_index;
    /* Bar graph row: statIndex == DM1_STATUS_VALUE_MANA == 2. */
    int stat_index;
    /* Initial mana pool (matches ReDMCSB M516_CHAMPIONS[i].CurrentMana). */
    int16_t maximum_mana;
    int16_t current_mana_before;
    /* Spell mutation: mana cost to deduct (positive number), or refill to apply.
     * F0399 spend = positive number. A refill uses the same delta field. */
    int16_t mana_delta;
    /* Symbol step after the spend: SYMBOL.C F0399 sets SymbolStep to
     * (previous + 1) & 3; the panel repaint is driven by STATISTICS, not by
     * SymbolStep, so this is recorded for source-lock evidence only. */
    int symbol_step_after;
    /* PANEL.C F0292:893-907 recompute source kind. */
    dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_kind_t kind;
} dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_mutation_t;

typedef struct {
    /* F0287 fixed-point bar graph height (0..25). */
    int height_before;
    int height_after;
    /* PC34 bar fill model before/after, derived from CHAMDRAW.C F0287:307-342. */
    int zone_id_before;
    int zone_id_after;
    int bar_x_before;
    int bar_x_after;
    int blank_height_before;
    int blank_height_after;
    int fill_y_before;
    int fill_y_after;
    int fill_height_before;
    int fill_height_after;
    bool emits_blank_before;
    bool emits_blank_after;
    bool emits_fill_before;
    bool emits_fill_after;
    /* Actual mana state observed after the mutation. */
    int16_t current_mana_after;
    int16_t maximum_mana_after;
    /* Mana cost actually applied to the mana pool, after F0399 checks. */
    int16_t mana_cost_applied;
    /* CHAMDRAW.C F0292:898-935 path that the panel must take after this mutation:
     *   bar_only        -> STATISTICS dirty only (bar graphs + values);
     *   bar_and_panel   -> STATISTICS | PANEL dirty (chest / object panel too);
     *   none            -> no draw needed (idempotent mutation).
     * The dirty bitmask matches the relevant DEFS.H M008 attributes. */
    uint16_t dirty_attributes;
    /* Deterministic bar-only visual deltas. */
    bool bar_height_changed;
    bool fill_band_changed;
    bool zone_id_changed;
    bool overflow_clamped;
    bool deplete_to_zero;
} dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_visible_t;

typedef struct {
    const char *champion_bar_anchor;
    const char *mana_spend_anchor;
    const char *pc34_bar_split_anchor;
    const char *draw_state_anchor;
    const char *dirty_attributes_anchor;
    const char *contract_scope;
    const char *no_real_asset_claim;
} dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_evidence_t;

typedef struct {
    bool contract_only;
    bool loads_graphics_dat;
    bool loads_dungeon_dat;
    bool uses_synthetic_champion_state;
    bool covers_mana_bar_repaint_only;
    bool follows_pc34_bar_split;
    bool applies_f0287_overflow_clamp;
    bool applies_f0287_zero_blanks_bar;
    bool dirty_attributes_are_set_when_height_changes;
    bool dirty_attributes_unset_when_height_unchanged;
    bool reports_zone_id_delta;
    bool reports_fill_band_delta;
} dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_invariant_t;

typedef struct {
    dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_invariant_t invariant;
    dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_evidence_t evidence;
    dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_mutation_t mutation;
    dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_visible_t visible;
    uint32_t determinism_hash;
    bool null_input_defaults_used;
    bool rejected_out_of_range_champion_index;
    bool rejected_negative_maximum_mana;
    bool rejected_zero_maximum_mana;
    bool rejected_negative_delta_with_refill_kind;
} dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_result_t;

dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_result_t
dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_run(
    const dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_mutation_t *input);

const dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_evidence_t *
dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_evidence(void);

const char *
dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_PC34_COMPAT_H */
