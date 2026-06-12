#include "firestaff/dm1/v1/mirror_candidate/resurrect_chest_close_order_pc34_compat.h"

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
        int probe_want_ = (int)(want);                                           \
        ++g_assertions;                                                          \
        if (probe_got_ != probe_want_) {                                         \
            ++g_failures;                                                        \
            printf("FAIL %s got=%d want=%d\n", id, probe_got_, probe_want_);    \
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
        dm1_v1_mirror_candidate_resurrect_chest_close_order_source_evidence_pc34();

    PROBE_ASSERT("evidence.present", evidence != 0);
    PROBE_ASSERT("evidence.f0280",
                 contains(evidence, "REVIVE.C F0280:124-132"));
    PROBE_ASSERT("evidence.f0282", contains(evidence, "F0282:744-806"));
    PROBE_ASSERT("evidence.f0333",
                 contains(evidence, "CHEST.C F0333:30-67"));
    PROBE_ASSERT("evidence.f0334", contains(evidence, "F0334:113-132"));
    PROBE_ASSERT("evidence.f0163",
                 contains(evidence, "DUNGEON.C F0163:1796-1837"));
    PROBE_ASSERT("evidence.f0297",
                 contains(evidence, "CHAMPION.C F0297:243-298"));
    PROBE_ASSERT("evidence.f0298", contains(evidence, "F0298:270-298"));
    PROBE_ASSERT("evidence.f0302", contains(evidence, "F0302:662-714"));
    PROBE_ASSERT("evidence.f0359",
                 contains(evidence, "COMMAND.C F0359:1452-1662"));
    PROBE_ASSERT("evidence.f0361", contains(evidence, "F0361:1709-1813"));
    PROBE_ASSERT("evidence.f0378", contains(evidence, "F0378:1956-1993"));
    PROBE_ASSERT("evidence.f0380", contains(evidence, "F0380:2045-2178"));
    PROBE_ASSERT("evidence.mouse.deviation",
                 contains(evidence, "Requested MOUSE.C") &&
                     contains(evidence, "not present"));
    PROBE_ASSERT("evidence.io.f0077", contains(evidence, "IO.C:1102-1122"));
    PROBE_ASSERT("evidence.utamscr.f0078",
                 contains(evidence, "UTAMSCR.C:141-150"));
    PROBE_ASSERT("evidence.defs.c037", contains(evidence, "C037/C038"));
    PROBE_ASSERT("evidence.defs.c040", contains(evidence, "C040"));
    PROBE_ASSERT("evidence.defs.c159", contains(evidence, "C159"));
    PROBE_ASSERT("evidence.defs.c160", contains(evidence, "C160..C162"));
    PROBE_ASSERT("evidence.defs.c537", contains(evidence, "C537..C544"));
    PROBE_ASSERT("evidence.defs.c545", contains(evidence, "C545"));
    PROBE_ASSERT("evidence.defs.g0299", contains(evidence, "G0299"));
    PROBE_ASSERT("evidence.defs.g0426", contains(evidence, "G0426"));
    PROBE_ASSERT("evidence.pass780",
                 contains(evidence,
                          "pass780_dm1_v1_mirror_candidate_resurrect_chest_close_order"));
}

static void test_default_state(void)
{
    Dm1V1MirrorCandidateResurrectChestCloseOrderStatePc34 state =
        dm1_v1_mirror_candidate_resurrect_chest_close_order_default_state_pc34();
    int i;

    PROBE_ASSERT_EQ("default.contract_only", state.contractOnly, 1);
    PROBE_ASSERT_EQ("default.no_assets", state.noAssetReads, 1);
    PROBE_ASSERT_EQ("default.no_dos_pixel_claim",
                    state.noOriginalDosPixelParityClaim, 1);
    PROBE_ASSERT_EQ("default.party_count", state.partyChampionCount, 3);
    PROBE_ASSERT_EQ("default.leader", state.leaderIndex, 1);
    PROBE_ASSERT_EQ("default.inventory_ordinal",
                    state.inventoryChampionOrdinal, 2);
    PROBE_ASSERT_EQ("default.candidate_ordinal",
                    state.candidateChampionOrdinal, 2);
    PROBE_ASSERT_EQ("default.candidate_index_byte",
                    state.candidateIndexByte, 1);
    PROBE_ASSERT_EQ("default.g0299", state.g0299CandidateOrdinal, 2);
    PROBE_ASSERT_EQ("default.c040_open", state.c040PanelOpen, 1);
    PROBE_ASSERT_EQ("default.c038_priority", state.c038PanelPriorityByte,
                    0x38);
    PROBE_ASSERT_EQ("default.c037_status_hand", state.c037StatusHandBoxByte,
                    0x37);
    PROBE_ASSERT_EQ("default.c159_icon", state.c159ChampionIconByte, 0x59);
    PROBE_ASSERT_EQ("default.g0426", state.g0426OpenChestThing, 0x6426);
    PROBE_ASSERT_EQ("default.leader_hand_c540", state.leaderHandThing,
                    0xc540);
    PROBE_ASSERT_EQ("default.f0280_published", state.f0280PublishCount, 1);
    PROBE_ASSERT_EQ("default.f0297_already_done",
                    state.f0297PutAlreadyDoneCount, 1);
    PROBE_ASSERT_EQ("default.candidate_chain_head", state.candidateChain[0],
                    2);
    PROBE_ASSERT_EQ("default.candidate_chain_tail", state.candidateChain[1],
                    4);
    for (i = 0; i < DM1_V1_MC_RCCO_CHEST_SLOT_COUNT_PC34; ++i) {
        if (i == 2) {
            PROBE_ASSERT_EQ("default.visible_slot_none",
                            state.chestVisibleSlots[i],
                            DM1_V1_MC_RCCO_NONE_PC34);
        } else {
            PROBE_ASSERT_EQ("default.visible_slot_zone",
                            state.chestVisibleSlots[i], 537 + i);
        }
    }
    PROBE_ASSERT("default.hash.nonzero",
                 dm1_v1_mirror_candidate_resurrect_chest_close_order_hash_pc34(
                     &state) != 0);
}

static void test_ordered_regression(uint32_t *out_hash)
{
    Dm1V1MirrorCandidateResurrectChestCloseOrderStatePc34 state =
        dm1_v1_mirror_candidate_resurrect_chest_close_order_default_state_pc34();
    Dm1V1MirrorCandidateResurrectChestCloseOrderResultPc34 result;
    int ok;
    int i;

    ok = dm1_v1_mirror_candidate_resurrect_chest_close_order_run_pc34(
        &state, &result);
    PROBE_ASSERT_EQ("run.ok", ok, 1);
    PROBE_ASSERT_EQ("result.accepted_first", result.acceptedFirst, 1);
    PROBE_ASSERT_EQ("result.g0299_cleared_first", result.g0299ClearedFirst,
                    1);
    PROBE_ASSERT_EQ("result.candidate_chain_removed_first",
                    result.candidateChainRemovedFirst, 1);
    PROBE_ASSERT_EQ("result.candidate_index_stable",
                    result.candidateIndexByteStable, 1);
    PROBE_ASSERT_EQ("result.c040_closed", result.c040PanelClosed, 1);
    PROBE_ASSERT_EQ("result.c038_preserved",
                    result.c038PanelPriorityPreserved, 1);
    PROBE_ASSERT_EQ("result.c037_stable", result.c037StatusHandBoxStable, 1);
    PROBE_ASSERT_EQ("result.chest_closed_second", result.chestClosedSecond,
                    1);
    PROBE_ASSERT_EQ("result.g0426_cleared_second", result.g0426ClearedSecond,
                    1);
    PROBE_ASSERT_EQ("result.visible_slots_cleared",
                    result.visibleSlotsClearedSecond, 1);
    PROBE_ASSERT_EQ("result.container_relinked", result.chestContainerRelinked,
                    1);
    PROBE_ASSERT_EQ("result.leader_hand_preserved",
                    result.leaderHandPreservedAfterClose, 1);
    PROBE_ASSERT_EQ("result.leader_hand_not_stripped",
                    result.leaderHandNotStripped, 1);
    PROBE_ASSERT_EQ("result.forward_queued_after_close",
                    result.forwardQueuedAfterClose, 1);
    PROBE_ASSERT_EQ("result.forward_drained_closed",
                    result.forwardDrainedOnClosedChest, 1);
    PROBE_ASSERT_EQ("result.wheel_leader_hand",
                    result.wheelAfterForwardLandedOnLeaderHand, 1);
    PROBE_ASSERT_EQ("result.wheel_not_chest", result.wheelDidNotLandOnChest,
                    1);
    PROBE_ASSERT_EQ("result.queue_order", result.queueWriteOrderPreserved, 1);
    PROBE_ASSERT_EQ("result.dispatch_order", result.dispatchOrderPreserved, 1);
    PROBE_ASSERT_EQ("result.f0380_all", result.f0380DrainProcessedAll, 1);
    PROBE_ASSERT_EQ("result.c159_stable", result.c159ChampionIconStable, 1);
    PROBE_ASSERT_EQ("result.anchors", result.sourceAnchorsPresent, 1);
    PROBE_ASSERT_EQ("state.g0299.final", state.g0299CandidateOrdinal, 0);
    PROBE_ASSERT_EQ("state.candidate_index.final", state.candidateIndexByte,
                    1);
    PROBE_ASSERT_EQ("state.candidate_removed0", state.candidateChain[0], 4);
    PROBE_ASSERT_EQ("state.candidate_removed1", state.candidateChain[1], 5);
    PROBE_ASSERT_EQ("state.c040.open.final", state.c040PanelOpen, 0);
    PROBE_ASSERT_EQ("state.c040.closed.final", state.c040PanelClosed, 1);
    PROBE_ASSERT_EQ("state.c038.final", state.c038PanelPriorityByte, 0x38);
    PROBE_ASSERT_EQ("state.c037.final", state.c037StatusHandBoxByte, 0x37);
    PROBE_ASSERT_EQ("state.c159.final", state.c159ChampionIconByte, 0x59);
    PROBE_ASSERT_EQ("state.g0426.final", state.g0426OpenChestThing,
                    DM1_V1_MC_RCCO_NONE_PC34);
    PROBE_ASSERT_EQ("state.leader_hand.final", state.leaderHandThing,
                    0xc540);
    PROBE_ASSERT_EQ("state.f0298.not_called", state.f0298RemoveCount, 0);
    PROBE_ASSERT_EQ("state.f0334.once", state.f0334ChestCloseCount, 1);
    PROBE_ASSERT_EQ("state.f0163.relinks", state.f0163RelinkCount, 6);
    PROBE_ASSERT_EQ("state.f0282.once", state.f0282AcceptClearCount, 1);
    PROBE_ASSERT_EQ("state.f0359.two", state.queueWriteCountF0359, 2);
    PROBE_ASSERT_EQ("state.f0361.one", state.queueWriteCountF0361, 1);
    PROBE_ASSERT_EQ("state.f0077.one", state.queueWriteCountWheelF0077, 1);
    PROBE_ASSERT_EQ("state.f0078.one", state.wheelDrainCountF0078, 1);
    PROBE_ASSERT_EQ("state.f0380.four", state.dispatchDrainCountF0380, 4);
    PROBE_ASSERT_EQ("state.f0302.wheel", state.f0302ClickDispatchCount, 1);
    PROBE_ASSERT_EQ("state.queue.empty", state.commandQueueDepth, 0);
    PROBE_ASSERT_EQ("state.forward.queued_closed",
                    state.forwardQueuedAfterChestClose, 1);
    PROBE_ASSERT_EQ("state.forward.drained_closed",
                    state.forwardDrainedOnClosedChest, 1);
    PROBE_ASSERT_EQ("state.wheel.queued_after_forward",
                    state.wheelQueuedAfterForward, 1);
    PROBE_ASSERT_EQ("state.wheel.saw_closed", state.wheelSawClosedChest, 1);
    PROBE_ASSERT_EQ("state.wheel.target", state.wheelTarget,
                    DM1_V1_MC_RCCO_WHEEL_TARGET_LEADER_HAND_PC34);
    PROBE_ASSERT_EQ("state.dispatch0", state.dispatchOrder[0],
                    DM1_V1_MC_RCCO_COMMAND_C040_YES_PC34);
    PROBE_ASSERT_EQ("state.dispatch1", state.dispatchOrder[1],
                    DM1_V1_MC_RCCO_COMMAND_CHEST_CLOSE_PC34);
    PROBE_ASSERT_EQ("state.dispatch2", state.dispatchOrder[2],
                    DM1_V1_MC_RCCO_COMMAND_MOVE_FORWARD_PC34);
    PROBE_ASSERT_EQ("state.dispatch3", state.dispatchOrder[3],
                    DM1_V1_MC_RCCO_COMMAND_WHEEL_UP_PC34);
    PROBE_ASSERT_EQ("state.trace.seed", state.trace[0], 7800);
    PROBE_ASSERT_EQ("state.trace.queue_yes", state.trace[1], 7801);
    PROBE_ASSERT_EQ("state.trace.drain_yes", state.trace[2], 7802);
    PROBE_ASSERT_EQ("state.trace.queue_close", state.trace[3], 7803);
    PROBE_ASSERT_EQ("state.trace.drain_close", state.trace[4], 7804);
    PROBE_ASSERT_EQ("state.trace.queue_forward", state.trace[5], 7805);
    PROBE_ASSERT_EQ("state.trace.drain_forward", state.trace[6], 7806);
    PROBE_ASSERT_EQ("state.trace.queue_wheel", state.trace[7], 7807);
    PROBE_ASSERT_EQ("state.trace.drain_wheel", state.trace[8], 7808);
    for (i = 0; i < DM1_V1_MC_RCCO_CHEST_SLOT_COUNT_PC34; ++i) {
        PROBE_ASSERT_EQ("state.visible_cleared", state.chestVisibleSlots[i],
                        DM1_V1_MC_RCCO_NONE_PC34);
    }
    PROBE_ASSERT_EQ("state.container.chain0", state.chestContainerChain[0],
                    537);
    PROBE_ASSERT_EQ("state.container.chain1", state.chestContainerChain[1],
                    538);
    PROBE_ASSERT_EQ("state.container.chain2.skip_none",
                    state.chestContainerChain[2], 540);
    PROBE_ASSERT("hash.before.nonzero", result.beforeHash != 0);
    PROBE_ASSERT("hash.after_accept.nonzero", result.afterAcceptHash != 0);
    PROBE_ASSERT("hash.after_close.nonzero", result.afterChestCloseHash != 0);
    PROBE_ASSERT("hash.after_forward.nonzero", result.afterForwardHash != 0);
    PROBE_ASSERT("hash.after_wheel.nonzero", result.afterWheelHash != 0);
    PROBE_ASSERT("hash.accept_changes", result.afterAcceptHash !=
                                             result.beforeHash);
    PROBE_ASSERT("hash.close_changes", result.afterChestCloseHash !=
                                            result.afterAcceptHash);
    PROBE_ASSERT("hash.forward_changes", result.afterForwardHash !=
                                              result.afterChestCloseHash);
    PROBE_ASSERT("hash.wheel_changes", result.afterWheelHash !=
                                            result.afterForwardHash);
    PROBE_ASSERT_U32("hash.result_matches", result.hash,
                     dm1_v1_mirror_candidate_resurrect_chest_close_order_hash_pc34(
                         &state));
    *out_hash = result.hash;
}

static void test_guards(void)
{
    Dm1V1MirrorCandidateResurrectChestCloseOrderStatePc34 state =
        dm1_v1_mirror_candidate_resurrect_chest_close_order_default_state_pc34();
    Dm1V1MirrorCandidateResurrectChestCloseOrderResultPc34 result;

    PROBE_ASSERT_EQ("guard.null_state",
                    dm1_v1_mirror_candidate_resurrect_chest_close_order_run_pc34(
                        0, &result),
                    0);
    PROBE_ASSERT_EQ("guard.null_result",
                    dm1_v1_mirror_candidate_resurrect_chest_close_order_run_pc34(
                        &state, 0),
                    0);
    state =
        dm1_v1_mirror_candidate_resurrect_chest_close_order_default_state_pc34();
    state.contractOnly = 0;
    PROBE_ASSERT_EQ("guard.non_contract",
                    dm1_v1_mirror_candidate_resurrect_chest_close_order_run_pc34(
                        &state, &result),
                    0);
    state =
        dm1_v1_mirror_candidate_resurrect_chest_close_order_default_state_pc34();
    state.g0299CandidateOrdinal = 0;
    PROBE_ASSERT_EQ("guard.no_candidate",
                    dm1_v1_mirror_candidate_resurrect_chest_close_order_run_pc34(
                        &state, &result),
                    0);
    state =
        dm1_v1_mirror_candidate_resurrect_chest_close_order_default_state_pc34();
    state.g0426OpenChestThing = DM1_V1_MC_RCCO_NONE_PC34;
    PROBE_ASSERT_EQ("guard.closed_chest",
                    dm1_v1_mirror_candidate_resurrect_chest_close_order_run_pc34(
                        &state, &result),
                    0);
}

int main(void)
{
    uint32_t hash = 0;

    test_source_evidence();
    test_default_state();
    test_ordered_regression(&hash);
    test_guards();

    if (g_failures != 0 || g_assertions < 115) {
        printf("FAIL test_dm1_v1_mirror_candidate_resurrect_chest_close_order_pc34_compat assertions=%d failures=%d hash=0x%08X\n",
               g_assertions, g_failures, hash);
        return 1;
    }
    printf("PASS test_dm1_v1_mirror_candidate_resurrect_chest_close_order_pc34_compat assertions=%d failures=0 hash=0x%08X\n",
           g_assertions, hash);
    return 0;
}
