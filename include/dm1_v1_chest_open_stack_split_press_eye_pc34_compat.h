#ifndef FIRESTAFF_DM1_V1_CHEST_OPEN_STACK_SPLIT_PRESS_EYE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_OPEN_STACK_SPLIT_PRESS_EYE_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * pass799 — chest open-stack-split press-eye path.
 *
 * Source-locked deterministic contract gate only; no real-asset pixel or
 * icon parity claim. The slice is the press-eye F0333 auto-close gate
 * (P0694_B_PressingEye = TRUE) where the chest contents include a
 * stack (two of the same itemType in adjacent G0425 slots) and the
 * leader's C09 action hand (DM1_PC34_SLOT_ACTION_HAND) click on one
 * stack member picks up only the slot contents (not the whole stack);
 * F0334 then closes the remaining items in link-order, preserving
 * the stack-pattern observation in the closed list and during reopen.
 *
 * Non-duplicative: pass797 covers the press-eye + C09 action-hand
 * put-replacement scenario (replacement container dropped into a
 * not-yet-visited slot mid-close). pass799 covers the press-eye + C09
 * action-hand pick-from-stack scenario (one stack member picked up
 * mid-close, the remaining stack member is linked in close-order).
 * The C09 slot-box is C09_SLOT_BOX_INVENTORY_ACTION_HAND on PC 3.4.
 */

enum {
    DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT =
        DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_CHEST_OPEN_STACK_SPLIT_FIRST_ITEM = 0x7F10,
    DM1_PC34_CHEST_OPEN_STACK_SPLIT_CHEST_THING = 0x7F00,
    DM1_PC34_CHEST_OPEN_STACK_SPLIT_STACK_A = 0x7F30,
    DM1_PC34_CHEST_OPEN_STACK_SPLIT_STACK_B = 0x7F31,
    DM1_PC34_CHEST_OPEN_STACK_SPLIT_REOPEN_THING = 0x7F01,
    DM1_PC34_CHEST_OPEN_STACK_SPLIT_PREFIX_COUNT = 2,
    DM1_PC34_CHEST_OPEN_STACK_SPLIT_STACK_INDEX = 4,
    DM1_PC34_CHEST_OPEN_STACK_SPLIT_PICKUP_INDEX = 3,
    DM1_PC34_CHEST_OPEN_STACK_SPLIT_ENDOFLIST = 0xFFFE,
    DM1_PC34_CHEST_OPEN_STACK_SPLIT_NONE = 0xFFFF
};

typedef struct {
    const char* contractMarker;
    const char* chosenBecause;
    int contractOnly;
    int chestSlotCount;
    int prefixCount;
    int stackIndex;
    int stackPc34Slot;
    int pickupIndex;
    int pickupPc34Slot;
    int chestThing;
    int firstItemType;
    int stackAItemType;
    int stackBItemType;
    int pressEyePath;
    int endOfListSentinel;
    int noneSentinel;
    int actionHandC09SlotIndex;
} DM1_V1_ChestOpenStackSplitPressEyeSpecPc34;

typedef struct {
    int contractOnly;

    /* press-eye open + stack-pattern in G0425. */
    int pressEyePathEnabled;
    int pressEyeIconBlitSkipped;
    int openResult;
    int openThing;
    int openVisibleCount;
    int openedTypes[DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT];
    int openedWeights[DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT];
    int openedOrderMatchesInput;
    int stackPairAdjacent;

    /* C09 action-hand before close: empty (the leader will pick up
     * the stack member from the chest, not put a replacement). */
    int actionHandBeforeCloseType;
    int actionHandBeforeCloseEmpty;
    int pickupAllowedByActionHand;

    /* F0334 begin. */
    int closeBeginResult;
    int openThingBeforeCloseBegin;
    int openThingAfterCloseBegin;
    int containerSlotClearedToEnd;
    int containerSlotEndOfListTerminator;

    /* F0334 close prefix. */
    int prefixProcessedCount;
    int prefixClosedCount;
    int prefixClosedTypes[DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT];
    int prefixSlotsCleared[DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT];
    int prefixLastLinkedType;
    int prefixEndOfListTerminator;

    /* C09 action-hand click on the stack member slot (the pickup). */
    int midPickupPc34Slot;
    int midPickupSourcePc34Slot;
    int midPickupResult;
    int midPickupLeaderBeforeType;
    int midPickupSlotBeforeType;
    int midPickupLeaderRemovedCall;
    int midPickupSlotRemovedCall;
    int midPickupPutSlotInLeaderCall;
    int midPickupPutLeaderInSlotCall;
    int midPickupLeaderAfterType;
    int midPickupSlotAfterType;
    int midPickupLeaderEmptyAfterPickup;
    int pickupSourceAbsentFromSlotAfterPickup;

    /* F0334 close remainder. */
    int closeCount;
    int closedTypes[DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT];
    int closedWeights[DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT];
    int stackBLinkedAtStackPosition;
    int stackAAbsentFromClosedLinks;
    int pickupAbsentFromClosedLinks;
    int closedOrderPreservesStackPattern;
    int pickupSlotClearedByClose;
    int stackSlotClearedByClose;
    int allSlotsEmptyAfterClose;
    int actionHandAfterCloseType;
    int actionHandAfterCloseWeight;
    int actionHandAfterCloseEmpty;
    int closedEndOfListTerminator;

    /* Reopen via F0333 press-eye again. */
    int reopenResult;
    int reopenThing;
    int reopenedVisibleCount;
    int reopenedTypes[DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT];
    int reopenedOrderMatchesClosed;
    int stackBVisibleAfterReopen;
    int stackAAbsentAfterReopen;
    int pickupAbsentAfterReopen;
} DM1_V1_ChestOpenStackSplitPressEyeProbePc34;

const char*
dm1_v1_chest_open_stack_split_press_eye_source_evidence_pc34(void);

const DM1_V1_ChestOpenStackSplitPressEyeSpecPc34*
dm1_v1_chest_open_stack_split_press_eye_spec_pc34(void);

int dm1_v1_chest_open_stack_split_press_eye_run_pc34(
    DM1_V1_ChestOpenStackSplitPressEyeProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_OPEN_STACK_SPLIT_PRESS_EYE_PC34_COMPAT_H */
