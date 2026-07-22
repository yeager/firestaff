#ifndef FIRESTAFF_DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_PC34_COMPAT_H

#include "dm1_v1_champion_top_row_m11_lifecycle_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Dm1V1ChampionTopRowM11HostRenderStatePc34 {
    unsigned int lastTick;
    unsigned int pendingClearGeneration;
    unsigned int lastCompositionGeneration;
} Dm1V1ChampionTopRowM11HostRenderStatePc34;

typedef enum Dm1V1ChampionTopRowM11HostRenderCommandKindPc34 {
    DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_CLEAR_PC34 = 1,
    DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_BLIT_C008_PC34,
    DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_COMPOSE_C028_PC34,
    DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_STATUS_BAR_PC34
} Dm1V1ChampionTopRowM11HostRenderCommandKindPc34;

typedef struct Dm1V1ChampionTopRowM11HostRenderCommandPc34 {
    Dm1V1ChampionTopRowM11HostRenderCommandKindPc34 kind;
    Dm1V1ChampionTopRowAtomicFrameOpPc34 source;
} Dm1V1ChampionTopRowM11HostRenderCommandPc34;

typedef struct Dm1V1ChampionTopRowM11HostRenderReceiptPc34 {
    int valid;
    int clearOnly;
    unsigned int tick;
    unsigned int generation;
    int commandCount;
    Dm1V1ChampionTopRowM11HostRenderCommandPc34
        commands[DM1_V1_CHAMPION_TOP_ROW_ATOMIC_FRAME_MAX_OPS_PC34];
} Dm1V1ChampionTopRowM11HostRenderReceiptPc34;

void dm1_v1_champion_top_row_m11_host_render_init_pc34(
    Dm1V1ChampionTopRowM11HostRenderStatePc34 *state);

/* Converts active M11-bound lifecycle receipts into host render commands.
 * A stale-proof lifecycle receipt produces only its explicit relevant-zone
 * clears; no original material survives that route. */
int dm1_v1_champion_top_row_m11_host_render_receipt_pc34(
    Dm1V1ChampionTopRowM11HostRenderStatePc34 *state,
    const Dm1V1ChampionTopRowM11LifecycleReceiptPc34 *lifecycle,
    Dm1V1ChampionTopRowM11HostRenderReceiptPc34 *outReceipt);

const char *dm1_v1_champion_top_row_m11_host_render_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
