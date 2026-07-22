#ifndef FIRESTAFF_DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_LIFECYCLE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_LIFECYCLE_PC34_COMPAT_H

#include "dm1_v1_champion_top_row_m11_host_render_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Dm1V1ChampionTopRowM11HostRenderLifecycleStatePc34 {
    unsigned int lastTick;
    unsigned int lastClearGeneration;
    unsigned int pendingClearGeneration;
    unsigned int lastCompositionGeneration;
} Dm1V1ChampionTopRowM11HostRenderLifecycleStatePc34;

typedef enum Dm1V1ChampionTopRowM11HostRenderLifecyclePublicationPc34 {
    DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_LIFECYCLE_CLEAR_PC34 = 1,
    DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_LIFECYCLE_COMPOSITION_PC34,
    DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_LIFECYCLE_STALE_CLEAR_PC34
} Dm1V1ChampionTopRowM11HostRenderLifecyclePublicationPc34;

typedef struct Dm1V1ChampionTopRowM11HostRenderLifecycleReceiptPc34 {
    int valid;
    unsigned int tick;
    unsigned int generation;
    Dm1V1ChampionTopRowM11HostRenderLifecyclePublicationPc34 publication;
    Dm1V1ChampionTopRowM11HostRenderReceiptPc34 hostRender;
} Dm1V1ChampionTopRowM11HostRenderLifecycleReceiptPc34;

void dm1_v1_champion_top_row_m11_host_render_lifecycle_init_pc34(
    Dm1V1ChampionTopRowM11HostRenderLifecycleStatePc34 *state);

/* Advances host-render output across ticks. Stale data is admitted only as a
 * clear-only receipt, preserving the newest pending clear generation. */
int dm1_v1_champion_top_row_m11_host_render_lifecycle_step_pc34(
    Dm1V1ChampionTopRowM11HostRenderLifecycleStatePc34 *state,
    const Dm1V1ChampionTopRowM11HostRenderReceiptPc34 *hostRender,
    Dm1V1ChampionTopRowM11HostRenderLifecycleReceiptPc34 *outReceipt);

const char *dm1_v1_champion_top_row_m11_host_render_lifecycle_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
