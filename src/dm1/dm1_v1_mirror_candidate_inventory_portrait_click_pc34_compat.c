#include "dm1_v1_mirror_candidate_inventory_portrait_click_pc34_compat.h"

#include <string.h>

/* ReDMCSB source-lock anchors:
 * COMMAND.C F0380:2159-2181 gates status-box and inventory dispatch on
 * !G0299_ui_CandidateChampionOrdinal while C040 owns the pending candidate.
 * COMMAND.C:484-488 binds the separate C159..C162 champion name-row zones to
 * G0455 leader commands; this portrait-box gate must not enter that route.
 * DEFS.H:338-340 names C160/C161/C162 panel commands in the local source.
 * DEFS.H:3787-3790 names C159..C162 champion-name zones; in this checkout
 * DEFS.H:4041-4042 are viewport wall zones, not C160/C161/C162.
 * PANEL.C F0354:2208-2240 draws C175_ZONE_FIRST_CHAMPION_STATUS_BOX +
 * championIndex as the inventory status-box portrait rectangle.
 * CHAMDRAW.C F0292:810-812 dispatches inventory champions to F0354.
 */

enum {
    kLeaderChampionIndex = 0,
    kInitialLeaderHandThing = 0x4d2,
    kInitialFrontMirrorOrdinal = 1
};

static const Dm1V1MirrorCandidateInventoryPortraitClickEvidencePc34Compat
    s_evidence = {
        "COMMAND.C F0380:2159-2181 !G0299 guard for status-box and "
        "inventory/champion dispatch",
        "COMMAND.C:484-488 G0455 C159..C162 name-row route to C016..C019",
        "DEFS.H:338-340 C160/C161/C162 panel commands; local DEFS.H:4041-4042 "
        "are viewport wall zones, not the panel-command definitions",
        "DEFS.H:3787-3790 C159/C160/C161/C162 champion-name zones; "
        "DEFS.H:3793 C175 first champion status-box portrait zone",
        "PANEL.C F0354:2208-2240 portrait-box rectangle and C175+championIndex",
        "CHAMDRAW.C F0292:810-812 inventory champion portrait dispatch to F0354",
        "contract-only runtime regression gate; no real inventory, champion "
        "bitmap, savegame, dungeon, or asset data is loaded or claimed",
        "C175+0 portrait-box click is disjoint from the C159 champion-name "
        "G0455 route and from existing C159 champion-icon/candidate tests"
    };

static int valid_champion_index(int championIndex)
{
    return championIndex >= 0 &&
           championIndex <
               DM1_V1_MIRROR_CANDIDATE_INVENTORY_PORTRAIT_CLICK_CHAMPION_COUNT_PC34_COMPAT;
}

static int champion_index_from_portrait_zone(int zone)
{
    int championIndex =
        zone -
        DM1_V1_MIRROR_CANDIDATE_INVENTORY_PORTRAIT_CLICK_C175_FIRST_PORTRAIT_ZONE_PC34_COMPAT;

    if (!valid_champion_index(championIndex)) {
        return DM1_V1_MIRROR_CANDIDATE_INVENTORY_PORTRAIT_CLICK_NONE_PC34_COMPAT;
    }
    return championIndex;
}

static int inventory_is_open(
    const Dm1V1MirrorCandidateInventoryPortraitClickStatePc34Compat *state)
{
    return state && state->inventoryChampionOrdinal != 0u;
}

static void begin_result(
    const Dm1V1MirrorCandidateInventoryPortraitClickStatePc34Compat *state,
    int zone,
    Dm1V1MirrorCandidateInventoryPortraitClickResultPc34Compat *result)
{
    int championIndex;

    if (!result) {
        return;
    }
    memset(result, 0, sizeof(*result));
    result->evidence = &s_evidence;
    result->requestedZone = zone;
    result->requestedChampionIndex =
        DM1_V1_MIRROR_CANDIDATE_INVENTORY_PORTRAIT_CLICK_NONE_PC34_COMPAT;
    result->frontMirrorOrdinalBefore =
        DM1_V1_MIRROR_CANDIDATE_INVENTORY_PORTRAIT_CLICK_NONE_PC34_COMPAT;
    result->frontMirrorOrdinalAfter = result->frontMirrorOrdinalBefore;
    championIndex = champion_index_from_portrait_zone(zone);
    if (championIndex !=
        DM1_V1_MIRROR_CANDIDATE_INVENTORY_PORTRAIT_CLICK_NONE_PC34_COMPAT) {
        result->requestedChampionIndex = championIndex;
        result->isPortraitZone = 1;
        result->isChampionZeroPortraitZone = championIndex == 0;
    }
    result->isC159NameZone =
        zone ==
        DM1_V1_MIRROR_CANDIDATE_INVENTORY_PORTRAIT_CLICK_C159_NAME_ZONE_0_PC34_COMPAT;
    result->c159NonOverlap =
        result->isChampionZeroPortraitZone && !result->isC159NameZone;
    if (!state) {
        return;
    }

    result->inventoryOpenBefore = inventory_is_open(state);
    result->inventoryOpenAfter = result->inventoryOpenBefore;
    result->c040PanelOpenBefore = state->c040PanelOpen;
    result->c040PanelOpenAfter = state->c040PanelOpen;
    result->candidateOrdinalBefore = state->candidateChampionOrdinal;
    result->candidateOrdinalAfter = state->candidateChampionOrdinal;
    result->inventoryOrdinalBefore = state->inventoryChampionOrdinal;
    result->inventoryOrdinalAfter = state->inventoryChampionOrdinal;
    result->leaderIndexBefore = state->leaderIndex;
    result->leaderIndexAfter = state->leaderIndex;
    result->leaderHandThingBefore = state->leaderHandThingOrdinal;
    result->leaderHandThingAfter = state->leaderHandThingOrdinal;
    result->frontMirrorOrdinalBefore = state->frontD1cMirrorChampionOrdinal;
    result->frontMirrorOrdinalAfter = state->frontD1cMirrorChampionOrdinal;
    result->portraitClickRejectCountBefore = state->portraitClickRejectCount;
    result->portraitClickRejectCountAfter = state->portraitClickRejectCount;
    result->portraitClickAcceptCountBefore = state->portraitClickAcceptCount;
    result->portraitClickAcceptCountAfter = state->portraitClickAcceptCount;
    result->c159NameRouteCountBefore = state->c159NameRouteCount;
    result->c159NameRouteCountAfter = state->c159NameRouteCount;
    result->commandGuardRejectCountBefore = state->commandGuardRejectCount;
    result->commandGuardRejectCountAfter = state->commandGuardRejectCount;
}

static void finish_result(
    const Dm1V1MirrorCandidateInventoryPortraitClickStatePc34Compat *state,
    Dm1V1MirrorCandidateInventoryPortraitClickResultPc34Compat *result)
{
    if (!state || !result) {
        return;
    }
    result->inventoryOpenAfter = inventory_is_open(state);
    result->c040PanelOpenAfter = state->c040PanelOpen;
    result->candidateOrdinalAfter = state->candidateChampionOrdinal;
    result->inventoryOrdinalAfter = state->inventoryChampionOrdinal;
    result->leaderIndexAfter = state->leaderIndex;
    result->leaderHandThingAfter = state->leaderHandThingOrdinal;
    result->frontMirrorOrdinalAfter = state->frontD1cMirrorChampionOrdinal;
    result->portraitClickRejectCountAfter = state->portraitClickRejectCount;
    result->portraitClickAcceptCountAfter = state->portraitClickAcceptCount;
    result->c159NameRouteCountAfter = state->c159NameRouteCount;
    result->commandGuardRejectCountAfter = state->commandGuardRejectCount;
    result->leaderPreserved =
        result->leaderIndexBefore == result->leaderIndexAfter;
    result->candidatePreserved =
        result->candidateOrdinalBefore == result->candidateOrdinalAfter;
    result->inventoryLeaderPreserved =
        result->inventoryOrdinalBefore == result->inventoryOrdinalAfter;
    result->leaderHandPreserved =
        result->leaderHandThingBefore == result->leaderHandThingAfter;
    result->mirrorRoutePreserved =
        result->frontMirrorOrdinalBefore == result->frontMirrorOrdinalAfter;
}

void DM1_V1_MirrorCandidateInventoryPortraitClick_InitPc34Compat(
    Dm1V1MirrorCandidateInventoryPortraitClickStatePc34Compat *state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->partyChampionCount = 2;
    state->leaderIndex = kLeaderChampionIndex;
    state->frontD1cMirrorChampionOrdinal = kInitialFrontMirrorOrdinal;
    state->leaderHandThingOrdinal = kInitialLeaderHandThing;
}

int DM1_V1_MirrorCandidateInventoryPortraitClick_OpenInventoryPc34Compat(
    Dm1V1MirrorCandidateInventoryPortraitClickStatePc34Compat *state,
    unsigned int championOrdinal)
{
    if (!state || championOrdinal == 0u ||
        championOrdinal > (unsigned int)state->partyChampionCount) {
        return 0;
    }
    state->inventoryChampionOrdinal = championOrdinal;
    ++state->inventoryOpenCount;
    return 1;
}

int DM1_V1_MirrorCandidateInventoryPortraitClick_OpenC040FromResurrectClickPc34Compat(
    Dm1V1MirrorCandidateInventoryPortraitClickStatePc34Compat *state,
    unsigned int candidateChampionOrdinal)
{
    if (!state || candidateChampionOrdinal == 0u ||
        candidateChampionOrdinal > (unsigned int)state->partyChampionCount) {
        return 0;
    }

    /* ReDMCSB: MOVESENS.C and REVIVE.C F0280 publish G0299 before the C040
     * resurrect/reincarnate panel accepts C160/C161/C162 commands. */
    state->candidateChampionOrdinal = candidateChampionOrdinal;
    state->c040PanelOpen = 1;
    state->c040PanelGraphic =
        DM1_V1_MIRROR_CANDIDATE_INVENTORY_PORTRAIT_CLICK_C040_PANEL_GRAPHIC_PC34_COMPAT;
    ++state->resurrectClickOpenCount;
    return 1;
}

int DM1_V1_MirrorCandidateInventoryPortraitClick_ProcessPortraitZonePc34Compat(
    Dm1V1MirrorCandidateInventoryPortraitClickStatePc34Compat *state,
    int zone,
    Dm1V1MirrorCandidateInventoryPortraitClickResultPc34Compat *outResult)
{
    int championIndex;

    begin_result(state, zone, outResult);
    if (!state || !outResult) {
        return 0;
    }

    if (outResult->isC159NameZone) {
        ++state->c159NameRouteCount;
        finish_result(state, outResult);
        return 0;
    }

    championIndex = outResult->requestedChampionIndex;
    if (!outResult->isPortraitZone || !inventory_is_open(state) ||
        championIndex >= state->partyChampionCount) {
        outResult->ignored = 1;
        finish_result(state, outResult);
        return 0;
    }

    ++state->portraitClickAttemptCount;
    outResult->commandGuardChecked = 1;
    if (state->candidateChampionOrdinal != 0u) {
        ++state->portraitClickRejectCount;
        ++state->commandGuardRejectCount;
        outResult->rejectedByG0299 = 1;
        outResult->ignored = 1;
        finish_result(state, outResult);
        return 0;
    }

    state->leaderIndex = championIndex;
    ++state->portraitClickAcceptCount;
    outResult->acceptedLeaderSwitch = 1;
    finish_result(state, outResult);
    return 1;
}

const Dm1V1MirrorCandidateInventoryPortraitClickEvidencePc34Compat *
DM1_V1_MirrorCandidateInventoryPortraitClick_EvidencePc34Compat(void)
{
    return &s_evidence;
}

static void self_check(int condition, int *passed, int *failed)
{
    if (condition) {
        ++*passed;
    } else {
        ++*failed;
    }
}

int dm1_v1_mirror_candidate_inventory_portrait_click_run(
    int *passed,
    int *failed)
{
    Dm1V1MirrorCandidateInventoryPortraitClickStatePc34Compat state;
    Dm1V1MirrorCandidateInventoryPortraitClickResultPc34Compat blocked;
    Dm1V1MirrorCandidateInventoryPortraitClickResultPc34Compat baseline;
    const Dm1V1MirrorCandidateInventoryPortraitClickEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateInventoryPortraitClick_EvidencePc34Compat();
    int localPassed = 0;
    int localFailed = 0;
    int openedInventory;
    int openedPanel;
    int returned;

    DM1_V1_MirrorCandidateInventoryPortraitClick_InitPc34Compat(&state);
    openedInventory =
        DM1_V1_MirrorCandidateInventoryPortraitClick_OpenInventoryPc34Compat(
            &state, 1u);
    openedPanel =
        DM1_V1_MirrorCandidateInventoryPortraitClick_OpenC040FromResurrectClickPc34Compat(
            &state, 2u);
    returned =
        DM1_V1_MirrorCandidateInventoryPortraitClick_ProcessPortraitZonePc34Compat(
            &state,
            DM1_V1_MIRROR_CANDIDATE_INVENTORY_PORTRAIT_CLICK_C175_FIRST_PORTRAIT_ZONE_PC34_COMPAT,
            &blocked);

    self_check(e != 0, &localPassed, &localFailed);
    self_check(openedInventory == 1, &localPassed, &localFailed);
    self_check(openedPanel == 1, &localPassed, &localFailed);
    self_check(returned == 0, &localPassed, &localFailed);
    self_check(blocked.isChampionZeroPortraitZone == 1, &localPassed,
               &localFailed);
    self_check(blocked.isC159NameZone == 0, &localPassed, &localFailed);
    self_check(blocked.c159NonOverlap == 1, &localPassed, &localFailed);
    self_check(blocked.commandGuardChecked == 1, &localPassed, &localFailed);
    self_check(blocked.rejectedByG0299 == 1, &localPassed, &localFailed);
    self_check(blocked.ignored == 1, &localPassed, &localFailed);
    self_check(blocked.leaderPreserved == 1, &localPassed, &localFailed);
    self_check(blocked.candidatePreserved == 1, &localPassed, &localFailed);
    self_check(blocked.inventoryLeaderPreserved == 1, &localPassed,
               &localFailed);
    self_check(blocked.leaderHandPreserved == 1, &localPassed, &localFailed);
    self_check(blocked.mirrorRoutePreserved == 1, &localPassed, &localFailed);
    self_check(state.leaderIndex == 0, &localPassed, &localFailed);
    self_check(state.candidateChampionOrdinal == 2u, &localPassed,
               &localFailed);
    self_check(state.inventoryChampionOrdinal == 1u, &localPassed,
               &localFailed);
    self_check(state.leaderHandThingOrdinal == kInitialLeaderHandThing,
               &localPassed, &localFailed);
    self_check(state.frontD1cMirrorChampionOrdinal == kInitialFrontMirrorOrdinal,
               &localPassed, &localFailed);
    self_check(state.portraitClickRejectCount == 1, &localPassed,
               &localFailed);
    self_check(state.commandGuardRejectCount == 1, &localPassed, &localFailed);
    self_check(state.c159NameRouteCount == 0, &localPassed, &localFailed);

    DM1_V1_MirrorCandidateInventoryPortraitClick_InitPc34Compat(&state);
    state.leaderIndex = 1;
    (void)DM1_V1_MirrorCandidateInventoryPortraitClick_OpenInventoryPc34Compat(
        &state, 2u);
    returned =
        DM1_V1_MirrorCandidateInventoryPortraitClick_ProcessPortraitZonePc34Compat(
            &state,
            DM1_V1_MIRROR_CANDIDATE_INVENTORY_PORTRAIT_CLICK_C175_FIRST_PORTRAIT_ZONE_PC34_COMPAT,
            &baseline);
    self_check(returned == 1, &localPassed, &localFailed);
    self_check(baseline.acceptedLeaderSwitch == 1, &localPassed, &localFailed);
    self_check(baseline.rejectedByG0299 == 0, &localPassed, &localFailed);
    self_check(baseline.leaderIndexBefore == 1, &localPassed, &localFailed);
    self_check(baseline.leaderIndexAfter == 0, &localPassed, &localFailed);
    self_check(state.leaderIndex == 0, &localPassed, &localFailed);

    if (passed) {
        *passed = localPassed;
    }
    if (failed) {
        *failed = localFailed;
    }
    return localFailed == 0;
}
