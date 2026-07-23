/* ReDMCSB DUNVIEW.C F0107 M648/C10 consumption gate. */

#include "dm1_v1_inscription_host_material_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int build_valid_material(DM1_V1_InscriptionHostMaterialReceiptPc34* material)
{
    int cursor = 0;
    int line;

    memset(material, 0, sizeof(*material));
    material->valid = 1;
    material->fontGraphicIndex = DM1_V1_INSCRIPTION_FONT_GRAPHIC_INDEX_PC34;
    material->transparentColor = DM1_V1_INSCRIPTION_TRANSPARENT_COLOR;
    material->glyphBytes[0] = 19;
    material->glyphBytes[1] = 4;
    material->glyphBytes[2] = 18;
    material->glyphBytes[3] = 19;
    material->glyphBytes[4] = 0x80U;
    material->glyphBytes[5] = 14;
    material->glyphBytes[6] = 10;
    material->glyphBytes[7] = 0x81U;
    material->glyphByteCount = 7;

    for (line = 0; line < DM1_V1_INSCRIPTION_MAX_LINES; ++line) {
        if (!DM1_V1_InscriptionBuildFrontWallLineDrawPlanPc34(
                material->glyphBytes, material->glyphByteCount + 1, cursor,
                line, 160, 111, &material->lines[line])) {
            return 0;
        }
        if (material->lines[line].glyphCount > 0) {
            ++material->lineCount;
        }
        if (material->lines[line].done) {
            return 1;
        }
        cursor = material->lines[line].nextCursor;
    }
    return 0;
}

static int build_all_visible_line_material(
    DM1_V1_InscriptionHostMaterialReceiptPc34* material)
{
    int cursor = 0;
    int line;

    memset(material, 0, sizeof(*material));
    material->valid = 1;
    material->fontGraphicIndex = DM1_V1_INSCRIPTION_FONT_GRAPHIC_INDEX_PC34;
    material->transparentColor = DM1_V1_INSCRIPTION_TRANSPARENT_COLOR;
    material->glyphBytes[0] = 0;
    material->glyphBytes[1] = 0x80U;
    material->glyphBytes[2] = 1;
    material->glyphBytes[3] = 0x80U;
    material->glyphBytes[4] = 2;
    material->glyphBytes[5] = 0x80U;
    material->glyphBytes[6] = 3;
    material->glyphBytes[7] = 0x81U;
    material->glyphByteCount = 7;

    for (line = 0; line < DM1_V1_INSCRIPTION_MAX_LINES; ++line) {
        if (!DM1_V1_InscriptionBuildFrontWallLineDrawPlanPc34(
                material->glyphBytes, material->glyphByteCount + 1, cursor,
                line, 160, 111, &material->lines[line])) {
            return 0;
        }
        if (material->lines[line].glyphCount > 0) {
            ++material->lineCount;
        }
        if (material->lines[line].done) {
            return material->lineCount == DM1_V1_INSCRIPTION_MAX_LINES;
        }
        cursor = material->lines[line].nextCursor;
    }
    return 0;
}

int main(void)
{
    DM1_V1_InscriptionHostMaterialReceiptPc34 material;
    DM1_V1_InscriptionHostMaterialReceiptPc34 tampered;
    DM1_V1_InscriptionHostMaterialReceiptPc34 allLines;
    unsigned int visibleLineMask = 0u;

    if (!build_valid_material(&material) ||
        !DM1_V1_InscriptionHostMaterialRasterGatePc34(
            &material, DM1_V1_INSCRIPTION_FONT_WIDTH_PC34,
            DM1_V1_INSCRIPTION_FONT_HEIGHT_PC34)) {
        fprintf(stderr, "valid F0107 M648/C10 material rejected\n");
        return 1;
    }

    tampered = material;
    tampered.fontGraphicIndex = 257;
    if (DM1_V1_InscriptionHostMaterialRasterGatePc34(
            &tampered, DM1_V1_INSCRIPTION_FONT_WIDTH_PC34,
            DM1_V1_INSCRIPTION_FONT_HEIGHT_PC34)) {
        fprintf(stderr, "non-M648 material accepted\n");
        return 1;
    }
    tampered = material;
    tampered.transparentColor = 9;
    if (DM1_V1_InscriptionHostMaterialRasterGatePc34(
            &tampered, DM1_V1_INSCRIPTION_FONT_WIDTH_PC34,
            DM1_V1_INSCRIPTION_FONT_HEIGHT_PC34)) {
        fprintf(stderr, "non-C10 transparency accepted\n");
        return 1;
    }
    tampered = material;
    ++tampered.lines[1].textX;
    if (DM1_V1_InscriptionHostMaterialRasterGatePc34(
            &tampered, DM1_V1_INSCRIPTION_FONT_WIDTH_PC34,
            DM1_V1_INSCRIPTION_FONT_HEIGHT_PC34)) {
        fprintf(stderr, "tampered later F0107 line accepted\n");
        return 1;
    }
    if (DM1_V1_InscriptionHostMaterialRasterGatePc34(
            &material, DM1_V1_INSCRIPTION_FONT_WIDTH_PC34 - 1,
            DM1_V1_INSCRIPTION_FONT_HEIGHT_PC34)) {
        fprintf(stderr, "undersized M648 raster accepted\n");
        return 1;
    }
    if (DM1_V1_InscriptionHostMaterialRasterGatePc34(
            &material, DM1_V1_INSCRIPTION_FONT_WIDTH_PC34 + 8,
            DM1_V1_INSCRIPTION_FONT_HEIGHT_PC34)) {
        fprintf(stderr, "padded/scaled M648 raster accepted\n");
        return 1;
    }
    if (!build_all_visible_line_material(&allLines) ||
        !DM1_V1_InscriptionSourceGlyphLayoutGatePc34(
            &allLines, DM1_V1_INSCRIPTION_FONT_WIDTH_PC34,
            DM1_V1_INSCRIPTION_FONT_HEIGHT_PC34, &visibleLineMask) ||
        visibleLineMask != 0x0fu ||
        allLines.lines[0].textY != 41 ||
        allLines.lines[1].textY != 52 ||
        allLines.lines[2].textY != 68 ||
        allLines.lines[3].textY != 79) {
        fprintf(stderr, "F0168 did not preserve all four F0107 line slots\n");
        return 1;
    }
    tampered = allLines;
    ++tampered.lines[2].textY;
    if (DM1_V1_InscriptionSourceGlyphLayoutGatePc34(
            &tampered, DM1_V1_INSCRIPTION_FONT_WIDTH_PC34,
            DM1_V1_INSCRIPTION_FONT_HEIGHT_PC34, 0)) {
        fprintf(stderr, "tampered F0107 line-slot position accepted\n");
        return 1;
    }
    {
        DM1_V1_InscriptionRasterCellBindingPc34 binding;
        if (!DM1_V1_InscriptionBuildRasterCellBindingPc34(&material, 0, 0,
                                                           &binding) ||
            binding.fontGraphicIndex !=
                DM1_V1_INSCRIPTION_FONT_GRAPHIC_INDEX_PC34 ||
            binding.transparentColor != DM1_V1_INSCRIPTION_TRANSPARENT_COLOR ||
            binding.sourceX != 19 * DM1_V1_INSCRIPTION_GLYPH_WIDTH ||
            binding.sourceY != 0 || binding.sourceWidth != 8 ||
            binding.sourceHeight != 8 ||
            binding.destinationX != material.lines[0].textX ||
            binding.destinationY != material.lines[0].textY) {
            fprintf(stderr, "M648 byte<<3 unscaled raster binding rejected\n");
            return 1;
        }
    }

    puts("ok: F0168/F0107 binds source glyphs to all M648 line slots and C10 cells");
    return 0;
}
