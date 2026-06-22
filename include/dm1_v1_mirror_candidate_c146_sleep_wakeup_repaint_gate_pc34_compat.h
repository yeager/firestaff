#ifndef DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * DM1 V1 mirror-candidate C040 panel redraw on C146_COMMAND_WAKE_UP gate.
 *
 * Source-lock anchors (ReDMCSB WIP 20210206, PC 3.4 path, MEDIA009+):
 *  - COMMAND.C F0380_COMMAND_ProcessQueue_CPSC:2361-2364 dispatches
 *    C146_COMMAND_WAKE_UP unconditionally (no G0299 guard), so the
 *    wake-up path must NOT touch the C040 mirror-candidate panel.
 *  - CHAMPION.C F0314_CHAMPION_WakeUp:1382-1414 owns the wake-up
 *    body: clears G0300_B_PartyIsResting, sets
 *    G0318_i_WaitForInputMaximumVerticalBlankCount, calls
 *    F0098_DUNGEONVIEW_DrawFloorAndCeiling, restores G0441..G0444
 *    input handlers, calls F0357_COMMAND_DiscardAllInput, and calls
 *    F0457_START_DrawEnabledMenus_CPSF. None of these touch the
 *    C040 panel rectangle (G0032_ai_Graphic562_Box_Panel) or
 *    G0299_ui_CandidateChampionOrdinal or F0346.
 *  - CHAMPION.C F0297_CHAMPION_PutObjectInLeaderHand + F0292 +
 *    F0293 panel hand redraw path: the wake-up path does NOT trigger
 *    this chain because G0506_ui_ActingChampionOrdinal is zero
 *    between rest cycles.
 *  - STARTUP2.C F0456_START_DrawDisabledMenus:335-385 + F0457_START_
 *    DrawEnabledMenus_CPSF:388-441 own the menu redraw on the
 *    rest/wake-up edge. F0457 calls F0379_COMMAND_DrawRestScreen +
 *    F0097_DUNGEONVIEW_DrawViewport when G0300_B_PartyIsResting is
 *    still true, otherwise it redraws the spell/action/inventory
 *    or floor+arrows via F0098_DUNGEONVIEW_DrawFloorAndCeiling +
 *    F0395_MENUS_DrawMovementArrows. None of these redraw the
 *    C040 panel rectangle (G0032_ai_Graphic562_Box_Panel).
 *  - COMMAND.C F0379_COMMAND_DrawRestScreen:1996-2034 only clears
 *    the viewport and prints "WAKE UP" centered; it does NOT touch
 *    the panel rectangle or the C040 graphic.
 *  - COMMAND.C F0380_COMMAND_ProcessQueue_CPSC:2336-2358 gates the
 *    C145_COMMAND_REST command on
 *    `!G0299_ui_CandidateChampionOrdinal` (CHANGE2_15_FIX) so the
 *    standard flow cannot enter rest while the C040 panel is live.
 *    BUG0_53 documents the duplicate-champion clone risk that this
 *    gate closes.
 *  - REVIVE.C F0280:124-132 publishes the mirror candidate (the
 *    F0280 candidate-open path); F0282:744-806 consumes/clears it.
 *    Neither runs on the wake-up path, so the C040 panel rectangle
 *    must remain byte-stable.
 *  - PANEL.C F0346_INVENTORY_DrawPanel_ResurrectReincarnate:1619-
 *    1637 owns the M568_PANEL_RESURRECT_REINCARNATE blit via
 *    M519_F0020_MAIN_BlitToViewport on
 *    G0032_ai_Graphic562_Box_Panel. The wake-up path does NOT call
 *    this blit so the panel rectangle pixels are preserved without
 *    flicker.
 *  - PANEL.C F0347_INVENTORY_DrawPanel:1639-1693 routes to F0346
 *    only when G0299_ui_CandidateChampionOrdinal != 0. The wake-up
 *    path does NOT call F0347.
 *  - DEFS.H:303 C070_COMMAND_CLICK_ON_MOUTH; DEFS.H:335
 *    C146_COMMAND_WAKE_UP; DEFS.H:336 C147_COMMAND_FREEZE_GAME;
 *    DEFS.H:337 C148_COMMAND_UNFREEZE_GAME; DEFS.H:2088
 *    C10_COLOR_FLESH; DEFS.H:2200 C040_GRAPHIC_PANEL_RESURRECT_
 *    REINCARNATE; DEFS.H:3001-3008 M568_PANEL_RESURRECT_REINCARNATE;
 *    DEFS.H:5326 G0032_ai_Graphic562_Box_Panel; DEFS.H:5694
 *    G0299_ui_CandidateChampionOrdinal; DEFS.H:5695
 *    G0300_B_PartyIsResting.
 *
 * No bitmap sampling, no GRAPHICS.DAT / DUNGEON.DAT load, no
 * real-asset or original-DOS pixel parity claim.
 *
 * Disjoint from:
 *  - pass784 (C040 cancel-then-reopen same tick),
 *  - pass785 (C040 inventory-toggle while panel live),
 *  - pass786 (C040 spell-area-click with G0514 magic-caster gate),
 *  - pass787 (C040 action-area-click while panel live),
 *  - pass788 (C040 status-box-click while panel live),
 *  - pass789 (C040 save-game while panel live),
 *  - c045_accept_dead_owner_guard / c045_close_after_non_candidate_
 *    transition / c045_food_water_*,
 *  - c061_drop_resurrect_pending,
 *  - c159_click_rotation_combo / c160_close_while_rotation_pending /
 *    c161_cancel_after_f0334_pending,
 *  - c545_accept_during_rotation / c545_drop_while_panel_live /
 *    c545_pickup_while_panel_live,
 *  - chest_close_leader_hand_pickup / chest_close_pending_panel /
 *    chest_open_during_pending,
 *  - click_cancel / click_cancel_with_rotation,
 *  - close_button / close_while_c045_pending,
 *  - double_open_close / double_open_close_guard,
 *  - full_chain / icon_refresh / inventory_full_swap,
 *  - m568_chest_close_order / open_after_save_load,
 *  - panel_redraw_after_inventory_exit,
 *  - resurrect_confirm_inventory_interrupt / resurrect_chest_close_
 *    order,
 *  - rotation_during_resurrect_confirmation,
 *  - teleporter_survival,
 *  - close_after_party_shuffle / close_order_party_shuffle,
 *  - close_while_resurrect_pending_with_inventory_pickup,
 *  - eye_slot_swap,
 *  - c040_redraw_after_chest_close (chest-close path; this gate
 *    covers the wake-up-from-rest path; they share the panel-hash-
 *    stable invariant but operate on different transitions).
 */

#define DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_SLOT_COUNT_PC34_COMPAT 8

enum {
    DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_NONE_PC34_COMPAT =
        0xFFFF,
    DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C10_FLESH_PC34_COMPAT = 10,
    DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C30_SLOT_PC34_COMPAT = 30,
    DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C37_SLOT_PC34_COMPAT = 37,
    DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C38_BOX_PC34_COMPAT = 38,
    DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C040_PANEL_PC34_COMPAT = 40,
    DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C070_MOUTH_PC34_COMPAT = 70,
    DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C146_WAKE_UP_PC34_COMPAT = 146,
    DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C147_FREEZE_GAME_PC34_COMPAT = 147,
    DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C148_UNFREEZE_GAME_PC34_COMPAT = 148,
    DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C537_VISIBLE_PC34_COMPAT = 537,
    DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C544_VISIBLE_PC34_COMPAT = 544,
    DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C545_ZONE_PC34_COMPAT = 545,
    DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_M568_CANDIDATE_PANEL_PC34_COMPAT = 568,
    DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_M569_CHEST_PANEL_PC34_COMPAT = 569,
    DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_M643_PANEL_SCROLL_PC34_COMPAT = 643,
    DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_M565_FOOD_WATER_PANEL_PC34_COMPAT = 565,
    DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_G0299_CANDIDATE_PC34_COMPAT = 299,
    DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_G0300_PARTY_RESTING_PC34_COMPAT = 300,
    DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_G0318_WAIT_VBLANKS_PC34_COMPAT = 318,
    DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_G0423_INVENTORY_CHAMPION_PC34_COMPAT = 423,
    DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_G0424_PANEL_CONTENT_PC34_COMPAT = 424
};

typedef struct
    Dm1V1MirrorC146SleepWakeupRepaintGateEvidencePc34Compat {
    int contractOnly;
    const char *wakeUpDispatchAnchor;
    const char *wakeUpBodyAnchor;
    const char *menuDisableAnchor;
    const char *menuEnableAnchor;
    const char *restScreenDrawAnchor;
    const char *floorCeilingAnchor;
    const char *movementArrowsAnchor;
    const char *restGateAnchor;
    const char *candidateOpenAnchor;
    const char *candidateCloseAnchor;
    const char *panelResurrectReincarnateAnchor;
    const char *panelDrawRouterAnchor;
    const char *panelHandRedrawAnchor;
    const char *actingChampionClearAnchor;
    const char *chestListOpenCloseAnchor;
    const char *defsAnchor;
    const char *contractScope;
} Dm1V1MirrorC146SleepWakeupRepaintGateEvidencePc34Compat;

typedef struct
    Dm1V1MirrorC146SleepWakeupRepaintGateSpecPc34Compat {
    unsigned int deterministicSeed;
    int leaderIndex;
    int partyChampionCount;
    unsigned int candidateOrdinal;
    int c146WakeUpCommand;
    int c145RestCommand;
    int c147FreezeGameCommand;
    int c148UnfreezeGameCommand;
    int c040PanelGraphic;
    int c10PanelColor;
    int m568CandidatePanel;
    int m565FoodWaterPanel;
    int m643ScrollPanel;
    int m569ChestPanel;
    int c070MouthCommand;
    int c537VisibleBase;
    int c544VisibleTop;
    int panelBoxXMin;
    int panelBoxXMax;
    int panelBoxYMin;
    int panelBoxYMax;
    unsigned int restEntryVerticalBlanks;
} Dm1V1MirrorC146SleepWakeupRepaintGateSpecPc34Compat;

typedef struct
    Dm1V1MirrorC146SleepWakeupRepaintGateStatePc34Compat {
    int contractOnly;
    unsigned int deterministicSeed;
    int leaderIndex;
    int partyChampionCount;
    unsigned int candidateOrdinal;
    unsigned int g0299CandidateOrdinal;
    int candidateMarkerThing;
    int c040PanelOpen;
    int c040PanelGraphic;
    int c040PanelCommand;
    int c040PanelColor;
    int c040PanelOwnerSlot;
    int c038SlotBox;
    int c030HandSlot;
    int c037HandSlot;
    int g0424PanelContent;
    int g0423InventoryChampionOrdinal;
    int g0300PartyIsResting;
    int g0506ActingChampionOrdinal;
    unsigned int g0318WaitVerticalBlanks;
    int c070MouthRouteOpen;
    int c147FreezeFilledViewport;
    int c148UnfreezeRedrewMenus;
    int c146WakeUpCommand;
    int c145RestCommand;
    int c147FreezeCommand;
    int c148UnfreezeCommand;
    int mouthRouteZone;
    int mouthRouteCommand;
    int visibleC537ToC544
        [DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_SLOT_COUNT_PC34_COMPAT];
    int g0425ChestList
        [DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_SLOT_COUNT_PC34_COMPAT];
    int championHandC537ToC544
        [DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_SLOT_COUNT_PC34_COMPAT];
    unsigned int panelHashBeforeWakeUp;
    unsigned int panelHashAfterWakeUp;
    int f0314WakeUpCount;
    int f0098FloorCeilingCount;
    int f0456DisabledMenusCount;
    int f0457EnabledMenusCount;
    int f0379RestScreenCount;
    int f0346PanelResurrectReincarnateCount;
    int f0347PanelDrawRouterCount;
    int f0280CandidateOpenCount;
    int f0282CandidateCloseCount;
    int f0357InputDiscardCount;
    int f0292ChampionDrawStateCount;
    int f0293ChampionDrawAllStatesCount;
    int f0457ReenteredRestBranchCount;
    int panelFlickerCount;
    int redrawClobberCount;
    int candidateLeakCount;
    int wakeUpRejectedRestGateFlag;
    int wakeUpRedrewPanelRectangleFlag;
} Dm1V1MirrorC146SleepWakeupRepaintGateStatePc34Compat;

typedef struct
    Dm1V1MirrorC146SleepWakeupRepaintGateResultPc34Compat {
    const Dm1V1MirrorC146SleepWakeupRepaintGateEvidencePc34Compat *evidence;
    const Dm1V1MirrorC146SleepWakeupRepaintGateSpecPc34Compat *spec;
    int accepted;
    unsigned int deterministicHash;
    unsigned int initialPanelHash;
    unsigned int finalPanelHash;
    int initialPanelOpen;
    int finalPanelOpen;
    int initialPanelGraphic;
    int finalPanelGraphic;
    int initialPanelCommand;
    int finalPanelCommand;
    unsigned int initialCandidateOrdinal;
    unsigned int finalCandidateOrdinal;
    int initialPartyIsResting;
    int finalPartyIsResting;
    int initialPanelContent;
    int finalPanelContent;
    int initialInventoryChampionOrdinal;
    int finalInventoryChampionOrdinal;
    unsigned int initialWaitVerticalBlanks;
    unsigned int finalWaitVerticalBlanks;
    int initialActingChampionOrdinal;
    int finalActingChampionOrdinal;
    int c146WakeUpDispatched;
    int c145RestBlockedByCandidate;
    int c147FreezeSkipped;
    int c148UnfreezeSkipped;
    int panelHashStable;
    int candidateStillLive;
    int noPanelFlicker;
    int noRedrawClobber;
    int noCandidateLeakage;
    int partyRestingCleared;
    int waitVerticalBlanksArmed;
    int inputDiscardFired;
    int menuRedrawFired;
    int panelRedrawSkipped;
    int candidateCloseSkipped;
    int candidateOpenStable;
    int actingChampionOrdinalStable;
    int inventoryChampionOrdinalStable;
    int panelContentStable;
    int visibleSlotsCleared;
    int chestListStable;
    int championHandStateStable;
    int f0314WakeUpCount;
    int f0098FloorCeilingCount;
    int f0456DisabledMenusCount;
    int f0457EnabledMenusCount;
    int f0379RestScreenCount;
    int f0346PanelResurrectReincarnateCount;
    int f0347PanelDrawRouterCount;
    int f0280CandidateOpenCount;
    int f0282CandidateCloseCount;
    int f0357InputDiscardCount;
    int f0292ChampionDrawStateCount;
    int f0293ChampionDrawAllStatesCount;
    int f0457ReenteredRestBranchCount;
    int visibleBefore
        [DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_SLOT_COUNT_PC34_COMPAT];
    int visibleAfter
        [DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_SLOT_COUNT_PC34_COMPAT];
    int chestListBefore
        [DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_SLOT_COUNT_PC34_COMPAT];
    int chestListAfter
        [DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_SLOT_COUNT_PC34_COMPAT];
    int championHandBefore
        [DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_SLOT_COUNT_PC34_COMPAT];
    int championHandAfter
        [DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_SLOT_COUNT_PC34_COMPAT];
    int rejectsNullState;
    int rejectsNullResult;
    int rejectsNonContract;
    int rejectsNoPanel;
    int rejectsNoCandidate;
    int rejectsPartyNotResting;
    int rejectsFrozenGame;
    int rejectsWrongWakeUpCommand;
    int rejectsWrongRestCommand;
    int rejectsWrongPanelContent;
    int rejectsMouthRouteOpen;
    int rejectsCandidateLeakPreload;
    int mutationGuardsOk;
} Dm1V1MirrorC146SleepWakeupRepaintGateResultPc34Compat;

void dm1_v1_mirror_candidate_c146_sleep_wakeup_repaint_gate_init_pc34_compat(
    Dm1V1MirrorC146SleepWakeupRepaintGateStatePc34Compat *state);

int dm1_v1_mirror_candidate_c146_sleep_wakeup_repaint_gate_run_pc34_compat(
    Dm1V1MirrorC146SleepWakeupRepaintGateStatePc34Compat *state,
    Dm1V1MirrorC146SleepWakeupRepaintGateResultPc34Compat *outResult);

const Dm1V1MirrorC146SleepWakeupRepaintGateEvidencePc34Compat *
dm1_v1_mirror_candidate_c146_sleep_wakeup_repaint_gate_evidence_pc34_compat(
    void);

const Dm1V1MirrorC146SleepWakeupRepaintGateSpecPc34Compat *
dm1_v1_mirror_candidate_c146_sleep_wakeup_repaint_gate_spec_pc34_compat(void);

const char *
dm1_v1_mirror_candidate_c146_sleep_wakeup_repaint_gate_source_evidence_pc34_compat(
    void);

#ifdef __cplusplus
}
#endif

#endif
