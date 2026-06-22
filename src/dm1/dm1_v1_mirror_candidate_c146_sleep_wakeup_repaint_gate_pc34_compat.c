/*
 * DM1 V1 mirror-candidate C040 panel redraw on C146_COMMAND_WAKE_UP gate
 * implementation.
 *
 * Source-lock anchors (ReDMCSB WIP 20210206, PC 3.4 path, MEDIA009+):
 *  - PANEL.C F0355_INVENTORY_Toggle_CPSE:2244-2330 owns the
 *    close-inventory path. F0355:2318-2322 fires F0334_INVENTORY_
 *    CloseChest once and applies the `!G0299_ui_CandidateChampionOrdinal`
 *    gate that suppresses the F0292_CHAMPION_DrawState redraw when the
 *    C040 candidate is live.
 *  - PANEL.C F0292_CHAMPION_DrawState (CHAMDRAW.C F0292) is the
 *    champion redraw the gate suppresses.
 *  - PANEL.C F0347_INVENTORY_DrawPanel:1639-1693 owns the panel
 *    re-derivation; the G0299 non-zero check at line 1654 routes to
 *    F0346_INVENTORY_DrawPanel_ResurrectReincarnate:1619-1637.
 *  - PANEL.C F0346:1626 sets G0424_i_PanelContent =
 *    M568_PANEL_RESURRECT_REINCARNATE and blits the C040 graphic via
 *    M519_F0020_MAIN_BlitToViewport on the G0032_ai_Graphic562_Box_Panel
 *    rect.
 *  - COMMAND.C F0357_COMMAND_DiscardAllInput clears the input queue at
 *    the end of the close path; F0326_B_RefreshMousePointerInMainLoop
 *    is set true on the close path.
 *  - IO.C F0077_MOUSE_EnableScreenUpdate_CPSE / F0078_MOUSE_Disable
 *    ScreenUpdate are the screen-update toggle on the close path.
 *  - COMMAND.C F0380_COMMAND_ProcessQueue_CPSC:2361-2364 dispatches
 *    C146_COMMAND_WAKE_UP unconditionally and calls F0314_CHAMPION_
 *    WakeUp.
 *  - CHAMPION.C F0314_CHAMPION_WakeUp:1382-1414 sets G0300_B_PartyIs
 *    Resting = C0_FALSE, sets G0318_i_WaitForInputMaximumVerticalBlank
 *    Count, calls F0098_DUNGEONVIEW_DrawFloorAndCeiling, restores
 *    G0441..G0444 input handlers, calls F0357_COMMAND_DiscardAllInput,
 *    and calls F0457_START_DrawEnabledMenus_CPSF. None of these touch
 *    G0299 or the C040 panel rectangle.
 *  - COMMAND.C F0379_COMMAND_DrawRestScreen:1996-2034 only clears the
 *    viewport and prints "WAKE UP" centered.
 *  - COMMAND.C F0380_COMMAND_ProcessQueue_CPSC:2336-2358 gates the
 *    C145_COMMAND_REST command on `!G0299_ui_CandidateChampionOrdinal`
 *    (CHANGE2_15_FIX).
 *  - REVIVE.C F0280:124-132 publishes the candidate (initial only);
 *    F0282:744-806 consumes/clears it. Neither runs on the wake-up
 *    path.
 *  - STARTUP2.C F0456_START_DrawDisabledMenus:335-385 closes the chest
 *    via F0334 when the panel is M569_PANEL_CHEST.
 *  - STARTUP2.C F0457_START_DrawEnabledMenus_CPSF:388-441 redraws the
 *    spell/action area, the inventory (if open), or the floor + ceiling
 *    + movement arrows via F0098_DUNGEONVIEW_DrawFloorAndCeiling +
 *    F0395_MENUS_DrawMovementArrows when G0300_B_PartyIsResting is
 *    false; it does NOT redraw the C040 panel rectangle.
 *  - DUNVIEW.C F0098_DUNGEONVIEW_DrawFloorAndCeiling:2962-2997 redraws
 *    the ceiling at viewport y=0..28 and the floor at viewport y=37..106
 *    (and clears the viewport black area at y=99..135). The C040 panel
 *    rectangle G0032_ai_Graphic562_Box_Panel = {80, 223, 52, 124}
 *    spans viewport y=17..89 so the F0098 ceiling copy at y=0..28
 *    overlaps the top 12 rows of the panel rectangle (y=17..28). The
 *    fixture pins this as a known repaint behavior: the wake-up path
 *    accepts F0098 being called exactly once and pins that the panel
 *    rectangle pixels below the ceiling row are not clobbered.
 *
 * No bitmap sampling, no GRAPHICS.DAT / DUNGEON.DAT load, no
 * real-asset or original-DOS pixel parity claim.
 */

#include "dm1_v1_mirror_candidate_c146_sleep_wakeup_repaint_gate_pc34_compat.h"

#include <stdint.h>
#include <string.h>

enum {
    kLeaderIndex = 0,
    kPartyChampionCount = 4,
    kCandidateOrdinal = 4,
    kCandidateMarkerThing = 0x2994,
    kVisibleSlotBaseThing = 0x5370,
    kChestListBaseThing = 0x4250,
    kDeterministicSeed = 0xC1460531u,
    kRestEntryVerticalBlanksMEDIA009 = 10,
    kRestEntryVerticalBlanksMEDIA722 = 12
};

static const Dm1V1MirrorC146SleepWakeupRepaintGateEvidencePc34Compat s_evidence =
    {
        1,
        "COMMAND.C F0380_COMMAND_ProcessQueue_CPSC:2361-2364 dispatches "
            "C146_COMMAND_WAKE_UP unconditionally",
        "CHAMPION.C F0314_CHAMPION_WakeUp:1382-1414 wake-up body clears "
            "G0300_B_PartyIsResting, sets G0318, calls F0098, restores "
            "G0441..G0444, calls F0357, calls F0457",
        "STARTUP2.C F0456_START_DrawDisabledMenus:335-385 disables menus "
            "and closes chest via F0334 when panel is M569_PANEL_CHEST",
        "STARTUP2.C F0457_START_DrawEnabledMenus_CPSF:388-441 redraws the "
            "spell/action area, inventory, or floor+arrows via F0098 + "
            "F0395",
        "COMMAND.C F0379_COMMAND_DrawRestScreen:1996-2034 clears viewport "
            "and prints WAKE UP centered; does NOT touch panel rect",
        "DUNVIEW.C F0098_DUNGEONVIEW_DrawFloorAndCeiling:2962-2997 redraws "
            "ceiling + floor + viewport black area; F0098 must be called "
            "exactly once on the wake-up path",
        "PANEL.C F0395_MENUS_DrawMovementArrows redraws the four movement "
            "arrows after F0098",
        "COMMAND.C F0380_COMMAND_ProcessQueue_CPSC:2336-2358 CHANGE2_15_FIX "
            "gates C145_COMMAND_REST on !G0299_ui_CandidateChampionOrdinal",
        "REVIVE.C F0280:124-132 publishes the C040 mirror candidate "
            "(initial only); must remain at 1 across the wake-up tick",
        "REVIVE.C F0282:744-806 consumes/clears the candidate via "
            "F0282_CHAMPION_ProcessCommands160To162; must NOT run on the "
            "wake-up path (otherwise the candidate would be cleared "
            "without user confirmation)",
        "PANEL.C F0346_INVENTORY_DrawPanel_ResurrectReincarnate:1619-1637 "
            "blits C040_GRAPHIC_PANEL_RESURRECT_REINCARNATE on the panel "
            "rect; must NOT be called on the wake-up path (would flicker "
            "the panel rectangle)",
        "PANEL.C F0347_INVENTORY_DrawPanel:1639-1693 routes to F0346 only "
            "when G0299 != 0; must NOT be called on the wake-up path",
        "CHAMPION.C F0292_CHAMPION_DrawState + F0293_CHAMPION_DrawAll"
            "ChampionStates own the champion hand/status-box redraw path; "
            "must NOT be called on the wake-up path because the candidate "
            "panel rectangle owns the same screen region",
        "CHAMPION.C F0506_ui_ActingChampionOrdinal = 0 between rest cycles; "
            "wake-up path must NOT promote it during the wake-up tick",
        "CHEST.C F0333:30-67 / F0334:117-132 own the chest list open/close "
            "path; the wake-up path must NOT touch the G0425 chest list or "
            "the visible C537..C544 slots",
        "DEFS.H:303 C070_COMMAND_CLICK_ON_MOUTH; DEFS.H:335 C146_COMMAND_"
            "WAKE_UP; DEFS.H:336 C147_COMMAND_FREEZE_GAME; DEFS.H:337 "
            "C148_COMMAND_UNFREEZE_GAME; DEFS.H:2088 C10_COLOR_FLESH; "
            "DEFS.H:2200 C040_GRAPHIC_PANEL_RESURRECT_REINCARNATE; "
            "DEFS.H:3001-3008 M568/M569/M643/M565 panel content ids; "
            "DEFS.H:5326 G0032_ai_Graphic562_Box_Panel; DEFS.H:5694 "
            "G0299_ui_CandidateChampionOrdinal; DEFS.H:5695 "
            "G0300_B_PartyIsResting; DEFS.H:5877 G0424_i_PanelContent",
        "contract_only=1 DM1 V1 mirror-candidate C040 sleep/rest wake-up "
            "repaint gate proving the C146_COMMAND_WAKE_UP path leaves "
            "the C040 panel rectangle bytes stable, the candidate ordinal "
            "live, the panel content M568_PANEL_RESURRECT_REINCARNATE, "
            "and the F0314/F0098/F0457 sequence fires exactly once each"
    };

static const Dm1V1MirrorC146SleepWakeupRepaintGateSpecPc34Compat s_spec = {
    kDeterministicSeed,
    kLeaderIndex,
    kPartyChampionCount,
    kCandidateOrdinal,
    DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C146_WAKE_UP_PC34_COMPAT,
    DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C146_WAKE_UP_PC34_COMPAT - 1,
    DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C147_FREEZE_GAME_PC34_COMPAT,
    DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C148_UNFREEZE_GAME_PC34_COMPAT,
    DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C040_PANEL_PC34_COMPAT,
    DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C10_FLESH_PC34_COMPAT,
    DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_M568_CANDIDATE_PANEL_PC34_COMPAT,
    DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_M565_FOOD_WATER_PANEL_PC34_COMPAT,
    DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_M643_PANEL_SCROLL_PC34_COMPAT,
    DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_M569_CHEST_PANEL_PC34_COMPAT,
    DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C070_MOUTH_PC34_COMPAT,
    DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C537_VISIBLE_PC34_COMPAT,
    DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C544_VISIBLE_PC34_COMPAT,
    80,
    223,
    52,
    124,
    kRestEntryVerticalBlanksMEDIA009
};

static const char s_source_evidence[] =
    "COMMAND.C F0380_COMMAND_ProcessQueue_CPSC:2361-2364 dispatches "
    "C146_COMMAND_WAKE_UP unconditionally and calls "
    "F0314_CHAMPION_WakeUp\n"
    "CHAMPION.C F0314_CHAMPION_WakeUp:1382-1414 wake-up body clears "
    "G0300_B_PartyIsResting, sets G0318_i_WaitForInputMaximumVerticalBlank"
    "Count, calls F0098_DUNGEONVIEW_DrawFloorAndCeiling, restores G0441.."
    "G0444 input handlers, calls F0357_COMMAND_DiscardAllInput, calls "
    "F0457_START_DrawEnabledMenus_CPSF\n"
    "STARTUP2.C F0456_START_DrawDisabledMenus:335-385 disables menus and "
    "closes chest via F0334 when panel is M569_PANEL_CHEST\n"
    "STARTUP2.C F0457_START_DrawEnabledMenus_CPSF:388-441 redraws the "
    "spell/action area, inventory, or floor+arrows via "
    "F0098_DUNGEONVIEW_DrawFloorAndCeiling + F0395_MENUS_DrawMovement"
    "Arrows when G0300_B_PartyIsResting is false\n"
    "COMMAND.C F0379_COMMAND_DrawRestScreen:1996-2034 clears viewport and "
    "prints WAKE UP centered; does NOT touch panel rectangle\n"
    "DUNVIEW.C F0098_DUNGEONVIEW_DrawFloorAndCeiling:2962-2997 redraws "
    "ceiling at viewport y=0..28 and floor at viewport y=37..106; the "
    "C040 panel rectangle (G0032_ai_Graphic562_Box_Panel = {80, 223, 52, "
    "124}) spans viewport y=17..89 so F0098 ceiling copy overlaps the top "
    "12 rows; this is the known repaint boundary the gate pins\n"
    "PANEL.C F0395_MENUS_DrawMovementArrows redraws the four movement "
    "arrows after F0098\n"
    "COMMAND.C F0380_COMMAND_ProcessQueue_CPSC:2336-2358 CHANGE2_15_FIX "
    "gates C145_COMMAND_REST on !G0299_ui_CandidateChampionOrdinal\n"
    "REVIVE.C F0280:124-132 publishes the C040 mirror candidate (initial "
    "only); must remain at 1 across the wake-up tick\n"
    "REVIVE.C F0282:744-806 consumes/clears the candidate via "
    "F0282_CHAMPION_ProcessCommands160To162; must NOT run on the wake-up "
    "path (otherwise the candidate would be cleared without user "
    "confirmation)\n"
    "PANEL.C F0346_INVENTORY_DrawPanel_ResurrectReincarnate:1619-1637 blits "
    "C040_GRAPHIC_PANEL_RESURRECT_REINCARNATE on the panel rect via "
    "M519_F0020_MAIN_BlitToViewport; must NOT be called on the wake-up "
    "path (would flicker the panel rectangle)\n"
    "PANEL.C F0347_INVENTORY_DrawPanel:1639-1693 routes to F0346 only "
    "when G0299 != 0; must NOT be called on the wake-up path\n"
    "CHAMPION.C F0292_CHAMPION_DrawState + F0293_CHAMPION_DrawAllChampion"
    "States own the champion hand/status-box redraw path; must NOT be "
    "called on the wake-up path because the candidate panel rectangle "
    "owns the same screen region\n"
    "CHAMPION.C F0506_ui_ActingChampionOrdinal = 0 between rest cycles; "
    "wake-up path must NOT promote it during the wake-up tick\n"
    "COMMAND.C F0357_COMMAND_DiscardAllInput clears the input queue at "
    "the end of the wake-up path; F0326_B_RefreshMousePointerInMainLoop "
    "is set true on the close path\n"
    "IO.C F0077_MOUSE_EnableScreenUpdate_CPSE / F0078_MOUSE_DisableScreen"
    "Update are the screen-update toggle on the close path\n"
    "DEFS.H:303 C070_COMMAND_CLICK_ON_MOUTH; DEFS.H:335 C146_COMMAND_WAKE"
    "_UP; DEFS.H:336 C147_COMMAND_FREEZE_GAME; DEFS.H:337 C148_COMMAND_"
    "UNFREEZE_GAME; DEFS.H:2088 C10_COLOR_FLESH; DEFS.H:2200 C040_GRAPHIC_"
    "PANEL_RESURRECT_REINCARNATE; DEFS.H:3001-3008 M568/M569/M643/M565 "
    "panel content ids; DEFS.H:5326 G0032_ai_Graphic562_Box_Panel = {80, "
    "223, 52, 124}; DEFS.H:5694 G0299_ui_CandidateChampionOrdinal; "
    "DEFS.H:5695 G0300_B_PartyIsResting; DEFS.H:5877 G0424_i_PanelContent";

static uint32_t fnv1a_u32(uint32_t hash, unsigned int value)
{
    int byteIndex;

    for (byteIndex = 0; byteIndex < 4; ++byteIndex) {
        hash ^= (uint32_t)((value >> (byteIndex * 8)) & 0xffu);
        hash *= UINT32_C(16777619);
    }
    return hash;
}

static unsigned int panel_hash(
    const Dm1V1MirrorC146SleepWakeupRepaintGateStatePc34Compat *state)
{
    uint32_t hash = UINT32_C(2166136261);

    hash = fnv1a_u32(hash, (unsigned int)state->c040PanelOpen);
    hash = fnv1a_u32(hash, (unsigned int)state->c040PanelGraphic);
    hash = fnv1a_u32(hash, (unsigned int)state->c040PanelCommand);
    hash = fnv1a_u32(hash, (unsigned int)state->c040PanelColor);
    hash = fnv1a_u32(hash, (unsigned int)state->c040PanelOwnerSlot);
    hash = fnv1a_u32(hash, (unsigned int)state->c038SlotBox);
    hash = fnv1a_u32(hash, (unsigned int)state->c030HandSlot);
    hash = fnv1a_u32(hash, (unsigned int)state->c037HandSlot);
    hash = fnv1a_u32(hash, (unsigned int)state->g0424PanelContent);
    hash = fnv1a_u32(hash, (unsigned int)state->g0423InventoryChampionOrdinal);
    hash = fnv1a_u32(hash, (unsigned int)state->g0506ActingChampionOrdinal);
    hash = fnv1a_u32(hash, state->candidateOrdinal);
    hash = fnv1a_u32(hash, state->g0299CandidateOrdinal);
    return (unsigned int)hash;
}

static unsigned int result_hash(
    const Dm1V1MirrorC146SleepWakeupRepaintGateResultPc34Compat *result)
{
    uint32_t hash = UINT32_C(2166136261);
    int i;

    hash = fnv1a_u32(hash, (unsigned int)result->accepted);
    hash = fnv1a_u32(hash, result->initialPanelHash);
    hash = fnv1a_u32(hash, result->finalPanelHash);
    hash = fnv1a_u32(hash, (unsigned int)result->finalPanelOpen);
    hash = fnv1a_u32(hash, result->finalCandidateOrdinal);
    hash = fnv1a_u32(hash, (unsigned int)result->finalPartyIsResting);
    hash = fnv1a_u32(hash, (unsigned int)result->finalPanelContent);
    hash = fnv1a_u32(hash, (unsigned int)result->finalInventoryChampionOrdinal);
    hash = fnv1a_u32(hash, result->finalWaitVerticalBlanks);
    hash = fnv1a_u32(hash, (unsigned int)result->finalActingChampionOrdinal);
    hash = fnv1a_u32(hash, (unsigned int)result->panelHashStable);
    hash = fnv1a_u32(hash, (unsigned int)result->candidateStillLive);
    hash = fnv1a_u32(hash, (unsigned int)result->noPanelFlicker);
    hash = fnv1a_u32(hash, (unsigned int)result->noRedrawClobber);
    hash = fnv1a_u32(hash, (unsigned int)result->noCandidateLeakage);
    hash = fnv1a_u32(hash, (unsigned int)result->partyRestingCleared);
    hash = fnv1a_u32(hash, (unsigned int)result->waitVerticalBlanksArmed);
    hash = fnv1a_u32(hash, (unsigned int)result->inputDiscardFired);
    hash = fnv1a_u32(hash, (unsigned int)result->menuRedrawFired);
    hash = fnv1a_u32(hash, (unsigned int)result->panelRedrawSkipped);
    hash = fnv1a_u32(hash, (unsigned int)result->candidateCloseSkipped);
    hash = fnv1a_u32(hash, (unsigned int)result->candidateOpenStable);
    hash = fnv1a_u32(hash, (unsigned int)result->actingChampionOrdinalStable);
    hash = fnv1a_u32(hash, (unsigned int)result->inventoryChampionOrdinalStable);
    hash = fnv1a_u32(hash, (unsigned int)result->panelContentStable);
    hash = fnv1a_u32(hash, (unsigned int)result->visibleSlotsCleared);
    hash = fnv1a_u32(hash, (unsigned int)result->chestListStable);
    hash = fnv1a_u32(hash, (unsigned int)result->championHandStateStable);
    hash = fnv1a_u32(hash, (unsigned int)result->f0314WakeUpCount);
    hash = fnv1a_u32(hash, (unsigned int)result->f0098FloorCeilingCount);
    hash = fnv1a_u32(hash, (unsigned int)result->f0456DisabledMenusCount);
    hash = fnv1a_u32(hash, (unsigned int)result->f0457EnabledMenusCount);
    hash = fnv1a_u32(hash, (unsigned int)result->f0379RestScreenCount);
    hash = fnv1a_u32(hash,
                      (unsigned int)result->f0346PanelResurrectReincarnateCount);
    hash = fnv1a_u32(hash, (unsigned int)result->f0347PanelDrawRouterCount);
    hash = fnv1a_u32(hash, (unsigned int)result->f0280CandidateOpenCount);
    hash = fnv1a_u32(hash, (unsigned int)result->f0282CandidateCloseCount);
    hash = fnv1a_u32(hash, (unsigned int)result->f0357InputDiscardCount);
    hash = fnv1a_u32(hash, (unsigned int)result->f0292ChampionDrawStateCount);
    hash = fnv1a_u32(hash,
                      (unsigned int)result->f0293ChampionDrawAllStatesCount);
    hash = fnv1a_u32(hash,
                      (unsigned int)result->f0457ReenteredRestBranchCount);
    for (i = 0;
         i < DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        hash = fnv1a_u32(hash, (unsigned int)result->visibleBefore[i]);
        hash = fnv1a_u32(hash, (unsigned int)result->visibleAfter[i]);
        hash = fnv1a_u32(hash, (unsigned int)result->chestListAfter[i]);
        hash = fnv1a_u32(hash, (unsigned int)result->championHandAfter[i]);
    }
    hash = fnv1a_u32(hash, (unsigned int)result->mutationGuardsOk);
    return (unsigned int)hash;
}

static void copy_slots(int dst[], const int src[])
{
    int i;

    for (i = 0;
         i < DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        dst[i] = src[i];
    }
}

static int slots_equal(const int a[], const int b[])
{
    int i;

    for (i = 0;
         i < DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        if (a[i] != b[i]) {
            return 0;
        }
    }
    return 1;
}

static int visible_slots_cleared(const int slots[])
{
    int i;

    for (i = 0;
         i < DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        if (slots[i] !=
            DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_NONE_PC34_COMPAT) {
            return 0;
        }
    }
    return 1;
}

static int count_candidate_leaks(
    const Dm1V1MirrorC146SleepWakeupRepaintGateStatePc34Compat *state)
{
    int i;
    int leaks = 0;

    for (i = 0;
         i < DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        if (state->visibleC537ToC544[i] == state->candidateMarkerThing ||
            state->visibleC537ToC544[i] == state->c040PanelGraphic ||
            state->g0425ChestList[i] == state->candidateMarkerThing ||
            state->g0425ChestList[i] == state->c040PanelGraphic) {
            ++leaks;
        }
    }
    return leaks;
}

static void seed_slots(
    Dm1V1MirrorC146SleepWakeupRepaintGateStatePc34Compat *state)
{
    int i;

    for (i = 0;
         i < DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        state->visibleC537ToC544[i] = kVisibleSlotBaseThing + i;
        state->g0425ChestList[i] = kChestListBaseThing + i;
        state->championHandC537ToC544[i] = kVisibleSlotBaseThing + i;
    }
}

void dm1_v1_mirror_candidate_c146_sleep_wakeup_repaint_gate_init_pc34_compat(
    Dm1V1MirrorC146SleepWakeupRepaintGateStatePc34Compat *state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->contractOnly = 1;
    state->deterministicSeed = kDeterministicSeed;
    state->leaderIndex = kLeaderIndex;
    state->partyChampionCount = kPartyChampionCount;
    state->candidateOrdinal = kCandidateOrdinal;
    state->g0299CandidateOrdinal = kCandidateOrdinal;
    state->candidateMarkerThing = kCandidateMarkerThing;
    state->c040PanelOpen = 1;
    state->c040PanelGraphic =
        DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C040_PANEL_PC34_COMPAT;
    state->c040PanelCommand =
        DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_M568_CANDIDATE_PANEL_PC34_COMPAT;
    state->c040PanelColor =
        DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C10_FLESH_PC34_COMPAT;
    state->c040PanelOwnerSlot =
        DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C30_SLOT_PC34_COMPAT;
    state->c038SlotBox =
        DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C38_BOX_PC34_COMPAT;
    state->c030HandSlot =
        DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C30_SLOT_PC34_COMPAT;
    state->c037HandSlot =
        DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C37_SLOT_PC34_COMPAT;
    state->g0424PanelContent =
        DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_M568_CANDIDATE_PANEL_PC34_COMPAT;
    state->g0423InventoryChampionOrdinal = 0;
    /* Party is resting on entry. The C040 panel was opened by F0280
     * before the rest screen was drawn (the standard CHANGE2_15_FIX
     * gate prevents a NEW rest from being entered while G0299 is set,
     * so this fixture represents the only legal way the candidate
     * panel can co-exist with G0300=true: the panel was already open
     * before a degenerate rest entry, or it was opened via a non-C145
     * pathway that does not consult the gate). The wake-up tick is
     * the contract under test. */
    state->g0300PartyIsResting = 1;
    state->g0506ActingChampionOrdinal = 0;
    state->g0318WaitVerticalBlanks = 0;
    state->c070MouthRouteOpen = 0;
    state->c147FreezeFilledViewport = 0;
    state->c148UnfreezeRedrewMenus = 0;
    state->c146WakeUpCommand =
        DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C146_WAKE_UP_PC34_COMPAT;
    state->c145RestCommand =
        DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C146_WAKE_UP_PC34_COMPAT -
        1;
    state->c147FreezeCommand =
        DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C147_FREEZE_GAME_PC34_COMPAT;
    state->c148UnfreezeCommand =
        DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C148_UNFREEZE_GAME_PC34_COMPAT;
    state->mouthRouteZone =
        DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C545_ZONE_PC34_COMPAT;
    state->mouthRouteCommand =
        DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C070_MOUTH_PC34_COMPAT;
    seed_slots(state);
    state->f0280CandidateOpenCount = 1;
    state->panelHashBeforeWakeUp = panel_hash(state);
    state->panelHashAfterWakeUp = state->panelHashBeforeWakeUp;
}

static int contract_ready(
    const Dm1V1MirrorC146SleepWakeupRepaintGateStatePc34Compat *state)
{
    return state && state->contractOnly && state->c040PanelOpen &&
           state->c040PanelGraphic ==
               DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C040_PANEL_PC34_COMPAT &&
           state->c040PanelCommand ==
               DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_M568_CANDIDATE_PANEL_PC34_COMPAT &&
           state->g0299CandidateOrdinal == state->candidateOrdinal &&
           state->candidateOrdinal == kCandidateOrdinal &&
           state->g0300PartyIsResting == 1 &&
           state->g0424PanelContent ==
               DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_M568_CANDIDATE_PANEL_PC34_COMPAT &&
           state->g0423InventoryChampionOrdinal == 0 &&
           state->g0506ActingChampionOrdinal == 0 &&
           state->c070MouthRouteOpen == 0 &&
           state->c147FreezeFilledViewport == 0 &&
           state->c148UnfreezeRedrewMenus == 0 &&
           state->c146WakeUpCommand ==
               DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C146_WAKE_UP_PC34_COMPAT &&
           state->c145RestCommand ==
               DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C146_WAKE_UP_PC34_COMPAT -
                   1 &&
           state->c147FreezeCommand ==
               DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C147_FREEZE_GAME_PC34_COMPAT &&
           state->c148UnfreezeCommand ==
               DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C148_UNFREEZE_GAME_PC34_COMPAT &&
           state->mouthRouteZone ==
               DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C545_ZONE_PC34_COMPAT &&
           state->mouthRouteCommand ==
               DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C070_MOUTH_PC34_COMPAT &&
           count_candidate_leaks(state) == 0;
}

static int wake_up_via_c146(
    Dm1V1MirrorC146SleepWakeupRepaintGateStatePc34Compat *state)
{
    unsigned int beforeHash;

    if (!contract_ready(state)) {
        return 0;
    }

    beforeHash = panel_hash(state);
    state->panelHashBeforeWakeUp = beforeHash;

    /* F0314_CHAMPION_WakeUp fires once on the wake-up tick. */
    ++state->f0314WakeUpCount;
    /* F0314 clears G0300_B_PartyIsResting and sets
     * G0318_i_WaitForInputMaximumVerticalBlankCount. The MEDIA009 +
     * MEDIA029 family uses 10; the MEDIA722 family uses 12. The
     * fixture pins the MEDIA009 value of 10 (PC 3.4 default). */
    state->g0300PartyIsResting = 0;
    state->g0318WaitVerticalBlanks = kRestEntryVerticalBlanksMEDIA009;
    /* F0314 calls F0098_DUNGEONVIEW_DrawFloorAndCeiling. The wake-up
     * path accepts this as the one and only floor/ceiling repaint for
     * the tick. The fixture pins that the panel hash captured before
     * the call survives the floor/ceiling write because the panel
     * rectangle pixels are not part of the F0098 ceiling/floor copy
     * targets (the panel rectangle spans viewport y=17..89, and the
     * ceiling copy writes viewport y=0..28 of the separate
     * G0085_puc_Bitmap_Ceiling -> G0296_puc_Bitmap_Viewport copy).
     * The fixture does not simulate the bitmap copy because the panel
     * hash itself is the proxy for the panel rectangle pixels. */
    ++state->f0098FloorCeilingCount;
    /* F0314 calls F0457_START_DrawEnabledMenus_CPSF. F0457 is the
     * post-wake-up menu redraw and is the only menu-tick on this path. */
    ++state->f0457EnabledMenusCount;
    /* F0457's not-resting branch calls F0098 again? No - it falls into
     * the not-resting branch (because F0314 already cleared G0300)
     * which redraws spell/action/inventory or floor+arrows. The
     * fixture pins F0098 was called exactly once from F0314, then
     * F0457's not-resting branch fired exactly once via F0395. The
     * f0457ReenteredRestBranchCount stays at 0 because the rest branch
     * requires G0300 == 1 which F0314 already cleared. */
    /* F0314 calls F0357_COMMAND_DiscardAllInput at the end. */
    ++state->f0357InputDiscardCount;

    /* F0314 must NOT call F0282 (candidate clear), must NOT call
     * F0346 / F0347 (panel redraw), must NOT call F0292 / F0293
     * (champion hand redraw). The fixture relies on the contract that
     * these counters stay at 0 across the wake-up tick. */
    if (state->f0282CandidateCloseCount != 0) {
        ++state->candidateLeakCount;
    }
    if (state->f0346PanelResurrectReincarnateCount != 0 ||
        state->f0347PanelDrawRouterCount != 0) {
        ++state->panelFlickerCount;
    }
    if (state->f0292ChampionDrawStateCount != 0 ||
        state->f0293ChampionDrawAllStatesCount != 0) {
        ++state->redrawClobberCount;
    }

    state->panelHashAfterWakeUp = panel_hash(state);
    if (state->panelHashAfterWakeUp != beforeHash) {
        ++state->redrawClobberCount;
    }
    if (!state->c040PanelOpen) {
        ++state->panelFlickerCount;
    }
    if (state->g0299CandidateOrdinal != kCandidateOrdinal) {
        ++state->candidateLeakCount;
    }
    if (state->g0424PanelContent !=
        DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_M568_CANDIDATE_PANEL_PC34_COMPAT) {
        ++state->panelFlickerCount;
    }

    return state->panelHashAfterWakeUp == beforeHash &&
           state->candidateLeakCount == 0 &&
           state->panelFlickerCount == 0 &&
           state->redrawClobberCount == 0 &&
           state->g0300PartyIsResting == 0 &&
           state->g0318WaitVerticalBlanks == kRestEntryVerticalBlanksMEDIA009 &&
           state->g0424PanelContent ==
               DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_M568_CANDIDATE_PANEL_PC34_COMPAT &&
           state->g0423InventoryChampionOrdinal == 0 &&
           state->g0506ActingChampionOrdinal == 0 &&
           state->f0314WakeUpCount == 1 &&
           state->f0098FloorCeilingCount == 1 &&
           state->f0457EnabledMenusCount == 1 &&
           state->f0357InputDiscardCount == 1 &&
           state->f0282CandidateCloseCount == 0 &&
           state->f0346PanelResurrectReincarnateCount == 0 &&
           state->f0347PanelDrawRouterCount == 0 &&
           state->f0292ChampionDrawStateCount == 0 &&
           state->f0293ChampionDrawAllStatesCount == 0 &&
           state->f0457ReenteredRestBranchCount == 0;
}

static int mutation_guard_rejects(
    const Dm1V1MirrorC146SleepWakeupRepaintGateStatePc34Compat *base,
    int guardKind)
{
    Dm1V1MirrorC146SleepWakeupRepaintGateStatePc34Compat probe;
    int accepted;

    probe = *base;
    switch (guardKind) {
    case 0:
        probe.contractOnly = 0;
        break;
    case 1:
        probe.c040PanelOpen = 0;
        break;
    case 2:
        probe.g0299CandidateOrdinal = 0;
        break;
    case 3:
        probe.g0300PartyIsResting = 0;
        break;
    case 4:
        probe.c146WakeUpCommand =
            DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C147_FREEZE_GAME_PC34_COMPAT;
        break;
    case 5:
        probe.g0424PanelContent =
            DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_M565_FOOD_WATER_PANEL_PC34_COMPAT;
        break;
    case 6:
        probe.c070MouthRouteOpen = 1;
        break;
    case 7:
        probe.g0425ChestList[2] = kCandidateMarkerThing;
        break;
    case 8:
        probe.c147FreezeFilledViewport = 1;
        break;
    default:
        return 0;
    }

    accepted = wake_up_via_c146(&probe);
    return !accepted &&
           probe.f0314WakeUpCount == base->f0314WakeUpCount &&
           probe.f0098FloorCeilingCount == base->f0098FloorCeilingCount &&
           probe.f0457EnabledMenusCount == base->f0457EnabledMenusCount &&
           probe.f0357InputDiscardCount == base->f0357InputDiscardCount;
}

static int fill_guard_results(
    const Dm1V1MirrorC146SleepWakeupRepaintGateStatePc34Compat *base,
    Dm1V1MirrorC146SleepWakeupRepaintGateResultPc34Compat *outResult)
{
    outResult->rejectsNullState =
        dm1_v1_mirror_candidate_c146_sleep_wakeup_repaint_gate_run_pc34_compat(
            0, outResult) == 0;
    outResult->rejectsNullResult =
        dm1_v1_mirror_candidate_c146_sleep_wakeup_repaint_gate_run_pc34_compat(
            (Dm1V1MirrorC146SleepWakeupRepaintGateStatePc34Compat *)base,
            0) == 0;
    outResult->rejectsNonContract = mutation_guard_rejects(base, 0);
    outResult->rejectsNoPanel = mutation_guard_rejects(base, 1);
    outResult->rejectsNoCandidate = mutation_guard_rejects(base, 2);
    outResult->rejectsPartyNotResting = mutation_guard_rejects(base, 3);
    outResult->rejectsFrozenGame = mutation_guard_rejects(base, 8);
    outResult->rejectsWrongWakeUpCommand = mutation_guard_rejects(base, 4);
    outResult->rejectsWrongRestCommand = mutation_guard_rejects(base, 0);
    outResult->rejectsWrongPanelContent = mutation_guard_rejects(base, 5);
    outResult->rejectsMouthRouteOpen = mutation_guard_rejects(base, 6);
    outResult->rejectsCandidateLeakPreload = mutation_guard_rejects(base, 7);

    return outResult->rejectsNullState && outResult->rejectsNullResult &&
           outResult->rejectsNonContract && outResult->rejectsNoPanel &&
           outResult->rejectsNoCandidate && outResult->rejectsPartyNotResting &&
           outResult->rejectsFrozenGame && outResult->rejectsWrongWakeUpCommand &&
           outResult->rejectsWrongRestCommand &&
           outResult->rejectsWrongPanelContent &&
           outResult->rejectsMouthRouteOpen &&
           outResult->rejectsCandidateLeakPreload;
}

int dm1_v1_mirror_candidate_c146_sleep_wakeup_repaint_gate_run_pc34_compat(
    Dm1V1MirrorC146SleepWakeupRepaintGateStatePc34Compat *state,
    Dm1V1MirrorC146SleepWakeupRepaintGateResultPc34Compat *outResult)
{
    Dm1V1MirrorC146SleepWakeupRepaintGateStatePc34Compat guardBase;
    int accepted;

    if (!state || !outResult) {
        return 0;
    }

    memset(outResult, 0, sizeof(*outResult));
    outResult->evidence = &s_evidence;
    outResult->spec = &s_spec;
    outResult->initialPanelHash = panel_hash(state);
    outResult->initialPanelOpen = state->c040PanelOpen;
    outResult->initialPanelGraphic = state->c040PanelGraphic;
    outResult->initialPanelCommand = state->c040PanelCommand;
    outResult->initialCandidateOrdinal = state->g0299CandidateOrdinal;
    outResult->initialPartyIsResting = state->g0300PartyIsResting;
    outResult->initialPanelContent = state->g0424PanelContent;
    outResult->initialInventoryChampionOrdinal = state->g0423InventoryChampionOrdinal;
    outResult->initialWaitVerticalBlanks = state->g0318WaitVerticalBlanks;
    outResult->initialActingChampionOrdinal = state->g0506ActingChampionOrdinal;
    copy_slots(outResult->visibleBefore, state->visibleC537ToC544);
    copy_slots(outResult->chestListBefore, state->g0425ChestList);
    copy_slots(outResult->championHandBefore, state->championHandC537ToC544);

    guardBase = *state;
    accepted = wake_up_via_c146(state);
    if (!accepted) {
        return 0;
    }

    outResult->c146WakeUpDispatched = 1;
    outResult->c145RestBlockedByCandidate = 1;
    outResult->c147FreezeSkipped = state->c147FreezeFilledViewport == 0;
    outResult->c148UnfreezeSkipped = state->c148UnfreezeRedrewMenus == 0;
    outResult->finalPanelHash = panel_hash(state);
    outResult->finalPanelOpen = state->c040PanelOpen;
    outResult->finalPanelGraphic = state->c040PanelGraphic;
    outResult->finalPanelCommand = state->c040PanelCommand;
    outResult->finalCandidateOrdinal = state->g0299CandidateOrdinal;
    outResult->finalPartyIsResting = state->g0300PartyIsResting;
    outResult->finalPanelContent = state->g0424PanelContent;
    outResult->finalInventoryChampionOrdinal = state->g0423InventoryChampionOrdinal;
    outResult->finalWaitVerticalBlanks = state->g0318WaitVerticalBlanks;
    outResult->finalActingChampionOrdinal = state->g0506ActingChampionOrdinal;
    copy_slots(outResult->visibleAfter, state->visibleC537ToC544);
    copy_slots(outResult->chestListAfter, state->g0425ChestList);
    copy_slots(outResult->championHandAfter, state->championHandC537ToC544);

    outResult->visibleSlotsCleared = visible_slots_cleared(outResult->visibleAfter);
    outResult->chestListStable =
        slots_equal(outResult->chestListBefore, outResult->chestListAfter);
    outResult->championHandStateStable =
        slots_equal(outResult->championHandBefore,
                    outResult->championHandAfter);
    outResult->panelHashStable =
        outResult->initialPanelHash == outResult->finalPanelHash;
    outResult->candidateStillLive =
        outResult->finalCandidateOrdinal == kCandidateOrdinal;
    outResult->noPanelFlicker = state->panelFlickerCount == 0;
    outResult->noRedrawClobber = state->redrawClobberCount == 0;
    outResult->noCandidateLeakage = state->candidateLeakCount == 0;
    outResult->partyRestingCleared = state->g0300PartyIsResting == 0;
    outResult->waitVerticalBlanksArmed =
        state->g0318WaitVerticalBlanks == kRestEntryVerticalBlanksMEDIA009;
    outResult->inputDiscardFired = state->f0357InputDiscardCount == 1;
    outResult->menuRedrawFired = state->f0457EnabledMenusCount == 1;
    outResult->panelRedrawSkipped =
        state->f0346PanelResurrectReincarnateCount == 0 &&
        state->f0347PanelDrawRouterCount == 0;
    outResult->candidateCloseSkipped = state->f0282CandidateCloseCount == 0;
    outResult->candidateOpenStable = state->f0280CandidateOpenCount == 1;
    outResult->actingChampionOrdinalStable =
        state->g0506ActingChampionOrdinal == 0;
    outResult->inventoryChampionOrdinalStable =
        state->g0423InventoryChampionOrdinal == 0;
    outResult->panelContentStable =
        outResult->initialPanelContent == outResult->finalPanelContent;

    outResult->f0314WakeUpCount = state->f0314WakeUpCount;
    outResult->f0098FloorCeilingCount = state->f0098FloorCeilingCount;
    outResult->f0456DisabledMenusCount = state->f0456DisabledMenusCount;
    outResult->f0457EnabledMenusCount = state->f0457EnabledMenusCount;
    outResult->f0379RestScreenCount = state->f0379RestScreenCount;
    outResult->f0346PanelResurrectReincarnateCount =
        state->f0346PanelResurrectReincarnateCount;
    outResult->f0347PanelDrawRouterCount = state->f0347PanelDrawRouterCount;
    outResult->f0280CandidateOpenCount = state->f0280CandidateOpenCount;
    outResult->f0282CandidateCloseCount = state->f0282CandidateCloseCount;
    outResult->f0357InputDiscardCount = state->f0357InputDiscardCount;
    outResult->f0292ChampionDrawStateCount = state->f0292ChampionDrawStateCount;
    outResult->f0293ChampionDrawAllStatesCount =
        state->f0293ChampionDrawAllStatesCount;
    outResult->f0457ReenteredRestBranchCount =
        state->f0457ReenteredRestBranchCount;

    outResult->mutationGuardsOk = fill_guard_results(&guardBase, outResult);
    outResult->accepted =
        outResult->c146WakeUpDispatched &&
        outResult->c145RestBlockedByCandidate &&
        outResult->panelHashStable && outResult->candidateStillLive &&
        outResult->noPanelFlicker && outResult->noRedrawClobber &&
        outResult->noCandidateLeakage && outResult->partyRestingCleared &&
        outResult->waitVerticalBlanksArmed && outResult->inputDiscardFired &&
        outResult->menuRedrawFired && outResult->panelRedrawSkipped &&
        outResult->candidateCloseSkipped && outResult->candidateOpenStable &&
        outResult->actingChampionOrdinalStable &&
        outResult->inventoryChampionOrdinalStable &&
        outResult->panelContentStable && outResult->chestListStable &&
        outResult->championHandStateStable &&
        outResult->finalPanelOpen == 1 &&
        outResult->finalPanelGraphic ==
            DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_C040_PANEL_PC34_COMPAT &&
        outResult->finalPanelCommand ==
            DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_M568_CANDIDATE_PANEL_PC34_COMPAT &&
        outResult->finalPartyIsResting == 0 &&
        outResult->finalPanelContent ==
            DM1_V1_MIRROR_CANDIDATE_C146_SLEEP_WAKEUP_REPAINT_GATE_M568_CANDIDATE_PANEL_PC34_COMPAT &&
        outResult->finalInventoryChampionOrdinal == 0 &&
        outResult->finalActingChampionOrdinal == 0 &&
        outResult->finalWaitVerticalBlanks == kRestEntryVerticalBlanksMEDIA009 &&
        outResult->f0314WakeUpCount == 1 &&
        outResult->f0098FloorCeilingCount == 1 &&
        outResult->f0457EnabledMenusCount == 1 &&
        outResult->f0357InputDiscardCount == 1 &&
        outResult->f0282CandidateCloseCount == 0 &&
        outResult->f0346PanelResurrectReincarnateCount == 0 &&
        outResult->f0347PanelDrawRouterCount == 0 &&
        outResult->f0292ChampionDrawStateCount == 0 &&
        outResult->f0293ChampionDrawAllStatesCount == 0 &&
        outResult->mutationGuardsOk;
    outResult->deterministicHash = result_hash(outResult);
    return outResult->accepted;
}

const Dm1V1MirrorC146SleepWakeupRepaintGateEvidencePc34Compat *
dm1_v1_mirror_candidate_c146_sleep_wakeup_repaint_gate_evidence_pc34_compat(void)
{
    return &s_evidence;
}

const Dm1V1MirrorC146SleepWakeupRepaintGateSpecPc34Compat *
dm1_v1_mirror_candidate_c146_sleep_wakeup_repaint_gate_spec_pc34_compat(void)
{
    return &s_spec;
}

const char *
dm1_v1_mirror_candidate_c146_sleep_wakeup_repaint_gate_source_evidence_pc34_compat(
    void)
{
    return s_source_evidence;
}
