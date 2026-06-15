/*
 * DM1 V1 mirror-candidate reopen-after-save/load gate.
 *
 * Contract-only, no-asset fixture. This pins the *narrow* slice of the
 * source-locked ReDMCSB save/load + F0280/F0282/F0346/F0347 resurrect
 * panel chain that the existing mirror-candidate family does not cover:
 *
 *   - LOADSAVE.C F0433:1502-1707 (F0433_STARTEND_ProcessCommand140_SaveGame_CPSCDF)
 *     serializes GLOBAL_DATA + M516_CHAMPIONS + G0407_s_Party
 *     (C2_SAVE_PART_PARTY at lines 1579-1584) plus ACTIVE_GROUPs,
 *     EVENTs and TIMELINEs. GLOBAL_DATA in DEFS.H:534-571 has fields
 *     for GameTime, LastRandomNumber, LeaderHandObject,
 *     PartyChampionCount, PartyMapX/Y, PartyDirection, PartyMapIndex,
 *     LeaderIndex, MagicCasterChampionIndex, EventCount and similar
 *     persistable scalars — but it does NOT include G0299
 *     (candidate ordinal), G0424 (panel content), G0425 (chest slot
 *     array), or G0426 (open chest thing).
 *
 *   - LOADSAVE.C F0435:2192-2660 (F0435_STARTEND_LoadGame) reads the
 *     same five save parts back and rebuilds the world. It never
 *     touches the runtime UI globals either.
 *
 *   - DEFS.H:5694 G0299_ui_CandidateChampionOrdinal, :5877
 *     G0424_i_PanelContent, :5878 G0425_aT_ChestSlots[8], :5881
 *     G0426_T_OpenChest are declared `extern` globals that live
 *     alongside the runtime but are not part of the save blob. They
 *     are UI / panel state, not world state.
 *
 *   - REVIVE.C F0280:124-132 (F0280_REVIVE_PublishCandidate) appends
 *     the candidate champion to the party, sets G0299, and routes
 *     the C040 panel through PANEL.C F0347:1639-1693 -> F0346:1619-1637
 *     (F0346_INVENTORY_DrawPanel_ResurrectReincarnate) which sets
 *     G0424 = M568_PANEL_RESURRECT_REINCARNATE.
 *
 *   - REVIVE.C F0282:744-806 (F0282_REVIVE_ClearCandidate) is the
 *     single function that clears G0299 back to 0 on confirm/cancel
 *     after the resurrect path runs.
 *
 *   - The mirror-candidate reopen path is therefore: while a C040
 *     candidate is live (G0299 != 0, G0424 = M568, C040 graphic) the
 *     player can take the in-game C140_COMMAND_SAVE_GAME / load menu
 *     path (LOADSAVE.C F0433 + F0435) and the save/load path MUST NOT
 *     mutate G0299, G0424, G0425 or G0426. After load, the player
 *     can re-trigger the C040 panel by re-publishing the candidate
 *     via F0280 (e.g. after a new mirror inspect), at which point
 *     the standard F0347 -> F0346 path takes over.
 *
 * This gate asserts the four-field no-mutate contract on the save
 * and load round-trip, then exercises the post-load reopen path
 * to confirm the C040 panel reaches the M568 command / C040 graphic
 * state after a fresh F0280 publication on the loaded party.
 *
 * Non-overlap marker: the lane is the *save/load + reopen* slice
 * that no other mirror-candidate gate covers together — the existing
 * c160_close_while_rotation_pending, c061_drop_resurrect_pending,
 * c045_food_water_accept_cross_rotation, c040_panel_browse_pickup_rotate_race,
 * panel_redraw_after_inventory_exit, resurrect_chest_close_order,
 * resurrect_confirm_inventory_interrupt, close_after_party_shuffle,
 * and close_while_resurrect_pending_with_inventory_pickup gates all
 * start from an already-live C040 candidate and do not cross the
 * F0433/F0435 save/load boundary. The DM1 V1 save/load runtime
 * boundary, runtime save/load slot, full save/load round-trip, and
 * the existing live C040 mirror-candidate save/load integration
 * test cover the surface above the panel layer; the lane is the
 * panel layer (G0299, G0424, G0425, G0426) crossing the F0433/F0435
 * boundary and the post-load reopen via F0280 -> F0347 -> F0346.
 *
 * ReDMCSB source-lock anchors (ReDMCSB WIP 20210206, PC 3.4 path,
 * MEDIA009+):
 *   - LOADSAVE.C F0433:1502-1707 (F0433_STARTEND_ProcessCommand140_SaveGame_CPSCDF):
 *     GLOBAL_DATA + M516_CHAMPIONS + G0407_s_Party serialization.
 *   - LOADSAVE.C F0433:1571-1584 (C2_SAVE_PART_PARTY): M516_CHAMPIONS +
 *     G0407_s_Party byte-count is exactly sizeof(M516_CHAMPIONS) +
 *     sizeof(G0407_s_Party), no UI globals.
 *   - LOADSAVE.C F0435:2192-2660 (F0435_STARTEND_LoadGame): the
 *     matching deserialization path. The five save parts (GLOBAL_DATA,
 *     ACTIVE_GROUPs, PARTY, EVENTs, TIMELINEs) are read but the
 *     runtime UI globals are not restored.
 *   - DEFS.H:534-571 GLOBAL_DATA struct: GameTime, LastRandomNumber,
 *     LeaderHandObject, PartyChampionCount, PartyMapX/Y,
 *     PartyDirection, PartyMapIndex, LeaderIndex,
 *     MagicCasterChampionIndex, EventCount, FirstUnusedEventIndex,
 *     EventMaximumCount, CurrentActiveGroupCount, LastCreatureAttackTime,
 *     LastPartyMovementTime, DisabledMovementTicks,
 *     ProjectileDisabledMovementTicks,
 *     LastProjectileDisabledMovementDirection, MaximumActiveGroupCount
 *     (no G0299, G0424, G0425, G0426).
 *   - DEFS.H:5694 extern G0299_ui_CandidateChampionOrdinal — runtime
 *     UI state, not saved.
 *   - DEFS.H:5877 extern G0424_i_PanelContent — runtime UI state,
 *     not saved.
 *   - DEFS.H:5878 extern G0425_aT_ChestSlots[8] — runtime UI state,
 *     not saved.
 *   - DEFS.H:5881 extern G0426_T_OpenChest — runtime UI state,
 *     not saved.
 *   - REVIVE.C F0280:124-132 (F0280_REVIVE_PublishCandidate) — single
 *     publication point for the C040 candidate.
 *   - REVIVE.C F0282:744-806 (F0282_REVIVE_ClearCandidate) — single
 *     clear point for the C040 candidate.
 *   - PANEL.C F0346:1619-1637 (F0346_INVENTORY_DrawPanel_ResurrectReincarnate)
 *     — sets G0424 = M568_PANEL_RESURRECT_REINCARNATE and draws the
 *     C040 graphic.
 *   - PANEL.C F0347:1639-1693 (F0347_INVENTORY_DrawPanel) — the
 *     G0299 != 0 reroute at line 1654 -> F0346.
 *   - PANEL.C F0355:2244-2330 (F0355_INVENTORY_Toggle_CPSE) — the
 *     inventory open/close path with the !G0299 candidate gate at
 *     lines 2318-2322.
 *   - CHEST.C F0333:30-67 (F0333_INVENTORY_OpenAndDrawChest) and
 *     F0334:79-130 (F0334_INVENTORY_CloseChest) — the G0426
 *     open/close pair that the save/load path must not invoke.
 *
 * Contract-only, deterministic, no game data, no GRAPHICS.DAT, no
 * DUNGEON.DAT, no real-asset pixels.
 */

#ifndef FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_REOPEN_AFTER_SAVE_LOAD_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_REOPEN_AFTER_SAVE_LOAD_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_MC_RASL_PARTY_COUNT_PC34 = 4,
    DM1_V1_MC_RASL_CHEST_SLOT_COUNT_PC34 = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_V1_MC_RASL_TRACE_COUNT_PC34 = 8,
    DM1_V1_MC_RASL_C040_GRAPHIC_PC34 = 40,
    DM1_V1_MC_RASL_M568_PANEL_RESURRECT_REINCARNATE_PC34 = 568,
    DM1_V1_MC_RASL_C00_PANEL_INVENTORY_PC34 = 0,
    DM1_V1_MC_RASL_NO_THING_PC34 = 0xFFFF,
    DM1_V1_MC_RASL_NO_CHAMPION_PC34 = -1,
    DM1_V1_MC_RASL_LEADER_PC34 = 0,
    DM1_V1_MC_RASL_CANDIDATE_PARTY_INDEX_PC34 = 1,
    DM1_V1_MC_RASL_INVENTORY_CHAMPION_PC34 = 0,
    DM1_V1_MC_RASL_C040_OWNER_PC34 = 0,
    DM1_V1_MC_RASL_GAME_TICKS_PC34 = 1234567u,
    DM1_V1_MC_RASL_C040_THING_PC34 = 0x6C40,
    DM1_V1_MC_RASL_DETERMINISTIC_SEED_PC34 = 0xF0433C04u
};

typedef enum {
    DM1_V1_MC_RASL_STEP_INIT_PC34 = 0,
    DM1_V1_MC_RASL_STEP_F0280_PUBLISH_PC34 = 1,
    DM1_V1_MC_RASL_STEP_F0355_OPEN_PC34 = 2,
    DM1_V1_MC_RASL_STEP_F0433_SAVE_PC34 = 3,
    DM1_V1_MC_RASL_STEP_F0435_LOAD_PC34 = 4,
    DM1_V1_MC_RASL_STEP_ASSERT_NO_UI_MUTATE_PC34 = 5,
    DM1_V1_MC_RASL_STEP_F0280_REOPEN_PC34 = 6,
    DM1_V1_MC_RASL_STEP_F0347_REOPEN_PC34 = 7
} DM1_V1_MirrorCandidateReopenAfterSaveLoadStepPc34;

typedef struct {
    const char* f0433SaveAnchor;
    const char* f0435LoadAnchor;
    const char* f0280PublishAnchor;
    const char* f0282ClearAnchor;
    const char* f0346ResurrectAnchor;
    const char* f0347PanelAnchor;
    const char* f0355ToggleAnchor;
    const char* f0333OpenAnchor;
    const char* f0334CloseAnchor;
    const char* defsGlobalDataAnchor;
    const char* defsUiGlobalsAnchor;
    const char* panelRedrawAfterCloseAnchor;
    const char* disjointness;
    const char* contractMarker;
    uint32_t deterministicSeed;
    int contractOnly;
    int noGameData;
    int noGraphicsDatLoad;
    int noDungeonDatLoad;
    int noRealAssetPixels;
} DM1_V1_MirrorCandidateReopenAfterSaveLoadSpecPc34;

typedef struct {
    int contractOnly;
    int noGameData;
    int noGraphicsDatLoad;
    int noDungeonDatLoad;
    int noRealAssetPixels;

    uint32_t deterministicSeed;
    uint32_t deterministicHash;
    int stepTrace[DM1_V1_MC_RASL_TRACE_COUNT_PC34];
    int stepCount;

    int f0280PublishCount;
    int f0282ClearCount;
    int f0346ResurrectDrawCount;
    int f0347PanelDrawCount;
    int f0355ToggleSuppressedByCandidateCount;
    int f0355OpenForLeaderCount;
    int f0333OpenCount;
    int f0334CloseCount;
    int f0433SaveCount;
    int f0435LoadCount;

    int g0299BeforeSave;
    int g0299AfterSave;
    int g0299AfterLoad;
    int g0299AfterReopen;
    int g0299ClearedBySave;
    int g0299ClearedByLoad;
    int g0299ReopenedByF0280;

    int g0424BeforeSave;
    int g0424AfterSave;
    int g0424AfterLoad;
    int g0424AfterReopen;
    int g0424MutatedBySave;
    int g0424MutatedByLoad;
    int g0424AtResurrectAfterReopen;

    int g0426BeforeSave;
    int g0426AfterSave;
    int g0426AfterLoad;
    int g0426MutatedBySave;
    int g0426MutatedByLoad;

    int g0425NonEmptyBeforeSave;
    int g0425VisibleCountBeforeSave;
    int g0425MutatedBySave;
    int g0425MutatedByLoad;
    int g0425ClearedByLoad;

    int g0424ResetToInventoryByLoad;
    int g0299ResetToZeroByLoad;
    int g0426ResetToNoThingByLoad;
    int g0425ResetToAllNoneByLoad;

    int panelContentBeforeSave;
    int panelContentAfterLoad;
    int panelContentAfterReopen;
    int leaderHandItemBeforeSave;
    int leaderHandItemAfterLoad;
    int partyChampionCountBeforeSave;
    int partyChampionCountAfterLoad;
    int activeChampionBeforeSave;
    int activeChampionAfterLoad;

    int reopenRoutedToF0346;
    int reopenC040GraphicDrawn;
    int reopenM568PanelSet;
    int reopenNoF0282Clear;
    int reopenPartyPreserved;
    int reopenLeaderHandPreserved;

    int f0333NotInvokedAcrossSaveLoad;
    int f0334NotInvokedAcrossSaveLoad;
    int f0355ToggleSuppressedByLoad;

    int noPassC160CloseRotationPending;
    int noPassC061DropResurrectPending;
    int noPassC045FoodWaterAcceptCrossRotation;
    int noPassC040PanelBrowsePickupRotateRace;
    int noPassPanelRedrawAfterInventoryExit;
    int noPassResurrectChestCloseOrder;
    int noPassResurrectConfirmInventoryInterrupt;
    int noPassCloseAfterPartyShuffle;
    int noPassCloseWhileResurrectPendingWithInventoryPickup;
    int noPassC040EyeLiveCandidate;
    int noPassC040OwnerSwap;
    int noPassC040RedrawAfterChestClose;
} DM1_V1_MirrorCandidateReopenAfterSaveLoadProbePc34;

const char*
dm1_v1_mirror_candidate_reopen_after_save_load_source_evidence_pc34(void);
const DM1_V1_MirrorCandidateReopenAfterSaveLoadSpecPc34*
dm1_v1_mirror_candidate_reopen_after_save_load_spec_pc34(void);
int dm1_v1_mirror_candidate_reopen_after_save_load_run_pc34(
    DM1_V1_MirrorCandidateReopenAfterSaveLoadProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_REOPEN_AFTER_SAVE_LOAD_PC34_COMPAT_H */
