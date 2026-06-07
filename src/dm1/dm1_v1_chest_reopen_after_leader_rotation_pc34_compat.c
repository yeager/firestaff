#include "dm1_v1_chest_reopen_after_leader_rotation_pc34_compat.h"

#include <string.h>

enum {
    DM1_REOPEN_ROTATION_OPEN_NONE =
        DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_THING_NONE
};

static const char s_source_evidence[] =
    "CHEST.C F0333:31-67 copies visible chest links into G0425 and sets G0426\n"
    "CHEST.C F0334:113-132 clears G0426 and rewrites visible G0425 returns\n"
    "CHAMPION.C F0297:243-298 and F0298 preserve leader-hand thing identity\n"
    "CHAMPION.C F0300/F0301/F0302 keep slot swaps separate from the hand thing\n"
    "DUNGEON.C F0163:1796-1837 clears Next and appends visible-input returns\n"
    "COMMAND.C F0359 M568/C040 dispatch can rotate party/leader state\n"
    "DEFS.H:2088/C30/G0425/G0426/G0423/G0305 define sentinels and ordinals";

static const char* const s_case_names[
    DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_CASE_COUNT] = {
    "close chest A, rotate to new leader, reopen chest B",
    "non-empty leader hand survives leader rotation",
    "rotate A to B to A before reopening chest B",
    "hidden-tail leader hand survives mid-close rotation",
    "chest B close with full leader hand then reopen",
    "empty leader-hand no-op state survives rotation"
};

static void clear_case(
    M11_GameView_ChestReopenAfterLeaderRotationCasePc34* out)
{
    memset(out, 0, sizeof(*out));
}

static void fill_visible(int* slots, int firstThing)
{
    int i;

    for (i = 0;
         i < DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_SLOT_COUNT;
         ++i) {
        slots[i] = firstThing + i;
    }
}

static void copy_slots(int* dst, const int* src)
{
    int i;

    for (i = 0;
         i < DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_SLOT_COUNT;
         ++i) {
        dst[i] = src[i];
    }
}

static int count_visible(const int* slots)
{
    int count = 0;
    int i;

    for (i = 0;
         i < DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_SLOT_COUNT;
         ++i) {
        if (slots[i] != DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_THING_NONE) {
            ++count;
        }
    }
    return count;
}

static void append_action(
    M11_GameView_ChestReopenAfterLeaderRotationCasePc34* out,
    M11_GameView_ChestReopenAfterLeaderRotationActionPc34 action,
    int chestThing,
    int openChestThing,
    int leaderOrdinal,
    int inventoryChampionOrdinal,
    int leaderHandThing)
{
    M11_GameView_ChestReopenAfterLeaderRotationLogEntryPc34* entry;

    if (out->actionLog.count >=
        DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_LOG_CAPACITY) {
        return;
    }
    entry = &out->actionLog.entries[out->actionLog.count++];
    entry->action = action;
    entry->chestThing = chestThing;
    entry->openChestThing = openChestThing;
    entry->leaderOrdinal = leaderOrdinal;
    entry->inventoryChampionOrdinal = inventoryChampionOrdinal;
    entry->leaderHandThing = leaderHandThing;
}

static void configure_common_context(
    M11_GameView_ChestReopenAfterLeaderRotationCasePc34* out,
    int caseIndex,
    int leaderHandThing,
    int chestBFirstThing)
{
    int i;

    out->caseIndex = caseIndex;
    out->caseName = s_case_names[caseIndex];
    out->context.chestAThing =
        DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_CHEST_A + caseIndex * 2;
    out->context.chestBThing =
        DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_CHEST_B + caseIndex * 2;
    out->context.partyChampionCount =
        DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_PARTY_COUNT;
    for (i = 0;
         i < DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_PARTY_COUNT;
         ++i) {
        out->context.partyRosterOrdinals[i] = i + 1;
    }
    out->context.originalLeaderOrdinal = 1;
    out->context.originalInventoryChampionOrdinal = 1;
    out->context.currentLeaderOrdinalAfterRotation = 2;
    out->context.leaderHandThing = leaderHandThing;
    fill_visible(out->context.chestAVisibleSlots, 0x4100 + caseIndex * 0x20);
    fill_visible(out->context.chestBVisibleSlots, chestBFirstThing);
    out->context.chestBHiddenTail =
        DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_THING_NONE;
}

static void run_contract_trace(
    M11_GameView_ChestReopenAfterLeaderRotationCasePc34* out,
    int rotateBackBeforeReopen,
    int rotateDuringChestBClose)
{
    int leaderOrdinal = out->context.originalLeaderOrdinal;
    int inventoryOrdinal = out->context.originalInventoryChampionOrdinal;
    int handThing = out->context.leaderHandThing;

    /* ReDMCSB CHEST.C F0333:31-67 materializes chest A into G0425/G0426. */
    append_action(out,
                  M11_DM1_PC34_CHEST_REOPEN_LEADER_ROTATION_ACTION_OPEN_CHEST_A,
                  out->context.chestAThing, out->context.chestAThing,
                  leaderOrdinal, inventoryOrdinal, handThing);

    /* ReDMCSB CHEST.C F0334:113-132 recompacts chest A visible slots. */
    append_action(
        out,
        M11_DM1_PC34_CHEST_REOPEN_LEADER_ROTATION_ACTION_CLOSE_CHEST_A,
        out->context.chestAThing, DM1_REOPEN_ROTATION_OPEN_NONE,
        leaderOrdinal, inventoryOrdinal, handThing);
    out->expected.closeCountChestA =
        count_visible(out->context.chestAVisibleSlots);
    out->expected.chestALinkHead = out->context.chestAVisibleSlots[0];

    /* ReDMCSB COMMAND.C F0359 M568/C040 can change the leader ordinal while
     * G4055_s_LeaderHandObject remains a distinct hand object. */
    leaderOrdinal = out->context.currentLeaderOrdinalAfterRotation;
    out->expected.rotationCount = 1;
    append_action(
        out,
        M11_DM1_PC34_CHEST_REOPEN_LEADER_ROTATION_ACTION_ROTATE_LEADER,
        DM1_REOPEN_ROTATION_OPEN_NONE, DM1_REOPEN_ROTATION_OPEN_NONE,
        leaderOrdinal, inventoryOrdinal, handThing);
    copy_slots(out->expected.slotsAfterRotation,
               out->context.chestBVisibleSlots);

    append_action(out,
                  M11_DM1_PC34_CHEST_REOPEN_LEADER_ROTATION_ACTION_OPEN_CHEST_B,
                  out->context.chestBThing, out->context.chestBThing,
                  leaderOrdinal, inventoryOrdinal, handThing);

    if (rotateDuringChestBClose) {
        leaderOrdinal = 3;
        out->expected.rotationCount++;
        append_action(
            out,
            M11_DM1_PC34_CHEST_REOPEN_LEADER_ROTATION_ACTION_ROTATE_LEADER,
            out->context.chestBThing, out->context.chestBThing,
            leaderOrdinal, inventoryOrdinal, handThing);
    }

    append_action(
        out,
        M11_DM1_PC34_CHEST_REOPEN_LEADER_ROTATION_ACTION_CLOSE_CHEST_B,
        out->context.chestBThing, DM1_REOPEN_ROTATION_OPEN_NONE,
        leaderOrdinal, inventoryOrdinal, handThing);

    if (rotateBackBeforeReopen) {
        leaderOrdinal = out->context.originalLeaderOrdinal;
        out->expected.rotationCount++;
        append_action(
            out,
            M11_DM1_PC34_CHEST_REOPEN_LEADER_ROTATION_ACTION_ROTATE_LEADER_BACK,
            DM1_REOPEN_ROTATION_OPEN_NONE, DM1_REOPEN_ROTATION_OPEN_NONE,
            leaderOrdinal, inventoryOrdinal, handThing);
    }

    append_action(
        out,
        M11_DM1_PC34_CHEST_REOPEN_LEADER_ROTATION_ACTION_REOPEN_CHEST_B,
        out->context.chestBThing, out->context.chestBThing,
        leaderOrdinal, inventoryOrdinal, handThing);

    out->expected.closeCountChestB =
        count_visible(out->context.chestBVisibleSlots);
    out->expected.reopenedVisibleCount = out->expected.closeCountChestB;
    out->expected.chestBLinkHead = out->context.chestBVisibleSlots[0];
    out->expected.leaderHandAfter = handThing;
    copy_slots(out->expected.visibleSlotOrderOnReopen,
               out->context.chestBVisibleSlots);
    out->expected.finalOpenChestThing = out->context.chestBThing;
    out->expected.finalLeaderOrdinal = leaderOrdinal;
    out->expected.finalInventoryChampionOrdinal = inventoryOrdinal;
    out->expected.hiddenTailAfter =
        handThing ==
            DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_HIDDEN_TAIL ?
        handThing : out->context.chestBHiddenTail;
    out->expected.noDetachedC30PlusOccupant = 1;
    out->expected.leaderHandIdentityPreserved =
        handThing == out->expected.leaderHandAfter ? 1 : 0;
    out->expected.chestBVisibleOrderPreserved = 1;
    out->expected.hiddenTailStaysWithLeader =
        handThing ==
        DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_HIDDEN_TAIL ? 1 : 0;
    out->expected.fullLeaderHandStillFull =
        handThing !=
        DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_THING_NONE ? 1 : 0;
    out->expected.emptyLeaderHandNoopPreserved =
        handThing ==
        DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_THING_NONE ? 1 : 0;
}

const char*
M11_GameView_ChestReopenAfterLeaderRotationSourceEvidencePc34(void)
{
    return s_source_evidence;
}

const char* M11_GameView_ChestReopenAfterLeaderRotationCaseNamePc34(
    int caseIndex)
{
    if (caseIndex < 0 ||
        caseIndex >= DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_CASE_COUNT) {
        return "";
    }
    return s_case_names[caseIndex];
}

int M11_GameView_ChestReopenAfterLeaderRotationBuildCasePc34(
    int caseIndex,
    M11_GameView_ChestReopenAfterLeaderRotationCasePc34* out)
{
    if (!out || caseIndex < 0 ||
        caseIndex >= DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_CASE_COUNT) {
        return 0;
    }

    clear_case(out);
    switch (caseIndex) {
    case DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_CASE_BASIC:
        configure_common_context(
            out, caseIndex,
            DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_THING_NONE,
            0x5100);
        run_contract_trace(out, 0, 0);
        break;
    case DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_CASE_FULL_HAND:
        configure_common_context(
            out, caseIndex,
            DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_LEADER_AMULET,
            0x5200);
        run_contract_trace(out, 0, 0);
        break;
    case DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_CASE_DOUBLE_ROTATE:
        configure_common_context(
            out, caseIndex,
            DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_THING_NONE,
            0x5300);
        run_contract_trace(out, 1, 0);
        break;
    case DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_CASE_HIDDEN_TAIL_HAND:
        configure_common_context(
            out, caseIndex,
            DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_HIDDEN_TAIL,
            0x5400);
        run_contract_trace(out, 0, 1);
        break;
    case DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_CASE_CLOSE_FULL_HAND:
        configure_common_context(
            out, caseIndex,
            DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_LEADER_SHIELD,
            0x5500);
        run_contract_trace(out, 0, 0);
        break;
    case DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_CASE_EMPTY_NOOP:
        configure_common_context(
            out, caseIndex,
            DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_THING_NONE,
            0x5600);
        run_contract_trace(out, 0, 0);
        break;
    default:
        return 0;
    }

    return 1;
}

int M11_GameView_ChestReopenAfterLeaderRotationRunPc34(
    M11_GameView_ChestReopenAfterLeaderRotationProbePc34* out)
{
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    out->sourceLockedContractOnly = 1;
    out->c0xFFFFThingNone =
        DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_THING_NONE;
    out->c537Pc34Slot = DM1_PC34_SLOT_CHEST_1;
    out->c544Pc34Slot = DM1_PC34_SLOT_CHEST_8;
    out->chestSlotCount =
        DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_SLOT_COUNT;
    out->caseCount = DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_CASE_COUNT;

    for (i = 0;
         i < DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_CASE_COUNT;
         ++i) {
        if (!M11_GameView_ChestReopenAfterLeaderRotationBuildCasePc34(
                i, &out->cases[i])) {
            return 0;
        }
    }
    return 1;
}
