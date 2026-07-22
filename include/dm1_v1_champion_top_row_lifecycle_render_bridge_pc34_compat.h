#ifndef FIRESTAFF_DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_BRIDGE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_BRIDGE_PC34_COMPAT_H

#include "dm1_v1_champion_top_row_atomic_lifecycle_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Dm1V1ChampionTopRowLifecycleRenderBridgeStatePc34 {
    unsigned int lastClearGeneration;
    unsigned int lastCompositionGeneration;
    int clearPublishedSinceComposition;
} Dm1V1ChampionTopRowLifecycleRenderBridgeStatePc34;

typedef enum Dm1V1ChampionTopRowLifecycleRenderCommandKindPc34 {
    DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_CLEAR_PC34 = 1,
    DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_BLIT_C008_PC34,
    DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_COMPOSE_C028_PC34,
    DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_STATUS_BAR_PC34
} Dm1V1ChampionTopRowLifecycleRenderCommandKindPc34;

typedef struct Dm1V1ChampionTopRowLifecycleRenderCommandPc34 {
    Dm1V1ChampionTopRowLifecycleRenderCommandKindPc34 kind;
    Dm1V1ChampionTopRowAtomicFrameOpPc34 source;
} Dm1V1ChampionTopRowLifecycleRenderCommandPc34;

typedef struct Dm1V1ChampionTopRowLifecycleRenderReceiptPc34 {
    int valid;
    Dm1V1ChampionTopRowAtomicLifecyclePublicationPc34 publication;
    unsigned int generation;
    int commandCount;
    Dm1V1ChampionTopRowLifecycleRenderCommandPc34
        commands[DM1_V1_CHAMPION_TOP_ROW_ATOMIC_FRAME_MAX_OPS_PC34];
} Dm1V1ChampionTopRowLifecycleRenderReceiptPc34;

void dm1_v1_champion_top_row_lifecycle_render_bridge_init_pc34(
    Dm1V1ChampionTopRowLifecycleRenderBridgeStatePc34 *state);

/* Converts one lifecycle receipt into render commands while retaining the
 * clear-before-composition invariant independently of the caller. */
int dm1_v1_champion_top_row_lifecycle_render_bridge_pc34(
    Dm1V1ChampionTopRowLifecycleRenderBridgeStatePc34 *state,
    const Dm1V1ChampionTopRowAtomicLifecycleReceiptPc34 *lifecycle,
    Dm1V1ChampionTopRowLifecycleRenderReceiptPc34 *outReceipt);

const char *dm1_v1_champion_top_row_lifecycle_render_bridge_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
