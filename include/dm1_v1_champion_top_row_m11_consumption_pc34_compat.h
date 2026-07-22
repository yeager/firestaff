#ifndef FIRESTAFF_DM1_V1_CHAMPION_TOP_ROW_M11_CONSUMPTION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_TOP_ROW_M11_CONSUMPTION_PC34_COMPAT_H

#include "dm1_v1_champion_top_row_host_lifecycle_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Retained original material proof for a later M11 consumer. This module does
 * not invoke or modify M11; it establishes the source-only admission boundary. */
typedef struct Dm1V1ChampionTopRowM11OriginalMaterialsPc34 {
    int c008Original;
    const uint8_t *c008Pixels;
    int c008GraphicIndex;
    int c008Width;
    int c008Height;
    int c028Original;
    const uint8_t *c028Pixels;
    int c028GraphicIndex;
    int c028Width;
    int c028Height;
    int indexedPaletteOriginal;
    const uint8_t *indexedPalette;
    int indexedPaletteEntryCount;
    int indexedSurfaceOriginal;
    uint8_t *indexedSurface;
} Dm1V1ChampionTopRowM11OriginalMaterialsPc34;

typedef struct Dm1V1ChampionTopRowM11ConsumptionStatePc34 {
    unsigned int lastTick;
    unsigned int pendingClearGeneration;
    unsigned int lastCompositionGeneration;
} Dm1V1ChampionTopRowM11ConsumptionStatePc34;

typedef struct Dm1V1ChampionTopRowM11ConsumptionReceiptPc34 {
    int valid;
    unsigned int tick;
    Dm1V1ChampionTopRowAtomicLifecyclePublicationPc34 publication;
    unsigned int generation;
    Dm1V1ChampionTopRowM11OriginalMaterialsPc34 originalMaterials;
    int operationCount;
    Dm1V1ChampionTopRowHostConsumptionOpPc34
        operations[DM1_V1_CHAMPION_TOP_ROW_ATOMIC_FRAME_MAX_OPS_PC34];
} Dm1V1ChampionTopRowM11ConsumptionReceiptPc34;

void dm1_v1_champion_top_row_m11_consumption_init_pc34(
    Dm1V1ChampionTopRowM11ConsumptionStatePc34 *state);

/* Admits one active host-lifecycle receipt for a future M11 consumer. A full
 * composition must follow the matching clear generation and prove retained
 * C008, C028, indexed palette, and indexed surface material. */
int dm1_v1_champion_top_row_m11_consumption_receipt_pc34(
    Dm1V1ChampionTopRowM11ConsumptionStatePc34 *state,
    const Dm1V1ChampionTopRowHostLifecycleReceiptPc34 *hostLifecycle,
    const Dm1V1ChampionTopRowM11OriginalMaterialsPc34 *originalMaterials,
    Dm1V1ChampionTopRowM11ConsumptionReceiptPc34 *outReceipt);

const char *dm1_v1_champion_top_row_m11_consumption_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
