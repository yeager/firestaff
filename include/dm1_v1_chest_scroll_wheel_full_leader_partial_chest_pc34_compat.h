#ifndef FIRESTAFF_DM1_V1_CHEST_SCROLL_WHEEL_FULL_LEADER_PARTIAL_CHEST_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_SCROLL_WHEEL_FULL_LEADER_PARTIAL_CHEST_PC34_COMPAT_H

#include <stddef.h>

/*
 * DM1 V1 chest scroll-wheel composite routing contract.
 *
 * ReDMCSB anchors:
 * - CHEST.C F0333:30-67: same-open no-op, partial G0425 chest-slot fill,
 *   and cross-champion inventory dispatch while C537..C544 hold visible cells.
 * - CHEST.C F0334:117-132: close rewrite skips empty G0425 entries and
 *   relinks non-empty cells, recovering a clean container list.
 * - CHAMPION.C F0297:243-268 and F0298:270-298: leader-hand put/remove owns
 *   G4055, pointer/icon identity, leader load, and load-refresh flags.
 * - CHAMPION.C F0302:662-710: C30+ chest-slot clicks read G0425, snapshot the
 *   leader hand, and suppress empty/empty or disallowed slot movement.
 * - PANEL.C F0344:1895-1944 and F0345:1946-1999: requested panel click and
 *   per-cell highlight rotation lineage for scroll-wheel focus.
 * - COMMAND.C F0359:1985-1990: requested M568/C040 panel dispatch marker.
 * - MOUSE.C F0077:97-126 and F0078:128-168: requested wheel queue lineage.
 * - OBJECT.C F0033:147-212: visible object icon identity.
 * - BLITMASK.C F0133:30-33: partial-mask presentation dispatch.
 * - DEFS.H:810 and 3906-3913: C30 and C537..C544 slot constants.
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM1_V1_CHEST_SCROLL_WHEEL_DISPATCH_PICKUP = 0,
    DM1_V1_CHEST_SCROLL_WHEEL_SUPPRESS_LEADER_FULL = 1,
    DM1_V1_CHEST_SCROLL_WHEEL_FOCUS_CHEST_SLOT = 2,
    DM1_V1_CHEST_SCROLL_WHEEL_SCROLL_TO_NEXT_SLOT = 3
} DM1_V1_ChestScrollWheelCompositeDecision;

typedef enum {
    DM1_V1_CHEST_SCROLL_WHEEL_DIRECTION_NONE = 0,
    DM1_V1_CHEST_SCROLL_WHEEL_DIRECTION_UP = -1,
    DM1_V1_CHEST_SCROLL_WHEEL_DIRECTION_DOWN = 1
} DM1_V1_ChestScrollWheelCompositeDirection;

typedef struct {
    int leaderHandFull;
    int chestSlotCount;
    int chestSlotOccupied;
    DM1_V1_ChestScrollWheelCompositeDirection scrollDirection;
    int targetChampionIndex;
    DM1_V1_ChestScrollWheelCompositeDecision expected;
} DM1_V1_ChestScrollWheelCompositeCasePc34;

enum {
    DM1_V1_CHEST_SCROLL_WHEEL_COMPOSITE_C30 = 30,
    DM1_V1_CHEST_SCROLL_WHEEL_COMPOSITE_C537 = 537,
    DM1_V1_CHEST_SCROLL_WHEEL_COMPOSITE_C538 = 538,
    DM1_V1_CHEST_SCROLL_WHEEL_COMPOSITE_C544 = 544,
    DM1_V1_CHEST_SCROLL_WHEEL_COMPOSITE_VISIBLE_SLOTS = 8,
    DM1_V1_CHEST_SCROLL_WHEEL_COMPOSITE_PARTIAL_SLOTS = 2
};

DM1_V1_ChestScrollWheelCompositeDecision
dm1_v1_chest_scroll_wheel_composite_decide(
    int leaderHandFull,
    int chestSlotCount,
    int chestSlotOccupied,
    DM1_V1_ChestScrollWheelCompositeDirection scrollDirection,
    int targetChampionIndex);

const DM1_V1_ChestScrollWheelCompositeCasePc34*
dm1_v1_chest_scroll_wheel_composite_cases(size_t* count);

const char*
dm1_v1_chest_scroll_wheel_composite_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_SCROLL_WHEEL_FULL_LEADER_PARTIAL_CHEST_PC34_COMPAT_H */
