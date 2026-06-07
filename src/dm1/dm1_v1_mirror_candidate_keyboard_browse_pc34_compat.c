#include "dm1_v1_mirror_candidate_keyboard_browse_pc34_compat.h"

#include <string.h>

/* Source-lock anchors for this keyboard-browse-only regression gate:
 * CHAMDRAW.C F0622_PrepareChampionIconBitmap:32-68 prepares champion icon
 * bitmaps and CHAMDRAW.C F0292 area:1040-1051 reuses F0622 for redraw.
 * CHAMPION.C F0331_CHAMPION_InitializeParty:1408-1412 installs primary and
 * secondary keyboard tables before discarding stale input.
 * COMMAND.C F0361_COMMAND_ProcessKeyPress:1709-1806 maps keyboard rows to
 * queued commands; COMMAND.C F0380:2164-2170 routes C125-C129 champion-icon
 * commands and COMMAND.C F0380:2180-2182/2302-2311 gates inventory, spell,
 * and action dispatch while G0299 is live.
 * COMMAND.C panel route tables:228-240 and 508-512 define C160/C161/C162;
 * COMMAND.C F0359/F0380 panel dispatch:1968-1990 reaches F0282 only through
 * M568 panel mouse routes, not through browse keys.
 * CLIKMENU.C F0366 movement highlight/damage route:262-294 is intentionally
 * outside this browse path.
 * COORD.C F0640_LoadLayoutData:2542-2561 and F0641:2564-2570 load layout
 * ranges used by panel zones; this test does not load layout graphics.
 */

enum {
    kInitialPageStart = 0,
    kInitialVisibleIndex = 0,
    kInitialPortraitReadCount = 1,
    kC127SensorBase = 127,
    kPortraitTokenBase = 0xC1270000u
};

static const Dm1V1MirrorCandidateKeyboardBrowseEvidencePc34Compat
    s_evidence = {
        "CHAMDRAW.C F0622_PrepareChampionIconBitmap:32-68; "
        "CHAMDRAW.C F0292 redraw area:1040-1051",
        "CHAMPION.C F0331_CHAMPION_InitializeParty:1408-1412",
        "COMMAND.C F0361_COMMAND_ProcessKeyPress:1709-1806",
        "COMMAND.C C160/C161/C162 tables:228-240,508-512; "
        "COMMAND.C F0359/F0380 panel dispatch:1968-1990",
        "COMMAND.C F0380 gates:2164-2170,2180-2182,2302-2311",
        "CLIKMENU.C F0366 movement highlight/damage route:262-294",
        "COORD.C F0640_LoadLayoutData:2542-2561; F0641:2564-2570",
        "non-overlap: mirror_candidate_click_cancel covers click cancel + "
        "deadzone; mirror_candidate_icon_refresh covers click-driven refresh; "
        "this gate covers keyboard-driven paged browse with non-trigger "
        "contract for resurrect/reincarnate/inventory-toggle",
        "contract-only deterministic roster and key sequence; C127 portrait "
        "tokens are synthetic and do not claim real-asset parity"
    };

static int clamp_visible_index(int visibleIndex, int pageSize)
{
    if (visibleIndex < 0) {
        return 0;
    }
    if (visibleIndex >= pageSize) {
        return pageSize - 1;
    }
    return visibleIndex;
}

static int highlighted_index(const Dm1V1MirrorCandidateKeyboardBrowseStatePc34Compat *state)
{
    if (!state) {
        return DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_NONE_PC34_COMPAT;
    }
    return state->pageStartIndex + state->visibleIndex;
}

static unsigned int highlighted_ordinal(
    const Dm1V1MirrorCandidateKeyboardBrowseStatePc34Compat *state)
{
    int index = highlighted_index(state);
    if (!state || index < 0 || index >= state->rosterCount) {
        return 0u;
    }
    return state->roster[index].championOrdinal;
}

static void read_highlight_portrait(
    Dm1V1MirrorCandidateKeyboardBrowseStatePc34Compat *state)
{
    int index;

    if (!state) {
        return;
    }
    index = highlighted_index(state);
    if (index < 0 || index >= state->rosterCount) {
        state->lastPortraitSensorIndex =
            DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_NONE_PC34_COMPAT;
        state->lastPortraitBitmapToken = 0u;
        return;
    }
    state->lastPortraitSensorIndex = state->roster[index].c127SensorIndex;
    state->lastPortraitBitmapToken = state->roster[index].portraitBitmapToken;
    ++state->portraitReadCount;
}

static void snapshot_begin(
    const Dm1V1MirrorCandidateKeyboardBrowseStatePc34Compat *state,
    Dm1V1MirrorCandidateKeyboardBrowseKeyPc34Compat key,
    Dm1V1MirrorCandidateKeyboardBrowseResultPc34Compat *result)
{
    if (!result) {
        return;
    }
    memset(result, 0, sizeof(*result));
    result->evidence = &s_evidence;
    result->key = key;
    if (!state) {
        return;
    }
    result->pageStartBefore = state->pageStartIndex;
    result->pageStartAfter = state->pageStartIndex;
    result->visibleIndexBefore = state->visibleIndex;
    result->visibleIndexAfter = state->visibleIndex;
    result->highlightedRosterIndexBefore = highlighted_index(state);
    result->highlightedRosterIndexAfter = result->highlightedRosterIndexBefore;
    result->highlightedChampionOrdinalBefore = highlighted_ordinal(state);
    result->highlightedChampionOrdinalAfter =
        result->highlightedChampionOrdinalBefore;
    result->pendingPressChampionOrdinalBefore =
        state->pendingPressChampionOrdinal;
    result->pendingPressChampionOrdinalAfter =
        state->pendingPressChampionOrdinal;
    result->selectedChampionOrdinalBefore = state->selectedChampionOrdinal;
    result->selectedChampionOrdinalAfter = state->selectedChampionOrdinal;
    result->candidateChampionOrdinalBefore = state->candidateChampionOrdinal;
    result->candidateChampionOrdinalAfter = state->candidateChampionOrdinal;
    result->inventoryChampionOrdinalBefore = state->inventoryChampionOrdinal;
    result->inventoryChampionOrdinalAfter = state->inventoryChampionOrdinal;
    result->inventoryPanelOpenBefore = state->inventoryPanelOpen;
    result->inventoryPanelOpenAfter = state->inventoryPanelOpen;
    result->resurrectDispatchCountBefore = state->resurrectDispatchCount;
    result->resurrectDispatchCountAfter = state->resurrectDispatchCount;
    result->reincarnateDispatchCountBefore = state->reincarnateDispatchCount;
    result->reincarnateDispatchCountAfter = state->reincarnateDispatchCount;
    result->inventoryToggleDispatchCountBefore =
        state->inventoryToggleDispatchCount;
    result->inventoryToggleDispatchCountAfter =
        state->inventoryToggleDispatchCount;
    result->cancelDispatchCountBefore = state->cancelDispatchCount;
    result->cancelDispatchCountAfter = state->cancelDispatchCount;
    result->deadzoneCancelCountBefore = state->deadzoneCancelCount;
    result->deadzoneCancelCountAfter = state->deadzoneCancelCount;
    result->portraitReadCountBefore = state->portraitReadCount;
    result->portraitReadCountAfter = state->portraitReadCount;
    result->portraitSensorIndexBefore = state->lastPortraitSensorIndex;
    result->portraitSensorIndexAfter = state->lastPortraitSensorIndex;
    result->portraitBitmapTokenBefore = state->lastPortraitBitmapToken;
    result->portraitBitmapTokenAfter = state->lastPortraitBitmapToken;
}

static void snapshot_finish(
    const Dm1V1MirrorCandidateKeyboardBrowseStatePc34Compat *state,
    Dm1V1MirrorCandidateKeyboardBrowseResultPc34Compat *result)
{
    if (!state || !result) {
        return;
    }
    result->pageStartAfter = state->pageStartIndex;
    result->visibleIndexAfter = state->visibleIndex;
    result->highlightedRosterIndexAfter = highlighted_index(state);
    result->highlightedChampionOrdinalAfter = highlighted_ordinal(state);
    result->pendingPressChampionOrdinalAfter =
        state->pendingPressChampionOrdinal;
    result->selectedChampionOrdinalAfter = state->selectedChampionOrdinal;
    result->candidateChampionOrdinalAfter = state->candidateChampionOrdinal;
    result->inventoryChampionOrdinalAfter = state->inventoryChampionOrdinal;
    result->inventoryPanelOpenAfter = state->inventoryPanelOpen;
    result->resurrectDispatchCountAfter = state->resurrectDispatchCount;
    result->reincarnateDispatchCountAfter = state->reincarnateDispatchCount;
    result->inventoryToggleDispatchCountAfter =
        state->inventoryToggleDispatchCount;
    result->cancelDispatchCountAfter = state->cancelDispatchCount;
    result->deadzoneCancelCountAfter = state->deadzoneCancelCount;
    result->portraitReadCountAfter = state->portraitReadCount;
    result->portraitSensorIndexAfter = state->lastPortraitSensorIndex;
    result->portraitBitmapTokenAfter = state->lastPortraitBitmapToken;
    result->visibleIndexStayedInPage =
        state->visibleIndex >= 0 && state->visibleIndex < state->pageSize;
    result->pageDownResetVisibleIndex =
        result->key !=
            DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_KEY_PAGE_DOWN_PC34_COMPAT ||
        (result->pageStartAfter == result->pageStartBefore + state->pageSize &&
         result->visibleIndexAfter == 0);
    result->pageUpWrappedPreviousPage =
        result->key !=
            DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_KEY_PAGE_UP_PC34_COMPAT ||
        (result->pageStartBefore == 0 &&
         result->pageStartAfter == state->rosterCount - state->pageSize &&
         result->visibleIndexAfter == 0);
    result->nonTriggerContractHeld =
        result->candidateChampionOrdinalBefore ==
            result->candidateChampionOrdinalAfter &&
        result->inventoryChampionOrdinalBefore ==
            result->inventoryChampionOrdinalAfter &&
        result->inventoryPanelOpenBefore == result->inventoryPanelOpenAfter &&
        result->resurrectDispatchCountBefore ==
            result->resurrectDispatchCountAfter &&
        result->reincarnateDispatchCountBefore ==
            result->reincarnateDispatchCountAfter &&
        result->inventoryToggleDispatchCountBefore ==
            result->inventoryToggleDispatchCountAfter &&
        result->cancelDispatchCountBefore == result->cancelDispatchCountAfter;
    result->partialPressReleasedCleanly =
        result->key !=
            DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_KEY_RELEASE_PC34_COMPAT ||
        (result->pendingPressChampionOrdinalBefore != 0u &&
         result->pendingPressChampionOrdinalAfter == 0u &&
         result->selectedChampionOrdinalBefore ==
            result->selectedChampionOrdinalAfter);
    result->portraitRefreshContractOnly =
        state->portraitRealAssetParityClaimed == 0 &&
        (result->key !=
             DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_KEY_PAGE_DOWN_PC34_COMPAT ||
         (result->portraitReadCountAfter ==
              result->portraitReadCountBefore + 1 &&
          result->portraitBitmapTokenAfter !=
              result->portraitBitmapTokenBefore));
}

void DM1_V1_MirrorCandidateKeyboardBrowse_InitPc34Compat(
    Dm1V1MirrorCandidateKeyboardBrowseStatePc34Compat *state)
{
    int i;

    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->panelActive = 1;
    state->rosterCount =
        DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_ROSTER_COUNT_PC34_COMPAT;
    state->pageSize =
        DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_PAGE_SIZE_PC34_COMPAT;
    state->pageStartIndex = kInitialPageStart;
    state->visibleIndex = kInitialVisibleIndex;
    state->highlightedRosterIndex = highlighted_index(state);
    state->pendingPressVisibleIndex =
        DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_NONE_PC34_COMPAT;
    state->lastPortraitSensorIndex =
        DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_NONE_PC34_COMPAT;

    for (i = 0; i < state->rosterCount; ++i) {
        state->roster[i].championOrdinal = (unsigned int)(i + 1);
        state->roster[i].c127SensorIndex = kC127SensorBase + i;
        state->roster[i].portraitBitmapToken =
            kPortraitTokenBase | (unsigned int)(i + 1);
    }

    read_highlight_portrait(state);
    state->portraitReadCount = kInitialPortraitReadCount;
}

int DM1_V1_MirrorCandidateKeyboardBrowse_ApplyKeyPc34Compat(
    Dm1V1MirrorCandidateKeyboardBrowseStatePc34Compat *state,
    Dm1V1MirrorCandidateKeyboardBrowseKeyPc34Compat key,
    Dm1V1MirrorCandidateKeyboardBrowseResultPc34Compat *outResult)
{
    int maxPageStart;

    snapshot_begin(state, key, outResult);
    if (!state || !outResult || !state->panelActive || state->rosterCount <= 0 ||
        state->pageSize <= 0) {
        snapshot_finish(state, outResult);
        return 0;
    }

    maxPageStart = state->rosterCount - state->pageSize;
    if (maxPageStart < 0) {
        maxPageStart = 0;
    }

    switch (key) {
    case DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_KEY_NEXT_PC34_COMPAT:
        state->visibleIndex =
            (state->visibleIndex + 1) % state->pageSize;
        outResult->consumed = 1;
        break;
    case DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_KEY_PREVIOUS_PC34_COMPAT:
        state->visibleIndex =
            (state->visibleIndex + state->pageSize - 1) % state->pageSize;
        outResult->consumed = 1;
        break;
    case DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_KEY_PAGE_DOWN_PC34_COMPAT:
        state->pageStartIndex += state->pageSize;
        if (state->pageStartIndex > maxPageStart) {
            state->pageStartIndex = 0;
        }
        state->visibleIndex = 0;
        read_highlight_portrait(state);
        outResult->consumed = 1;
        break;
    case DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_KEY_PAGE_UP_PC34_COMPAT:
        if (state->pageStartIndex == 0) {
            state->pageStartIndex = maxPageStart;
        } else {
            state->pageStartIndex -= state->pageSize;
            if (state->pageStartIndex < 0) {
                state->pageStartIndex = 0;
            }
        }
        state->visibleIndex = 0;
        read_highlight_portrait(state);
        outResult->consumed = 1;
        break;
    case DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_KEY_SELECT_PRESS_PC34_COMPAT:
        state->pendingPressVisibleIndex =
            clamp_visible_index(state->visibleIndex, state->pageSize);
        state->pendingPressChampionOrdinal = highlighted_ordinal(state);
        outResult->consumed = 1;
        break;
    case DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_KEY_RELEASE_PC34_COMPAT:
        if (state->pendingPressChampionOrdinal != 0u) {
            ++state->deadzoneCancelCount;
        }
        state->pendingPressVisibleIndex =
            DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_NONE_PC34_COMPAT;
        state->pendingPressChampionOrdinal = 0u;
        outResult->consumed = 1;
        break;
    default:
        break;
    }

    state->visibleIndex = clamp_visible_index(state->visibleIndex,
                                              state->pageSize);
    state->highlightedRosterIndex = highlighted_index(state);
    snapshot_finish(state, outResult);
    return outResult->consumed;
}

const Dm1V1MirrorCandidateKeyboardBrowseEvidencePc34Compat *
DM1_V1_MirrorCandidateKeyboardBrowse_EvidencePc34Compat(void)
{
    return &s_evidence;
}
