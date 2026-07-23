#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_MATERIAL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_MATERIAL_PC34_COMPAT_H

#include "asset_loader_m11.h"
#include "font_m11.h"

#include <stdint.h>

/* One source-only admission for CHAMDRAW.C F0292/F0293/F0296 and
 * CHAMPION.C F0302 panel material.  It exposes no generated art. */
typedef struct Dm1V1ChampionPanelMaterialReceiptPc34 {
    int valid;
    int suppressSyntheticFallback;
    int m653GraphicIndex;
    uint32_t materialFingerprint;
} Dm1V1ChampionPanelMaterialReceiptPc34;

uint32_t dm1_v1_champion_panel_material_fnv1a_pc34(
    const unsigned char *bytes, int byteCount);

int dm1_v1_champion_panel_material_from_m11_loader_pc34(
    M11_AssetLoader *loader,
    const M11_FontState *font,
    const unsigned char *palette,
    int paletteEntryCount,
    Dm1V1ChampionPanelMaterialReceiptPc34 *outReceipt);

#endif
