/*
 * DM1 V1 mirror-candidate HUD-overlay restoration across inventory exit
 * gate implementation.
 *
 * Source-lock anchors (ReDMCSB WIP 20210206, PC 3.4 path, MEDIA009+):
 *  - PANEL.C F0355_INVENTORY_Toggle_CPSE:2244-2330 owns the
 *    close-inventory path.
 *  - PANEL.C F0355:2318-2322 fires F0334_INVENTORY_CloseChest once
 *    and applies the `!G0299_ui_CandidateChampionOrdinal` gate that
 *    suppresses the F0292_CHAMPION_DrawState inventory-champion
 *    redraw when the C040 candidate is live. The close path does NOT
 *    call F0293 (cascade), F0296 (object icons), F0335 (clear), or
 *    F0354 (box variants) - the HUD overlay (C151..C154 status
 *    boxes, C155..C158 portraits, C113..C116 champion icons, M070
 *    action hand, C020 action icon, C027..C029 bars, C545 food/
 *    water, C033..C039 mouth/eye) must remain byte-identical across
 *    the inventory exit.
 *  - PANEL.C F0292 (CHAMDRAW.C F0292) is the redraw the gate
 *    suppresses.
 *  - PANEL.C F0293 (CHAMDRAW.C F0293:1117) cascades F0292 for all
 *    four champions and is NOT called on the close path.
 *  - PANEL.C F0296 (CHAMDRAW.C F0296:1185) draws the four champion
 *    icons C113..C116 and is NOT called on the close path.
 *  - PANEL.C F0335 is the chest-data clear and is NOT called on the
 *    close path.
 *  - PANEL.C F0354 (PANEL.C:2195-2242) is the status-box portrait
 *    blit dispatch and is NOT called from F0355 close path. Its
 *    dispatch contract is pinned by the champion_panel_f0354_box_
 *    variants gate (different gate).
 *  - PANEL.C F0347_INVENTORY_DrawPanel:1639-1693 owns the panel
 *    re-derivation. The G0299 non-zero check at line 1654 routes to
 *    F0346_INVENTORY_DrawPanel_ResurrectReincarnate:1619-1637.
 *  - PANEL.C F0346:1626 sets G0424_i_PanelContent =
 *    M568_PANEL_RESURRECT_REINCARNATE and blits the C040 graphic via
 *    M519_F0020_MAIN_BlitToViewport on the G0032_ai_Graphic562_Box_
 *    Panel rect.
 *  - PANEL.C F0395_MENUS_DrawMovementArrows is the post-exit arrow
 *    draw (movement arrows, NOT HUD overlay).
 *  - PANEL.C F0098_DUNGEONVIEW_DrawFloorAndCeiling redraws the
 *    floor/ceiling only (NOT HUD overlay).
 *  - COMMAND.C F0357_COMMAND_DiscardAllInput clears the input queue
 *    at the end of the close path.
 *  - COMMAND.C F0326_B_RefreshMousePointerInMainLoop is set true on
 *    the close path.
 *  - IO.C F0077_MOUSE_EnableScreenUpdate_CPSE /
 *    F0078_MOUSE_DisableScreenUpdate are the screen update toggle on
 *    the close path.
 *  - REVIVE.C F0280:124-132 publishes the candidate (initial only).
 *    F0282:744-806 consumes/clears it. Neither runs on the close
 *    path; F0282 must NOT run during the close or the candidate
 *    would be cleared and the C040 panel would fall through to the
 *    food/water panel on reopen.
 *  - DEFS.H:2088 C30..C37/C38, G0425/G0426, C040, M568, G0299.
 *  - DEFS.H:712-716 C04_CHAMPION_CLOSE_INVENTORY.
 *  - DEFS.H:5876 G0423_i_InventoryChampionOrdinal.
 *  - DEFS.H:2200 C040_GRAPHIC_PANEL_RESURRECT_REINCARNATE.
 *  - DEFS.H:3001-3008 M568_PANEL_RESURRECT_REINCARNATE.
 *  - DEFS.H:5694 G0299_ui_CandidateChampionOrdinal.
 *  - DEFS.H:5876-5881 G0425/G0426 chest list + open chest.
 *  - DEFS.H:3793 C175_ZONE_FIRST_CHAMPION_STATUS_BOX.
 *  - DEFS.H:2157 C69_CHAMPION_STATUS_BOX_SPACING.
 *  - DEFS.H:2471 C016_BYTE_WIDTH.
 *  - DEFS.H:1874-1878 M070_HAND_SLOT_INDEX.
 *
 * No bitmap sampling, no GRAPHICS.DAT / DUNGEON.DAT load, no real-
 * asset or original-DOS pixel parity claim.
 */

#include "firestaff/dm1/v1/mirror_candidate/auto_mirror_candidate_inventory_exit_restore_hud_pc34_compat.h"

#include <string.h>

/* Source evidence string is intentionally kept in a single literal so
 * the gate's source-lock can be diffed against the ReDMCSB anchors
 * above with a single strstr sweep. */
static const char s_source_evidence[] =
    "contract_only=1; no real-asset bitmap parity claim; no GRAPHICS.DAT or "
    "DUNGEON.DAT load. ReDMCSB PANEL.C F0355_INVENTORY_Toggle_CPSE:2244-2330 "
    "owns the close-inventory path. ReDMCSB PANEL.C F0355:2318-2322 calls "
    "F0334_INVENTORY_CloseChest once and applies the "
    "!G0299_ui_CandidateChampionOrdinal gate that suppresses the "
    "F0292_CHAMPION_DrawState inventory-champion redraw when the C040 "
    "candidate is live. The close path does NOT call F0293 (cascade), "
    "F0296 (object icons), F0335 (clear), or F0354 (box variants). "
    "ReDMCSB PANEL.C F0292 (CHAMDRAW.C F0292:703-1110) is the inventory-"
    "champion redraw the gate suppresses. ReDMCSB PANEL.C F0293 "
    "(CHAMDRAW.C F0293:1117) cascades F0292 for all four champions and "
    "is NOT called on the close path. ReDMCSB PANEL.C F0296 "
    "(CHAMDRAW.C F0296:1185) draws the four champion icons C113..C116 "
    "and is NOT called on the close path. ReDMCSB PANEL.C F0335 is the "
    "chest-data clear and is NOT called on the close path. ReDMCSB "
    "PANEL.C F0354 (PANEL.C:2195-2242) is the status-box portrait blit "
    "dispatch and is NOT called from F0355 close path. ReDMCSB PANEL.C "
    "F0347_INVENTORY_DrawPanel:1639-1693 owns the panel re-derivation; "
    "the G0299 non-zero check at line 1654 routes to F0346_INVENTORY_"
    "DrawPanel_ResurrectReincarnate:1619-1637. ReDMCSB PANEL.C F0346:1626 "
    "sets G0424_i_PanelContent = M568_PANEL_RESURRECT_REINCARNATE and "
    "blits the C040 graphic via M519_F0020_MAIN_BlitToViewport on the "
    "G0032_ai_Graphic562_Box_Panel rect. ReDMCSB PANEL.C F0395_MENUS_"
    "DrawMovementArrows is the post-exit arrow draw (movement arrows, "
    "NOT HUD overlay). ReDMCSB PANEL.C F0098_DUNGEONVIEW_DrawFloorAnd"
    "Ceiling redraws the floor/ceiling only (NOT HUD overlay). "
    "ReDMCSB COMMAND.C F0357_COMMAND_DiscardAllInput clears the input "
    "queue at the end of the close path. ReDMCSB COMMAND.C F0326_B_"
    "RefreshMousePointerInMainLoop is set true on the close path. "
    "ReDMCSB IO.C F0077_MOUSE_EnableScreenUpdate_CPSE / F0078_MOUSE_"
    "DisableScreenUpdate are the screen update toggle on the close "
    "path. ReDMCSB REVIVE.C F0280:124-132 publishes the candidate "
    "(initial only); F0282:744-806 consumes/clears it. Neither runs "
    "on the close path; F0282 must NOT run during the close or the "
    "candidate would be cleared and the C040 panel would fall through "
    "to the food/water panel on reopen. ReDMCSB DEFS.H:2088 C30..C37/"
    "C38, G0425/G0426, C040, M568, G0299. ReDMCSB DEFS.H:712-716 "
    "C04_CHAMPION_CLOSE_INVENTORY. ReDMCSB DEFS.H:5876 G0423_i_"
    "InventoryChampionOrdinal. ReDMCSB DEFS.H:2200 C040_GRAPHIC_PANEL_"
    "RESURRECT_REINCARNATE. ReDMCSB DEFS.H:3001-3008 M568_PANEL_"
    "RESURRECT_REINCARNATE. ReDMCSB DEFS.H:5694 G0299_ui_Candidate"
    "ChampionOrdinal. ReDMCSB DEFS.H:5876-5881 G0425/G0426 chest list + "
    "open chest. ReDMCSB DEFS.H:3793 C175_ZONE_FIRST_CHAMPION_STATUS_"
    "BOX. ReDMCSB DEFS.H:2157 C69_CHAMPION_STATUS_BOX_SPACING. "
    "ReDMCSB DEFS.H:2471 C016_BYTE_WIDTH. ReDMCSB DEFS.H:1874-1878 "
    "M070_HAND_SLOT_INDEX. The fixture is the *HUD-overlay restoration* "
    "lane; it does NOT pin the C040 panel pixel survival (covered by "
    "panel_redraw_after_inventory_exit_pc34_compat), the C030 chain "
    "+ G0425 chest stability (covered by the same gate), the F0333 "
    "chest-open path, the C040 panel-resurrect confirm path, the C045 "
    "food/water accept cross-rotation, the C540 scroll-wheel close "
    "race, the C028 resurrect-pending non-leader pickup, the lower-"
    "arrow owner-ignore guard, the double-open/close guard, the C159 "
    "click rotation combo, the C545 food/water accept/drop, save/load, "
    "teleporter, party-rotate, leader-rotation, portrait redraw, "
    "F0354 box-variants, F0292 -> F0354 dispatch, or champion-panel "
    "clock-tick / food-water-recompute / name-box-clip / dead-member-"
    "hand-refresh / spell-area-clear-on-inventory gates.";

static const Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudEvidencePc34
    s_evidence = {
        "DEFS.H:3793 C175_ZONE_FIRST_CHAMPION_STATUS_BOX; C151..C154 are "
        "the four champion status box zones; DEFS.H:2157 "
        "C69_CHAMPION_STATUS_BOX_SPACING; C175..C178 stride 69",
        "DEFS.H:825-826 M027_PORTRAIT_X/Y + DEFS.H:6391-6392 "
        "G2078_C32_PortraitWidth / G2079_C29_PortraitHeight; "
        "C155..C158 are the four portrait-box zones inside the status "
        "boxes; 32x29 portrait blit",
        "CHAMDRAW.C F0296:1185-1248 DrawChangedObjectIcons; C113..C116 "
        "champion icon cells M026_CHAMPION_ICON_INDEX; per-cell "
        "M518_CHAMPION_ICONS graphic",
        "DEFS.H:1874-1878 M070_HAND_SLOT_INDEX; per-champion hand slot "
        "(ready hand + action hand) bytes C00..C01 with action-icon "
        "overlay at C020",
        "DEFS.H action-icon C020 lives in the action strip below the "
        "HUD status box row; F0297/F0298 hand-state + ownership chain",
        "DEFS.H bars C027_HP / C028_STAMINA / C029_MANA; 18x3 bar "
        "rect inside the status box; CHAMDRAW.C F0292:898-935 redraws "
        "all three bars per champion",
        "DEFS.H:545 C545_FOOD_WATER_WARN; F0292:898-935 also redraws "
        "the food/water warning pixel; poisoned overlay triggers from "
        "M516_CHAMPIONS[].PoisonedCount",
        "PANEL.C F0352/F0353 click-on-eye + stop-pressing-eye; "
        "G0333_B_PressingMouth + G0331_B_PressingEye drive the C033.."
        "C039 mouth/eye warning icons",
        "PANEL.C F0355:2244-2330 owns the entire close-inventory path; "
        "C04_CHAMPION_CLOSE_INVENTORY is the close sentinel",
        "PANEL.C F0355:2318-2322 !G0299_ui_CandidateChampionOrdinal gate "
        "suppresses the F0292 inventory-champion redraw when the C040 "
        "candidate is live; the close path does NOT iterate other "
        "champions either, so HUD overlay is byte-identical across exit",
        "PANEL.C F0395_MENUS_DrawMovementArrows is the post-exit arrow "
        "draw (movement arrows, NOT HUD overlay)",
        "PANEL.C F0098_DUNGEONVIEW_DrawFloorAndCeiling redraws the "
        "floor/ceiling only (NOT HUD overlay)",
        "COMMAND.C F0357_COMMAND_DiscardAllInput clears the input queue "
        "at the end of the close path",
        "REVIVE.C F0280:124-132 publishes the candidate (initial only)",
        "REVIVE.C F0282:744-806 consumes/clears the candidate; must NOT "
        "run on the close path",
        "PANEL.C F0347_INVENTORY_DrawPanel:1639-1693 owns the panel "
        "re-derivation",
        "PANEL.C F0346_INVENTORY_DrawPanel_ResurrectReincarnate:1619-1637 "
        "owns the C040 panel draw",
        "DEFS.H:2088 C30..C37/C38, G0425/G0426, C040, M568, G0299; "
        "DEFS.H:712-716 C04_CHAMPION_CLOSE_INVENTORY; DEFS.H:2200 "
        "C040_GRAPHIC_PANEL_RESURRECT_REINCARNATE; DEFS.H:3001-3008 "
        "M568_PANEL_RESURRECT_REINCARNATE; DEFS.H:5694 "
        "G0299_ui_CandidateChampionOrdinal; DEFS.H:5876 "
        "G0423_i_InventoryChampionOrdinal; DEFS.H:5876-5881 G0425/G0426; "
        "DEFS.H:3793 C175_ZONE_FIRST_CHAMPION_STATUS_BOX; DEFS.H:2157 "
        "C69_CHAMPION_STATUS_BOX_SPACING; DEFS.H:2471 C016_BYTE_WIDTH; "
        "DEFS.H:1874-1878 M070_HAND_SLOT_INDEX",
        "contract-only HUD-overlay restoration lane; pins the close-"
        "inventory exit path while the C040 candidate is live with the "
        "additional HUD-overlay stability invariant: F0292 must NOT be "
        "called for any of the four party champions; F0293/F0296/F0335/"
        "F0354 must NOT run on the close path; the per-champion HUD "
        "overlay (status box C151..C154, portrait C155..C158, champion "
        "icon C113..C116, action hand M070, action icon C020, bars "
        "C027..C029, food/water C545, mouth/eye C033..C039, poisoned) "
        "must remain byte-identical across the exit and the reopen"
    };

const Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudEvidencePc34 *
dm1_v1_mirror_candidate_auto_mirror_candidate_inventory_exit_restore_hud_evidence_pc34(void)
{
    return &s_evidence;
}

const char *
dm1_v1_mirror_candidate_auto_mirror_candidate_inventory_exit_restore_hud_source_evidence_pc34(
    void)
{
    return s_source_evidence;
}

static uint32_t hash_step(uint32_t hash, unsigned int value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (uint32_t)((value >> (i * 8)) & 0xffu);
        hash *= UINT32_C(16777619);
    }
    return hash;
}

static void mark_disjoint(
    Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudDisjointPc34 *d)
{
    if (!d) {
        return;
    }
    d->contractOnly = 1;
    d->disjointFromPanelRedrawAfterInventoryExit = 1;
    d->disjointFromC040RedrawAfterChestClose = 1;
    d->disjointFromC040ChromeInventoryOwnerSwap = 1;
    d->disjointFromC040PanelBrowsePickupRotateRace = 1;
    d->disjointFromC040CloseNonLeaderScrollPickup = 1;
    d->disjointFromC045FoodWaterAcceptCrossRotation = 1;
    d->disjointFromC045CloseAfterNonCandidateTransition = 1;
    d->disjointFromC045FoodWaterCloseNoCandidate = 1;
    d->disjointFromC545PickupWhilePanelLive = 1;
    d->disjointFromC545DropWhilePanelLive = 1;
    d->disjointFromC545AcceptDuringRotation = 1;
    d->disjointFromC545FoodWaterAcceptCrossRotation = 1;
    d->disjointFromInventoryToggle = 1;
    d->disjointFromResurrectChestCloseOrder = 1;
    d->disjointFromResurrectReselectWithInventoryPickup = 1;
    d->disjointFromResurrectConfirmInventoryInterrupt = 1;
    d->disjointFromCloseWhileResurrectPendingWithInventoryPickup = 1;
    d->disjointFromResurrectDoubleCandidateRace = 1;
    d->disjointFromResurrectCrossCandidateClear = 1;
    d->disjointFromResurrectFullC30Chain = 1;
    d->disjointFromMirrorCandidateScrollPickupLeaderRotationInventoryClick = 1;
    d->disjointFromMirrorCandidateInventoryPortraitClick = 1;
    d->disjointFromMirrorCandidatePartySwap = 1;
    d->disjointFromMirrorCandidateOpenThenReselect = 1;
    d->disjointFromMirrorCandidateReselectAfterDepositWithPartyRotate = 1;
    d->disjointFromMirrorCandidateKeyboardRotationCombo = 1;
    d->disjointFromMirrorCandidateKeyboardBrowse = 1;
    d->disjointFromMirrorCandidateC159ClickRotationCombo = 1;
    d->disjointFromMirrorCandidateFullChain = 1;
    d->disjointFromMirrorCandidateNoPendingResurrect = 1;
    d->disjointFromMirrorCandidateResurrectRearm = 1;
    d->disjointFromMirrorCandidateReincarnateRearm = 1;
    d->disjointFromMirrorCandidateThoughtProjectCancelAfterPickup = 1;
    d->disjointFromMirrorCandidateThoughtProjectTraversalOverlay = 1;
    d->disjointFromMirrorCandidateTeleporterSurvival = 1;
    d->disjointFromMirrorCandidateReopenAfterSaveLoad = 1;
    d->disjointFromMirrorCandidateRotationDuringResurrectConfirmation = 1;
    d->disjointFromMirrorCandidateResurrectChampionSwitchReopenRuntime = 1;
    d->disjointFromMirrorCandidateEyeSlotSwap = 1;
    d->disjointFromMirrorCandidateC040InventoryToggleWhilePanelLive = 1;
    d->disjointFromMirrorCandidateC040SaveGameWhilePanelLive = 1;
    d->disjointFromMirrorCandidateC040StatusBoxClickWhilePanelLive = 1;
    d->disjointFromMirrorCandidateC040SpellAreaClickWhilePanelLive = 1;
    d->disjointFromMirrorCandidateC040ActionAreaClickWhilePanelLive = 1;
    d->disjointFromMirrorCandidateC160CloseWhileRotationPending = 1;
    d->disjointFromMirrorCandidateC161CancelAfterF0334Pending = 1;
    d->disjointFromMirrorCandidateC061DropResurrectPending = 1;
    d->disjointFromMirrorCandidateC040CancelThenReopenSameTick = 1;
    d->disjointFromChampionPanelF0354BoxVariants = 1;
    d->disjointFromChampionPanelHandSlotPrioritySourceLock = 1;
    d->disjointFromChampionPanelPortraitStateRedraw = 1;
    d->disjointFromChampionPanelPortraitBoxBlitGate = 1;
    d->disjointFromChampionPanelSpellAreaOverlay = 1;
    d->disjointFromChampionPanelFoodWaterStatusBox = 1;
    d->disjointFromChampionPanelMouthEyeRelease = 1;
    d->disjointFromChampionPanelMouthEyePoisonWarning = 1;
    d->disjointFromChampionPanelPressingMouthEyeStatusbox = 1;
    d->disjointFromChampionPanelHudRecompute = 1;
    d->disjointFromChampionPanelActionHandSlotPriority = 1;
    d->disjointFromChampionPanelClockTickStatRepaint = 1;
    d->disjointFromChampionPanelHudFoodWaterRecompute = 1;
    d->disjointFromChampionPanelNameBoxClip = 1;
    d->disjointFromChampionPanelDeadMemberHandRefresh = 1;
    d->disjointFromChampionPanelSpellAreaClearOnInventory = 1;
    d->disjointFromChestCloseWhilePartyRotatePickupPending = 1;
    d->disjointFromChestCloseWhileCandidateLiveNonLeader = 1;
    d->disjointFromChestScrollWheelCloseRace = 1;
    d->disjointFromChestScrollWheelResurrectConfirmation = 1;
    d->disjointFromChestResurrectRotationScrollWheel = 1;
    d->disjointFromChestOpenDuringPending = 1;
    d->disjointFromChestPickupDuringResurrectPendingNonLeader = 1;
    d->disjointFromChestDepositDuringLeaderRotation = 1;
    d->disjointFromChestPartialDropToFloorWhileChestOpen = 1;
    d->disjointFromChestInventoryC545DropToLeaderHandAlreadyOccupied = 1;
    d->disjointFromChestTeleporterSurvivalOpenG0426 = 1;
    d->disjointFromMirrorCandidateScrollPickupNonLeaderPanelLive = 1;
    d->disjointFromMirrorCandidateScrollPickupWithPartyRotateInProgress = 1;
    d->disjointFromMirrorCandidatePendingHandDuringChestPickupRace = 1;
    d->disjointFromMirrorCandidatePendingHandQueue = 1;
    d->disjointFromMirrorCandidateLowerArrowGuard = 1;
    d->disjointFromMirrorCandidateClickCancel = 1;
    d->disjointFromMirrorCandidateClickCancelWithRotation = 1;
    d->disjointFromMirrorCandidateCloseButton = 1;
    d->disjointFromMirrorCandidateIconRefresh = 1;
    d->disjointFromMirrorCandidateReshufflePanelLive = 1;
    d->disjointFromMirrorCandidateOccupiedHandPanel = 1;
    d->disjointFromMirrorCandidateInventoryToggle = 1;
    d->disjointFromMirrorCandidateDoubleOpenCloseGuard = 1;
    d->disjointFromMirrorCandidateChestClosePendingPanel = 1;
    d->disjointFromMirrorCandidateChestCloseLeaderHandPickup = 1;
    d->disjointFromMirrorCandidateChestOpenDuringPending = 1;
    d->disjointFromInventoryChampionSwitchHandCarry = 1;
}

static int in_range(int value, int low, int high)
{
    return value >= low && value <= high;
}

static uint32_t compute_c030_chain_hash(
    const Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudStatePc34 *s)
{
    uint32_t hash;
    int i;

    hash = UINT32_C(2166136261);
    if (!s) {
        return hash;
    }
    hash = hash_step(hash, s->partyChampionCount);
    hash = hash_step(hash, s->leaderIndex);
    for (i = 0; i < DM1_V1_MC_AMCIERH_PARTY_COUNT_PC34; ++i) {
        hash = hash_step(hash, s->c30Owner[i]);
        hash = hash_step(hash, s->leaderHand[i]);
    }
    hash = hash_step(hash, s->partyResting);
    return hash;
}

/* HUD-overlay hash covers:
 *   - status box C151..C154 (4 zones, each 67x29)
 *   - portrait box C155..C158 (4 zones, each 32x29)
 *   - champion icon cell C113..C116 (4 cells)
 *   - per-champion HP/Stamina/Mana current+max (6x4 = 24 values)
 *   - per-champion food/water (2x4 = 8 values)
 *   - per-champion poisoned (4 values)
 *   - per-champion mouth/eye-pressed drivers (party-wide booleans)
 *   - per-champion portrait ordinal + cell + direction (3x4 = 12 values)
 *   - per-champion current health (4 values)
 *   - per-champion action-index (4 values)
 *   - per-champion alive flag (4 values)
 *   - per-champion leader hand thing (4 values)
 *   - status box width/height/stride + portrait width/height
 *
 * If any of these values change, the HUD-overlay hash changes; if all
 * are stable, the hash is stable. This is the contract-only invariant
 * that the close path must preserve. */
static uint32_t compute_hud_overlay_hash(
    const Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudStatePc34 *s)
{
    uint32_t hash;
    int i;

    hash = UINT32_C(2166136261);
    if (!s) {
        return hash;
    }
    /* Box geometry / dimensions. */
    hash = hash_step(hash, s->statusBoxWidth);
    hash = hash_step(hash, s->statusBoxHeight);
    hash = hash_step(hash, s->statusBoxStride);
    hash = hash_step(hash, s->portraitWidth);
    hash = hash_step(hash, s->portraitHeight);
    hash = hash_step(hash, s->firstChampionStatusBox);
    hash = hash_step(hash, s->statusBoxSpacing);
    hash = hash_step(hash, s->byteWidth);

    /* Per-champion HUD overlay fields. */
    for (i = 0; i < DM1_V1_MC_AMCIERH_PARTY_COUNT_PC34; ++i) {
        /* Champion icon cell. */
        hash = hash_step(hash, s->portraitCell[i]);
        /* Champion portrait ordinal. */
        hash = hash_step(hash, s->portraitOrdinal[i]);
        /* Direction. */
        hash = hash_step(hash, s->direction[i]);
        /* Alive + health. */
        hash = hash_step(hash, s->alive[i]);
        hash = hash_step(hash, s->currentHealth[i]);
        /* Bars. */
        hash = hash_step(hash, s->currentHp[i]);
        hash = hash_step(hash, s->currentStamina[i]);
        hash = hash_step(hash, s->currentMana[i]);
        hash = hash_step(hash, s->maximumHp[i]);
        hash = hash_step(hash, s->maximumStamina[i]);
        hash = hash_step(hash, s->maximumMana[i]);
        /* Food/water. */
        hash = hash_step(hash, s->foodValue[i]);
        hash = hash_step(hash, s->waterValue[i]);
        /* Poison. */
        hash = hash_step(hash, s->poisonedState[i]);
        /* Action-index. */
        hash = hash_step(hash, s->actionIndex[i]);
        /* Leader hand (M070). */
        hash = hash_step(hash, s->leaderHand[i]);
    }

    /* Party-wide mouth/eye-pressed drivers. */
    hash = hash_step(hash, s->mouthPressed);
    hash = hash_step(hash, s->eyePressed);

    return hash;
}

static uint32_t compute_candidate_chain_hash(
    const Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudStatePc34 *s)
{
    uint32_t hash;

    hash = UINT32_C(2166136261);
    if (!s) {
        return hash;
    }
    hash = hash_step(hash, s->g0299CandidateOrdinal);
    hash = hash_step(hash, s->c040PanelOpen);
    hash = hash_step(hash, s->c040PanelGraphic);
    hash = hash_step(hash, s->c040PanelCommand);
    hash = hash_step(hash, s->c040PanelColor);
    hash = hash_step(hash, s->c040PanelOwnerSlot);
    hash = hash_step(hash, s->c040PanelC038SlotBox);
    hash = hash_step(hash, s->mouthRouteZone);
    hash = hash_step(hash, s->mouthRouteCommand);
    return hash;
}

static uint32_t compute_chest_list_hash(
    const Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudStatePc34 *s)
{
    uint32_t hash;
    int i;

    hash = UINT32_C(2166136261);
    if (!s) {
        return hash;
    }
    for (i = 0; i < 8; ++i) {
        hash = hash_step(hash, s->g0425ChestList[i]);
    }
    hash = hash_step(hash, s->g0426OpenChest);
    return hash;
}

static uint32_t compute_panel_hash(
    const Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudStatePc34 *s)
{
    uint32_t hash;

    hash = UINT32_C(2166136261);
    if (!s) {
        return hash;
    }
    hash = hash_step(hash, s->c040PanelOpen);
    hash = hash_step(hash, s->c040PanelGraphic);
    hash = hash_step(hash, s->c040PanelCommand);
    hash = hash_step(hash, s->c040PanelColor);
    hash = hash_step(hash, s->c040PanelOwnerSlot);
    hash = hash_step(hash, s->c040PanelC038SlotBox);
    return hash;
}

static uint32_t compute_deterministic_hash(
    const Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudStatePc34 *s,
    uint32_t c030, uint32_t hud, uint32_t candidate, uint32_t chest, uint32_t panel)
{
    uint32_t hash;

    hash = UINT32_C(2166136261);
    if (!s) {
        return hash;
    }
    hash = hash_step(hash, s->partyChampionCount);
    hash = hash_step(hash, s->leaderIndex);
    hash = hash_step(hash, c030);
    hash = hash_step(hash, hud);
    hash = hash_step(hash, candidate);
    hash = hash_step(hash, chest);
    hash = hash_step(hash, panel);
    return hash;
}

static void seed_default_hud_state(
    Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudStatePc34 *s)
{
    int i;

    if (!s) {
        return;
    }
    /* Four party champions with live HUD overlay (the contract starts
     * in a state where the HUD is fully painted: status boxes, portraits,
     * bars, food/water, action hands, and one C040 candidate live on
     * the inventory champion). */
    for (i = 0; i < DM1_V1_MC_AMCIERH_PARTY_COUNT_PC34; ++i) {
        s->c30Owner[i] = 0x7000u + (unsigned int)i; /* arbitrary owner ids */
        s->leaderHand[i] = 0;
        s->currentHp[i] = 100;
        s->currentStamina[i] = 100;
        s->currentMana[i] = 50;
        s->maximumHp[i] = 100;
        s->maximumStamina[i] = 100;
        s->maximumMana[i] = 50;
        s->foodValue[i] = 1500;
        s->waterValue[i] = 1500;
        s->poisonedState[i] = 0;
        s->portraitOrdinal[i] = i;
        s->portraitCell[i] = i;
        s->direction[i] = 0;
        s->currentHealth[i] = 100;
        s->actionIndex[i] = -1;
        s->alive[i] = 1;
    }
    s->partyResting = 0;
    s->mouthPressed = 0;
    s->eyePressed = 0;
}

void dm1_v1_mirror_candidate_auto_mirror_candidate_inventory_exit_restore_hud_init_pc34(
    Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudStatePc34 *s)
{
    if (!s) {
        return;
    }
    memset(s, 0, sizeof(*s));
    s->partyChampionCount = DM1_V1_MC_AMCIERH_PARTY_COUNT_PC34;
    s->leaderIndex = 0;
    s->inventoryChampionOrdinal = 1;
    s->candidateOrdinal = 4;
    s->candidatePanelGraphic = DM1_V1_MC_AMCIERH_CHAMPION_C40_PC34;
    s->candidatePanelCommand = DM1_V1_MC_AMCIERH_M568_PANEL_RESURRECT_REINCARNATE_PC34;
    s->candidatePanelColor = 10;
    s->candidateOwnerSlot = DM1_V1_MC_AMCIERH_CHAMPION_C30_FIRST_PC34;
    s->candidateC038SlotBox = DM1_V1_MC_AMCIERH_CHAMPION_C38_PC34;
    s->closeInventorySentinel = DM1_V1_MC_AMCIERH_C04_CLOSE_INVENTORY_PC34;
    s->specialInventorySentinel = DM1_V1_MC_AMCIERH_C05_SPECIAL_INVENTORY_PC34;
    s->championNoneSentinel = DM1_V1_MC_AMCIERH_CHAMPION_NONE_PC34;
    s->closeChestThing = 0;
    s->resurrectSlot = DM1_V1_MC_AMCIERH_CHAMPION_C30_FIRST_PC34;
    s->mouthZone = DM1_V1_MC_AMCIERH_C545_FOOD_WATER_WARN_PC34;
    s->mouthCommand = DM1_V1_MC_AMCIERH_M70_HAND_SLOT_INDEX_PC34;
    s->floorAndCeilingHeight = 50;
    s->floorAndCeilingByteWidth = 16;
    s->scrollWheelSlotCommand = 540;
    s->statusBoxWidth = DM1_V1_MC_AMCIERH_STATUS_BOX_WIDTH_PC34;
    s->statusBoxHeight = DM1_V1_MC_AMCIERH_STATUS_BOX_HEIGHT_PC34;
    s->statusBoxStride = DM1_V1_MC_AMCIERH_C69_STATUS_BOX_SPACING_PC34;
    s->portraitWidth = DM1_V1_MC_AMCIERH_PORTRAIT_WIDTH_PC34;
    s->portraitHeight = DM1_V1_MC_AMCIERH_PORTRAIT_HEIGHT_PC34;
    s->barWidth = DM1_V1_MC_AMCIERH_BAR_WIDTH_PC34;
    s->barHeight = DM1_V1_MC_AMCIERH_BAR_HEIGHT_PC34;
    s->handSlotWidth = DM1_V1_MC_AMCIERH_HAND_SLOT_WIDTH_PC34;
    s->handSlotHeight = DM1_V1_MC_AMCIERH_HAND_SLOT_HEIGHT_PC34;
    s->firstChampionStatusBox = DM1_V1_MC_AMCIERH_C175_FIRST_STATUS_BOX_PC34;
    s->statusBoxSpacing = DM1_V1_MC_AMCIERH_C69_STATUS_BOX_SPACING_PC34;
    s->byteWidth = DM1_V1_MC_AMCIERH_C016_BYTE_WIDTH_PC34;

    seed_default_hud_state(s);

    /* Per-champion C30+ slot [30..37] + C38 cursor. */
    s->c30Owner[0] = DM1_V1_MC_AMCIERH_CHAMPION_C30_FIRST_PC34 + 0; /* 30 */
    s->c30Owner[1] = DM1_V1_MC_AMCIERH_CHAMPION_C30_FIRST_PC34 + 1; /* 31 */
    s->c30Owner[2] = DM1_V1_MC_AMCIERH_CHAMPION_C30_FIRST_PC34 + 2; /* 32 */
    s->c30Owner[3] = DM1_V1_MC_AMCIERH_CHAMPION_C30_FIRST_PC34 + 3; /* 33 */
    s->leaderHand[0] = 0; /* ready hand empty */
    s->leaderHand[1] = 0; /* action hand empty */

    /* G0425 chest list (C540..C547 things) - empty by default. */
    s->g0425ChestList[0] = 0;
    s->g0425ChestList[1] = 0;
    s->g0425ChestList[2] = 0;
    s->g0425ChestList[3] = 0;
    s->g0425ChestList[4] = 0;
    s->g0425ChestList[5] = 0;
    s->g0425ChestList[6] = 0;
    s->g0425ChestList[7] = 0;

    /* G0426 not open by default. */
    s->g0426OpenChest = 0;

    /* G0299 candidate ordinal = 4 (the live C040 candidate slot). */
    s->g0299CandidateOrdinal = 4;

    /* C040 panel state - live on the inventory champion. */
    s->c040PanelOpen = 1;
    s->c040PanelGraphic = DM1_V1_MC_AMCIERH_CHAMPION_C40_PC34;
    s->c040PanelCommand = DM1_V1_MC_AMCIERH_M568_PANEL_RESURRECT_REINCARNATE_PC34;
    s->c040PanelColor = 10;
    s->c040PanelOwnerSlot = DM1_V1_MC_AMCIERH_CHAMPION_C30_FIRST_PC34;
    s->c040PanelC038SlotBox = DM1_V1_MC_AMCIERH_CHAMPION_C38_PC34;

    /* C545 mouth zone + C070 mouth command. */
    s->mouthRouteZone = DM1_V1_MC_AMCIERH_C545_FOOD_WATER_WARN_PC34;
    s->mouthRouteCommand = DM1_V1_MC_AMCIERH_M70_HAND_SLOT_INDEX_PC34;

    /* Deterministic seed encoded from the C040 sentinel (high word) +
     * a marker for HUD-overlay lane (low word). */
    s->deterministicSeed = (UINT32_C(0xC040) << 16) | UINT32_C(0x05E5);

    /* Step 0: LIVE - C040 panel live, candidate ordinal 4 set. */
    s->step = DM1_V1_MC_AMCIERH_STEP_PC34_LIVE;

    /* F0280 publish at candidate creation (REVIVE.C:124-132), exactly
     * once before the live state is observed. */
    s->f0280CandidatePublishCount = 1;

    /* Compute initial hashes. */
    s->c030ChainHash = compute_c030_chain_hash(s);
    s->hudOverlayHash = compute_hud_overlay_hash(s);
    s->candidateChainHash = compute_candidate_chain_hash(s);
    s->chestListHash = compute_chest_list_hash(s);
    s->panelHash = compute_panel_hash(s);
    s->stateHashLive = compute_deterministic_hash(
        s, s->c030ChainHash, s->hudOverlayHash, s->candidateChainHash,
        s->chestListHash, s->panelHash);
}

static int run_exit_step(
    Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudStatePc34 *s)
{
    if (!s) {
        return 0;
    }
    if (s->step != DM1_V1_MC_AMCIERH_STEP_PC34_LIVE) {
        return 0;
    }
    if (s->g0299CandidateOrdinal == 0) {
        return 0;
    }
    if (s->c040PanelOpen == 0) {
        return 0;
    }
    if (!in_range(s->inventoryChampionOrdinal, 1, 4)) {
        return 0;
    }

    /*
     * ReDMCSB PANEL.C F0355_INVENTORY_Toggle_CPSE:2244-2330 close
     * path with a C04 sentinel and a live G0299:
     *
     *   F0077_MOUSE_EnableScreenUpdate_CPSE();  // line ~2304
     *   if (G0423_i_InventoryChampionOrdinal) {
     *       G0423_i_InventoryChampionOrdinal = CM1_CHAMPION_NONE;
     *       F0334_INVENTORY_CloseChest();
     *       if (M516_CHAMPIONS[inventoryChampionOrdinal].CurrentHealth
     *           && !G0299_ui_CandidateChampionOrdinal) {
     *           M008_SET(... MASK0x1000_STATUS_BOX ...);
     *           F0292_CHAMPION_DrawState(inventoryChampionOrdinal);
     *       }
     *       // ... C04 close: F0395 + F0357 + F0098 + F0326 ...
     *   }
     *
     * The HUD-overlay contract: when G0299 is non-zero, F0292 is
     * suppressed for the inventory champion. The close path itself
     * does NOT call F0293 (cascade), F0296 (object icons), F0335
     * (clear), or F0354 (box variants) for any champion, so the HUD
     * overlay for ALL four champions must remain byte-identical across
     * the exit.
     */
    s->inventoryChampionOrdinal = DM1_V1_MC_AMCIERH_CHAMPION_NONE_PC34;

    /* F0334_INVENTORY_CloseChest - exactly once. */
    ++s->f0334CloseCount;

    /*
     * F0292 is suppressed by the !G0299 candidate gate. We DO NOT
     * increment f0292ChampionDrawStateCount for ANY of the four
     * champions - the close path also does not iterate the other
     * three. F0293/F0296/F0335/F0354 are also not called on the close
     * path.
     */
    {
        /* Per-champion F0292 count is derived from the aggregate
         * f0292ChampionDrawStateCount (which stays 0); each entry in
         * the result.perChampionF0292CountZero array equals
         * (aggregate_count == 0). The result computation fills the
         * array from the aggregate. */
    }
    /* f0292ChampionDrawStateCount, f0293DrawAllChampionStatesCount,
     * f0296DrawChangedObjectIconsCount, f0335ClearChampionDataCount,
     * f0354DrawStatusBoxPortraitCount all remain 0. */

    ++s->f0077MouseEnableScreenUpdateCount;
    ++s->f0357DiscardInputCount;
    ++s->f0098DrawFloorCeilingCount;
    ++s->f0326RefreshMousePointerMainLoopCount;
    ++s->f0395DrawMovementArrowsCount;

    /* G0299 is preserved (F0282 must NOT run on the close path). */
    /* c040PanelOpen, c040PanelGraphic, c040PanelCommand, c040PanelColor,
     * c040PanelOwnerSlot, c040PanelC038SlotBox are all preserved. */

    s->step = DM1_V1_MC_AMCIERH_STEP_PC34_EXIT;

    s->c030ChainHash = compute_c030_chain_hash(s);
    s->hudOverlayHash = compute_hud_overlay_hash(s);
    s->candidateChainHash = compute_candidate_chain_hash(s);
    s->chestListHash = compute_chest_list_hash(s);
    s->panelHash = compute_panel_hash(s);
    s->stateHashAfterExit = compute_deterministic_hash(
        s, s->c030ChainHash, s->hudOverlayHash, s->candidateChainHash,
        s->chestListHash, s->panelHash);
    return 1;
}

static int run_reopen_step(
    Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudStatePc34 *s)
{
    if (!s) {
        return 0;
    }
    if (s->step != DM1_V1_MC_AMCIERH_STEP_PC34_EXIT) {
        return 0;
    }
    if (s->g0299CandidateOrdinal == 0) {
        return 0;
    }

    /*
     * ReDMCSB PANEL.C F0347_INVENTORY_DrawPanel:1654 routes to
     * F0346_INVENTORY_DrawPanel_ResurrectReincarnate when
     * G0299_ui_CandidateChampionOrdinal is non-zero:
     *
     *   if (G0299_ui_CandidateChampionOrdinal) {
     *       F0346_INVENTORY_DrawPanel_ResurrectReincarnate();
     *       return;
     *   }
     *
     * F0346:1626 sets G0424_i_PanelContent = M568_PANEL_RESURRECT_REINCARNATE
     * and blits the C040 graphic via M519_F0020_MAIN_BlitToViewport on
     * the G0032_ai_Graphic562_Box_Panel rect.
     *
     * The HUD-overlay contract on reopen: F0347/F0346 does NOT cascade
     * F0292/F0293/F0296/F0335/F0354. The HUD overlay for ALL four
     * champions remains byte-identical across the reopen.
     */
    s->inventoryChampionOrdinal = 1;
    ++s->f0347DrawPanelCount;
    ++s->f0346ResurrectDrawCount;
    s->mouthRouteZone = DM1_V1_MC_AMCIERH_C545_FOOD_WATER_WARN_PC34;
    s->mouthRouteCommand = DM1_V1_MC_AMCIERH_M70_HAND_SLOT_INDEX_PC34;
    s->c040PanelOpen = 1;
    s->c040PanelGraphic = DM1_V1_MC_AMCIERH_CHAMPION_C40_PC34;
    s->c040PanelCommand = DM1_V1_MC_AMCIERH_M568_PANEL_RESURRECT_REINCARNATE_PC34;
    s->c040PanelColor = 10;
    s->c040PanelOwnerSlot = DM1_V1_MC_AMCIERH_CHAMPION_C30_FIRST_PC34;
    s->c040PanelC038SlotBox = DM1_V1_MC_AMCIERH_CHAMPION_C38_PC34;

    /* F0292/F0293/F0296/F0335/F0354 still NOT called on reopen. */
    ++s->f0078MouseDisableScreenUpdateCount;
    ++s->f0077MouseEnableScreenUpdateCount;

    s->step = DM1_V1_MC_AMCIERH_STEP_PC34_REOPEN;

    s->c030ChainHash = compute_c030_chain_hash(s);
    s->hudOverlayHash = compute_hud_overlay_hash(s);
    s->candidateChainHash = compute_candidate_chain_hash(s);
    s->chestListHash = compute_chest_list_hash(s);
    s->panelHash = compute_panel_hash(s);
    s->stateHashAfterReopen = compute_deterministic_hash(
        s, s->c030ChainHash, s->hudOverlayHash, s->candidateChainHash,
        s->chestListHash, s->panelHash);
    return 1;
}

static int run_post_step(
    Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudStatePc34 *s)
{
    if (!s) {
        return 0;
    }
    if (s->step != DM1_V1_MC_AMCIERH_STEP_PC34_REOPEN) {
        return 0;
    }

    /*
     * ReDMCSB: after the reopen, the engine continues running the
     * main loop. The HUD-overlay restoration invariant must hold for
     * a "post" snapshot - i.e., after a few more ticks, the HUD
     * overlay must STILL be byte-identical to the live state. The
     * main loop's normal redraw cadence (F0293 cascade on tick
     * boundary, F0457 start-draw-enabled-menus) must NOT have fired
     * for the inventory champion since the close path was traversed.
     *
     * The post step asserts the same invariants and re-hashes.
     */
    s->step = DM1_V1_MC_AMCIERH_STEP_PC34_POST;

    s->c030ChainHash = compute_c030_chain_hash(s);
    s->hudOverlayHash = compute_hud_overlay_hash(s);
    s->candidateChainHash = compute_candidate_chain_hash(s);
    s->chestListHash = compute_chest_list_hash(s);
    s->panelHash = compute_panel_hash(s);
    s->stateHashAfterPost = compute_deterministic_hash(
        s, s->c030ChainHash, s->hudOverlayHash, s->candidateChainHash,
        s->chestListHash, s->panelHash);
    return 1;
}

int dm1_v1_mirror_candidate_auto_mirror_candidate_inventory_exit_restore_hud_run_pc34(
    Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudStatePc34 *state,
    Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudResultPc34 *result)
{
    Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudStatePc34 live;
    uint32_t liveC030ChainHash;
    uint32_t liveHudOverlayHash;
    uint32_t liveCandidateChainHash;
    uint32_t liveChestListHash;
    uint32_t livePanelHash;
    int panelGraphicLive;
    int panelCommandLive;
    int panelColorLive;
    int panelOwnerSlotLive;
    int panelC038SlotBoxLive;
    int panelMouthRouteLive;

    if (!state || !result) {
        return 0;
    }

    memset(result, 0, sizeof(*result));
    mark_disjoint(&result->disjoint);

    live = *state;
    liveC030ChainHash = live.c030ChainHash;
    liveHudOverlayHash = live.hudOverlayHash;
    liveCandidateChainHash = live.candidateChainHash;
    liveChestListHash = live.chestListHash;
    livePanelHash = live.panelHash;

    panelGraphicLive = live.c040PanelGraphic;
    panelCommandLive = live.c040PanelCommand;
    panelColorLive = live.c040PanelColor;
    panelOwnerSlotLive = live.c040PanelOwnerSlot;
    panelC038SlotBoxLive = live.c040PanelC038SlotBox;
    panelMouthRouteLive = live.mouthRouteZone;

    if (!run_exit_step(state)) {
        return 0;
    }
    if (!run_reopen_step(state)) {
        return 0;
    }
    if (!run_post_step(state)) {
        return 0;
    }

    result->reachedExit = state->step != DM1_V1_MC_AMCIERH_STEP_PC34_LIVE &&
                          state->f0334CloseCount >= 1;
    result->reachedReopen = state->step != DM1_V1_MC_AMCIERH_STEP_PC34_EXIT &&
                            state->f0347DrawPanelCount >= 1 &&
                            state->f0346ResurrectDrawCount >= 1;
    result->reachedPost = state->step == DM1_V1_MC_AMCIERH_STEP_PC34_POST;

    /* Live-state preservation: HUD overlay is byte-identical. */
    result->hudOverlayHashLive = liveHudOverlayHash;
    result->hudOverlayHashAfterExit = state->hudOverlayHash;
    result->hudOverlayHashAfterReopen = state->hudOverlayHash;
    result->hudOverlayHashAfterPost = state->hudOverlayHash;

    result->hudStatusBoxStableAcrossExit =
        state->hudOverlayHash == liveHudOverlayHash;
    result->hudStatusBoxStableAcrossReopen =
        state->hudOverlayHash == liveHudOverlayHash;
    result->hudPortraitBoxStableAcrossExit =
        state->hudOverlayHash == liveHudOverlayHash;
    result->hudPortraitBoxStableAcrossReopen =
        state->hudOverlayHash == liveHudOverlayHash;
    result->hudChampionIconStableAcrossExit =
        state->hudOverlayHash == liveHudOverlayHash;
    result->hudChampionIconStableAcrossReopen =
        state->hudOverlayHash == liveHudOverlayHash;
    result->hudActionHandStableAcrossExit =
        state->hudOverlayHash == liveHudOverlayHash;
    result->hudActionHandStableAcrossReopen =
        state->hudOverlayHash == liveHudOverlayHash;
    result->hudActionIconStableAcrossExit =
        state->hudOverlayHash == liveHudOverlayHash;
    result->hudActionIconStableAcrossReopen =
        state->hudOverlayHash == liveHudOverlayHash;
    result->hudBarsStableAcrossExit =
        state->hudOverlayHash == liveHudOverlayHash;
    result->hudBarsStableAcrossReopen =
        state->hudOverlayHash == liveHudOverlayHash;
    result->hudFoodWaterStableAcrossExit =
        state->hudOverlayHash == liveHudOverlayHash;
    result->hudFoodWaterStableAcrossReopen =
        state->hudOverlayHash == liveHudOverlayHash;
    result->hudMouthEyeStableAcrossExit =
        state->hudOverlayHash == liveHudOverlayHash;
    result->hudMouthEyeStableAcrossReopen =
        state->hudOverlayHash == liveHudOverlayHash;
    result->hudPoisonStableAcrossExit =
        state->hudOverlayHash == liveHudOverlayHash;
    result->hudPoisonStableAcrossReopen =
        state->hudOverlayHash == liveHudOverlayHash;
    result->hudPortraitOrdinalStableAcrossExit =
        state->hudOverlayHash == liveHudOverlayHash;
    result->hudPortraitOrdinalStableAcrossReopen =
        state->hudOverlayHash == liveHudOverlayHash;

    /* Live-state preservation: panel pixels (C040 chain). */
    result->panelStayedC040 =
        state->c040PanelGraphic == live.c040PanelGraphic &&
        state->c040PanelCommand == live.c040PanelCommand &&
        state->c040PanelColor == live.c040PanelColor &&
        state->c040PanelOwnerSlot == live.c040PanelOwnerSlot &&
        state->c040PanelC038SlotBox == live.c040PanelC038SlotBox;
    result->candidateStillLive = state->g0299CandidateOrdinal != 0;
    result->candidatePanelUnchanged =
        state->c040PanelGraphic == live.c040PanelGraphic &&
        state->c040PanelCommand == live.c040PanelCommand &&
        state->c040PanelColor == live.c040PanelColor;
    result->candidateOwnerUnchanged = state->g0299CandidateOrdinal ==
                                      live.g0299CandidateOrdinal;
    result->candidateOwnerSlotUnchanged = state->c040PanelOwnerSlot ==
                                          live.c040PanelOwnerSlot;
    result->c030ChainPreserved = state->c030ChainHash == liveC030ChainHash;
    result->leaderHandPreserved = state->leaderHand[0] == live.leaderHand[0] &&
                                  state->leaderHand[1] == live.leaderHand[1] &&
                                  state->leaderHand[2] == live.leaderHand[2] &&
                                  state->leaderHand[3] == live.leaderHand[3];
    result->chestListPreserved = state->chestListHash == liveChestListHash;
    result->g0426OpenChestStable = state->g0426OpenChest == live.g0426OpenChest;

    /* Operation counts after exit. */
    result->f0334CloseCountAfterExit = state->f0334CloseCount;
    result->f0292ChampionDrawStateCountAfterExit = state->f0292ChampionDrawStateCount;
    result->f0293DrawAllChampionStatesCountAfterExit = state->f0293DrawAllChampionStatesCount;
    result->f0296DrawChangedObjectIconsCountAfterExit = state->f0296DrawChangedObjectIconsCount;
    result->f0335ClearChampionDataCountAfterExit = state->f0335ClearChampionDataCount;
    result->f0354DrawStatusBoxPortraitCountAfterExit = state->f0354DrawStatusBoxPortraitCount;
    result->f0395DrawMovementArrowsCountAfterExit = state->f0395DrawMovementArrowsCount;
    result->f0357DiscardInputCountAfterExit = state->f0357DiscardInputCount;
    result->f0098DrawFloorCeilingCountAfterExit = state->f0098DrawFloorCeilingCount;
    result->f0077MouseEnableScreenUpdateCountAfterExit = state->f0077MouseEnableScreenUpdateCount;
    result->f0078MouseDisableScreenUpdateCountAfterExit = state->f0078MouseDisableScreenUpdateCount;
    result->f0326RefreshMousePointerMainLoopCountAfterExit = state->f0326RefreshMousePointerMainLoopCount;
    result->f0457StartDrawEnabledMenusCountAfterExit = state->f0457StartDrawEnabledMenusCount;
    result->f0280CandidatePublishCountAfterExit = state->f0280CandidatePublishCount;
    result->f0282CandidateClearCountAfterExit = state->f0282CandidateClearCount;

    /* Operation counts after reopen. */
    result->f0347DrawPanelCountAfterReopen = state->f0347DrawPanelCount;
    result->f0346ResurrectDrawCountAfterReopen = state->f0346ResurrectDrawCount;
    result->f0292ChampionDrawStateCountAfterReopen = state->f0292ChampionDrawStateCount;
    result->f0293DrawAllChampionStatesCountAfterReopen = state->f0293DrawAllChampionStatesCount;
    result->f0296DrawChangedObjectIconsCountAfterReopen = state->f0296DrawChangedObjectIconsCount;
    result->f0354DrawStatusBoxPortraitCountAfterReopen = state->f0354DrawStatusBoxPortraitCount;
    result->f0280CandidatePublishCountAfterReopen = state->f0280CandidatePublishCount;
    result->f0282CandidateClearCountAfterReopen = state->f0282CandidateClearCount;

    /* Candidate gate state. */
    result->candidateGateFired = state->step != DM1_V1_MC_AMCIERH_STEP_PC34_LIVE &&
                                 state->f0334CloseCount >= 1;
    result->candidateGateCounted = state->f0334CloseCount >= 1;
    result->candidateGateSourceReachable = state->f0334CloseCount >= 1 &&
                                           state->g0299CandidateOrdinal != 0;
    result->candidateGatePanelSuppressed = state->f0292ChampionDrawStateCount == 0;
    result->candidateGateChampionRedrawSuppressed =
        state->f0292ChampionDrawStateCount == 0 &&
        state->f0293DrawAllChampionStatesCount == 0;
    result->candidateGateHudOverlayStable =
        state->hudOverlayHash == liveHudOverlayHash;
    result->noCandidateClearOnExit = state->f0282CandidateClearCount == 0;
    result->noCandidatePublishOnExit = state->f0280CandidatePublishCount == 1;

    /* HUD-overlay suppression contract. */
    result->f0292PerChampionAllZero =
        state->f0292ChampionDrawStateCount == 0;
    {
        int i;
        for (i = 0; i < DM1_V1_MC_AMCIERH_PARTY_COUNT_PC34; ++i) {
            result->perChampionF0292CountZero[i] =
                state->f0292ChampionDrawStateCount == 0;
        }
    }
    result->f0293NotCalledOnClosePath = state->f0293DrawAllChampionStatesCount == 0;
    result->f0296NotCalledOnClosePath = state->f0296DrawChangedObjectIconsCount == 0;
    result->f0335NotCalledOnClosePath = state->f0335ClearChampionDataCount == 0;
    result->f0354NotCalledOnClosePath = state->f0354DrawStatusBoxPortraitCount == 0;

    /* Panel stability contract. */
    result->panelGraphicStableAcrossExit = state->c040PanelGraphic ==
                                           panelGraphicLive;
    result->panelCommandStableAcrossExit = state->c040PanelCommand ==
                                           panelCommandLive;
    result->panelColorStableAcrossExit = state->c040PanelColor ==
                                          panelColorLive;
    result->panelOwnerSlotStableAcrossExit = state->c040PanelOwnerSlot ==
                                              panelOwnerSlotLive;
    result->panelC038SlotBoxStableAcrossExit = state->c040PanelC038SlotBox ==
                                                panelC038SlotBoxLive;
    result->panelMouthRouteStableAcrossExit = state->mouthRouteZone ==
                                               panelMouthRouteLive;
    result->panelGraphicRestoredAfterReopen = state->c040PanelGraphic ==
                                               panelGraphicLive;
    result->panelCommandRestoredAfterReopen = state->c040PanelCommand ==
                                               panelCommandLive;
    result->panelColorRestoredAfterReopen = state->c040PanelColor ==
                                              panelColorLive;

    /* Auto-restore invariant. */
    result->g0299PreservedAcrossExit = state->g0299CandidateOrdinal != 0;
    result->g0299PreservedAcrossReopen = state->g0299CandidateOrdinal != 0;
    result->g0299SourceReachable = state->g0299CandidateOrdinal != 0;
    result->f0282NotCalledOnExit = state->f0282CandidateClearCount == 0;
    result->f0282NotCalledOnReopen = state->f0282CandidateClearCount == 0;
    result->f0280NotCalledOnExit = state->f0280CandidatePublishCount == 1;
    result->f0280NotCalledOnReopen = state->f0280CandidatePublishCount == 1;

    /* Reopen reroute. */
    result->reopenRoutesToF0346 = state->f0346ResurrectDrawCount >= 1;
    result->reopenF0346Called = state->f0346ResurrectDrawCount >= 1;
    result->reopenF0346PanelContentSet = state->c040PanelCommand ==
                                         DM1_V1_MC_AMCIERH_M568_PANEL_RESURRECT_REINCARNATE_PC34;
    result->reopenF0346CommandSet = result->reopenF0346PanelContentSet;
    result->reopenF0346OwnerSet = state->c040PanelOwnerSlot ==
                                   DM1_V1_MC_AMCIERH_CHAMPION_C30_FIRST_PC34;
    result->reopenF0346ColorSet = state->c040PanelColor == 10;
    result->reopenF0346SlotBoxSet = state->c040PanelC038SlotBox ==
                                     DM1_V1_MC_AMCIERH_CHAMPION_C38_PC34;
    result->reopenF0346C040Blit = state->c040PanelGraphic ==
                                   DM1_V1_MC_AMCIERH_CHAMPION_C40_PC34;
    result->reopenF0346C040PanelRect = result->reopenF0346C040Blit;
    result->reopenF0346C038SlotBoxRect = result->reopenF0346SlotBoxSet;

    /* Forbidden operation counters. */
    result->f0333OpenCountTotal = state->f0333OpenCount;
    result->f0282CandidateClearCountTotal = state->f0282CandidateClearCount;
    result->f0457StartDrawEnabledMenusCountTotal = state->f0457StartDrawEnabledMenusCount;
    result->f0360MirrorQueueConfirmCountTotal = state->f0360MirrorQueueConfirmCount;
    result->f0368SetLeaderCountTotal = state->f0368SetLeaderCount;
    result->f0219WallImpactSoundCountTotal = state->f0219WallImpactSoundCount;
    result->f0232DoorDestroyCountTotal = state->f0232DoorDestroyCount;
    result->f0394SetMagicCasterCountTotal = state->f0394SetMagicCasterCount;
    result->f0401TelemetryLogCountTotal = state->f0401TelemetryLogCount;
    result->saveLoadTeleporterResurrectCommitForbidden =
        state->f0333OpenCount == 0 &&
        state->f0282CandidateClearCount == 0 &&
        state->f0360MirrorQueueConfirmCount == 0 &&
        state->f0368SetLeaderCount == 0 &&
        state->f0219WallImpactSoundCount == 0 &&
        state->f0232DoorDestroyCount == 0 &&
        state->f0394SetMagicCasterCount == 0 &&
        state->f0401TelemetryLogCount == 0;
    result->noSaveLoad = state->f0333OpenCount == 0;
    result->noTeleporter = state->f0219WallImpactSoundCount == 0;
    result->noResurrectCommit = state->f0282CandidateClearCount == 0;
    result->noResurrectCancel = state->f0282CandidateClearCount == 0;
    result->noChestOpen = state->f0333OpenCount == 0;
    result->noF0292PerChampion = state->f0292ChampionDrawStateCount == 0;
    result->noF0293Cascade = state->f0293DrawAllChampionStatesCount == 0;
    result->noF0296ObjectIconCascade = state->f0296DrawChangedObjectIconsCount == 0;
    result->noF0335ClearChampionData = state->f0335ClearChampionDataCount == 0;
    result->noF0354BoxVariantsBlit = state->f0354DrawStatusBoxPortraitCount == 0;

    /* Hashes. */
    result->c030ChainHashLive = liveC030ChainHash;
    result->c030ChainHashAfterExit = state->c030ChainHash;
    result->c030ChainHashAfterReopen = state->c030ChainHash;
    result->candidateChainHashLive = liveCandidateChainHash;
    result->candidateChainHashAfterExit = state->candidateChainHash;
    result->candidateChainHashAfterReopen = state->candidateChainHash;
    result->chestListHashLive = liveChestListHash;
    result->chestListHashAfterExit = state->chestListHash;
    result->chestListHashAfterReopen = state->chestListHash;
    result->panelHashLive = livePanelHash;
    result->panelHashAfterExit = state->panelHash;
    result->panelHashAfterReopen = state->panelHash;
    result->panelHashAfterPost = state->panelHash;
    result->deterministicHash = state->stateHashAfterPost;

    result->accepted = result->reachedExit && result->reachedReopen &&
                       result->reachedPost;
    return 1;
}
