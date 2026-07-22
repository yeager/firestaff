#ifndef FIRESTAFF_DM1_V1_CHAMPION_TOP_ROW_RUNTIME_HOST_OUTPUT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_TOP_ROW_RUNTIME_HOST_OUTPUT_PC34_COMPAT_H

#include "dm1_v1_champion_top_row_runtime_frame_admission_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Dm1V1ChampionTopRowRuntimeHostOutputStatePc34 {
    unsigned int lastTick;
    unsigned int lastClearGeneration;
    unsigned int pendingClearGeneration;
    unsigned int lastCompositionGeneration;
    int hostCompositionActive;
} Dm1V1ChampionTopRowRuntimeHostOutputStatePc34;

typedef enum Dm1V1ChampionTopRowRuntimeHostOutputActionPc34 {
    DM1_V1_CHAMPION_TOP_ROW_RUNTIME_HOST_OUTPUT_CLEAR_PC34 = 1,
    DM1_V1_CHAMPION_TOP_ROW_RUNTIME_HOST_OUTPUT_PUBLISH_PC34,
    DM1_V1_CHAMPION_TOP_ROW_RUNTIME_HOST_OUTPUT_REVOKE_CLEAR_PC34
} Dm1V1ChampionTopRowRuntimeHostOutputActionPc34;

typedef struct Dm1V1ChampionTopRowRuntimeHostOutputReceiptPc34 {
    int valid;
    int clearOnly;
    unsigned int tick;
    unsigned int generation;
    Dm1V1ChampionTopRowRuntimeHostOutputActionPc34 action;
    int commandCount;
    Dm1V1ChampionTopRowM11HostRenderCommandPc34
        commands[DM1_V1_CHAMPION_TOP_ROW_ATOMIC_FRAME_MAX_OPS_PC34];
} Dm1V1ChampionTopRowRuntimeHostOutputReceiptPc34;

void dm1_v1_champion_top_row_runtime_host_output_init_pc34(
    Dm1V1ChampionTopRowRuntimeHostOutputStatePc34 *state);

/* Drives runtime-admitted top-row frames across ticks. A clear-only receipt
 * revokes an active host composition before forwarding only clear commands;
 * a new composition cannot publish until its matching clear generation. */
int dm1_v1_champion_top_row_runtime_host_output_step_pc34(
    Dm1V1ChampionTopRowRuntimeHostOutputStatePc34 *state,
    const Dm1V1ChampionTopRowRuntimeFrameAdmissionReceiptPc34 *admission,
    Dm1V1ChampionTopRowRuntimeHostOutputReceiptPc34 *outReceipt);

const char *dm1_v1_champion_top_row_runtime_host_output_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
