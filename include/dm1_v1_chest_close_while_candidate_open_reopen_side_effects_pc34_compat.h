#ifndef FIRESTAFF_DM1_V1_CHEST_CLOSE_WHILE_CANDIDATE_OPEN_REOPEN_SIDE_EFFECTS_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_CLOSE_WHILE_CANDIDATE_OPEN_REOPEN_SIDE_EFFECTS_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_C10_COLOR_FLESH = 10,
    DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_C025_PANEL_CHEST = 25,
    DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_C040_PANEL_CANDIDATE = 40,
    DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_C30_FIRST = 30,
    DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_C38_FIRST = 38,
    DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_C537_FIRST = 537,
    DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_C544_LAST = 544,
    DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_M568_CANDIDATE = 568,
    DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_M569_CHEST = 569,
    DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_G0299_ORDINAL = 3,
    DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_G0426_FIRST = 0x7426,
    DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_G0426_SECOND = 0x7526,
    DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_FIRST_CHAMPION = 0,
    DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_REOPEN_CHAMPION = 1,
    DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_EXPECTED_ASSERTIONS = 59
};

#define DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_EXPECTED_HASH \
    UINT64_C(0x524a81cc3012d0d3)

typedef struct {
    const char* contractMarker;
    unsigned int deterministicSeed;
    int partyChampionCount;
    int firstChampionOrdinal;
    int reopenChampionOrdinal;
    int c537Ordinal;
    int c544Ordinal;
    int c040PanelGraphic;
    int c025ChestPanelGraphic;
} DM1_V1_ChestCloseWhileCandidateOpenReopenSideEffectsSpecPc34Compat;

typedef struct {
    M11_InventoryState runtime;
    unsigned int deterministicSeed;
    int partyChampionCount;
    int inventoryChampionOrdinal;
    int candidateChampionOrdinal;
    int panelContent;
    int panelGraphic;
    int c040CandidateLive;
    int c040RedrawOnCloseCount;
    int c025RedrawOnReopenCount;
    M11_Item firstLinked[DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_SLOT_COUNT];
    M11_Item secondLinked[DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_SLOT_COUNT];
    int firstLinkedCount;
    int secondLinkedCount;
} DM1_V1_ChestCloseWhileCandidateOpenReopenSideEffectsRuntimePc34Compat;

typedef struct {
    int initResult;
    int exerciseResult;
    int sourceLockedContractOnly;
    unsigned int deterministicSeed;
    int partyChampionCount;
    int firstChampionOrdinal;
    int reopenChampionOrdinal;
    int candidateBefore;
    int candidateAfterClose;
    int candidateAfterReopen;
    int c040LiveBefore;
    int c040LiveAfterClose;
    int c040LiveAfterReopen;
    int panelBefore;
    int panelAfterClose;
    int panelAfterReopen;
    int panelGraphicAfterClose;
    int panelGraphicAfterReopen;
    int c040RedrawOnCloseCount;
    int c025RedrawOnReopenCount;
    int firstOpenThingBeforeClose;
    int firstOpenThingAfterClose;
    int reopenOpenThing;
    int firstClosedCount;
    int secondClosedCount;
    int reopenedVisibleCount;
    int firstCloseClearedG0426;
    int firstCloseCompactedVisibleChain;
    int secondCloseCompactedVisibleChain;
    int reopenedFirstChestOnDifferentChampion;
    int championZeroVisibleCleared;
    int noFirstLeakIntoSecondClosedChain;
    int noSecondLeakIntoReopenedFirstChain;
    int c537ToC544ReboundToReopenedChest;
    int firstBeforeTypes[DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_SLOT_COUNT];
    int firstClosedTypes[DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_SLOT_COUNT];
    int secondBeforeTypes[DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_SLOT_COUNT];
    int secondClosedTypes[DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_SLOT_COUNT];
    int reopenedTypes[DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_SLOT_COUNT];
    int reopenedZoneOrdinals[DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_SLOT_COUNT];
    uint64_t deterministicHash;
    int totalAssertions;
    int passedAssertions;
    int failedAssertions;
} DM1_V1_ChestCloseWhileCandidateOpenReopenSideEffectsProbePc34Compat;

extern const DM1_V1_ChestCloseWhileCandidateOpenReopenSideEffectsSpecPc34Compat
    dm1_v1_chest_close_while_candidate_open_reopen_side_effects_spec_data_pc34_compat;

const char*
dm1_v1_chest_close_while_candidate_open_reopen_side_effects_source_evidence_pc34_compat(
    void);

const DM1_V1_ChestCloseWhileCandidateOpenReopenSideEffectsSpecPc34Compat*
dm1_v1_chest_close_while_candidate_open_reopen_side_effects_spec_pc34_compat(
    void);

int dm1_v1_chest_close_while_candidate_open_reopen_side_effects_init_pc34_compat(
    DM1_V1_ChestCloseWhileCandidateOpenReopenSideEffectsRuntimePc34Compat* state,
    unsigned int deterministicSeed);

int dm1_v1_chest_close_while_candidate_open_reopen_side_effects_exercise_pc34_compat(
    DM1_V1_ChestCloseWhileCandidateOpenReopenSideEffectsRuntimePc34Compat* state,
    DM1_V1_ChestCloseWhileCandidateOpenReopenSideEffectsProbePc34Compat* out);

int dm1_v1_chest_close_while_candidate_open_reopen_side_effects_pc34_compat(
    DM1_V1_ChestCloseWhileCandidateOpenReopenSideEffectsProbePc34Compat* out);

#ifdef __cplusplus
}
#endif

#endif
