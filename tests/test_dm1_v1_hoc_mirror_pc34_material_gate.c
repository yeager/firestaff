/*
 * Real PC34 Hall of Champions C127 -> C346/C026 material regression.
 *
 * ReDMCSB DUNGEON.C F0172/F0174 publishes a front-wall C127 sensor's
 * data as G0289. DUNVIEW.C F0107:3913-3928 draws C346 first, then the
 * selected C026 atlas cell at G0109. This stays in DM1/M10: no M11 draw
 * helper or fallback visual participates in the assertion.
 */

#include "dm1_v1_champion_mirror_pc34_compat.h"
#include "dm1_v1_graphic_ids_pc34_compat.h"
#include "dm1_v1_wall_ornament_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int regular_file_has_bytes(const char* path)
{
    FILE* file;
    long size;
    if (!path || !(file = fopen(path, "rb"))) {
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

static int find_hoc_portrait_sensor(const struct DungeonDatState_Compat* dungeon,
                                    const struct DungeonThings_Compat* things,
                                    const struct DungeonSensor_Compat** outSensor,
                                    int* outCell)
{
    const struct DungeonMapDesc_Compat* map;
    int y;
    if (!dungeon || !things || !outSensor || !outCell ||
        dungeon->header.mapCount <= 0 || !dungeon->tiles ||
        !dungeon->tiles[0].squareData) {
        return 0;
    }
    map = &dungeon->maps[0]; /* Original DM1 Hall of Champions. */
    for (y = 0; y < (int)map->height; ++y) {
        int x;
        for (x = 0; x < (int)map->width; ++x) {
            unsigned short thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
                dungeon, things, 0, x, y);
            int safety = 0;
            while (thing != THING_ENDOFLIST && thing != THING_NONE &&
                   safety++ < 64) {
                if (THING_GET_TYPE(thing) == THING_TYPE_SENSOR) {
                    int index = (int)THING_GET_INDEX(thing);
                    if (index >= 0 && index < things->sensorCount &&
                        things->sensors[index].sensorType ==
                            SENSOR_WALL_TYPE_PORTRAIT &&
                        things->sensors[index].sensorData >= 0 &&
                        things->sensors[index].sensorData <
                            DM1_V1_CHAMPION_PORTRAIT_COUNT_PC34) {
                        *outSensor = &things->sensors[index];
                        *outCell = THING_GET_CELL(thing);
                        return 1;
                    }
                }
                thing = F0512_DUNGEON_GetThingNext_Compat(things, thing);
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
    const struct DungeonSensor_Compat* sensor = NULL;
    DM1_V1_ChampionMirrorFrontWallReceiptPc34 front;
    DM1_V1_ChampionMirrorRenderReceiptPc34 render;
    DM1_V1_ChampionMirrorHostDrawReceiptPc34 material;
    DM1_V1_ChampionMirrorHostDrawReceiptPc34 noDraw;
    DM1_V1_ObjectIconSourceZonePc34 expectedPortrait;
    DM1_FrontMirrorRenderPlanPc34 expectedMirror;
    int cell = -1;
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
    memset(&front, 0, sizeof(front));
    memset(&render, 0, sizeof(render));
    memset(&material, 0, sizeof(material));
    memset(&noDraw, 0, sizeof(noDraw));
    if (!F0500_DUNGEON_LoadDatHeader_Compat(dungeonPath, &dungeon) ||
        !F0502_DUNGEON_LoadTileData_Compat(dungeonPath, &dungeon) ||
        !F0504_DUNGEON_LoadThingData_Compat(dungeonPath, &dungeon, &things) ||
        !find_hoc_portrait_sensor(&dungeon, &things, &sensor, &cell) ||
        !DM1_V1_ChampionMirror_F0172FrontWallSensorReceiptPc34(
            sensor->sensorType, sensor->sensorData, sensor->ornamentOrdinal,
            cell, cell, &front) ||
        !DM1_V1_ChampionMirror_BuildRenderReceiptPc34(&front, &render) ||
        !dm1_v1_graphic_champion_portrait_source_zone_pc34(
            sensor->sensorData, &expectedPortrait) ||
        !dm1_v1_front_mirror_render_plan_pc34(
            sensor->sensorData, &expectedMirror) ||
        !DM1_V1_ChampionMirror_BuildHostDrawReceiptPc34(
            &render, 0, 1, &material) ||
        !DM1_V1_ChampionMirror_BuildHostDrawReceiptPc34(
            &render, 0, 0, &noDraw)) {
        fprintf(stderr, "could not build a real HoC C127/C346/C026 receipt\n");
        result = 0;
        goto cleanup;
    }

    if (!front.isFrontMirror ||
        front.championPortraitOrdinal != (int)sensor->sensorData + 1 ||
        front.championPortraitRenderIndex != (int)sensor->sensorData ||
        !render.valid || !render.drawChampionPortrait || !render.drawMirrorBacking ||
        render.graphicIndex != expectedPortrait.graphic_index ||
        render.sourceX != expectedPortrait.x || render.sourceY != expectedPortrait.y ||
        render.width != expectedPortrait.w || render.height != expectedPortrait.h ||
        render.dstX != 96 || render.dstY != 35 ||
        render.backingGraphicIndex != 346 ||
        render.backingDstX != expectedMirror.backingDstX ||
        render.backingDstY != expectedMirror.backingDstY ||
        render.backingWidth != expectedMirror.backingWidth ||
        render.backingHeight != expectedMirror.backingHeight ||
        !material.valid || !material.drawChampionPortrait ||
        !material.drawMirrorBackingAsset ||
        material.drawMirrorBackingFallbackRect ||
        !material.suppressHostFallbackVisuals) {
        fprintf(stderr, "real HoC C346/C026 material placement diverged from F0107\n");
        result = 0;
        goto cleanup;
    }

    if (noDraw.valid || noDraw.drawChampionPortrait ||
        noDraw.drawMirrorBackingAsset || noDraw.drawMirrorBackingFallbackRect ||
        noDraw.drawInvariantBackingRect || noDraw.suppressHostFallbackVisuals) {
        fprintf(stderr, "missing real C346 backing did not fail closed\n");
        result = 0;
        goto cleanup;
    }

    printf("ok: real HoC C127 ordinal %u uses C346 then C026; missing C346 is no-draw\n",
           (unsigned int)sensor->sensorData);

cleanup:
    F0504_DUNGEON_FreeThingData_Compat(&things);
    F0500_DUNGEON_FreeDatHeader_Compat(&dungeon);
    return result ? 0 : 1;
}
