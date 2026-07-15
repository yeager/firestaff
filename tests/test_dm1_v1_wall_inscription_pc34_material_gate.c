/*
 * Real PC34 wall-inscription material regression, without an M11 renderer.
 *
 * ReDMCSB DUNGEON.C F0168 decodes the wall TextString; DUNVIEW.C
 * F0107:3619-3706 selects M648 and blits its source cells with C10
 * transparency.  Use the installed PC34 DUNGEON.DAT rather than a fixture,
 * then prove that the DM1 receipt retains only that source material.  A
 * missing or undersized M648 texture is not drawable.
 */

#include "dm1_v1_inscription_host_material_pc34_compat.h"
#include "dm1_v1_wall_inscription_presentation_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int regular_file_has_bytes(const char* path)
{
    FILE* file;
    long size;
    if (!path) {
        return 0;
    }
    file = fopen(path, "rb");
    if (!file) {
        return 0;
    }
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return 0;
    }
    size = ftell(file);
    fclose(file);
    return size > 0;
}

static int square_is_wall(const struct DungeonDatState_Compat* dungeon,
                          int mapIndex, int x, int y)
{
    const struct DungeonMapDesc_Compat* map;
    int squareIndex;
    if (!dungeon || !dungeon->tiles || mapIndex < 0 ||
        mapIndex >= (int)dungeon->header.mapCount) {
        return 0;
    }
    map = &dungeon->maps[mapIndex];
    if (x < 0 || y < 0 || x >= (int)map->width || y >= (int)map->height) {
        return 0;
    }
    squareIndex = x * (int)map->height + y;
    return (dungeon->tiles[mapIndex].squareData[squareIndex] &
            DUNGEON_SQUARE_MASK_TYPE) == 0;
}

static int find_visible_wall_text(const struct DungeonDatState_Compat* dungeon,
                                  const struct DungeonThings_Compat* things,
                                  int* outTextIndex,
                                  unsigned short* outFirstThing)
{
    int mapIndex;
    if (!dungeon || !things || !outTextIndex || !outFirstThing) {
        return 0;
    }
    for (mapIndex = 0; mapIndex < (int)dungeon->header.mapCount; ++mapIndex) {
        const struct DungeonMapDesc_Compat* map = &dungeon->maps[mapIndex];
        int y;
        for (y = 0; y < (int)map->height; ++y) {
            int x;
            for (x = 0; x < (int)map->width; ++x) {
                unsigned short firstThing;
                unsigned short thing;
                int safety = 0;
                if (!square_is_wall(dungeon, mapIndex, x, y)) {
                    continue;
                }
                firstThing = F0511_DUNGEON_GetSquareFirstThing_Compat(
                    dungeon, things, mapIndex, x, y);
                thing = firstThing;
                while (thing != THING_ENDOFLIST && thing != THING_NONE &&
                       safety++ < 64) {
                    if (THING_GET_TYPE(thing) == THING_TYPE_TEXTSTRING) {
                        int textIndex = (int)THING_GET_INDEX(thing);
                        int textDataWordOffset;
                        if (dm1_v1_inscription_get_visible_raw_text_offset_pc34(
                                things, textIndex, &textDataWordOffset)) {
                            *outTextIndex = textIndex;
                            *outFirstThing = firstThing;
                            return 1;
                        }
                    }
                    thing = F0512_DUNGEON_GetThingNext_Compat(things, thing);
                }
            }
        }
    }
    return 0;
}

static int verify_unscaled_raster_binding(
    const DM1_V1_InscriptionHostMaterialReceiptPc34* receipt)
{
    int line;

    if (!receipt || !DM1_V1_InscriptionHostMaterialRasterGatePc34(
            receipt, DM1_V1_INSCRIPTION_FONT_WIDTH_PC34,
            DM1_V1_INSCRIPTION_FONT_HEIGHT_PC34)) {
        return 0;
    }
    for (line = 0; line < DM1_V1_INSCRIPTION_MAX_LINES; ++line) {
        const DM1_V1_InscriptionFrontWallLineDrawPlanPc34* plan =
            &receipt->lines[line];
        int glyphOffset;
        for (glyphOffset = 0; glyphOffset < plan->glyphCount; ++glyphOffset) {
            DM1_V1_InscriptionRasterCellBindingPc34 binding;
            const int glyph = DM1_V1_InscriptionGlyphIndexFromSourceByte(
                receipt->glyphBytes[plan->glyphStart + glyphOffset]);
            if (glyph < 0 ||
                !DM1_V1_InscriptionBuildRasterCellBindingPc34(
                    receipt, line, glyphOffset, &binding) ||
                binding.fontGraphicIndex !=
                    DM1_V1_INSCRIPTION_FONT_GRAPHIC_INDEX_PC34 ||
                binding.transparentColor != DM1_V1_INSCRIPTION_TRANSPARENT_COLOR ||
                binding.sourceX != glyph * DM1_V1_INSCRIPTION_GLYPH_WIDTH ||
                binding.sourceY != 0 || binding.sourceWidth != 8 ||
                binding.sourceHeight != 8 ||
                binding.destinationX != plan->textX + glyphOffset * 8 ||
                binding.destinationY != plan->textY) {
                return 0;
            }
        }
        if (plan->done) {
            return 1;
        }
    }
    return 0;
}

int main(void)
{
    const char* dataDir = getenv("FIRESTAFF_DM1_DATA_DIR");
    char defaultDir[1024];
    char dungeonPath[1200];
    char graphicsPath[1200];
    const char* home;
    struct DungeonDatState_Compat dungeon;
    struct DungeonThings_Compat things;
    DM1_V1_InscriptionHostMaterialReceiptPc34 receipt;
    DM1_V1_InscriptionHostMaterialReceiptPc34 selectedReceipt;
    unsigned char decoded[DM1_V1_INSCRIPTION_HOST_MATERIAL_MAX_GLYPHS_PC34];
    unsigned short firstThing = THING_ENDOFLIST;
    int textIndex = -1;
    int textDataWordOffset = -1;
    int decodedCount;
    int line;
    int result = 1;

    if (!dataDir || !dataDir[0]) {
        home = getenv("HOME");
        if (!home || !home[0]) {
            return 0;
        }
        snprintf(defaultDir, sizeof(defaultDir), "%s/.firestaff/data/dm1", home);
        dataDir = defaultDir;
    }
    snprintf(dungeonPath, sizeof(dungeonPath), "%s/DUNGEON.DAT", dataDir);
    snprintf(graphicsPath, sizeof(graphicsPath), "%s/GRAPHICS.DAT", dataDir);
    if (!regular_file_has_bytes(dungeonPath) || !regular_file_has_bytes(graphicsPath)) {
        if (getenv("FIRESTAFF_DM1_DATA_DIR")) {
            fprintf(stderr, "configured PC34 DUNGEON.DAT/GRAPHICS.DAT is unavailable\n");
            return 1;
        }
        return 0;
    }

    memset(&dungeon, 0, sizeof(dungeon));
    memset(&things, 0, sizeof(things));
    memset(&receipt, 0, sizeof(receipt));
    if (!F0500_DUNGEON_LoadDatHeader_Compat(dungeonPath, &dungeon) ||
        !F0502_DUNGEON_LoadTileData_Compat(dungeonPath, &dungeon) ||
        !F0504_DUNGEON_LoadThingData_Compat(dungeonPath, &dungeon, &things) ||
        !find_visible_wall_text(&dungeon, &things, &textIndex, &firstThing) ||
        !dm1_v1_inscription_get_visible_raw_text_offset_pc34(
            &things, textIndex, &textDataWordOffset) ||
        !dm1_v1_inscription_host_material_from_world_pc34(
            &things, textIndex, firstThing, &receipt)) {
        fprintf(stderr, "could not build a real PC34 F0168/F0107 inscription receipt\n");
        result = 0;
        goto cleanup;
    }

    /* F0172's selected-wall route and F0168's world route must reach the
     * identical original M648 cells.  This locks HoC/non-HoC call sites to
     * byte<<3, native 8x8 scale and C10 rather than a host text substitute. */
    memset(&selectedReceipt, 0, sizeof(selectedReceipt));
    if (!dm1_v1_inscription_host_material_from_selected_wall_pc34(
            &things, textIndex, &selectedReceipt) ||
        memcmp(&selectedReceipt, &receipt, sizeof(receipt)) != 0 ||
        !verify_unscaled_raster_binding(&receipt) ||
        !verify_unscaled_raster_binding(&selectedReceipt)) {
        fprintf(stderr, "F0168/F0172 M648 raster/palette routes diverged\n");
        result = 0;
        goto cleanup;
    }

    {
        DM1_V1_ViewportInscriptionReceiptPc34 frontReceipt;
        DM1_V1_ViewportInscriptionReceiptPc34 sideReceipt;
        DM1_V1_ViewportInscriptionReceiptPc34 mirrorReceipt;

        memset(&frontReceipt, 0, sizeof(frontReceipt));
        memset(&sideReceipt, 0, sizeof(sideReceipt));
        memset(&mirrorReceipt, 0, sizeof(mirrorReceipt));
        if (!dm1_v1_viewport_inscription_receipt_from_world_pc34(
                &things, textIndex, firstThing,
                DM1_V1_INSCRIPTION_PROJECTION_D1C_FRONT_PC34,
                0, &frontReceipt) ||
            !frontReceipt.valid || !frontReceipt.clearPreviousMaterial ||
            !frontReceipt.drawFrontMaterial ||
            memcmp(&frontReceipt.frontMaterial, &receipt, sizeof(receipt)) != 0 ||
            !dm1_v1_viewport_inscription_receipt_from_world_pc34(
                &things, textIndex, firstThing,
                DM1_V1_INSCRIPTION_PROJECTION_SIDE_OR_DEPTH_PC34,
                0, &sideReceipt) ||
            !sideReceipt.valid || !sideReceipt.clearPreviousMaterial ||
            sideReceipt.drawFrontMaterial || sideReceipt.frontMaterial.valid ||
            !dm1_v1_viewport_inscription_receipt_from_world_pc34(
                &things, textIndex, firstThing,
                DM1_V1_INSCRIPTION_PROJECTION_D1C_FRONT_PC34,
                1, &mirrorReceipt) ||
            !mirrorReceipt.valid || !mirrorReceipt.clearPreviousMaterial ||
            mirrorReceipt.drawFrontMaterial || mirrorReceipt.frontMaterial.valid) {
            fprintf(stderr,
                    "F0107 projection gate exposed readable M648 outside D1C front\n");
            result = 0;
            goto cleanup;
        }
    }

    decodedCount = DM1_V1_InscriptionDecodeRawGlyphsFromWordsPc34(
        things.textData, things.textDataWordCount,
        textDataWordOffset,
        decoded, (int)sizeof(decoded));
    if (!receipt.valid || receipt.textStringIndex != textIndex ||
        receipt.fontGraphicIndex != DM1_V1_INSCRIPTION_FONT_GRAPHIC_INDEX_PC34 ||
        receipt.transparentColor != DM1_V1_INSCRIPTION_TRANSPARENT_COLOR ||
        decodedCount <= 1 || decoded[decodedCount - 1] != 0x81U ||
        receipt.glyphByteCount != decodedCount - 1 ||
        memcmp(receipt.glyphBytes, decoded, (size_t)receipt.glyphByteCount) != 0) {
        fprintf(stderr, "F0168 source bytes did not reach the M648 receipt exactly\n");
        result = 0;
        goto cleanup;
    }

    /* F0168's renderer input is G0284 raw TEXTSTRING, not the convenience
     * decoded mirror produced by the loader. Corrupt the mirror only and
     * require the real raw record to produce identical wall material. */
    if (things.textStrings && textIndex < things.textStringCount) {
        struct DungeonTextString_Compat saved = things.textStrings[textIndex];
        DM1_V1_InscriptionHostMaterialReceiptPc34 rawReceipt;
        things.textStrings[textIndex].visible = 0;
        things.textStrings[textIndex].textDataWordOffset = 0;
        memset(&rawReceipt, 0, sizeof(rawReceipt));
        if (!dm1_v1_inscription_host_material_from_world_pc34(
                &things, textIndex, firstThing, &rawReceipt) ||
            memcmp(&rawReceipt, &receipt, sizeof(receipt)) != 0) {
            fprintf(stderr, "F0168 HoC path consulted decoded TextString mirror\n");
            result = 0;
            things.textStrings[textIndex] = saved;
            goto cleanup;
        }
        things.textStrings[textIndex] = saved;
    }

    for (line = 0; line < receipt.lineCount; ++line) {
        const DM1_V1_InscriptionFrontWallLineDrawPlanPc34* plan =
            &receipt.lines[line];
        if (plan->glyphCount <= 0 ||
            !DM1_V1_InscriptionRawGlyphLineSupportedByFontPc34(
                receipt.glyphBytes + plan->glyphStart, plan->glyphCount,
                DM1_V1_INSCRIPTION_FONT_WIDTH_PC34,
                DM1_V1_INSCRIPTION_FONT_HEIGHT_PC34) ||
            DM1_V1_InscriptionRawGlyphLineSupportedByFontPc34(
                receipt.glyphBytes + plan->glyphStart, plan->glyphCount,
                0, 0) ||
            DM1_V1_InscriptionRawGlyphLineSupportedByFontPc34(
                receipt.glyphBytes + plan->glyphStart, plan->glyphCount,
                DM1_V1_INSCRIPTION_FONT_WIDTH_PC34 - 1,
                DM1_V1_INSCRIPTION_FONT_HEIGHT_PC34)) {
            fprintf(stderr, "M648 missing/short texture gate accepted a draw\n");
            result = 0;
            goto cleanup;
        }
    }

    printf("ok: real PC34 wall TextString %d keeps exact M648/C10 8x8 cells across F0168/F0172\n",
           textIndex);

cleanup:
    F0504_DUNGEON_FreeThingData_Compat(&things);
    F0500_DUNGEON_FreeDatHeader_Compat(&dungeon);
    return result ? 0 : 1;
}
