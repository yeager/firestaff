#ifndef M11_MIRROR_CANDIDATE_SCROLL_PICKUP_PC34_COMPAT_H
#define M11_MIRROR_CANDIDATE_SCROLL_PICKUP_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define M11_MIRROR_CANDIDATE_SCROLL_PICKUP_SLOT_COUNT_PC34_COMPAT 8
#define M11_MIRROR_CANDIDATE_SCROLL_PICKUP_NONE_PC34_COMPAT (-1)
#define M11_MIRROR_CANDIDATE_SCROLL_PICKUP_C537_PC34_COMPAT 537
#define M11_MIRROR_CANDIDATE_SCROLL_PICKUP_C544_PC34_COMPAT 544
#define M11_MIRROR_CANDIDATE_SCROLL_PICKUP_C151_SCROLL_PC34_COMPAT 151

typedef enum M11MirrorCandidateScrollPickupMirrorStatePc34Compat {
    M11_MIRROR_CANDIDATE_SCROLL_PICKUP_MIRROR_CLOSED_PC34_COMPAT = 0,
    M11_MIRROR_CANDIDATE_SCROLL_PICKUP_MIRROR_CHEST_OPEN_PC34_COMPAT = 1,
    M11_MIRROR_CANDIDATE_SCROLL_PICKUP_MIRROR_SCROLL_IN_HAND_PC34_COMPAT = 2
} M11MirrorCandidateScrollPickupMirrorStatePc34Compat;

typedef struct M11MirrorCandidateScrollPickupProbePc34Compat {
    int initialMirrorState;
    int initialChestSlots[
        M11_MIRROR_CANDIDATE_SCROLL_PICKUP_SLOT_COUNT_PC34_COMPAT];
    int scrollOnFloor;
    int expectedPickup;
    int expectedMirrorStateAfter;
    int expectedActionHand;
    const char *anchorString;
} M11MirrorCandidateScrollPickupProbePc34Compat;

int M11_MirrorCandidateScrollPickup_RunPc34Compat(
    const M11MirrorCandidateScrollPickupProbePc34Compat *probe,
    int *passed,
    int *failed);

#ifdef __cplusplus
}
#endif

#endif
