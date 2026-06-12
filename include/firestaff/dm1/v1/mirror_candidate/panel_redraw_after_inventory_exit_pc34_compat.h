/*
 * DM1 V1 mirror-candidate C040 panel redraw after inventory exit gate.
 *
 * Contract-only, no-asset fixture. This pins the *narrow* slice of the
 * PANEL.C F0355_INVENTORY_Toggle_CPSE(C04_CHAMPION_CLOSE_INVENTORY) close
 * path that the existing mirror-candidate family does not cover:
 *
 *   - When a C040 mirror-candidate is live (G0299_ui_CandidateChampionOrdinal
 *     is non-zero) and the player exits the inventory via F0355(C04),
 *     the engine MUST call F0334_INVENTORY_CloseChest exactly once.
 *   - The F0292_CHAMPION_DrawState redraw of the inventory champion is
 *     SKIPPED by the explicit `!G0299_ui_CandidateChampionOrdinal`
 *     gate at PANEL.C F0355:2318-2322. This gate is the *only* reason
 *     the C040 panel survives the inventory exit; if a future
 *     refactor removes the gate, the next F0292 redraw would clobber
 *     the C040 panel pixels and the resurrect-confirm flow would
 *     fall through to the food/water panel before the next F0347
 *     reroute.
 *   - The C040 panel pixels (graphic C040, command M568, F0346
 *     resurrect route) MUST remain stable across the inventory exit.
 *   - The C030 owner chain (CHAMPION.C F0300/F0301/F0302 + DEFS.H
 *     C30..C37/C38), the M070/M516 champion state, the G0425 chest
 *     list, and the candidate chain MUST be stable across the exit.
 *   - The close path ends in F0395_MENUS_DrawMovementArrows +
 *     F0357_COMMAND_DiscardAllInput + F0098_DUNGEONVIEW_DrawFloorAndCeiling.
 *     No F0282 candidate clear, no F0380 resurrect dispatch, no
 *     save/load or teleporter path runs during the exit.
 *   - When the inventory re-opens (F0355 with C05_CHAMPION_SPECIAL_INVENTORY
 *     or another champion ordinal) F0347_INVENTORY_DrawPanel MUST route
 *     to F0346_INVENTORY_DrawPanel_ResurrectReincarnate because
 *     G0299 is still non-zero. The C040 panel re-renders at the same
 *     pixels with the same graphic and command.
 *
 * Source-lock anchors (ReDMCSB WIP 20210206, PC 3.4 path, MEDIA009+):
 *   - PANEL.C F0355_INVENTORY_Toggle_CPSE:2244-2330 owns the entire
 *     close path and the candidate gate at lines 2318-2322.
 *   - PANEL.C F0334_INVENTORY_CloseChest (CHEST.C F0334) is the only
 *     close call F0355 issues on the close path.
 *   - PANEL.C F0292_CHAMPION_DrawState (CHAMDRAW.C F0292) is the
 *     inventory champion redraw that the candidate gate suppresses.
 *   - PANEL.C F0347_INVENTORY_DrawPanel:1639-1693 owns the panel
 *     re-derivation. The G0299 non-zero check at line 1654 routes to
 *     F0346_INVENTORY_DrawPanel_ResurrectReincarnate:1619-1637.
 *   - PANEL.C F0346:1626 sets G0424_i_PanelContent = M568_PANEL_RESURRECT_REINCARNATE.
 *   - PANEL.C F0395_MENUS_DrawMovementArrows is the post-exit arrow draw.
 *   - PANEL.C F0098_DUNGEONVIEW_DrawFloorAndCeiling redraws the
 *     floor/ceiling only (not the C040 panel).
 *   - COMMAND.C F0357_COMMAND_DiscardAllInput clears the input queue
 *     at the end of the close path.
 *   - REVIVE.C F0280:124-132 publishes the candidate; F0282:744-806
 *     consumes/clears it. Neither runs on the close path.
 *   - DEFS.H:2088 C30..C37/C38, G0425/G0426, C040, M568, G0299.
 *   - DEFS.H:712-716 C04_CHAMPION_CLOSE_INVENTORY.
 *   - DEFS.H:5876 G0423_i_InventoryChampionOrdinal.
 *
 * This fixture is the *close+reopen* lane; it does NOT pin:
 *   - The F0333 chest-open path (covered by c040_redraw_after_chest_close
 *     and the chest_close_while_candidate_live_non_leader gate).
 *   - The C040 panel-resurrect confirm path (covered by
 *     resurrect_chest_close_order and resurrect_reselect_with_inventory_pickup).
 *   - The C045 food/water accept cross-rotation (covered by c045_food_water_*).
 *   - The C540 scroll-wheel slot-command + chest-close + leader-rotation
 *     drain race (covered by chest_scroll_wheel_close_race and friends).
 *   - The C028 resurrect-pending non-leader pickup (covered by
 *     chest_pickup_during_resurrect_pending_non_leader).
 *   - The lower-arrow owner-ignore guard (covered by mirror panel
 *     lower-arrow guard).
 *   - The mirror-candidate double-open/close guard.
 *   - The mirror-candidate C159 click rotation combo.
 *   - The C545 food/water accept during rotation.
 *   - The C545 drop while panel live.
 *   - Save/load, teleporter, party-rotate, leader-rotation, portrait
 *     redraw, or F0354 box-variants gates.
 *
 * No bitmap sampling, no GRAPHICS.DAT / DUNGEON.DAT load, no real-asset
 * or original-DOS pixel parity claim.
 */

#ifndef FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_PANEL_REDRAW_AFTER_INVENTORY_EXIT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_PANEL_REDRAW_AFTER_INVENTORY_EXIT_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MC_PRAIE_PARTY_COUNT_PC34 4
#define DM1_V1_MC_PRAIE_CHAMPION_C30_SLOT_PC34 30
#define DM1_V1_MC_PRAIE_CHAMPION_HAND_SLOT_PC34 1
#define DM1_V1_MC_PRAIE_C30_FIRST_PC34 30
#define DM1_V1_MC_PRAIE_C30_LAST_PC34 37
#define DM1_V1_MC_PRAIE_C38_PC34 38
#define DM1_V1_MC_PRAIE_C40_PC34 40
#define DM1_V1_MC_PRAIE_M70_PC34 70
#define DM1_V1_MC_PRAIE_M516_PC34 516
#define DM1_V1_MC_PRAIE_M568_PC34 568
#define DM1_V1_MC_PRAIE_C04_CLOSE_INVENTORY_PC34 4
#define DM1_V1_MC_PRAIE_C05_SPECIAL_INVENTORY_PC34 5
#define DM1_V1_MC_PRAIE_CM1_CHAMPION_NONE_PC34 (-1)

typedef enum {
    DM1_V1_MC_PRAIE_STEP_PC34_LIVE = 0,
    DM1_V1_MC_PRAIE_STEP_PC34_EXIT = 1,
    DM1_V1_MC_PRAIE_STEP_PC34_REOPEN = 2,
    DM1_V1_MC_PRAIE_STEP_PC34_POST = 3,
    DM1_V1_MC_PRAIE_STEP_PC34_INVALID = 4
} DM1_V1_MC_PRAIE_StepPc34;

typedef struct {
    const char *chestCloseAnchor;
    const char *candidateGateAnchor;
    const char *championDrawStateAnchor;
    const char *panelDrawAnchor;
    const char *panelResurrectAnchor;
    const char *panelRedrawOnReopenAnchor;
    const char *drawMovementArrowsAnchor;
    const char *drawFloorCeilingAnchor;
    const char *discardInputAnchor;
    const char *reviveOpenAnchor;
    const char *reviveClearAnchor;
    const char *commandQueueAnchor;
    const char *defsAnchor;
    const char *contractScope;
} Dm1V1MirrorCandidatePanelRedrawAfterInventoryExitEvidencePc34;

typedef struct {
    int contractOnly;
    int disjointFromC040RedrawAfterChestClose;
    int disjointFromC040ChromeInventoryOwnerSwap;
    int disjointFromC040PanelBrowsePickupRotateRace;
    int disjointFromC040CloseNonLeaderScrollPickup;
    int disjointFromC045FoodWaterAcceptCrossRotation;
    int disjointFromC045CloseAfterNonCandidateTransition;
    int disjointFromC545PickupWhilePanelLive;
    int disjointFromC545DropWhilePanelLive;
    int disjointFromC545AcceptDuringRotation;
    int disjointFromInventoryToggle;
    int disjointFromResurrectChestCloseOrder;
    int disjointFromResurrectReselectWithInventoryPickup;
    int disjointFromResurrectConfirmInventoryInterrupt;
    int disjointFromCloseWhileResurrectPendingWithInventoryPickup;
    int disjointFromResurrectDoubleCandidateRace;
    int disjointFromResurrectCrossCandidateClear;
    int disjointFromResurrectFullC30Chain;
    int disjointFromMirrorCandidateScrollPickupLeaderRotationInventoryClick;
    int disjointFromMirrorCandidateInventoryPortraitClick;
    int disjointFromMirrorCandidatePartySwap;
    int disjointFromMirrorCandidateOpenThenReselect;
    int disjointFromMirrorCandidateReselectAfterDepositWithPartyRotate;
    int disjointFromMirrorCandidateKeyboardRotationCombo;
    int disjointFromMirrorCandidateKeyboardBrowse;
    int disjointFromMirrorCandidateC159ClickRotationCombo;
    int disjointFromMirrorCandidateFullChain;
    int disjointFromMirrorCandidateNoPendingResurrect;
    int disjointFromMirrorCandidateResurrectRearm;
    int disjointFromMirrorCandidateReincarnateRearm;
    int disjointFromMirrorCandidateThoughtProjectCancelAfterPickup;
    int disjointFromMirrorCandidateThoughtProjectTraversalOverlay;
    int disjointFromMirrorCandidateTeleporterSurvival;
    int disjointFromChampionPanelF0354BoxVariants;
    int disjointFromChampionPanelHandSlotPrioritySourceLock;
    int disjointFromChampionPanelPortraitStateRedraw;
    int disjointFromChampionPanelPortraitBoxBlitGate;
    int disjointFromChampionPanelSpellAreaOverlay;
    int disjointFromChampionPanelFoodWaterStatusBox;
    int disjointFromChampionPanelMouthEyeRelease;
    int disjointFromChampionPanelMouthEyePoisonWarning;
    int disjointFromChampionPanelPressingMouthEyeStatusbox;
    int disjointFromChampionPanelHudRecompute;
    int disjointFromChampionPanelActionHandSlotPriority;
    int disjointFromChestCloseWhilePartyRotatePickupPending;
    int disjointFromChestCloseWhileCandidateLiveNonLeader;
    int disjointFromChestScrollWheelCloseRace;
    int disjointFromChestScrollWheelResurrectConfirmation;
    int disjointFromChestResurrectRotationScrollWheel;
    int disjointFromChestOpenDuringPending;
    int disjointFromChestPickupDuringResurrectPendingNonLeader;
    int disjointFromChestDepositDuringLeaderRotation;
    int disjointFromChestPartialDropToFloorWhileChestOpen;
    int disjointFromChestInventoryC545DropToLeaderHandAlreadyOccupied;
    int disjointFromChestTeleporterSurvivalOpenG0426;
    int disjointFromMirrorCandidateScrollPickupNonLeaderPanelLive;
    int disjointFromMirrorCandidateScrollPickupWithPartyRotateInProgress;
    int disjointFromMirrorCandidatePendingHandDuringChestPickupRace;
    int disjointFromMirrorCandidatePendingHandQueue;
    int disjointFromMirrorCandidateLowerArrowGuard;
    int disjointFromMirrorCandidateClickCancel;
    int disjointFromMirrorCandidateClickCancelWithRotation;
    int disjointFromMirrorCandidateCloseButton;
    int disjointFromMirrorCandidateIconRefresh;
    int disjointFromMirrorCandidateRotationDuringResurrectConfirmation;
    int disjointFromMirrorCandidateResurrectChampionSwitchReopenRuntime;
    int disjointFromMirrorCandidateReshufflePanelLive;
    int disjointFromMirrorCandidateOccupiedHandPanel;
    int disjointFromMirrorCandidateInventoryToggle;
    int disjointFromMirrorCandidateDoubleOpenCloseGuard;
    int disjointFromMirrorCandidateChestClosePendingPanel;
    int disjointFromMirrorCandidateChestCloseLeaderHandPickup;
    int disjointFromMirrorCandidateChestOpenDuringPending;
    int disjointFromInventoryChampionSwitchHandCarry;
} Dm1V1MirrorCandidatePanelRedrawAfterInventoryExitDisjointPc34;

typedef struct {
    /* Spec metadata. */
    int partyChampionCount;
    int leaderIndex;
    int inventoryChampionOrdinal;
    int candidateOrdinal;
    int candidatePanelGraphic;
    int candidatePanelCommand;
    int candidatePanelColor;
    int candidateOwnerSlot;
    int candidateC038SlotBox;
    int closeInventorySentinel;
    int specialInventorySentinel;
    int championNoneSentinel;
    int closeChestThing;
    int resurrectSlot;
    int mouthZone;
    int mouthCommand;
    int floorAndCeilingHeight;
    int floorAndCeilingByteWidth;
    int scrollWheelSlotCommand;
    uint32_t deterministicSeed;

    /* Per-champion C30+ slot [30..37] + C38 cursor. Each entry is the
     * thing id of the slot's owner; 0xFFFF means the slot is empty. */
    int c30Owner[DM1_V1_MC_PRAIE_PARTY_COUNT_PC34];

    /* Per-champion leader hand thing. */
    int leaderHand[DM1_V1_MC_PRAIE_PARTY_COUNT_PC34];

    /* Party resting flag (F0300 chain depends on this). */
    int partyResting;

    /* G0425 chest list (C540..C547 things). */
    int g0425ChestList[8];

    /* G0426 open chest thing. */
    int g0426OpenChest;

    /* G0299 candidate ordinal (the live C040 candidate). */
    int g0299CandidateOrdinal;

    /* C040 panel state. */
    int c040PanelOpen;
    int c040PanelGraphic;
    int c040PanelCommand;
    int c040PanelColor;
    int c040PanelOwnerSlot;
    int c040PanelC038SlotBox;

    /* C545 mouth zone + C070 mouth command. */
    int mouthRouteZone;
    int mouthRouteCommand;

    /* Step machine. */
    DM1_V1_MC_PRAIE_StepPc34 step;

    /* Per-step operation counts. */
    int f0334CloseCount;
    int f0292ChampionDrawStateCount;
    int f0395DrawMovementArrowsCount;
    int f0357DiscardInputCount;
    int f0098DrawFloorCeilingCount;
    int f0347DrawPanelCount;
    int f0346ResurrectDrawCount;
    int f0077MouseEnableScreenUpdateCount;
    int f0078MouseDisableScreenUpdateCount;
    int f0326RefreshMousePointerMainLoopCount;
    int f0457StartDrawEnabledMenusCount;
    int f0280CandidatePublishCount;
    int f0282CandidateClearCount;
    int f0284ChampionSwitchCount;
    int f0297HandStateCount;
    int f0298OwnershipCount;
    int f0300ListWalkCount;
    int f0301ListWalkCount;
    int f0302ListWalkCount;
    int f0291DrawSlotCount;
    int f0352ClickOnEyeCount;
    int f0353StopPressingEyeCount;
    int f0354DrawStatusBoxPortraitCount;
    int f0358MouseCommandCount;
    int f0359MirrorQueueWriteCount;
    int f0360MirrorQueueConfirmCount;
    int f0361QueueWriteCount;
    int f0368SetLeaderCount;
    int f0378PanelRouteCount;
    int f0380QueueDispatchCount;
    int f0219WallImpactSoundCount;
    int f0333OpenCount;
    int f0334VisibleRewriteCount;
    int f0312GetStrengthCount;
    int f0226GetActionHandCount;
    int f0232DoorDestroyCount;
    int f0394SetMagicCasterCount;
    int f0401TelemetryLogCount;

    /* Stable hashes (per-step). */
    uint32_t c030ChainHash;
    uint32_t candidateChainHash;
    uint32_t chestListHash;
    uint32_t panelHash;
    uint32_t stateHashLive;
    uint32_t stateHashAfterExit;
    uint32_t stateHashAfterReopen;
    uint32_t stateHashAfterPost;
} Dm1V1MirrorCandidatePanelRedrawAfterInventoryExitStatePc34;

typedef struct {
    /* Step transition results. */
    int accepted;
    int reachedExit;
    int reachedReopen;
    int reachedPost;

    /* Live-state preservation. */
    int panelStayedC040;
    int candidateStillLive;
    int candidatePanelUnchanged;
    int candidateOwnerUnchanged;
    int candidateOwnerSlotUnchanged;
    int c030ChainPreserved;
    int leaderHandPreserved;
    int chestListPreserved;
    int g0426OpenChestStable;

    /* Operation counts after exit. */
    int f0334CloseCountAfterExit;
    int f0292ChampionDrawStateCountAfterExit;
    int f0395DrawMovementArrowsCountAfterExit;
    int f0357DiscardInputCountAfterExit;
    int f0098DrawFloorCeilingCountAfterExit;
    int f0077MouseEnableScreenUpdateCountAfterExit;
    int f0078MouseDisableScreenUpdateCountAfterExit;
    int f0326RefreshMousePointerMainLoopCountAfterExit;
    int f0457StartDrawEnabledMenusCountAfterExit;
    int f0280CandidatePublishCountAfterExit;
    int f0282CandidateClearCountAfterExit;

    /* Operation counts after reopen. */
    int f0347DrawPanelCountAfterReopen;
    int f0346ResurrectDrawCountAfterReopen;
    int f0292ChampionDrawStateCountAfterReopen;
    int f0280CandidatePublishCountAfterReopen;
    int f0282CandidateClearCountAfterReopen;

    /* Candidate gate state. */
    int candidateGateFired;
    int candidateGateCounted;
    int candidateGateSourceReachable;
    int candidateGatePanelSuppressed;
    int candidateGateChampionRedrawSuppressed;
    int noCandidateClearOnExit;

    /* Panel stability contract. */
    int panelGraphicStableAcrossExit;
    int panelCommandStableAcrossExit;
    int panelColorStableAcrossExit;
    int panelOwnerSlotStableAcrossExit;
    int panelC038SlotBoxStableAcrossExit;
    int panelMouthRouteStableAcrossExit;
    int panelGraphicRestoredAfterReopen;
    int panelCommandRestoredAfterReopen;
    int panelColorRestoredAfterReopen;

    /* Reopen reroute. */
    int reopenRoutesToF0346;
    int reopenF0346Called;
    int reopenF0346PanelContentSet;
    int reopenF0346CommandSet;
    int reopenF0346OwnerSet;
    int reopenF0346ColorSet;
    int reopenF0346SlotBoxSet;
    int reopenF0346C040Blit;
    int reopenF0346C040PanelRect;
    int reopenF0346C038SlotBoxRect;

    /* Forbidden operation counters (must stay zero). */
    int f0333OpenCountTotal;
    int f0282CandidateClearCountTotal;
    int f0457StartDrawEnabledMenusCountTotal;
    int f0360MirrorQueueConfirmCountTotal;
    int f0368SetLeaderCountTotal;
    int f0219WallImpactSoundCountTotal;
    int f0232DoorDestroyCountTotal;
    int f0394SetMagicCasterCountTotal;
    int f0401TelemetryLogCountTotal;
    int saveLoadTeleporterResurrectCommitForbidden;
    int noSaveLoad;
    int noTeleporter;
    int noResurrectCommit;
    int noResurrectCancel;
    int noChestOpen;

    /* Disjoint contract. */
    Dm1V1MirrorCandidatePanelRedrawAfterInventoryExitDisjointPc34 disjoint;

    /* Hashes. */
    uint32_t c030ChainHashLive;
    uint32_t c030ChainHashAfterExit;
    uint32_t c030ChainHashAfterReopen;
    uint32_t candidateChainHashLive;
    uint32_t candidateChainHashAfterExit;
    uint32_t candidateChainHashAfterReopen;
    uint32_t chestListHashLive;
    uint32_t chestListHashAfterExit;
    uint32_t chestListHashAfterReopen;
    uint32_t panelHashLive;
    uint32_t panelHashAfterExit;
    uint32_t panelHashAfterReopen;
    uint32_t panelHashAfterPost;
    uint32_t deterministicHash;
} Dm1V1MirrorCandidatePanelRedrawAfterInventoryExitResultPc34;

void dm1_v1_mirror_candidate_panel_redraw_after_inventory_exit_init_pc34(
    Dm1V1MirrorCandidatePanelRedrawAfterInventoryExitStatePc34 *state);

int dm1_v1_mirror_candidate_panel_redraw_after_inventory_exit_run_pc34(
    Dm1V1MirrorCandidatePanelRedrawAfterInventoryExitStatePc34 *state,
    Dm1V1MirrorCandidatePanelRedrawAfterInventoryExitResultPc34 *result);

const Dm1V1MirrorCandidatePanelRedrawAfterInventoryExitEvidencePc34 *
dm1_v1_mirror_candidate_panel_redraw_after_inventory_exit_evidence_pc34(
    void);

const char *
dm1_v1_mirror_candidate_panel_redraw_after_inventory_exit_source_evidence_pc34(
    void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_PANEL_REDRAW_AFTER_INVENTORY_EXIT_PC34_COMPAT_H */
