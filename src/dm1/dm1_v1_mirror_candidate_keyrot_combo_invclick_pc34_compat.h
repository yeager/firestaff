#ifndef DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_PC34_COMPAT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_REDRAW_BYTES_PC34 16

typedef enum Dm1V1MirrorCandidateKeyrotComboInvclickTurnPc34Compat {
    DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_TURN_LEFT_PC34 = 1,
    DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_TURN_RIGHT_PC34 = 2
} Dm1V1MirrorCandidateKeyrotComboInvclickTurnPc34Compat;

typedef enum Dm1V1MirrorCandidateKeyrotComboInvclickCommandPc34Compat {
    DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_COMMAND_NONE_PC34 = 0,
    DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_COMMAND_TURN_LEFT_PC34 = 1,
    DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_COMMAND_TURN_RIGHT_PC34 = 2,
    DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_COMMAND_TOGGLE_INVENTORY_PC34 = 7,
    DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_COMMAND_STATUS_BOX_PC34 = 12,
    DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_COMMAND_SLOT_BOX_20_PC34 = 40,
    DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_COMMAND_PANEL_CANCEL_PC34 = 162
} Dm1V1MirrorCandidateKeyrotComboInvclickCommandPc34Compat;

typedef struct Dm1V1MirrorCandidateKeyrotComboInvclickEvidencePc34Compat {
    int contractOnly;
    const char *f0359PanelDispatchAnchor;
    const char *f0361KeyboardQueueAnchor;
    const char *f0380QueueDispatchAnchor;
    const char *f0280CandidatePendingAnchor;
    const char *f0282CandidateClearAnchor;
    const char *f0297LeaderHandPutAnchor;
    const char *f0302OccupiedSlotClickAnchor;
    const char *f0291StatusHandInteractionAnchor;
    const char *f0292DrawStateAnchor;
    const char *f0293RedrawAnchor;
    const char *defsAnchor;
    const char *nonDuplicateScope;
} Dm1V1MirrorCandidateKeyrotComboInvclickEvidencePc34Compat;

typedef struct Dm1V1MirrorCandidateKeyrotComboInvclickStatePc34Compat {
    int contractOnly;
    unsigned int g0299CandidateChampionOrdinal;
    unsigned int g0305PartyChampionCount;
    int g0308PartyDirection;
    int g0423InventoryChampionOrdinal;
    int g0424PanelContent;
    unsigned int g0415LeaderEmptyHanded;
    int queueLocked;
    int f0380InFlight;
    int pendingClickCommand;
    int deferredClickCommand;
    int lastKeyQueuedCommand;
    int rotationDispatchCount;
    int inventoryClickDispatchCount;
    int candidateClearCount;
    int f0297LeaderHandPutCount;
    int f0302SlotDispatchCount;
    int f0291StatusHandDrawCount;
    int f0292DrawStateCount;
    int f0293RedrawCount;
    unsigned int leaderHandThing;
    unsigned int inventorySlotThing;
} Dm1V1MirrorCandidateKeyrotComboInvclickStatePc34Compat;

typedef struct Dm1V1MirrorCandidateKeyrotComboInvclickResultPc34Compat {
    const Dm1V1MirrorCandidateKeyrotComboInvclickEvidencePc34Compat *evidence;
    unsigned int candidateBefore;
    unsigned int candidateAfter;
    int directionBefore;
    int directionAfterNoClick;
    int directionAfterClick;
    int keyQueuedCommand;
    int f0380InFlightObservedByClick;
    int pendingClickCommand;
    int deferredClickCommand;
    int clickDidNotClearCandidate;
    int clickDidNotDispatchInventoryMutation;
    int rotationProcessedNormally;
    int redrawByteIdenticalToNoClick;
    int noClickRedrawCount;
    int withClickRedrawCount;
    unsigned char noClickRedraw
        [DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_REDRAW_BYTES_PC34];
    unsigned char withClickRedraw
        [DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_REDRAW_BYTES_PC34];
} Dm1V1MirrorCandidateKeyrotComboInvclickResultPc34Compat;

void dm1_v1_mirror_candidate_keyrot_combo_invclick_init_pc34_compat(
    Dm1V1MirrorCandidateKeyrotComboInvclickStatePc34Compat *state);

int dm1_v1_mirror_candidate_keyrot_combo_invclick_run_pc34_compat(
    Dm1V1MirrorCandidateKeyrotComboInvclickTurnPc34Compat turn,
    Dm1V1MirrorCandidateKeyrotComboInvclickResultPc34Compat *outResult);

int dm1_v1_mirror_candidate_keyrot_combo_invclick_run_case_pc34_compat(
    const Dm1V1MirrorCandidateKeyrotComboInvclickStatePc34Compat *initial,
    Dm1V1MirrorCandidateKeyrotComboInvclickTurnPc34Compat turn,
    int pendingClickCommand,
    Dm1V1MirrorCandidateKeyrotComboInvclickResultPc34Compat *outResult);

const Dm1V1MirrorCandidateKeyrotComboInvclickEvidencePc34Compat *
dm1_v1_mirror_candidate_keyrot_combo_invclick_evidence_pc34_compat(void);

#ifdef __cplusplus
}
#endif

#endif
