#include "dm1/dm1_v1_champion_panel_pending_damage_apply_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

static void expect_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d at %s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d (%s)\n", id, want, anchor);
    }
}

static void expect_hex(const char *id, uint32_t got, uint32_t want,
                       const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=0x%04X want=0x%04X at %s\n",
               id, (unsigned)got, (unsigned)want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == 0x%04X (%s)\n", id, (unsigned)want, anchor);
    }
}

static void expect_bool(const char *id, bool got, bool want,
                        const char *anchor)
{
    expect_int(id, got ? 1 : 0, want ? 1 : 0, anchor);
}

static void expect_str_eq(const char *id, const char *got, const char *want,
                          const char *anchor)
{
    ++g_assertions;
    if (!got || !want || strcmp(got, want) != 0) {
        printf("FAIL %s got=\"%s\" want=\"%s\" at %s\n",
               id, got ? got : "(null)", want ? want : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s == \"%s\" (%s)\n", id, want, anchor);
    }
}

static void expect_contains(const char *id, const char *haystack,
                            const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing \"%s\" at %s\n",
               id, needle ? needle : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains \"%s\" (%s)\n", id, needle, anchor);
    }
}

static DM1_V1_ChampionPanelPendingDamageApplyInputPc34Compat
base_input(void)
{
    DM1_V1_ChampionPanelPendingDamageApplyInputPc34Compat input;
    DM1_V1_ChampionPanelPendingDamageApply_DefaultInputPc34Compat(&input);
    input.champion_index = 1;
    input.pending_damage = 7;
    input.pending_wounds = 0;
    input.current_health = 40;
    input.was_alive_before = true;
    input.attributes_before = 0;
    return input;
}

static void expect_common(const char *prefix,
                          const DM1_V1_ChampionPanelPendingDamageApplyResultPc34Compat *result,
                          int champion_index,
                          int pending_damage,
                          int pending_wounds,
                          int current_health_before,
                          uint32_t attrs_before)
{
    char id[128];

    snprintf(id, sizeof(id), "%s.valid", prefix);
    expect_bool(id, result->valid, true,
                "CHAMPION.C F0320:1719-1740 valid synthetic route");
    snprintf(id, sizeof(id), "%s.contract_only", prefix);
    expect_bool(id, result->contract_only, true,
                "F0320 pre-damage mutation gate is contract-only");
    snprintf(id, sizeof(id), "%s.champion_index", prefix);
    expect_int(id, result->champion_index, champion_index,
               "CHAMPION.C F0320:1719 active champion index");
    snprintf(id, sizeof(id), "%s.champion_ordinal", prefix);
    expect_int(id, result->champion_ordinal, champion_index + 1,
               "COMPILE.H M000_INDEX_TO_ORDINAL(value) ((value) + 1)");
    snprintf(id, sizeof(id), "%s.pending_damage", prefix);
    expect_int(id, result->pending_damage, pending_damage,
               "CHAMPION.C F0320:1723 G0409 pending damage read");
    snprintf(id, sizeof(id), "%s.pending_wounds", prefix);
    expect_int(id, result->pending_wounds, pending_wounds,
               "CHAMPION.C F0320:1720 G0410 pending wounds read");
    snprintf(id, sizeof(id), "%s.wounds_applied", prefix);
    expect_int(id, result->wounds_applied, pending_wounds,
               "CHAMPION.C F0320:1720 champion->Wounds |= pendingWounds");
    snprintf(id, sizeof(id), "%s.health_before", prefix);
    expect_int(id, result->current_health_before, current_health_before,
               "CHAMPION.C F0320:1728 CurrentHealth guard");
    snprintf(id, sizeof(id), "%s.attrs_before", prefix);
    expect_hex(id, result->attributes_before, attrs_before,
               "COMPILE.H:1042 M008_SET preserves pre-existing bits");
    snprintf(id, sizeof(id), "%s.evidence", prefix);
    expect_bool(id, result->evidence != NULL, true,
                "F0320 source evidence pointer");
}

static void test_evidence(void)
{
    const DM1_V1_ChampionPanelPendingDamageApplyEvidencePc34Compat *e =
        DM1_V1_ChampionPanelPendingDamageApply_EvidencePc34Compat();
    const char *source =
        DM1_V1_ChampionPanelPendingDamageApply_SourceEvidencePc34Compat();

    expect_bool("evidence.contract_only", e->contract_only, true,
                "contract-only no real asset claim");
    expect_str_eq("evidence.applier", e->applier_function_anchor,
                  "CHAMPION.C F0320_CHAMPION_ApplyAndDrawPendingDamageAndWounds:1689-1800",
                  "CHAMPION.C F0320");
    expect_str_eq("evidence.kill", e->kill_function_anchor,
                  "CHAMPION.C F0319_CHAMPION_Kill:1552-1609",
                  "CHAMPION.C F0319");
    expect_contains("evidence.m008", e->m008_set_anchor, "M008_SET",
                    "COMPILE.H:1042");
    expect_contains("evidence.statistics", e->defs_mask_anchor,
                    "MASK0x0100_STATISTICS", "DEFS.H:725");
    expect_contains("evidence.status_box", e->defs_status_box_anchor,
                    "MASK0x1000_STATUS_BOX", "DEFS.H:729");
    expect_contains("evidence.wounds", e->defs_wounds_anchor,
                    "MASK0x2000_WOUNDS", "DEFS.H:730");
    expect_contains("evidence.loop", e->loop_init_anchor,
                    "1719-1722", "CHAMPION.C F0320");
    expect_contains("evidence.dead", e->dead_champion_skip_anchor,
                    "1728-1729", "CHAMPION.C F0320");
    expect_contains("evidence.lethal", e->lethal_branch_anchor,
                    "1731-1734", "CHAMPION.C F0320");
    expect_contains("evidence.nonlethal", e->nonlethal_branch_anchor,
                    "1735-1737", "CHAMPION.C F0320");
    expect_contains("evidence.conditional_wounds",
                    e->conditional_wounds_anchor, "1738-1739",
                    "CHAMPION.C F0320");
    expect_contains("evidence.scope", e->scope_note,
                    "contract-only", "bounded synthetic scope");
    expect_contains("evidence.no_real_asset", e->no_real_asset_claim,
                    "without claiming real-asset parity",
                    "no original-DOS pixel claim");

    expect_contains("source.loop", source, "G0409", "F0320 pending damage");
    expect_contains("source.wounds", source, "G0410", "F0320 pending wounds");
    expect_contains("source.dead", source, "already-dead champion",
                    "F0320 dead skip");
    expect_contains("source.kill", source, "F0319_CHAMPION_Kill",
                    "F0320 lethal branch");
    expect_contains("source.no_f0623", source, "never reach the F0623",
                    "F0320 kill branch boundary");
}

static void test_constants(void)
{
    expect_int("const.champion_count", DM1_V1_CPDA_CHAMPION_COUNT_PC34, 4,
               "CHAMPION.C F0320:1719-1721 champion panel slots");
    expect_int("const.pending_damage_max",
               DM1_V1_CPDA_PENDING_DAMAGE_MAX_PC34, 32767,
               "F0320 synthetic int16 pending damage cap");
    expect_int("const.pending_wounds_max",
               DM1_V1_CPDA_PENDING_WOUNDS_MAX_PC34, 65535,
               "G0410 pending wounds unsigned 16-bit accumulator");
    expect_hex("const.none", DM1_V1_CPDA_ATTR_NONE_PC34, 0x0000u,
               "DEFS.H attributes");
    expect_hex("const.statistics", DM1_V1_CPDA_ATTR_STATISTICS_PC34,
               0x0100u, "DEFS.H:725 MASK0x0100_STATISTICS");
    expect_hex("const.load", DM1_V1_CPDA_ATTR_LOAD_PC34, 0x0200u,
               "DEFS.H mask neighbor preserved");
    expect_hex("const.icon", DM1_V1_CPDA_ATTR_ICON_PC34, 0x0400u,
               "DEFS.H mask neighbor preserved");
    expect_hex("const.panel", DM1_V1_CPDA_ATTR_PANEL_PC34, 0x0800u,
               "DEFS.H mask neighbor preserved");
    expect_hex("const.status_box", DM1_V1_CPDA_ATTR_STATUS_BOX_PC34,
               0x1000u, "DEFS.H:729 MASK0x1000_STATUS_BOX");
    expect_hex("const.wounds", DM1_V1_CPDA_ATTR_WOUNDS_PC34, 0x2000u,
               "DEFS.H:730 MASK0x2000_WOUNDS");
    expect_hex("const.action_hand", DM1_V1_CPDA_ATTR_ACTION_HAND_PC34,
               0x8000u, "DEFS.H mask neighbor preserved");
}

static void test_default_and_zero_pending(void)
{
    DM1_V1_ChampionPanelPendingDamageApplyResultPc34Compat result;
    DM1_V1_ChampionPanelPendingDamageApplyInputPc34Compat input;

    expect_int("default.build",
               DM1_V1_ChampionPanelPendingDamageApply_BuildPc34Compat(NULL,
                                                                      &result),
               1, "default input follows F0320 zero-pending guard");
    expect_common("default", &result, 0, 0, 0, 100,
                  DM1_V1_CPDA_ATTR_NONE_PC34);
    expect_int("default.outcome", result.outcome,
               DM1_V1_CPDA_OUTCOME_SKIPPED_NO_PENDING_PC34,
               "CHAMPION.C F0320:1723-1725 pendingDamage == 0");
    expect_int("default.health_after", result.current_health_after, 100,
               "F0320 no pending damage leaves health unchanged");
    expect_bool("default.alive_after", result.alive_after, true,
                "F0320 no pending damage leaves alive flag unchanged");
    expect_hex("default.attrs_after", result.attributes_after,
               DM1_V1_CPDA_ATTR_NONE_PC34,
               "F0320:1737/1739 not reached");

    input = base_input();
    input.pending_damage = 0;
    input.pending_wounds = 0x0021;
    input.attributes_before = DM1_V1_CPDA_ATTR_PANEL_PC34;
    expect_int("zero_wounds.build",
               DM1_V1_ChampionPanelPendingDamageApply_BuildPc34Compat(&input,
                                                                      &result),
               1, "F0320 mounts Wounds before pendingDamage guard");
    expect_common("zero_wounds", &result, 1, 0, 0x0021, 40,
                  DM1_V1_CPDA_ATTR_PANEL_PC34);
    expect_int("zero_wounds.outcome", result.outcome,
               DM1_V1_CPDA_OUTCOME_SKIPPED_NO_PENDING_PC34,
               "CHAMPION.C F0320:1723-1725 continue");
    expect_hex("zero_wounds.attrs_after", result.attributes_after,
               DM1_V1_CPDA_ATTR_PANEL_PC34,
               "MASK0x2000_WOUNDS not set unless F0320 reaches 1738-1739");
    expect_bool("zero_wounds.no_mouse", result.mouse_screen_update_enabled,
                false, "F0320:1741 not reached");
}

static void test_nonlethal_without_wounds(void)
{
    DM1_V1_ChampionPanelPendingDamageApplyInputPc34Compat input = base_input();
    DM1_V1_ChampionPanelPendingDamageApplyResultPc34Compat result;

    input.champion_index = 2;
    input.pending_damage = 13;
    input.pending_wounds = 0;
    input.current_health = 50;
    input.attributes_before =
        DM1_V1_CPDA_ATTR_LOAD_PC34 | DM1_V1_CPDA_ATTR_ICON_PC34;

    expect_int("nonlethal.build",
               DM1_V1_ChampionPanelPendingDamageApply_BuildPc34Compat(&input,
                                                                      &result),
               1, "CHAMPION.C F0320:1735-1737 non-lethal branch");
    expect_common("nonlethal", &result, 2, 13, 0, 50,
                  DM1_V1_CPDA_ATTR_LOAD_PC34 |
                  DM1_V1_CPDA_ATTR_ICON_PC34);
    expect_int("nonlethal.outcome", result.outcome,
               DM1_V1_CPDA_OUTCOME_HEALTH_UPDATED_PC34,
               "F0320 non-lethal damage");
    expect_int("nonlethal.damage_applied", result.pending_damage_applied, 13,
               "F0320:1731-1736 subtracts pending damage");
    expect_int("nonlethal.health_after", result.current_health_after, 37,
               "F0320:1736 CurrentHealth writeback");
    expect_bool("nonlethal.alive_after", result.alive_after, true,
                "non-lethal branch keeps champion alive");
    expect_hex("nonlethal.attrs_after", result.attributes_after,
               DM1_V1_CPDA_ATTR_LOAD_PC34 |
               DM1_V1_CPDA_ATTR_ICON_PC34 |
               DM1_V1_CPDA_ATTR_STATISTICS_PC34,
               "F0320:1737 sets STATISTICS and preserves other bits");
    expect_bool("nonlethal.f0319", result.f0319_kill_called, false,
                "F0319 not called on non-lethal branch");
    expect_bool("nonlethal.mouse", result.mouse_screen_update_enabled, true,
                "F0320:1741 post-mutation draw path becomes reachable");
}

static void test_nonlethal_with_wounds(void)
{
    DM1_V1_ChampionPanelPendingDamageApplyInputPc34Compat input = base_input();
    DM1_V1_ChampionPanelPendingDamageApplyResultPc34Compat result;

    input.pending_damage = 1;
    input.pending_wounds = 0x0033;
    input.current_health = 2;
    input.attributes_before = DM1_V1_CPDA_ATTR_ACTION_HAND_PC34;

    expect_int("wounded.build",
               DM1_V1_ChampionPanelPendingDamageApply_BuildPc34Compat(&input,
                                                                      &result),
               1, "CHAMPION.C F0320:1738-1739 conditional WOUNDS attr");
    expect_common("wounded", &result, 1, 1, 0x0033, 2,
                  DM1_V1_CPDA_ATTR_ACTION_HAND_PC34);
    expect_int("wounded.outcome", result.outcome,
               DM1_V1_CPDA_OUTCOME_HEALTH_UPDATED_PC34,
               "non-lethal wounded branch");
    expect_int("wounded.health_after", result.current_health_after, 1,
               "F0320:1736 CurrentHealth writeback");
    expect_hex("wounded.attrs_after", result.attributes_after,
               DM1_V1_CPDA_ATTR_ACTION_HAND_PC34 |
               DM1_V1_CPDA_ATTR_STATISTICS_PC34 |
               DM1_V1_CPDA_ATTR_WOUNDS_PC34,
               "F0320:1737 STATISTICS + F0320:1739 WOUNDS");
}

static void test_dead_skip_and_lethal(void)
{
    DM1_V1_ChampionPanelPendingDamageApplyInputPc34Compat input = base_input();
    DM1_V1_ChampionPanelPendingDamageApplyResultPc34Compat result;

    input.champion_index = 3;
    input.pending_damage = 4;
    input.pending_wounds = 0x0040;
    input.current_health = 0;
    input.was_alive_before = false;
    input.attributes_before = DM1_V1_CPDA_ATTR_WOUNDS_PC34;
    expect_int("dead.build",
               DM1_V1_ChampionPanelPendingDamageApply_BuildPc34Compat(&input,
                                                                      &result),
               1, "CHAMPION.C F0320:1728-1729 dead champion skip");
    expect_common("dead", &result, 3, 4, 0x0040, 0,
                  DM1_V1_CPDA_ATTR_WOUNDS_PC34);
    expect_int("dead.outcome", result.outcome,
               DM1_V1_CPDA_OUTCOME_SKIPPED_DEAD_PC34,
               "F0320 CurrentHealth == 0 continue");
    expect_hex("dead.attrs_after", result.attributes_after,
               DM1_V1_CPDA_ATTR_WOUNDS_PC34,
               "dead skip does not add STATISTICS or STATUS_BOX");
    expect_bool("dead.f0319", result.f0319_kill_called, false,
                "F0319 not called for already-dead skip");

    input = base_input();
    input.champion_index = 0;
    input.pending_damage = 40;
    input.pending_wounds = 0x0008;
    input.current_health = 40;
    input.attributes_before =
        DM1_V1_CPDA_ATTR_PANEL_PC34 | DM1_V1_CPDA_ATTR_STATISTICS_PC34;
    expect_int("lethal.equal.build",
               DM1_V1_ChampionPanelPendingDamageApply_BuildPc34Compat(&input,
                                                                      &result),
               1, "CHAMPION.C F0320:1731 damage == health lethal");
    expect_common("lethal.equal", &result, 0, 40, 0x0008, 40,
                  DM1_V1_CPDA_ATTR_PANEL_PC34 |
                  DM1_V1_CPDA_ATTR_STATISTICS_PC34);
    expect_int("lethal.equal.outcome", result.outcome,
               DM1_V1_CPDA_OUTCOME_KILLED_BY_F0319_PC34,
               "F0320:1733-1734 F0319 dispatch");
    expect_int("lethal.equal.damage_applied", result.pending_damage_applied,
               40, "F0320 lethal pending damage consumed");
    expect_int("lethal.equal.health_after", result.current_health_after, 0,
               "F0319:1569 CurrentHealth = 0");
    expect_bool("lethal.equal.alive_after", result.alive_after, false,
                "F0319 kill branch");
    expect_bool("lethal.equal.f0319", result.f0319_kill_called, true,
                "F0320:1734 calls F0319_CHAMPION_Kill");
    expect_bool("lethal.equal.status_dispatch",
                result.status_box_redraw_dispatched, true,
                "F0319:1574 STATUS_BOX redraw");
    expect_hex("lethal.equal.attrs_after", result.attributes_after,
               DM1_V1_CPDA_ATTR_PANEL_PC34 |
               DM1_V1_CPDA_ATTR_STATISTICS_PC34 |
               DM1_V1_CPDA_ATTR_STATUS_BOX_PC34,
               "F0319 M008_SET STATUS_BOX preserves pre-existing attrs");
    expect_bool("lethal.equal.no_mouse", result.mouse_screen_update_enabled,
                false, "F0320 lethal branch skips F0623 draw block");

    input.pending_damage = DM1_V1_CPDA_PENDING_DAMAGE_MAX_PC34;
    input.current_health = 1;
    input.attributes_before = 0;
    expect_int("lethal.max.build",
               DM1_V1_ChampionPanelPendingDamageApply_BuildPc34Compat(&input,
                                                                      &result),
               1, "max int16 pending damage lethal branch");
    expect_int("lethal.max.outcome", result.outcome,
               DM1_V1_CPDA_OUTCOME_KILLED_BY_F0319_PC34,
               "F0320:1731-1734 high damage lethal");
    expect_hex("lethal.max.attrs_after", result.attributes_after,
               DM1_V1_CPDA_ATTR_STATUS_BOX_PC34,
               "F0319 STATUS_BOX only when no pre-existing attrs");
}

static void test_invalid_inputs(void)
{
    DM1_V1_ChampionPanelPendingDamageApplyInputPc34Compat input = base_input();
    DM1_V1_ChampionPanelPendingDamageApplyResultPc34Compat result;

    expect_int("invalid.null_out",
               DM1_V1_ChampionPanelPendingDamageApply_BuildPc34Compat(NULL,
                                                                      NULL),
               0, "NULL out guard");

    input.champion_index = -1;
    expect_int("invalid.negative_index.build",
               DM1_V1_ChampionPanelPendingDamageApply_BuildPc34Compat(&input,
                                                                      &result),
               0, "F0320 champion index lower bound");
    expect_bool("invalid.negative_index.flag", result.rejected_index, true,
                "rejected_index");
    expect_int("invalid.negative_index.outcome", result.outcome,
               DM1_V1_CPDA_OUTCOME_REJECTED_INDEX_PC34,
               "negative index rejected");
    expect_hex("invalid.negative_index.attrs_after", result.attributes_after,
               input.attributes_before, "reject path preserves attrs");

    input = base_input();
    input.champion_index = DM1_V1_CPDA_CHAMPION_COUNT_PC34;
    expect_int("invalid.high_index.build",
               DM1_V1_ChampionPanelPendingDamageApply_BuildPc34Compat(&input,
                                                                      &result),
               0, "F0320 champion index upper bound");
    expect_bool("invalid.high_index.flag", result.rejected_index, true,
                "rejected_index");

    input = base_input();
    input.pending_damage = -1;
    expect_int("invalid.negative_damage.build",
               DM1_V1_ChampionPanelPendingDamageApply_BuildPc34Compat(&input,
                                                                      &result),
               0, "synthetic pending damage lower bound");
    expect_bool("invalid.negative_damage.flag",
                result.rejected_pending_damage, true,
                "rejected_pending_damage");
    expect_int("invalid.negative_damage.outcome", result.outcome,
               DM1_V1_CPDA_OUTCOME_REJECTED_PENDING_DAMAGE_PC34,
               "negative pending damage rejected");

    input.pending_damage = DM1_V1_CPDA_PENDING_DAMAGE_MAX_PC34 + 1;
    expect_int("invalid.high_damage.build",
               DM1_V1_ChampionPanelPendingDamageApply_BuildPc34Compat(&input,
                                                                      &result),
               0, "synthetic pending damage int16 cap");
    expect_bool("invalid.high_damage.flag", result.rejected_pending_damage,
                true, "rejected_pending_damage");
}

int main(void)
{
    test_evidence();
    test_constants();
    test_default_and_zero_pending();
    test_nonlethal_without_wounds();
    test_nonlethal_with_wounds();
    test_dead_skip_and_lethal();
    test_invalid_inputs();

    printf("dm1_v1_champion_panel_pending_damage_apply_pc34_compat: "
           "assertions=%d failures=%d\n",
           g_assertions, g_failures);
    return g_failures ? 1 : 0;
}
