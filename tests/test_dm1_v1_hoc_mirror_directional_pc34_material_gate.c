#include "dm1_v1_champion_mirror_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Real-PC34 HoC regression for the direction gate around a C127 mirror.
 * ReDMCSB DUNGEON.C F0172/F0174:2558,2608-2612 only publishes G0289 while
 * processing the current front wall.  DUNVIEW.C F0107:3913-3928 then draws
 * C346 before C026 at D1C-front only.  REVIVE.C F0280:794-800 confirms that
 * each original mirror square owns its sensor rather than a synthetic panel.
 */

typedef struct HocMirrorSensorPc34 {
    int mapIndex;
    int mapX;
    int mapY;
    int thingCell;
    int sensorData;
    int ornamentOrdinal;
} HocMirrorSensorPc34;

static int regular_file_has_bytes(const char *path)
{
    FILE *file;
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

static int data_path(char *out, size_t outSize, const char *directory,
                     const char *name)
{
    int written;

    if (!out || !directory || !name) {
        return 0;
    }
    written = snprintf(out, outSize, "%s/%s", directory, name);
    return written > 0 && (size_t)written < outSize;
}

static int verify_directional_gate(const HocMirrorSensorPc34 *sensor)
{
    int visibleCell;

    if (!sensor) {
        return 0;
    }
    for (visibleCell = 0; visibleCell < 4; ++visibleCell) {
        DM1_V1_ChampionMirrorFrontWallReceiptPc34 front;
        DM1_V1_ChampionMirrorRenderReceiptPc34 render;
        DM1_V1_ChampionMirrorHostDrawReceiptPc34 host;
        const int matchingCell = visibleCell == sensor->thingCell;

        if (!DM1_V1_ChampionMirror_F0172FrontWallSensorReceiptPc34(
                SENSOR_WALL_TYPE_PORTRAIT, sensor->sensorData,
                sensor->ornamentOrdinal, sensor->thingCell, visibleCell,
                &front) ||
            !DM1_V1_ChampionMirror_BuildViewportRenderReceiptPc34(
                1, &front, &render) ||
            !DM1_V1_ChampionMirror_BuildSourceOwnedHostDrawReceiptPc34(
                &render, 0, 1, &host)) {
            fprintf(stderr, "FAIL C127 receipt failed for visible cell %d\n",
                    visibleCell);
            return 0;
        }

        if (matchingCell) {
            if (!front.isFrontMirror || !render.valid ||
                !render.drawMirrorBacking || !render.drawChampionPortrait ||
                render.backingGraphicIndex != 346 ||
                render.graphicIndex !=
                    DM1_V1_CHAMPION_MIRROR_PORTRAIT_GRAPHIC_PC34_COMPAT ||
                render.sourceX != (sensor->sensorData & 7) * 32 ||
                render.sourceY != (sensor->sensorData >> 3) * 29 ||
                render.dstX != DM1_V1_CHAMPION_MIRROR_PORTRAIT_DST_X_PC34_COMPAT ||
                render.dstY != DM1_V1_CHAMPION_MIRROR_PORTRAIT_DST_Y_PC34_COMPAT ||
                !host.valid || !host.drawMirrorBackingAsset ||
                !host.drawChampionPortrait ||
                host.drawMirrorBackingFallbackRect ||
                host.drawInvariantBackingRect || !host.suppressHostFallbackVisuals) {
                fprintf(stderr,
                        "FAIL C127 cell %d material front=%d render=%d backing=%d "
                        "portrait=%d graphic=%d host=%d asset=%d invariant=%d\n",
                        visibleCell, front.isFrontMirror, render.valid,
                        render.drawMirrorBacking, render.drawChampionPortrait,
                        render.backingGraphicIndex, host.valid,
                        host.drawMirrorBackingAsset,
                        host.drawInvariantBackingRect);
                return 0;
            }
        } else if (front.isFrontMirror || !render.valid ||
                   render.drawMirrorBacking || render.drawChampionPortrait ||
                   !render.clearStaleChampionPortraitOrdinal ||
                   !render.suppressChampionPortrait ||
                   host.valid || host.drawMirrorBackingAsset ||
                   host.drawChampionPortrait || host.drawMirrorBackingFallbackRect ||
                   host.drawInvariantBackingRect) {
            fprintf(stderr,
                    "FAIL non-front C127 cell %d leaked C346/C026 material\n",
                    visibleCell);
            return 0;
        }
    }
    return 1;
}

static int verify_all_hoc_c127_sensors(
    const struct DungeonDatState_Compat *dungeon,
    const struct DungeonThings_Compat *things, HocMirrorSensorPc34 *outFirst,
    int *outSensorCount, int *outDistinctPortraitCount)
{
    const struct DungeonMapDesc_Compat *map;
    unsigned int portrait_mask = 0U;
    int sensor_count = 0;
    int x;

    if (!dungeon || !things || !outFirst || !outSensorCount ||
        !outDistinctPortraitCount || dungeon->header.mapCount < 1 ||
        !things->sensors) return 0;
    map = &dungeon->maps[0];
    memset(outFirst, 0, sizeof(*outFirst));
    for (x = 0; x < (int)map->width; ++x) {
        int y;
        for (y = 0; y < (int)map->height; ++y) {
            unsigned short thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
                dungeon, things, 0, x, y);
            int safety = 0;

            while (thing != THING_NONE && thing != THING_ENDOFLIST &&
                   safety++ < 64) {
                if (THING_GET_TYPE(thing) == THING_TYPE_SENSOR) {
                    int sensor_index = (int)THING_GET_INDEX(thing);
                    if (sensor_index >= 0 && sensor_index < things->sensorCount &&
                        things->sensors[sensor_index].sensorType ==
                            SENSOR_WALL_TYPE_PORTRAIT) {
                        HocMirrorSensorPc34 sensor;

                        memset(&sensor, 0, sizeof(sensor));
                        sensor.mapIndex = 0;
                        sensor.mapX = x;
                        sensor.mapY = y;
                        sensor.thingCell = (int)THING_GET_CELL(thing);
                        sensor.sensorData =
                            (int)things->sensors[sensor_index].sensorData;
                        sensor.ornamentOrdinal =
                            (int)things->sensors[sensor_index].ornamentOrdinal;
                        if (sensor.sensorData < 0 || sensor.sensorData >=
                            DM1_V1_CHAMPION_MIRROR_PORTRAIT_ATLAS_COUNT_PC34_COMPAT) {
                            fprintf(stderr,
                                    "FAIL C127 at %d,%d has C026 atlas index %d outside 0..%d\n",
                                    x, y, sensor.sensorData,
                                    DM1_V1_CHAMPION_MIRROR_PORTRAIT_ATLAS_COUNT_PC34_COMPAT - 1);
                            return 0;
                        }
                        if (!verify_directional_gate(&sensor)) return 0;
                        if (sensor_count == 0) *outFirst = sensor;
                        portrait_mask |= 1U << sensor.sensorData;
                        ++sensor_count;
                    }
                }
                thing = F0512_DUNGEON_GetThingNext_Compat(things, thing);
            }
        }
    }
    *outSensorCount = sensor_count;
    *outDistinctPortraitCount = 0;
    while (portrait_mask) {
        *outDistinctPortraitCount += (int)(portrait_mask & 1U);
        portrait_mask >>= 1;
    }
    return sensor_count > 0;
}

int main(void)
{
    const char *directory = getenv("FIRESTAFF_DM1_DATA_DIR");
    char dungeonPath[1024];
    char graphicsPath[1024];
    struct DungeonDatState_Compat dungeon;
    struct DungeonThings_Compat things;
    HocMirrorSensorPc34 sensor;
    int sensorCount = 0;
    int distinctPortraitCount = 0;
    int result = 1;

    if (!directory || !data_path(dungeonPath, sizeof(dungeonPath), directory,
                                 "DUNGEON.DAT") ||
        !data_path(graphicsPath, sizeof(graphicsPath), directory,
                   "GRAPHICS.DAT")) {
        puts("SKIP: FIRESTAFF_DM1_DATA_DIR is not selected");
        return 0;
    }
    if (!regular_file_has_bytes(dungeonPath) ||
        !regular_file_has_bytes(graphicsPath)) {
        fprintf(stderr, "FAIL missing real PC34 data under %s\n", directory);
        return 1;
    }

    memset(&dungeon, 0, sizeof(dungeon));
    memset(&things, 0, sizeof(things));
    memset(&sensor, 0, sizeof(sensor));
    if (!F0500_DUNGEON_LoadDatHeader_Compat(dungeonPath, &dungeon) ||
        !F0502_DUNGEON_LoadTileData_Compat(dungeonPath, &dungeon) ||
        !F0504_DUNGEON_LoadThingData_Compat(dungeonPath, &dungeon, &things)) {
        fprintf(stderr, "FAIL could not decode real PC34 HoC dungeon\n");
        result = 0;
    } else if (!verify_all_hoc_c127_sensors(&dungeon, &things, &sensor,
                                              &sensorCount,
                                              &distinctPortraitCount)) {
        fprintf(stderr, "FAIL real PC34 HoC has no C127 mirror sensor\n");
        result = 0;
    }

    F0504_DUNGEON_FreeThingData_Compat(&things);
    F0500_DUNGEON_FreeDatHeader_Compat(&dungeon);
    if (result) {
        printf("probe=dm1_v1_hoc_mirror_directional_pc34_material_gate\n");
        printf("sourceEvidence=ReDMCSB DUNGEON.C F0172/F0174; DUNVIEW.C F0107:3913-3928; REVIVE.C F0280\n");
        printf("hocMirrorSensors=%d distinctPortraits=%d first=%d,%d cell=%d portrait=%d\n",
               sensorCount, distinctPortraitCount, sensor.mapX, sensor.mapY,
               sensor.thingCell, sensor.sensorData);
    }
    return result ? 0 : 1;
}
