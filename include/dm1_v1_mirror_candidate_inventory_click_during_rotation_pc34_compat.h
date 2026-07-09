#ifndef DM1_V1_MIRROR_CANDIDATE_INVENTORY_CLICK_DURING_ROTATION_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_INVENTORY_CLICK_DURING_ROTATION_PC34_COMPAT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum DM1_V1_MirrorCandidateInventoryClickRotationPc34Compat {
    DM1_V1_MIRROR_CANDIDATE_ROTATION_COMPLETE_PC34_COMPAT = 0,
    DM1_V1_MIRROR_CANDIDATE_ROTATION_C156_TURN_LEFT_PC34_COMPAT = 156,
    DM1_V1_MIRROR_CANDIDATE_ROTATION_C157_TURN_RIGHT_PC34_COMPAT = 157,
    DM1_V1_MIRROR_CANDIDATE_ROTATION_PAUSED_PC34_COMPAT = 900
} DM1_V1_MirrorCandidateInventoryClickRotationPc34Compat;

typedef enum DM1_V1_MirrorCandidateInventoryClickTypePc34Compat {
    DM1_V1_MIRROR_CANDIDATE_CLICK_LEFT_PC34_COMPAT = 1,
    DM1_V1_MIRROR_CANDIDATE_CLICK_RIGHT_PC34_COMPAT = 2,
    DM1_V1_MIRROR_CANDIDATE_CLICK_MIDDLE_PC34_COMPAT = 3
} DM1_V1_MirrorCandidateInventoryClickTypePc34Compat;

typedef enum DM1_V1_MirrorCandidateInventoryClickTargetPc34Compat {
    DM1_V1_MIRROR_CANDIDATE_CLICK_TARGET_PORTRAIT_PC34_COMPAT = 1,
    DM1_V1_MIRROR_CANDIDATE_CLICK_TARGET_ACTION_HAND_SLOT_PC34_COMPAT = 2,
    DM1_V1_MIRROR_CANDIDATE_CLICK_TARGET_CLOSED_INVENTORY_PC34_COMPAT = 3,
    DM1_V1_MIRROR_CANDIDATE_CLICK_TARGET_OPEN_INVENTORY_PC34_COMPAT = 4
} DM1_V1_MirrorCandidateInventoryClickTargetPc34Compat;

typedef enum DM1_V1_MirrorCandidateInventoryClickDecisionPc34Compat {
    DM1_V1_MIRROR_CANDIDATE_DISPATCH_NORMALLY_PC34_COMPAT = 1,
    DM1_V1_MIRROR_CANDIDATE_SUPPRESS_DURING_ROTATION_PC34_COMPAT = 2,
    DM1_V1_MIRROR_CANDIDATE_QUEUE_FOR_AFTER_ROTATION_PC34_COMPAT = 3,
    DM1_V1_MIRROR_CANDIDATE_REJECT_INVALID_CLICK_TYPE_PC34_COMPAT = 4
} DM1_V1_MirrorCandidateInventoryClickDecisionPc34Compat;

typedef struct DM1_V1_MirrorCandidateInventoryClickDuringRotationCasePc34Compat {
    int rotationInProgress;
    int clickType;
    int clickTarget;
    DM1_V1_MirrorCandidateInventoryClickDecisionPc34Compat expectedDecision;
    const char *caseName;
    const char *sourceAnchor;
} DM1_V1_MirrorCandidateInventoryClickDuringRotationCasePc34Compat;

typedef struct DM1_V1_MirrorCandidateInventoryClickDuringRotationEvidencePc34Compat {
    int contractOnly;
    const char *championPartyDirectionAnchor;
    const char *commandPanelDispatchAnchor;
    const char *mouseClickQueueAnchor;
    const char *championRedrawAnchor;
    const char *panelPortraitBlitAnchor;
    const char *contractScope;
} DM1_V1_MirrorCandidateInventoryClickDuringRotationEvidencePc34Compat;

typedef DM1_V1_MirrorCandidateInventoryClickRotationPc34Compat
    Dm1V1MirrorCandidateInventoryClickRotationPc34Compat;
typedef DM1_V1_MirrorCandidateInventoryClickTypePc34Compat
    Dm1V1MirrorCandidateInventoryClickTypePc34Compat;
typedef DM1_V1_MirrorCandidateInventoryClickTargetPc34Compat
    Dm1V1MirrorCandidateInventoryClickTargetPc34Compat;
typedef DM1_V1_MirrorCandidateInventoryClickDecisionPc34Compat
    Dm1V1MirrorCandidateInventoryClickDecisionPc34Compat;
typedef DM1_V1_MirrorCandidateInventoryClickDuringRotationCasePc34Compat
    Dm1V1MirrorCandidateInventoryClickDuringRotationCasePc34Compat;
typedef DM1_V1_MirrorCandidateInventoryClickDuringRotationEvidencePc34Compat
    Dm1V1MirrorCandidateInventoryClickDuringRotationEvidencePc34Compat;

DM1_V1_MirrorCandidateInventoryClickDecisionPc34Compat
dm1_v1_mirror_candidate_inventory_click_during_rotation_decide(
    int rotationInProgress,
    int clickType,
    int clickTarget);

const DM1_V1_MirrorCandidateInventoryClickDuringRotationCasePc34Compat *
dm1_v1_mirror_candidate_inventory_click_during_rotation_table(
    size_t *outCount);

int dm1_v1_mirror_candidate_inventory_click_during_rotation_case_at(
    size_t index,
    const DM1_V1_MirrorCandidateInventoryClickDuringRotationCasePc34Compat
        **outCase);

const DM1_V1_MirrorCandidateInventoryClickDuringRotationEvidencePc34Compat *
dm1_v1_mirror_candidate_inventory_click_during_rotation_evidence(void);

#ifdef __cplusplus
}
#endif

#endif
