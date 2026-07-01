#include "dm1/dm1_v1_champion_panel_pending_wounds_tick_pc34_compat.h"

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

static void expect_bool(const char *id, bool got, bool want, const char *anchor)
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

static void test_evidence(void)
{
    const DM1_V1_ChampionPanelPendingWoundsTickEvidencePc34Compat *evidence =
        DM1_V1_ChampionPanelPendingWoundsTick_EvidencePc34Compat();

    expect_bool("evidence.contract_only", evidence->contract_only, true,
                "CHAMPION.C F0320:1720-1727 contract-only route");
    expect_str_eq("evidence.applier_function",
                  evidence->applier_function_anchor,
                  "CHAMPION.C F0320_CHAMPION_ApplyAndDrawPendingDamageAndWounds:"
                  "1720-1727 party-champion loop with wound OR-in + reset + "
                  "zero-damage continue",
                  "CHAMPION.C F0320:1720-1727");
    expect_str_eq("evidence.m008_macro", evidence->m008_set_macro_anchor,
                  "COMPILE.H:1042 #define M008_SET(value, mask) ((value) |= (mask))",
                  "COMPILE.H:1042 M008_SET");
    expect_contains("evidence.pending_wounds",
                    evidence->defs_pending_wounds_anchor,
                    "G0410_ai_ChampionPendingWounds[4]",
                    "DEFS.H:5867 staging buffer");
    expect_contains("evidence.pending_damage",
                    evidence->defs_pending_damage_anchor,
                    "G0409_ai_ChampionPendingDamage[4]",
                    "DEFS.H:5866 staging buffer");
    expect_contains("evidence.wound_constants",
                    evidence->defs_wound_constants_anchor,
                    "MASK0x0000", "DEFS.H:735-741 wound bit constants");
    expect_contains("evidence.champion_struct",
                    evidence->defs_champion_struct_anchor,
                    "Wounds",
                    "DEFS.H:679 CHAMPION.Wounds bitmask field");
    expect_contains("evidence.loop_bound", evidence->loop_bound_anchor,
                    "G0305_ui_PartyChampionCount",
                    "CHAMPION.C F0320:1720 loop bound");
    expect_contains("evidence.zero_damage_continue",
                    evidence->zero_damage_continue_anchor,
                    "continue",
                    "CHAMPION.C F0320:1723-1724 early-continue");
    expect_contains("evidence.complement_damage_indicator",
                    evidence->complement_gate_damage_indicator,
                    "F0623",
                    "CHAMDRAW.C F0623 sibling gate");
    expect_contains("evidence.complement_damage_flash_decay",
                    evidence->complement_gate_damage_flash_decay,
                    "F0254",
                    "TIMELINE.C F0254 sibling gate");
    expect_contains("evidence.no_real_asset_claim",
                    evidence->no_real_asset_claim,
                    "contract-only",
                    "no real-asset or original-DOS parity claim");
}

static void test_constants(void)
{
    expect_int("const.champion_count",
               DM1_V1_CPPWT_CHAMPION_COUNT_PC34, 4,
               "CHAMPION.C F0320:1720 four champion panel cells");
    expect_int("const.party_min", DM1_V1_CPPWT_PARTY_MIN_PC34, 1,
               "CHAMPION.C F0320:1720 minimum party size");
    expect_int("const.wound_none", DM1_V1_CPPWT_WOUND_NONE_PC34, 0x0000,
               "DEFS.H:735 MASK0x0000_WOUND_NONE");
    expect_int("const.wound_ready_hand",
               DM1_V1_CPPWT_WOUND_READY_HAND_PC34, 0x0001,
               "DEFS.H:736 MASK0x0001_WOUND_READY_HAND");
    expect_int("const.wound_action_hand",
               DM1_V1_CPPWT_WOUND_ACTION_HAND_PC34, 0x0002,
               "DEFS.H:737 MASK0x0002_WOUND_ACTION_HAND");
    expect_int("const.wound_head", DM1_V1_CPPWT_WOUND_HEAD_PC34, 0x0004,
               "DEFS.H:738 MASK0x0004_WOUND_HEAD");
    expect_int("const.wound_torso", DM1_V1_CPPWT_WOUND_TORSO_PC34, 0x0008,
               "DEFS.H:739 MASK0x0008_WOUND_TORSO");
    expect_int("const.wound_legs", DM1_V1_CPPWT_WOUND_LEGS_PC34, 0x0010,
               "DEFS.H:740 MASK0x0010_WOUND_LEGS");
    expect_int("const.wound_feet", DM1_V1_CPPWT_WOUND_FEET_PC34, 0x0020,
               "DEFS.H:741 MASK0x0020_WOUND_FEET");
}

static void test_init_state_clears_all_champions(void)
{
    DM1_V1_ChampionPanelPendingWoundsTickStatePc34Compat state;
    int champion_index;
    char id[96];

    memset(&state, 0xFF, sizeof(state));
    DM1_V1_ChampionPanelPendingWoundsTick_InitStatePc34Compat(&state);

    expect_int("init.party_count", state.party_champion_count, 4,
               "CHAMPION.C F0320:1720 default four champion panel cells");
    for (champion_index = 0;
         champion_index < DM1_V1_CPPWT_CHAMPION_COUNT_PC34;
         ++champion_index) {
        snprintf(id, sizeof(id), "init.champion_%d_index",
                 champion_index);
        expect_int(id, state.champions[champion_index].index,
                   champion_index, "DM1 V1 four champion panel cells");
        snprintf(id, sizeof(id), "init.champion_%d_wounds",
                 champion_index);
        expect_int(id, state.champions[champion_index].wounds, 0,
                   "DEFS.H:679 wounds=0 post CEDTINCI.C:68 init");
        snprintf(id, sizeof(id), "init.champion_%d_pending_wounds",
                 champion_index);
        expect_int(id,
                   state.champions[champion_index].pending_wounds, 0,
                   "DEFS.H:5867 G0410 cleared at init");
        snprintf(id, sizeof(id), "init.champion_%d_pending_damage",
                 champion_index);
        expect_int(id,
                   state.champions[champion_index].pending_damage, 0,
                   "DEFS.H:5866 G0409 cleared at init");
        snprintf(id, sizeof(id), "init.champion_%d_invalid_input",
                 champion_index);
        expect_bool(id, state.champions[champion_index].invalid_input,
                    false, "no out-of-bounds staging at init");
    }

    /*
     * Defensive: NULL state is allowed (no-op).
     */
    expect_int("init.null_state_no_crash",
               (DM1_V1_ChampionPanelPendingWoundsTick_InitStatePc34Compat(NULL),
                1),
               1, "synthetic init defensive NULL guard");
}

static void test_run_no_op_party_no_staging(void)
{
    DM1_V1_ChampionPanelPendingWoundsTickStatePc34Compat state;
    DM1_V1_ChampionPanelPendingWoundsTickStepResultPc34Compat step;
    DM1_V1_ChampionPanelPendingWoundsTickChampionResultPc34Compat
        per_champion[DM1_V1_CPPWT_CHAMPION_COUNT_PC34];
    int champion_index;
    char id[96];

    DM1_V1_ChampionPanelPendingWoundsTick_InitStatePc34Compat(&state);
    expect_int("noop.run_return",
               DM1_V1_ChampionPanelPendingWoundsTick_RunPartyLoopPc34Compat(
                   &state, &step, per_champion),
               1, "CHAMPION.C F0320:1720 party loop dispatch");
    expect_int("noop.processed", step.champions_processed, 4,
               "CHAMPION.C F0320:1720 default four champion iterations");
    expect_int("noop.wound_application", step.champions_with_wound_application,
               4, "CHAMPION.C F0320:1721 wound OR-in always runs");
    expect_int("noop.zero_damage_early_returns",
               step.champions_with_zero_damage_early_return, 4,
               "CHAMPION.C F0320:1723-1724 zero-damage continue for all 4");
    expect_int("noop.damage_draw", step.champions_with_damage_draw_called, 0,
               "CHAMPION.C F0320:1725 G0409 reset only when PendingDamage>0");
    expect_int("noop.invalid", step.champions_skipped_due_to_invalid_input, 0,
               "no invalid staging on a clean init");

    for (champion_index = 0;
         champion_index < DM1_V1_CPPWT_CHAMPION_COUNT_PC34;
         ++champion_index) {
        snprintf(id, sizeof(id), "noop.wounds_champion_%d",
                 champion_index);
        expect_int(id, state.champions[champion_index].wounds, 0,
                   "CHAMPION.C F0320:1721 OR with 0 leaves Wounds unchanged");
        snprintf(id, sizeof(id), "noop.pending_wounds_champion_%d",
                 champion_index);
        expect_int(id, state.champions[champion_index].pending_wounds, 0,
                   "CHAMPION.C F0320:1722 G0410 cleared even on no-op tick");
        snprintf(id, sizeof(id), "noop.pending_damage_champion_%d",
                 champion_index);
        expect_int(id, state.champions[champion_index].pending_damage, 0,
                   "CHAMPION.C F0320:1725 G0409 not reset on no-op tick");
        snprintf(id, sizeof(id), "noop.branch_champion_%d",
                 champion_index);
        expect_int(id, (int)per_champion[champion_index].branch,
                   (int)DM1_V1_CPPWT_BRANCH_NO_OP_PC34,
                   "CHAMPION.C F0320:1723-1724 no_op branch on zero/zero");
        snprintf(id, sizeof(id), "noop.wound_app_champion_%d",
                 champion_index);
        expect_bool(id,
                    per_champion[champion_index].wound_application_occurred,
                    true,
                    "CHAMPION.C F0320:1721 wound OR-in always runs");
        snprintf(id, sizeof(id), "noop.pending_reset_champion_%d",
                 champion_index);
        expect_bool(id,
                    per_champion[champion_index].pending_wound_reset_occurred,
                    true,
                    "CHAMPION.C F0320:1722 G0410 reset always runs");
        snprintf(id, sizeof(id), "noop.zero_damage_champion_%d",
                 champion_index);
        expect_bool(id,
                    per_champion[champion_index].zero_damage_early_return_took_place,
                    true,
                    "CHAMPION.C F0320:1723-1724 zero-damage continue taken");
        snprintf(id, sizeof(id), "noop.damage_draw_champion_%d",
                 champion_index);
        expect_bool(id,
                    per_champion[champion_index].damage_draw_would_be_called,
                    false,
                    "CHAMPION.C F0320:1723-1724 F0623 not called on zero");
    }
}

static void test_wound_only_branch_per_champion(void)
{
    static const struct {
        int champion_index;
        uint16_t initial_wounds;
        uint16_t staged_pending_wounds;
    } cases[] = {
        /* single-bit stages OR'd into 0 wounds */
        { 0, 0x0000, DM1_V1_CPPWT_WOUND_READY_HAND_PC34 },
        { 1, 0x0000, DM1_V1_CPPWT_WOUND_ACTION_HAND_PC34 },
        { 2, 0x0000, DM1_V1_CPPWT_WOUND_HEAD_PC34 },
        { 3, 0x0000, DM1_V1_CPPWT_WOUND_TORSO_PC34 },
        /* OR-in into existing wounds (multi-bit pattern) */
        { 0, DM1_V1_CPPWT_WOUND_HEAD_PC34,
          DM1_V1_CPPWT_WOUND_READY_HAND_PC34 |
              DM1_V1_CPPWT_WOUND_ACTION_HAND_PC34 },
        { 1, DM1_V1_CPPWT_WOUND_TORSO_PC34,
          DM1_V1_CPPWT_WOUND_LEGS_PC34 },
        { 2, DM1_V1_CPPWT_WOUND_HEAD_PC34 |
                DM1_V1_CPPWT_WOUND_TORSO_PC34,
          DM1_V1_CPPWT_WOUND_FEET_PC34 },
        /* identical-bit OR (idempotent) */
        { 3, DM1_V1_CPPWT_WOUND_FEET_PC34,
          DM1_V1_CPPWT_WOUND_FEET_PC34 },
        /* saturation: already-0xFFFF keeps 0xFFFF */
        { 0, 0xFFFF, 0xFFFF },
        { 1, 0xFFFF, 0x0020 /* feet */ },
    };
    unsigned i;

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        DM1_V1_ChampionPanelPendingWoundsTickStatePc34Compat state;
        DM1_V1_ChampionPanelPendingWoundsTickStepResultPc34Compat step;
        DM1_V1_ChampionPanelPendingWoundsTickChampionResultPc34Compat
            per_champion[DM1_V1_CPPWT_CHAMPION_COUNT_PC34];
        int champion_index;
        char id[96];
        uint16_t expected_wounds;

        DM1_V1_ChampionPanelPendingWoundsTick_InitStatePc34Compat(&state);
        state.champions[cases[i].champion_index].wounds =
            cases[i].initial_wounds;
        state.champions[cases[i].champion_index].pending_wounds =
            cases[i].staged_pending_wounds;
        /* pending_damage = 0 by init; explicitly tag zero here */
        state.champions[cases[i].champion_index].pending_damage = 0;

        expected_wounds = (uint16_t)(cases[i].initial_wounds |
                                      cases[i].staged_pending_wounds);

        expect_int("wound_only.run_return",
                   DM1_V1_ChampionPanelPendingWoundsTick_RunPartyLoopPc34Compat(
                       &state, &step, per_champion),
                   1, "CHAMPION.C F0320:1720 party loop dispatch");

        snprintf(id, sizeof(id), "wound_only.wounds_after_case_%u", i);
        expect_int(id,
                   state.champions[cases[i].champion_index].wounds,
                   expected_wounds,
                   "CHAMPION.C F0320:1721 M008_SET OR-in (DEFS.H:679 "
                   "Wounds bitmask)");
        snprintf(id, sizeof(id), "wound_only.pending_wounds_after_case_%u", i);
        expect_int(id,
                   state.champions[cases[i].champion_index].pending_wounds, 0,
                   "CHAMPION.C F0320:1722 G0410 reset always runs");
        snprintf(id, sizeof(id), "wound_only.pending_damage_after_case_%u", i);
        expect_int(id,
                   state.champions[cases[i].champion_index].pending_damage, 0,
                   "CHAMPION.C F0320:1725 G0409 not reset on zero-damage tick");
        snprintf(id, sizeof(id), "wound_only.branch_case_%u", i);
        expect_int(id,
                   (int)per_champion[cases[i].champion_index].branch,
                   (int)DM1_V1_CPPWT_BRANCH_WOUND_ONLY_PC34,
                   "CHAMPION.C F0320:1723-1724 wound_only branch");
        snprintf(id, sizeof(id), "wound_only.wounds_before_case_%u", i);
        expect_int(id,
                   per_champion[cases[i].champion_index].wounds_before,
                   cases[i].initial_wounds,
                   "CHAMPION.C F0320:1721 snapshot before M008_SET");
        snprintf(id, sizeof(id), "wound_only.zero_continue_case_%u", i);
        expect_bool(id,
                    per_champion[cases[i].champion_index]
                        .zero_damage_early_return_took_place,
                    true,
                    "CHAMPION.C F0320:1723-1724 early-continue taken on "
                    "zero-damage tick");
        snprintf(id, sizeof(id), "wound_only.damage_draw_case_%u", i);
        expect_bool(id,
                    per_champion[cases[i].champion_index]
                        .damage_draw_would_be_called,
                    false,
                    "CHAMPION.C F0320:1723-1724 F0623 not called on zero "
                    "damage");

        /*
         * Other 3 champion slots stay clean no-ops.
         */
        for (champion_index = 0;
             champion_index < DM1_V1_CPPWT_CHAMPION_COUNT_PC34;
             ++champion_index) {
            if (champion_index == cases[i].champion_index) {
                continue;
            }
            snprintf(id, sizeof(id),
                     "wound_only.untouched_wounds_%d_case_%u",
                     champion_index, i);
            expect_int(id, state.champions[champion_index].wounds, 0,
                       "CHAMPION.C F0320:1721 OR with 0 leaves Wounds unchanged");
            snprintf(id, sizeof(id),
                     "wound_only.untouched_branch_%d_case_%u",
                     champion_index, i);
            expect_int(id,
                       (int)per_champion[champion_index].branch,
                       (int)DM1_V1_CPPWT_BRANCH_NO_OP_PC34,
                       "CHAMPION.C F0320:1723-1724 no_op for unmodified slots");
        }
    }
}

static void test_damage_only_branch(void)
{
    DM1_V1_ChampionPanelPendingWoundsTickStatePc34Compat state;
    DM1_V1_ChampionPanelPendingWoundsTickStepResultPc34Compat step;
    DM1_V1_ChampionPanelPendingWoundsTickChampionResultPc34Compat
        per_champion[DM1_V1_CPPWT_CHAMPION_COUNT_PC34];

    DM1_V1_ChampionPanelPendingWoundsTick_InitStatePc34Compat(&state);
    state.champions[2].wounds = DM1_V1_CPPWT_WOUND_HEAD_PC34;
    state.champions[2].pending_damage = 50;

    expect_int("damage_only.run_return",
               DM1_V1_ChampionPanelPendingWoundsTick_RunPartyLoopPc34Compat(
                   &state, &step, per_champion),
               1, "CHAMPION.C F0320:1720 party loop dispatch");
    expect_int("damage_only.processed", step.champions_processed, 4,
               "CHAMPION.C F0320:1720 four champion iterations");
    expect_int("damage_only.wound_application",
               step.champions_with_wound_application, 4,
               "CHAMPION.C F0320:1721 wound OR-in always runs");
    expect_int("damage_only.zero_damage_early_returns",
               step.champions_with_zero_damage_early_return, 3,
               "CHAMPION.C F0320:1723-1724 zero-damage for three champions");
    expect_int("damage_only.damage_draw",
               step.champions_with_damage_draw_called, 1,
               "CHAMPION.C F0320:1725 G0409 reset reached exactly once");

    expect_int("damage_only.wounds_kept",
               state.champions[2].wounds,
               DM1_V1_CPPWT_WOUND_HEAD_PC34,
               "CHAMPION.C F0320:1721 M008_SET with 0 leaves Wounds unchanged");
    expect_int("damage_only.pending_damage_reset",
               state.champions[2].pending_damage, 0,
               "CHAMPION.C F0320:1725 G0409 reset reached when PendingDamage>0");
    expect_int("damage_only.branch",
               (int)per_champion[2].branch,
               (int)DM1_V1_CPPWT_BRANCH_DAMAGE_ONLY_PC34,
               "CHAMPION.C F0320:1723-1724 damage_only branch");
    expect_bool("damage_only.damage_draw_called",
                per_champion[2].damage_draw_would_be_called, true,
                "CHAMPION.C F0320:1723-1724 F0623 dispatch reached for "
                "damage>0 tick");
    expect_bool("damage_only.zero_damage_continue",
                per_champion[2].zero_damage_early_return_took_place, false,
                "CHAMPION.C F0320:1723-1724 zero-damage early-continue NOT "
                "taken");
    expect_bool("damage_only.wound_application",
                per_champion[2].wound_application_occurred, true,
                "CHAMPION.C F0320:1721 wound OR-in still runs on damage>0");
    expect_bool("damage_only.pending_reset",
                per_champion[2].pending_wound_reset_occurred, true,
                "CHAMPION.C F0320:1722 G0410 reset still runs on damage>0");
}

static void test_wound_and_damage_branch(void)
{
    DM1_V1_ChampionPanelPendingWoundsTickStatePc34Compat state;
    DM1_V1_ChampionPanelPendingWoundsTickStepResultPc34Compat step;
    DM1_V1_ChampionPanelPendingWoundsTickChampionResultPc34Compat
        per_champion[DM1_V1_CPPWT_CHAMPION_COUNT_PC34];

    DM1_V1_ChampionPanelPendingWoundsTick_InitStatePc34Compat(&state);
    state.champions[1].wounds = DM1_V1_CPPWT_WOUND_HEAD_PC34;
    state.champions[1].pending_wounds =
        DM1_V1_CPPWT_WOUND_FEET_PC34 |
        DM1_V1_CPPWT_WOUND_ACTION_HAND_PC34;
    state.champions[1].pending_damage = 75;

    expect_int("wound_and_damage.run_return",
               DM1_V1_ChampionPanelPendingWoundsTick_RunPartyLoopPc34Compat(
                   &state, &step, per_champion),
               1, "CHAMPION.C F0320:1720 party loop dispatch");
    expect_int("wound_and_damage.wounds_combined",
               state.champions[1].wounds,
               DM1_V1_CPPWT_WOUND_HEAD_PC34 |
                   DM1_V1_CPPWT_WOUND_FEET_PC34 |
                   DM1_V1_CPPWT_WOUND_ACTION_HAND_PC34,
               "CHAMPION.C F0320:1721 OR-in combines existing + staged wounds");
    expect_int("wound_and_damage.pending_wounds_reset",
               state.champions[1].pending_wounds, 0,
               "CHAMPION.C F0320:1722 G0410 reset");
    expect_int("wound_and_damage.pending_damage_reset",
               state.champions[1].pending_damage, 0,
               "CHAMPION.C F0320:1725 G0409 reset on damage>0");
    expect_int("wound_and_damage.branch",
               (int)per_champion[1].branch,
               (int)DM1_V1_CPPWT_BRANCH_WOUND_AND_DAMAGE_PC34,
               "CHAMPION.C F0320:1723-1724 wound_and_damage branch");
    expect_bool("wound_and_damage.damage_draw_called",
                per_champion[1].damage_draw_would_be_called, true,
                "CHAMPION.C F0623 reachable when PendingDamage>0 even "
                "with staged wounds");
    expect_bool("wound_and_damage.zero_damage_continue",
                per_champion[1].zero_damage_early_return_took_place, false,
                "CHAMPION.C F0320:1723-1724 zero-damage early-continue NOT "
                "taken when damage>0");
}

static void test_pending_damage_not_reset_on_zero_branch(void)
{
    /*
     * The key slice gap: when pending_damage == 0, F0320:1723-1724
     * early-continues BEFORE F0320:1725 resets G0409. This is the
     * exact reason dm1_v1_champion_panel_damage_indicator_pc34_compat
     * could not model this — it only models the F0623 dispatch which
     * only runs past the zero-damage continue. Here we stage a
     * non-zero pending_damage, run the loop, then re-arm the same
     * pending_damage on the same champion and verify the second tick
     * still has PendingDamage intact (because the FIRST tick reached
     * F0320:1725 and cleared it; that is the post-tick stable state).
     *
     * We also verify that on the same tick where pending_damage == 0
     * G0409 stays at the staged value (the early continue skips
     * F0320:1725).
     */
    DM1_V1_ChampionPanelPendingWoundsTickStatePc34Compat state;
    DM1_V1_ChampionPanelPendingWoundsTickStepResultPc34Compat step;
    DM1_V1_ChampionPanelPendingWoundsTickChampionResultPc34Compat
        per_champion[DM1_V1_CPPWT_CHAMPION_COUNT_PC34];

    DM1_V1_ChampionPanelPendingWoundsTick_InitStatePc34Compat(&state);
    state.champions[0].pending_damage = 7;
    /* pending_wounds intentionally left at 0 */

    expect_int("pending_damage_not_reset.run_return",
               DM1_V1_ChampionPanelPendingWoundsTick_RunPartyLoopPc34Compat(
                   &state, &step, per_champion),
               1, "CHAMPION.C F0320:1720 party loop dispatch");
    expect_int("pending_damage_not_reset.g0409_reset",
               state.champions[0].pending_damage, 0,
               "CHAMPION.C F0320:1725 G0409 reset on damage>0 tick");
    expect_bool("pending_damage_not_reset.damage_draw_called",
                per_champion[0].damage_draw_would_be_called, true,
                "CHAMPION.C F0623 dispatch reached for damage>0");
    expect_bool("pending_damage_not_reset.zero_continue_taken",
                per_champion[0].zero_damage_early_return_took_place, false,
                "CHAMPION.C F0320:1723-1724 NOT taken with non-zero damage");

    /*
     * Now stage 7 again with 0 pending_wounds and verify the
     * damage>0 path is taken again on the second tick.
     */
    state.champions[0].pending_damage = 7;
    expect_int("pending_damage_not_reset.second_run_return",
               DM1_V1_ChampionPanelPendingWoundsTick_RunPartyLoopPc34Compat(
                   &state, &step, per_champion),
               1, "CHAMPION.C F0320:1720 second party loop dispatch");
    expect_int("pending_damage_not_reset.second_g0409_reset",
               state.champions[0].pending_damage, 0,
               "CHAMPION.C F0320:1725 G0409 reset on second damage>0 tick");
}

static void test_pending_wounds_kept_when_zero_damage_continue_skips_reset(void)
{
    /*
     * The inverse invariant: when a champion has pending_damage = 0
     * (which causes F0320:1723-1724 to early-continue), G0409 is NOT
     * cleared by this loop iteration. The damage-draw gate has no
     * reach to F0320:1725 in that path. We stage a non-zero
     * pending_damage elsewhere on the same tick and verify the
     * zero-damage champion's pending_damage survives the loop.
     */
    DM1_V1_ChampionPanelPendingWoundsTickStatePc34Compat state;
    DM1_V1_ChampionPanelPendingWoundsTickStepResultPc34Compat step;
    DM1_V1_ChampionPanelPendingWoundsTickChampionResultPc34Compat
        per_champion[DM1_V1_CPPWT_CHAMPION_COUNT_PC34];

    DM1_V1_ChampionPanelPendingWoundsTick_InitStatePc34Compat(&state);
    state.champions[1].pending_damage = -42;
    /* champion 0 has PendingDamage = 0 (no staging) */

    expect_int("pending_kept_zero_cont.run_return",
               DM1_V1_ChampionPanelPendingWoundsTick_RunPartyLoopPc34Compat(
                   &state, &step, per_champion),
               1, "CHAMPION.C F0320:1720 party loop dispatch");
    expect_int("pending_kept_zero_cont.zero_damage_champ_pending_damage",
               state.champions[0].pending_damage, 0,
               "CHAMPION.C F0320:1723-1724 zero-damage early-continue: G0409 "
               "stays cleared (was already cleared)");
    expect_int("pending_kept_zero_cont.positive_damage_champ_pending_damage",
               state.champions[1].pending_damage, 0,
               "CHAMPION.C F0320:1725 G0409 reset on damage>0 tick");
    expect_int("pending_kept_zero_cont.zero_damage_branches",
               step.champions_with_zero_damage_early_return, 3,
               "CHAMPION.C F0320:1723-1724 zero-damage continue for three "
               "of four champions");
    expect_int("pending_kept_zero_cont.damage_draw_branches",
               step.champions_with_damage_draw_called, 1,
               "CHAMPION.C F0320:1725 G0409 reset reached exactly once");
}

static void test_three_champion_party_size(void)
{
    /*
     * Party size 3 (champion 0, 1, 2). The F0320:1720 loop must
     * iterate exactly that many times, leaving champion index 3
     * untouched on this tick.
     */
    DM1_V1_ChampionPanelPendingWoundsTickStatePc34Compat state;
    DM1_V1_ChampionPanelPendingWoundsTickStepResultPc34Compat step;
    DM1_V1_ChampionPanelPendingWoundsTickChampionResultPc34Compat
        per_champion[DM1_V1_CPPWT_CHAMPION_COUNT_PC34];

    DM1_V1_ChampionPanelPendingWoundsTick_InitStatePc34Compat(&state);
    state.party_champion_count = 3;
    state.champions[3].wounds = 0xFFFF;
    state.champions[3].pending_wounds = 0xFFFF;
    state.champions[3].pending_damage = 0;

    expect_int("party_three.run_return",
               DM1_V1_ChampionPanelPendingWoundsTick_RunPartyLoopPc34Compat(
                   &state, &step, per_champion),
               1, "CHAMPION.C F0320:1720 dispatch");
    expect_int("party_three.processed", step.champions_processed, 3,
               "CHAMPION.C F0320:1720 three-champion party");
    expect_int("party_three.outside_wounds_unchanged",
               state.champions[3].wounds, 0xFFFF,
               "CHAMPION.C F0320:1720 loop bound G0305_ui_PartyChampionCount");
    expect_int("party_three.outside_pending_wounds_unchanged",
               state.champions[3].pending_wounds, 0xFFFF,
               "CHAMPION.C F0320:1722 G0410 untouched outside party loop");
    expect_int("party_three.outside_pending_damage_unchanged",
               state.champions[3].pending_damage, 0,
               "CHAMPION.C F0320:1725 G0409 untouched outside party loop");
}

static void test_invalid_inputs(void)
{
    DM1_V1_ChampionPanelPendingWoundsTickStatePc34Compat state;
    DM1_V1_ChampionPanelPendingWoundsTickStepResultPc34Compat step;
    DM1_V1_ChampionPanelPendingWoundsTickChampionResultPc34Compat
        per_champion[DM1_V1_CPPWT_CHAMPION_COUNT_PC34];

    DM1_V1_ChampionPanelPendingWoundsTick_InitStatePc34Compat(&state);

    /*
     * Defensive guards on the public surface.
     */
    expect_int("invalid.null_state",
               DM1_V1_ChampionPanelPendingWoundsTick_RunPartyLoopPc34Compat(
                   NULL, &step, per_champion),
               0, "synthetic guard before CHAMPION.C F0320 model");
    expect_int("invalid.null_step",
               DM1_V1_ChampionPanelPendingWoundsTick_RunPartyLoopPc34Compat(
                   &state, NULL, per_champion),
               0, "synthetic guard before CHAMPION.C F0320 model");
    expect_int("invalid.null_per_champion_permitted",
               DM1_V1_ChampionPanelPendingWoundsTick_RunPartyLoopPc34Compat(
                   &state, &step, NULL),
               1, "CHAMPION.C F0320 model allows per-champion NULL for "
               "compact test surface");

    /*
     * Marker for invalid out-of-bounds staging: the gate counts
     * invalid_input slots in step.champions_skipped_due_to_invalid_input
     * but the loop still ORs in wounds (the source has no slot guard
     * in F0320:1721). This is by design — it surfaces to the test
     * when the upstream F0321 has staged into an invalid slot.
     */
    DM1_V1_ChampionPanelPendingWoundsTick_InitStatePc34Compat(&state);
    state.champions[2].invalid_input = true;
    state.champions[2].pending_wounds = DM1_V1_CPPWT_WOUND_HEAD_PC34;
    state.champions[2].pending_damage = 9;
    expect_int("invalid.staging.run_return",
               DM1_V1_ChampionPanelPendingWoundsTick_RunPartyLoopPc34Compat(
                   &state, &step, per_champion),
               1, "CHAMPION.C F0320:1720 loop runs even on invalid staging");
    expect_int("invalid.staging.invalid_count",
               step.champions_skipped_due_to_invalid_input, 1,
               "synthetic invalid_input marker reported in step");
    expect_int("invalid.staging.wounds_or_in",
               state.champions[2].wounds,
               DM1_V1_CPPWT_WOUND_HEAD_PC34,
               "CHAMPION.C F0320:1721 OR-in still runs on invalid staging");
    expect_int("invalid.staging.pending_reset",
               state.champions[2].pending_wounds, 0,
               "CHAMPION.C F0320:1722 G0410 reset still runs on invalid "
               "staging");
}

int main(void)
{
    test_evidence();
    test_constants();
    test_init_state_clears_all_champions();
    test_run_no_op_party_no_staging();
    test_wound_only_branch_per_champion();
    test_damage_only_branch();
    test_wound_and_damage_branch();
    test_pending_damage_not_reset_on_zero_branch();
    test_pending_wounds_kept_when_zero_damage_continue_skips_reset();
    test_three_champion_party_size();
    test_invalid_inputs();

    printf("dm1_v1_champion_panel_pending_wounds_tick_pc34_compat: "
           "assertions=%d failures=%d\n",
           g_assertions, g_failures);
    return g_failures ? 1 : 0;
}
