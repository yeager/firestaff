/*
 * DM1 V1 champion-panel DISABLED/UNAVAILABLE ICON STATE gate test.
 *
 * Source-locked to:
 *   - ReDMCSB CHAMPION.C F0330_CHAMPION_DisableAction:2208-2255
 *   - ReDMCSB ACTIDRAW.C F0386_MENUS_DrawActionIcon:201-296
 *   - ReDMCSB MENU.C G0491_auc_Graphic560_ActionDisabledTicks[44]:27,157
 *   - M11 m11_collect_v1_status_shield_border_graphics
 *   - M11_GameView_ShouldHatchV1ActionIconCells (m11_game_view.c:17904)
 *
 * Companion to:
 *   - dm1_v1_graphic560_action_disabled_ticks_pc34_compat (table bytes)
 *   - M11_GameView_GetV1StatusShieldBorderGraphicForChampion (asset path)
 *   - M11_GameView_ShouldHatchV1ActionIconCells (global gate)
 *   - firestaff_dm1_v1_champion_panel_shield_border_pixel_probe
 *     (asset-backed ENABLED shield pixel probe)
 *   - firestaff_dm1_v1_champion_panel_icon_direction_swap_runtime_probe
 *     (asset-backed icon-direction pixel probe)
 *
 * Contract-only — does NOT load GRAPHICS.DAT, run live rendering,
 * capture pixels, run DOSBox, or claim original-vs-Firestaff parity.
 * Disjoint from pass784-790 (mirror-candidate C040), pass791 (ammo),
 * pass793 (action-hand slot-priority), pass794 (all-states redraw),
 * pass795-797 (leader/mirror/chest-action-hand), pass798-806
 * (graphics.dat init-table gates), pass922 (G0491 table bytes).
 *
 * Source-locked CTest registration: pass1074.
 */

#include "dm1_v1_champion_panel_disabled_icon_state_pc34_compat.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

static void expect_int(const char *id, int got, int want,
                       const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d at %s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d (%s)\n", id, want, anchor);
    }
}

static void expect_u16(const char *id, uint16_t got, uint16_t want,
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

static void test_evidence_and_invariants(void)
{
    const DM1_V1_ChampionPanelDisabledIconEvidencePc34Compat *evidence =
        DM1_V1_ChampionPanelDisabledIconState_EvidencePc34Compat();
    const char *source =
        DM1_V1_ChampionPanelDisabledIconState_SourceEvidencePc34Compat();

    expect_bool("invariant.contract_only", evidence->contract_only, true,
                "F0330:2208-2255 + F0386:201-296 contract-only");
    expect_contains("evidence.f0330_disable", evidence->f0330_disable_action_anchor,
                    "F0330_CHAMPION_DisableAction:2208-2255",
                    "CHAMPION.C F0330 anchor");
    expect_contains("evidence.f0330_attribute", evidence->f0330_attribute_set_anchor,
                    "MASK0x8000_ACTION_HAND | MASK0x0008_DISABLE_ACTION",
                    "CHAMPION.C F0330:2252 M008_SET anchor");
    expect_contains("evidence.f0330_event11", evidence->f0330_event11_schedule_anchor,
                    "C11_EVENT_ENABLE_CHAMPION_ACTION",
                    "CHAMPION.C F0330:2253-2255 C11 anchor");
    expect_contains("evidence.f0386_hatch", evidence->f0386_hatch_gate_anchor,
                    "MASK0x0008_DISABLE_ACTION",
                    "ACTIDRAW.C F0386:282 hatch gate anchor");
    expect_contains("evidence.f0386_dead", evidence->f0386_dead_champion_anchor,
                    "CurrentHealth == 0",
                    "ACTIDRAW.C F0386:234-238 dead-champion anchor");
    expect_contains("evidence.f0386_empty", evidence->f0386_empty_hand_anchor,
                    "C201_ICON_ACTION_ICON_EMPTY_HAND",
                    "ACTIDRAW.C F0386:262-264 empty-hand anchor");
    expect_contains("evidence.g0491", evidence->g0491_action_disabled_ticks_anchor,
                    "G0491_auc_Graphic560_ActionDisabledTicks[44]",
                    "MENU.C:27,157 anchor");
    expect_contains("evidence.shield_disabled",
                    evidence->shield_border_disabled_anchor,
                    "m11_collect_v1_status_shield_border_graphics",
                    "M11 shield-border disabled contract anchor");
    expect_contains("evidence.global_hatch", evidence->global_hatch_gate_anchor,
                    "M11_GameView_ShouldHatchV1ActionIconCells",
                    "M11 global hatch gate anchor");
    expect_str_eq("evidence.defs_action_hand",
                  evidence->defs_action_hand_anchor,
                  "DEFS.H MASK0x8000_ACTION_HAND = 0x8000",
                  "DEFS.H MASK0x8000 anchor");
    expect_str_eq("evidence.defs_disable_action",
                  evidence->defs_disable_action_anchor,
                  "DEFS.H MASK0x0008_DISABLE_ACTION = 0x0008",
                  "DEFS.H MASK0x0008 anchor");
    expect_bool("no.real_asset_claim", true,
                strstr(evidence->no_real_asset_claim, "no real GRAPHICS.DAT") != NULL,
                "no real-asset claim");
    expect_bool("no.dosbox_claim", true,
                strstr(evidence->no_dosbox_claim, "DOSBox") != NULL,
                "no DOSBox claim");
    expect_bool("no.pixel_parity_claim", true,
                strstr(evidence->no_pixel_parity_claim, "pixel parity") != NULL,
                "no pixel parity claim");
    expect_bool("non_overlap.shield_border", true,
                strstr(evidence->non_overlap_with_shield_border,
                       "shield_border_pixel_probe") != NULL,
                "disjoint from ENABLED shield_border_pixel_probe");
    expect_bool("non_overlap.icon_direction", true,
                strstr(evidence->non_overlap_with_icon_direction,
                       "icon_direction_swap") != NULL,
                "disjoint from ENABLED icon_direction_swap_runtime_probe");
    expect_bool("non_overlap.g0491_only", true,
                strstr(evidence->non_overlap_with_g0491_table_only,
                       "pass922") != NULL,
                "disjoint from pass922 G0491 table bytes");
    expect_contains("source.evidence.string", source,
                    "F0330:2208-2255",
                    "source evidence string anchor");
    expect_contains("source.evidence.g0491", source,
                    "G0491[44]",
                    "source evidence string G0491 anchor");
}

/*
 * ReDMCSB MENU.C:157 PC 3.4 EN init pinned byte-for-byte.  When
 * the action_index is out-of-range the contract returns -1 so
 * callers (CHAMPION.C F0330:2270 and the F0407_MENUS_IsAction
 * Performed action dispatch) can distinguish a missing row from
 * a zero-tick row.  G0491[0]=N, G0491[3]=X, G0491[42]=THROW are
 * the canonical zero-tick rows whose icons never disable.
 */
static void test_g0491_disabled_ticks_table(void)
{
    int i;
    /* Action indices that DO disable per G0491[44]. */
    int positive_indices[] = {1, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
                              14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
                              24, 25, 27, 28, 29, 30, 31, 32, 33, 34,
                              35, 36, 37, 38, 39, 40, 41, 43};
    int n_positive = (int)(sizeof(positive_indices) / sizeof(positive_indices[0]));
    /* Action indices that NEVER disable per G0491[44] = {0, 3, 42}. */
    int zero_indices[] = {0, 3, 42};
    int n_zero = (int)(sizeof(zero_indices) / sizeof(zero_indices[0]));

    for (i = 0; i < n_positive; ++i) {
        char id[64];
        snprintf(id, sizeof(id), "g0491.positive[%d]", positive_indices[i]);
        expect_int(id,
                   DM1_V1_ChampionPanelDisabledIconState_DisabledTicksPc34Compat(
                       positive_indices[i]) > 0 ? 1 : 0,
                   1,
                   "MENU.C:157 PC 3.4 EN init");
    }
    for (i = 0; i < n_zero; ++i) {
        char id[64];
        snprintf(id, sizeof(id), "g0491.zero[%d]", zero_indices[i]);
        expect_int(id,
                   DM1_V1_ChampionPanelDisabledIconState_DisabledTicksPc34Compat(
                       zero_indices[i]),
                   0,
                   "MENU.C:157 PC 3.4 EN init");
    }

    /* Specific anchor values from ReDMCSB MENU.C:157. */
    expect_int("g0491.BLOCK[1]",
               DM1_V1_ChampionPanelDisabledIconState_DisabledTicksPc34Compat(1),
               6, "MENU.C:157 BLOCK");
    expect_int("g0491.BERZERK[19]",
               DM1_V1_ChampionPanelDisabledIconState_DisabledTicksPc34Compat(19),
               30, "MENU.C:157 BERZERK");
    expect_int("g0491.SPELLSHIELD[33]",
               DM1_V1_ChampionPanelDisabledIconState_DisabledTicksPc34Compat(33),
               30, "MENU.C:157 SPELLSHIELD");
    expect_int("g0491.FIRESHIELD[34]",
               DM1_V1_ChampionPanelDisabledIconState_DisabledTicksPc34Compat(34),
               35, "MENU.C:157 FIRESHIELD");

    /* Out-of-range returns -1 (CHAMPION.C F0330:2270 missing-row guard). */
    expect_int("g0491.oob[-1]",
               DM1_V1_ChampionPanelDisabledIconState_DisabledTicksPc34Compat(-1),
               -1, "CHAMPION.C F0330:2270 OOB guard");
    expect_int("g0491.oob[44]",
               DM1_V1_ChampionPanelDisabledIconState_DisabledTicksPc34Compat(44),
               -1, "CHAMPION.C F0330:2270 OOB guard");
    expect_int("g0491.oob[999]",
               DM1_V1_ChampionPanelDisabledIconState_DisabledTicksPc34Compat(999),
               -1, "CHAMPION.C F0330:2270 OOB guard");
}

/*
 * ReDMCSB CHAMPION.C F0330:2252 — when ApplyDisableAction is called
 * with an action whose G0491 entry is 0 (N, X, THROW), the M008_SET
 * path is skipped and the champion attributes remain free of the
 * two action-hand / disable-action bits.  This is the per-champion
 * DISABLED-AVAILABLE side of the gate: the icon is NOT hatched.
 *
 * ReDMCSB CHAMPION.C F0330:2252-2255 — when the action index has a
 * positive G0491 entry, both bits are set and the disabled ticks
 * counter holds the per-action delay (6 for BLOCK, 35 for
 * CLIMB DOWN, 42 for FIREBALL, 30 for SPELLSHIELD, etc.).
 */
static void test_apply_disable_action_attribute_paths(void)
{
    DM1_V1_ChampionPanelDisabledIconStatePc34Compat state;
    DM1_V1_ChampionPanelDisabledIconResolveResultPc34Compat result;

    DM1_V1_ChampionPanelDisabledIconState_InitStatePc34Compat(&state, 4);

    /* Apply BLOCK (action_index=1) to champion 0 — should set the
     * two bits and remaining_disabled_ticks=6. */
    expect_int("apply.BLOCK.rc",
               DM1_V1_ChampionPanelDisabledIconState_ApplyDisableActionPc34Compat(
                   &state, 0, 1),
               1, "CHAMPION.C F0330:2252-2255 BLOCK path");
    expect_u16("apply.BLOCK.attrs", state.champions[0].attributes,
               (uint16_t)(DM1_V1_CPDIS_MASK_ACTION_HAND_PC34 |
                          DM1_V1_CPDIS_MASK_DISABLE_ACTION_PC34),
               "CHAMPION.C F0330:2252 M008_SET");
    expect_int("apply.BLOCK.ticks",
               state.champions[0].remaining_disabled_ticks, 6,
               "MENU.C:157 BLOCK");

    /* Apply N (action_index=0) to champion 1 — should be a no-op. */
    expect_int("apply.N.rc",
               DM1_V1_ChampionPanelDisabledIconState_ApplyDisableActionPc34Compat(
                   &state, 1, 0),
               0, "MENU.C:834 N never disables");
    expect_u16("apply.N.attrs", state.champions[1].attributes, 0,
               "MENU.C:834 N never sets MASK0x0008");
    expect_int("apply.N.ticks",
               state.champions[1].remaining_disabled_ticks, 0,
               "MENU.C:834 N no remaining ticks");

    /* Apply FIREBALL (action_index=20) to champion 2. */
    expect_int("apply.FIREBALL.rc",
               DM1_V1_ChampionPanelDisabledIconState_ApplyDisableActionPc34Compat(
                   &state, 2, 20),
               1, "MENU.C:157 FIREBALL");
    expect_int("apply.FIREBALL.ticks",
               state.champions[2].remaining_disabled_ticks, 42,
               "MENU.C:157 FIREBALL=42");

    /* Out-of-range champion or action index → -1. */
    expect_int("apply.oob.champion",
               DM1_V1_ChampionPanelDisabledIconState_ApplyDisableActionPc34Compat(
                   &state, 99, 1),
               -1, "CHAMPION.C F0330:2212 OOB champion guard");
    expect_int("apply.oob.action",
               DM1_V1_ChampionPanelDisabledIconState_ApplyDisableActionPc34Compat(
                   &state, 0, 999),
               -1, "CHAMPION.C F0330:2270 OOB action guard");

    DM1_V1_ChampionPanelDisabledIconState_ResolvePc34Compat(&state, &result);
    expect_int("apply.resolve.rc", result.accepted, 1,
               "resolve after apply");
    expect_bool("apply.resolve.hatched", result.any_hatched, true,
                "F0386:282 hatch on MASK0x0008");
    expect_bool("apply.resolve.per_champion",
                result.any_per_champion_disable_action, true,
                "F0386:282 per-champion hatch present");
    expect_int("apply.resolve.champions",
               result.champions_processed, 4,
               "all 4 champions processed");
    expect_int("apply.resolve.state[0]",
               (int)result.champion_results[0].state,
               (int)DM1_V1_CPDIS_STATE_PER_CHAMPION_DISABLE_ACTION_PC34,
               "champion 0 BLOCK");
    expect_int("apply.resolve.state[1]",
               (int)result.champion_results[1].state,
               (int)DM1_V1_CPDIS_STATE_AVAILABLE_PC34,
               "champion 1 N (no disable)");
    expect_int("apply.resolve.state[2]",
               (int)result.champion_results[2].state,
               (int)DM1_V1_CPDIS_STATE_PER_CHAMPION_DISABLE_ACTION_PC34,
               "champion 2 FIREBALL");
    expect_int("apply.resolve.state[3]",
               (int)result.champion_results[3].state,
               (int)DM1_V1_CPDIS_STATE_SLOT_EMPTY_PC34,
               "champion 3 untouched — empty-hand branch");
}

/*
 * ReDMCSB CHAMPION.C F0330:2255 + TIMELINE.C F0253 — the C11
 * EVENT_ENABLE_CHAMPION_ACTION event fires when GameTime has
 * advanced past the scheduled time.  AdvanceTimeline models that
 * moment: when remaining_disabled_ticks reaches 0 both attribute
 * bits are cleared and the icon becomes AVAILABLE again.
 */
static void test_advance_timeline_enable_event(void)
{
    DM1_V1_ChampionPanelDisabledIconStatePc34Compat state;
    DM1_V1_ChampionPanelDisabledIconResolveResultPc34Compat before;
    DM1_V1_ChampionPanelDisabledIconResolveResultPc34Compat after;

    DM1_V1_ChampionPanelDisabledIconState_InitStatePc34Compat(&state, 4);

    DM1_V1_ChampionPanelDisabledIconState_ApplyDisableActionPc34Compat(
        &state, 0, 1); /* BLOCK = 6 ticks */
    DM1_V1_ChampionPanelDisabledIconState_ApplyDisableActionPc34Compat(
        &state, 1, 19); /* BERZERK = 30 ticks */

    DM1_V1_ChampionPanelDisabledIconState_ResolvePc34Compat(&state, &before);
    expect_bool("advance.before.hatch0",
                before.champion_results[0].should_hatch_cell, true,
                "F0386:282 hatch before timeline tick");
    expect_bool("advance.before.hatch1",
                before.champion_results[1].should_hatch_cell, true,
                "F0386:282 hatch before timeline tick");

    /* Advance 6 ticks → champion 0 clears, champion 1 still ticking. */
    expect_int("advance.6ticks.rc",
               DM1_V1_ChampionPanelDisabledIconState_AdvanceTimelinePc34Compat(
                   &state, 6),
               6, "TIMELINE.C F0253 +5 enable tick");
    DM1_V1_ChampionPanelDisabledIconState_ResolvePc34Compat(&state, &after);
    expect_int("advance.6.state0",
               (int)after.champion_results[0].state,
               (int)DM1_V1_CPDIS_STATE_AVAILABLE_PC34,
               "TIMELINE.C F0253 enable cleared MASK0x0008");
    expect_u16("advance.6.attrs0", state.champions[0].attributes, 0,
               "TIMELINE.C F0253 enable cleared attribute bits");
    expect_int("advance.6.state1",
               (int)after.champion_results[1].state,
               (int)DM1_V1_CPDIS_STATE_PER_CHAMPION_DISABLE_ACTION_PC34,
               "champion 1 still ticking at 24");
    expect_int("advance.6.ticks1",
               state.champions[1].remaining_disabled_ticks, 24,
               "BERZERK=30 - 6");

    /* Advance the remaining 24 ticks → champion 1 also clears. */
    DM1_V1_ChampionPanelDisabledIconState_AdvanceTimelinePc34Compat(&state, 24);
    DM1_V1_ChampionPanelDisabledIconState_ResolvePc34Compat(&state, &after);
    expect_int("advance.30.state1",
               (int)after.champion_results[1].state,
               (int)DM1_V1_CPDIS_STATE_AVAILABLE_PC34,
               "TIMELINE.C F0253 enable cleared MASK0x0008");
    expect_u16("advance.30.attrs1", state.champions[1].attributes, 0,
               "TIMELINE.C F0253 enable cleared attribute bits");

    /* Negative ticks rejected. */
    expect_int("advance.negative.rc",
               DM1_V1_ChampionPanelDisabledIconState_AdvanceTimelinePc34Compat(
                   &state, -1),
               -1, "TIMELINE.C F0253 negative guard");
}

/*
 * ReDMCSB ACTIDRAW.C F0386:282 hatch gate — when
 * G0299_ui_CandidateChampionOrdinal > 0 OR
 * candidateMirrorPanelActive is set OR G0300_B_PartyIsResting is
 * set, every living champion's action icon cell is hatched
 * independently of the per-champion MASK0x0008 bit.  This pins
 * the GLOBAL gates of the source-locked hatch OR.  The per-champion
 * MASK0x0008 path is exercised separately by
 * test_apply_disable_action_attribute_paths.
 */
static void test_global_hatch_gates(void)
{
    DM1_V1_ChampionPanelDisabledIconStatePc34Compat state;
    DM1_V1_ChampionPanelDisabledIconResolveResultPc34Compat r;
    int i;

    /* Candidate gate: G0299_ui_CandidateChampionOrdinal = 1. */
    DM1_V1_ChampionPanelDisabledIconState_InitStatePc34Compat(&state, 4);
    /* Mark all four champions as having an action-hand object so the
     * global hatch gate exercises the F0386:262-266 "object branch",
     * not the F0386:262-264 empty-hand branch. */
    for (i = 0; i < 4; ++i) {
        state.champions[i].action_hand_empty = false;
    }
    state.candidate_champion_ordinal = 1;
    DM1_V1_ChampionPanelDisabledIconState_ApplyDisableActionPc34Compat(
        &state, 0, 0); /* N — would be AVAILABLE without the candidate gate */
    DM1_V1_ChampionPanelDisabledIconState_ResolvePc34Compat(&state, &r);
    expect_bool("global.candidate.hatched", r.any_hatched, true,
                "F0386:282 G0299 candidate hatch");
    expect_bool("global.candidate.flag", r.any_candidate_hatch, true,
                "F0386:282 G0299 candidate hatch flag");
    expect_int("global.candidate.state0",
               (int)r.champion_results[0].state,
               (int)DM1_V1_CPDIS_STATE_CANDIDATE_ACTIVE_PC34,
               "F0386:282 G0299 candidate state");

    /* Candidate panel active gate (no ordinal but panel flag). */
    DM1_V1_ChampionPanelDisabledIconState_InitStatePc34Compat(&state, 4);
    for (i = 0; i < 4; ++i) {
        state.champions[i].action_hand_empty = false;
    }
    state.candidate_mirror_panel_active = true;
    DM1_V1_ChampionPanelDisabledIconState_ResolvePc34Compat(&state, &r);
    expect_bool("global.panel.hatched", r.any_hatched, true,
                "F0386:282 candidate_mirror_panel_active hatch");
    expect_int("global.panel.state0",
               (int)r.champion_results[0].state,
               (int)DM1_V1_CPDIS_STATE_CANDIDATE_ACTIVE_PC34,
                "F0386:282 candidate_mirror_panel_active state");

    /* Party resting gate. */
    DM1_V1_ChampionPanelDisabledIconState_InitStatePc34Compat(&state, 4);
    for (i = 0; i < 4; ++i) {
        state.champions[i].action_hand_empty = false;
    }
    state.party_is_resting = true;
    DM1_V1_ChampionPanelDisabledIconState_ResolvePc34Compat(&state, &r);
    expect_bool("global.rest.hatched", r.any_hatched, true,
                "F0386:282 G0300 party-resting hatch");
    expect_bool("global.rest.flag", r.any_resting_hatch, true,
                "F0386:282 G0300 party-resting hatch flag");
    expect_int("global.rest.state0",
               (int)r.champion_results[0].state,
               (int)DM1_V1_CPDIS_STATE_PARTY_RESTING_PC34,
               "F0386:282 G0300 party-resting state");

    /* No gate fires → all 4 are AVAILABLE. */
    DM1_V1_ChampionPanelDisabledIconState_InitStatePc34Compat(&state, 4);
    for (i = 0; i < 4; ++i) {
        state.champions[i].action_hand_empty = false;
    }
    DM1_V1_ChampionPanelDisabledIconState_ResolvePc34Compat(&state, &r);
    expect_bool("global.none.hatched", r.any_hatched, false,
                "F0386:282 no global gate → no hatch");
    for (i = 0; i < 4; ++i) {
        char id[64];
        snprintf(id, sizeof(id), "global.none.state%d", i);
        expect_int(id, (int)r.champion_results[i].state,
                   (int)DM1_V1_CPDIS_STATE_AVAILABLE_PC34,
                   "F0386:282 no gate → AVAILABLE");
    }
}

/*
 * ReDMCSB ACTIDRAW.C F0386:234-238 dead-champion early-return —
 * when champion->CurrentHealth == 0 F0386 fills the C089..C092
 * cell BLACK and returns without entering the hatch gate.  This
 * pins the SOURCE-LOCKED disabled/unavailable side of the dead
 * champion state for the champion-panel gate.
 */
static void test_dead_champion_early_return(void)
{
    DM1_V1_ChampionPanelDisabledIconStatePc34Compat state;
    DM1_V1_ChampionPanelDisabledIconResolveResultPc34Compat r;

    DM1_V1_ChampionPanelDisabledIconState_InitStatePc34Compat(&state, 4);
    for (int i = 0; i < 4; ++i) {
        state.champions[i].action_hand_empty = false;
    }
    state.champions[2].current_health = 0;
    DM1_V1_ChampionPanelDisabledIconState_ResolvePc34Compat(&state, &r);
    expect_bool("dead.flag", r.any_dead_champion, true,
                "F0386:234-238 dead-champion early-return");
    expect_int("dead.state2",
               (int)r.champion_results[2].state,
               (int)DM1_V1_CPDIS_STATE_DEAD_CHAMPION_PC34,
               "F0386:234-238 DEAD_CHAMPION state id");
    expect_bool("dead.no_hatch",
                r.champion_results[2].should_hatch_cell, false,
                "F0386:234-238 no hatch on dead-champion cell");
    expect_bool("dead.early_return",
                r.champion_results[2].dead_champion_early_return, true,
                "F0386:234-238 L1183 CurrentHealth == 0");
    /* Dead-champion does not poison the other champions' state. */
    expect_int("dead.state0",
               (int)r.champion_results[0].state,
               (int)DM1_V1_CPDIS_STATE_AVAILABLE_PC34,
               "F0386:234-238 alive champion still AVAILABLE");
}

/*
 * ReDMCSB ACTIDRAW.C F0386:262-264 empty-hand blit — when the
 * champion's action-hand slot is C0xFFFF_THING_NONE, F0386 fills
 * the cell cyan and blits C201_ICON_ACTION_ICON_EMPTY_HAND.  The
 * hatch gate is not entered.  This pins the AVAILABLE-but-empty
 * side of the gate (distinct from PER_CHAMPION_DISABLE_ACTION).
 */
static void test_empty_hand_available(void)
{
    DM1_V1_ChampionPanelDisabledIconStatePc34Compat state;
    DM1_V1_ChampionPanelDisabledIconResolveResultPc34Compat r;

    DM1_V1_ChampionPanelDisabledIconState_InitStatePc34Compat(&state, 4);
    /* All four champions start with last_action_index=-1 (the
     * InitState default), so the resolve sees an empty-hand
     * configuration. */
    DM1_V1_ChampionPanelDisabledIconState_ResolvePc34Compat(&state, &r);
    expect_bool("empty.flag", r.any_empty_hand, true,
                "F0386:262-264 empty-hand blit");
    expect_int("empty.icon",
               r.champion_results[0].empty_hand_icon_index,
               (int)DM1_V1_CPDIS_ICON_EMPTY_HAND_PC34,
               "F0386:262-264 C201_ICON_ACTION_ICON_EMPTY_HAND");
    for (int i = 0; i < 4; ++i) {
        char id[64];
        snprintf(id, sizeof(id), "empty.state%d", i);
        expect_int(id, (int)r.champion_results[i].state,
                   (int)DM1_V1_CPDIS_STATE_SLOT_EMPTY_PC34,
                   "F0386:262-264 SLOT_EMPTY state id");
        snprintf(id, sizeof(id), "empty.no_hatch%d", i);
        expect_bool(id, r.champion_results[i].should_hatch_cell, false,
                    "F0386:262-264 empty hand never hatches");
    }
}

/*
 * ReDMCSB shield-border DISABLED side — when ALL three of
 * partyShieldDefense, spellShieldDefense, fireShieldDefense are 0,
 * the M11 m11_collect_v1_status_shield_border_graphics append loop
 * produces 0 borders.  This pins the inactive side of the
 * shield-border state; the active side is covered by the asset-
 * backed shield_border_pixel_probe ENABLED lane.
 */
static void test_shield_border_disabled_state(void)
{
    DM1_V1_ChampionPanelDisabledIconStatePc34Compat state;
    DM1_V1_ChampionPanelDisabledIconResolveResultPc34Compat r;

    /* All shields 0 → all 4 champions produce 0 borders. */
    DM1_V1_ChampionPanelDisabledIconState_InitStatePc34Compat(&state, 4);
    DM1_V1_ChampionPanelDisabledIconState_ResolvePc34Compat(&state, &r);
    expect_bool("shield.disabled.flag", r.any_shield_border_active, false,
                "M11 m11_collect_v1_status_shield_border_graphics 0 borders");
    for (int i = 0; i < 4; ++i) {
        char id[64];
        snprintf(id, sizeof(id), "shield.disabled.count%d", i);
        expect_int(id, r.champion_results[i].shield_border_count, 0,
                   "F0292 shield append returns 0 when no defense");
    }

    /* Single party shield → exactly 1 border, last draw order. */
    DM1_V1_ChampionPanelDisabledIconState_InitStatePc34Compat(&state, 4);
    state.champions[0].party_shield_defense = 4;
    state.champions[0].spell_shield_defense = 0;
    state.champions[0].fire_shield_defense = 0;
    DM1_V1_ChampionPanelDisabledIconState_ResolvePc34Compat(&state, &r);
    expect_int("shield.party.count",
               r.champion_results[0].shield_border_count, 1,
               "F0292 append order fire,spell,party reversed");
    expect_int("shield.party.gfx",
               r.champion_results[0].shield_border_graphics[0],
               (int)DM1_V1_CPDIS_GRAPHIC_PARTY_SHIELD_PC34,
               "F0292 reversed append: party draws first/topmost");
    expect_bool("shield.party.active", r.any_shield_border_active, true,
                "F0292 active side");

    /* All three defenses → 3 borders in reversed append order
     * (party, spell, fire — same order the M11
     * m11_collect_v1_status_shield_border_graphics helper uses). */
    DM1_V1_ChampionPanelDisabledIconState_InitStatePc34Compat(&state, 4);
    state.champions[1].party_shield_defense = 4;
    state.champions[1].spell_shield_defense = 4;
    state.champions[1].fire_shield_defense = 4;
    DM1_V1_ChampionPanelDisabledIconState_ResolvePc34Compat(&state, &r);
    expect_int("shield.full.count",
               r.champion_results[1].shield_border_count, 3,
               "F0292 append order fire,spell,party — reversed");
    expect_int("shield.full.gfx0",
               r.champion_results[1].shield_border_graphics[0],
               (int)DM1_V1_CPDIS_GRAPHIC_PARTY_SHIELD_PC34,
               "F0292 first draw is party");
    expect_int("shield.full.gfx1",
               r.champion_results[1].shield_border_graphics[1],
               (int)DM1_V1_CPDIS_GRAPHIC_SPELL_SHIELD_PC34,
               "F0292 second draw is spell");
    expect_int("shield.full.gfx2",
               r.champion_results[1].shield_border_graphics[2],
               (int)DM1_V1_CPDIS_GRAPHIC_FIRE_SHIELD_PC34,
               "F0292 third draw is fire (topmost)");
}

/*
 * ReDMCSB CHAMPION.C F0330:2208 + the F0386 hatch OR — the
 * per-champion MASK0x0008_DISABLE_ACTION path takes priority over
 * the global candidate / resting gates in the result enum (the
 * source-locked code branches on the bit-set first inside the F0386
 * L282 condition).  The Result enum reports the per-champion bit
 * as the dominant reason so callers can tell the two apart.
 */
static void test_per_champion_priority_over_global(void)
{
    DM1_V1_ChampionPanelDisabledIconStatePc34Compat state;
    DM1_V1_ChampionPanelDisabledIconResolveResultPc34Compat r;

    DM1_V1_ChampionPanelDisabledIconState_InitStatePc34Compat(&state, 4);
    for (int i = 0; i < 4; ++i) {
        state.champions[i].action_hand_empty = false;
    }
    state.candidate_champion_ordinal = 1;
    state.party_is_resting = true;
    DM1_V1_ChampionPanelDisabledIconState_ApplyDisableActionPc34Compat(
        &state, 0, 1); /* BLOCK: per-champion MASK0x0008 */

    DM1_V1_ChampionPanelDisabledIconState_ResolvePc34Compat(&state, &r);
    expect_int("priority.state0",
               (int)r.champion_results[0].state,
               (int)DM1_V1_CPDIS_STATE_PER_CHAMPION_DISABLE_ACTION_PC34,
               "F0330:2252 MASK0x0008 priority over global gates");
    expect_bool("priority.per_champion_only0",
                r.champion_results[0].should_hatch_cell_per_champion_only,
                true,
                "F0386:282 MASK0x0008 reported as per-champion hatch reason");

    /* Champion 1 (no per-champion bit) → CANDIDATE_ACTIVE because
     * the candidate gate fires first in the priority chain (matches
     * F0386:282 short-circuit OR ordering). */
    expect_int("priority.state1",
               (int)r.champion_results[1].state,
               (int)DM1_V1_CPDIS_STATE_CANDIDATE_ACTIVE_PC34,
               "F0386:282 G0299 candidate before G0300 resting");
    /* Champion 2 (no per-champion bit, candidate=0 by ordinal but
     * we set candidate_champion_ordinal=1 above) → CANDIDATE_ACTIVE. */
    expect_int("priority.state2",
               (int)r.champion_results[2].state,
               (int)DM1_V1_CPDIS_STATE_CANDIDATE_ACTIVE_PC34,
               "F0386:282 G0299 candidate wins over G0300 resting");
}

/*
 * Boundary checks on the resolve contract.  An out-of-range
 * champion in ApplyDisableAction must NOT corrupt the state.
 * Party count > CHAMPION_MAX_PARTY clamps to 4.
 */
static void test_boundary_clamps(void)
{
    DM1_V1_ChampionPanelDisabledIconStatePc34Compat state;
    DM1_V1_ChampionPanelDisabledIconResolveResultPc34Compat r;

    DM1_V1_ChampionPanelDisabledIconState_InitStatePc34Compat(&state, 99);
    expect_int("clamp.party_count", state.party_champion_count,
               DM1_V1_CPDIS_CHAMPION_COUNT_PC34,
               "InitState clamps party count to MAX_PARTY");
    DM1_V1_ChampionPanelDisabledIconState_ResolvePc34Compat(&state, &r);
    expect_int("clamp.processed", r.champions_processed,
               DM1_V1_CPDIS_CHAMPION_COUNT_PC34,
               "Resolve processes exactly MAX_PARTY champions");

    /* Negative party count clamps to 0 — no champions processed. */
    DM1_V1_ChampionPanelDisabledIconState_InitStatePc34Compat(&state, -1);
    expect_int("clamp.negative", state.party_champion_count, 0,
               "InitState clamps negative party count to 0");
    DM1_V1_ChampionPanelDisabledIconState_ResolvePc34Compat(&state, &r);
    expect_int("clamp.negative.processed", r.champions_processed, 0,
               "Resolve processes zero champions when party is empty");
}

/*
 * Name-table for the state enum — the per-state name string is
 * part of the public contract (callers want stable strings for
 * log lines and parity-evidence reports).
 */
static void test_state_name_strings(void)
{
    expect_str_eq("name.AVAILABLE",
                  DM1_V1_ChampionPanelDisabledIconState_NamePc34Compat(
                      DM1_V1_CPDIS_STATE_AVAILABLE_PC34),
                  "AVAILABLE", "state enum name");
    expect_str_eq("name.DEAD_CHAMPION",
                  DM1_V1_ChampionPanelDisabledIconState_NamePc34Compat(
                      DM1_V1_CPDIS_STATE_DEAD_CHAMPION_PC34),
                  "DEAD_CHAMPION", "state enum name");
    expect_str_eq("name.PER_CHAMPION_DISABLE_ACTION",
                  DM1_V1_ChampionPanelDisabledIconState_NamePc34Compat(
                      DM1_V1_CPDIS_STATE_PER_CHAMPION_DISABLE_ACTION_PC34),
                  "PER_CHAMPION_DISABLE_ACTION", "state enum name");
    expect_str_eq("name.CANDIDATE_ACTIVE",
                  DM1_V1_ChampionPanelDisabledIconState_NamePc34Compat(
                      DM1_V1_CPDIS_STATE_CANDIDATE_ACTIVE_PC34),
                  "CANDIDATE_ACTIVE", "state enum name");
    expect_str_eq("name.PARTY_RESTING",
                  DM1_V1_ChampionPanelDisabledIconState_NamePc34Compat(
                      DM1_V1_CPDIS_STATE_PARTY_RESTING_PC34),
                  "PARTY_RESTING", "state enum name");
    expect_str_eq("name.SLOT_EMPTY",
                  DM1_V1_ChampionPanelDisabledIconState_NamePc34Compat(
                      DM1_V1_CPDIS_STATE_SLOT_EMPTY_PC34),
                  "SLOT_EMPTY", "state enum name");
    expect_str_eq("name.PARTY_INCOMPLETE",
                  DM1_V1_ChampionPanelDisabledIconState_NamePc34Compat(
                      DM1_V1_CPDIS_STATE_PARTY_INCOMPLETE_PC34),
                  "PARTY_INCOMPLETE", "state enum name");
}

int main(void)
{
    test_evidence_and_invariants();
    test_g0491_disabled_ticks_table();
    test_apply_disable_action_attribute_paths();
    test_advance_timeline_enable_event();
    test_global_hatch_gates();
    test_dead_champion_early_return();
    test_empty_hand_available();
    test_shield_border_disabled_state();
    test_per_champion_priority_over_global();
    test_boundary_clamps();
    test_state_name_strings();

    printf("Assertions: %d\n", g_assertions);
    printf("Failures: %d\n", g_failures);
    if (g_assertions < 100) {
        printf("FAIL assertion floor got=%d want>=100\n", g_assertions);
        return 1;
    }
    if (g_failures) {
        return 1;
    }
    printf("DM1_V1_CHAMPION_PANEL_DISABLED_ICON_STATE_PC34_COMPAT_OK\n");
    return 0;
}
