#include "dm1/dm1_v1_champion_panel_hud_damage_attribute_cascade_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

static void check_int(const char *label, int actual, int expected,
                      const char *anchor)
{
    ++g_assertions;
    if (actual != expected) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d at %s\n",
               label, actual, expected, anchor);
    } else {
        printf("PASS %s == %d (%s)\n", label, expected, anchor);
    }
}

static void check_true(const char *label, int value, const char *anchor)
{
    check_int(label, value ? 1 : 0, 1, anchor);
}

static void check_false(const char *label, int value, const char *anchor)
{
    check_int(label, value ? 1 : 0, 0, anchor);
}

static void check_contains(const char *label, const char *haystack,
                           const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || !strstr(haystack, needle)) {
        ++g_failures;
        printf("FAIL %s missing='%s' at %s\n",
               label, needle ? needle : "(null)", anchor);
    } else {
        printf("PASS %s contains '%s' (%s)\n", label, needle, anchor);
    }
}

/*
 * Deterministic 32-bit FNV-1a hash, identical to the convention used by
 * the sibling DM1 V1 champion-panel pc34_compat tests. Used to assert
 * cross-run snapshot stability of repeated Apply calls.
 */
static unsigned int fnv1a_32(const unsigned char *data, size_t len)
{
    const unsigned int FNV_OFFSET = 2166136261u;
    const unsigned int FNV_PRIME = 16777619u;
    unsigned int hash = FNV_OFFSET;
    size_t i;

    for (i = 0; i < len; ++i) {
        hash ^= (unsigned int)data[i];
        hash *= FNV_PRIME;
    }
    return hash;
}

static unsigned int fnv1a_result_hash(
    const DM1_V1_ChampionPanelHudDamageAttributeCascadeResultPc34Compat *r)
{
    unsigned char buf[256];
    size_t off = 0;

    /*
     * Snapshot the structural fields of the result. Padding bytes are
     * intentionally excluded - only the data the contract reads is hashed.
     */
    #define CPHUDAC_PUT_I(field) \
        do { \
            int _v = (int)(field); \
            buf[off++] = (unsigned char)(_v & 0xFF); \
            buf[off++] = (unsigned char)((_v >> 8) & 0xFF); \
            buf[off++] = (unsigned char)((_v >> 16) & 0xFF); \
            buf[off++] = (unsigned char)((_v >> 24) & 0xFF); \
        } while (0)

    #define CPHUDAC_PUT_B(field) \
        do { \
            buf[off++] = (unsigned char)((field) ? 1 : 0); \
        } while (0)

    if (off < sizeof(buf)) CPHUDAC_PUT_I(r->valid);
    if (off < sizeof(buf)) CPHUDAC_PUT_B(r->contract_only);
    if (off < sizeof(buf)) CPHUDAC_PUT_B(r->rejected_null_output);
    if (off < sizeof(buf)) CPHUDAC_PUT_B(r->rejected_champion_index);
    if (off < sizeof(buf)) CPHUDAC_PUT_B(r->rejected_negative_health);
    if (off < sizeof(buf)) CPHUDAC_PUT_B(r->skipped_pending_damage_zero);
    if (off < sizeof(buf)) CPHUDAC_PUT_B(r->skipped_dead_champion);
    if (off < sizeof(buf)) CPHUDAC_PUT_B(r->applied_damage);
    if (off < sizeof(buf)) CPHUDAC_PUT_B(r->set_statistics_bit);
    if (off < sizeof(buf)) CPHUDAC_PUT_B(r->set_wounds_bit);
    if (off < sizeof(buf)) CPHUDAC_PUT_B(r->f0292_will_short_circuit);
    if (off < sizeof(buf)) CPHUDAC_PUT_B(r->f0292_will_redraw_statistics);
    if (off < sizeof(buf)) CPHUDAC_PUT_B(r->f0292_will_redraw_wounds);
    if (off < sizeof(buf)) CPHUDAC_PUT_B(r->f0292_will_clear_after_redraw);
    if (off < sizeof(buf)) CPHUDAC_PUT_I(r->champion_index);
    if (off < sizeof(buf)) CPHUDAC_PUT_I(r->champion_ordinal);
    if (off < sizeof(buf)) CPHUDAC_PUT_I(r->attributes_after_apply);
    if (off < sizeof(buf)) CPHUDAC_PUT_I(r->wound_bits_after_apply);
    if (off < sizeof(buf)) CPHUDAC_PUT_I(r->wound_count_after_apply);
    if (off < sizeof(buf)) CPHUDAC_PUT_I(r->health_after_apply);
    if (off < sizeof(buf)) CPHUDAC_PUT_I(r->wound_slot_redraw_first);
    if (off < sizeof(buf)) CPHUDAC_PUT_I(r->wound_slot_redraw_last);
    if (off < sizeof(buf)) CPHUDAC_PUT_I((int)r->outcome);
    if (off < sizeof(buf)) CPHUDAC_PUT_I((int)r->redraw);

    #undef CPHUDAC_PUT_I
    #undef CPHUDAC_PUT_B

    return fnv1a_32(buf, off);
}

static void test_evidence_anchors(void)
{
    const DM1_V1_ChampionPanelHudDamageAttributeCascadeEvidencePc34Compat
        *evidence =
        DM1_V1_ChampionPanelHudDamageAttributeCascade_EvidencePc34Compat();
    const char *src =
        DM1_V1_ChampionPanelHudDamageAttributeCascade_SourceEvidencePc34Compat();

    check_true("evidence.contract_only", evidence->contract_only,
               "CHAMPION.C F0320:1720-1792 + CHAMDRAW.C F0292 contract-only");
    check_contains("evidence.producer_function",
                   evidence->producer_function_anchor,
                   "CHAMPION.C F0320",
                   "CHAMPION.C F0320:1720-1792 producer function");
    check_contains("evidence.producer_set_statistics",
                   evidence->producer_set_statistics_anchor,
                   "MASK0x0100_STATISTICS",
                   "CHAMPION.C F0320:1738 M008_SET MASK0x0100_STATISTICS");
    check_contains("evidence.producer_set_wounds",
                   evidence->producer_set_wounds_anchor,
                   "MASK0x2000_WOUNDS",
                   "CHAMPION.C F0320:1740 M008_SET MASK0x2000_WOUNDS");
    check_contains("evidence.producer_skip_zero",
                   evidence->producer_skip_zero_anchor,
                   "G0409",
                   "CHAMPION.C F0320:1724-1725 G0409 == 0 continue");
    check_contains("evidence.producer_skip_dead",
                   evidence->producer_skip_dead_anchor,
                   "CurrentHealth",
                   "CHAMPION.C F0320:1726-1727 CurrentHealth == 0 continue");
    check_contains("evidence.consumer_function",
                   evidence->consumer_function_anchor,
                   "CHAMDRAW.C F0292",
                   "CHAMDRAW.C F0292_CHAMPION_DrawState:755-1115");
    check_contains("evidence.consumer_short_circuit",
                   evidence->consumer_short_circuit_anchor,
                   "F0292:757",
                   "CHAMDRAW.C F0292:757 nine-bit short-circuit");
    check_contains("evidence.consumer_statistics_redraw",
                   evidence->consumer_statistics_redraw_anchor,
                   "F0292:898",
                   "CHAMDRAW.C F0292:898 MASK0x0100_STATISTICS redraw");
    check_contains("evidence.consumer_wounds_redraw",
                   evidence->consumer_wounds_redraw_anchor,
                   "F0292:937",
                   "CHAMDRAW.C F0292:937 MASK0x2000_WOUNDS redraw");
    check_contains("evidence.consumer_clear",
                   evidence->consumer_clear_anchor,
                   "F0292:1110",
                   "CHAMDRAW.C F0292:1110 M009_CLEAR nine-bit Attributes");
    check_contains("evidence.wound_slot_anchor",
                   evidence->wound_slot_anchor_anchor,
                   "DEFS.H:724-732",
                   "DEFS.H:724-732 nine MASK0x0080..MASK0x8000 bits");
    check_contains("evidence.no_real_asset_claim",
                   evidence->no_real_asset_claim,
                   "without claiming real-asset parity",
                   "contract-only no original-DOS parity claim");

    /* Source-evidence helper must mention every anchor we rely on. */
    check_contains("source.f0320:1738", src, "F0320:1738",
                   "CHAMPION.C F0320:1738 MASK0x0100_STATISTICS");
    check_contains("source.f0320:1740", src, "F0320:1740",
                   "CHAMPION.C F0320:1740 MASK0x2000_WOUNDS");
    check_contains("source.f0292:757", src, "F0292:757",
                   "CHAMDRAW.C F0292:757 nine-bit short-circuit");
    check_contains("source.f0292:1110", src, "F0292:1110",
                   "CHAMDRAW.C F0292:1110 M009_CLEAR nine-bit Attributes");
    check_contains("source.f0292:898", src, "F0292:898",
                   "CHAMDRAW.C F0292:898 statistics redraw anchor");
    check_contains("source.f0292:937", src, "F0292:937",
                   "CHAMDRAW.C F0292:937 wounds redraw anchor");
    check_contains("source.defs_anchor",
                   src,
                   "MASK0x0080_NAME_TITLE",
                   "Source-evidence string carries MASK0x0080_NAME_TITLE");
}

static void test_constants(void)
{
    check_int("const.champion_count",
              DM1_V1_CPHUDAC_CHAMPION_COUNT_PC34, 4,
              "CHAMPION.C F0320:1720-1721 four champion panel cells");
    check_int("const.attributes_none",
              DM1_V1_CPHUDAC_ATTRIBUTES_NONE_PC34, 0x0000,
              "DEFS.H:724-732 cleared Attributes baseline");
    check_int("const.mask0x0080_name_title",
              DM1_V1_CPHUDAC_MASK0x0080_NAME_TITLE_PC34, 0x0080,
              "DEFS.H:724 MASK0x0080_NAME_TITLE");
    check_int("const.mask0x0100_statistics",
              DM1_V1_CPHUDAC_MASK0x0100_STATISTICS_PC34, 0x0100,
              "DEFS.H:725 MASK0x0100_STATISTICS");
    check_int("const.mask0x0200_load",
              DM1_V1_CPHUDAC_MASK0x0200_LOAD_PC34, 0x0200,
              "DEFS.H:726 MASK0x0200_LOAD");
    check_int("const.mask0x0400_icon",
              DM1_V1_CPHUDAC_MASK0x0400_ICON_PC34, 0x0400,
              "DEFS.H:727 MASK0x0400_ICON");
    check_int("const.mask0x0800_panel",
              DM1_V1_CPHUDAC_MASK0x0800_PANEL_PC34, 0x0800,
              "DEFS.H:728 MASK0x0800_PANEL");
    check_int("const.mask0x1000_status_box",
              DM1_V1_CPHUDAC_MASK0x1000_STATUS_BOX_PC34, 0x1000,
              "DEFS.H:729 MASK0x1000_STATUS_BOX");
    check_int("const.mask0x2000_wounds",
              DM1_V1_CPHUDAC_MASK0x2000_WOUNDS_PC34, 0x2000,
              "DEFS.H:730 MASK0x2000_WOUNDS");
    check_int("const.mask0x4000_viewport",
              DM1_V1_CPHUDAC_MASK0x4000_VIEWPORT_PC34, 0x4000,
              "DEFS.H:731 MASK0x4000_VIEWPORT");
    check_int("const.mask0x8000_action_hand",
              DM1_V1_CPHUDAC_MASK0x8000_ACTION_HAND_PC34, 0x8000,
              "DEFS.H:732 MASK0x8000_ACTION_HAND");
    check_int("const.wound_count_max",
              DM1_V1_CPHUDAC_WOUND_COUNT_MAX_PC34, 6,
              "CHAMPION.C F0321:1847-1851 C00..C05 wound slot range");
    check_int("const.slot_ready_hand",
              DM1_V1_CPHUDAC_SLOT_READY_HAND_PC34, 0,
              "CHAMDRAW.C F0291 C00_SLOT_READY_HAND");
    check_int("const.slot_action_hand",
              DM1_V1_CPHUDAC_SLOT_ACTION_HAND_PC34, 1,
              "CHAMDRAW.C F0291 C01_SLOT_ACTION_HAND");
    check_int("const.slot_feet",
              DM1_V1_CPHUDAC_SLOT_FEET_PC34, 5,
              "CHAMDRAW.C F0291 C05_SLOT_FEET");
}

static void test_default_input_values(void)
{
    DM1_V1_ChampionPanelHudDamageAttributeCascadeInputPc34Compat input;

    memset(&input, 0xFF, sizeof(input));
    DM1_V1_ChampionPanelHudDamageAttributeCascade_DefaultInputPc34Compat(
        &input);
    check_int("default.champion_index", input.champion_index, 0,
              "default to champion 0 (ordinal 1)");
    check_int("default.current_health", input.current_health, 100,
              "healthy baseline 100 HP");
    check_int("default.pending_damage", input.pending_damage, 5,
              "synthetic non-zero pending damage");
    check_int("default.pending_wounds_mask", input.pending_wounds_mask, 0,
              "no wound bits by default");
    check_true("default.alive", input.alive,
               "default champion is alive");
    check_false("default.is_inventory_champion",
                input.is_inventory_champion,
                "default champion is non-inventory (i.e. ordinal != 1)");
    check_false("default.party_is_resting", input.party_is_resting,
                "default party is not resting");
}

static void test_apply_null_output_returns_zero(void)
{
    DM1_V1_ChampionPanelHudDamageAttributeCascadeInputPc34Compat input;

    DM1_V1_ChampionPanelHudDamageAttributeCascade_DefaultInputPc34Compat(
        &input);
    check_int("null_apply.build_return",
              DM1_V1_ChampionPanelHudDamageAttributeCascade_ApplyPc34Compat(
                  &input, NULL),
              0,
              "Apply with NULL out_result rejected before any work");
}

static void test_apply_negative_health_rejected(void)
{
    DM1_V1_ChampionPanelHudDamageAttributeCascadeInputPc34Compat input;
    DM1_V1_ChampionPanelHudDamageAttributeCascadeResultPc34Compat result;

    DM1_V1_ChampionPanelHudDamageAttributeCascade_DefaultInputPc34Compat(
        &input);
    input.current_health = -1;
    input.pending_damage = 5;
    input.alive = true;
    check_int("neg_h.build_return",
              DM1_V1_ChampionPanelHudDamageAttributeCascade_ApplyPc34Compat(
                  &input, &result),
              0,
              "Apply rejects negative current_health");
    check_true("neg_h.rejected_negative_health",
               result.rejected_negative_health,
               "CHAMPION.C F0320 current_health non-negative invariant");
    check_false("neg_h.valid", result.valid,
                "rejected result is not valid");
}

static void test_apply_champion_index_out_of_range(void)
{
    DM1_V1_ChampionPanelHudDamageAttributeCascadeInputPc34Compat input;
    DM1_V1_ChampionPanelHudDamageAttributeCascadeResultPc34Compat result;

    DM1_V1_ChampionPanelHudDamageAttributeCascade_DefaultInputPc34Compat(
        &input);
    input.champion_index = -1;
    input.current_health = 100;
    input.pending_damage = 5;
    input.alive = true;
    check_int("idx_neg.build_return",
              DM1_V1_ChampionPanelHudDamageAttributeCascade_ApplyPc34Compat(
                  &input, &result),
              0,
              "Apply rejects negative champion_index");
    check_true("idx_neg.rejected_champion_index",
               result.rejected_champion_index,
               "CHAMPION.C F0320:1721 for-loop bound [0,4)");

    DM1_V1_ChampionPanelHudDamageAttributeCascade_DefaultInputPc34Compat(
        &input);
    input.champion_index = DM1_V1_CPHUDAC_CHAMPION_COUNT_PC34;
    check_int("idx_high.build_return",
              DM1_V1_ChampionPanelHudDamageAttributeCascade_ApplyPc34Compat(
                  &input, &result),
              0,
              "Apply rejects champion_index == 4");
    check_true("idx_high.rejected_champion_index",
               result.rejected_champion_index,
               "CHAMPION.C F0320:1721 for-loop bound [0,4)");
}

static void test_apply_pending_damage_zero_skips(void)
{
    DM1_V1_ChampionPanelHudDamageAttributeCascadeInputPc34Compat input;
    DM1_V1_ChampionPanelHudDamageAttributeCascadeResultPc34Compat result;

    DM1_V1_ChampionPanelHudDamageAttributeCascade_DefaultInputPc34Compat(
        &input);
    input.pending_damage = 0;
    input.current_health = 100;
    input.pending_wounds_mask = 0;
    input.alive = true;
    check_int("zero.build_return",
              DM1_V1_ChampionPanelHudDamageAttributeCascade_ApplyPc34Compat(
                  &input, &result),
              1,
              "Apply returns 1 for valid pending_damage==0 input");
    check_true("zero.valid", result.valid,
               "Apply on pending_damage==0 is valid");
    check_true("zero.contract_only", result.contract_only,
               "Apply result is contract_only");
    check_true("zero.skipped_pending_damage_zero",
               result.skipped_pending_damage_zero,
               "CHAMPION.C F0320:1724-1725 G0409[i] == 0 continue");
    check_false("zero.skipped_dead_champion",
                result.skipped_dead_champion,
                "not a dead-champion skip");
    check_false("zero.applied_damage", result.applied_damage,
                "no damage applied when pending_damage == 0");
    check_false("zero.set_statistics_bit",
                result.set_statistics_bit,
                "F0320:1738 MASK0x0100_STATISTICS not set");
    check_false("zero.set_wounds_bit",
                result.set_wounds_bit,
                "F0320:1740 MASK0x2000_WOUNDS not set");
    check_int("zero.attributes_after_apply",
              result.attributes_after_apply,
              DM1_V1_CPHUDAC_ATTRIBUTES_NONE_PC34,
              "CHAMDRAW.C F0292:757 nine-bit mask stays 0");
    check_true("zero.f0292_will_short_circuit",
               result.f0292_will_short_circuit,
               "F0292 short-circuits when no bits set");
    check_false("zero.f0292_will_redraw_statistics",
                result.f0292_will_redraw_statistics,
                "F0292 does NOT redraw statistics");
    check_false("zero.f0292_will_redraw_wounds",
                result.f0292_will_redraw_wounds,
                "F0292 does NOT redraw wound slots");
    check_int("zero.outcome", (int)result.outcome,
              (int)DM1_V1_CPHUDAC_OUTCOME_PENDING_DAMAGE_ZERO_PC34,
              "outcome PENDING_DAMAGE_ZERO");
    check_int("zero.redraw", (int)result.redraw,
              (int)DM1_V1_CPHUDAC_REDRAW_NONE_PC34,
              "redraw kind NONE");
    check_int("zero.health_after_apply", result.health_after_apply, 100,
              "current_health unchanged on skip");
    check_int("zero.wound_count_after_apply",
              result.wound_count_after_apply, 0,
              "no wound bits accumulated on skip");
    check_int("zero.wound_slot_redraw_first",
              result.wound_slot_redraw_first, -1,
              "no wound-slot sweep when wounds bit clear");
    check_int("zero.wound_slot_redraw_last",
              result.wound_slot_redraw_last, -1,
              "no wound-slot sweep when wounds bit clear");
    check_true("zero.evidence", result.evidence != NULL,
               "evidence pointer set on every valid Apply result");
}

static void test_apply_dead_champion_skips(void)
{
    DM1_V1_ChampionPanelHudDamageAttributeCascadeInputPc34Compat input;
    DM1_V1_ChampionPanelHudDamageAttributeCascadeResultPc34Compat result;

    DM1_V1_ChampionPanelHudDamageAttributeCascade_DefaultInputPc34Compat(
        &input);
    input.alive = false;
    input.current_health = 0;
    input.pending_damage = 5;
    input.pending_wounds_mask = (1 << 0) | (1 << 3);
    check_int("dead.build_return",
              DM1_V1_ChampionPanelHudDamageAttributeCascade_ApplyPc34Compat(
                  &input, &result),
              1,
              "Apply on dead champion returns 1 (skip not reject)");
    check_true("dead.skipped_dead_champion",
               result.skipped_dead_champion,
               "CHAMPION.C F0320:1726-1727 CurrentHealth == 0 continue");
    check_int("dead.attributes_after_apply",
              result.attributes_after_apply,
              DM1_V1_CPHUDAC_ATTRIBUTES_NONE_PC34,
              "no STATISTICS / WOUNDS bits set for dead champion");
    check_int("dead.outcome", (int)result.outcome,
              (int)DM1_V1_CPHUDAC_OUTCOME_DEAD_CHAMPION_PC34,
              "outcome DEAD_CHAMPION");
    check_int("dead.redraw", (int)result.redraw,
              (int)DM1_V1_CPHUDAC_REDRAW_NONE_PC34,
              "redraw kind NONE");
    check_int("dead.health_after_apply", result.health_after_apply, 0,
              "current_health stays at 0 on dead skip");
}

static void test_apply_lethal_damage_kill_branch(void)
{
    DM1_V1_ChampionPanelHudDamageAttributeCascadeInputPc34Compat input;
    DM1_V1_ChampionPanelHudDamageAttributeCascadeResultPc34Compat result;

    DM1_V1_ChampionPanelHudDamageAttributeCascade_DefaultInputPc34Compat(
        &input);
    input.current_health = 30;
    input.alive = true;
    input.pending_damage = 100; /* lethal - would zero health */
    input.pending_wounds_mask = (1 << 1); /* action hand wound bit */
    check_int("lethal.build_return",
              DM1_V1_ChampionPanelHudDamageAttributeCascade_ApplyPc34Compat(
                  &input, &result),
              1,
              "Apply on lethal pending damage returns 1 (kill branch owns)");
    check_true("lethal.skipped_dead_champion",
               result.skipped_dead_champion,
               "F0319_CHAMPION_Kill owns the redraw on lethal damage");
    check_int("lethal.attributes_after_apply",
              result.attributes_after_apply,
              DM1_V1_CPHUDAC_ATTRIBUTES_NONE_PC34,
              "no STATISTICS / WOUNDS bits set on kill branch");
    check_int("lethal.health_after_apply", result.health_after_apply, 0,
              "CHAMPION.C F0320:1737 CurrentHealth pinned to 0 on kill");
    check_int("lethal.outcome", (int)result.outcome,
              (int)DM1_V1_CPHUDAC_OUTCOME_DEAD_CHAMPION_PC34,
              "outcome DEAD_CHAMPION on lethal branch");
    check_int("lethal.redraw", (int)result.redraw,
              (int)DM1_V1_CPHUDAC_REDRAW_NONE_PC34,
              "redraw kind NONE on lethal branch (F0319 owns)");
    check_int("lethal.wound_count_after_apply",
              result.wound_count_after_apply, 0,
              "wound bits are not accumulated when kill branch fires");
}

static void test_apply_damage_only_sets_statistics(void)
{
    DM1_V1_ChampionPanelHudDamageAttributeCascadeInputPc34Compat input;
    DM1_V1_ChampionPanelHudDamageAttributeCascadeResultPc34Compat result;

    DM1_V1_ChampionPanelHudDamageAttributeCascade_DefaultInputPc34Compat(
        &input);
    input.current_health = 100;
    input.alive = true;
    input.pending_damage = 5;
    input.pending_wounds_mask = 0;
    input.champion_index = 1;
    check_int("dmg_only.build_return",
              DM1_V1_ChampionPanelHudDamageAttributeCascade_ApplyPc34Compat(
                  &input, &result),
              1,
              "Apply on nonlethal damage returns 1");
    check_true("dmg_only.applied_damage", result.applied_damage,
               "CHAMPION.C F0320:1737 CurrentHealth -= pendingDamage");
    check_true("dmg_only.set_statistics_bit",
               result.set_statistics_bit,
               "CHAMPION.C F0320:1738 M008_SET MASK0x0100_STATISTICS");
    check_false("dmg_only.set_wounds_bit",
                result.set_wounds_bit,
                "F0320:1740 MASK0x2000_WOUNDS not set when wounds == 0");
    check_int("dmg_only.attributes_after_apply",
              result.attributes_after_apply,
              DM1_V1_CPHUDAC_MASK0x0100_STATISTICS_PC34,
              "Attributes == 0x0100 after nonlethal damage with no wounds");
    check_int("dmg_only.health_after_apply",
              result.health_after_apply, 95,
              "current_health = 100 - 5 = 95");
    check_int("dmg_only.wound_count_after_apply",
              result.wound_count_after_apply, 0,
              "no wound bits when wounds_mask == 0");
    check_int("dmg_only.outcome", (int)result.outcome,
              (int)DM1_V1_CPHUDAC_OUTCOME_DAMAGE_ONLY_PC34,
              "outcome DAMAGE_ONLY");
    check_int("dmg_only.redraw", (int)result.redraw,
              (int)DM1_V1_CPHUDAC_REDRAW_STATISTICS_PC34,
              "redraw kind REDRAW_STATISTICS");
    check_false("dmg_only.f0292_will_short_circuit",
                result.f0292_will_short_circuit,
                "F0292:757 short-circuit FAILS when STATISTICS bit set");
    check_true("dmg_only.f0292_will_redraw_statistics",
               result.f0292_will_redraw_statistics,
               "CHAMDRAW.C F0292:898 statistics redraw fires");
    check_false("dmg_only.f0292_will_redraw_wounds",
                result.f0292_will_redraw_wounds,
                "CHAMDRAW.C F0292:937 wounds redraw does NOT fire");
    check_true("dmg_only.f0292_will_clear_after_redraw",
               result.f0292_will_clear_after_redraw,
               "CHAMDRAW.C F0292:1110 M009_CLEAR planned");
    check_int("dmg_only.champion_index", result.champion_index, 1,
              "champion_index echo");
    check_int("dmg_only.champion_ordinal", result.champion_ordinal, 2,
              "champion_ordinal = champion_index + 1 (PC 3.4 EN rule)");
    /*
     * Damage-only path enters the late-stage branch; with wounds==0
     * the implementation explicitly sets wound-slot fields to -1.
     */
    check_int("dmg_only.wound_slot_redraw_first",
              result.wound_slot_redraw_first, -1,
              "late-stage no-wound: first sweep slot sentinel -1");
    check_int("dmg_only.wound_slot_redraw_last",
              result.wound_slot_redraw_last, -1,
              "late-stage no-wound: last sweep slot sentinel -1");
}

static void test_apply_damage_and_wounds_inventory_champion(void)
{
    DM1_V1_ChampionPanelHudDamageAttributeCascadeInputPc34Compat input;
    DM1_V1_ChampionPanelHudDamageAttributeCascadeResultPc34Compat result;

    DM1_V1_ChampionPanelHudDamageAttributeCascade_DefaultInputPc34Compat(
        &input);
    input.current_health = 100;
    input.alive = true;
    input.pending_damage = 12;
    /* C00 ready hand + C03 torso + C05 feet - three wounds */
    input.pending_wounds_mask = (1 << 0) | (1 << 3) | (1 << 5);
    input.is_inventory_champion = true;
    input.champion_index = 0;
    check_int("dmg_wound_inv.build_return",
              DM1_V1_ChampionPanelHudDamageAttributeCascade_ApplyPc34Compat(
                  &input, &result),
              1,
              "Apply on damage + wounds returns 1");
    check_true("dmg_wound_inv.set_statistics_bit",
               result.set_statistics_bit,
               "F0320:1738 MASK0x0100_STATISTICS set");
    check_true("dmg_wound_inv.set_wounds_bit",
               result.set_wounds_bit,
               "F0320:1740 MASK0x2000_WOUNDS set when wounds > 0");
    check_int("dmg_wound_inv.attributes_after_apply",
              result.attributes_after_apply,
              DM1_V1_CPHUDAC_MASK0x0100_STATISTICS_PC34 |
                  DM1_V1_CPHUDAC_MASK0x2000_WOUNDS_PC34,
              "Attributes == 0x2100 after damage + wounds");
    check_int("dmg_wound_inv.wound_bits_after_apply",
              result.wound_bits_after_apply,
              input.pending_wounds_mask,
              "wound mask preserved");
    check_int("dmg_wound_inv.wound_count_after_apply",
              result.wound_count_after_apply, 3,
              "popcount of three wound bits");
    check_int("dmg_wound_inv.health_after_apply",
              result.health_after_apply, 88,
              "current_health = 100 - 12 = 88");
    check_int("dmg_wound_inv.outcome", (int)result.outcome,
              (int)DM1_V1_CPHUDAC_OUTCOME_DAMAGE_AND_WOUNDS_PC34,
              "outcome DAMAGE_AND_WOUNDS");
    check_int("dmg_wound_inv.redraw", (int)result.redraw,
              (int)DM1_V1_CPHUDAC_REDRAW_STATISTICS_AND_WOUND_SLOTS_PC34,
              "redraw kind REDRAW_STATISTICS_AND_WOUND_SLOTS");
    check_false("dmg_wound_inv.f0292_will_short_circuit",
                result.f0292_will_short_circuit,
                "F0292:757 short-circuit FAILS when STATISTICS+WOUNDS set");
    check_true("dmg_wound_inv.f0292_will_redraw_statistics",
               result.f0292_will_redraw_statistics,
               "F0292:898 statistics redraw fires");
    check_true("dmg_wound_inv.f0292_will_redraw_wounds",
               result.f0292_will_redraw_wounds,
               "F0292:937 wounds redraw fires");
    check_int("dmg_wound_inv.wound_slot_redraw_first",
              result.wound_slot_redraw_first, 0,
              "inventory champion: first wound slot is C00 (ready hand)");
    check_int("dmg_wound_inv.wound_slot_redraw_last",
              result.wound_slot_redraw_last, 5,
              "inventory champion: last wound slot is C05 (feet)");
}

static void test_apply_damage_and_wounds_non_inventory_champion(void)
{
    DM1_V1_ChampionPanelHudDamageAttributeCascadeInputPc34Compat input;
    DM1_V1_ChampionPanelHudDamageAttributeCascadeResultPc34Compat result;

    DM1_V1_ChampionPanelHudDamageAttributeCascade_DefaultInputPc34Compat(
        &input);
    input.current_health = 100;
    input.alive = true;
    input.pending_damage = 8;
    /* C00 ready hand + C03 torso + C05 feet - only C00 falls inside
     * the non-inventory sub-range [0,1]; C03/C05 must be clamped out */
    input.pending_wounds_mask = (1 << 0) | (1 << 3) | (1 << 5);
    input.is_inventory_champion = false;
    input.champion_index = 1;
    check_int("dmg_wound_noninv.build_return",
              DM1_V1_ChampionPanelHudDamageAttributeCascade_ApplyPc34Compat(
                  &input, &result),
              1,
              "Apply on damage + wounds (non-inventory) returns 1");
    check_int("dmg_wound_noninv.attributes_after_apply",
              result.attributes_after_apply,
              DM1_V1_CPHUDAC_MASK0x0100_STATISTICS_PC34 |
                  DM1_V1_CPHUDAC_MASK0x2000_WOUNDS_PC34,
              "Attributes == 0x2100 regardless of inventory flag");
    check_int("dmg_wound_noninv.redraw", (int)result.redraw,
              (int)DM1_V1_CPHUDAC_REDRAW_STATISTICS_AND_WOUND_SLOTS_PC34,
              "redraw kind REDRAW_STATISTICS_AND_WOUND_SLOTS");
    check_int("dmg_wound_noninv.wound_slot_redraw_first",
              result.wound_slot_redraw_first, 0,
              "non-inventory champion: clamped to C00 sub-range");
    check_int("dmg_wound_noninv.wound_slot_redraw_last",
              result.wound_slot_redraw_last,
              DM1_V1_CPHUDAC_SLOT_ACTION_HAND_PC34,
              "non-inventory champion: WOUNDS redraw reaches C01");
}

static void test_apply_non_inventory_no_action_hand_wound(void)
{
    DM1_V1_ChampionPanelHudDamageAttributeCascadeInputPc34Compat input;
    DM1_V1_ChampionPanelHudDamageAttributeCascadeResultPc34Compat result;

    DM1_V1_ChampionPanelHudDamageAttributeCascade_DefaultInputPc34Compat(
        &input);
    input.current_health = 100;
    input.alive = true;
    input.pending_damage = 8;
    /* C03 torso only - outside [0,1]; F0292 still redraws the whole
     * visible non-inventory hand-slot range C01..C00 once WOUNDS is set. */
    input.pending_wounds_mask = (1 << 3);
    input.is_inventory_champion = false;
    input.champion_index = 2;
    check_int("dmg_wound_noninv_empty.build_return",
              DM1_V1_ChampionPanelHudDamageAttributeCascade_ApplyPc34Compat(
                  &input, &result),
              1,
              "Apply on damage + non-action-hand wound returns 1");
    check_int("dmg_wound_noninv_empty.attributes_after_apply",
              result.attributes_after_apply,
              DM1_V1_CPHUDAC_MASK0x0100_STATISTICS_PC34 |
                  DM1_V1_CPHUDAC_MASK0x2000_WOUNDS_PC34,
              "Attributes == 0x2100 (WOUNDS bit still set for F0292)");
    check_int("dmg_wound_noninv_empty.wound_slot_redraw_first",
              result.wound_slot_redraw_first,
              DM1_V1_CPHUDAC_SLOT_READY_HAND_PC34,
              "non-inventory WOUNDS redraw still starts at C00");
    check_int("dmg_wound_noninv_empty.wound_slot_redraw_last",
              result.wound_slot_redraw_last,
              DM1_V1_CPHUDAC_SLOT_ACTION_HAND_PC34,
              "non-inventory WOUNDS redraw still reaches C01");
}

static void test_apply_wound_mask_trimmed_to_six_bits(void)
{
    DM1_V1_ChampionPanelHudDamageAttributeCascadeInputPc34Compat input;
    DM1_V1_ChampionPanelHudDamageAttributeCascadeResultPc34Compat result;

    DM1_V1_ChampionPanelHudDamageAttributeCascade_DefaultInputPc34Compat(
        &input);
    input.current_health = 100;
    input.alive = true;
    input.pending_damage = 4;
    /* Bits 6 and 7 are outside the documented C00..C05 slot range;
     * the contract must clamp the wound mask and keep wound_count <= 6. */
    input.pending_wounds_mask = (1 << 0) | (1 << 1) | (1 << 6) | (1 << 7);
    input.is_inventory_champion = true;
    check_int("trim.build_return",
              DM1_V1_ChampionPanelHudDamageAttributeCascade_ApplyPc34Compat(
                  &input, &result),
              1,
              "Apply with bits beyond C00..C05 returns 1");
    check_int("trim.wound_count_after_apply",
              result.wound_count_after_apply, 2,
              "wound_count clamped to bits 0..5 (popcount = 2)");
    check_true("trim.wound_count_le_max",
               result.wound_count_after_apply <=
                   DM1_V1_CPHUDAC_WOUND_COUNT_MAX_PC34,
               "wound_count never exceeds DM1_V1_CPHUDAC_WOUND_COUNT_MAX_PC34=6");
    check_int("trim.wound_bits_after_apply",
              result.wound_bits_after_apply,
              (1 << 0) | (1 << 1),
              "wound mask trimmed: only bits 0..1 survive");
}

static void test_redraw_consumes_attributes(void)
{
    DM1_V1_ChampionPanelHudDamageAttributeCascadeInputPc34Compat input;
    DM1_V1_ChampionPanelHudDamageAttributeCascadeResultPc34Compat state;
    DM1_V1_ChampionPanelHudDamageAttributeCascadeResultPc34Compat cleared;

    DM1_V1_ChampionPanelHudDamageAttributeCascade_DefaultInputPc34Compat(
        &input);
    input.current_health = 100;
    input.alive = true;
    input.pending_damage = 7;
    input.pending_wounds_mask = (1 << 2); /* head wound */
    input.is_inventory_champion = true;
    DM1_V1_ChampionPanelHudDamageAttributeCascade_ApplyPc34Compat(
        &input, &state);

    check_int("redraw.build_return",
              DM1_V1_ChampionPanelHudDamageAttributeCascade_RedrawPc34Compat(
                  &state, &cleared),
              1,
              "Redraw with valid state returns 1");
    check_true("redraw.valid", cleared.valid,
               "Redraw result is valid");
    check_true("redraw.contract_only", cleared.contract_only,
               "Redraw result is contract_only");
    check_int("redraw.attributes_after_apply",
              cleared.attributes_after_apply,
              DM1_V1_CPHUDAC_ATTRIBUTES_NONE_PC34,
              "CHAMDRAW.C F0292:1110 M009_CLEAR clears all nine bits");
    check_true("redraw.f0292_will_short_circuit",
               cleared.f0292_will_short_circuit,
               "Next-tick F0292:757 short-circuit TRUE after clear");
    check_int("redraw.redraw", (int)cleared.redraw,
              (int)DM1_V1_CPHUDAC_REDRAW_FULL_REDRAW_MASK_PC34,
              "redraw kind REDRAW_FULL_REDRAW_MASK");
    check_true("redraw.f0292_will_clear_after_redraw",
               cleared.f0292_will_clear_after_redraw,
               "Redraw result records f0292_will_clear_after_redraw=true");
    check_int("redraw.champion_index", cleared.champion_index,
              state.champion_index,
              "Redraw echoes champion_index");
}

static void test_redraw_null_state_is_noop(void)
{
    DM1_V1_ChampionPanelHudDamageAttributeCascadeResultPc34Compat cleared;

    check_int("redraw_null_state.build_return",
              DM1_V1_ChampionPanelHudDamageAttributeCascade_RedrawPc34Compat(
                  NULL, &cleared),
              1,
              "Redraw with NULL state returns 1 (F0292:757 short-circuit)");
    check_true("redraw_null_state.valid", cleared.valid,
               "Redraw NULL-state result is valid (no-op is not an error)");
    check_int("redraw_null_state.attributes_after_apply",
              cleared.attributes_after_apply,
              DM1_V1_CPHUDAC_ATTRIBUTES_NONE_PC34,
              "Redraw NULL-state: attributes stays NONE");
    check_true("redraw_null_state.f0292_will_short_circuit",
               cleared.f0292_will_short_circuit,
               "Redraw NULL-state: F0292:757 short-circuit TRUE");
    check_int("redraw_null_state.redraw", (int)cleared.redraw,
              (int)DM1_V1_CPHUDAC_REDRAW_NONE_PC34,
              "Redraw NULL-state: redraw kind NONE");
}

static void test_redraw_null_output_returns_zero(void)
{
    DM1_V1_ChampionPanelHudDamageAttributeCascadeInputPc34Compat input;
    DM1_V1_ChampionPanelHudDamageAttributeCascadeResultPc34Compat state;

    DM1_V1_ChampionPanelHudDamageAttributeCascade_DefaultInputPc34Compat(
        &input);
    input.pending_damage = 5;
    DM1_V1_ChampionPanelHudDamageAttributeCascade_ApplyPc34Compat(
        &input, &state);
    check_int("redraw_null_out.build_return",
              DM1_V1_ChampionPanelHudDamageAttributeCascade_RedrawPc34Compat(
                  &state, NULL),
              0,
              "Redraw with NULL out_result rejected");
}

static void test_redraw_rejects_invalid_state(void)
{
    DM1_V1_ChampionPanelHudDamageAttributeCascadeResultPc34Compat bad_state;
    DM1_V1_ChampionPanelHudDamageAttributeCascadeResultPc34Compat cleared;

    memset(&bad_state, 0, sizeof(bad_state));
    bad_state.valid = false;
    check_int("redraw_bad_state.build_return",
              DM1_V1_ChampionPanelHudDamageAttributeCascade_RedrawPc34Compat(
                  &bad_state, &cleared),
              0,
              "Redraw with invalid state rejected");
    check_true("redraw_bad_state.rejected_null_output",
               cleared.rejected_null_output,
               "Redraw invalid state marks rejected_null_output");
}

static void test_apply_zero_attributes_short_circuit_predicate(void)
{
    /*
     * The F0292:757 short-circuit predicate is encoded as
     * (attributes_after_apply & 0xFF80u) == 0u - any of the nine
     * redraw bits (MASK0x0080..MASK0x8000) defeats it. Verify both
     * sides of the predicate directly with the helper invariants.
     */
    DM1_V1_ChampionPanelHudDamageAttributeCascadeInputPc34Compat input;
    DM1_V1_ChampionPanelHudDamageAttributeCascadeResultPc34Compat result;

    /* Zero-attribute case: short-circuit TRUE */
    DM1_V1_ChampionPanelHudDamageAttributeCascade_DefaultInputPc34Compat(
        &input);
    input.pending_damage = 0;
    input.current_health = 100;
    input.alive = true;
    DM1_V1_ChampionPanelHudDamageAttributeCascade_ApplyPc34Compat(
        &input, &result);
    check_int("sc_zero.mask_clear",
              result.attributes_after_apply & 0xFF80u, 0,
              "F0292:757 predicate: zero attributes -> mask clears");
    check_true("sc_zero.f0292_will_short_circuit",
               result.f0292_will_short_circuit,
               "F0292:757 short-circuit TRUE on zero attributes");

    /* STATISTICS bit set case: short-circuit FALSE */
    DM1_V1_ChampionPanelHudDamageAttributeCascade_DefaultInputPc34Compat(
        &input);
    input.pending_damage = 5;
    input.current_health = 100;
    input.alive = true;
    input.pending_wounds_mask = 0;
    DM1_V1_ChampionPanelHudDamageAttributeCascade_ApplyPc34Compat(
        &input, &result);
    check_int("sc_stats.mask_nonzero",
              (result.attributes_after_apply & 0xFF80u) != 0, 1,
              "F0292:757 predicate: STATISTICS bit set -> mask non-zero");
    check_false("sc_stats.f0292_will_short_circuit",
                result.f0292_will_short_circuit,
                "F0292:757 short-circuit FALSE when STATISTICS bit set");

    /* WOUNDS bit set case: short-circuit FALSE */
    DM1_V1_ChampionPanelHudDamageAttributeCascade_DefaultInputPc34Compat(
        &input);
    input.pending_damage = 5;
    input.current_health = 100;
    input.alive = true;
    input.pending_wounds_mask = (1 << 2);
    DM1_V1_ChampionPanelHudDamageAttributeCascade_ApplyPc34Compat(
        &input, &result);
    check_int("sc_wounds.mask_nonzero",
              (result.attributes_after_apply & 0xFF80u) != 0, 1,
              "F0292:757 predicate: WOUNDS bit set -> mask non-zero");
    check_false("sc_wounds.f0292_will_short_circuit",
                result.f0292_will_short_circuit,
                "F0292:757 short-circuit FALSE when WOUNDS bit set");
}

static void test_apply_per_champion_index_passthrough(void)
{
    DM1_V1_ChampionPanelHudDamageAttributeCascadeInputPc34Compat input;
    DM1_V1_ChampionPanelHudDamageAttributeCascadeResultPc34Compat result;
    int ci;

    for (ci = 0; ci < DM1_V1_CPHUDAC_CHAMPION_COUNT_PC34; ++ci) {
        DM1_V1_ChampionPanelHudDamageAttributeCascade_DefaultInputPc34Compat(
            &input);
        input.champion_index = ci;
        input.current_health = 50;
        input.alive = true;
        input.pending_damage = 3;
        input.pending_wounds_mask = 0;
        DM1_V1_ChampionPanelHudDamageAttributeCascade_ApplyPc34Compat(
            &input, &result);
        check_int("pc_echo.champion_index", result.champion_index, ci,
                  "champion_index echoed per slot");
        check_int("pc_echo.champion_ordinal", result.champion_ordinal, ci + 1,
                  "champion_ordinal = champion_index + 1");
        check_int("pc_echo.attributes",
                  result.attributes_after_apply,
                  DM1_V1_CPHUDAC_MASK0x0100_STATISTICS_PC34,
                  "Per-champion STATISTICS bit set after nonlethal damage");
        check_int("pc_echo.health", result.health_after_apply, 47,
                  "Per-champion health = 50 - 3 = 47");
    }
}

static void test_determinism_fnv1a_identical_inputs(void)
{
    DM1_V1_ChampionPanelHudDamageAttributeCascadeInputPc34Compat input;
    DM1_V1_ChampionPanelHudDamageAttributeCascadeResultPc34Compat result_a;
    DM1_V1_ChampionPanelHudDamageAttributeCascadeResultPc34Compat result_b;
    unsigned int hash_a;
    unsigned int hash_b;

    DM1_V1_ChampionPanelHudDamageAttributeCascade_DefaultInputPc34Compat(
        &input);
    input.champion_index = 2;
    input.current_health = 80;
    input.alive = true;
    input.pending_damage = 14;
    input.pending_wounds_mask = (1 << 1) | (1 << 4);
    input.is_inventory_champion = false;

    DM1_V1_ChampionPanelHudDamageAttributeCascade_ApplyPc34Compat(
        &input, &result_a);
    DM1_V1_ChampionPanelHudDamageAttributeCascade_ApplyPc34Compat(
        &input, &result_b);

    hash_a = fnv1a_result_hash(&result_a);
    hash_b = fnv1a_result_hash(&result_b);

    check_int("fnv.hash_match", (int)(hash_a == hash_b), 1,
              "FNV-1a hash of two independent Apply calls is stable");
    check_int("fnv.hash_nonzero", (int)(hash_a != 0u), 1,
              "FNV-1a hash is non-zero for a non-trivial Apply result");
}

static void test_redraw_after_apply_clear_next_tick_noop(void)
{
    /*
     * After Redraw consumes the Attributes field, a hypothetical second
     * F0292 call on the same tick must short-circuit at line 757
     * (no double-draw).
     */
    DM1_V1_ChampionPanelHudDamageAttributeCascadeInputPc34Compat input;
    DM1_V1_ChampionPanelHudDamageAttributeCascadeResultPc34Compat state;
    DM1_V1_ChampionPanelHudDamageAttributeCascadeResultPc34Compat cleared;

    DM1_V1_ChampionPanelHudDamageAttributeCascade_DefaultInputPc34Compat(
        &input);
    input.current_health = 100;
    input.alive = true;
    input.pending_damage = 6;
    input.pending_wounds_mask = (1 << 2);
    input.is_inventory_champion = true;
    DM1_V1_ChampionPanelHudDamageAttributeCascade_ApplyPc34Compat(
        &input, &state);
    check_int("next_tick.pre_clear_attributes",
              state.attributes_after_apply,
              DM1_V1_CPHUDAC_MASK0x0100_STATISTICS_PC34 |
                  DM1_V1_CPHUDAC_MASK0x2000_WOUNDS_PC34,
              "After Apply: attributes have STATISTICS + WOUNDS bits");

    DM1_V1_ChampionPanelHudDamageAttributeCascade_RedrawPc34Compat(
        &state, &cleared);
    check_int("next_tick.post_clear_attributes",
              cleared.attributes_after_apply,
              DM1_V1_CPHUDAC_ATTRIBUTES_NONE_PC34,
              "After Redraw: attributes fully cleared by F0292:1110");
    check_true("next_tick.post_clear_short_circuit",
               cleared.f0292_will_short_circuit,
               "Next-tick F0292:757 short-circuit TRUE after clear");
}

int main(void)
{
    test_evidence_anchors();
    test_constants();
    test_default_input_values();
    test_apply_null_output_returns_zero();
    test_apply_negative_health_rejected();
    test_apply_champion_index_out_of_range();
    test_apply_pending_damage_zero_skips();
    test_apply_dead_champion_skips();
    test_apply_lethal_damage_kill_branch();
    test_apply_damage_only_sets_statistics();
    test_apply_damage_and_wounds_inventory_champion();
    test_apply_damage_and_wounds_non_inventory_champion();
    test_apply_non_inventory_no_action_hand_wound();
    test_apply_wound_mask_trimmed_to_six_bits();
    test_redraw_consumes_attributes();
    test_redraw_null_state_is_noop();
    test_redraw_null_output_returns_zero();
    test_redraw_rejects_invalid_state();
    test_apply_zero_attributes_short_circuit_predicate();
    test_apply_per_champion_index_passthrough();
    test_determinism_fnv1a_identical_inputs();
    test_redraw_after_apply_clear_next_tick_noop();

    printf("dm1_v1_champion_panel_hud_damage_attribute_cascade_pc34_compat: "
           "assertions=%d failures=%d\n",
           g_assertions, g_failures);
    return g_failures ? 1 : 0;
}
