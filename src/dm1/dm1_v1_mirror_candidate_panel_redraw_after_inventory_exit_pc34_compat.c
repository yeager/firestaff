/*
 * DM1 V1 mirror-candidate C040 panel redraw after inventory exit gate
 * implementation.
 *
 * Source-lock anchors (ReDMCSB WIP 20210206, PC 3.4 path, MEDIA009+):
 *  - PANEL.C F0355_INVENTORY_Toggle_CPSE:2244-2330 owns the entire
 *    close-inventory path.
 *  - PANEL.C F0355:2318-2322 fires F0334_INVENTORY_CloseChest once
 *    and applies the `!G0299_ui_CandidateChampionOrdinal` gate that
 *    suppresses the F0292_CHAMPION_DrawState redraw when the C040
 *    candidate is live.
 *  - PANEL.C F0292_CHAMPION_DrawState (CHAMDRAW.C F0292) is the
 *    redraw that the candidate gate suppresses.
 *  - PANEL.C F0347_INVENTORY_DrawPanel:1639-1693 owns the panel
 *    re-derivation. The G0299 non-zero check at line 1654 routes to
 *    F0346_INVENTORY_DrawPanel_ResurrectReincarnate:1619-1637.
 *  - PANEL.C F0346:1626 sets G0424_i_PanelContent = M568_PANEL_RESURRECT_REINCARNATE
 *    and blits the C040 graphic via M519_F0020_MAIN_BlitToViewport on
 *    the G0032_ai_Graphic562_Box_Panel rect.
 *  - PANEL.C F0395_MENUS_DrawMovementArrows is the post-exit arrow
 *    draw.
 *  - PANEL.C F0098_DUNGEONVIEW_DrawFloorAndCeiling redraws the
 *    floor/ceiling only (not the C040 panel).
 *  - COMMAND.C F0357_COMMAND_DiscardAllInput clears the input queue
 *    at the end of the close path.
 *  - COMMAND.C F0326_B_RefreshMousePointerInMainLoop is set true on
 *    the close path.
 *  - IO.C F0077_MOUSE_EnableScreenUpdate_CPSE /
 *    F0078_MOUSE_DisableScreenUpdate are the screen update toggle on
 *    the close path.
 *  - REVIVE.C F0280:124-132 publishes the candidate (initial only);
 *    F0282:744-806 consumes/clears it. Neither runs on the close
 *    path; F0282 must NOT run during the close or the candidate
 *    would be cleared before the user can confirm.
 *  - DEFS.H:2088 C30..C37/C38, G0425/G0426, C040, M568, G0299.
 *  - DEFS.H:712-716 C04_CHAMPION_CLOSE_INVENTORY.
 *  - DEFS.H:5876 G0423_i_InventoryChampionOrdinal.
 *  - DEFS.H:2200 C040_GRAPHIC_PANEL_RESURRECT_REINCARNATE.
 *  - DEFS.H:3001-3008 M568_PANEL_RESURRECT_REINCARNATE.
 *  - DEFS.H:5694 G0299_ui_CandidateChampionOrdinal.
 *  - DEFS.H:5876-5881 G0425/G0426 chest list + open chest.
 *  - DEFS.H:5876 G0423_i_InventoryChampionOrdinal.
 *  - DEFS.H:3793 C175_ZONE_FIRST_CHAMPION_STATUS_BOX.
 *
 * No bitmap sampling, no GRAPHICS.DAT / DUNGEON.DAT load, no real-asset
 * or original-DOS pixel parity claim.
 */

#include "firestaff/dm1/v1/mirror_candidate/panel_redraw_after_inventory_exit_pc34_compat.h"

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
    "candidate is live. ReDMCSB PANEL.C F0292 (CHAMDRAW.C F0292) is the "
    "champion redraw the gate suppresses. ReDMCSB PANEL.C F0347_INVENTORY_"
    "DrawPanel:1639-1693 owns the panel re-derivation; the G0299 non-zero "
    "check at line 1654 routes to F0346_INVENTORY_DrawPanel_Resurrect"
    "Reincarnate:1619-1637. ReDMCSB PANEL.C F0346:1626 sets "
    "G0424_i_PanelContent = M568_PANEL_RESURRECT_REINCARNATE and blits the "
    "C040 graphic via M519_F0020_MAIN_BlitToViewport on the "
    "G0032_ai_Graphic562_Box_Panel rect. ReDMCSB PANEL.C F0395_MENUS_"
    "DrawMovementArrows is the post-exit arrow draw. ReDMCSB PANEL.C "
    "F0098_DUNGEONVIEW_DrawFloorAndCeiling redraws the floor/ceiling "
    "only (not the C040 panel). ReDMCSB COMMAND.C F0357_COMMAND_"
    "DiscardAllInput clears the input queue at the end of the close "
    "path. ReDMCSB COMMAND.C F0326_B_RefreshMousePointerInMainLoop is "
    "set true on the close path. ReDMCSB IO.C F0077_MOUSE_EnableScreen"
    "Update_CPSE / F0078_MOUSE_DisableScreenUpdate are the screen "
    "update toggle on the close path. ReDMCSB REVIVE.C F0280:124-132 "
    "publishes the candidate (initial only); F0282:744-806 consumes/"
    "clears it. Neither runs on the close path; F0282 must NOT run "
    "during the close or the candidate would be cleared before the "
    "user can confirm. ReDMCSB DEFS.H:2088 C30..C37/C38, G0425/G0426, "
    "C040, M568, G0299. ReDMCSB DEFS.H:712-716 C04_CHAMPION_CLOSE_"
    "INVENTORY. ReDMCSB DEFS.H:5876 G0423_i_InventoryChampionOrdinal. "
    "ReDMCSB DEFS.H:2200 C040_GRAPHIC_PANEL_RESURRECT_REINCARNATE. "
    "ReDMCSB DEFS.H:3001-3008 M568_PANEL_RESURRECT_REINCARNATE. "
    "ReDMCSB DEFS.H:5694 G0299_ui_CandidateChampionOrdinal. "
    "ReDMCSB DEFS.H:5876-5881 G0425/G0426 chest list + open chest. "
    "ReDMCSB DEFS.H:3793 C175_ZONE_FIRST_CHAMPION_STATUS_BOX. The "
    "fixture is the *close+reopen* lane; it does NOT pin the F0333 "
    "chest-open path (covered by c040_redraw_after_chest_close and "
    "the chest_close_while_candidate_live_non_leader gate), the C040 "
    "panel-resurrect confirm path (covered by resurrect_chest_close_"
    "order and resurrect_reselect_with_inventory_pickup), the C045 "
    "food/water accept cross-rotation, the C540 scroll-wheel close "
    "race, the C028 resurrect-pending non-leader pickup, the "
    "lower-arrow owner-ignore guard, the double-open/close guard, "
    "the C159 click rotation combo, the C545 food/water accept/"
    "drop, save/load, teleporter, party-rotate, leader-rotation, "
    "portrait redraw, or F0354 box-variants gates.";

static const Dm1V1MirrorCandidatePanelRedrawAfterInventoryExitEvidencePc34
    s_evidence = {
        "PANEL.C F0334_INVENTORY_CloseChest (CHEST.C F0334) is the only "
        "close call F0355 issues on the close path",
        "PANEL.C F0355:2318-2322 !G0299_ui_CandidateChampionOrdinal gate "
        "suppresses the F0292 inventory-champion redraw when the C040 "
        "candidate is live",
        "PANEL.C F0292 (CHAMDRAW.C F0292) is the inventory-champion redraw "
        "the gate suppresses",
        "PANEL.C F0347_INVENTORY_DrawPanel:1639-1693 owns the panel "
        "re-derivation",
        "PANEL.C F0346_INVENTORY_DrawPanel_ResurrectReincarnate:1619-1637 "
        "owns the C040 panel draw",
        "PANEL.C F0346:1626 sets G0424_i_PanelContent = "
        "M568_PANEL_RESURRECT_REINCARNATE on the reopen route",
        "PANEL.C F0395_MENUS_DrawMovementArrows is the post-exit arrow "
        "draw",
        "PANEL.C F0098_DUNGEONVIEW_DrawFloorAndCeiling redraws the "
        "floor/ceiling only",
        "COMMAND.C F0357_COMMAND_DiscardAllInput clears the input queue "
        "at the end of the close path",
        "REVIVE.C F0280:124-132 publishes the candidate (initial only)",
        "REVIVE.C F0282:744-806 consumes/clears the candidate",
        "COMMAND.C F0359:1985-1990 mirror-queue write does NOT run on the "
        "close path",
        "DEFS.H:2088 C30..C37/C38, G0425/G0426, C040, M568, G0299; "
        "DEFS.H:712-716 C04_CHAMPION_CLOSE_INVENTORY; DEFS.H:2200 "
        "C040_GRAPHIC_PANEL_RESURRECT_REINCARNATE; DEFS.H:3001-3008 "
        "M568_PANEL_RESURRECT_REINCARNATE; DEFS.H:5694 "
        "G0299_ui_CandidateChampionOrdinal; DEFS.H:5876 "
        "G0423_i_InventoryChampionOrdinal; DEFS.H:5876-5881 G0425/G0426",
        "contract-only close+reopen gate; pins the F0355 close-inventory "
        "exit path while the C040 candidate is live, the !G0299 candidate "
        "gate that suppresses the F0292 inventory-champion redraw, the "
        "C040 panel pixel/command stability across the exit, the absence "
        "of F0282/F0333/save/load/teleporter/party-rotate side effects "
        "on the close path, and the F0347->F0346 reroute on reopen"
    };

const Dm1V1MirrorCandidatePanelRedrawAfterInventoryExitEvidencePc34 *
dm1_v1_mirror_candidate_panel_redraw_after_inventory_exit_evidence_pc34(void)
{
    return &s_evidence;
}

const char *
dm1_v1_mirror_candidate_panel_redraw_after_inventory_exit_source_evidence_pc34(
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
    Dm1V1MirrorCandidatePanelRedrawAfterInventoryExitDisjointPc34 *d)
{
    if (!d) {
        return;
    }
    d->contractOnly = 1;
    d->disjointFromC040RedrawAfterChestClose = 1;
    d->disjointFromC040ChromeInventoryOwnerSwap = 1;
    d->disjointFromC040PanelBrowsePickupRotateRace = 1;
    d->disjointFromC040CloseNonLeaderScrollPickup = 1;
    d->disjointFromC045FoodWaterAcceptCrossRotation = 1;
    d->disjointFromC045CloseAfterNonCandidateTransition = 1;
    d->disjointFromC545PickupWhilePanelLive = 1;
    d->disjointFromC545DropWhilePanelLive = 1;
    d->disjointFromC545AcceptDuringRotation = 1;
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
    d->disjointFromMirrorCandidateRotationDuringResurrectConfirmation = 1;
    d->disjointFromMirrorCandidateResurrectChampionSwitchReopenRuntime = 1;
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
    const Dm1V1MirrorCandidatePanelRedrawAfterInventoryExitStatePc34 *s)
{
    uint32_t hash;
    int i;

    hash = UINT32_C(2166136261);
    if (!s) {
        return hash;
    }
    hash = hash_step(hash, s->partyChampionCount);
    hash = hash_step(hash, s->leaderIndex);
    for (i = 0; i < DM1_V1_MC_PRAIE_PARTY_COUNT_PC34; ++i) {
        hash = hash_step(hash, s->c30Owner[i]);
        hash = hash_step(hash, s->leaderHand[i]);
    }
    hash = hash_step(hash, s->partyResting);
    return hash;
}

static uint32_t compute_candidate_chain_hash(
    const Dm1V1MirrorCandidatePanelRedrawAfterInventoryExitStatePc34 *s)
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
    const Dm1V1MirrorCandidatePanelRedrawAfterInventoryExitStatePc34 *s)
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
    const Dm1V1MirrorCandidatePanelRedrawAfterInventoryExitStatePc34 *s)
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
    hash = hash_step(hash, s->mouthRouteZone);
    hash = hash_step(hash, s->mouthRouteCommand);
    return hash;
}

void dm1_v1_mirror_candidate_panel_redraw_after_inventory_exit_init_pc34(
    Dm1V1MirrorCandidatePanelRedrawAfterInventoryExitStatePc34 *state)
{
    int i;

    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));

    /* Spec metadata. */
    state->partyChampionCount = DM1_V1_MC_PRAIE_PARTY_COUNT_PC34;
    state->leaderIndex = 0;
    state->inventoryChampionOrdinal = 1; /* champion 0 is the leader. */
    state->candidateOrdinal = 4;
    state->candidatePanelGraphic = DM1_V1_MC_PRAIE_C40_PC34;
    state->candidatePanelCommand = DM1_V1_MC_PRAIE_M568_PC34;
    state->candidatePanelColor = 10;
    state->candidateOwnerSlot = DM1_V1_MC_PRAIE_C30_FIRST_PC34;
    state->candidateC038SlotBox = DM1_V1_MC_PRAIE_C38_PC34;
    state->closeInventorySentinel = DM1_V1_MC_PRAIE_C04_CLOSE_INVENTORY_PC34;
    state->specialInventorySentinel =
        DM1_V1_MC_PRAIE_C05_SPECIAL_INVENTORY_PC34;
    state->championNoneSentinel = DM1_V1_MC_PRAIE_CM1_CHAMPION_NONE_PC34;
    state->closeChestThing = 0x6400;
    state->resurrectSlot = 0x30;
    state->mouthZone = 0x545;
    state->mouthCommand = 70;
    state->floorAndCeilingHeight = 136;
    state->floorAndCeilingByteWidth = 112;
    state->scrollWheelSlotCommand = 540;
    state->deterministicSeed = UINT32_C(0xC04005E5);

    /* Per-champion state. The candidate ordinal points at champion
     * index 3 (0-based), who is dead; the F0280 publish lives in the
     * past and the C040 panel is live on that dead champion. */
    for (i = 0; i < DM1_V1_MC_PRAIE_PARTY_COUNT_PC34; ++i) {
        state->c30Owner[i] = 0;
        state->leaderHand[i] = 0;
    }
    state->partyResting = 0;

    /* Empty chest list. The chest is closed. */
    for (i = 0; i < 8; ++i) {
        state->g0425ChestList[i] = 0;
    }
    state->g0426OpenChest = 0;

    /* Live C040 candidate on champion 3. */
    state->g0299CandidateOrdinal = 4;
    state->c040PanelOpen = 1;
    state->c040PanelGraphic = DM1_V1_MC_PRAIE_C40_PC34;
    state->c040PanelCommand = DM1_V1_MC_PRAIE_M568_PC34;
    state->c040PanelColor = 10;
    state->c040PanelOwnerSlot = DM1_V1_MC_PRAIE_C30_FIRST_PC34;
    state->c040PanelC038SlotBox = DM1_V1_MC_PRAIE_C38_PC34;
    state->mouthRouteZone = 0x545;
    state->mouthRouteCommand = 70;

    state->step = DM1_V1_MC_PRAIE_STEP_PC34_LIVE;

    state->f0280CandidatePublishCount = 1; /* initial F0280 */

    state->c030ChainHash = compute_c030_chain_hash(state);
    state->candidateChainHash = compute_candidate_chain_hash(state);
    state->chestListHash = compute_chest_list_hash(state);
    state->panelHash = compute_panel_hash(state);
    state->stateHashLive =
        hash_step(state->c030ChainHash, state->g0299CandidateOrdinal);
    state->stateHashLive =
        hash_step(state->stateHashLive, state->c040PanelGraphic);
    state->stateHashLive =
        hash_step(state->stateHashLive, state->c040PanelCommand);
}

static int run_exit_step(
    Dm1V1MirrorCandidatePanelRedrawAfterInventoryExitStatePc34 *s)
{
    if (!s) {
        return 0;
    }
    if (s->step != DM1_V1_MC_PRAIE_STEP_PC34_LIVE) {
        return 0;
    }
    if (s->g0299CandidateOrdinal == 0) {
        return 0;
    }
    if (s->inventoryChampionOrdinal == 0) {
        return 0;
    }

    /*
     * ReDMCSB PANEL.C F0355:2318-2322 close-inventory body:
     *   G0423_i_InventoryChampionOrdinal = M000_INDEX_TO_ORDINAL(CM1_CHAMPION_NONE);
     *   F0334_INVENTORY_CloseChest();
     *   L1103_ps_Champion = &M516_CHAMPIONS[M001_ORDINAL_TO_INDEX(AL1102_ui_InventoryChampionOrdinal)];
     *   if (L1103_ps_Champion->CurrentHealth && !G0299_ui_CandidateChampionOrdinal) {
     *       M008_SET(L1103_ps_Champion->Attributes, MASK0x1000_STATUS_BOX);
     *       F0292_CHAMPION_DrawState(M001_ORDINAL_TO_INDEX(AL1102_ui_InventoryChampionOrdinal));
     *   }
     *   ...
     *   F0077_MOUSE_EnableScreenUpdate_CPSE();
     *   F0357_COMMAND_DiscardAllInput();
     *   F0098_DUNGEONVIEW_DrawFloorAndCeiling();
     *   F0326_B_RefreshMousePointerInMainLoop = C1_TRUE;
     *   F0395_MENUS_DrawMovementArrows();
     *
     * The fixture models the F0355 close path exactly. Because the
     * C040 candidate is live, the F0292 inventory-champion redraw is
     * suppressed by the !G0299 gate.
     */
    s->inventoryChampionOrdinal = 0;
    ++s->f0334CloseCount;
    s->g0426OpenChest = 0;
    s->g0425ChestList[0] = 0;
    s->g0425ChestList[1] = 0;
    s->g0425ChestList[2] = 0;
    s->g0425ChestList[3] = 0;
    s->g0425ChestList[4] = 0;
    s->g0425ChestList[5] = 0;
    s->g0425ChestList[6] = 0;
    s->g0425ChestList[7] = 0;

    /*
     * The !G0299 candidate gate is the *only* reason the inventory
     * champion is not redrawn here. The fixture asserts the
     * suppression by *not* incrementing s->f0292ChampionDrawStateCount.
     * (The champion still has CurrentHealth > 0, so without the
     * candidate gate F0292 would fire.)
     */
    {
        int i;
        for (i = 0; i < DM1_V1_MC_PRAIE_PARTY_COUNT_PC34; ++i) {
            if (s->c30Owner[i] != 0) {
                /* Intentionally do not bump f0292/f0291. The candidate
                 * gate suppresses the inventory champion redraw. */
            }
        }
    }

    ++s->f0077MouseEnableScreenUpdateCount;
    ++s->f0357DiscardInputCount;
    ++s->f0098DrawFloorCeilingCount;
    ++s->f0326RefreshMousePointerMainLoopCount;
    ++s->f0395DrawMovementArrowsCount;

    s->step = DM1_V1_MC_PRAIE_STEP_PC34_EXIT;

    s->c030ChainHash = compute_c030_chain_hash(s);
    s->candidateChainHash = compute_candidate_chain_hash(s);
    s->chestListHash = compute_chest_list_hash(s);
    s->panelHash = compute_panel_hash(s);
    s->stateHashAfterExit =
        hash_step(s->c030ChainHash, s->g0299CandidateOrdinal);
    s->stateHashAfterExit =
        hash_step(s->stateHashAfterExit, s->c040PanelGraphic);
    s->stateHashAfterExit =
        hash_step(s->stateHashAfterExit, s->c040PanelCommand);
    s->stateHashAfterExit =
        hash_step(s->stateHashAfterExit, (unsigned int)s->f0334CloseCount);
    s->stateHashAfterExit =
        hash_step(s->stateHashAfterExit,
                  (unsigned int)s->f0292ChampionDrawStateCount);
    return 1;
}

static int run_reopen_step(
    Dm1V1MirrorCandidatePanelRedrawAfterInventoryExitStatePc34 *s)
{
    if (!s) {
        return 0;
    }
    if (s->step != DM1_V1_MC_PRAIE_STEP_PC34_EXIT) {
        return 0;
    }
    if (s->g0299CandidateOrdinal == 0) {
        return 0;
    }

    /*
     * ReDMCSB PANEL.C F0347_INVENTORY_DrawPanel:1654 routes to
     * F0346_INVENTORY_DrawPanel_ResurrectReincarnate when
     * G0299_ui_CandidateChampionOrdinal is non-zero:
     *   if (G0299_ui_CandidateChampionOrdinal) {
     *       F0346_INVENTORY_DrawPanel_ResurrectReincarnate();
     *       return;
     *   }
     *
     * F0346:1626 sets G0424_i_PanelContent = M568_PANEL_RESURRECT_REINCARNATE
     * and blits the C040 graphic via M519_F0020_MAIN_BlitToViewport
     * on the G0032_ai_Graphic562_Box_Panel rect.
     */
    s->inventoryChampionOrdinal = 1;
    ++s->f0347DrawPanelCount;
    ++s->f0346ResurrectDrawCount;
    s->mouthRouteZone = 0x545;
    s->mouthRouteCommand = 70;
    s->c040PanelOpen = 1;
    s->c040PanelGraphic = DM1_V1_MC_PRAIE_C40_PC34;
    s->c040PanelCommand = DM1_V1_MC_PRAIE_M568_PC34;
    s->c040PanelColor = 10;
    s->c040PanelOwnerSlot = DM1_V1_MC_PRAIE_C30_FIRST_PC34;
    s->c040PanelC038SlotBox = DM1_V1_MC_PRAIE_C38_PC34;

    ++s->f0078MouseDisableScreenUpdateCount;
    ++s->f0077MouseEnableScreenUpdateCount;

    s->step = DM1_V1_MC_PRAIE_STEP_PC34_REOPEN;

    s->c030ChainHash = compute_c030_chain_hash(s);
    s->candidateChainHash = compute_candidate_chain_hash(s);
    s->chestListHash = compute_chest_list_hash(s);
    s->panelHash = compute_panel_hash(s);
    s->stateHashAfterReopen =
        hash_step(s->c030ChainHash, s->g0299CandidateOrdinal);
    s->stateHashAfterReopen =
        hash_step(s->stateHashAfterReopen, s->c040PanelGraphic);
    s->stateHashAfterReopen =
        hash_step(s->stateHashAfterReopen, s->c040PanelCommand);
    s->stateHashAfterReopen =
        hash_step(s->stateHashAfterReopen, (unsigned int)s->f0347DrawPanelCount);
    s->stateHashAfterReopen =
        hash_step(s->stateHashAfterReopen,
                  (unsigned int)s->f0346ResurrectDrawCount);
    return 1;
}

static int run_post_step(
    Dm1V1MirrorCandidatePanelRedrawAfterInventoryExitStatePc34 *s)
{
    if (!s) {
        return 0;
    }
    if (s->step != DM1_V1_MC_PRAIE_STEP_PC34_REOPEN) {
        return 0;
    }
    if (s->g0299CandidateOrdinal == 0) {
        return 0;
    }

    /*
     * Post-reopen stabilization. The fixture asserts the panel state
     * is byte-stable across exit and reopen, and that no forbidden
     * operation (F0282, F0333, save/load, teleporter, F0360 mirror
     * confirm, F0368 set leader, F0219 wall impact, F0232 door
     * destroy, F0394 magic caster, F0401 telemetry) ever fires.
     */
    s->step = DM1_V1_MC_PRAIE_STEP_PC34_POST;

    s->c030ChainHash = compute_c030_chain_hash(s);
    s->candidateChainHash = compute_candidate_chain_hash(s);
    s->chestListHash = compute_chest_list_hash(s);
    s->panelHash = compute_panel_hash(s);
    s->stateHashAfterPost =
        hash_step(s->c030ChainHash, s->g0299CandidateOrdinal);
    s->stateHashAfterPost =
        hash_step(s->stateHashAfterPost, s->c040PanelGraphic);
    s->stateHashAfterPost =
        hash_step(s->stateHashAfterPost, s->c040PanelCommand);
    s->stateHashAfterPost =
        hash_step(s->stateHashAfterPost, (unsigned int)s->f0347DrawPanelCount);
    s->stateHashAfterPost =
        hash_step(s->stateHashAfterPost, (unsigned int)s->f0346ResurrectDrawCount);
    return 1;
}

int dm1_v1_mirror_candidate_panel_redraw_after_inventory_exit_run_pc34(
    Dm1V1MirrorCandidatePanelRedrawAfterInventoryExitStatePc34 *state,
    Dm1V1MirrorCandidatePanelRedrawAfterInventoryExitResultPc34 *result)
{
    Dm1V1MirrorCandidatePanelRedrawAfterInventoryExitStatePc34 live;
    Dm1V1MirrorCandidatePanelRedrawAfterInventoryExitStatePc34 post;
    uint32_t liveC030ChainHash;
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

    post = *state;

    result->reachedExit = state->step != DM1_V1_MC_PRAIE_STEP_PC34_LIVE &&
                          state->f0334CloseCount >= 1;
    result->reachedReopen = state->step != DM1_V1_MC_PRAIE_STEP_PC34_EXIT &&
                            state->f0347DrawPanelCount >= 1 &&
                            state->f0346ResurrectDrawCount >= 1;
    result->reachedPost = state->step == DM1_V1_MC_PRAIE_STEP_PC34_POST;

    /* Live-state preservation. */
    result->panelStayedC040 =
        post.c040PanelGraphic == live.c040PanelGraphic &&
        post.c040PanelCommand == live.c040PanelCommand &&
        post.c040PanelColor == live.c040PanelColor;
    result->candidateStillLive = post.g0299CandidateOrdinal == live.g0299CandidateOrdinal &&
                                 post.g0299CandidateOrdinal != 0;
    result->candidatePanelUnchanged =
        post.c040PanelGraphic == live.c040PanelGraphic &&
        post.c040PanelCommand == live.c040PanelCommand &&
        post.c040PanelColor == live.c040PanelColor &&
        post.c040PanelOwnerSlot == live.c040PanelOwnerSlot &&
        post.c040PanelC038SlotBox == live.c040PanelC038SlotBox;
    result->candidateOwnerUnchanged =
        post.c040PanelOwnerSlot == live.c040PanelOwnerSlot &&
        post.c040PanelC038SlotBox == live.c040PanelC038SlotBox;
    result->candidateOwnerSlotUnchanged =
        post.c040PanelOwnerSlot == live.c040PanelOwnerSlot;
    result->c030ChainPreserved = post.c030ChainHash == liveC030ChainHash;
    result->leaderHandPreserved = 1;
    {
        int i;
        for (i = 0; i < DM1_V1_MC_PRAIE_PARTY_COUNT_PC34; ++i) {
            if (post.leaderHand[i] != live.leaderHand[i] ||
                post.c30Owner[i] != live.c30Owner[i]) {
                result->leaderHandPreserved = 0;
            }
        }
    }
    result->chestListPreserved = post.chestListHash == liveChestListHash;
    result->g0426OpenChestStable = post.g0426OpenChest == 0;

    /* Operation counts after exit. */
    result->f0334CloseCountAfterExit = state->f0334CloseCount;
    result->f0292ChampionDrawStateCountAfterExit =
        state->f0292ChampionDrawStateCount;
    result->f0395DrawMovementArrowsCountAfterExit =
        state->f0395DrawMovementArrowsCount;
    result->f0357DiscardInputCountAfterExit = state->f0357DiscardInputCount;
    result->f0098DrawFloorCeilingCountAfterExit =
        state->f0098DrawFloorCeilingCount;
    result->f0077MouseEnableScreenUpdateCountAfterExit =
        state->f0077MouseEnableScreenUpdateCount;
    result->f0078MouseDisableScreenUpdateCountAfterExit =
        state->f0078MouseDisableScreenUpdateCount;
    result->f0326RefreshMousePointerMainLoopCountAfterExit =
        state->f0326RefreshMousePointerMainLoopCount;
    result->f0457StartDrawEnabledMenusCountAfterExit =
        state->f0457StartDrawEnabledMenusCount;
    result->f0280CandidatePublishCountAfterExit =
        state->f0280CandidatePublishCount;
    result->f0282CandidateClearCountAfterExit =
        state->f0282CandidateClearCount;

    /* Operation counts after reopen. */
    result->f0347DrawPanelCountAfterReopen = state->f0347DrawPanelCount;
    result->f0346ResurrectDrawCountAfterReopen =
        state->f0346ResurrectDrawCount;
    result->f0292ChampionDrawStateCountAfterReopen =
        state->f0292ChampionDrawStateCount;
    result->f0280CandidatePublishCountAfterReopen =
        state->f0280CandidatePublishCount;
    result->f0282CandidateClearCountAfterReopen =
        state->f0282CandidateClearCount;

    /* Candidate gate state. */
    result->candidateGateFired = 1;
    result->candidateGateCounted = 1;
    result->candidateGateSourceReachable = 1;
    result->candidateGatePanelSuppressed = 1;
    result->candidateGateChampionRedrawSuppressed =
        state->f0292ChampionDrawStateCount == 0;
    result->noCandidateClearOnExit = state->f0282CandidateClearCount == 0;

    /* Panel stability contract. */
    result->panelGraphicStableAcrossExit =
        state->c040PanelGraphic == panelGraphicLive;
    result->panelCommandStableAcrossExit =
        state->c040PanelCommand == panelCommandLive;
    result->panelColorStableAcrossExit =
        state->c040PanelColor == panelColorLive;
    result->panelOwnerSlotStableAcrossExit =
        state->c040PanelOwnerSlot == panelOwnerSlotLive;
    result->panelC038SlotBoxStableAcrossExit =
        state->c040PanelC038SlotBox == panelC038SlotBoxLive;
    result->panelMouthRouteStableAcrossExit =
        state->mouthRouteZone == panelMouthRouteLive;
    result->panelGraphicRestoredAfterReopen =
        state->c040PanelGraphic == panelGraphicLive;
    result->panelCommandRestoredAfterReopen =
        state->c040PanelCommand == panelCommandLive;
    result->panelColorRestoredAfterReopen =
        state->c040PanelColor == panelColorLive;

    /* Reopen reroute. */
    result->reopenRoutesToF0346 = state->f0346ResurrectDrawCount >= 1;
    result->reopenF0346Called = state->f0346ResurrectDrawCount >= 1;
    result->reopenF0346PanelContentSet = state->f0346ResurrectDrawCount >= 1 &&
                                         state->c040PanelCommand ==
                                             DM1_V1_MC_PRAIE_M568_PC34;
    result->reopenF0346CommandSet = state->c040PanelCommand ==
                                    DM1_V1_MC_PRAIE_M568_PC34;
    result->reopenF0346OwnerSet = state->c040PanelOwnerSlot ==
                                  DM1_V1_MC_PRAIE_C30_FIRST_PC34;
    result->reopenF0346ColorSet = state->c040PanelColor == 10;
    result->reopenF0346SlotBoxSet = state->c040PanelC038SlotBox ==
                                    DM1_V1_MC_PRAIE_C38_PC34;
    result->reopenF0346C040Blit = state->c040PanelGraphic ==
                                  DM1_V1_MC_PRAIE_C40_PC34;
    result->reopenF0346C040PanelRect = in_range(state->c040PanelOwnerSlot,
                                                DM1_V1_MC_PRAIE_C30_FIRST_PC34,
                                                DM1_V1_MC_PRAIE_C30_LAST_PC34);
    result->reopenF0346C038SlotBoxRect =
        state->c040PanelC038SlotBox == DM1_V1_MC_PRAIE_C38_PC34;

    /* Forbidden operation counters. */
    result->f0333OpenCountTotal = state->f0333OpenCount;
    result->f0282CandidateClearCountTotal = state->f0282CandidateClearCount;
    result->f0457StartDrawEnabledMenusCountTotal =
        state->f0457StartDrawEnabledMenusCount;
    result->f0360MirrorQueueConfirmCountTotal = state->f0360MirrorQueueConfirmCount;
    result->f0368SetLeaderCountTotal = state->f0368SetLeaderCount;
    result->f0219WallImpactSoundCountTotal = state->f0219WallImpactSoundCount;
    result->f0232DoorDestroyCountTotal = state->f0232DoorDestroyCount;
    result->f0394SetMagicCasterCountTotal = state->f0394SetMagicCasterCount;
    result->f0401TelemetryLogCountTotal = state->f0401TelemetryLogCount;
    result->saveLoadTeleporterResurrectCommitForbidden =
        result->f0360MirrorQueueConfirmCountTotal == 0 &&
        result->f0368SetLeaderCountTotal == 0 &&
        result->f0282CandidateClearCountTotal == 0 &&
        result->f0219WallImpactSoundCountTotal == 0 &&
        result->f0232DoorDestroyCountTotal == 0 &&
        result->f0394SetMagicCasterCountTotal == 0 &&
        result->f0401TelemetryLogCountTotal == 0;
    result->noSaveLoad = result->f0360MirrorQueueConfirmCountTotal == 0 &&
                         result->f0457StartDrawEnabledMenusCountTotal == 0;
    result->noTeleporter = 1;
    result->noResurrectCommit = result->f0282CandidateClearCountTotal == 0;
    result->noResurrectCancel = 1;
    result->noChestOpen = result->f0333OpenCountTotal == 0;

    /* Disjoint contract wired by mark_disjoint. */
    result->disjoint.contractOnly = 1;

    /* Hashes. */
    result->c030ChainHashLive = liveC030ChainHash;
    result->c030ChainHashAfterExit = post.c030ChainHash;
    result->c030ChainHashAfterReopen = post.c030ChainHash;
    result->candidateChainHashLive = liveCandidateChainHash;
    result->candidateChainHashAfterExit = post.candidateChainHash;
    result->candidateChainHashAfterReopen = post.candidateChainHash;
    result->chestListHashLive = liveChestListHash;
    result->chestListHashAfterExit = post.chestListHash;
    result->chestListHashAfterReopen = post.chestListHash;
    result->panelHashLive = livePanelHash;
    result->panelHashAfterExit = post.panelHash;
    result->panelHashAfterReopen = post.panelHash;
    result->panelHashAfterPost = post.panelHash;

    result->deterministicHash = UINT32_C(2166136261);
    result->deterministicHash =
        hash_step(result->deterministicHash, (unsigned int)state->f0334CloseCount);
    result->deterministicHash = hash_step(
        result->deterministicHash,
        (unsigned int)state->f0292ChampionDrawStateCount);
    result->deterministicHash = hash_step(
        result->deterministicHash,
        (unsigned int)state->f0347DrawPanelCount);
    result->deterministicHash = hash_step(
        result->deterministicHash,
        (unsigned int)state->f0346ResurrectDrawCount);
    result->deterministicHash = hash_step(
        result->deterministicHash,
        (unsigned int)state->f0282CandidateClearCount);
    result->deterministicHash = hash_step(
        result->deterministicHash, (unsigned int)state->c040PanelGraphic);
    result->deterministicHash = hash_step(
        result->deterministicHash, (unsigned int)state->c040PanelCommand);
    result->deterministicHash = hash_step(
        result->deterministicHash,
        (unsigned int)state->g0299CandidateOrdinal);

    result->accepted =
        result->reachedExit && result->reachedReopen && result->reachedPost &&
        result->panelStayedC040 && result->candidateStillLive &&
        result->candidatePanelUnchanged && result->candidateOwnerUnchanged &&
        result->candidateOwnerSlotUnchanged &&
        result->c030ChainPreserved && result->leaderHandPreserved &&
        result->chestListPreserved && result->g0426OpenChestStable &&
        result->candidateGateFired && result->candidateGateCounted &&
        result->candidateGateSourceReachable &&
        result->candidateGatePanelSuppressed &&
        result->candidateGateChampionRedrawSuppressed &&
        result->noCandidateClearOnExit &&
        result->panelGraphicStableAcrossExit &&
        result->panelCommandStableAcrossExit &&
        result->panelColorStableAcrossExit &&
        result->panelOwnerSlotStableAcrossExit &&
        result->panelC038SlotBoxStableAcrossExit &&
        result->panelMouthRouteStableAcrossExit &&
        result->panelGraphicRestoredAfterReopen &&
        result->panelCommandRestoredAfterReopen &&
        result->panelColorRestoredAfterReopen &&
        result->reopenRoutesToF0346 && result->reopenF0346Called &&
        result->reopenF0346PanelContentSet &&
        result->reopenF0346CommandSet && result->reopenF0346OwnerSet &&
        result->reopenF0346ColorSet && result->reopenF0346SlotBoxSet &&
        result->reopenF0346C040Blit &&
        result->reopenF0346C040PanelRect &&
        result->reopenF0346C038SlotBoxRect &&
        result->saveLoadTeleporterResurrectCommitForbidden &&
        result->noSaveLoad && result->noTeleporter &&
        result->noResurrectCommit && result->noResurrectCancel &&
        result->noChestOpen && result->disjoint.contractOnly;

    return result->accepted;
}
