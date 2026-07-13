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
                        if (textIndex >= 0 && textIndex < things->textStringCount &&
                            things->textStrings[textIndex].visible) {
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
    unsigned char decoded[DM1_V1_INSCRIPTION_HOST_MATERIAL_MAX_GLYPHS_PC34];
    unsigned short firstThing = THING_ENDOFLIST;
    int textIndex = -1;
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
        !dm1_v1_inscription_host_material_from_world_pc34(
            &things, textIndex, firstThing, &receipt)) {
        fprintf(stderr, "could not build a real PC34 F0168/F0107 inscription receipt\n");
        result = 0;
        goto cleanup;
    }

    decodedCount = DM1_V1_InscriptionDecodeRawGlyphsFromWordsPc34(
        things.textData, things.textDataWordCount,
        (int)things.textStrings[textIndex].textDataWordOffset,
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

    printf("ok: real PC34 wall TextString %d reaches only M648 material; missing texture is no-draw\n",
           textIndex);

cleanup:
    F0504_DUNGEON_FreeThingData_Compat(&things);
    F0500_DUNGEON_FreeDatHeader_Compat(&dungeon);
    return result ? 0 : 1;
}
