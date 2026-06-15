#include "dm1_v1_mirror_candidate_c159_click_rotation_combo_pc34_compat.h"

#include <string.h>

/* Source-lock anchors for this contract_only=1 click combo:
 * COMMAND.C:484-488 maps C159..C162 champion-name rows to C016..C019.
 * COMMAND.C F0359:1985-1990 dispatches only the M568/C040 panel's
 * C160/C161/C162 route while the resurrect/reincarnate panel is open.
 * COMMAND.C F0380:2159-2181,2302-2311,2366-2370 gates status/name,
 * inventory, spell/action, and save dispatch on !G0299.
 * MOVESENS.C:1501-1503 calls REVIVE.C F0280, whose lines 124-132 guard
 * publish and 272-276 publish G0299. REVIVE.C F0282:744-758 and 785-806
 * clear G0299 for cancel and accept panel commands.
 * CHAMPION.C F0297/F0298/F0302:243-285,662-706 are the leader-hand and
 * slot routes this C159-with-G0299 contract must not enter.
 * PANEL.C F0354:2208-2240 draws C175+champion portrait boxes, while
 * DEFS.H:338-340 and 3787-3790 define the C160..C162 panel commands and
 * C159..C162 champion-name zones. In this ReDMCSB checkout the panel zone
 * constants used by COMMAND.C:508-511 are M664/M665/M666 at DEFS.H:
 * 3941-3943 and 3980-3982; DEFS.H:4041-4042 are viewport wall zones.
 */

enum {
    kInitialPartyChampionCount = 2,
    kInitialInventoryOrdinal = 1,
    kInitialLeaderIndex = 0,
    kInitialLeaderHandThing = 0x5a5,
    kCandidateOrdinal = 3
};

static const Dm1V1MirrorCandidateC159ClickRotationComboEvidencePc34Compat
    s_evidence = {
        1,
        "COMMAND.C:484-488 G0455 maps C159/C160/C161/C162 champion-name "
        "rows to C016/C017/C018/C019 set-leader commands",
        "COMMAND.C F0359:1985-1990 M568/C040 dispatch to F0282 "
        "C160/C161/C162 panel commands",
        "COMMAND.C F0380:2159-2181 !G0299 guard for C012/C007 status-box "
        "and inventory-toggle processing",
        "COMMAND.C F0380:2302-2311 !G0299 guard for C100 spell and C111 "
        "action-area processing",
        "COMMAND.C F0380:2366-2370 !G0299 guard for C140 save input",
        "REVIVE.C F0280:124-132 leader-empty and party-size publish gate",
        "MOVESENS.C:1501-1503 portrait sensor calls REVIVE.C F0280; "
        "REVIVE.C F0280:272-276 publishes G0299 candidate ordinal",
        "REVIVE.C F0282:744-758 clears G0299 on C162 cancel; "
        "REVIVE.C F0282:785-806 clears G0299 on C160/C161 accept path",
        "MOVESENS.C:1501-1503 C127 champion portrait sensor publish route",
        "CHAMPION.C F0297/F0298/F0302:243-285,662-706 leader-hand put, "
        "leader-hand remove, and slot-box routes not entered by this combo",
        "PANEL.C F0354:2208-2240 C175+champion portrait box; "
        "COMMAND.C:484-488 C159..C162 champion-name row zones",
        "DEFS.H:338-340 C160/C161/C162 panel command constants",
        "DEFS.H:3787-3790 C159/C160/C161/C162 champion-name zones",
        "COMMAND.C:508-511 uses M664/M665/M666 panel zones; local "
        "DEFS.H:3941-3943 and 3980-3982 define them, while DEFS.H:4041-4042 "
        "are viewport wall zones in this checkout",
        "contract_only=1 deterministic C159 row-click plus C160/C161/C162 "
        "panel-command combo; no real assets, dungeon sensors, save files, "
        "or full UI hit-testing parity are claimed"
    };

static int is_valid_panel_command(int command)
{
    return command ==
               DM1_V1_MIRROR_CANDIDATE_C159_CLICK_ROTATION_COMBO_C160_RESURRECT_PC34_COMPAT ||
           command ==
               DM1_V1_MIRROR_CANDIDATE_C159_CLICK_ROTATION_COMBO_C161_REINCARNATE_PC34_COMPAT ||
           command ==
               DM1_V1_MIRROR_CANDIDATE_C159_CLICK_ROTATION_COMBO_C162_CANCEL_PC34_COMPAT;
}

static void capture_before(
    const Dm1V1MirrorCandidateC159ClickRotationComboStatePc34Compat *state,
    int panelCommand,
    Dm1V1MirrorCandidateC159ClickRotationComboResultPc34Compat *result)
{
    memset(result, 0, sizeof(*result));
    result->evidence = &s_evidence;
    result->panelCommand = panelCommand;
    result->panelCommandValid = is_valid_panel_command(panelCommand);
    result->partyChampionCountBefore = state->partyChampionCount;
    result->partyChampionCountAfterC159 = state->partyChampionCount;
    result->partyChampionCountAfterPanel = state->partyChampionCount;
    result->candidateOrdinalBefore = state->candidateChampionOrdinal;
    result->candidateOrdinalAfterC159 = state->candidateChampionOrdinal;
    result->candidateOrdinalAfterPanel = state->candidateChampionOrdinal;
    result->leaderIndexBefore = state->leaderIndex;
    result->leaderIndexAfter = state->leaderIndex;
    result->leaderHandThingBefore = state->leaderHandThingOrdinal;
    result->leaderHandThingAfter = state->leaderHandThingOrdinal;
    result->c040PanelOpenBefore = state->c040PanelOpen;
    result->c040PanelOpenAfterC159 = state->c040PanelOpen;
    result->c040PanelOpenAfterPanel = state->c040PanelOpen;
    result->c159BlockedCountBefore = state->c159BlockedByG0299Count;
    result->c159BlockedCountAfter = state->c159BlockedByG0299Count;
    result->f0367StatusDispatchCountBefore = state->f0367StatusDispatchCount;
    result->f0367StatusDispatchCountAfter = state->f0367StatusDispatchCount;
    result->f0368SetLeaderCountBefore = state->f0368SetLeaderCount;
    result->f0368SetLeaderCountAfter = state->f0368SetLeaderCount;
    result->panelDispatchCountBefore = state->panelDispatchCount;
    result->panelDispatchCountAfter = state->panelDispatchCount;
    result->candidateClearCountBefore = state->candidateClearCount;
    result->candidateClearCountAfter = state->candidateClearCount;
}

static void finish_result(
    const Dm1V1MirrorCandidateC159ClickRotationComboStatePc34Compat *state,
    Dm1V1MirrorCandidateC159ClickRotationComboResultPc34Compat *result)
{
    result->partyChampionCountAfterPanel = state->partyChampionCount;
    result->candidateOrdinalAfterPanel = state->candidateChampionOrdinal;
    result->leaderIndexAfter = state->leaderIndex;
    result->leaderHandThingAfter = state->leaderHandThingOrdinal;
    result->c040PanelOpenAfterPanel = state->c040PanelOpen;
    result->c159BlockedCountAfter = state->c159BlockedByG0299Count;
    result->f0367StatusDispatchCountAfter = state->f0367StatusDispatchCount;
    result->f0368SetLeaderCountAfter = state->f0368SetLeaderCount;
    result->panelDispatchCountAfter = state->panelDispatchCount;
    result->candidateClearCountAfter = state->candidateClearCount;
    result->candidateClearedByPanelCommand =
        result->candidateOrdinalAfterC159 != 0u &&
        result->candidateOrdinalAfterPanel == 0u &&
        result->candidateClearCountAfter ==
            result->candidateClearCountBefore + 1;
    result->cancelRemovedCandidateChampion =
        result->panelCommand ==
            DM1_V1_MIRROR_CANDIDATE_C159_CLICK_ROTATION_COMBO_C162_CANCEL_PC34_COMPAT &&
        result->partyChampionCountAfterPanel ==
            result->partyChampionCountBefore - 1u;
    result->acceptedCandidateChampionRemains =
        (result->panelCommand ==
             DM1_V1_MIRROR_CANDIDATE_C159_CLICK_ROTATION_COMBO_C160_RESURRECT_PC34_COMPAT ||
         result->panelCommand ==
             DM1_V1_MIRROR_CANDIDATE_C159_CLICK_ROTATION_COMBO_C161_REINCARNATE_PC34_COMPAT) &&
        result->partyChampionCountAfterPanel ==
            result->partyChampionCountBefore;
    result->leaderHandPreserved =
        result->leaderHandThingBefore == result->leaderHandThingAfter;
    result->noLeaderHandRoutes =
        state->leaderHandPutCount == 0 &&
        state->leaderHandRemoveCount == 0 &&
        result->leaderHandPreserved;
    result->noSlotRoutes = state->slotRouteCount == 0;
    result->nonPanelInputsBlockedByG0299 =
        state->spellDispatchCount == 0 &&
        state->actionDispatchCount == 0 &&
        state->saveDispatchCount == 0;
}

void DM1_V1_MirrorCandidateC159ClickRotationCombo_InitPc34Compat(
    Dm1V1MirrorCandidateC159ClickRotationComboStatePc34Compat *state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->contractOnly = 1;
    state->partyChampionCount = kInitialPartyChampionCount;
    state->inventoryChampionOrdinal = kInitialInventoryOrdinal;
    state->leaderIndex = kInitialLeaderIndex;
    state->leaderEmptyHanded = 1;
    state->leaderHandThingOrdinal = kInitialLeaderHandThing;
}

int DM1_V1_MirrorCandidateC159ClickRotationCombo_PublishCandidatePc34Compat(
    Dm1V1MirrorCandidateC159ClickRotationComboStatePc34Compat *state)
{
    if (!state || !state->contractOnly || !state->leaderEmptyHanded ||
        state->partyChampionCount >= 4u ||
        state->candidateChampionOrdinal != 0u) {
        return 0;
    }

    state->candidateChampionOrdinal = kCandidateOrdinal;
    ++state->partyChampionCount;
    state->c040PanelOpen = 1;
    state->panelKind =
        DM1_V1_MIRROR_CANDIDATE_C159_CLICK_ROTATION_COMBO_M568_PANEL_PC34_COMPAT;
    state->c040PanelGraphic =
        DM1_V1_MIRROR_CANDIDATE_C159_CLICK_ROTATION_COMBO_C040_GRAPHIC_PC34_COMPAT;
    ++state->candidatePublishCount;
    return 1;
}

static void click_c159_row(
    Dm1V1MirrorCandidateC159ClickRotationComboStatePc34Compat *state,
    Dm1V1MirrorCandidateC159ClickRotationComboResultPc34Compat *result)
{
    ++state->c159RowClickCount;
    state->c159MappedLeaderCommand =
        DM1_V1_MIRROR_CANDIDATE_C159_CLICK_ROTATION_COMBO_C016_PC34_COMPAT;
    result->c159Clicked = 1;
    result->c159MapsToC016 = 1;

    if (state->candidateChampionOrdinal != 0u) {
        ++state->c159BlockedByG0299Count;
        result->c159BlockedByG0299 = 1;
        result->c159SetLeaderSkipped = 1;
        result->partyChampionCountAfterC159 = state->partyChampionCount;
        result->candidateOrdinalAfterC159 = state->candidateChampionOrdinal;
        result->c040PanelOpenAfterC159 = state->c040PanelOpen;
        result->panelStillOwnedAfterC159 =
            state->c040PanelOpen &&
            state->panelKind ==
                DM1_V1_MIRROR_CANDIDATE_C159_CLICK_ROTATION_COMBO_M568_PANEL_PC34_COMPAT;
        return;
    }

    ++state->f0367StatusDispatchCount;
    ++state->f0368SetLeaderCount;
    state->leaderIndex = 0;
    result->partyChampionCountAfterC159 = state->partyChampionCount;
    result->candidateOrdinalAfterC159 = state->candidateChampionOrdinal;
    result->c040PanelOpenAfterC159 = state->c040PanelOpen;
}

static void probe_non_panel_guards(
    Dm1V1MirrorCandidateC159ClickRotationComboStatePc34Compat *state)
{
    if (state->candidateChampionOrdinal == 0u) {
        ++state->spellDispatchCount;
        ++state->actionDispatchCount;
        ++state->saveDispatchCount;
    }
}

static void dispatch_panel_command(
    Dm1V1MirrorCandidateC159ClickRotationComboStatePc34Compat *state,
    int panelCommand,
    Dm1V1MirrorCandidateC159ClickRotationComboResultPc34Compat *result)
{
    if (!state->c040PanelOpen ||
        state->panelKind !=
            DM1_V1_MIRROR_CANDIDATE_C159_CLICK_ROTATION_COMBO_M568_PANEL_PC34_COMPAT ||
        !state->leaderEmptyHanded || !is_valid_panel_command(panelCommand)) {
        return;
    }

    ++state->panelDispatchCount;
    result->panelCommandDispatchedAfterC159 = result->panelStillOwnedAfterC159;
    if (panelCommand ==
        DM1_V1_MIRROR_CANDIDATE_C159_CLICK_ROTATION_COMBO_C160_RESURRECT_PC34_COMPAT) {
        ++state->resurrectDispatchCount;
    } else if (panelCommand ==
               DM1_V1_MIRROR_CANDIDATE_C159_CLICK_ROTATION_COMBO_C161_REINCARNATE_PC34_COMPAT) {
        ++state->reincarnateDispatchCount;
    } else {
        ++state->cancelDispatchCount;
        if (state->partyChampionCount > 0u) {
            --state->partyChampionCount;
        }
    }

    if (state->candidateChampionOrdinal != 0u) {
        state->candidateChampionOrdinal =
            DM1_V1_MIRROR_CANDIDATE_C159_CLICK_ROTATION_COMBO_NONE_PC34_COMPAT;
        ++state->candidateClearCount;
    }
    state->c040PanelOpen = 0;
    state->panelKind = 0;
}

int DM1_V1_MirrorCandidateC159ClickRotationCombo_RunPc34Compat(
    Dm1V1MirrorCandidateC159ClickRotationComboStatePc34Compat *state,
    int panelCommand,
    Dm1V1MirrorCandidateC159ClickRotationComboResultPc34Compat *outResult)
{
    if (!state || !outResult || !state->contractOnly) {
        return 0;
    }

    capture_before(state, panelCommand, outResult);
    click_c159_row(state, outResult);
    probe_non_panel_guards(state);
    dispatch_panel_command(state, panelCommand, outResult);
    finish_result(state, outResult);

    return outResult->panelCommandValid &&
           outResult->c159BlockedByG0299 &&
           outResult->panelCommandDispatchedAfterC159 &&
           outResult->candidateClearedByPanelCommand &&
           outResult->nonPanelInputsBlockedByG0299 &&
           outResult->noLeaderHandRoutes &&
           outResult->noSlotRoutes;
}

const Dm1V1MirrorCandidateC159ClickRotationComboEvidencePc34Compat *
DM1_V1_MirrorCandidateC159ClickRotationCombo_EvidencePc34Compat(void)
{
    return &s_evidence;
}
