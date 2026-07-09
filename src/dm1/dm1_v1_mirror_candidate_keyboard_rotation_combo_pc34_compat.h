#ifndef DM1_V1_MIRROR_CANDIDATE_KEYBOARD_ROTATION_COMBO_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_KEYBOARD_ROTATION_COMBO_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_CANDIDATE_KEYBOARD_ROTATION_COMBO_ROSTER_COUNT_PC34_COMPAT 4
#define DM1_V1_MIRROR_CANDIDATE_KEYBOARD_ROTATION_COMBO_RUNE_BUFFER_SIZE_PC34_COMPAT 4

typedef enum DM1_V1_MirrorCandidateKeyboardRotationComboRotationPc34Compat {
    DM1_V1_MIRROR_CANDIDATE_KEYBOARD_ROTATION_COMBO_PREVIOUS_PC34_COMPAT = -1,
    DM1_V1_MIRROR_CANDIDATE_KEYBOARD_ROTATION_COMBO_NEXT_PC34_COMPAT = 1
} DM1_V1_MirrorCandidateKeyboardRotationComboRotationPc34Compat;

typedef enum DM1_V1_MirrorCandidateKeyboardRotationComboInputPc34Compat {
    DM1_V1_MIRROR_CANDIDATE_KEYBOARD_ROTATION_COMBO_STATUS_BOX_PC34_COMPAT = 12,
    DM1_V1_MIRROR_CANDIDATE_KEYBOARD_ROTATION_COMBO_SPELL_RUNE_PC34_COMPAT = 100,
    DM1_V1_MIRROR_CANDIDATE_KEYBOARD_ROTATION_COMBO_SAVE_PC34_COMPAT = 140,
    DM1_V1_MIRROR_CANDIDATE_KEYBOARD_ROTATION_COMBO_REST_PC34_COMPAT = 145
} DM1_V1_MirrorCandidateKeyboardRotationComboInputPc34Compat;

typedef struct DM1_V1_MirrorCandidateKeyboardRotationComboEvidencePc34Compat {
    int contractOnly;
    const char *commandKeyboardQueueAnchor;
    const char *commandRotationDispatchAnchor;
    const char *commandStatusInventoryGuardAnchor;
    const char *commandSpellActionGuardAnchor;
    const char *commandRestGuardAnchor;
    const char *commandSaveGuardAnchor;
    const char *liveCandidateOnlyAllowedPath;
    const char *contractScope;
} DM1_V1_MirrorCandidateKeyboardRotationComboEvidencePc34Compat;

typedef struct DM1_V1_MirrorCandidateKeyboardRotationComboStatePc34Compat {
    int contractOnly;
    unsigned int g0299CandidateChampionOrdinal;
    unsigned int g0420CandidateIdentityOrdinal;
    unsigned int roster
        [DM1_V1_MIRROR_CANDIDATE_KEYBOARD_ROTATION_COMBO_ROSTER_COUNT_PC34_COMPAT];
    int rosterIndex;
    int rotationDispatchCount;
    int statusBoxDispatchCount;
    int spellRuneDispatchCount;
    int saveDispatchCount;
    int restDispatchCount;
    int lastSaveTick;
    int runeCount;
    char runeBuffer
        [DM1_V1_MIRROR_CANDIDATE_KEYBOARD_ROTATION_COMBO_RUNE_BUFFER_SIZE_PC34_COMPAT];
} DM1_V1_MirrorCandidateKeyboardRotationComboStatePc34Compat;

typedef struct DM1_V1_MirrorCandidateKeyboardRotationComboResultPc34Compat {
    const DM1_V1_MirrorCandidateKeyboardRotationComboEvidencePc34Compat *evidence;
    unsigned int candidateBefore;
    unsigned int candidateAfterRotation;
    unsigned int candidateAfterCombo;
    int rosterIndexBefore;
    int rosterIndexAfter;
    int rotationProcessedFirst;
    int rotationAllowedWhileCandidateLive;
    int nonRotationRejectedByCandidate;
    int onlyRotationSucceeded;
    int statusBoxDispatchCountBefore;
    int statusBoxDispatchCountAfter;
    int spellRuneDispatchCountBefore;
    int spellRuneDispatchCountAfter;
    int saveDispatchCountBefore;
    int saveDispatchCountAfter;
    int restDispatchCountBefore;
    int restDispatchCountAfter;
    int lastSaveTickBefore;
    int lastSaveTickAfter;
    int runeCountBefore;
    int runeCountAfter;
    char runeBufferBefore
        [DM1_V1_MIRROR_CANDIDATE_KEYBOARD_ROTATION_COMBO_RUNE_BUFFER_SIZE_PC34_COMPAT];
    char runeBufferAfter
        [DM1_V1_MIRROR_CANDIDATE_KEYBOARD_ROTATION_COMBO_RUNE_BUFFER_SIZE_PC34_COMPAT];
} DM1_V1_MirrorCandidateKeyboardRotationComboResultPc34Compat;

typedef DM1_V1_MirrorCandidateKeyboardRotationComboRotationPc34Compat
    Dm1V1MirrorCandidateKeyboardRotationComboRotationPc34Compat;
typedef DM1_V1_MirrorCandidateKeyboardRotationComboInputPc34Compat
    Dm1V1MirrorCandidateKeyboardRotationComboInputPc34Compat;
typedef DM1_V1_MirrorCandidateKeyboardRotationComboEvidencePc34Compat
    Dm1V1MirrorCandidateKeyboardRotationComboEvidencePc34Compat;
typedef DM1_V1_MirrorCandidateKeyboardRotationComboStatePc34Compat
    Dm1V1MirrorCandidateKeyboardRotationComboStatePc34Compat;
typedef DM1_V1_MirrorCandidateKeyboardRotationComboResultPc34Compat
    Dm1V1MirrorCandidateKeyboardRotationComboResultPc34Compat;

void DM1_V1_MirrorCandidateKeyboardRotationCombo_InitPc34Compat(
    DM1_V1_MirrorCandidateKeyboardRotationComboStatePc34Compat *state);

int DM1_V1_MirrorCandidateKeyboardRotationCombo_ApplyPc34Compat(
    DM1_V1_MirrorCandidateKeyboardRotationComboStatePc34Compat *state,
    DM1_V1_MirrorCandidateKeyboardRotationComboRotationPc34Compat rotation,
    DM1_V1_MirrorCandidateKeyboardRotationComboInputPc34Compat nonRotation,
    DM1_V1_MirrorCandidateKeyboardRotationComboResultPc34Compat *outResult);

const DM1_V1_MirrorCandidateKeyboardRotationComboEvidencePc34Compat *
DM1_V1_MirrorCandidateKeyboardRotationCombo_EvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif
