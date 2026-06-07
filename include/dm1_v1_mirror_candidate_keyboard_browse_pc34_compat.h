#ifndef DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_ROSTER_COUNT_PC34_COMPAT 8
#define DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_PAGE_SIZE_PC34_COMPAT 4
#define DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_NONE_PC34_COMPAT (-1)

typedef enum Dm1V1MirrorCandidateKeyboardBrowseKeyPc34Compat {
    DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_KEY_NEXT_PC34_COMPAT = 1,
    DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_KEY_PREVIOUS_PC34_COMPAT = 2,
    DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_KEY_PAGE_DOWN_PC34_COMPAT = 3,
    DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_KEY_PAGE_UP_PC34_COMPAT = 4,
    DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_KEY_SELECT_PRESS_PC34_COMPAT = 5,
    DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_KEY_RELEASE_PC34_COMPAT = 6
} Dm1V1MirrorCandidateKeyboardBrowseKeyPc34Compat;

typedef struct Dm1V1MirrorCandidateKeyboardBrowseEvidencePc34Compat {
    const char *chamdrawIconBitmapAnchor;
    const char *championInterfaceInputAnchor;
    const char *commandKeyboardQueueAnchor;
    const char *commandPanelRouteAnchor;
    const char *commandCandidateGateAnchor;
    const char *clikmenuMovementHighlightAnchor;
    const char *coordLayoutAnchor;
    const char *nonOverlapNote;
    const char *contractScope;
} Dm1V1MirrorCandidateKeyboardBrowseEvidencePc34Compat;

typedef struct Dm1V1MirrorCandidateKeyboardBrowseRosterPc34Compat {
    unsigned int championOrdinal;
    int c127SensorIndex;
    unsigned int portraitBitmapToken;
} Dm1V1MirrorCandidateKeyboardBrowseRosterPc34Compat;

typedef struct Dm1V1MirrorCandidateKeyboardBrowseStatePc34Compat {
    int panelActive;
    int rosterCount;
    int pageSize;
    int pageStartIndex;
    int visibleIndex;
    int highlightedRosterIndex;
    int pendingPressVisibleIndex;
    unsigned int pendingPressChampionOrdinal;
    unsigned int selectedChampionOrdinal;
    unsigned int candidateChampionOrdinal;
    unsigned int inventoryChampionOrdinal;
    int inventoryPanelOpen;
    int resurrectDispatchCount;
    int reincarnateDispatchCount;
    int inventoryToggleDispatchCount;
    int cancelDispatchCount;
    int deadzoneCancelCount;
    int portraitReadCount;
    int portraitRealAssetParityClaimed;
    int lastPortraitSensorIndex;
    unsigned int lastPortraitBitmapToken;
    Dm1V1MirrorCandidateKeyboardBrowseRosterPc34Compat roster
        [DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_ROSTER_COUNT_PC34_COMPAT];
} Dm1V1MirrorCandidateKeyboardBrowseStatePc34Compat;

typedef struct Dm1V1MirrorCandidateKeyboardBrowseResultPc34Compat {
    const Dm1V1MirrorCandidateKeyboardBrowseEvidencePc34Compat *evidence;
    Dm1V1MirrorCandidateKeyboardBrowseKeyPc34Compat key;
    int consumed;
    int pageStartBefore;
    int pageStartAfter;
    int visibleIndexBefore;
    int visibleIndexAfter;
    int highlightedRosterIndexBefore;
    int highlightedRosterIndexAfter;
    unsigned int highlightedChampionOrdinalBefore;
    unsigned int highlightedChampionOrdinalAfter;
    unsigned int pendingPressChampionOrdinalBefore;
    unsigned int pendingPressChampionOrdinalAfter;
    unsigned int selectedChampionOrdinalBefore;
    unsigned int selectedChampionOrdinalAfter;
    unsigned int candidateChampionOrdinalBefore;
    unsigned int candidateChampionOrdinalAfter;
    unsigned int inventoryChampionOrdinalBefore;
    unsigned int inventoryChampionOrdinalAfter;
    int inventoryPanelOpenBefore;
    int inventoryPanelOpenAfter;
    int resurrectDispatchCountBefore;
    int resurrectDispatchCountAfter;
    int reincarnateDispatchCountBefore;
    int reincarnateDispatchCountAfter;
    int inventoryToggleDispatchCountBefore;
    int inventoryToggleDispatchCountAfter;
    int cancelDispatchCountBefore;
    int cancelDispatchCountAfter;
    int deadzoneCancelCountBefore;
    int deadzoneCancelCountAfter;
    int portraitReadCountBefore;
    int portraitReadCountAfter;
    int portraitSensorIndexBefore;
    int portraitSensorIndexAfter;
    unsigned int portraitBitmapTokenBefore;
    unsigned int portraitBitmapTokenAfter;
    int visibleIndexStayedInPage;
    int pageDownResetVisibleIndex;
    int pageUpWrappedPreviousPage;
    int nonTriggerContractHeld;
    int partialPressReleasedCleanly;
    int portraitRefreshContractOnly;
} Dm1V1MirrorCandidateKeyboardBrowseResultPc34Compat;

void DM1_V1_MirrorCandidateKeyboardBrowse_InitPc34Compat(
    Dm1V1MirrorCandidateKeyboardBrowseStatePc34Compat *state);

int DM1_V1_MirrorCandidateKeyboardBrowse_ApplyKeyPc34Compat(
    Dm1V1MirrorCandidateKeyboardBrowseStatePc34Compat *state,
    Dm1V1MirrorCandidateKeyboardBrowseKeyPc34Compat key,
    Dm1V1MirrorCandidateKeyboardBrowseResultPc34Compat *outResult);

const Dm1V1MirrorCandidateKeyboardBrowseEvidencePc34Compat *
DM1_V1_MirrorCandidateKeyboardBrowse_EvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif
