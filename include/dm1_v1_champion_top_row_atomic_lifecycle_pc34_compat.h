#ifndef FIRESTAFF_DM1_V1_CHAMPION_TOP_ROW_ATOMIC_LIFECYCLE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_TOP_ROW_ATOMIC_LIFECYCLE_PC34_COMPAT_H

#include "dm1_v1_champion_top_row_atomic_frame_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Dm1V1ChampionTopRowAtomicLifecycleStatePc34 {
    int initialized;
    int clearPublishedSinceComposition;
    unsigned int clearGeneration;
    unsigned int compositionGeneration;
} Dm1V1ChampionTopRowAtomicLifecycleStatePc34;

typedef enum Dm1V1ChampionTopRowAtomicLifecyclePublicationPc34 {
    DM1_V1_CHAMPION_TOP_ROW_ATOMIC_LIFECYCLE_NONE_PC34 = 0,
    DM1_V1_CHAMPION_TOP_ROW_ATOMIC_LIFECYCLE_CLEAR_PC34,
    DM1_V1_CHAMPION_TOP_ROW_ATOMIC_LIFECYCLE_COMPOSITION_PC34
} Dm1V1ChampionTopRowAtomicLifecyclePublicationPc34;

typedef struct Dm1V1ChampionTopRowAtomicLifecycleReceiptPc34 {
    int valid;
    Dm1V1ChampionTopRowAtomicLifecyclePublicationPc34 publication;
    unsigned int generation;
    Dm1V1ChampionTopRowAtomicFrameReceiptPc34 frame;
} Dm1V1ChampionTopRowAtomicLifecycleReceiptPc34;

void dm1_v1_champion_top_row_atomic_lifecycle_init_pc34(
    Dm1V1ChampionTopRowAtomicLifecycleStatePc34 *state);

/* Advances one source frame at a time. A clear-only frame must publish before
 * the following complete C008/C028/statusbar frame. Invalid or partial input
 * returns failure with neither lifecycle state nor output receipt advanced. */
int dm1_v1_champion_top_row_atomic_lifecycle_step_pc34(
    Dm1V1ChampionTopRowAtomicLifecycleStatePc34 *state,
    const Dm1V1ChampionTopRowAtomicFrameReceiptPc34 *frame,
    Dm1V1ChampionTopRowAtomicLifecycleReceiptPc34 *outReceipt);

const char *dm1_v1_champion_top_row_atomic_lifecycle_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
