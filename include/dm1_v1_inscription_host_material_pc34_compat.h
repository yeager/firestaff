#ifndef FIRESTAFF_DM1_V1_INSCRIPTION_HOST_MATERIAL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_INSCRIPTION_HOST_MATERIAL_PC34_COMPAT_H

#include "dm1_v1_inscription_font_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_INSCRIPTION_HOST_MATERIAL_MAX_GLYPHS_PC34 128

/* ReDMCSB DUNGEON.C F0168 + DUNVIEW.C F0107 material hand-off. The
 * decoded bytes are original TextString codes, never host text. */
typedef struct DM1_V1_InscriptionHostMaterialReceiptPc34 {
    int valid;
    int textStringIndex;
    int fontGraphicIndex;
    int transparentColor;
    int glyphByteCount;
    int lineCount;
    unsigned char glyphBytes[DM1_V1_INSCRIPTION_HOST_MATERIAL_MAX_GLYPHS_PC34];
    DM1_V1_InscriptionFrontWallLineDrawPlanPc34
        lines[DM1_V1_INSCRIPTION_MAX_LINES];
} DM1_V1_InscriptionHostMaterialReceiptPc34;

/* ReDMCSB DUNGEON.C F0168 reads TEXTSTRING from G0284 before checking
 * Visible and TextDataWordOffset. Keep the HoC renderer on that raw PC3.4
 * record: the decoded DungeonTextString mirror is not an authority here. */
int dm1_v1_inscription_get_visible_raw_text_offset_pc34(
    const struct DungeonThings_Compat* things,
    int textStringIndex,
    int* outTextDataWordOffset);

int dm1_v1_inscription_host_material_from_world_pc34(
    const struct DungeonThings_Compat* things,
    int preferredTextIndex,
    unsigned short firstThing,
    DM1_V1_InscriptionHostMaterialReceiptPc34* outReceipt);

/* F0172 has already selected G0290's current visible wall TextString.  M11
 * uses this strict form for F0107/M648 so a malformed selected record cannot
 * fall through to another visible TextString in a broader square list. */
int dm1_v1_inscription_host_material_from_selected_wall_pc34(
    const struct DungeonThings_Compat* things,
    int selectedTextIndex,
    DM1_V1_InscriptionHostMaterialReceiptPc34* outReceipt);

/* The M11 consumer must accept only the exact F0107 M648/C10 raster plan.
 * Checking the complete receipt before the first blit prevents a malformed
 * later line from leaving a partial host inscription on the wall. */
static inline int DM1_V1_InscriptionHostMaterialRasterGatePc34(
    const DM1_V1_InscriptionHostMaterialReceiptPc34* material,
    int fontWidth,
    int fontHeight)
{
    int cursor = 0;
    int line;
    int lineCount = 0;

    if (!material || !material->valid ||
        material->fontGraphicIndex != DM1_V1_INSCRIPTION_FONT_GRAPHIC_INDEX_PC34 ||
        material->transparentColor != DM1_V1_INSCRIPTION_TRANSPARENT_COLOR ||
        material->glyphByteCount <= 0 ||
        material->glyphByteCount >= DM1_V1_INSCRIPTION_HOST_MATERIAL_MAX_GLYPHS_PC34 ||
        material->glyphBytes[material->glyphByteCount] != 0x81U) {
        return 0;
    }

    for (line = 0; line < DM1_V1_INSCRIPTION_MAX_LINES; ++line) {
        DM1_V1_InscriptionFrontWallLineDrawPlanPc34 expected;
        const DM1_V1_InscriptionFrontWallLineDrawPlanPc34* actual =
            &material->lines[line];

        if (!DM1_V1_InscriptionBuildFrontWallLineDrawPlanPc34(
                material->glyphBytes, material->glyphByteCount + 1,
                cursor, line, 160, 111, &expected) ||
            memcmp(actual, &expected, sizeof(expected)) != 0) {
            return 0;
        }
        if (actual->glyphCount > 0) {
            if (!DM1_V1_InscriptionRawGlyphLineSupportedByFontPc34(
                    material->glyphBytes + actual->glyphStart,
                    actual->glyphCount, fontWidth, fontHeight)) {
                return 0;
            }
            ++lineCount;
        }
        if (actual->done) {
            return lineCount == material->lineCount;
        }
        cursor = actual->nextCursor;
    }
    return 0;
}

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_INSCRIPTION_HOST_MATERIAL_PC34_COMPAT_H */
