#ifndef FIRESTAFF_DM1_V1_CHEST_SCROLL_WHEEL_LEADER_STACK_ROTATION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_SCROLL_WHEEL_LEADER_STACK_ROTATION_PC34_COMPAT_H

/*
 * DM1 V1 chest scroll-wheel leader-stack rotation gate.
 *
 * ReDMCSB anchors used by this asset-free probe:
 * - CHEST.C F0333:30-67: same-open guard and C537..C544 G0425 fill.
 * - CHEST.C F0334:113-132: G0426 clear and non-empty visible-slot rewrite.
 * - CHAMPION.C F0297:243-268 and F0298:270-298: leader-hand put/remove,
 *   pointer/icon identity, load accounting, and mouse-update bracketing.
 * - CHAMPION.C F0301:606-614: C30+ writes through G0425 chest slots.
 * - CHAMPION.C F0302:662-710: C30+ reads, G4055 snapshot, and swap order.
 * - COMMAND.C F0378:1973-1983: chest panel click dispatch to F0302.
 * - PANEL.C F0354:2307-2344: inventory close path brackets
 *   F0334_INVENTORY_CloseChest with mouse update calls.
 * - UTAMSCR.C F0077:147-151 and F0078:141-145: pointer update wrappers.
 * - OBJECT.C F0033:147-212: visible icon identity.
 * - BLITMASK.C F0133:30-33: partial-mask presentation dispatch.
 * - DEFS.H:810-816 and 3906-3913: C30..C36 and C537..C544 constants.
 */

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_CHEST_LEADER_STACK_ROTATION_C30 = 30,
    DM1_V1_CHEST_LEADER_STACK_ROTATION_C537 = 537,
    DM1_V1_CHEST_LEADER_STACK_ROTATION_C540 = 540,
    DM1_V1_CHEST_LEADER_STACK_ROTATION_C541 = 541,
    DM1_V1_CHEST_LEADER_STACK_ROTATION_C542 = 542,
    DM1_V1_CHEST_LEADER_STACK_ROTATION_C544 = 544,
    DM1_V1_CHEST_LEADER_STACK_ROTATION_SLOT_COUNT = 8,
    DM1_V1_CHEST_LEADER_STACK_ROTATION_STACK_COUNT = 3,
    DM1_V1_CHEST_LEADER_STACK_ROTATION_WHEEL_STEPS = 4,
    DM1_V1_CHEST_LEADER_STACK_ROTATION_NONE = 0xFFFF,
    DM1_V1_CHEST_LEADER_STACK_ROTATION_END = 0xFFFE,
    DM1_V1_CHEST_LEADER_STACK_ROTATION_OPEN_CHEST = 0x7900,
    DM1_V1_CHEST_LEADER_STACK_ROTATION_ITEM0 = 0x7910,
    DM1_V1_CHEST_LEADER_STACK_ROTATION_ITEM1 = 0x7911,
    DM1_V1_CHEST_LEADER_STACK_ROTATION_ITEM2 = 0x7912,
    DM1_V1_CHEST_LEADER_STACK_ROTATION_LEADER0 = 0x7920,
    DM1_V1_CHEST_LEADER_STACK_ROTATION_LEADER1 = 0x7921,
    DM1_V1_CHEST_LEADER_STACK_ROTATION_LEADER2 = 0x7922,
    DM1_V1_CHEST_LEADER_STACK_ROTATION_LOAD_MASK = 0x0200
};

typedef struct {
    int thing;
    int icon;
    int weight;
} DM1_V1_ChestLeaderStackRotationItemPc34;

typedef struct {
    const char* sourceEvidence;
    const char* f0333OpenAnchor;
    const char* f0334CloseAnchor;
    const char* f0297PutAnchor;
    const char* f0298RemoveAnchor;
    const char* f0301WriteAnchor;
    const char* f0302DispatchAnchor;
    const char* commandDispatchAnchor;
    const char* panelCloseAnchor;
    const char* mouseAnchor;
    const char* objectAnchor;
    const char* blitMaskAnchor;
    const char* defsAnchor;
} DM1_V1_ChestLeaderStackRotationSpecPc34;

typedef struct {
    int openChestThing;
    int initialChest[DM1_V1_CHEST_LEADER_STACK_ROTATION_SLOT_COUNT];
    int chestAfterWheel[DM1_V1_CHEST_LEADER_STACK_ROTATION_WHEEL_STEPS]
                       [DM1_V1_CHEST_LEADER_STACK_ROTATION_SLOT_COUNT];
    int initialLeaderStack[DM1_V1_CHEST_LEADER_STACK_ROTATION_STACK_COUNT];
    int leaderStackAfterWheel[DM1_V1_CHEST_LEADER_STACK_ROTATION_WHEEL_STEPS]
                             [DM1_V1_CHEST_LEADER_STACK_ROTATION_STACK_COUNT];
    int focusTrace[DM1_V1_CHEST_LEADER_STACK_ROTATION_WHEEL_STEPS + 1];
    int focusZoneTrace[DM1_V1_CHEST_LEADER_STACK_ROTATION_WHEEL_STEPS + 1];
    int chestStableBeforeMutation[DM1_V1_CHEST_LEADER_STACK_ROTATION_WHEEL_STEPS];
    int leaderStackStableBeforeMutation[DM1_V1_CHEST_LEADER_STACK_ROTATION_WHEEL_STEPS];
    int partialMaskDispatches;
    int mouseEnableCount;
    int mouseDisableCount;
    int commandDispatchCount;
    int f0302DispatchCount;
    int firstMutationStep;
    int firstMutationZone;
    int firstMutationCommandSlot;
    int firstMutationLeaderThing;
    int firstMutationChestBefore;
    int chestAfterMutation[DM1_V1_CHEST_LEADER_STACK_ROTATION_SLOT_COUNT];
    int leaderStackAfterMutation[DM1_V1_CHEST_LEADER_STACK_ROTATION_STACK_COUNT];
    int f0298RemoveCount;
    int f0301WriteCount;
    int f0297PutCount;
    int loadMaskAfterMutation;
    int closeCount;
    int closedHead;
    int closedSlots[DM1_V1_CHEST_LEADER_STACK_ROTATION_SLOT_COUNT];
    int closeEndSentinel;
    int openChestAfterClose;
    int g0425ClearedAfterClose;
} DM1_V1_ChestLeaderStackRotationProbePc34;

const DM1_V1_ChestLeaderStackRotationSpecPc34*
dm1_v1_chest_scroll_wheel_leader_stack_rotation_spec_pc34(void);

int dm1_v1_chest_scroll_wheel_leader_stack_rotation_pc34(
    DM1_V1_ChestLeaderStackRotationProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_SCROLL_WHEEL_LEADER_STACK_ROTATION_PC34_COMPAT_H */
