#ifndef FIRESTAFF_DM1_V1_CHAMPION_TOP_ROW_RUNTIME_FRAME_ADMISSION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_TOP_ROW_RUNTIME_FRAME_ADMISSION_PC34_COMPAT_H

#include "dm1_v1_champion_top_row_m11_host_render_lifecycle_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Dm1V1ChampionTopRowRuntimeFrameAdmissionStatePc34 {
    unsigned int lastTick;
    unsigned int pendingClearGeneration;
    unsigned int lastCompositionGeneration;
} Dm1V1ChampionTopRowRuntimeFrameAdmissionStatePc34;

typedef enum Dm1V1ChampionTopRowRuntimeFrameAdmissionKindPc34 {
    DM1_V1_CHAMPION_TOP_ROW_RUNTIME_FRAME_CLEAR_PC34 = 1,
    DM1_V1_CHAMPION_TOP_ROW_RUNTIME_FRAME_COMPOSITION_PC34
} Dm1V1ChampionTopRowRuntimeFrameAdmissionKindPc34;

typedef struct Dm1V1ChampionTopRowRuntimeFrameAdmissionReceiptPc34 {
    int valid;
    int clearOnly;
    unsigned int tick;
    unsigned int generation;
    Dm1V1ChampionTopRowRuntimeFrameAdmissionKindPc34 kind;
    int commandCount;
    Dm1V1ChampionTopRowM11HostRenderCommandPc34
        commands[DM1_V1_CHAMPION_TOP_ROW_ATOMIC_FRAME_MAX_OPS_PC34];
} Dm1V1ChampionTopRowRuntimeFrameAdmissionReceiptPc34;

void dm1_v1_champion_top_row_runtime_frame_admission_init_pc34(
    Dm1V1ChampionTopRowRuntimeFrameAdmissionStatePc34 *state);

/* Source-owned final runtime gate for top-row output. It admits a full
 * composition only when atomic C008/C028/statusbar data matches the active
 * host-render lifecycle; stale or clear lifecycle paths admit clear commands
 * only and never forward original material pointers. */
int dm1_v1_champion_top_row_runtime_frame_admission_pc34(
    Dm1V1ChampionTopRowRuntimeFrameAdmissionStatePc34 *state,
    const Dm1V1ChampionTopRowAtomicFrameReceiptPc34 *atomicFrame,
    const Dm1V1ChampionTopRowM11HostRenderLifecycleReceiptPc34 *hostLifecycle,
    Dm1V1ChampionTopRowRuntimeFrameAdmissionReceiptPc34 *outReceipt);

const char *dm1_v1_champion_top_row_runtime_frame_admission_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
