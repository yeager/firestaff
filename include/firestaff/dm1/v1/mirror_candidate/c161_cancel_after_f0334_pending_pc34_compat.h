#ifndef FIRESTAFF_DM1_V1_MIRROR_C161_CANCEL_AFTER_F0334_PENDING_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MIRROR_C161_CANCEL_AFTER_F0334_PENDING_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_C161_AFTER_F0334_CHEST_SLOT_COUNT_PC34 8
#define DM1_V1_MIRROR_C161_AFTER_F0334_TRACE_CAPACITY_PC34 16
#define DM1_V1_MIRROR_C161_AFTER_F0334_ANCHOR_COUNT_PC34 16

#define DM1_V1_MIRROR_C161_AFTER_F0334_ANCHOR_CHAMPION_PC34 \
    "CHAMPION.C F0297:243-268; F0298:270-298; F0300:485,564,575; F0301:606-660; F0302:662-714"
#define DM1_V1_MIRROR_C161_AFTER_F0334_ANCHOR_COMMAND_PC34 \
    "COMMAND.C F0359:1452-1662; F0360:1692-1707; F0378:1956-1993; F0380:2045-2178"
#define DM1_V1_MIRROR_C161_AFTER_F0334_ANCHOR_CHEST_PC34 \
    "CHEST.C F0333:30-67; F0334:79-130"
#define DM1_V1_MIRROR_C161_AFTER_F0334_ANCHOR_MOUSE_PC34 \
    "MOUSE.C F0077:1-32; F0078:33-64"
#define DM1_V1_MIRROR_C161_AFTER_F0334_ANCHOR_PANEL_PC34 \
    "PANEL.C F0344:1493-1561; F0345:1563-1617; F0346:1619-1637; F0347:1639-1693; F0355:2280-2440"
#define DM1_V1_MIRROR_C161_AFTER_F0334_ANCHOR_REVIVE_PC34 \
    "REVIVE.C F0280:124-132; F0282:744-806; F0286 statistics-reset"
#define DM1_V1_MIRROR_C161_AFTER_F0334_ANCHOR_DEFS_PC34 \
    "DEFS.H C040/C045/C160/C161/C162/C537..C544/G0299/G0424/G0425/G0426/M568/M070/M516"

typedef enum {
    DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0359_QUEUE_PC34 = 0x1610,
    DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0360_ROUTE_CLICK_PC34,
    DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0378_ROUTE_PANEL_PC34,
    DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0380_DRAIN_PC34,
    DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0282_C161_CANCEL_PC34,
    DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0077_BRACKET_OPEN_PC34,
    DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0078_BRACKET_CLOSE_PC34,
    DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0457_DRAW_ENABLED_MENUS_PC34,
    DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0347_PANEL_REDRAW_PC34,
    DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0344_FOOD_READ_PC34,
    DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0344_WATER_READ_PC34,
    DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0286_REJECT_PC34
} Dm1V1MirrorC161AfterF0334OpcodePc34;

typedef enum {
    DM1_V1_MIRROR_C161_AFTER_F0334_REJECT_NONE_PC34 = 0,
    DM1_V1_MIRROR_C161_AFTER_F0334_REJECT_NULL_PC34,
    DM1_V1_MIRROR_C161_AFTER_F0334_REJECT_NON_CONTRACT_PC34,
    DM1_V1_MIRROR_C161_AFTER_F0334_REJECT_NO_CANDIDATE_PC34,
    DM1_V1_MIRROR_C161_AFTER_F0334_REJECT_ALIVE_CANDIDATE_PC34,
    DM1_V1_MIRROR_C161_AFTER_F0334_REJECT_NO_RESURRECT_PENDING_PC34,
    DM1_V1_MIRROR_C161_AFTER_F0334_REJECT_CHEST_STILL_OPEN_PC34,
    DM1_V1_MIRROR_C161_AFTER_F0334_REJECT_F0334_NOT_FIRED_PC34
} Dm1V1MirrorC161AfterF0334RejectPc34;

typedef struct {
    uint16_t itemType;
    uint16_t weight;
    uint16_t charges;
} Dm1V1MirrorC161AfterF0334LeaderHandPc34;

typedef struct {
    Dm1V1MirrorC161AfterF0334OpcodePc34 opcode;
    int candidateCurrentHealth;
    int candidateFood;
    int candidateWater;
    Dm1V1MirrorC161AfterF0334LeaderHandPc34 leaderHand;
} Dm1V1MirrorC161AfterF0334TracePc34;

typedef struct {
    int contractOnly;
    int noGameData;
    int opcodeCount;
    const Dm1V1MirrorC161AfterF0334OpcodePc34 *opcodes;
    const char *const *anchors;
    int anchorCount;
} Dm1V1MirrorC161AfterF0334SourceLockPc34;

typedef struct {
    int contractOnly;
    int noGameData;
    int f0334CloseFired;
    int chestCloseRewiredG0425;
    uint16_t g0425ChestSlots[DM1_V1_MIRROR_C161_AFTER_F0334_CHEST_SLOT_COUNT_PC34];
    uint16_t chestContainerSlots[DM1_V1_MIRROR_C161_AFTER_F0334_CHEST_SLOT_COUNT_PC34];
    uint16_t g0426OpenChest;
    Dm1V1MirrorC161AfterF0334LeaderHandPc34 leaderHandBeforeClose;
    Dm1V1MirrorC161AfterF0334LeaderHandPc34 leaderHand;
    int resurrectPending;
    int candidateOwnerIndex;
    int candidateCurrentHealth;
    int candidateFood;
    int candidateWater;
    uint16_t g0299CandidateOrdinal;
    int c040PanelOpen;
    int c040PanelGraphic;
    int c040PanelCommand;
    int panelContentM568;
} Dm1V1MirrorC161AfterF0334StatePc34;

typedef struct {
    int accepted;
    Dm1V1MirrorC161AfterF0334RejectPc34 rejectCode;
    int opcodeCount;
    Dm1V1MirrorC161AfterF0334TracePc34
        trace[DM1_V1_MIRROR_C161_AFTER_F0334_TRACE_CAPACITY_PC34];
    int f0359QueueCount;
    int f0360RouteClickCount;
    int f0378RouteCount;
    int f0380DrainCount;
    int f0282C161CancelCount;
    int f0286StatisticsResetRejectCount;
    int f0286StatisticsResetCallCount;
    int f0077BracketOpenCount;
    int f0078BracketCloseCount;
    int f0457DrawEnabledMenusCount;
    int f0347PanelRedrawCount;
    int f0344FoodWaterReadCount;
    int g0299Cleared;
    int resurrectPendingCleared;
    int m568Cleared;
    int c040Cleared;
    int candidateStayedDead;
    int candidateFoodWaterStayedZero;
    int leaderHandStableAcrossCancel;
    int leaderHandStableAcrossClose;
    int chestSlotsRewiredBeforeCancel;
    uint32_t hash;
} Dm1V1MirrorC161AfterF0334ResultPc34;

const Dm1V1MirrorC161AfterF0334SourceLockPc34 *
dm1_v1_mirror_c161_cancel_after_f0334_pending_source_lock_pc34(void);

const char *
dm1_v1_mirror_c161_cancel_after_f0334_pending_source_evidence_pc34(void);

Dm1V1MirrorC161AfterF0334StatePc34
dm1_v1_mirror_c161_cancel_after_f0334_pending_default_state_pc34(void);

int dm1_v1_mirror_c161_cancel_after_f0334_pending_run_pc34(
    Dm1V1MirrorC161AfterF0334StatePc34 *state,
    Dm1V1MirrorC161AfterF0334ResultPc34 *result);

uint32_t dm1_v1_mirror_c161_cancel_after_f0334_pending_hash_pc34(
    const Dm1V1MirrorC161AfterF0334ResultPc34 *result);

#ifdef __cplusplus
}
#endif

#endif
