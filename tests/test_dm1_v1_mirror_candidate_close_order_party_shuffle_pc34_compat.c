#include "firestaff/dm1/v1/mirror_candidate/close_order_party_shuffle_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static void probe_fail(const char *id)
{
    ++g_failures;
    printf("FAIL %s\n", id);
}

#define PROBE_ASSERT(id, expr)                                                   \
    do {                                                                         \
        ++g_assertions;                                                          \
        if (!(expr)) {                                                           \
            probe_fail(id);                                                      \
        }                                                                        \
    } while (0)

#define PROBE_ASSERT_EQ(id, got, want)                                           \
    do {                                                                         \
        int probe_got_ = (int)(got);                                             \
        int probe_want_ = (int)(want);                                             \
        ++g_assertions;                                                          \
        if (probe_got_ != probe_want_) {                                          \
            ++g_failures;                                                         \
            printf("FAIL %s got=%d want=%d\n", id, probe_got_, probe_want_);   \
        }                                                                        \
    } while (0)

#define PROBE_ASSERT_U32(id, got, want)                                          \
    do {                                                                         \
        uint32_t probe_got_ = (uint32_t)(got);                                   \
        uint32_t probe_want_ = (uint32_t)(want);                                 \
        ++g_assertions;                                                          \
        if (probe_got_ != probe_want_) {                                         \
            ++g_failures;                                                        \
            printf("FAIL %s got=0x%08x want=0x%08x\n", id, probe_got_,          \
                   probe_want_);                                                 \
        }                                                                        \
    } while (0)

static int contains(const char *haystack, const char *needle)
{
    return haystack && needle && strstr(haystack, needle) != 0;
}

static void test_source_evidence(void)
{
    const char *evidence =
        dm1_v1_mirror_candidate_close_order_party_shuffle_source_evidence_pc34();

    PROBE_ASSERT("evidence.present", evidence != 0);
    PROBE_ASSERT("evidence.f0282",
                 contains(evidence,
                          "REVIVE.C F0282:744-806 F0282_CHAMPION_"
                          "ProcessCommands160To162_"
                          "ClickInResurrectReincarnatePanel"));
    PROBE_ASSERT("evidence.f0355.close",
                 contains(evidence,
                          "F0355_INVENTORY_Toggle_CPSE(C04_CHAMPION_CLOSE_"
                          "INVENTORY)"));
    PROBE_ASSERT("evidence.line_895", contains(evidence, "line 895"));
    PROBE_ASSERT("evidence.f0284",
                 contains(evidence,
                          "CHAMPION.C F0284:93-130 F0284_CHAMPION_"
                          "SetPartyDirection"));
    PROBE_ASSERT("evidence.f0296",
                 contains(evidence, "F0296_CHAMPION_DrawChangedObjectIcons"));
    PROBE_ASSERT("evidence.f0365",
                 contains(evidence, "CLIKMENU.C F0365:142-174"));
    PROBE_ASSERT("evidence.f0355.panel",
                 contains(evidence, "PANEL.C F0355:2244+"));
    PROBE_ASSERT("evidence.f0334", contains(evidence, "F0334"));
    PROBE_ASSERT("evidence.f0361", contains(evidence, "COMMAND.C F0361:1709-1813"));
    PROBE_ASSERT("evidence.f0359", contains(evidence, "COMMAND.C F0359:1452-1662"));
    PROBE_ASSERT("evidence.f0380", contains(evidence, "COMMAND.C F0380:2045-2156"));
    PROBE_ASSERT("evidence.defs.c040", contains(evidence, "C040"));
    PROBE_ASSERT("evidence.defs.c160", contains(evidence, "C160, C161, C162"));
    PROBE_ASSERT("evidence.defs.c038", contains(evidence, "C038"));
    PROBE_ASSERT("evidence.defs.c037", contains(evidence, "C037"));
    PROBE_ASSERT("evidence.defs.c159", contains(evidence, "C159"));
    PROBE_ASSERT("evidence.defs.m070", contains(evidence, "M070"));
    PROBE_ASSERT("evidence.defs.m516", contains(evidence, "M516"));
    PROBE_ASSERT("evidence.non_overlap_pass783", contains(evidence, "pass783"));
    PROBE_ASSERT("evidence.non_overlap_pass788", contains(evidence, "pass788"));
    PROBE_ASSERT("evidence.non_overlap_pass780", contains(evidence, "pass780"));
    PROBE_ASSERT("evidence.non_overlap_party_direction",
                 contains(evidence, "party_direction"));
    PROBE_ASSERT("evidence.non_overlap_reselect",
                 contains(evidence,
                          "reselect_after_deposit_with_party_rotate"));
    PROBE_ASSERT("evidence.non_overlap_reshuffle",
                 contains(evidence, "reshuffle_panel_live"));
    PROBE_ASSERT("evidence.non_overlap_close_button",
                 contains(evidence, "close_button"));
    PROBE_ASSERT("evidence.non_overlap_owner_swap",
                 contains(evidence, "c040_chrome_inventory_owner_swap"));
    PROBE_ASSERT(
        "evidence.marker",
        contains(evidence,
                 "pass790plus_dm1_v1_auto_mirror_candidate_party_"
                 "shuffle_close_order_gate"));
}

static void test_default_state(void)
{
    Dm1V1MirrorCandidateCloseOrderPartyShuffleStatePc34 state =
        dm1_v1_mirror_candidate_close_order_party_shuffle_default_state_pc34();

    PROBE_ASSERT_EQ("default.contract_only", state.contractOnly, 1);
    PROBE_ASSERT_EQ("default.no_assets", state.noAssetReads, 1);
    PROBE_ASSERT_EQ("default.no_dos_pixel_claim",
                    state.noOriginalDosPixelParityClaim, 1);
    PROBE_ASSERT_EQ("default.party_count", state.partyChampionCount, 3);
    PROBE_ASSERT_EQ("default.candidate_ordinal",
                    state.candidatePartyOrdinal, 2);
    PROBE_ASSERT_EQ("default.candidate_index_byte", state.candidateIndexByte, 1);
    PROBE_ASSERT_EQ("default.g0299", state.g0299CandidateOrdinal, 2);
    PROBE_ASSERT_EQ("default.g0305", state.g0305PartyChampionCount, 3);
    PROBE_ASSERT_EQ("default.g0308", state.g0308PartyDirection, 0);
    PROBE_ASSERT_EQ("default.c040_open", state.c040PanelOpen, 1);
    PROBE_ASSERT_EQ("default.c040_closed", state.c040PanelClosed, 0);
    PROBE_ASSERT_EQ("default.c038_priority", state.c038PanelPriorityByte, 0x38);
    PROBE_ASSERT_EQ("default.c037_status_hand", state.c037StatusHandBoxByte,
                    0x37);
    PROBE_ASSERT_EQ("default.c159_icon", state.c159ChampionIconByte, 0x59);
    PROBE_ASSERT_EQ("default.m568", state.m568PanelResurrectReincarnate, 568);
    PROBE_ASSERT_EQ("default.m070", state.m070PanelOwnerOrdinal, 2);
    PROBE_ASSERT_EQ("default.cell0", state.championCell[0], 0);
    PROBE_ASSERT_EQ("default.cell1", state.championCell[1], 0);
    PROBE_ASSERT_EQ("default.cell2", state.championCell[2], 0);
    PROBE_ASSERT_EQ("default.dir0", state.championDirection[0], 0);
    PROBE_ASSERT_EQ("default.dir1", state.championDirection[1], 0);
    PROBE_ASSERT_EQ("default.dir2", state.championDirection[2], 0);
    PROBE_ASSERT_EQ("default.f0284.count", state.f0284SetPartyDirectionCount, 0);
    PROBE_ASSERT_EQ("default.f0284.first", state.f0284FirstDelta, 0);
    PROBE_ASSERT_EQ("default.f0284.second", state.f0284SecondDelta, 0);
    PROBE_ASSERT_EQ("default.f0296.count",
                    state.f0296DrawChangedObjectIconsCount, 0);
    PROBE_ASSERT_EQ("default.f0282.accept", state.f0282AcceptClearCount, 0);
    PROBE_ASSERT_EQ("default.f0282.cancel", state.f0282CancelClearCount, 0);
    PROBE_ASSERT_EQ("default.f0282.pre", state.f0282ReadsPreShuffleCandidate,
                    0);
    PROBE_ASSERT_EQ("default.queue.depth", state.commandQueueDepth, 0);
    PROBE_ASSERT_EQ("default.queue.turn", state.queueWriteCountF0361Turn, 0);
    PROBE_ASSERT_EQ("default.queue.click", state.queueWriteCountF0359PanelClick,
                    0);
    PROBE_ASSERT_EQ("default.drain", state.dispatchDrainCountF0380, 0);
    PROBE_ASSERT("default.hash.nonzero",
                 dm1_v1_mirror_candidate_close_order_party_shuffle_hash_pc34(
                     &state) != 0);
}

static void test_ordered_regression(uint32_t *out_hash)
{
    Dm1V1MirrorCandidateCloseOrderPartyShuffleStatePc34 state =
        dm1_v1_mirror_candidate_close_order_party_shuffle_default_state_pc34();
    Dm1V1MirrorCandidateCloseOrderPartyShuffleResultPc34 result;
    int ok;

    ok = dm1_v1_mirror_candidate_close_order_party_shuffle_run_pc34(&state,
                                                                     &result);
    PROBE_ASSERT_EQ("run.ok", ok, 1);
    PROBE_ASSERT_EQ("result.closeDispatchedFirst",
                    result.closeDispatchedFirst, 1);
    PROBE_ASSERT_EQ("result.f0282FiredOnPreShuffleParty",
                    result.f0282FiredOnPreShuffleParty, 1);
    PROBE_ASSERT_EQ("result.g0299ClearedOnClose", result.g0299ClearedOnClose,
                    1);
    PROBE_ASSERT_EQ("result.g0305DecrementedOnClose",
                    result.g0305DecrementedOnClose, 1);
    PROBE_ASSERT_EQ("result.candidateIndexStableThroughClose",
                    result.candidateIndexByteStableThroughClose, 1);
    PROBE_ASSERT_EQ("result.c040PanelClosedOnClose",
                    result.c040PanelClosedOnClose, 1);
    PROBE_ASSERT_EQ("result.shuffledFirst", result.shuffledFirst, 1);
    PROBE_ASSERT_EQ("result.shuffledSecond", result.shuffledSecond, 1);
    PROBE_ASSERT_EQ("result.f0284FiredTwice", result.f0284FiredTwice, 1);
    PROBE_ASSERT_EQ("result.f0284FirstDelta", result.f0284FirstDeltaCorrect, 1);
    PROBE_ASSERT_EQ("result.f0284SecondDelta", result.f0284SecondDeltaCorrect,
                    1);
    PROBE_ASSERT_EQ("result.f0296CalledTwice", result.f0296CalledTwice, 1);
    PROBE_ASSERT_EQ("result.g0308EastAfterFirst", result.g0308EastAfterFirst, 1);
    PROBE_ASSERT_EQ("result.g0308SouthAfterSecond",
                    result.g0308SouthAfterSecond, 1);
    PROBE_ASSERT_EQ("result.panelStayedClosed",
                    result.panelStayedClosedThroughShuffle, 1);
    PROBE_ASSERT_EQ("result.c038Preserved", result.c038PanelPriorityPreserved,
                    1);
    PROBE_ASSERT_EQ("result.c037Preserved", result.c037StatusHandBoxPreserved,
                    1);
    PROBE_ASSERT_EQ("result.c159Preserved", result.c159ChampionIconPreserved, 1);
    PROBE_ASSERT_EQ("result.m070Preserved", result.m070PanelOwnerPreserved, 1);
    PROBE_ASSERT_EQ("result.m568Preserved", result.m568PanelContentPreserved, 1);
    PROBE_ASSERT_EQ("result.g0308SouthAfterShuffle",
                    result.g0308SouthAfterShuffle, 1);
    PROBE_ASSERT_EQ("result.cellsAfterShuffle", result.championCellsAfterShuffle,
                    1);
    PROBE_ASSERT_EQ("result.directionsAfterShuffle",
                    result.championDirectionsAfterShuffle, 1);
    PROBE_ASSERT_EQ("result.cellMutatedThroughShuffle",
                    result.cellMutatedThroughShuffle, 1);
    PROBE_ASSERT_EQ("result.directionMutatedThroughShuffle",
                    result.directionMutatedThroughShuffle, 1);
    PROBE_ASSERT_EQ("result.queueWriteOrder", result.queueWriteOrderPreserved,
                    1);
    PROBE_ASSERT_EQ("result.dispatchOrder", result.dispatchOrderPreserved, 1);
    PROBE_ASSERT_EQ("result.f0380DrainAll", result.f0380DrainProcessedAll, 1);
    PROBE_ASSERT_EQ("result.anchors", result.sourceAnchorsPresent, 1);
    PROBE_ASSERT_EQ("state.g0299.final", state.g0299CandidateOrdinal, 0);
    PROBE_ASSERT_EQ("state.g0305.final", state.g0305PartyChampionCount, 2);
    PROBE_ASSERT_EQ("state.g0308.final", state.g0308PartyDirection, 2);
    PROBE_ASSERT_EQ("state.c040.open.final", state.c040PanelOpen, 0);
    PROBE_ASSERT_EQ("state.c040.closed.final", state.c040PanelClosed, 1);
    PROBE_ASSERT_EQ("state.c038.final", state.c038PanelPriorityByte, 0x38);
    PROBE_ASSERT_EQ("state.c037.final", state.c037StatusHandBoxByte, 0x37);
    PROBE_ASSERT_EQ("state.c159.final", state.c159ChampionIconByte, 0x59);
    PROBE_ASSERT_EQ("state.m070.final", state.m070PanelOwnerOrdinal, 2);
    PROBE_ASSERT_EQ("state.m568.final", state.m568PanelResurrectReincarnate, 568);
    PROBE_ASSERT_EQ("state.cell0.final", state.championCell[0], 2);
    PROBE_ASSERT_EQ("state.cell1.final", state.championCell[1], 2);
    /* The 3rd champion was removed by the C160 close, so its
     * cell stays at the post-decrement value (0). */
    PROBE_ASSERT_EQ("state.cell2.final", state.championCell[2], 0);
    PROBE_ASSERT_EQ("state.dir0.final", state.championDirection[0], 2);
    PROBE_ASSERT_EQ("state.dir1.final", state.championDirection[1], 2);
    PROBE_ASSERT_EQ("state.dir2.final", state.championDirection[2], 0);
    PROBE_ASSERT_EQ("state.f0284.count.final", state.f0284SetPartyDirectionCount,
                    2);
    PROBE_ASSERT_EQ("state.f0284.first.final", state.f0284FirstDelta, 1);
    PROBE_ASSERT_EQ("state.f0284.second.final", state.f0284SecondDelta, 1);
    PROBE_ASSERT_EQ("state.f0296.count.final",
                    state.f0296DrawChangedObjectIconsCount, 2);
    PROBE_ASSERT_EQ("state.f0282.accept.final", state.f0282AcceptClearCount, 1);
    PROBE_ASSERT_EQ("state.f0282.cancel.final", state.f0282CancelClearCount, 0);
    PROBE_ASSERT_EQ("state.f0282.pre.final",
                    state.f0282ReadsPreShuffleCandidate, 1);
    PROBE_ASSERT_EQ("state.queue.depth.final", state.commandQueueDepth, 0);
    PROBE_ASSERT_EQ("state.queue.turn.final", state.queueWriteCountF0361Turn, 2);
    PROBE_ASSERT_EQ("state.queue.click.final",
                    state.queueWriteCountF0359PanelClick, 1);
    PROBE_ASSERT_EQ("state.drain.final", state.dispatchDrainCountF0380, 3);
    PROBE_ASSERT_EQ("state.dispatch0", state.dispatchOrder[0], 160);
    PROBE_ASSERT_EQ("state.dispatch1", state.dispatchOrder[1], 1);
    PROBE_ASSERT_EQ("state.dispatch2", state.dispatchOrder[2], 1);
    PROBE_ASSERT_EQ("state.trace.seed", state.trace[0], 7901);
    PROBE_ASSERT_EQ("state.trace.queue_yes", state.trace[1], 7902);
    PROBE_ASSERT_EQ("state.trace.drain_yes", state.trace[2], 7903);
    PROBE_ASSERT_EQ("state.trace.f0282_accept", state.trace[3], 7904);
    PROBE_ASSERT_EQ("state.trace.f0282_done", state.trace[4], 7905);
    PROBE_ASSERT_EQ("state.trace.queue_turn1", state.trace[5], 7906);
    PROBE_ASSERT_EQ("state.trace.drain_turn1", state.trace[6], 7907);
    PROBE_ASSERT_EQ("state.trace.f0284_first", state.trace[7], 7908);
    PROBE_ASSERT_EQ("state.trace.f0284_first_done", state.trace[8], 7909);
    PROBE_ASSERT_EQ("state.trace.queue_turn2", state.trace[9], 7910);
    PROBE_ASSERT_EQ("state.trace.drain_turn2", state.trace[10], 7911);
    PROBE_ASSERT_EQ("state.trace.f0284_second", state.trace[11], 7912);
    PROBE_ASSERT_EQ("state.trace.f0284_second_done", state.trace[12], 7913);
    PROBE_ASSERT("hash.before.nonzero", result.beforeHash != 0);
    PROBE_ASSERT("hash.after_close.nonzero", result.afterCloseHash != 0);
    PROBE_ASSERT("hash.after_first.nonzero", result.afterFirstShuffleHash != 0);
    PROBE_ASSERT("hash.after_second.nonzero",
                 result.afterSecondShuffleHash != 0);
    PROBE_ASSERT("hash.close_changes",
                 result.afterCloseHash != result.beforeHash);
    PROBE_ASSERT("hash.first_changes",
                 result.afterFirstShuffleHash != result.afterCloseHash);
    PROBE_ASSERT("hash.second_changes",
                 result.afterSecondShuffleHash != result.afterFirstShuffleHash);
    PROBE_ASSERT_U32(
        "hash.result_matches", result.hash,
        dm1_v1_mirror_candidate_close_order_party_shuffle_hash_pc34(&state));
    *out_hash = result.hash;
}

static void test_guards(void)
{
    Dm1V1MirrorCandidateCloseOrderPartyShuffleStatePc34 state =
        dm1_v1_mirror_candidate_close_order_party_shuffle_default_state_pc34();
    Dm1V1MirrorCandidateCloseOrderPartyShuffleResultPc34 result;

    PROBE_ASSERT_EQ(
        "guard.null_state",
        dm1_v1_mirror_candidate_close_order_party_shuffle_run_pc34(0, &result),
        0);
    PROBE_ASSERT_EQ(
        "guard.null_result",
        dm1_v1_mirror_candidate_close_order_party_shuffle_run_pc34(&state, 0),
        0);
    state =
        dm1_v1_mirror_candidate_close_order_party_shuffle_default_state_pc34();
    state.contractOnly = 0;
    PROBE_ASSERT_EQ(
        "guard.non_contract",
        dm1_v1_mirror_candidate_close_order_party_shuffle_run_pc34(&state,
                                                                    &result),
        0);
    state =
        dm1_v1_mirror_candidate_close_order_party_shuffle_default_state_pc34();
    state.c040PanelOpen = 0;
    PROBE_ASSERT_EQ(
        "guard.panel_closed",
        dm1_v1_mirror_candidate_close_order_party_shuffle_run_pc34(&state,
                                                                    &result),
        0);
    state =
        dm1_v1_mirror_candidate_close_order_party_shuffle_default_state_pc34();
    state.g0299CandidateOrdinal = 0;
    PROBE_ASSERT_EQ(
        "guard.no_candidate",
        dm1_v1_mirror_candidate_close_order_party_shuffle_run_pc34(&state,
                                                                    &result),
        0);
}

int main(void)
{
    uint32_t hash = 0;

    test_source_evidence();
    test_default_state();
    test_ordered_regression(&hash);
    test_guards();

    if (g_failures != 0 || g_assertions < 110) {
        printf("FAIL test_dm1_v1_mirror_candidate_close_order_party_shuffle_pc34_compat assertions=%d failures=%d hash=0x%08X\n",
               g_assertions, g_failures, hash);
        return 1;
    }
    printf("PASS test_dm1_v1_mirror_candidate_close_order_party_shuffle_pc34_compat assertions=%d failures=0 hash=0x%08X\n",
           g_assertions, hash);
    return 0;
}
