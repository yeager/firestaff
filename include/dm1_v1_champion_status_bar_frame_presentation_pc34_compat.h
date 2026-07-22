#ifndef FIRESTAFF_DM1_V1_CHAMPION_STATUS_BAR_FRAME_PRESENTATION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_STATUS_BAR_FRAME_PRESENTATION_PC34_COMPAT_H

#include "dm1_v1_champion_status_bar_redraw_commands_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Dm1V1ChampionStatusBarFrameMaterialsPc34 {
    int originalPaletteReady;
    const uint8_t *originalPalette;
    int originalPaletteEntryCount;
    int originalSurfaceReady;
    uint8_t *originalIndexedSurface;
    int surfaceWidth;
    int surfaceHeight;
    int surfacePitch;
} Dm1V1ChampionStatusBarFrameMaterialsPc34;

typedef struct Dm1V1ChampionStatusBarFrameOpPc34 {
    Dm1V1ChampionStatusBarRedrawOperationPc34 operation;
    Dm1V1ChampionPortraitStatusRedrawRoutePc34 route;
    int championIndex;
    int statIndex;
    int zoneId;
    int x;
    int y;
    int width;
    int height;
    uint8_t colorIndex;
    const uint8_t *originalPalette;
    uint8_t *originalIndexedSurface;
    int surfacePitch;
} Dm1V1ChampionStatusBarFrameOpPc34;

typedef struct Dm1V1ChampionStatusBarFramePresentationReceiptPc34 {
    int valid;
    int atomicPublish;
    int clearCount;
    int repaintCount;
    int operationCount;
    Dm1V1ChampionStatusBarFrameOpPc34
        operations[DM1_V1_CHAMPION_STATUS_BAR_REDRAW_MAX_OPS_PC34];
} Dm1V1ChampionStatusBarFramePresentationReceiptPc34;

/* Atomically binds a command sequence to one retained original indexed
 * surface and its matching original palette. A missing material, palette
 * mismatch, or out-of-bounds command returns no partial frame receipt. */
int dm1_v1_champion_status_bar_frame_presentation_pc34(
    const Dm1V1ChampionStatusBarRedrawCommandSequencePc34 *commands,
    const Dm1V1ChampionStatusBarFrameMaterialsPc34 *materials,
    Dm1V1ChampionStatusBarFramePresentationReceiptPc34 *outReceipt);

const char *dm1_v1_champion_status_bar_frame_presentation_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
