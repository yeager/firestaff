#include "dm1_v1_champion_panel_material_pc34_compat.h"

#include <string.h>

typedef struct RequiredSurface {
    int graphic;
    int width;
    int height;
} RequiredSurface;

static const RequiredSurface kRequired[] = {
    { 8, 67, 29 },   /* F0292 dead status C008 */
    { 17, 224, 136 },/* F0296/F0302 inventory C017 */
    { 26, 256, 87 }, /* original 8x3 portrait atlas C026 */
    { 28, 76, 14 },  /* F0292/F0622 status icons C028 */
    { 32, 96, 15 },  /* F0292 poison C032 */
    { 15, 45, 7 },   /* F0292 damage C015 */
    { 16, 32, 29 },  /* F0296 inventory damage C016 */
    { 33, 18, 18 }, { 34, 18, 18 }, { 35, 18, 18 } /* F0291 */
};

uint32_t dm1_v1_champion_panel_material_fnv1a_pc34(
    const unsigned char *bytes, int byteCount)
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

int dm1_v1_champion_panel_material_from_m11_loader_pc34(
    M11_AssetLoader *loader, const M11_FontState *font,
    const unsigned char *palette, int paletteEntryCount,
    Dm1V1ChampionPanelMaterialReceiptPc34 *outReceipt)
{
    Dm1V1ChampionPanelMaterialReceiptPc34 receipt;
    uint32_t fingerprint = 2166136261u;
    int i;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!loader || !M11_AssetLoader_IsReady(loader) || !font ||
        !M11_Font_IsLoaded(font) ||
        (M11_Font_ResolvedGraphicIndex(font) != M11_FONT_GRAPHIC_INDEX_PC34 &&
         M11_Font_ResolvedGraphicIndex(font) != M11_FONT_GRAPHIC_INDEX_LEGACY) ||
        !palette || paletteEntryCount != 16) return 0;

    for (i = 0; i < (int)(sizeof(kRequired) / sizeof(kRequired[0])); ++i) {
        const M11_AssetSlot *slot = M11_AssetLoader_Load(loader, kRequired[i].graphic);
        uint32_t hash;
        if (!slot || !slot->loaded || !slot->pixels ||
            (int)slot->graphicIndex != kRequired[i].graphic ||
            (int)slot->width != kRequired[i].width ||
            (int)slot->height != kRequired[i].height) return 0;
        hash = dm1_v1_champion_panel_material_fnv1a_pc34(
            slot->pixels, (int)slot->width * (int)slot->height);
        if (!hash) return 0;
        fingerprint ^= hash; fingerprint *= 16777619u;
    }
    fingerprint ^= dm1_v1_champion_panel_material_fnv1a_pc34(font->bitmap,
                                                               M11_FONT_BITMAP_BYTES);
    fingerprint *= 16777619u;
    fingerprint ^= dm1_v1_champion_panel_material_fnv1a_pc34(palette, 16 * 3);
    fingerprint *= 16777619u;
    if (!fingerprint) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    receipt.suppressSyntheticFallback = 1;
    receipt.m653GraphicIndex = M11_Font_ResolvedGraphicIndex(font);
    receipt.materialFingerprint = fingerprint;
    *outReceipt = receipt;
    return 1;
}
