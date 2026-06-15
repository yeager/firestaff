#ifndef DM1_V1_MIRROR_CANDIDATE_LOWER_ARROW_STATE_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_LOWER_ARROW_STATE_PC34_COMPAT_H

#include <stdint.h>

/*
 * DM1 V1 mirror-candidate lower movement arrow state gate
 *
 * Contract-only, no-asset fixture. This pins the *narrow* slice of the
 * PANEL.C / COMMAND.C path that the existing mirror-candidate family
 * does not cover: the G0424 panel state survives an incoming C004 /
 * C005 / C006 lower movement arrow click while a C040 mirror-candidate
 * panel is supposed to be live. In the ReDMCSB WIP20210206 checkout
 * the lower movement arrow row (y=147-167 in the G0463 graphic-561
 * movement arrows box) is owned by G0448_SecondaryMouseInput_Movement
 * and routes to F0366_COMMAND_ProcessTypes3To6_MoveParty on the
 * C003..C006 range without an explicit `!G0299_ui_CandidateChampionOrdinal`
 * guard. The contract this gate pins is the *narrow* statement that
 * even though the dispatch reaches F0366, the C040 panel state MUST
 * stay byte-stable across the lower-arrow click:
 *
 *   - G0424_i_PanelContent stays M568_PANEL_RESURRECT_REINCARNATE.
 *   - The c040PanelOpen flag stays 1.
 *   - The c040PanelGraphic stays 40 (C040_GRAPHIC_PANEL_RESURRECT_REINCARNATE).
 *   - The c040PanelCommand stays 568 (M568_PANEL_RESURRECT_REINCARNATE).
 *   - The c040PanelOwnerSlot stays at the published candidate owner.
 *   - The c040PanelC038SlotBox anchor stays at 38 (C38 panel slot box).
 *   - G0299_ui_CandidateChampionOrdinal stays non-zero (no F0282 clear).
 *   - The owner-slot chain (C30..C37) stays byte-stable.
 *   - The F0380_COMMAND_ProcessQueue_CPSC dispatch is the
 *     *only* dispatch this lane observes; no F0282 / F0334 / F0355
 *     close path, no save/load, no teleporter, no resurrect-commit,
 *     no party-rotate, no leader-rotation entry fires on the
 *     lower-arrow click.
 *   - The F0346_INVENTORY_DrawPanel_ResurrectReincarnate redraw
 *     path remains the *only* reachable C040 route, not the F0342
 *     inventory object path or the F0345 food/water poisoned path.
 *   - The F0366_COMMAND_ProcessTypes3To6_MoveParty entry is recorded
 *     exactly once per C004/C005/C006 lower-arrow click but does NOT
 *     mutate G0299, G0424, the c040Panel* fields, or the owner chain.
 *   - The 8 G0425_aT_ChestSlots stay all-zero and G0426_T_OpenChest
 *     stays 0 across the lower-arrow click (no chest dispatch in this
 *     lane).
 *   - The F0395_MENUS_DrawMovementArrows call (post-close arrow draw
 *     in the F0355 path) is NOT entered on the lower-arrow click
 *     (the C040 panel is not closed on this path).
 *   - When the input is something other than a lower arrow (C001
 *     turn, C002 turn, C003 forward, C007 inventory toggle, C011
 *     close-inventory, C160/C161/C162 panel command) the F0366
 *     entry is rejected and the F0380 dispatch returns with the
 *     C040 panel state still byte-stable.
 *
 * Source-lock anchors (ReDMCSB WIP 20210206, PC 3.4 path, MEDIA009+):
 *   - COMMAND.C:107-112 G0448_SecondaryMouseInput_Movement maps the
 *     lower row y=147-167 to C006/C005/C004 MOVE LEFT/BACKWARD/RIGHT.
 *   - COMMAND.C:323-328 G0463_aai_Graphic561_Box_MovementArrows
 *     defines the movement arrow boxes (top row y=125-145 = C001/
 *     C003/C002, lower row y=147-167 = C006/C005/C004).
 *   - COMMAND.C F0380:2151-2156 routes the C003..C006 range to
 *     F0366_COMMAND_ProcessTypes3To6_MoveParty without an explicit
 *     `!G0299` guard; this is the lower-arrow state-gate contract.
 *   - COMMAND.C F0380:2159-2181 keeps the !G0299 guard on the
 *     C012..C015 status-box / C007..C011 inventory-toggle range.
 *   - COMMAND.C F0380:2302-2311 keeps the !G0299 guard on the
 *     C100 spell and C111 action-area range.
 *   - COMMAND.C F0380:2366-2370 keeps the !G0299 guard on the
 *     C140 save input range.
 *   - CLIKMENU.C F0366_COMMAND_ProcessTypes3To6_MoveParty:180-280
 *     walks champions, decrements stamina, advances the party map
 *     square; it does NOT touch G0299 / G0424 / C040 panel state.
 *   - PANEL.C F0346_INVENTORY_DrawPanel_ResurrectReincarnate:1619-1637
 *     sets G0424_i_PanelContent = M568_PANEL_RESURRECT_REINCARNATE
 *     and blits the C040 graphic on the G0032_ai_Graphic562_Box_Panel
 *     rect; the F0346 route is the *only* C040 redraw path while
 *     G0299 is non-zero.
 *   - PANEL.C F0347_INVENTORY_DrawPanel:1654 routes to F0346 when
 *     G0299 is non-zero.
 *   - REVIVE.C F0280:124-132 publishes the candidate; F0282:744-806
 *     consumes/clears it; neither runs on the lower-arrow click.
 *   - DEFS.H:2088 C30..C37/C38, G0425/G0426, C040, M568, G0299.
 *   - DEFS.H:2200 C040_GRAPHIC_PANEL_RESURRECT_REINCARNATE.
 *   - DEFS.H:3001-3008 M568_PANEL_RESURRECT_REINCARNATE.
 *   - DEFS.H:5694 G0299_ui_CandidateChampionOrdinal.
 *   - DEFS.H:5876-5881 G0425/G0426 chest list + open chest.
 *   - DEFS.H:4041-4042 viewport wall zones (C004/C005/C006 lower-row
 *     movement arrows are screen-relative, not viewport-relative).
 *
 * Disjoint contract — this lane does NOT pin:
 *   - The C004..C006 click-through / leader-hand / chest-open path
 *     (covered by chest_scroll_wheel_* and the C540 / C061 chest
 *     drop-rotation tests).
 *   - The C159 name-row C016 set-leader click while C040 is live
 *     (covered by c159_click_rotation_combo).
 *   - The C007..C011 inventory toggle while C040 is live (covered
 *     by panel_redraw_after_inventory_exit which is the
 *     F0355(C04) close path that returns to the G0448 movement
 *     secondary mouse input and triggers F0395_MENUS_DrawMovementArrows).
 *   - The C100 spell-area / C111 action-area `!G0299` guard (covered
 *     by c159_click_rotation_combo's `commandSpellActionGuardAnchor`).
 *   - The C140 save-game `!G0299` guard (covered by
 *     c159_click_rotation_combo's `commandSaveGuardAnchor`).
 *   - The C160/C161/C162 resurrect/reincarnate/cancel panel dispatch
 *     (covered by c160_close_while_rotation_pending, c159_click_rotation_combo,
 *     resurrect_chest_close_order, and resurrect_reselect_with_inventory_pickup).
 *   - The C070 mouth / C071 eye click while C040 is live (covered
 *     by c040_eye_live_candidate).
 *   - The C028..C057 inventory slot box click while C040 is live
 *     (covered by c040_close_non_leader_scroll_pickup, c040_panel_browse_pickup_rotate_race,
 *     and panel_redraw_after_inventory_exit).
 *   - The C580-C645 chest close / open / scroll-wheel / resurrect-rotation
 *     / teleporter / save-load / mirror-candidate pickup paths.
 *   - The C160/C161/C162 panel-command replay after a C045 stale route
 *     (covered by close_while_c045_pending).
 *   - Save/load, teleporter, party-rotate, leader-rotation, portrait
 *     redraw, F0354 box-variants, F0334 chest close, or F0333 chest open
 *     entries on the lower-arrow click.
 *   - The F0457_START_DrawEnabledMenus_CPSF post-resurrect orchestrator
 *     redraw (this lane does not enter the F0457 path).
 *   - The F0456_START_DrawDisabledMenus post-rest orchestrator.
 *   - The F0283_CHAMPION_ViAltarRebirth / F0281_CHAMPION_Rename paths.
 *
 * No bitmap sampling, no GRAPHICS.DAT / DUNGEON.DAT load, no real-asset
 * or original-DOS pixel parity claim.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Deterministic seed baseline. The seed high word is the lane id
 * shorthand (0xC004 = lower movement arrow C004_COMMAND_MOVE_RIGHT) and
 * the low word is the C038 neck slot box ordinal (0x0038 = 56). */
#define DM1_V1_MC_LAS_PARTY_COUNT_PC34 4
#define DM1_V1_MC_LAS_CHAMPION_C30_FIRST_PC34 30
#define DM1_V1_MC_LAS_CHAMPION_C30_LAST_PC34 37
#define DM1_V1_MC_LAS_CHAMPION_C38_PC34 38
#define DM1_V1_MC_LAS_C04_PC34 4
#define DM1_V1_MC_LAS_C05_PC34 5
#define DM1_V1_MC_LAS_C30_PC34 30
#define DM1_V1_MC_LAS_C38_PC34 38
#define DM1_V1_MC_LAS_C40_GRAPHIC_PC34 40
#define DM1_V1_MC_LAS_C159_PC34 159
#define DM1_V1_MC_LAS_C160_PC34 160
#define DM1_V1_MC_LAS_C161_PC34 161
#define DM1_V1_MC_LAS_C162_PC34 162
#define DM1_V1_MC_LAS_M70_PC34 70
#define DM1_V1_MC_LAS_M516_PC34 516
#define DM1_V1_MC_LAS_M568_PANEL_PC34 568
#define DM1_V1_MC_LAS_CM1_CHAMPION_NONE_PC34 (-1)

#define DM1_V1_MC_LAS_COMMAND_NONE_PC34 0
#define DM1_V1_MC_LAS_COMMAND_MOVE_FORWARD_PC34 3
#define DM1_V1_MC_LAS_COMMAND_MOVE_RIGHT_PC34 4
#define DM1_V1_MC_LAS_COMMAND_MOVE_BACKWARD_PC34 5
#define DM1_V1_MC_LAS_COMMAND_MOVE_LEFT_PC34 6
#define DM1_V1_MC_LAS_COMMAND_CLOSE_INVENTORY_PC34 11
#define DM1_V1_MC_LAS_COMMAND_SPELL_AREA_PC34 100
#define DM1_V1_MC_LAS_COMMAND_ACTION_AREA_PC34 111
#define DM1_V1_MC_LAS_COMMAND_SAVE_GAME_PC34 140
#define DM1_V1_MC_LAS_COMMAND_REST_PC34 145

#define DM1_V1_MC_LAS_CHEST_SLOT_COUNT_PC34 8
#define DM1_V1_MC_LAS_DETERMINISTIC_SEED_PC34 0xC0040038u

typedef struct {
    const char *movementMouseTableAnchor;
    const char *movementArrowBoxesAnchor;
    const char *commandQueueMoveForwardAnchor;
    const char *commandQueueMoveRangeAnchor;
    const char *commandQueueStatusInventoryGuardAnchor;
    const char *commandQueueSpellActionGuardAnchor;
    const char *commandQueueSaveGuardAnchor;
    const char *panelC040RedrawAnchor;
    const char *panelResurrectAnchor;
    const char *panelDrawRouteAnchor;
    const char *reviveCandidatePublishAnchor;
    const char *reviveCandidateClearAnchor;
    const char *movePartyNoPanelStateAnchor;
    const char *defsPanelContentAnchor;
    const char *defsPanelGraphicAnchor;
    const char *defsCandidateOrdinalAnchor;
    const char *defsChestListAnchor;
    const char *movementArrowRowYAnchor;
    const char *contractScope;
} Dm1V1MirrorCandidateLowerArrowStateEvidencePc34;

typedef struct {
    int contractOnly;
    int noGameDataRequired;
    int disjointFromC159ClickRotationCombo;
    int disjointFromC160CloseWhileRotationPending;
    int disjointFromPanelRedrawAfterInventoryExit;
    int disjointFromC040CloseNonLeaderScrollPickup;
    int disjointFromC040PanelBrowsePickupRotateRace;
    int disjointFromC040RedrawAfterChestClose;
    int disjointFromC040EyeLiveCandidate;
    int disjointFromCloseWhileC045Pending;
    int disjointFromC545AcceptDuringRotation;
    int disjointFromC545DropWhilePanelLive;
    int disjointFromC545PickupWhilePanelLive;
    int disjointFromResurrectChestCloseOrder;
    int disjointFromResurrectConfirmInventoryInterrupt;
    int disjointFromResurrectReincarnateSkills;
    int disjointFromResurrectReselectWithInventoryPickup;
    int disjointFromResurrectDoubleCandidateRace;
    int disjointFromResurrectCrossCandidateClear;
    int disjointFromResurrectFullC30Chain;
    int disjointFromChestCloseWhilePartyRotatePickupPending;
    int disjointFromChestCloseWhileCandidateLiveNonLeader;
    int disjointFromChestScrollWheelCloseRace;
    int disjointFromChestScrollWheelResurrectConfirmation;
    int disjointFromChestResurrectRotationScrollWheel;
    int disjointFromChestOpenDuringPending;
    int disjointFromChestPickupDuringResurrectPendingNonLeader;
    int disjointFromChestDepositDuringLeaderRotation;
    int disjointFromMirrorCandidateSaveLoad;
    int disjointFromMirrorCandidateTeleporterSurvival;
    int disjointFromMirrorCandidateCloseButton;
    int disjointFromMirrorCandidateClickCancel;
    int disjointFromMirrorCandidateClickCancelWithRotation;
    int disjointFromMirrorCandidateIconRefresh;
    int disjointFromMirrorCandidateDoubleOpenCloseGuard;
    int disjointFromMirrorCandidateInventoryToggle;
    int disjointFromMirrorCandidateInventoryPortraitClick;
    int disjointFromMirrorCandidatePartySwap;
    int disjointFromMirrorCandidateRotationDuringResurrectConfirmation;
    int disjointFromMirrorCandidateResurrectChampionSwitchReopen;
    int disjointFromMirrorCandidateReshufflePanelLive;
    int disjointFromMirrorCandidateOccupiedHandPanel;
    int disjointFromChampionPanelF0354BoxVariants;
    int disjointFromChampionPanelHandSlotPrioritySourceLock;
    int disjointFromChampionPanelPortraitStateRedraw;
    int disjointFromChampionPanelPortraitBoxBlitGate;
    int disjointFromChampionPanelSpellAreaOverlay;
    int disjointFromChampionPanelHudFoodWaterRecompute;
    int disjointFromRoomTransitionRedrawOnly;
    int disjointFromStairsInventoryState;
    int disjointFromDoorBashFeedbackSourceLock;
    int disjointFromWallImpactProjectileSound;
    int disjointFromSaveLoadContract;
    int disjointFromTeleporterContract;
    int disjointFromLeaderRotationContract;
    int disjointFromPartyRotateContract;
    int disjointFromResurrectCommitContract;
    int disjointFromResurrectCancelContract;
    int disjointFromChestOpenContract;
    int disjointFromChestCloseContract;
} Dm1V1MirrorCandidateLowerArrowStateDisjointPc34;

typedef struct {
    int contractOnly;
    int noGameDataRequired;
    int partyChampionCount;
    int leaderIndex;
    int inventoryChampionOrdinal;
    int inventoryPanelOpen;
    int candidateOrdinal;
    int candidateOwnerIndex;
    int candidateOwnerSlot;
    int c040PanelOpen;
    int c040PanelGraphic;
    int c040PanelCommand;
    int c040PanelColor;
    int c040PanelOwnerSlot;
    int c040PanelC038SlotBox;
    int panelContent;
    int leaderHandEmpty;
    int leaderHandThingOrdinal;
    int g0426OpenChest;
    int chestSlots[DM1_V1_MC_LAS_CHEST_SLOT_COUNT_PC34];
    int partyResting;
    int openChestThing;
    int f0333OpenCount;
    int f0334CloseCount;
    int f0282CandidateClearCount;
    int f0280CandidatePublishCount;
    int f0292ChampionDrawStateCount;
    int f0300ListWalkCount;
    int f0301ListWalkCount;
    int f0302ListWalkCount;
    int f0342DrawPanelObjectCount;
    int f0345FoodWaterPoisonedCount;
    int f0346C040DrawCount;
    int f0347PanelDrawCount;
    int f0355ToggleSuppressedByCandidateCount;
    int f0359FreshClickCount;
    int f0360PendingReplayCount;
    int f0366MovePartyEnterCount;
    int f0367StatusBoxClickCount;
    int f0368SetLeaderCount;
    int f0378PanelRouteCount;
    int f0380DrainCount;
    int f0098FloorCeilingDrawCount;
    int f0326MousePointerRefreshCount;
    int f0395DrawMovementArrowsCount;
    int f0457StartDrawEnabledMenusCount;
    int f0219WallImpactSoundCount;
    int f0232DoorDestroyCount;
    int f0334VisibleRewriteCount;
    int saveLoadCount;
    int teleporterCount;
    int partyRotateCount;
    int championRotationPendingCount;
    int f0283ViAltarRebirthCount;
    int f0281ChampionRenameCount;
    uint32_t c030Owner[DM1_V1_MC_LAS_PARTY_COUNT_PC34];
    uint32_t c030OwnerHash;
    uint32_t c040PanelHash;
    uint32_t candidateChainHash;
    uint32_t chestListHash;
    uint32_t leaderHandHash;
    uint32_t deterministicSeed;
    uint32_t lastAcceptedLowerArrowCommand;
} Dm1V1MirrorCandidateLowerArrowStateStatePc34;

typedef struct {
    int accepted;
    int reachedF0366MoveParty;
    int reachedF0380QueueDrain;
    int lowerArrowC004Accepted;
    int lowerArrowC005Accepted;
    int lowerArrowC006Accepted;
    int upperArrowC001Accepted;
    int upperArrowC002Accepted;
    int upperArrowC003Accepted;
    int inventoryToggleC007Accepted;
    int closeInventoryC011Accepted;
    int spellAreaC100Accepted;
    int actionAreaC111Accepted;
    int saveGameC140Accepted;
    int restC145Accepted;
    int panelCommandC160Accepted;
    int panelCommandC161Accepted;
    int panelCommandC162Accepted;
    int panelStayedC040;
    int panelContentStayedM568;
    int panelGraphicStayed40;
    int panelCommandStayed568;
    int panelOwnerSlotStayedCandidateOwner;
    int panelC038SlotBoxStayed38;
    int candidateOrdinalPreserved;
    int panelOpenPreserved;
    int ownerChainPreserved;
    int leaderHandPreserved;
    int chestListPreserved;
    int noPanelContentMutation;
    int noF0282ClearOnLowerArrow;
    int noF0333OpenOnLowerArrow;
    int noF0334CloseOnLowerArrow;
    int noF0345FoodWaterOnLowerArrow;
    int noF0355ToggleOnLowerArrow;
    int noF0342ObjectOnLowerArrow;
    int noF0395MovementArrowsOnLowerArrow;
    int noF0457StartDrawEnabledOnLowerArrow;
    int noF0219WallImpactOnLowerArrow;
    int noF0232DoorDestroyOnLowerArrow;
    int noF0283ViAltarRebirthOnLowerArrow;
    int noF0281ChampionRenameOnLowerArrow;
    int noSaveLoadOnLowerArrow;
    int noTeleporterOnLowerArrow;
    int noPartyRotateOnLowerArrow;
    int noLeaderRotationOnLowerArrow;
    int noResurrectCommitOnLowerArrow;
    int noResurrectCancelOnLowerArrow;
    int c040PanelHashStable;
    int candidateChainHashStable;
    int c030OwnerHashStable;
    int leaderHandHashStable;
    int chestListHashStable;
    int f0380DrainCountRecorded;
    int f0366MovePartyCountRecorded;
    int noF0334VisibleRewriteOnLowerArrow;
    int f0359FreshClickCountRecorded;
    int f0360PendingReplayCountRecorded;
    Dm1V1MirrorCandidateLowerArrowStateDisjointPc34 disjoint;
    uint32_t deterministicHash;
} Dm1V1MirrorCandidateLowerArrowStateResultPc34;

void dm1_v1_mirror_candidate_lower_arrow_state_init_pc34(
    Dm1V1MirrorCandidateLowerArrowStateStatePc34 *state);

int dm1_v1_mirror_candidate_lower_arrow_state_publish_candidate_pc34(
    Dm1V1MirrorCandidateLowerArrowStateStatePc34 *state);

int dm1_v1_mirror_candidate_lower_arrow_state_dispatch_pc34(
    Dm1V1MirrorCandidateLowerArrowStateStatePc34 *state,
    int command,
    Dm1V1MirrorCandidateLowerArrowStateResultPc34 *result);

const Dm1V1MirrorCandidateLowerArrowStateEvidencePc34 *
dm1_v1_mirror_candidate_lower_arrow_state_evidence_pc34(void);

const char *
dm1_v1_mirror_candidate_lower_arrow_state_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_MIRROR_CANDIDATE_LOWER_ARROW_STATE_PC34_COMPAT_H */
