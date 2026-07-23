#include "dm1_v1_f0344_f0658_hud_material_pc34_compat.h"

#include <string.h>

uint32_t dm1_v1_f0344_f0658_hud_material_fnv1a_pc34(
    const unsigned char* bytes, int byteCount)
{
    uint32_t hash = 2166136261u;
    int i;
    if (!bytes || byteCount <= 0) return 0u;
    for (i = 0; i < byteCount; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

static const DM1_V1_HudSourceSurfacePc34* find_surface(
    const DM1_V1_HudSourceSurfacePc34* surfaces, int surfaceCount,
    int graphicIndex, int width, int height, uint32_t* outHash)
{
    int i;
    if (outHash) *outHash = 0u;
    if (!surfaces || surfaceCount <= 0) return 0;
    for (i = 0; i < surfaceCount; ++i) {
        const DM1_V1_HudSourceSurfacePc34* surface = &surfaces[i];
        uint32_t hash;
        if (!surface->graphicsDatOwned || surface->graphicIndex != graphicIndex ||
            surface->width != width || surface->height != height ||
            !surface->indexedPixels ||
            surface->indexedPixelCount < width * height) continue;
        hash = dm1_v1_f0344_f0658_hud_material_fnv1a_pc34(
            surface->indexedPixels, surface->indexedPixelCount);
        if (!hash || hash != surface->pixelsFNV1a) continue;
        if (outHash) *outHash = hash;
        return surface;
    }
    return 0;
}

static const DM1_V1_HudGlyphSourcePc34* find_m653(
    const DM1_V1_HudGlyphSourcePc34* glyphs, int glyphCount, uint32_t* outHash)
{
    int i;
    if (outHash) *outHash = 0u;
    if (!glyphs || glyphCount <= 0) return 0;
    for (i = 0; i < glyphCount; ++i) {
        const DM1_V1_HudGlyphSourcePc34* glyph = &glyphs[i];
        uint32_t hash;
        if (!glyph->graphicsDatOwned ||
            (glyph->graphicIndex != 695 && glyph->graphicIndex != 557) ||
            !glyph->bits || glyph->byteCount != DM1_V1_HUD_PC34_M653_BYTES) continue;
        hash = dm1_v1_f0344_f0658_hud_material_fnv1a_pc34(
            glyph->bits, glyph->byteCount);
        if (!hash || hash != glyph->bitsFNV1a) continue;
        if (outHash) *outHash = hash;
        return glyph;
    }
    return 0;
}

static void add_operation(DM1_V1_F0344F0658HudMaterialReceiptPc34* receipt,
                          int kind, int graphic, int zone, int x, int y,
                          int w, int h, int fg, int bg, int transparent,
                          int sourceLine)
{
    DM1_V1_HudMaterialOperationPc34* operation =
        &receipt->operations[receipt->operationCount++];
    operation->kind = kind;
    operation->graphicIndex = graphic;
    operation->zoneIndex = zone;
    operation->sourceX = x;
    operation->sourceY = y;
    operation->sourceW = w;
    operation->sourceH = h;
    operation->paletteForeground = fg;
    operation->paletteBackground = bg;
    operation->transparentColor = transparent;
    operation->sourceLine = sourceLine;
}

int dm1_v1_f0344_f0658_hud_material_receipt_pc34(
    const DM1_V1_HudSourceSurfacePc34* surfaces,
    int surfaceCount,
    const DM1_V1_HudGlyphSourcePc34* glyphs,
    int glyphCount,
    DM1_V1_F0344F0658HudMaterialReceiptPc34* outReceipt)
{
    DM1_V1_F0344F0658HudMaterialReceiptPc34 receipt;
    const DM1_V1_HudGlyphSourcePc34* glyph;
    uint32_t hashes[8];
    uint32_t fingerprint = 2166136261u;
    int i;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    /* C010 backs F0387's C079/C077/C011 crops; C009/C011 back F0394;
     * C020/C030..C032 are the F0344/F0658 panel transaction. */
    if (!find_surface(surfaces, surfaceCount, 10, 87, 45, &hashes[0]) ||
        !find_surface(surfaces, surfaceCount, 9, 87, 25, &hashes[1]) ||
        !find_surface(surfaces, surfaceCount, 11, 14, 39, &hashes[2]) ||
        !find_surface(surfaces, surfaceCount, 20, 144, 73, &hashes[3]) ||
        !find_surface(surfaces, surfaceCount, 30, 34, 9, &hashes[4]) ||
        !find_surface(surfaces, surfaceCount, 31, 46, 9, &hashes[5]) ||
        !find_surface(surfaces, surfaceCount, 32, 96, 15, &hashes[6]) ||
        !(glyph = find_m653(glyphs, glyphCount, &hashes[7]))) return 0;

    memset(&receipt, 0, sizeof(receipt));
    receipt.suppressSyntheticFallback = 1;
    receipt.m653GraphicIndex = glyph->graphicIndex;
    receipt.m653BitsFNV1a = hashes[7];
    /* ACTIDRAW.C F0387: C079/C077/C011 select progressively taller crops
     * from C010. The zone number is source ownership, not another bitmap. */
    add_operation(&receipt, DM1_V1_HUD_OPERATION_ACTION_ONE_ROW_PC34,
                  10, 79, 0, 0, 87, 21, 4, 0, -1, 348);
    add_operation(&receipt, DM1_V1_HUD_OPERATION_ACTION_TWO_ROWS_PC34,
                  10, 77, 0, 0, 87, 33, 4, 0, -1, 348);
    add_operation(&receipt, DM1_V1_HUD_OPERATION_ACTION_THREE_ROWS_PC34,
                  10, 11, 0, 0, 87, 45, 4, 0, -1, 348);
    /* CASTER.C F0394 + MENUDRAW.C F0396: C011 rows 13 and 26. */
    add_operation(&receipt, DM1_V1_HUD_OPERATION_SPELL_BACKGROUND_PC34,
                  9, 13, 0, 0, 87, 25, 4, 0, -1, 31);
    add_operation(&receipt, DM1_V1_HUD_OPERATION_SPELL_AVAILABLE_ROW_PC34,
                  11, 245, 0, 13, 14, 12, 4, 0, -1, 80);
    add_operation(&receipt, DM1_V1_HUD_OPERATION_SPELL_SELECTED_ROW_PC34,
                  11, 261, 0, 26, 14, 12, 4, 0, -1, 96);
    add_operation(&receipt, DM1_V1_HUD_OPERATION_PANEL_BACKGROUND_PC34,
                  20, 101, 0, 0, 144, 73, 4, 0, -1, 1582);
    add_operation(&receipt, DM1_V1_HUD_OPERATION_FOOD_LABEL_PC34,
                  30, 500, 0, 0, 34, 9, 0, 0, 12, 1598);
    add_operation(&receipt, DM1_V1_HUD_OPERATION_WATER_LABEL_PC34,
                  31, 501, 0, 0, 46, 9, 0, 0, 12, 1599);
    /* C032 uses the same F0658 material rule but is conditional on poison. */
    if (receipt.operationCount >= DM1_V1_HUD_PC34_OPERATION_MAX) return 0;
    add_operation(&receipt, DM1_V1_HUD_OPERATION_POISON_LABEL_PC34,
                  32, 502, 0, 0, 96, 15, 0, 0, 12, 1606);

    for (i = 0; i < 8; ++i) {
        fingerprint ^= hashes[i];
        fingerprint *= 16777619u;
    }
    if (!fingerprint || receipt.operationCount != DM1_V1_HUD_PC34_OPERATION_MAX) {
        return 0;
    }
    receipt.materialFingerprint = fingerprint;
    receipt.valid = 1;
    *outReceipt = receipt;
    return 1;
}
