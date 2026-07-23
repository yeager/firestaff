#ifndef FIRESTAFF_DM1_V1_INSCRIPTION_HOST_MATERIAL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_INSCRIPTION_HOST_MATERIAL_PC34_COMPAT_H

#include "dm1_v1_inscription_font_pc34_compat.h"

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_INSCRIPTION_HOST_MATERIAL_MAX_GLYPHS_PC34 128

struct DungeonThings_Compat;

typedef DM1_V1_InscriptionLinePlanPc34
    DM1_V1_InscriptionFrontWallLineDrawPlanPc34;

static inline int DM1_V1_InscriptionBuildFrontWallLineDrawPlanPc34(
    const unsigned char* glyphs,
    int glyphCapacity,
    int cursor,
    int line,
    int centerX,
    int centerY,
    DM1_V1_InscriptionFrontWallLineDrawPlanPc34* outPlan)
{
    (void)centerX;
    (void)centerY;
    return DM1_V1_InscriptionLinePlanFromRawGlyphsPc34(
        glyphs, glyphCapacity, cursor, line, outPlan);
}

/* ReDMCSB DUNGEON.C F0168 + DUNVIEW.C F0107 material hand-off. The
 * decoded bytes are original TextString codes, never host text. */
typedef struct DM1_V1_InscriptionHostMaterialReceiptPc34 {
    int valid;
    int textStringIndex;
    /* Exact packed PC34 TEXTSTRING span selected by F0168 before its
     * 5-bit decode.  The FNV is over little-endian source word bytes. */
    int textDataWordOffset;
    int textDataWordCount;
    uint32_t textDataFNV1a;
    int fontGraphicIndex;
    int transparentColor;
    int glyphByteCount;
    uint32_t glyphBytesFNV1a;
    int lineCount;
    unsigned char glyphBytes[DM1_V1_INSCRIPTION_HOST_MATERIAL_MAX_GLYPHS_PC34];
    DM1_V1_InscriptionFrontWallLineDrawPlanPc34
        lines[DM1_V1_INSCRIPTION_MAX_LINES];
} DM1_V1_InscriptionHostMaterialReceiptPc34;

/* One unscaled F0107 M648 source cell.  ReDMCSB selects each glyph at
 * `decodedByte << 3` and copies its native 8x8 indexed pixels directly to
 * the front-wall line.  Keeping this derivable binding explicit lets every
 * caller prove that neither a host font nor a scaled replacement entered the
 * wall route. */
typedef struct DM1_V1_InscriptionRasterCellBindingPc34 {
    int fontGraphicIndex;
    int transparentColor;
    int sourceX;
    int sourceY;
    int sourceWidth;
    int sourceHeight;
    int destinationX;
    int destinationY;
} DM1_V1_InscriptionRasterCellBindingPc34;

static inline int DM1_V1_InscriptionBuildRasterCellBindingPc34(
    const DM1_V1_InscriptionHostMaterialReceiptPc34* material,
    int lineIndex,
    int glyphOffset,
    DM1_V1_InscriptionRasterCellBindingPc34* outBinding)
{
    const DM1_V1_InscriptionFrontWallLineDrawPlanPc34* line;
    int glyph;

    if (!material || !outBinding || lineIndex < 0 ||
        lineIndex >= DM1_V1_INSCRIPTION_MAX_LINES || glyphOffset < 0) {
        return 0;
    }
    line = &material->lines[lineIndex];
    if (line->glyphCount <= 0 || glyphOffset >= line->glyphCount ||
        line->glyphStart < 0 ||
        line->glyphStart + glyphOffset >= material->glyphByteCount) {
        return 0;
    }
    glyph = DM1_V1_InscriptionGlyphIndexFromSourceByte(
        material->glyphBytes[line->glyphStart + glyphOffset]);
    if (glyph < 0 || (glyph + 1) * DM1_V1_INSCRIPTION_GLYPH_WIDTH >
                         DM1_V1_INSCRIPTION_FONT_WIDTH_PC34) {
        return 0;
    }
    outBinding->fontGraphicIndex = material->fontGraphicIndex;
    outBinding->transparentColor = material->transparentColor;
    outBinding->sourceX = glyph * DM1_V1_INSCRIPTION_GLYPH_WIDTH;
    outBinding->sourceY = 0;
    outBinding->sourceWidth = DM1_V1_INSCRIPTION_GLYPH_WIDTH;
    outBinding->sourceHeight = DM1_V1_INSCRIPTION_GLYPH_HEIGHT;
    outBinding->destinationX = line->textX +
        glyphOffset * DM1_V1_INSCRIPTION_GLYPH_WIDTH;
    outBinding->destinationY = line->textY;
    return 1;
}

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

/* Validate the complete F0168 -> F0107 line layout before a caller can
 * consume any M648 cells. `outVisibleLineMask` has one bit for every
 * non-empty source line (bit 0..3); it intentionally tracks original line
 * slots, so an empty source line cannot collapse later text upward. */
static inline int DM1_V1_InscriptionSourceGlyphLayoutGatePc34(
    const DM1_V1_InscriptionHostMaterialReceiptPc34* material,
    int fontWidth,
    int fontHeight,
    unsigned int* outVisibleLineMask)
{
    int cursor = 0;
    int line;
    int lineCount = 0;
    unsigned int visibleLineMask = 0u;

    if (outVisibleLineMask) {
        *outVisibleLineMask = 0u;
    }
    if (!material || !material->valid ||
        material->fontGraphicIndex != DM1_V1_INSCRIPTION_FONT_GRAPHIC_INDEX_PC34 ||
        material->transparentColor != DM1_V1_INSCRIPTION_TRANSPARENT_COLOR ||
        material->glyphByteCount <= 0 ||
        material->glyphByteCount >= DM1_V1_INSCRIPTION_HOST_MATERIAL_MAX_GLYPHS_PC34 ||
        fontWidth != DM1_V1_INSCRIPTION_FONT_WIDTH_PC34 ||
        fontHeight != DM1_V1_INSCRIPTION_FONT_HEIGHT_PC34 ||
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
            memcmp(actual, &expected, sizeof(expected)) != 0 ||
            actual->textY != DM1_V1_InscriptionFrontWallLineTextYPc34(line)) {
            return 0;
        }
        if (actual->glyphCount > 0) {
            if (!DM1_V1_InscriptionRawGlyphLineSupportedByFontPc34(
                    material->glyphBytes + actual->glyphStart,
                    actual->glyphCount, fontWidth, fontHeight)) {
                return 0;
            }
            visibleLineMask |= 1u << (unsigned int)line;
            ++lineCount;
        }
        if (actual->done) {
            if (lineCount != material->lineCount) {
                return 0;
            }
            if (outVisibleLineMask) {
                *outVisibleLineMask = visibleLineMask;
            }
            return visibleLineMask != 0u;
        }
        cursor = actual->nextCursor;
    }
    return 0;
}

/* The M11 consumer must accept only the exact F0107 M648/C10 raster plan.
 * Checking the complete receipt before the first blit prevents a malformed
 * later line from leaving a partial host inscription on the wall. */
static inline int DM1_V1_InscriptionHostMaterialRasterGatePc34(
    const DM1_V1_InscriptionHostMaterialReceiptPc34* material,
    int fontWidth,
    int fontHeight)
{
    int line;

    if (!DM1_V1_InscriptionSourceGlyphLayoutGatePc34(
            material, fontWidth, fontHeight, 0)) {
        return 0;
    }

    for (line = 0; line < DM1_V1_INSCRIPTION_MAX_LINES; ++line) {
        const DM1_V1_InscriptionFrontWallLineDrawPlanPc34* actual =
            &material->lines[line];

        if (actual->glyphCount > 0) {
            int glyphOffset;
            for (glyphOffset = 0; glyphOffset < actual->glyphCount;
                 ++glyphOffset) {
                DM1_V1_InscriptionRasterCellBindingPc34 binding;
                if (!DM1_V1_InscriptionBuildRasterCellBindingPc34(
                        material, line, glyphOffset, &binding) ||
                    binding.fontGraphicIndex != material->fontGraphicIndex ||
                    binding.transparentColor != material->transparentColor ||
                    binding.sourceWidth != DM1_V1_INSCRIPTION_GLYPH_WIDTH ||
                    binding.sourceHeight != DM1_V1_INSCRIPTION_GLYPH_HEIGHT ||
                    binding.destinationY != actual->textY ||
                    binding.destinationX != actual->textX +
                        glyphOffset * DM1_V1_INSCRIPTION_GLYPH_WIDTH) {
                    return 0;
                }
            }
        }
        if (actual->done) {
            return 1;
        }
    }
    return 0;
}

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_INSCRIPTION_HOST_MATERIAL_PC34_COMPAT_H */
