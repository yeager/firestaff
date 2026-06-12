#ifndef FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_C040_EYE_LIVE_CANDIDATE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_C040_EYE_LIVE_CANDIDATE_PC34_COMPAT_H

#define DM1_V1_MC040ELC_CHEST_SLOT_COUNT_PC34 8

typedef struct DM1_V1_MirrorCandidateC040EyeLiveCandidateStatePc34 {
    int contractOnly;
    int candidateChampionOrdinal;
    int leaderEmptyHanded;
    int leaderHandThing;
    int pressingEye;
    int ignoreMouseMovements;
    int pointerHidden;
    int panelContent;
    int panelGraphic;
    int openChestThing;
    int chestSlots[DM1_V1_MC040ELC_CHEST_SLOT_COUNT_PC34];
    int chestCloseCount;
    int c040RedrawCount;
    int f0282DispatchCount;
    int objectPanelDrawCount;
    int championStatsDrawCount;
    int eyeIconGraphic;
    int viewportRedrawCount;
} DM1_V1_MirrorCandidateC040EyeLiveCandidateStatePc34;

typedef struct DM1_V1_MirrorCandidateC040EyeLiveCandidateResultPc34 {
    int accepted;
    int assertionCount;
    int candidateOrdinalBefore;
    int candidateOrdinalAfterPress;
    int candidateOrdinalAfterRelease;
    int panelContentBefore;
    int panelContentAfterPress;
    int panelContentAfterRelease;
    int panelGraphicAfterRelease;
    int pressingEyeAfterPress;
    int pressingEyeAfterRelease;
    int pointerHiddenAfterPress;
    int pointerHiddenAfterRelease;
    int openChestBefore;
    int openChestAfterRelease;
    int chestClosedOnRelease;
    int chestSlotsCleared;
    int f0347RedrewC040;
    int f0282NotDispatched;
    int leaderHandPreserved;
    int objectPanelDrawnDuringPress;
    int viewportRedrawnForPressAndRelease;
} DM1_V1_MirrorCandidateC040EyeLiveCandidateResultPc34;

typedef struct DM1_V1_MirrorCandidateC040EyeLiveCandidateSpecPc34 {
    const char *sourceEvidence;
    const char *nonOverlap;
    const char *f0280Anchor;
    const char *f0352Anchor;
    const char *f0353Anchor;
    const char *f0347Anchor;
    const char *f0334Anchor;
    const char *f0359Anchor;
    int c040PanelContent;
    int c040PanelGraphic;
    int c546EyeZone;
    int eyeLookingGraphic;
    int eyeNotLookingGraphic;
} DM1_V1_MirrorCandidateC040EyeLiveCandidateSpecPc34;

const DM1_V1_MirrorCandidateC040EyeLiveCandidateSpecPc34 *
dm1_v1_mirror_candidate_c040_eye_live_candidate_spec_pc34(void);

const char *
dm1_v1_mirror_candidate_c040_eye_live_candidate_source_evidence_pc34(void);

void dm1_v1_mirror_candidate_c040_eye_live_candidate_init_pc34(
    DM1_V1_MirrorCandidateC040EyeLiveCandidateStatePc34 *state);

int dm1_v1_mirror_candidate_c040_eye_live_candidate_run_pc34(
    DM1_V1_MirrorCandidateC040EyeLiveCandidateResultPc34 *out);

#endif
