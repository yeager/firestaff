#ifndef FIRESTAFF_DM1_V1_AUTO_CHEST_ACTION_HAND_SWAP_DURING_CLOSE_GATE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_AUTO_CHEST_ACTION_HAND_SWAP_DURING_CLOSE_GATE_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * pass797 — auto chest action-hand swap during close gate.
 *
 * Source-locked deterministic contract gate only; no real-asset pixel or
 * icon parity claim. The slice is the press-eye auto-close path
 * (CHEST.C F0333 with P0694_B_PressingEye = TRUE) followed by the leader's
 * action hand (C09_SLOT_BOX_INVENTORY_ACTION_HAND,
 * DM1_PC34_SLOT_ACTION_HAND) holding a replacement container and that
 * container being put into a not-yet-visited G0425 slot mid-iteration of
 * F0334. The contract pins the G0425 link-order result, the F0334
 * already-visited-slot immutability, the C09 action-hand rewire, and the
 * C0xFFFE end-of-list terminator placement.
 *
 * Non-duplicative: tests/test_dm1_v1_chest_mid_close_hand_swap_pc34_compat.c
 * covers a manual F0333 open + a mid-close hand swap. pass797 adds the
 * press-eye auto-close gate (P0694_B_PressingEye = TRUE skips the
 * C145_ICON_CONTAINER_CHEST_OPEN blit), uses C09 (action hand) as the
 * swap source, and verifies the C0xFFFE end-of-list terminator.
 */

enum {
    DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT =
        DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_AUTO_CHEST_ACTION_HAND_FIRST_ITEM = 0x7E10,
    DM1_PC34_AUTO_CHEST_ACTION_HAND_CHEST_THING = 0x7E00,
    DM1_PC34_AUTO_CHEST_ACTION_HAND_REPLACEMENT_CONTAINER = 0x7E90,
    DM1_PC34_AUTO_CHEST_ACTION_HAND_REOPEN_THING = 0x7E01,
    DM1_PC34_AUTO_CHEST_ACTION_HAND_PREFIX_COUNT = 2,
    DM1_PC34_AUTO_CHEST_ACTION_HAND_FUTURE_INDEX = 5,
    DM1_PC34_AUTO_CHEST_ACTION_HAND_ALREADY_PROCESSED_INDEX = 0,
    DM1_PC34_AUTO_CHEST_ACTION_HAND_ENDOFLIST = 0xFFFE,
    DM1_PC34_AUTO_CHEST_ACTION_HAND_NONE = 0xFFFF
};

typedef struct {
    const char* contractMarker;
    const char* chosenBecause;
    int contractOnly;
    int chestSlotCount;
    int prefixCount;
    int futureIndex;
    int futurePc34Slot;
    int alreadyProcessedIndex;
    int alreadyProcessedPc34Slot;
    int chestThing;
    int firstItemType;
    int futureOriginalItemType;
    int replacementContainerType;
    int pressEyePath;
    int endOfListSentinel;
    int noneSentinel;
} DM1_V1_AutoChestActionHandSwapDuringCloseGateSpecPc34;

typedef struct {
    int contractOnly;

    /* press-eye open + auto-close gate result. */
    int pressEyePathEnabled;
    int pressEyeIconBlitSkipped;
    int openResult;
    int openThing;
    int openVisibleCount;
    int openedTypes[DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT];
    int openedWeights[DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT];
    int openedOrderMatchesInput;

    /* action-hand C09 contents (DM1_PC34_SLOT_ACTION_HAND). */
    int actionHandBeforeCloseType;
    int actionHandBeforeCloseWeight;
    int actionHandBeforeCloseAllowedSlots;
    int replacementCanEnterChest;

    /* F0334 begin-of-close state. */
    int closeBeginResult;
    int openThingBeforeCloseBegin;
    int openThingAfterCloseBegin;
    int containerSlotClearedToEnd;
    int containerSlotEndOfListTerminator;

    /* F0334 close prefix. */
    int prefixProcessedCount;
    int prefixClosedCount;
    int prefixClosedTypes[DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT];
    int prefixSlotsCleared[DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT];
    int prefixLastLinkedType;
    int prefixEndOfListTerminator;

    /* C09 action-hand mid-close swap into future slot. */
    int midSwapPc34Slot;
    int midSwapDestinationPc34Slot;
    int midSwapResult;
    int midSwapLeaderBeforeType;
    int midSwapSlotBeforeType;
    int midSwapLeaderRemovedCall;
    int midSwapSlotRemovedCall;
    int midSwapPutDisplacedInLeaderCall;
    int midSwapPutReplacementInSlotCall;
    int midSwapLeaderAfterType;
    int midSwapSlotAfterType;
    int futureOriginalAbsentFromSlotAfterSwap;
    int actionHandBecomesEmptyAfterSwap;

    /* F0334 close remainder. */
    int closeCount;
    int closedTypes[DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT];
    int closedWeights[DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT];
    int replacementLinkedAtFuturePosition;
    int displacedAbsentFromClosedLinks;
    int closedOrderUsesCurrentFutureSlot;
    int futureSlotClearedByClose;
    int allSlotsEmptyAfterClose;
    int actionHandAfterCloseType;
    int actionHandAfterCloseWeight;
    int closedEndOfListTerminator;

    /* Reopen via F0333 again. */
    int reopenResult;
    int reopenThing;
    int reopenedVisibleCount;
    int reopenedTypes[DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT];
    int reopenedOrderMatchesClosed;
    int replacementVisibleAfterReopen;
    int displacedAbsentAfterReopen;

    /* F0334 already-processed (visited) slot late write contract:
     * F0334 does not re-visit visited slots, so a late put into a
     * visited slot must be a no-op for the close-link list and a
     * F0334-visible stale write in the G0425 array. */
    int alreadyProcessedPc34Slot;
    int alreadyProcessedLeaderBeforeType;
    int alreadyProcessedSlotBeforeLateWriteType;
    int alreadyProcessedSwapResult;
    int alreadyProcessedSlotAfterLateWriteType;
    int alreadyProcessedCloseCount;
    int alreadyProcessedReplacementLinked;
    int alreadyProcessedLateWriteNotRevisited;
    int alreadyProcessedStaleSlotAfterCloseType;
    int alreadyProcessedLeaderAfterCloseType;

    /* Auto-close gate (press-eye) replay: F0334 from a second chest open
     * via press-eye must complete even when the action hand already
     * contains a container. */
    int secondPressEyeOpenResult;
    int secondPressEyeOpenThing;
    int secondCloseBeginResult;
    int secondActionHandBeforeCloseType;
    int secondCloseCount;
    int secondClosedEndOfListTerminator;
    int secondContainerSlotClearedToEnd;
} DM1_V1_AutoChestActionHandSwapDuringCloseGateProbePc34;

const char*
dm1_v1_auto_chest_action_hand_swap_during_close_gate_source_evidence_pc34(void);

const DM1_V1_AutoChestActionHandSwapDuringCloseGateSpecPc34*
dm1_v1_auto_chest_action_hand_swap_during_close_gate_spec_pc34(void);

int dm1_v1_auto_chest_action_hand_swap_during_close_gate_run_pc34(
    DM1_V1_AutoChestActionHandSwapDuringCloseGateProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_AUTO_CHEST_ACTION_HAND_SWAP_DURING_CLOSE_GATE_PC34_COMPAT_H */
