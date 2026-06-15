#ifndef FIRESTAFF_DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_SLOT_COUNT_PC34 = 8,
    DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_COMMAND_COUNT_PC34 = 2,
    DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_THING_NONE_PC34 = 0xFFFF,
    DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_THING_END_PC34 = 0xFFFE,
    DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_LEADER_INDEX_PC34 = 0,
    DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_NON_LEADER_INDEX_PC34 = 1,
    DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_INVENTORY_ORDINAL_PC34 = 1,
    DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_PARTY_COUNT_PC34 = 4,
    DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_READY_HAND_PC34 = 0,
    DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_TARGET_INDEX_PC34 = 3,
    DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_C30_BASE_PC34 = 30,
    DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_C537_BASE_PC34 = 537,
    DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_C540_ZONE_PC34 = 540,
    DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_NON_LEADER_SLOT_BOX_PC34 = 2,
    DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_ALLOWED_HANDS_PC34 = 0x0200,
    DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_ALLOWED_CONTAINER_PC34 = 0x0400,
    DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_OPEN_CHEST_PC34 = 0xC540,
    DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_LEADER_WEAPON_PC34 = 0x7101,
    DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_C540_WEAPON_PC34 = 0x7204
};

typedef struct {
    int thing;
    int weight;
    int allowedSlots;
    int next;
} DM1_V1_ChestOccupiedSlotSwapItemPc34;

typedef struct {
    const char* contractMarker;
    int contractOnly;
    int deterministicSeed;
    int partyChampionCount;
    int inventoryChampionOrdinal;
    int leaderIndex;
    int nonLeaderIndex;
    int targetChestIndex;
    int targetPc34Slot;
    int targetZone;
    int nonLeaderSlotBox;
    int nonLeaderHandSlot;
    int thingNone;
    int thingEnd;
    const char* f0333Anchor;
    const char* f0334NegativeAnchor;
    const char* f0297Anchor;
    const char* f0298Anchor;
    const char* f0300Anchor;
    const char* f0301Anchor;
    const char* f0302Anchor;
    const char* f0140Anchor;
    const char* f0159Anchor;
    const char* f0163Anchor;
    const char* f0164Anchor;
    const char* f0032Anchor;
    const char* f0033Anchor;
    const char* f0359Anchor;
    const char* f0380Anchor;
    const char* f0077Anchor;
    const char* f0078Anchor;
    const char* defsAnchor;
    const char* disjointness;
} DM1_V1_ChestOccupiedSlotSwapSpecPc34;

typedef struct {
    int command;
    int x;
    int y;
    int pc34Slot;
    int zone;
    int resolvedChampion;
    int resolvedHandSlot;
} DM1_V1_ChestOccupiedSlotSwapCommandPc34;

typedef struct {
    int setupResult;
    int exerciseResult;
    int contractOnly;
    int deterministicSeed;
    uint32_t hash;

    int partyChampionCount;
    int inventoryChampionOrdinal;
    int candidateChampionOrdinal;
    int leaderIndex;
    int nonLeaderIndex;
    int leaderCurrentHealth;
    int nonLeaderCurrentHealth;
    int openChestBefore;
    int openChestAfterReceive;
    int openChestAfterChestClick;
    int panelWasChestBefore;
    int panelStillChestAfter;

    int queuedCount;
    int queueWriteOrder[DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_COMMAND_COUNT_PC34];
    int queueDrainOrder[DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_COMMAND_COUNT_PC34];
    DM1_V1_ChestOccupiedSlotSwapCommandPc34 commands[DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_COMMAND_COUNT_PC34];

    int beforeChestTypes[DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_SLOT_COUNT_PC34];
    int beforeChestNext[DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_SLOT_COUNT_PC34];
    int afterReceiveChestTypes[DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_SLOT_COUNT_PC34];
    int afterReceiveChestNext[DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_SLOT_COUNT_PC34];
    int afterChestTypes[DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_SLOT_COUNT_PC34];
    int afterChestNext[DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_SLOT_COUNT_PC34];
    int expectedAfterChestTypes[DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_SLOT_COUNT_PC34];
    int expectedAfterChestNext[DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_SLOT_COUNT_PC34];

    int leaderHandBefore;
    int leaderHandAfterReceive;
    int leaderHandAfterChestClick;
    int nonLeaderHandBefore;
    int nonLeaderHandAfterReceive;
    int nonLeaderHandAfterChestClick;
    int targetChestBefore;
    int targetChestAfterReceive;
    int targetChestAfterChestClick;

    int f0359QueuedChestC540;
    int f0359QueuedNonLeaderHand;
    int f0380DrainedNonLeaderFirst;
    int f0380DrainedChestSecond;
    int f0302ResolvedNonLeaderHand;
    int f0302ResolvedChestC540;
    int f0298RemovedLeaderWeaponForReceive;
    int f0301WroteLeaderWeaponToNonLeaderHand;
    int f0300ClearedC540;
    int f0297PutC540WeaponInLeaderHand;
    int f0164DetachedOldC540FromOpenChain;
    int f0334CloseCallCount;
    int f0333OpenCallCount;
    int f0077CallCount;
    int f0078CallCount;

    int c537Stable;
    int c538Stable;
    int c539Stable;
    int c541Stable;
    int c542Stable;
    int c543Stable;
    int c544Stable;
    int c540Cleared;
    int noDuplicateLeaderWeapon;
    int noDuplicateC540Weapon;
    int noThingNoneInsideNonEmptyPrefix;
    int oldLeaderWeaponMovedToNonLeader;
    int oldC540WeaponMovedToLeader;
    int chestStayedOpen;
    int closePathNotUsed;
} DM1_V1_ChestOccupiedSlotSwapProbePc34;

const char* dm1_v1_chest_occupied_slot_swap_source_evidence_pc34(void);
const DM1_V1_ChestOccupiedSlotSwapSpecPc34*
dm1_v1_chest_occupied_slot_swap_spec_pc34(void);
int dm1_v1_chest_occupied_slot_swap_pc34(
    DM1_V1_ChestOccupiedSlotSwapProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif
