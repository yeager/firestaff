/*
 * DM1 V1 mirror-candidate HUD-overlay restoration across inventory exit
 * gate.
 *
 * Contract-only, no-asset fixture. This pins the *HUD-overlay*
 * (champion status boxes C151..C154 + portrait boxes C155..C158 +
 * champion icon cells M026 + action hand M070 + action icon C020 +
 * HP/Stamina/Mana bars C027..C029 + food/water/mouth/eye warning
 * C545/C033..C039 + poison warning) slice of the PANEL.C
 * F0355_INVENTORY_Toggle_CPSE(C04_CHAMPION_CLOSE_INVENTORY) close path
 * that the existing mirror-candidate family does NOT cover:
 *
 *   - When a C040 mirror-candidate is live (G0299_ui_CandidateChampionOrdinal
 *     non-zero) and the player exits the inventory via F0355(C04), the
 *     engine MUST NOT call F0292_CHAMPION_DrawState for ANY of the four
 *     party champions. The candidate gate at PANEL.C F0355:2318-2322
 *     suppresses the inventory-champion redraw; the close path itself
 *     does not iterate the other three champions. The HUD status boxes
 *     C151..C154, portrait boxes C155..C158, action hand M070, action
 *     icon C020, bars C027..C029, and food/water/mouth/eye C545/C033..C039
 *     must therefore all stay byte-identical across the inventory exit.
 *   - The HUD-overlay restoration on REOPEN must re-route through
 *     PANEL.C F0347:1654 (G0299 non-zero check) to F0346 resurrect draw
 *     which repaints the C040 panel graphic at the same pixels; the HUD
 *     overlay for the four champions must still be byte-identical across
 *     the reopen (no F0293_F0292 cascade on reopen).
 *   - The C040 panel pixels (graphic C040, command M568, F0346 resurrect
 *     route) MUST remain stable across the inventory exit. This is the
 *     *narrow* HUD-side complement to the existing
 *     panel_redraw_after_inventory_exit gate: that gate pins the C040
 *     panel + C030 chain + G0425/G0426 chest stability; this gate pins
 *     the per-champion HUD-overlay stability that the close path must
 *     not touch.
 *   - The close path ends in F0395_MENUS_DrawMovementArrows +
 *     F0357_COMMAND_DiscardAllInput + F0098_DUNGEONVIEW_DrawFloorAndCeiling
 *     + F0326_B_RefreshMousePointerInMainLoop. None of these redraw the
 *     HUD overlay.
 *   - When the inventory re-opens (F0355 with C05_CHAMPION_SPECIAL_INVENTORY
 *     or another champion ordinal) F0347_INVENTORY_DrawPanel MUST route
 *     to F0346_INVENTORY_DrawPanel_ResurrectReincarnate because G0299 is
 *     still non-zero. The C040 panel re-renders at the same pixels with
 *     the same graphic and command. The HUD overlay for the four
 *     champions must stay byte-identical (no F0292 cascade, no
 *     F0293_CHAMPION_DrawAllChampionStates cascade, no F0296
 *     DrawChangedObjectIcons cascade, no F0335 redraw).
 *   - The auto-restore invariant: G0299 is NOT cleared on the close
 *     path. F0282_CHAMPION_ProcessCommands160To162 must NOT run on the
 *     close path. F0280_CHAMPION_AddCandidateChampionToParty runs at
 *     publish time only (REVIVE.C:124-132). When the inventory re-opens,
 *     the C040 chain auto-restores from G0299 + M516_CHAMPIONS +
 *     G0424_i_PanelContent = M568_PANEL_RESURRECT_REINCARNATE.
 *
 * Source-lock anchors (ReDMCSB WIP 20210206, PC 3.4 path, MEDIA009+):
 *   - PANEL.C F0355_INVENTORY_Toggle_CPSE:2244-2330 owns the entire
 *     close path and the candidate gate at lines 2318-2322.
 *   - PANEL.C F0334_INVENTORY_CloseChest (CHEST.C F0334) is the only
 *     close call F0355 issues on the close path. It closes the chest
 *     list and never touches the HUD overlay.
 *   - PANEL.C F0292_CHAMPION_DrawState (CHAMDRAW.C F0292:703-1110) is
 *     the inventory-champion redraw that the candidate gate suppresses
 *     AND is the only function in the close path that redraws HUD
 *     chrome. Its suppression keeps the HUD status box / portrait /
 *     action hand / bars / food/water/mouth/eye pixels stable.
 *   - PANEL.C F0347_INVENTORY_DrawPanel:1639-1693 owns the panel
 *     re-derivation. The G0299 non-zero check at line 1654 routes to
 *     F0346_INVENTORY_DrawPanel_ResurrectReincarnate:1619-1637.
 *   - PANEL.C F0346:1626 sets G0424_i_PanelContent = M568_PANEL_RESURRECT_REINCARNATE.
 *   - PANEL.C F0395_MENUS_DrawMovementArrows is the post-exit arrow
 *     draw (movement arrows, NOT HUD overlay).
 *   - PANEL.C F0098_DUNGEONVIEW_DrawFloorAndCeiling redraws the
 *     floor/ceiling only (NOT HUD overlay).
 *   - PANEL.C F0293_CHAMPION_DrawAllChampionStates (CHAMDRAW.C:1117)
 *     cascades F0292 for all four champions and is NOT called on the
 *     close path; it must not be called during the exit either.
 *   - PANEL.C F0296_CHAMPION_DrawChangedObjectIcons (CHAMDRAW.C:1185)
 *     draws the four champion icons C113..C116 and is NOT called on
 *     the close path; it must not be called during the exit either.
 *   - PANEL.C F0354_INVENTORY_DrawStatusBoxPortrait (PANEL.C:2195-2242)
 *     is the status-box portrait blit dispatch; it is NOT called from
 *     F0355 close path. Its dispatch contract is pinned by the
 *     champion_panel_f0354_box_variants gate (different gate).
 *   - COMMAND.C F0357_COMMAND_DiscardAllInput clears the input queue
 *     at the end of the close path.
 *   - COMMAND.C F0326_B_RefreshMousePointerInMainLoop is set true on
 *     the close path (mouse pointer refresh, NOT HUD overlay).
 *   - IO.C F0077_MOUSE_EnableScreenUpdate_CPSE /
 *     F0078_MOUSE_DisableScreenUpdate are the screen update toggle on
 *     the close path.
 *   - REVIVE.C F0280:124-132 publishes the candidate (initial only).
 *     F0282:744-806 consumes/clears it. Neither runs on the close
 *     path; F0282 must NOT run during the close or the candidate
 *     would be cleared and the C040 panel would fall through to the
 *     food/water panel on reopen.
 *   - DEFS.H:2088 C30..C37/C38, G0425/G0426, C040, M568, G0299.
 *   - DEFS.H:712-716 C04_CHAMPION_CLOSE_INVENTORY.
 *   - DEFS.H:5876 G0423_i_InventoryChampionOrdinal.
 *   - DEFS.H:5876-5881 G0425/G0426 chest list + open chest.
 *   - DEFS.H:3793 C175_ZONE_FIRST_CHAMPION_STATUS_BOX (= 175; C151..C154
 *     champion status boxes are C175..C178; C155..C158 portrait boxes
 *     are C179..C182; C113..C116 champion icons are C113..C116).
 *   - DEFS.H:2157 C69_CHAMPION_STATUS_BOX_SPACING (= 69).
 *   - DEFS.H:2471 C016_BYTE_WIDTH (= 16 screen bytes per row).
 *   - DEFS.H:1874-1878 M070_HAND_SLOT_INDEX.
 *   - DEFS.H:1878 M070.
 *
 * This fixture is the *HUD-overlay restoration* lane; it does NOT pin:
 *   - The C040 panel pixel survival (covered by
 *     panel_redraw_after_inventory_exit_pc34_compat gate).
 *   - The C030 chain + G0425 chest stability (covered by the same
 *     gate).
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
 *   - The F0354 box-variants geometry (covered by
 *     champion_panel_f0354_box_variants gate).
 *   - The F0292 -> F0354 dispatch predicate (covered by
 *     champion_panel_portrait_box_blit_gate).
 *   - The HUD champion-panel status box redraw cascade (covered by
 *     clock_tick_stat_repaint + hud_food_water_recompute + name_box_clip
 *     gates).
 *   - Save/load, teleporter, party-rotate, leader-rotation,
 *     resurrect-commit / resurrect-cancel, F0333 chest open side
 *     effects.
 *
 * No bitmap sampling, no GRAPHICS.DAT / DUNGEON.DAT load, no real-asset
 * or original-DOS pixel parity claim.
 */

#ifndef FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_AUTO_MIRROR_CANDIDATE_INVENTORY_EXIT_RESTORE_HUD_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_AUTO_MIRROR_CANDIDATE_INVENTORY_EXIT_RESTORE_HUD_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MC_AMCIERH_PARTY_COUNT_PC34 4
#define DM1_V1_MC_AMCIERH_CHAMPION_C30_FIRST_PC34 30
#define DM1_V1_MC_AMCIERH_CHAMPION_C30_LAST_PC34 37
#define DM1_V1_MC_AMCIERH_CHAMPION_C38_PC34 38
#define DM1_V1_MC_AMCIERH_CHAMPION_C40_PC34 40
#define DM1_V1_MC_AMCIERH_M70_HAND_SLOT_PC34 70
#define DM1_V1_MC_AMCIERH_M516_CHAMPIONS_PC34 516
#define DM1_V1_MC_AMCIERH_M568_PANEL_RESURRECT_REINCARNATE_PC34 568
#define DM1_V1_MC_AMCIERH_M70_HAND_SLOT_INDEX_PC34 70
#define DM1_V1_MC_AMCIERH_M518_CHAMPION_ICON_PC34 518
#define DM1_V1_MC_AMCIERH_M519_BLIT_TO_VIEWPORT_PC34 519
#define DM1_V1_MC_AMCIERH_M026_CHAMPION_ICON_CELL_PC34 26
#define DM1_V1_MC_AMCIERH_C04_CLOSE_INVENTORY_PC34 4
#define DM1_V1_MC_AMCIERH_C05_SPECIAL_INVENTORY_PC34 5
#define DM1_V1_MC_AMCIERH_C027_BAR_HP_PC34 27
#define DM1_V1_MC_AMCIERH_C028_BAR_STAMINA_PC34 28
#define DM1_V1_MC_AMCIERH_C029_BAR_MANA_PC34 29
#define DM1_V1_MC_AMCIERH_C020_ACTION_ICON_PC34 20
#define DM1_V1_MC_AMCIERH_C033_MOUTH_WARN_PC34 33
#define DM1_V1_MC_AMCIERH_C034_EYE_WARN_PC34 34
#define DM1_V1_MC_AMCIERH_C039_FOOD_WARN_PC34 39
#define DM1_V1_MC_AMCIERH_C545_FOOD_WATER_WARN_PC34 545
#define DM1_V1_MC_AMCIERH_C151_CHAMPION_0_STATUS_PC34 151
#define DM1_V1_MC_AMCIERH_C152_CHAMPION_1_STATUS_PC34 152
#define DM1_V1_MC_AMCIERH_C153_CHAMPION_2_STATUS_PC34 153
#define DM1_V1_MC_AMCIERH_C154_CHAMPION_3_STATUS_PC34 154
#define DM1_V1_MC_AMCIERH_C155_CHAMPION_0_PORTRAIT_PC34 155
#define DM1_V1_MC_AMCIERH_C158_CHAMPION_3_PORTRAIT_PC34 158
#define DM1_V1_MC_AMCIERH_C113_CHAMPION_0_ICON_PC34 113
#define DM1_V1_MC_AMCIERH_C116_CHAMPION_3_ICON_PC34 116
#define DM1_V1_MC_AMCIERH_C175_FIRST_STATUS_BOX_PC34 175
#define DM1_V1_MC_AMCIERH_C69_STATUS_BOX_SPACING_PC34 69
#define DM1_V1_MC_AMCIERH_C016_BYTE_WIDTH_PC34 16
#define DM1_V1_MC_AMCIERH_STATUS_BOX_WIDTH_PC34 67
#define DM1_V1_MC_AMCIERH_STATUS_BOX_HEIGHT_PC34 29
#define DM1_V1_MC_AMCIERH_PORTRAIT_WIDTH_PC34 32
#define DM1_V1_MC_AMCIERH_PORTRAIT_HEIGHT_PC34 29
#define DM1_V1_MC_AMCIERH_BAR_WIDTH_PC34 18
#define DM1_V1_MC_AMCIERH_BAR_HEIGHT_PC34 3
#define DM1_V1_MC_AMCIERH_HAND_SLOT_WIDTH_PC34 18
#define DM1_V1_MC_AMCIERH_HAND_SLOT_HEIGHT_PC34 18
#define DM1_V1_MC_AMCIERH_CHAMPION_NONE_PC34 (-1)

typedef enum {
    DM1_V1_MC_AMCIERH_STEP_PC34_LIVE = 0,
    DM1_V1_MC_AMCIERH_STEP_PC34_EXIT = 1,
    DM1_V1_MC_AMCIERH_STEP_PC34_REOPEN = 2,
    DM1_V1_MC_AMCIERH_STEP_PC34_POST = 3,
    DM1_V1_MC_AMCIERH_STEP_PC34_INVALID = 4
} DM1_V1_MC_AMCIERH_StepPc34;

typedef struct {
    const char *hudStatusBoxAnchor;
    const char *hudPortraitBoxAnchor;
    const char *hudChampionIconAnchor;
    const char *hudActionHandAnchor;
    const char *hudActionIconAnchor;
    const char *hudBarsAnchor;
    const char *hudFoodWaterAnchor;
    const char *hudMouthEyeAnchor;
    const char *closeInventoryAnchor;
    const char *candidateGateAnchor;
    const char *drawMovementArrowsAnchor;
    const char *drawFloorCeilingAnchor;
    const char *discardInputAnchor;
    const char *reviveOpenAnchor;
    const char *reviveClearAnchor;
    const char *panelDrawAnchor;
    const char *panelResurrectAnchor;
    const char *defsAnchor;
    const char *contractScope;
} Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudEvidencePc34;

typedef struct {
    /* Disjoint contract. */
    int contractOnly;
    int disjointFromPanelRedrawAfterInventoryExit;
    int disjointFromC040RedrawAfterChestClose;
    int disjointFromC040ChromeInventoryOwnerSwap;
    int disjointFromC040PanelBrowsePickupRotateRace;
    int disjointFromC040CloseNonLeaderScrollPickup;
    int disjointFromC045FoodWaterAcceptCrossRotation;
    int disjointFromC045CloseAfterNonCandidateTransition;
    int disjointFromC045FoodWaterCloseNoCandidate;
    int disjointFromC545PickupWhilePanelLive;
    int disjointFromC545DropWhilePanelLive;
    int disjointFromC545AcceptDuringRotation;
    int disjointFromC545FoodWaterAcceptCrossRotation;
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
    int disjointFromMirrorCandidateReopenAfterSaveLoad;
    int disjointFromMirrorCandidateRotationDuringResurrectConfirmation;
    int disjointFromMirrorCandidateResurrectChampionSwitchReopenRuntime;
    int disjointFromMirrorCandidateEyeSlotSwap;
    int disjointFromMirrorCandidateC040InventoryToggleWhilePanelLive;
    int disjointFromMirrorCandidateC040SaveGameWhilePanelLive;
    int disjointFromMirrorCandidateC040StatusBoxClickWhilePanelLive;
    int disjointFromMirrorCandidateC040SpellAreaClickWhilePanelLive;
    int disjointFromMirrorCandidateC040ActionAreaClickWhilePanelLive;
    int disjointFromMirrorCandidateC160CloseWhileRotationPending;
    int disjointFromMirrorCandidateC161CancelAfterF0334Pending;
    int disjointFromMirrorCandidateC061DropResurrectPending;
    int disjointFromMirrorCandidateC040CancelThenReopenSameTick;
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
    int disjointFromChampionPanelClockTickStatRepaint;
    int disjointFromChampionPanelHudFoodWaterRecompute;
    int disjointFromChampionPanelNameBoxClip;
    int disjointFromChampionPanelDeadMemberHandRefresh;
    int disjointFromChampionPanelSpellAreaClearOnInventory;
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
    int disjointFromMirrorCandidateReshufflePanelLive;
    int disjointFromMirrorCandidateOccupiedHandPanel;
    int disjointFromMirrorCandidateInventoryToggle;
    int disjointFromMirrorCandidateDoubleOpenCloseGuard;
    int disjointFromMirrorCandidateChestClosePendingPanel;
    int disjointFromMirrorCandidateChestCloseLeaderHandPickup;
    int disjointFromMirrorCandidateChestOpenDuringPending;
    int disjointFromInventoryChampionSwitchHandCarry;
} Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudDisjointPc34;

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
    int statusBoxWidth;
    int statusBoxHeight;
    int statusBoxStride;
    int portraitWidth;
    int portraitHeight;
    int barWidth;
    int barHeight;
    int handSlotWidth;
    int handSlotHeight;
    int firstChampionStatusBox;
    int statusBoxSpacing;
    int byteWidth;
    uint32_t deterministicSeed;

    /* Per-champion C30+ slot [30..37] + C38 cursor. Each entry is the
     * thing id of the slot's owner; 0xFFFF means the slot is empty. */
    int c30Owner[DM1_V1_MC_AMCIERH_PARTY_COUNT_PC34];

    /* Per-champion leader hand thing. */
    int leaderHand[DM1_V1_MC_AMCIERH_PARTY_COUNT_PC34];

    /* Per-champion HP/Stamina/Mana current (HUD bars C027..C029). */
    int currentHp[DM1_V1_MC_AMCIERH_PARTY_COUNT_PC34];
    int currentStamina[DM1_V1_MC_AMCIERH_PARTY_COUNT_PC34];
    int currentMana[DM1_V1_MC_AMCIERH_PARTY_COUNT_PC34];
    int maximumHp[DM1_V1_MC_AMCIERH_PARTY_COUNT_PC34];
    int maximumStamina[DM1_V1_MC_AMCIERH_PARTY_COUNT_PC34];
    int maximumMana[DM1_V1_MC_AMCIERH_PARTY_COUNT_PC34];

    /* Per-champion food/water values (food/water warning C545). */
    int foodValue[DM1_V1_MC_AMCIERH_PARTY_COUNT_PC34];
    int waterValue[DM1_V1_MC_AMCIERH_PARTY_COUNT_PC34];

    /* Per-champion poisoned state (poisoned overlay). */
    int poisonedState[DM1_V1_MC_AMCIERH_PARTY_COUNT_PC34];

    /* Per-champion mouth-pressed + eye-pressed (M033..M039 warning
     * icons are overlay-driven; the gates G0333_B_PressingMouth and
     * G0331_B_PressingEye track this). */
    int mouthPressed;
    int eyePressed;

    /* Per-champion portrait ordinal (C026 atlas ordinal 0..23). */
    int portraitOrdinal[DM1_V1_MC_AMCIERH_PARTY_COUNT_PC34];

    /* Per-champion portrait cell (where they stand in view cells
     * M018_VIEW_CELL_*, normalized by G0308 direction). */
    int portraitCell[DM1_V1_MC_AMCIERH_PARTY_COUNT_PC34];

    /* Per-champion direction (0..3). */
    int direction[DM1_V1_MC_AMCIERH_PARTY_COUNT_PC34];

    /* Per-champion current health. */
    int currentHealth[DM1_V1_MC_AMCIERH_PARTY_COUNT_PC34];

    /* Per-champion action-index (M070/M070_HAND_SLOT_INDEX base). */
    int actionIndex[DM1_V1_MC_AMCIERH_PARTY_COUNT_PC34];

    /* Per-champion alive flag. */
    int alive[DM1_V1_MC_AMCIERH_PARTY_COUNT_PC34];

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
    DM1_V1_MC_AMCIERH_StepPc34 step;

    /* Per-step operation counts. */
    int f0334CloseCount;
    int f0292ChampionDrawStateCount;
    int f0293DrawAllChampionStatesCount;
    int f0296DrawChangedObjectIconsCount;
    int f0335ClearChampionDataCount;
    int f0354DrawStatusBoxPortraitCount;
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
    uint32_t hudOverlayHash;
    uint32_t candidateChainHash;
    uint32_t chestListHash;
    uint32_t panelHash;
    uint32_t stateHashLive;
    uint32_t stateHashAfterExit;
    uint32_t stateHashAfterReopen;
    uint32_t stateHashAfterPost;
} Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudStatePc34;

typedef struct {
    /* Step transition results. */
    int accepted;
    int reachedExit;
    int reachedReopen;
    int reachedPost;

    /* Live-state preservation: HUD overlay is byte-identical. */
    int hudStatusBoxStableAcrossExit;
    int hudStatusBoxStableAcrossReopen;
    int hudPortraitBoxStableAcrossExit;
    int hudPortraitBoxStableAcrossReopen;
    int hudChampionIconStableAcrossExit;
    int hudChampionIconStableAcrossReopen;
    int hudActionHandStableAcrossExit;
    int hudActionHandStableAcrossReopen;
    int hudActionIconStableAcrossExit;
    int hudActionIconStableAcrossReopen;
    int hudBarsStableAcrossExit;
    int hudBarsStableAcrossReopen;
    int hudFoodWaterStableAcrossExit;
    int hudFoodWaterStableAcrossReopen;
    int hudMouthEyeStableAcrossExit;
    int hudMouthEyeStableAcrossReopen;
    int hudPoisonStableAcrossExit;
    int hudPoisonStableAcrossReopen;
    int hudPortraitOrdinalStableAcrossExit;
    int hudPortraitOrdinalStableAcrossReopen;

    /* Live-state preservation: panel pixels (C040 chain). */
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
    int f0293DrawAllChampionStatesCountAfterExit;
    int f0296DrawChangedObjectIconsCountAfterExit;
    int f0335ClearChampionDataCountAfterExit;
    int f0354DrawStatusBoxPortraitCountAfterExit;
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
    int f0293DrawAllChampionStatesCountAfterReopen;
    int f0296DrawChangedObjectIconsCountAfterReopen;
    int f0354DrawStatusBoxPortraitCountAfterReopen;
    int f0280CandidatePublishCountAfterReopen;
    int f0282CandidateClearCountAfterReopen;

    /* Candidate gate state. */
    int candidateGateFired;
    int candidateGateCounted;
    int candidateGateSourceReachable;
    int candidateGatePanelSuppressed;
    int candidateGateChampionRedrawSuppressed;
    int candidateGateHudOverlayStable;
    int noCandidateClearOnExit;
    int noCandidatePublishOnExit;

    /* HUD-overlay suppression contract. */
    int f0292PerChampionAllZero;
    int f0293NotCalledOnClosePath;
    int f0296NotCalledOnClosePath;
    int f0335NotCalledOnClosePath;
    int f0354NotCalledOnClosePath;
    int perChampionF0292CountZero[DM1_V1_MC_AMCIERH_PARTY_COUNT_PC34];

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

    /* Auto-restore invariant. */
    int g0299PreservedAcrossExit;
    int g0299PreservedAcrossReopen;
    int g0299SourceReachable;
    int f0282NotCalledOnExit;
    int f0282NotCalledOnReopen;
    int f0280NotCalledOnExit;
    int f0280NotCalledOnReopen;

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
    int noF0292PerChampion;
    int noF0293Cascade;
    int noF0296ObjectIconCascade;
    int noF0335ClearChampionData;
    int noF0354BoxVariantsBlit;

    /* Disjoint contract. */
    Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudDisjointPc34 disjoint;

    /* Hashes. */
    uint32_t c030ChainHashLive;
    uint32_t c030ChainHashAfterExit;
    uint32_t c030ChainHashAfterReopen;
    uint32_t hudOverlayHashLive;
    uint32_t hudOverlayHashAfterExit;
    uint32_t hudOverlayHashAfterReopen;
    uint32_t hudOverlayHashAfterPost;
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
} Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudResultPc34;

void dm1_v1_mirror_candidate_auto_mirror_candidate_inventory_exit_restore_hud_init_pc34(
    Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudStatePc34 *state);

int dm1_v1_mirror_candidate_auto_mirror_candidate_inventory_exit_restore_hud_run_pc34(
    Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudStatePc34 *state,
    Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudResultPc34 *result);

const Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudEvidencePc34 *
dm1_v1_mirror_candidate_auto_mirror_candidate_inventory_exit_restore_hud_evidence_pc34(
    void);

const char *
dm1_v1_mirror_candidate_auto_mirror_candidate_inventory_exit_restore_hud_source_evidence_pc34(
    void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_AUTO_MIRROR_CANDIDATE_INVENTORY_EXIT_RESTORE_HUD_PC34_COMPAT_H */
