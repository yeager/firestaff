#include "firestaff/dm1/v1/mirror_candidate/c161_cancel_after_f0334_pending_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_passed;

static void check_true(const char *id, int condition)
{
    ++g_assertions;
    if (condition) {
        ++g_passed;
    } else {
        printf("FAIL %s\n", id);
    }
}

static void check_int(const char *id, int got, int want)
{
    ++g_assertions;
    if (got == want) {
        ++g_passed;
    } else {
        printf("FAIL %s got=%d want=%d\n", id, got, want);
    }
}

static void check_u32(const char *id, uint32_t got, uint32_t want)
{
    ++g_assertions;
    if (got == want) {
        ++g_passed;
    } else {
        printf("FAIL %s got=0x%08x want=0x%08x\n", id,
               (unsigned int)got, (unsigned int)want);
    }
}

static int contains(const char *haystack, const char *needle)
{
    return haystack && needle && strstr(haystack, needle) != 0;
}

static void check_source_evidence(void)
{
    const char *e =
        dm1_v1_mirror_c161_cancel_after_f0334_pending_source_evidence_pc34();

    check_true("evidence.present", e != 0);
    check_true("evidence.champion.f0297", contains(e, "F0297:243-268"));
    check_true("evidence.champion.f0298", contains(e, "F0298:270-298"));
    check_true("evidence.champion.f0300", contains(e, "F0300:485,564,575"));
    check_true("evidence.champion.f0301", contains(e, "F0301:606-660"));
    check_true("evidence.champion.f0302", contains(e, "F0302:662-714"));
    check_true("evidence.command.f0359",
               contains(e, "COMMAND.C F0359:1452-1662"));
    check_true("evidence.command.f0360", contains(e, "F0360:1692-1707"));
    check_true("evidence.command.f0378", contains(e, "F0378:1956-1993"));
    check_true("evidence.command.f0380", contains(e, "F0380:2045-2178"));
    check_true("evidence.chest.f0333",
               contains(e, "CHEST.C F0333:30-67"));
    check_true("evidence.chest.f0334", contains(e, "F0334:79-130"));
    check_true("evidence.mouse.f0077",
               contains(e, "MOUSE.C F0077:1-32"));
    check_true("evidence.mouse.f0078", contains(e, "F0078:33-64"));
    check_true("evidence.panel.f0344",
               contains(e, "PANEL.C F0344:1493-1561"));
    check_true("evidence.panel.f0345", contains(e, "F0345:1563-1617"));
    check_true("evidence.panel.f0346", contains(e, "F0346:1619-1637"));
    check_true("evidence.panel.f0347", contains(e, "F0347:1639-1693"));
    check_true("evidence.panel.f0355", contains(e, "F0355:2280-2440"));
    check_true("evidence.revive.f0280",
               contains(e, "REVIVE.C F0280:124-132"));
    check_true("evidence.revive.f0282", contains(e, "F0282:744-806"));
    check_true("evidence.revive.f0286",
               contains(e, "F0286 statistics-reset"));
    check_true("evidence.defs.c040", contains(e, "C040"));
    check_true("evidence.defs.c045", contains(e, "C045"));
    check_true("evidence.defs.c160", contains(e, "C160"));
    check_true("evidence.defs.c161", contains(e, "C161"));
    check_true("evidence.defs.c162", contains(e, "C162"));
    check_true("evidence.defs.c537", contains(e, "C537..C544"));
    check_true("evidence.defs.g0299", contains(e, "G0299"));
    check_true("evidence.defs.g0424", contains(e, "G0424"));
    check_true("evidence.defs.g0425", contains(e, "G0425"));
    check_true("evidence.defs.g0426", contains(e, "G0426"));
    check_true("evidence.defs.m568", contains(e, "M568"));
    check_true("evidence.defs.m070", contains(e, "M070"));
    check_true("evidence.defs.m516", contains(e, "M516"));
    check_true("evidence.contract", contains(e, "contract-only=1"));
    check_true("evidence.no_game_data", contains(e, "no_game_data=1"));
}

static void check_source_lock_metadata(void)
{
    const Dm1V1MirrorC161AfterF0334SourceLockPc34 *m =
        dm1_v1_mirror_c161_cancel_after_f0334_pending_source_lock_pc34();

    check_true("metadata.present", m != 0);
    check_int("metadata.contract_only", m ? m->contractOnly : 0, 1);
    check_int("metadata.no_game_data", m ? m->noGameData : 0, 1);
    check_int("metadata.opcode_count", m ? m->opcodeCount : 0, 12);
    check_true("metadata.opcodes", m && m->opcodes != 0);
    check_true("metadata.anchors", m && m->anchors != 0);
    check_int("metadata.anchor_count", m ? m->anchorCount : 0,
              DM1_V1_MIRROR_C161_AFTER_F0334_ANCHOR_COUNT_PC34);
    check_int("metadata.op0", m ? (int)m->opcodes[0] : 0,
              DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0359_QUEUE_PC34);
    check_int("metadata.op4", m ? (int)m->opcodes[4] : 0,
              DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0077_BRACKET_OPEN_PC34);
    check_int("metadata.op5", m ? (int)m->opcodes[5] : 0,
              DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0282_C161_CANCEL_PC34);
    check_int("metadata.op11", m ? (int)m->opcodes[11] : 0,
              DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0344_WATER_READ_PC34);
}

static void check_default_state(void)
{
    Dm1V1MirrorC161AfterF0334StatePc34 state =
        dm1_v1_mirror_c161_cancel_after_f0334_pending_default_state_pc34();
    int i;

    check_int("default.contract_only", state.contractOnly, 1);
    check_int("default.no_game_data", state.noGameData, 1);
    check_int("default.f0334", state.f0334CloseFired, 1);
    check_int("default.rewired", state.chestCloseRewiredG0425, 1);
    check_int("default.g0426", state.g0426OpenChest, 0);
    for (i = 0; i < DM1_V1_MIRROR_C161_AFTER_F0334_CHEST_SLOT_COUNT_PC34;
         ++i) {
        check_int("default.g0425.clear", state.g0425ChestSlots[i], 0);
        check_true("default.container.rewired", state.chestContainerSlots[i] != 0u);
    }
    check_int("default.pending", state.resurrectPending, 1);
    check_int("default.owner", state.candidateOwnerIndex, 2);
    check_int("default.health", state.candidateCurrentHealth, 0);
    check_int("default.food", state.candidateFood, 0);
    check_int("default.water", state.candidateWater, 0);
    check_int("default.g0299", state.g0299CandidateOrdinal, 3);
    check_int("default.c040.open", state.c040PanelOpen, 1);
    check_int("default.c040.graphic", state.c040PanelGraphic, 40);
    check_int("default.c040.command", state.c040PanelCommand, 568);
    check_int("default.panel.m568", state.panelContentM568, 568);
    check_int("default.hand.type", state.leaderHand.itemType,
              state.leaderHandBeforeClose.itemType);
    check_int("default.hand.weight", state.leaderHand.weight,
              state.leaderHandBeforeClose.weight);
    check_int("default.hand.charges", state.leaderHand.charges,
              state.leaderHandBeforeClose.charges);
}

static uint32_t check_success_path(void)
{
    Dm1V1MirrorC161AfterF0334StatePc34 state =
        dm1_v1_mirror_c161_cancel_after_f0334_pending_default_state_pc34();
    Dm1V1MirrorC161AfterF0334ResultPc34 result;
    uint32_t recomputed;
    int ok;
    int i;

    ok = dm1_v1_mirror_c161_cancel_after_f0334_pending_run_pc34(&state,
                                                                 &result);
    recomputed =
        dm1_v1_mirror_c161_cancel_after_f0334_pending_hash_pc34(&result);
    check_int("run.accepted", ok, 1);
    check_int("result.accepted", result.accepted, 1);
    check_int("result.reject", result.rejectCode,
              DM1_V1_MIRROR_C161_AFTER_F0334_REJECT_NONE_PC34);
    check_int("result.opcode_count", result.opcodeCount, 12);
    check_int("count.f0359", result.f0359QueueCount, 1);
    check_int("count.f0360", result.f0360RouteClickCount, 1);
    check_int("count.f0378", result.f0378RouteCount, 1);
    check_int("count.f0380", result.f0380DrainCount, 1);
    check_int("count.f0282", result.f0282C161CancelCount, 1);
    check_int("count.f0286.reject", result.f0286StatisticsResetRejectCount, 1);
    check_int("count.f0286.call", result.f0286StatisticsResetCallCount, 0);
    check_int("count.f0077", result.f0077BracketOpenCount, 1);
    check_int("count.f0078", result.f0078BracketCloseCount, 1);
    check_int("count.f0457", result.f0457DrawEnabledMenusCount, 1);
    check_int("count.f0347", result.f0347PanelRedrawCount, 1);
    check_int("count.f0344.reads", result.f0344FoodWaterReadCount, 2);
    check_int("clear.g0299", result.g0299Cleared, 1);
    check_int("clear.pending", result.resurrectPendingCleared, 1);
    check_int("clear.m568", result.m568Cleared, 1);
    check_int("clear.c040", result.c040Cleared, 1);
    check_int("candidate.dead", result.candidateStayedDead, 1);
    check_int("candidate.foodwater.zero", result.candidateFoodWaterStayedZero,
              1);
    check_int("hand.stable.cancel", result.leaderHandStableAcrossCancel, 1);
    check_int("hand.stable.close", result.leaderHandStableAcrossClose, 1);
    check_int("chest.rewired.before.cancel",
              result.chestSlotsRewiredBeforeCancel, 1);
    check_int("state.pending.final", state.resurrectPending, 0);
    check_int("state.g0299.final", state.g0299CandidateOrdinal, 0);
    check_int("state.panel.final", state.panelContentM568, 0);
    check_int("state.c040.open.final", state.c040PanelOpen, 0);
    check_int("state.c040.graphic.final", state.c040PanelGraphic, 0);
    check_int("state.c040.command.final", state.c040PanelCommand, 0);
    check_int("state.health.final", state.candidateCurrentHealth, 0);
    check_int("state.food.final", state.candidateFood, 0);
    check_int("state.water.final", state.candidateWater, 0);
    check_int("state.health.not_f0286", state.candidateCurrentHealth == 30, 0);
    check_int("state.food.not_f0286", state.candidateFood == 2300, 0);
    check_int("state.water.not_f0286", state.candidateWater == 2100, 0);
    check_int("state.hand.type", state.leaderHand.itemType,
              state.leaderHandBeforeClose.itemType);
    check_int("state.hand.weight", state.leaderHand.weight,
              state.leaderHandBeforeClose.weight);
    check_int("state.hand.charges", state.leaderHand.charges,
              state.leaderHandBeforeClose.charges);
    check_int("trace.0", result.trace[0].opcode,
              DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0359_QUEUE_PC34);
    check_int("trace.1", result.trace[1].opcode,
              DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0360_ROUTE_CLICK_PC34);
    check_int("trace.2", result.trace[2].opcode,
              DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0378_ROUTE_PANEL_PC34);
    check_int("trace.3", result.trace[3].opcode,
              DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0380_DRAIN_PC34);
    check_int("trace.4", result.trace[4].opcode,
              DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0077_BRACKET_OPEN_PC34);
    check_int("trace.5", result.trace[5].opcode,
              DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0282_C161_CANCEL_PC34);
    check_int("trace.6", result.trace[6].opcode,
              DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0286_REJECT_PC34);
    check_int("trace.7", result.trace[7].opcode,
              DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0457_DRAW_ENABLED_MENUS_PC34);
    check_int("trace.8", result.trace[8].opcode,
              DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0078_BRACKET_CLOSE_PC34);
    check_int("trace.9", result.trace[9].opcode,
              DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0347_PANEL_REDRAW_PC34);
    check_int("trace.10", result.trace[10].opcode,
              DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0344_FOOD_READ_PC34);
    check_int("trace.11", result.trace[11].opcode,
              DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0344_WATER_READ_PC34);
    for (i = 0; i < result.opcodeCount; ++i) {
        check_int("trace.health.byte_stable",
                  result.trace[i].candidateCurrentHealth, 0);
        check_int("trace.food.byte_stable", result.trace[i].candidateFood, 0);
        check_int("trace.water.byte_stable", result.trace[i].candidateWater, 0);
        check_int("trace.hand.type.byte_stable",
                  result.trace[i].leaderHand.itemType,
                  state.leaderHandBeforeClose.itemType);
        check_int("trace.hand.weight.byte_stable",
                  result.trace[i].leaderHand.weight,
                  state.leaderHandBeforeClose.weight);
        check_int("trace.hand.charges.byte_stable",
                  result.trace[i].leaderHand.charges,
                  state.leaderHandBeforeClose.charges);
    }
    check_true("hash.nonzero", result.hash != 0u);
    check_u32("hash.recomputed", recomputed, result.hash);
    return result.hash;
}

static void check_reject(const char *id,
                         Dm1V1MirrorC161AfterF0334StatePc34 state,
                         Dm1V1MirrorC161AfterF0334RejectPc34 want)
{
    Dm1V1MirrorC161AfterF0334ResultPc34 result;
    int beforeHealth = state.candidateCurrentHealth;
    int beforeFood = state.candidateFood;
    int beforeWater = state.candidateWater;
    uint16_t beforeG0299 = state.g0299CandidateOrdinal;
    int ok;

    ok = dm1_v1_mirror_c161_cancel_after_f0334_pending_run_pc34(&state,
                                                                 &result);
    check_int(id, ok, 0);
    check_int("reject.code", result.rejectCode, want);
    check_int("reject.no_op.opcodes", result.opcodeCount, 0);
    check_int("reject.no_f0282", result.f0282C161CancelCount, 0);
    check_int("reject.no_f0286", result.f0286StatisticsResetCallCount, 0);
    check_int("reject.health.stable", state.candidateCurrentHealth,
              beforeHealth);
    check_int("reject.food.stable", state.candidateFood, beforeFood);
    check_int("reject.water.stable", state.candidateWater, beforeWater);
    check_int("reject.g0299.stable", state.g0299CandidateOrdinal, beforeG0299);
}

static void check_reject_paths(void)
{
    Dm1V1MirrorC161AfterF0334StatePc34 state;

    state =
        dm1_v1_mirror_c161_cancel_after_f0334_pending_default_state_pc34();
    state.g0299CandidateOrdinal = 0u;
    state.candidateOwnerIndex = -1;
    check_reject("reject.no_candidate", state,
                 DM1_V1_MIRROR_C161_AFTER_F0334_REJECT_NO_CANDIDATE_PC34);

    state =
        dm1_v1_mirror_c161_cancel_after_f0334_pending_default_state_pc34();
    state.candidateCurrentHealth = 30;
    state.candidateFood = 2300;
    state.candidateWater = 2100;
    check_reject("reject.alive_candidate", state,
                 DM1_V1_MIRROR_C161_AFTER_F0334_REJECT_ALIVE_CANDIDATE_PC34);

    state =
        dm1_v1_mirror_c161_cancel_after_f0334_pending_default_state_pc34();
    state.resurrectPending = 0;
    check_reject("reject.no_resurrect_pending", state,
                 DM1_V1_MIRROR_C161_AFTER_F0334_REJECT_NO_RESURRECT_PENDING_PC34);

    state =
        dm1_v1_mirror_c161_cancel_after_f0334_pending_default_state_pc34();
    state.g0426OpenChest = 0x4242u;
    state.g0425ChestSlots[0] = 0xc537u;
    check_reject("reject.chest_open", state,
                 DM1_V1_MIRROR_C161_AFTER_F0334_REJECT_CHEST_STILL_OPEN_PC34);

    state =
        dm1_v1_mirror_c161_cancel_after_f0334_pending_default_state_pc34();
    state.f0334CloseFired = 0;
    state.chestCloseRewiredG0425 = 0;
    state.g0425ChestSlots[0] = 0xc537u;
    state.chestContainerSlots[0] = 0u;
    check_reject("reject.f0334_not_fired", state,
                 DM1_V1_MIRROR_C161_AFTER_F0334_REJECT_F0334_NOT_FIRED_PC34);
}

int main(void)
{
    uint32_t hash;

    printf("probe=firestaff_dm1_v1_mirror_c161_cancel_after_f0334_pending\n");
    printf("%s\n",
           dm1_v1_mirror_c161_cancel_after_f0334_pending_source_evidence_pc34());
    check_source_evidence();
    check_source_lock_metadata();
    check_default_state();
    hash = check_success_path();
    check_reject_paths();
    printf("deterministic_hash=0x%08x\n", (unsigned int)hash);
    printf("summary: %d/%d assertions passed\n", g_passed, g_assertions);
    if (g_passed != g_assertions) {
        return 1;
    }
    return 0;
}
