#ifndef FIRESTAFF_DM1_V1_CHAMPION_STATUS_BAR_REDRAW_COMMANDS_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_STATUS_BAR_REDRAW_COMMANDS_PC34_COMPAT_H

#include "dm1_v1_champion_status_bar_redraw_receipt_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Dm1V1ChampionStatusBarRedrawMaterialsPc34 {
    int statusTargetReady;
    int indexedPaletteOriginal;
    const uint8_t *indexedPalette;
    int indexedPaletteEntryCount;
} Dm1V1ChampionStatusBarRedrawMaterialsPc34;

typedef struct Dm1V1ChampionStatusBarRedrawCommandPc34 {
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
} Dm1V1ChampionStatusBarRedrawCommandPc34;

typedef struct Dm1V1ChampionStatusBarRedrawCommandSequencePc34 {
    int valid;
    int commandCount;
    Dm1V1ChampionStatusBarRedrawCommandPc34
        commands[DM1_V1_CHAMPION_STATUS_BAR_REDRAW_MAX_OPS_PC34];
} Dm1V1ChampionStatusBarRedrawCommandSequencePc34;

/* Converts a validated source receipt into renderer-neutral commands without
 * reordering it. Status bars use indexed colors, so the target and original
 * indexed palette must both be explicitly admitted before any command exists. */
int dm1_v1_champion_status_bar_redraw_commands_pc34(
    const Dm1V1ChampionStatusBarRedrawReceiptPc34 *receipt,
    const Dm1V1ChampionStatusBarRedrawMaterialsPc34 *materials,
    Dm1V1ChampionStatusBarRedrawCommandSequencePc34 *outSequence);

const char *dm1_v1_champion_status_bar_redraw_commands_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
