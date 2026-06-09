#ifndef FIRESTAFF_DM1_V1_CHEST_EMPTY_REOPEN_RUNTIME_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_EMPTY_REOPEN_RUNTIME_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    /* Source-locked sentinel values for the empty-reopen runtime probe.
     * The chest thing ids do not need to match any real DUNGEON.DAT slot;
     * they only need to be distinct, non-zero, and treated as opaque tokens
     * by m11_inventory_open_chest / m11_inventory_close_chest. */
    DM1_PC34_CHEST_EMPTY_REOPEN_CHEST_A = 0x6E10,
    DM1_PC34_CHEST_EMPTY_REOPEN_CHEST_B = 0x6E20,
    DM1_PC34_CHEST_EMPTY_REOPEN_CHEST_C = 0x6E30,
    /* A "leader hand" identity that is non-empty, weight-bearing, and has
     * charges so the probe can prove F0333/F0334 do not mutate any of the
     * three hand fields. */
    DM1_PC34_CHEST_EMPTY_REOPEN_LEADER_ITEM = 0x4A11,
    DM1_PC34_CHEST_EMPTY_REOPEN_LEADER_WEIGHT = 17,
    DM1_PC34_CHEST_EMPTY_REOPEN_LEADER_CHARGES = 9,
    DM1_PC34_CHEST_EMPTY_REOPEN_LEADER_ALLOWED_SLOTS =
        DM1_PC34_ALLOWED_HEAD | DM1_PC34_ALLOWED_CONTAINER,
    DM1_PC34_CHEST_EMPTY_REOPEN_STALE_C537 = 0x5E01,
    DM1_PC34_CHEST_EMPTY_REOPEN_STALE_C544 = 0x5E08,
    DM1_PC34_CHEST_EMPTY_REOPEN_STALE_OUTPUT = 0x5E80
};

typedef struct {
    /* M569_PANEL_CHEST → F0378 → F0358 → F0302 is gated by the panel
     * content and the leader presence; this contract gate covers the
     * narrower "empty chest" pathway of that route. */
    const char* contractMarker;
    int chestA;
    int chestB;
    int chestC;
    int leaderItem;
    int leaderWeight;
    int leaderCharges;
    int leaderAllowedSlots;
    int thingNoneSentinel;
    int expectedChestSlotCount;
    const char* f0333VisibleFillAnchor;
    const char* f0333SameChestNoopAnchor;
    const char* f0333TransitiveCloseAnchor;
    const char* f0334NoOpenChestAnchor;
    const char* f0334G0425ClearAnchor;
    const char* f0334G0426ClearAnchor;
    const char* f0297F0298LeaderHandAnchor;
    const char* defsC537C544Anchor;
} DM1_V1_ChestEmptyReopenRuntimeSpecPc34;

typedef struct {
    int openAResult;
    int openAOpenThing;
    int openACloseResult;
    int openACloseCount;
    int openAOpenThingAfterClose;
    int openAAllG0425NoneAfterOpen;
    int openAAllG0425NoneAfterClose;
    int openACloseOnAlreadyClosedResult;
    int openACloseOnAlreadyClosedCount;
    int sameChestReopenResult;
    int sameChestReopenOpenThing;
    int sameChestReopenG0425Stable;
    int sameChestReopenOpenThingStable;
    int crossChestBResult;
    int crossChestBPreviousCount;
    int crossChestBFinalOpenThing;
    int crossChestBPanelBeforeReplace;
    int crossChestBPanelAfterReplace;
    int crossChestBPanelAfterClose;
    int crossChestBG0425AllNone;
    int crossChestBCloseAfterBResult;
    int crossChestBCloseAfterBCount;
    int closeWhenNothingOpenResult;
    int closeWhenNothingOpenCount;
    int closeWhenNothingOpenThingAfter;
    int closeWhenNothingPanelBefore;
    int closeWhenNothingPanelAfter;
    int staleC537BeforeNoOpenClose;
    int staleC544BeforeNoOpenClose;
    int staleC537AfterNoOpenClose;
    int staleC544AfterNoOpenClose;
    int staleOutputBeforeNoOpenClose;
    int staleOutputAfterNoOpenClose;
    int noOpenClosePreservedStaleWindow;
    int noOpenClosePreservedOutputBuffer;
    int noOpenClosePreservedPanelContent;
    int leaderHandTypeBeforeCycles;
    int leaderHandWeightBeforeCycles;
    int leaderHandChargesBeforeCycles;
    int leaderHandTypeAfterCycles;
    int leaderHandWeightAfterCycles;
    int leaderHandChargesAfterCycles;
    int leaderHandIdenticalAcrossCycles;
    int leaderLoadBeforeCycles;
    int leaderLoadAfterCycles;
    int championLoadStableAcrossCycles;
    int noF0334SideEffectsOnClosedOpen;
    int openCG0425AllNone;
    int closeCReopensCleanly;
} DM1_V1_ChestEmptyReopenRuntimeProbePc34;

extern const DM1_V1_ChestEmptyReopenRuntimeSpecPc34
    dm1_v1_chest_empty_reopen_runtime_pc34_spec;

const char* dm1_v1_chest_empty_reopen_runtime_source_evidence_pc34(void);
const DM1_V1_ChestEmptyReopenRuntimeSpecPc34*
dm1_v1_chest_empty_reopen_runtime_spec_pc34(void);
int dm1_v1_chest_empty_reopen_runtime_run_pc34(
    DM1_V1_ChestEmptyReopenRuntimeProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_EMPTY_REOPEN_RUNTIME_PC34_COMPAT_H */
