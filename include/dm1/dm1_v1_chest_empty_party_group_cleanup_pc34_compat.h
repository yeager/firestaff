#ifndef FIRESTAFF_DM1_V1_CHEST_EMPTY_PARTY_GROUP_CLEANUP_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_EMPTY_PARTY_GROUP_CLEANUP_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_EMPTY_PARTY_GROUP_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_CHEST_EMPTY_PARTY_GROUP_THING_END = 0xFFFE,
    DM1_PC34_CHEST_EMPTY_PARTY_GROUP_THING_NONE = 0,
    DM1_PC34_CHEST_EMPTY_PARTY_GROUP_NOT_ON_SQUARE = -1,
    DM1_PC34_CHEST_EMPTY_PARTY_GROUP_CHEST_THING = 0x6601,
    DM1_PC34_CHEST_EMPTY_PARTY_GROUP_SENSOR_THING = 0x2202,
    DM1_PC34_CHEST_EMPTY_PARTY_GROUP_FLOOR_THING = 0x5503,
    DM1_PC34_CHEST_EMPTY_PARTY_GROUP_ITEM0 = 0x7100,
    DM1_PC34_CHEST_EMPTY_PARTY_GROUP_ITEM1 = 0x7101,
    DM1_PC34_CHEST_EMPTY_PARTY_GROUP_REOPEN_THING = 0x6602
};

typedef struct {
    int contractOnly;
    int slotCount;
    int thingEndOfList;
    int thingNone;
    int notOnSquare;
    int chestThing;
    int sensorThing;
    int floorThing;
    const char* f0333Anchor;
    const char* f0334Anchor;
    const char* f0163Anchor;
    const char* f0164Anchor;
    const char* f0172Anchor;
    const char* f0302Anchor;
    const char* scope;
} DM1_V1_ChestEmptyPartyGroupCleanupSpecPc34;

typedef struct {
    int openResult;
    int openThingAfterOpen;
    int emptyVisibleSlot0;
    int emptyVisibleSlot7;
    int closeCount;
    int openThingAfterClose;
    int getClosedSlotResult;
    int closeAgainCount;
    int noOpenCloseCount;
    int emptyCloseCleanupApplied;
    int emptyCloseSquarePresentBefore;
    int emptyCloseSquarePresentAfter;
    int emptyCloseFirstThingBefore;
    int emptyCloseFirstThingAfter;
    int emptyCloseSquareFirstThingCountBefore;
    int emptyCloseSquareFirstThingCountAfter;
    int emptyCloseColumn1Before;
    int emptyCloseColumn1After;
    int emptyCloseColumn2Before;
    int emptyCloseColumn2After;
    int emptyCloseRemovedNextAfterUnlink;
    int emptyCloseCellBitsCleared;
} DM1_V1_ChestEmptyPartyGroupCleanupEmptyClosePc34;

typedef struct {
    int headCleanupApplied;
    int headPresentBefore;
    int headPresentAfter;
    int headFirstBefore;
    int headFirstAfter;
    int headCountBefore;
    int headCountAfter;
    int headChestNextBefore;
    int headChestNextAfter;
    int headSensorNextAfter;

    int tailCleanupApplied;
    int tailPresentBefore;
    int tailPresentAfter;
    int tailFirstBefore;
    int tailFirstAfter;
    int tailCountBefore;
    int tailCountAfter;
    int tailSensorNextBefore;
    int tailSensorNextAfter;
    int tailChestNextAfter;

    int missingCleanupResult;
    int endOfListCleanupResult;
} DM1_V1_ChestEmptyPartyGroupCleanupSquareCasesPc34;

typedef struct {
    int openResult;
    int closeCount;
    int cleanupAttempted;
    int squarePresentAfterClose;
    int firstThingAfterClose;
    int closedTypes[DM1_PC34_CHEST_EMPTY_PARTY_GROUP_SLOT_COUNT];
    int closedWeights[DM1_PC34_CHEST_EMPTY_PARTY_GROUP_SLOT_COUNT];
    int closeClearsOpenThing;
    int reopenEmptyResult;
    int reopenEmptyCloseCount;
    int reopenEmptyCleanupApplied;
    int reopenEmptySquarePresentAfter;
    int reopenEmptyFirstThingAfter;
} DM1_V1_ChestEmptyPartyGroupCleanupOpenClosePc34;

typedef struct {
    int contractOnly;
    DM1_V1_ChestEmptyPartyGroupCleanupEmptyClosePc34 emptyClose;
    DM1_V1_ChestEmptyPartyGroupCleanupSquareCasesPc34 squareCases;
    DM1_V1_ChestEmptyPartyGroupCleanupOpenClosePc34 openClose;
} DM1_V1_ChestEmptyPartyGroupCleanupProbePc34;

const char*
dm1_v1_chest_empty_party_group_cleanup_source_evidence_pc34(void);
const DM1_V1_ChestEmptyPartyGroupCleanupSpecPc34*
dm1_v1_chest_empty_party_group_cleanup_spec_pc34(void);
int dm1_v1_chest_empty_party_group_cleanup_pc34(
    DM1_V1_ChestEmptyPartyGroupCleanupProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_EMPTY_PARTY_GROUP_CLEANUP_PC34_COMPAT_H */
