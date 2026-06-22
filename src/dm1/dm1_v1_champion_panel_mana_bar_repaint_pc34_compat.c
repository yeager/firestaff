#include "dm1_v1_champion_panel_mana_bar_repaint_pc34_compat.h"

#include "dm1_v1_champion_panel_hud_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-locked contract gate only.
 *
 * CHAMDRAW.C F0287 lines 141-154 give the bar graph height for mana on
 * the third row of the bar graph array (HP=0, Stamina=1, Mana=2). When the
 * casters' CurrentMana exceeds MaximumMana, the height is clamped to 25.
 * When CurrentMana is zero or negative, the height is zero.
 *
 * CHAMDRAW.C F0287 lines 307-342 emit the PC34 bar split: a C12 blank band
 * above and a G0046 champion-color fill band below. The fill band is sized
 * at height*current/maximum with a one-pixel floor when current != 0.
 *
 * SYMBOL.C F0399 lines 20-39 spend the symbol mana cost before appending
 * the rune character. CHAMDRAW.C F0292:898-935 recomputes all three bar
 * graphs after STATISTICS is set.
 */

static const char s_source_evidence[] =
    "contract_only=1; no real-asset bitmap parity claim; no GRAPHICS.DAT or "
    "DUNGEON.DAT load. CHAMDRAW.C F0287_CHAMPION_DrawBarGraphs:141-154 "
    "computes the mana bar height: if CurrentMana > MaximumMana the height "
    "is clamped to 25 (overflow), otherwise ceil(CurrentMana*25/MaximumMana); "
    "if CurrentMana <= 0 the height is 0 (full blank). CHAMDRAW.C "
    "F0287:307-342 emits the PC34 bar split: zoneId = C195 + champion + "
    "stat*4, blank band height = max(0, 25 - floor(25*current/maximum)) for "
    "current > 0, fill band height = max(1, 25 - blankHeight) for current > 0; "
    "the fill band uses G0046_auc_Graphic562_ChampionColor[champion] for the "
    "fill color. "
    "CHAMDRAW.C F0292_CHAMPION_DrawState:898-935 recomputes the bar graph "
    "heights whenever MASK0x0100_STATISTICS is set. SYMBOL.C F0399:20-39 "
    "spends the symbol mana cost G0485_aauc_Graphic560_SymbolBaseManaCost["
    "step][index] (multiplied by G0486_auc_Graphic560_SymbolManaCostMultiplier"
    "[powerIdx] >> 3 when step > 0) before appending the rune character. "
    "SYMBOL.C F0400 deletes the last rune character without refunding mana. "
    "DEFS.H M008 attributes: MASK0x0100_STATISTICS marks the champion for "
    "stat-bar + value recompute; MASK0x0800_PANEL marks chest/object panel "
    "recompute. CHAMDRAW.C F0289_CHAMPION_DrawHealthOrStaminaOrManaValue:489 "
    "and F0290_CHAMPION_DrawHealthStaminaManaValues:480-494 draw the mana "
    "numeric value (CurrentMana, MaximumMana) into C551/C552 zones when the "
    "STATISTICS bit is set.";

static const dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_evidence_t
    s_evidence = {
        "CHAMDRAW.C F0287:141-154",
        "SYMBOL.C F0399:20-39",
        "CHAMDRAW.C F0287:307-342",
        "CHAMDRAW.C F0292:898-935",
        "DEFS.H M008: MASK0x0100_STATISTICS / MASK0x0800_PANEL",
        "contract-only mana-bar repaint visible-delta gate",
        "no real-asset bitmap parity; no GRAPHICS.DAT/DUNGEON.DAT load"
    };

static const dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_invariant_t
    s_invariant = {
        true,  /* contract_only */
        false, /* loads_graphics_dat */
        false, /* loads_dungeon_dat */
        true,  /* uses_synthetic_champion_state */
        true,  /* covers_mana_bar_repaint_only */
        true,  /* follows_pc34_bar_split */
        true,  /* applies_f0287_overflow_clamp */
        true,  /* applies_f0287_zero_blanks_bar */
        true,  /* dirty_attributes_are_set_when_height_changes */
        true,  /* dirty_attributes_unset_when_height_unchanged */
        true,  /* reports_zone_id_delta */
        true   /* reports_fill_band_delta */
    };

/*
 * F0287 fixed-point mana bar height, ported directly from CHAMDRAW.C:141-154.
 * Mirrors DM1_ChampionPanel_BarGraphHeight(current, maximum, 1) but is
 * reproduced here so the contract gate does not depend on the champion_panel
 * hud module being in the same translation unit.
 */
static int f0287_mana_bar_height(int16_t current, int16_t maximum)
{
    if (current <= 0 || maximum <= 0) {
        return 0;
    }
    if (current > maximum) {
        return DM1_BAR_GRAPH_MAX_HEIGHT;
    }
    long scaled = (((long)current << 10) * (long)DM1_BAR_GRAPH_MAX_HEIGHT) /
                  (long)maximum;
    if (scaled & 0x3FFL) {
        return (int)(scaled >> 10) + 1;
    }
    return (int)(scaled >> 10);
}

/*
 * F0287 PC34 bar fill model ported from CHAMDRAW.C:307-342. Mirrors
 * DM1_ChampionPanel_BuildPc34BarFillModel but uses the F0287:146 ceil
 * formula directly so the visible fill band tracks the asm loop height
 * (the asm compares height > line_index at line 226 to decide which
 * scanlines to color). The result is the same as BuildPc34BarFillModel
 * for round ratios and differs only when floor() and ceil() disagree.
 */
static void f0287_pc34_mana_bar_split(int champion_index,
                                      int stat_index,
                                      int16_t current,
                                      int16_t maximum,
                                      int *zone_id,
                                      int *bar_x,
                                      int *blank_height,
                                      int *fill_y,
                                      int *fill_height,
                                      bool *emits_blank,
                                      bool *emits_fill)
{
    int filled_height;

    if (champion_index < 0 || champion_index >= DM1_CHAMPION_COUNT ||
        stat_index != DM1_STATUS_VALUE_MANA || maximum <= 0) {
        *zone_id = 0;
        *bar_x = 0;
        *blank_height = 0;
        *fill_y = 0;
        *fill_height = 0;
        *emits_blank = false;
        *emits_fill = false;
        return;
    }

    int x;
    int y;
    DM1_ChampionPanel_BarGraphScreenXY(champion_index, stat_index, &x, &y);
    *zone_id = 195 + champion_index + (stat_index * 4);
    *bar_x = x;
    filled_height = f0287_mana_bar_height(current, maximum);
    *blank_height = DM1_BAR_GRAPH_MAX_HEIGHT - filled_height;
    *emits_blank = *blank_height > 0;
    *fill_y = y + *blank_height;
    *fill_height = filled_height;
    *emits_fill = *fill_height > 0;
}

static uint32_t hash_mix(uint32_t hash, uint32_t value)
{
    hash ^= value;
    hash *= 16777619u;
    return hash;
}

dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_result_t
dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_run(
    const dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_mutation_t *input)
{
    dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_result_t result;
    dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_mutation_t local;
    int16_t mana_before;
    int16_t mana_after;
    int16_t mana_cost_applied;
    int16_t clamp_maximum;
    int height_before;
    int height_after;

    memset(&result, 0, sizeof(result));
    result.invariant = s_invariant;
    result.evidence = s_evidence;
    result.determinism_hash = 2166136261u;

    if (!input) {
        memset(&local, 0, sizeof(local));
        local.champion_index = 0;
        local.stat_index = DM1_STATUS_VALUE_MANA;
        local.maximum_mana = 50;
        local.current_mana_before = 50;
        local.mana_delta = 1;
        local.symbol_step_after = 1;
        local.kind = DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_KIND_SPEND_RUNE_PC34;
        input = &local;
        result.null_input_defaults_used = true;
    }

    result.mutation = *input;

    if (input->champion_index < 0 ||
        input->champion_index >= DM1_CHAMPION_COUNT) {
        result.rejected_out_of_range_champion_index = true;
        return result;
    }
    if (input->maximum_mana < 0) {
        result.rejected_negative_maximum_mana = true;
        return result;
    }
    if (input->maximum_mana == 0) {
        result.rejected_zero_maximum_mana = true;
        return result;
    }
    if (input->kind ==
            DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_KIND_REFILL_PC34 &&
        input->mana_delta < 0) {
        result.rejected_negative_delta_with_refill_kind = true;
        return result;
    }
    if (input->mana_delta < 0 &&
        input->kind !=
            DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_KIND_REFILL_PC34) {
        /* A negative mana_delta on a SPEND kind is not a F0399 spend path.
         * The contract clamps it to a no-op rather than refunding. */
        mana_cost_applied = 0;
    } else {
        mana_cost_applied = input->mana_delta;
    }

    mana_before = input->current_mana_before;
    clamp_maximum = input->maximum_mana;
    if (input->kind ==
        DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_KIND_OVERFLOW_CLAMP_PC34) {
        /*
         * F0287 line 144: when CurrentMana > MaximumMana, the height is
         * clamped to 25 with no further fill-band shrink. We mirror that
         * by leaving mana_after above clamp_maximum so the F0287 height
         * helper returns 25 directly.
         */
        mana_after = (int16_t)(clamp_maximum + 1);
        mana_cost_applied = 0;
    } else if (input->kind ==
               DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_KIND_DEPLETE_TO_ZERO_PC34) {
        mana_after = 0;
        mana_cost_applied = mana_before;
    } else if (input->kind ==
               DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_KIND_REFILL_PC34) {
        /* Refill: current + delta (capped at MaximumMana). */
        int32_t refilled = (int32_t)mana_before + (int32_t)mana_cost_applied;
        if (refilled > clamp_maximum) {
            refilled = clamp_maximum;
        }
        if (refilled < 0) {
            refilled = 0;
        }
        mana_after = (int16_t)refilled;
    } else {
        /* SPEND_RUNE: F0399 spends mana_cost; reject if it exceeds current. */
        int32_t after = (int32_t)mana_before - (int32_t)mana_cost_applied;
        if (after < 0) {
            after = 0;
            mana_cost_applied = mana_before;
        }
        mana_after = (int16_t)after;
    }

    height_before = f0287_mana_bar_height(mana_before, clamp_maximum);
    height_after = f0287_mana_bar_height(mana_after, clamp_maximum);

    result.visible.height_before = height_before;
    result.visible.height_after = height_after;
    result.visible.current_mana_after = mana_after;
    result.visible.maximum_mana_after = clamp_maximum;
    result.visible.mana_cost_applied = mana_cost_applied;
    result.visible.bar_height_changed = height_before != height_after;
    result.visible.overflow_clamped =
        input->kind ==
            DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_KIND_OVERFLOW_CLAMP_PC34 ||
        (height_after == DM1_BAR_GRAPH_MAX_HEIGHT &&
         mana_after > clamp_maximum);
    result.visible.deplete_to_zero =
        input->kind ==
            DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_KIND_DEPLETE_TO_ZERO_PC34 ||
        (mana_before > 0 && mana_after == 0);

    f0287_pc34_mana_bar_split(
        input->champion_index, input->stat_index,
        mana_before, clamp_maximum,
        &result.visible.zone_id_before,
        &result.visible.bar_x_before,
        &result.visible.blank_height_before,
        &result.visible.fill_y_before,
        &result.visible.fill_height_before,
        &result.visible.emits_blank_before,
        &result.visible.emits_fill_before);
    f0287_pc34_mana_bar_split(
        input->champion_index, input->stat_index,
        mana_after, clamp_maximum,
        &result.visible.zone_id_after,
        &result.visible.bar_x_after,
        &result.visible.blank_height_after,
        &result.visible.fill_y_after,
        &result.visible.fill_height_after,
        &result.visible.emits_blank_after,
        &result.visible.emits_fill_after);

    result.visible.zone_id_changed =
        result.visible.zone_id_before != result.visible.zone_id_after;
    result.visible.fill_band_changed =
        result.visible.blank_height_before !=
            result.visible.blank_height_after ||
        result.visible.fill_height_before !=
            result.visible.fill_height_after ||
        result.visible.fill_y_before != result.visible.fill_y_after ||
        result.visible.emits_fill_before != result.visible.emits_fill_after;

    /* Dirty attribute policy: STATISTICS is set whenever the bar height
     * changes; PANEL is additionally set when the mutation is also
     * expected to redraw the chest / object panel (overflow clamp and
     * deplete-to-zero are stat-bar-only). Idempotent mutations leave
     * both dirty bits clear. */
    if (result.visible.bar_height_changed ||
        result.visible.fill_band_changed) {
        result.visible.dirty_attributes =
            DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_ATTR_STATISTICS_PC34;
        if (input->kind ==
                DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_KIND_OVERFLOW_CLAMP_PC34 ||
            input->kind ==
                DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_KIND_DEPLETE_TO_ZERO_PC34) {
            result.visible.dirty_attributes |=
                DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_ATTR_PANEL_PC34;
        }
    } else {
        result.visible.dirty_attributes = 0;
    }

    /* Determinism hash: kind, champion, stat_index, before/after mana,
     * before/after bar split dimensions. */
    result.determinism_hash = hash_mix(
        result.determinism_hash, (uint32_t)input->kind);
    result.determinism_hash = hash_mix(
        result.determinism_hash, (uint32_t)input->champion_index);
    result.determinism_hash = hash_mix(
        result.determinism_hash, (uint32_t)input->stat_index);
    result.determinism_hash = hash_mix(
        result.determinism_hash, (uint32_t)mana_before);
    result.determinism_hash = hash_mix(
        result.determinism_hash, (uint32_t)mana_after);
    result.determinism_hash = hash_mix(
        result.determinism_hash, (uint32_t)height_before);
    result.determinism_hash = hash_mix(
        result.determinism_hash, (uint32_t)height_after);
    result.determinism_hash = hash_mix(
        result.determinism_hash, (uint32_t)result.visible.blank_height_after);
    result.determinism_hash = hash_mix(
        result.determinism_hash, (uint32_t)result.visible.fill_height_after);
    result.determinism_hash = hash_mix(
        result.determinism_hash, result.visible.dirty_attributes);

    return result;
}

const dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_evidence_t *
dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_evidence(void)
{
    return &s_evidence;
}

const char *
dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_source_evidence(void)
{
    return s_source_evidence;
}
