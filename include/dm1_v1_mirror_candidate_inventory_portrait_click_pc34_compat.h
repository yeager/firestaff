#ifndef DM1_V1_MIRROR_CANDIDATE_INVENTORY_PORTRAIT_CLICK_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_INVENTORY_PORTRAIT_CLICK_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_CANDIDATE_INVENTORY_PORTRAIT_CLICK_CHAMPION_COUNT_PC34_COMPAT 4
#define DM1_V1_MIRROR_CANDIDATE_INVENTORY_PORTRAIT_CLICK_NONE_PC34_COMPAT (-1)

#define DM1_V1_MIRROR_CANDIDATE_INVENTORY_PORTRAIT_CLICK_C012_STATUS_BOX_0_PC34_COMPAT 12
#define DM1_V1_MIRROR_CANDIDATE_INVENTORY_PORTRAIT_CLICK_C016_SET_LEADER_0_PC34_COMPAT 16
#define DM1_V1_MIRROR_CANDIDATE_INVENTORY_PORTRAIT_CLICK_C159_NAME_ZONE_0_PC34_COMPAT 159
#define DM1_V1_MIRROR_CANDIDATE_INVENTORY_PORTRAIT_CLICK_C160_RESURRECT_PC34_COMPAT 160
#define DM1_V1_MIRROR_CANDIDATE_INVENTORY_PORTRAIT_CLICK_C161_REINCARNATE_PC34_COMPAT 161
#define DM1_V1_MIRROR_CANDIDATE_INVENTORY_PORTRAIT_CLICK_C162_CANCEL_PC34_COMPAT 162
#define DM1_V1_MIRROR_CANDIDATE_INVENTORY_PORTRAIT_CLICK_C175_FIRST_PORTRAIT_ZONE_PC34_COMPAT 175
#define DM1_V1_MIRROR_CANDIDATE_INVENTORY_PORTRAIT_CLICK_C040_PANEL_GRAPHIC_PC34_COMPAT 40

typedef struct Dm1V1MirrorCandidateInventoryPortraitClickEvidencePc34Compat {
    const char *commandGuardAnchor;
    const char *c159NameRouteAnchor;
    const char *defsPanelCommandAnchor;
    const char *defsNameZoneAnchor;
    const char *panelPortraitBoxAnchor;
    const char *chamdrawPortraitDispatchAnchor;
    const char *contractScope;
    const char *nonOverlapNote;
} Dm1V1MirrorCandidateInventoryPortraitClickEvidencePc34Compat;

typedef struct Dm1V1MirrorCandidateInventoryPortraitClickStatePc34Compat {
    int partyChampionCount;
    unsigned int candidateChampionOrdinal;
    unsigned int inventoryChampionOrdinal;
    int leaderIndex;
    int frontD1cMirrorChampionOrdinal;
    int c040PanelOpen;
    int c040PanelGraphic;
    int leaderHandThingOrdinal;
    int inventoryOpenCount;
    int resurrectClickOpenCount;
    int portraitClickAttemptCount;
    int portraitClickRejectCount;
    int portraitClickAcceptCount;
    int c159NameRouteCount;
    int commandGuardRejectCount;
} Dm1V1MirrorCandidateInventoryPortraitClickStatePc34Compat;

typedef struct Dm1V1MirrorCandidateInventoryPortraitClickResultPc34Compat {
    const Dm1V1MirrorCandidateInventoryPortraitClickEvidencePc34Compat *evidence;
    int requestedZone;
    int requestedChampionIndex;
    int isPortraitZone;
    int isChampionZeroPortraitZone;
    int isC159NameZone;
    int c159NonOverlap;
    int commandGuardChecked;
    int rejectedByG0299;
    int acceptedLeaderSwitch;
    int ignored;
    int inventoryOpenBefore;
    int inventoryOpenAfter;
    int c040PanelOpenBefore;
    int c040PanelOpenAfter;
    unsigned int candidateOrdinalBefore;
    unsigned int candidateOrdinalAfter;
    unsigned int inventoryOrdinalBefore;
    unsigned int inventoryOrdinalAfter;
    int leaderIndexBefore;
    int leaderIndexAfter;
    int leaderHandThingBefore;
    int leaderHandThingAfter;
    int frontMirrorOrdinalBefore;
    int frontMirrorOrdinalAfter;
    int portraitClickRejectCountBefore;
    int portraitClickRejectCountAfter;
    int portraitClickAcceptCountBefore;
    int portraitClickAcceptCountAfter;
    int c159NameRouteCountBefore;
    int c159NameRouteCountAfter;
    int commandGuardRejectCountBefore;
    int commandGuardRejectCountAfter;
    int leaderPreserved;
    int candidatePreserved;
    int inventoryLeaderPreserved;
    int leaderHandPreserved;
    int mirrorRoutePreserved;
} Dm1V1MirrorCandidateInventoryPortraitClickResultPc34Compat;

void DM1_V1_MirrorCandidateInventoryPortraitClick_InitPc34Compat(
    Dm1V1MirrorCandidateInventoryPortraitClickStatePc34Compat *state);

int DM1_V1_MirrorCandidateInventoryPortraitClick_OpenInventoryPc34Compat(
    Dm1V1MirrorCandidateInventoryPortraitClickStatePc34Compat *state,
    unsigned int championOrdinal);

int DM1_V1_MirrorCandidateInventoryPortraitClick_OpenC040FromResurrectClickPc34Compat(
    Dm1V1MirrorCandidateInventoryPortraitClickStatePc34Compat *state,
    unsigned int candidateChampionOrdinal);

int DM1_V1_MirrorCandidateInventoryPortraitClick_ProcessPortraitZonePc34Compat(
    Dm1V1MirrorCandidateInventoryPortraitClickStatePc34Compat *state,
    int zone,
    Dm1V1MirrorCandidateInventoryPortraitClickResultPc34Compat *outResult);

const Dm1V1MirrorCandidateInventoryPortraitClickEvidencePc34Compat *
DM1_V1_MirrorCandidateInventoryPortraitClick_EvidencePc34Compat(void);

int dm1_v1_mirror_candidate_inventory_portrait_click_run(
    int *passed,
    int *failed);

#ifdef __cplusplus
}
#endif

#endif
