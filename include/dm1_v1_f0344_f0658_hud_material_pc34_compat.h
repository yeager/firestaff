#ifndef FIRESTAFF_DM1_V1_F0344_F0658_HUD_MATERIAL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0344_F0658_HUD_MATERIAL_PC34_COMPAT_H

/* ReDMCSB PANEL.C F0344/F0345 and BASE.C F0658 source-only HUD material
 * receipt. This is a DM1 asset boundary, not an M11 presentation adapter. */

#include <stdint.h>

enum {
    DM1_V1_HUD_PC34_C00_BLACK = 0,
    DM1_V1_HUD_PC34_C04_CYAN = 4,
    DM1_V1_HUD_PC34_C12_DARKEST_GRAY = 12,
    DM1_V1_HUD_PC34_M653_BYTES = 768,
    DM1_V1_HUD_PC34_OPERATION_MAX = 10
};

typedef struct DM1_V1_HudSourceSurfacePc34 {
    int graphicsDatOwned;
    int graphicIndex;
    int width;
    int height;
    int indexedPixelCount;
    const unsigned char* indexedPixels;
    uint32_t pixelsFNV1a;
} DM1_V1_HudSourceSurfacePc34;

typedef struct DM1_V1_HudGlyphSourcePc34 {
    int graphicsDatOwned;
    int graphicIndex; /* PC34 M653=695, legacy original=557 */
    const unsigned char* bits;
    int byteCount;
    uint32_t bitsFNV1a;
} DM1_V1_HudGlyphSourcePc34;

typedef enum DM1_V1_HudMaterialOperationKindPc34 {
    DM1_V1_HUD_OPERATION_ACTION_ONE_ROW_PC34 = 1,
    DM1_V1_HUD_OPERATION_ACTION_TWO_ROWS_PC34,
    DM1_V1_HUD_OPERATION_ACTION_THREE_ROWS_PC34,
    DM1_V1_HUD_OPERATION_SPELL_BACKGROUND_PC34,
    DM1_V1_HUD_OPERATION_SPELL_AVAILABLE_ROW_PC34,
    DM1_V1_HUD_OPERATION_SPELL_SELECTED_ROW_PC34,
    DM1_V1_HUD_OPERATION_PANEL_BACKGROUND_PC34,
    DM1_V1_HUD_OPERATION_FOOD_LABEL_PC34,
    DM1_V1_HUD_OPERATION_WATER_LABEL_PC34,
    DM1_V1_HUD_OPERATION_POISON_LABEL_PC34
} DM1_V1_HudMaterialOperationKindPc34;

typedef struct DM1_V1_HudMaterialOperationPc34 {
    int kind;
    int graphicIndex;
    int zoneIndex;
    int sourceX;
    int sourceY;
    int sourceW;
    int sourceH;
    int paletteForeground;
    int paletteBackground;
    int transparentColor;
    int sourceLine;
} DM1_V1_HudMaterialOperationPc34;

typedef struct DM1_V1_F0344F0658HudMaterialReceiptPc34 {
    int valid;
    int suppressSyntheticFallback;
    int m653GraphicIndex;
    uint32_t m653BitsFNV1a;
    int operationCount;
    DM1_V1_HudMaterialOperationPc34
        operations[DM1_V1_HUD_PC34_OPERATION_MAX];
    uint32_t materialFingerprint;
} DM1_V1_F0344F0658HudMaterialReceiptPc34;

uint32_t dm1_v1_f0344_f0658_hud_material_fnv1a_pc34(
    const unsigned char* bytes, int byteCount);

/* Requires the complete C010/C009/C011/C020/C030/C031/C032 set and an
 * authentic M653 bitplane. Any unavailable, foreign, altered, or malformed
 * source returns no receipt; no generated panel, font, palette, or crop is
 * exposed. C077/C079 are retained as their original F0387 zone identities. */
int dm1_v1_f0344_f0658_hud_material_receipt_pc34(
    const DM1_V1_HudSourceSurfacePc34* surfaces,
    int surfaceCount,
    const DM1_V1_HudGlyphSourcePc34* glyphs,
    int glyphCount,
    DM1_V1_F0344F0658HudMaterialReceiptPc34* outReceipt);

#endif /* FIRESTAFF_DM1_V1_F0344_F0658_HUD_MATERIAL_PC34_COMPAT_H */
