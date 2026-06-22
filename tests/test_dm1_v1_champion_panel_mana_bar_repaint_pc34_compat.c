/*
 * CTest gate: DM1 V1 Champion Panel Mana Bar Repaint contract.
 *
 * Source-locks the deterministic mana bar repaint delta that follows a
 * SYMBOL.C F0399 mana spend or a stat mutation that changes CurrentMana.
 * Synthetic champion state only: no GRAPHICS.DAT/DUNGEON.DAT load.
 *
 * ReDMCSB source anchors:
 *   CHAMDRAW.C F0287:141-154  mana bar height (ceil + overflow clamp).
 *   CHAMDRAW.C F0287:307-342  PC34 bar split (zone, blank band, fill band).
 *   CHAMDRAW.C F0292:898-935  stat recompute path triggered by STATISTICS.
 *   SYMBOL.C     F0399:20-39  mana spend before rune append.
 *   DEFS.H                   M008_STATISTICS (0x0100) + M008_PANEL (0x0800).
 */

#include "dm1_v1_champion_panel_mana_bar_repaint_pc34_compat.h"
#include "dm1_v1_champion_panel_hud_pc34_compat.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static void check_int(const char *label, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d at %s\n", label, got, want, anchor);
    } else {
        printf("PASS %s == %d (%s)\n", label, want, anchor);
    }
}

static void check_u16(const char *label, uint16_t got, uint16_t want,
                      const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=0x%04X want=0x%04X at %s\n",
               label, (unsigned)got, (unsigned)want, anchor);
    } else {
        printf("PASS %s == 0x%04X (%s)\n", label, (unsigned)want, anchor);
    }
}

static void check_u32(const char *label, uint32_t got, uint32_t want,
                      const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=0x%08X want=0x%08X at %s\n",
               label, (unsigned)got, (unsigned)want, anchor);
    } else {
        printf("PASS %s == 0x%08X (%s)\n", label, (unsigned)want, anchor);
    }
}

static void check_bool(const char *label, bool got, bool want,
                       const char *anchor)
{
    check_int(label, got ? 1 : 0, want ? 1 : 0, anchor);
}

static void check_contains(const char *label, const char *haystack,
                           const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        ++g_failures;
        printf("FAIL %s missing \"%s\" at %s\n",
               label, needle ? needle : "(null)", anchor);
    } else {
        printf("PASS %s contains \"%s\" (%s)\n", label, needle, anchor);
    }
}

static void check_str_eq(const char *label, const char *got, const char *want,
                         const char *anchor)
{
    ++g_assertions;
    if (!got || !want || strcmp(got, want) != 0) {
        ++g_failures;
        printf("FAIL %s got=\"%s\" want=\"%s\" at %s\n",
               label, got ? got : "(null)", want ? want : "(null)", anchor);
    } else {
        printf("PASS %s == \"%s\" (%s)\n", label, want, anchor);
    }
}

static void test_invariants_and_evidence(void)
{
    const dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_evidence_t *evidence =
        dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_evidence();
    const char *source =
        dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_source_evidence();
    dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_result_t result =
        dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_run(NULL);

    check_bool("inv.contract_only", result.invariant.contract_only, true,
               "CHAMDRAW.C F0287:141-154 contract-only gate");
    check_bool("inv.no_graphics_dat",
               result.invariant.loads_graphics_dat, false,
               "CHAMDRAW.C F0287 no GRAPHICS.DAT load");
    check_bool("inv.no_dungeon_dat",
               result.invariant.loads_dungeon_dat, false,
               "CHAMDRAW.C F0292 no DUNGEON.DAT load");
    check_bool("inv.synthetic_state",
               result.invariant.uses_synthetic_champion_state, true,
               "synthetic champion mana pool");
    check_bool("inv.bar_repaint_only",
               result.invariant.covers_mana_bar_repaint_only, true,
               "mana bar row only");
    check_bool("inv.pc34_split",
               result.invariant.follows_pc34_bar_split, true,
               "CHAMDRAW.C F0287:307-342");
    check_bool("inv.overflow_clamp",
               result.invariant.applies_f0287_overflow_clamp, true,
               "CHAMDRAW.C F0287:144");
    check_bool("inv.zero_blank",
               result.invariant.applies_f0287_zero_blanks_bar, true,
               "CHAMDRAW.C F0287:154");
    check_bool("inv.dirty_set_on_change",
               result.invariant.dirty_attributes_are_set_when_height_changes,
               true, "DEFS.H M008_STATISTICS when bar height changes");
    check_bool("inv.dirty_clear_no_change",
               result.invariant.dirty_attributes_unset_when_height_unchanged,
               true, "DEFS.H M008_STATISTICS skipped for idempotent");
    check_bool("inv.zone_delta",
               result.invariant.reports_zone_id_delta, true,
               "F0287:307 zone anchor");
    check_bool("inv.fill_band_delta",
               result.invariant.reports_fill_band_delta, true,
               "F0287:307-342 fill band");

    check_str_eq("evidence.bar_anchor", evidence->champion_bar_anchor,
                 "CHAMDRAW.C F0287:141-154",
                 "CHAMDRAW.C F0287 mana bar anchor");
    check_str_eq("evidence.spend_anchor", evidence->mana_spend_anchor,
                 "SYMBOL.C F0399:20-39",
                 "SYMBOL.C F0399 mana spend anchor");
    check_str_eq("evidence.pc34_split_anchor",
                 evidence->pc34_bar_split_anchor,
                 "CHAMDRAW.C F0287:307-342",
                 "CHAMDRAW.C F0287 PC34 split anchor");
    check_str_eq("evidence.draw_state_anchor",
                 evidence->draw_state_anchor,
                 "CHAMDRAW.C F0292:898-935",
                 "CHAMDRAW.C F0292 draw state anchor");
    check_str_eq("evidence.dirty_anchor",
                 evidence->dirty_attributes_anchor,
                 "DEFS.H M008: MASK0x0100_STATISTICS / MASK0x0800_PANEL",
                 "DEFS.H M008 dirty attribute anchor");

    check_contains("src.F0287_height", source,
                   "CHAMDRAW.C F0287_CHAMPION_DrawBarGraphs:141-154",
                   "CHAMDRAW.C F0287:141-154 source evidence");
    check_contains("src.F0287_split", source,
                   "CHAMDRAW.C F0287:307-342",
                   "CHAMDRAW.C F0287:307-342 source evidence");
    check_contains("src.F0292", source, "CHAMDRAW.C F0292_CHAMPION_DrawState:898-935",
                   "CHAMDRAW.C F0292 source evidence");
    check_contains("src.F0399", source, "SYMBOL.C F0399:20-39",
                   "SYMBOL.C F0399 source evidence");
    check_contains("src.MASK0x0100", source, "MASK0x0100_STATISTICS",
                   "DEFS.H M008_STATISTICS");
    check_contains("src.MASK0x0800", source, "MASK0x0800_PANEL",
                   "DEFS.H M008_PANEL");
    check_contains("src.G0046", source, "G0046",
                   "CHAMDRAW.C F0287 champion-color G0046");
    check_contains("src.G0485", source, "G0485",
                   "SYMBOL.C F0399 base mana cost G0485");
    check_contains("src.G0486", source, "G0486",
                   "SYMBOL.C F0399 multiplier G0486");
    check_contains("src.C195", source, "C195",
                   "CHAMDRAW.C F0287 bar zone anchor C195");
    check_contains("src.clamp_25", source, "clamped to 25",
                   "CHAMDRAW.C F0287:144 overflow clamp");

    check_bool("null_input.defaults", result.null_input_defaults_used, true,
               "synthetic default state accepted");
}

static void test_full_mana_to_partial_mana_spend(void)
{
    dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_mutation_t input;
    dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_result_t result;

    /*
     * Champion 0 starts at 25/50 mana. Spend 1 mana (typical power-step 0
     * Lo rune per G0485[0][0]=1). The bar must drop from 13 to 12 fill
     * pixels with a 1-pixel blank band appearing at the top.
     */
    memset(&input, 0, sizeof(input));
    input.champion_index = 0;
    input.stat_index = DM1_STATUS_VALUE_MANA;
    input.maximum_mana = 50;
    input.current_mana_before = 25;
    input.mana_delta = 1;
    input.symbol_step_after = 1;
    input.kind =
        DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_KIND_SPEND_RUNE_PC34;

    result = dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_run(&input);

    check_int("spend.height_before", result.visible.height_before, 13,
              "CHAMDRAW.C F0287:146 25/50 ceil((25<<10)*25/50) = 13");
    check_int("spend.height_after", result.visible.height_after, 12,
              "CHAMDRAW.C F0287:146 24/50 ceil((24<<10)*25/50) = 12");
    check_int("spend.mana_after", result.visible.current_mana_after, 24,
              "F0399 spend 25 -> 24");
    check_int("spend.mana_cost_applied",
              result.visible.mana_cost_applied, 1,
              "F0399 cost 1 applied");
    check_bool("spend.height_changed",
               result.visible.bar_height_changed, true,
               "DEFS.H M008_STATISTICS must be set");
    check_bool("spend.fill_band_changed",
               result.visible.fill_band_changed, true,
               "PC34 fill band shrinks by 1px");
    check_bool("spend.zone_id_changed",
               result.visible.zone_id_changed, false,
              "champion index and stat_index unchanged");
    check_int("spend.zone_id_after", result.visible.zone_id_after, 203,
              "C195 + 0 + 2*4 = 203 (champion 0, mana row)");
    check_int("spend.bar_x_after", result.visible.bar_x_after, 60,
              "champion 0 mana x = 0*69+46+14 = 60");
    check_int("spend.blank_height_after",
              result.visible.blank_height_after, 13,
              "F0287:307 blankHeight = 25 - 12 = 13");
    check_int("spend.fill_y_after", result.visible.fill_y_after, 15,
              "F0287:307 fillY = 2 + blankHeight = 15");
    check_int("spend.fill_height_after",
              result.visible.fill_height_after, 12,
              "F0287:307 fillHeight = 12 (ceil form)");
    check_u16("spend.dirty_attributes",
              result.visible.dirty_attributes,
              DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_ATTR_STATISTICS_PC34,
              "DEFS.H M008_STATISTICS only");
    check_bool("spend.not_clamped", result.visible.overflow_clamped, false,
               "F0287:144 clamp not active for 24 < 50");
    check_bool("spend.not_zero", result.visible.deplete_to_zero, false,
               "F0287:154 zero-blank not active for 24 > 0");
}

static void test_three_quarter_to_half_spend(void)
{
    dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_mutation_t input;
    dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_result_t result;

    /* 75/100 -> 50/100: ceil = 19 -> 13; PC34 fill split = 12 + 13 blank. */
    memset(&input, 0, sizeof(input));
    input.champion_index = 1;
    input.stat_index = DM1_STATUS_VALUE_MANA;
    input.maximum_mana = 100;
    input.current_mana_before = 75;
    input.mana_delta = 25;
    input.symbol_step_after = 1;
    input.kind =
        DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_KIND_SPEND_RUNE_PC34;

    result = dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_run(&input);

    check_int("q.height_before", result.visible.height_before, 19,
              "F0287:146 75/100 ceil((75<<10)*25/100) = 19");
    check_int("q.height_after", result.visible.height_after, 13,
              "F0287:146 50/100 ceil((50<<10)*25/100) = 13");
    check_int("q.zone_id_after", result.visible.zone_id_after, 204,
              "C195 + 1 + 2*4 = 204 (champion 1, mana row)");
    check_int("q.bar_x_after", result.visible.bar_x_after, 129,
              "champion 1 mana x = 1*69+46+14 = 129");
    check_int("q.blank_height_after", result.visible.blank_height_after, 12,
              "F0287:307 blankHeight = 25 - 13 = 12");
    check_int("q.fill_y_after", result.visible.fill_y_after, 14,
              "F0287:307 fillY = 2 + blankHeight = 14");
    check_int("q.fill_height_after", result.visible.fill_height_after, 13,
              "F0287:307 fillHeight = 13 (ceil form)");
    check_bool("q.emits_blank_after", result.visible.emits_blank_after, true,
               "F0287:319 blankHeight > 0 emits C12 band");
    check_bool("q.emits_fill_after", result.visible.emits_fill_after, true,
               "F0287:335 fillHeight > 0 emits G0046 band");
    check_u16("q.dirty_attributes", result.visible.dirty_attributes,
              DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_ATTR_STATISTICS_PC34,
              "DEFS.H M008_STATISTICS only");
}

static void test_deplete_to_zero_mana_bar(void)
{
    dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_mutation_t input;
    dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_result_t result;

    /* Last mana point spent -> F0287:154 height = 0 (full blank). */
    memset(&input, 0, sizeof(input));
    input.champion_index = 2;
    input.stat_index = DM1_STATUS_VALUE_MANA;
    input.maximum_mana = 50;
    input.current_mana_before = 1;
    input.mana_delta = 1;
    input.symbol_step_after = 1;
    input.kind =
        DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_KIND_DEPLETE_TO_ZERO_PC34;

    result = dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_run(&input);

    check_int("zero.height_before", result.visible.height_before, 1,
              "F0287:146 1/50 ceil = 1");
    check_int("zero.height_after", result.visible.height_after, 0,
              "F0287:154 0/50 -> blank bar");
    check_bool("zero.bar_height_changed",
               result.visible.bar_height_changed, true,
               "M008_STATISTICS set");
    check_bool("zero.deplete_to_zero",
               result.visible.deplete_to_zero, true,
               "F0287:154 zero-blank path");
    check_bool("zero.not_clamped",
               result.visible.overflow_clamped, false,
               "F0287:154 clamp not active");
    check_int("zero.blank_height_after",
              result.visible.blank_height_after, 25,
              "F0287:154 blankHeight = 25 (no fill band)");
    check_int("zero.fill_height_after",
              result.visible.fill_height_after, 0,
              "F0287:154 fillHeight = 0");
    check_bool("zero.emits_fill_after",
               result.visible.emits_fill_after, false,
               "F0287:335 fillHeight == 0 -> no fill band");
    check_bool("zero.emits_blank_after",
               result.visible.emits_blank_after, true,
               "F0287:319 blankHeight == 25 -> full C12 band");
    check_u16("zero.dirty_attributes", result.visible.dirty_attributes,
              DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_ATTR_STATISTICS_PC34 |
                  DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_ATTR_PANEL_PC34,
              "DEFS.H M008_STATISTICS | M008_PANEL for deplete-to-zero");
}

static void test_overflow_clamp_mana_bar(void)
{
    dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_mutation_t input;
    dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_result_t result;

    /* Transition 24/50 -> 51/50: F0287:144 clamps to 25. */
    memset(&input, 0, sizeof(input));
    input.champion_index = 3;
    input.stat_index = DM1_STATUS_VALUE_MANA;
    input.maximum_mana = 50;
    input.current_mana_before = 24;
    input.mana_delta = 0;
    input.symbol_step_after = 0;
    input.kind =
        DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_KIND_OVERFLOW_CLAMP_PC34;

    result = dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_run(&input);

    check_int("over.height_before", result.visible.height_before, 12,
              "F0287:146 24/50 ceil = 12");
    check_int("over.height_after", result.visible.height_after, 25,
              "F0287:144 51 > 50 -> clamp to 25");
    check_bool("over.overflow_clamped",
               result.visible.overflow_clamped, true,
               "F0287:144 overflow clamp active");
    check_bool("over.not_deplete",
               result.visible.deplete_to_zero, false,
               "F0287:154 zero-blank not active");
    check_bool("over.bar_height_changed",
               result.visible.bar_height_changed, true,
               "M008_STATISTICS set on 12 -> 25 transition");
    check_int("over.blank_height_after",
              result.visible.blank_height_after, 0,
              "F0287:307 full fill, no blank");
    check_int("over.fill_height_after",
              result.visible.fill_height_after, 25,
              "F0287:335 fillHeight = 25 (full bar)");
    check_bool("over.emits_blank_after",
               result.visible.emits_blank_after, false,
               "F0287:319 no blank band");
    check_bool("over.emits_fill_after",
               result.visible.emits_fill_after, true,
               "F0287:335 full G0046 fill");
    check_u16("over.dirty_attributes", result.visible.dirty_attributes,
              DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_ATTR_STATISTICS_PC34 |
                  DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_ATTR_PANEL_PC34,
              "DEFS.H M008_STATISTICS | M008_PANEL for overflow clamp");
}

static void test_refill_idempotent_no_redraw(void)
{
    dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_mutation_t input;
    dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_result_t result;

    /* Refill to full -> height stays at 25 -> no dirty bits set. */
    memset(&input, 0, sizeof(input));
    input.champion_index = 0;
    input.stat_index = DM1_STATUS_VALUE_MANA;
    input.maximum_mana = 50;
    input.current_mana_before = 50;
    input.mana_delta = 0;
    input.symbol_step_after = 0;
    input.kind = DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_KIND_REFILL_PC34;

    result = dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_run(&input);

    check_int("idem.height_before", result.visible.height_before, 25,
              "F0287:146 50/50 full bar");
    check_int("idem.height_after", result.visible.height_after, 25,
              "F0287:146 50/50 still full bar");
    check_bool("idem.height_changed",
               result.visible.bar_height_changed, false,
               "M008_STATISTICS must NOT be set for idempotent");
    check_u16("idem.dirty_attributes", result.visible.dirty_attributes, 0,
              "DEFS.H M008 zero dirty bits");
}

static void test_refill_to_half(void)
{
    dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_mutation_t input;
    dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_result_t result;

    /* Refill from 12 -> 25 (cap at 25 == 50/2). */
    memset(&input, 0, sizeof(input));
    input.champion_index = 1;
    input.stat_index = DM1_STATUS_VALUE_MANA;
    input.maximum_mana = 50;
    input.current_mana_before = 12;
    input.mana_delta = 13;
    input.symbol_step_after = 0;
    input.kind = DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_KIND_REFILL_PC34;

    result = dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_run(&input);

    check_int("rf.height_before", result.visible.height_before, 6,
              "F0287:146 12/50 ceil((12<<10)*25/50) = 6");
    check_int("rf.height_after", result.visible.height_after, 13,
              "F0287:146 25/50 ceil((25<<10)*25/50) = 13");
    check_bool("rf.height_changed",
               result.visible.bar_height_changed, true,
               "M008_STATISTICS set on visible change");
    check_u16("rf.dirty_attributes", result.visible.dirty_attributes,
              DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_ATTR_STATISTICS_PC34,
              "DEFS.H M008_STATISTICS only for refill");
}

static void test_zone_x_layouts_per_champion(void)
{
    dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_mutation_t input;
    dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_result_t result;
    int champion_index;

    /* Same 100/100 fixture for each of the 4 champions; only x + zoneId
     * change. ReDMCSB CHAMDRAW.C:307-308: zoneId = 195 + champ + stat*4. */
    for (champion_index = 0; champion_index < 4; ++champion_index) {
        memset(&input, 0, sizeof(input));
        input.champion_index = champion_index;
        input.stat_index = DM1_STATUS_VALUE_MANA;
        input.maximum_mana = 50;
        input.current_mana_before = 50;
        input.mana_delta = 0;
        input.symbol_step_after = 0;
        input.kind = DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_KIND_REFILL_PC34;

        result =
            dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_run(&input);
        check_int("zone.before", result.visible.zone_id_before,
                  195 + champion_index + 8,
                  "F0287:308 zoneId before mutation");
        check_int("zone.after", result.visible.zone_id_after,
                  195 + champion_index + 8,
                  "F0287:308 zoneId unchanged for refill");
        check_int("zone.full_fill_height",
                  result.visible.fill_height_after, 25,
                  "F0287:307-342 50/50 fill band 25");
    }
}

static void test_validation_rejects(void)
{
    dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_mutation_t input;
    dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_result_t result;

    /* Out-of-range champion index. */
    memset(&input, 0, sizeof(input));
    input.champion_index = 4;
    input.stat_index = DM1_STATUS_VALUE_MANA;
    input.maximum_mana = 50;
    input.current_mana_before = 50;
    input.mana_delta = 1;
    input.kind =
        DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_KIND_SPEND_RUNE_PC34;
    result = dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_run(&input);
    check_bool("reject.champ_oob",
               result.rejected_out_of_range_champion_index, true,
               "DM1_CHAMPION_COUNT clamp");

    /* Negative maximum mana. */
    memset(&input, 0, sizeof(input));
    input.champion_index = 0;
    input.stat_index = DM1_STATUS_VALUE_MANA;
    input.maximum_mana = -1;
    input.current_mana_before = 0;
    input.mana_delta = 0;
    input.kind =
        DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_KIND_SPEND_RUNE_PC34;
    result = dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_run(&input);
    check_bool("reject.negative_max",
               result.rejected_negative_maximum_mana, true,
               "F0287 max must be >= 0");

    /* Zero maximum mana. */
    memset(&input, 0, sizeof(input));
    input.champion_index = 0;
    input.stat_index = DM1_STATUS_VALUE_MANA;
    input.maximum_mana = 0;
    input.current_mana_before = 0;
    input.mana_delta = 0;
    input.kind =
        DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_KIND_SPEND_RUNE_PC34;
    result = dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_run(&input);
    check_bool("reject.zero_max",
               result.rejected_zero_maximum_mana, true,
               "F0287:142 max <= 0 -> blank bar");

    /* Negative delta on REFILL kind. */
    memset(&input, 0, sizeof(input));
    input.champion_index = 0;
    input.stat_index = DM1_STATUS_VALUE_MANA;
    input.maximum_mana = 50;
    input.current_mana_before = 30;
    input.mana_delta = -5;
    input.kind = DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_KIND_REFILL_PC34;
    result = dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_run(&input);
    check_bool("reject.negative_refill",
               result.rejected_negative_delta_with_refill_kind, true,
               "REFILL must use non-negative delta");
}

static void test_determinism(void)
{
    dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_mutation_t input;
    dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_result_t first;
    dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_result_t second;

    memset(&input, 0, sizeof(input));
    input.champion_index = 0;
    input.stat_index = DM1_STATUS_VALUE_MANA;
    input.maximum_mana = 50;
    input.current_mana_before = 25;
    input.mana_delta = 1;
    input.symbol_step_after = 1;
    input.kind =
        DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_KIND_SPEND_RUNE_PC34;

    first = dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_run(&input);
    second = dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_run(&input);

    check_u32("deterministic.hash", first.determinism_hash,
              second.determinism_hash,
              "synthetic mana mutation must be deterministic");
    check_int("deterministic.height_after", first.visible.height_after,
              second.visible.height_after,
              "F0287:146 deterministic bar height");
    check_int("deterministic.fill_height_after",
              first.visible.fill_height_after,
              second.visible.fill_height_after,
              "F0287:307 deterministic fill band");
    check_u16("deterministic.dirty_attributes",
              first.visible.dirty_attributes,
              second.visible.dirty_attributes,
              "DEFS.H M008 deterministic dirty bits");
    check_int("deterministic.mana_after",
              first.visible.current_mana_after,
              second.visible.current_mana_after,
              "F0399 deterministic mana after spend");
}

int main(void)
{
    uint32_t probe_hash;
    dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_mutation_t probe;
    dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_result_t probe_result;

    printf("probe=dm1_v1_champion_panel_mana_bar_repaint_pc34_compat\n");
    printf("%s\n",
           dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_source_evidence());
    test_invariants_and_evidence();
    test_full_mana_to_partial_mana_spend();
    test_three_quarter_to_half_spend();
    test_deplete_to_zero_mana_bar();
    test_overflow_clamp_mana_bar();
    test_refill_idempotent_no_redraw();
    test_refill_to_half();
    test_zone_x_layouts_per_champion();
    test_validation_rejects();
    test_determinism();

    /* Probe hash for the canonical 25/50 -> 24/50 spend (the same shape
     * that the related gates use). */
    memset(&probe, 0, sizeof(probe));
    probe.champion_index = 0;
    probe.stat_index = DM1_STATUS_VALUE_MANA;
    probe.maximum_mana = 50;
    probe.current_mana_before = 25;
    probe.mana_delta = 1;
    probe.symbol_step_after = 1;
    probe.kind =
        DM1_V1_CHAMPION_PANEL_MANA_BAR_REPAINT_KIND_SPEND_RUNE_PC34;
    probe_result = dm1_v1_champion_panel_mana_bar_repaint_pc34_compat_run(&probe);
    probe_hash = probe_result.determinism_hash;

    if (g_failures || g_assertions < 80) {
        printf("FAIL dm1_v1_champion_panel_mana_bar_repaint_pc34_compat "
               "failures=%d assertions=%d hash=0x%08X\n",
               g_failures, g_assertions, probe_hash);
        return 1;
    }
    printf("PASS dm1_v1_champion_panel_mana_bar_repaint_pc34_compat "
           "assertions=%d failures=0 hash=0x%08X\n",
           g_assertions, probe_hash);
    return 0;
}
