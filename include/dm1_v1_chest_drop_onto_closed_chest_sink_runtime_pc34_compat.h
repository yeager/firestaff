#ifndef FIRESTAFF_DM1_V1_CHEST_DROP_ONTO_CLOSED_CHEST_SINK_RUNTIME_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_DROP_ONTO_CLOSED_CHEST_SINK_RUNTIME_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_DROP_CLOSED_SINK_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_CHEST_DROP_CLOSED_SINK_INITIAL_COUNT = 3,
    DM1_PC34_CHEST_DROP_CLOSED_SINK_CHEST_THING = 0x7E20,
    DM1_PC34_CHEST_DROP_CLOSED_SINK_HELD_OBJECT = 0x7E40,
    DM1_PC34_CHEST_DROP_CLOSED_SINK_OPEN_HELD_OBJECT = 0x7E41,
    DM1_PC34_CHEST_DROP_CLOSED_SINK_FIRST_CHEST_ITEM = 0x7E80,
    DM1_PC34_CHEST_DROP_CLOSED_SINK_TARGET_SLOT_INDEX = 3,
    DM1_PC34_CHEST_DROP_CLOSED_SINK_TARGET_PC34_SLOT =
        DM1_PC34_SLOT_CHEST_1 + DM1_PC34_CHEST_DROP_CLOSED_SINK_TARGET_SLOT_INDEX,
    DM1_PC34_CHEST_DROP_CLOSED_SINK_NONE = 0,
    DM1_PC34_CHEST_DROP_CLOSED_SINK_REASON_NO_G0426 = 0x4260,
    DM1_PC34_CHEST_DROP_CLOSED_SINK_REASON_ACCEPTED_OPEN_G0426 = 0x4261
};

typedef struct {
    const char* contractMarker;
    const char* f0333Anchor;
    const char* f0334Anchor;
    const char* f0374Anchor;
    const char* f0297F0298Anchor;
    const char* f0302Anchor;
    const char* f0033Anchor;
    const char* f0133Anchor;
    const char* defsSlotAnchor;
    const char* defsZoneAnchor;
    const char* dataSlotMaskAnchor;
    const char* f0163Anchor;
    const char* f0336Note;
    int chestThing;
    int closedReasonCode;
    int openReasonCode;
    int targetChestSlotIndex;
    int targetPc34Slot;
    int c537Pc34Slot;
    int c544Pc34Slot;
    int closedDropKeepsLeaderHand;
    int openDropStoresInG0425;
} DM1_V1_ChestDropOntoClosedChestSinkSpecPc34;

typedef struct {
    int rowIndex;
    int result;
    int reasonCode;
    int leaderHandBefore;
    int leaderHandAfter;
    int openChestBefore;
    int openChestAfter;
    int targetPc34Slot;
    int targetChestSlotIndex;
    int absorbedByChest;
    int droppedToFloor;
    int manifestHashBefore;
    int manifestHashAfter;
} DM1_V1_ChestDropOntoClosedChestSinkEventPc34;

typedef struct {
    int setupOpenResult;
    int setupOpenThing;
    int setupCloseCount;
    int setupClosedOpenThing;
    int closedChestHasManifest;
    int closedChestG0426BeforeDrop;
    int closedDropResult;
    int closedLeaderHandBefore;
    int closedLeaderHandAfter;
    int closedManifestHashBefore;
    int closedManifestHashAfter;
    int closedManifestUnchanged;
    int closedNoAbsorb;
    int closedNoFloorFallback;
    int closedEventCount;
    int closedEventReason;
    int closedEventLeaderHandPreserved;
    int closedEventManifestStable;
    int closedG0426AfterDrop;

    int openResult;
    int openThingBeforeDrop;
    int openTargetSlotBefore;
    int openDropResult;
    int openLeaderHandBefore;
    int openLeaderHandAfter;
    int openTargetSlotAfter;
    int openManifestHashBefore;
    int openManifestHashAfter;
    int openManifestChanged;
    int openStoredHeldObject;
    int openNoFloorFallback;
    int openEventCount;
    int openEventReason;
    int openEventLeaderHandCleared;
    int openEventManifestChanged;
    int openCloseCount;
    int openClosedCountIncludesDrop;
    int openDroppedObjectInClosedManifest;

    int initialTypes[DM1_PC34_CHEST_DROP_CLOSED_SINK_SLOT_COUNT];
    int initialWeights[DM1_PC34_CHEST_DROP_CLOSED_SINK_SLOT_COUNT];
    int initialCharges[DM1_PC34_CHEST_DROP_CLOSED_SINK_SLOT_COUNT];
    int initialAllowedSlots[DM1_PC34_CHEST_DROP_CLOSED_SINK_SLOT_COUNT];
    int closedBeforeTypes[DM1_PC34_CHEST_DROP_CLOSED_SINK_SLOT_COUNT];
    int closedAfterTypes[DM1_PC34_CHEST_DROP_CLOSED_SINK_SLOT_COUNT];
    int closedBeforeWeights[DM1_PC34_CHEST_DROP_CLOSED_SINK_SLOT_COUNT];
    int closedAfterWeights[DM1_PC34_CHEST_DROP_CLOSED_SINK_SLOT_COUNT];
    int openBeforeTypes[DM1_PC34_CHEST_DROP_CLOSED_SINK_SLOT_COUNT];
    int openAfterTypes[DM1_PC34_CHEST_DROP_CLOSED_SINK_SLOT_COUNT];
    int openClosedTypes[DM1_PC34_CHEST_DROP_CLOSED_SINK_SLOT_COUNT];
    int openClosedWeights[DM1_PC34_CHEST_DROP_CLOSED_SINK_SLOT_COUNT];
    DM1_V1_ChestDropOntoClosedChestSinkEventPc34 events[2];
} DM1_V1_ChestDropOntoClosedChestSinkProbePc34;

const char*
dm1_v1_chest_drop_onto_closed_chest_sink_source_evidence_pc34(void);
const DM1_V1_ChestDropOntoClosedChestSinkSpecPc34*
dm1_v1_chest_drop_onto_closed_chest_sink_spec_pc34(void);
int dm1_v1_chest_drop_onto_closed_chest_sink_pc34(
    DM1_V1_ChestDropOntoClosedChestSinkProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_DROP_ONTO_CLOSED_CHEST_SINK_RUNTIME_PC34_COMPAT_H */
