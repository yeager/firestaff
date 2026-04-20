#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory_dungeon_dat_pc34_compat.h"

/*
 * M10 Phase 8 probe: decode and verify Door and Sensor (actuator) things
 * from DUNGEON.DAT.
 *
 * Usage: probe <DUNGEON.DAT path> <output_dir>
 */

/* Walk linked list from a SquareFirstThing, counting doors and sensors. */
static void count_things_on_square(
        unsigned short firstThing,
        const struct DungeonThings_Compat* things,
        int* outDoors,
        int* outSensors)
{
        unsigned short t = firstThing;
        int safety = 1000;
        *outDoors = 0;
        *outSensors = 0;

        while (t != THING_NONE && t != THING_ENDOFLIST && safety-- > 0) {
                int type = THING_GET_TYPE(t);
                int idx  = THING_GET_INDEX(t);
                int byteCount;
                const unsigned char* raw;

                if (type == THING_TYPE_DOOR && idx < things->doorCount) {
                        (*outDoors)++;
                        t = things->doors[idx].next;
                } else if (type == THING_TYPE_SENSOR && idx < things->sensorCount) {
                        (*outSensors)++;
                        t = things->sensors[idx].next;
                } else if (type == THING_TYPE_TEXTSTRING && idx < things->textStringCount) {
                        t = things->textStrings[idx].next;
                } else if (type == THING_TYPE_TELEPORTER && idx < things->teleporterCount) {
                        t = things->teleporters[idx].next;
                } else {
                        /* Other thing types: follow raw Next pointer (first 2 bytes) */
                        byteCount = (int)s_thingDataByteCount[type];
                        if (byteCount >= 2 && things->rawThingData[type] &&
                            idx < things->thingCounts[type]) {
                                raw = things->rawThingData[type] + idx * byteCount;
                                t = (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8));
                        } else {
                                break;
                        }
                }
        }
}

int main(int argc, char* argv[]) {
        struct DungeonDatState_Compat state;
        struct DungeonThings_Compat things;
        FILE* report;
        FILE* invariants;
        FILE* dfile;
        char path_buf[512];
        int i, m, col;
        int fail_count = 0;
        int totalColumns = 0;
        int* cumTable = NULL;
        int* mapFirstCol = NULL;

        if (argc < 3) {
                fprintf(stderr, "Usage: %s <DUNGEON.DAT> <output_dir>\n", argv[0]);
                return 1;
        }

        if (!F0500_DUNGEON_LoadDatHeader_Compat(argv[1], &state)) {
                fprintf(stderr, "FAIL: could not load header\n");
                return 1;
        }
        if (!F0502_DUNGEON_LoadTileData_Compat(argv[1], &state)) {
                fprintf(stderr, "FAIL: could not load tiles\n");
                F0500_DUNGEON_FreeDatHeader_Compat(&state);
                return 1;
        }
        memset(&things, 0, sizeof(things));
        if (!F0504_DUNGEON_LoadThingData_Compat(argv[1], &state, &things)) {
                fprintf(stderr, "FAIL: could not load thing data\n");
                F0502_DUNGEON_FreeTileData_Compat(&state);
                F0500_DUNGEON_FreeDatHeader_Compat(&state);
                return 1;
        }

        /* Read cumulative column table from disk */
        for (m = 0; m < (int)state.header.mapCount; m++)
                totalColumns += state.maps[m].width;

        cumTable = (int*)calloc(totalColumns + 1, sizeof(int));
        mapFirstCol = (int*)calloc(state.header.mapCount, sizeof(int));
        if (!cumTable || !mapFirstCol) { fprintf(stderr, "OOM\n"); return 1; }

        dfile = fopen(argv[1], "rb");
        if (!dfile) { fprintf(stderr, "Cannot open dungeon\n"); return 1; }
        {
                long cumOffset = DUNGEON_HEADER_SIZE +
                        (long)state.header.mapCount * DUNGEON_MAP_DESC_SIZE;
                fseek(dfile, cumOffset, SEEK_SET);
                for (i = 0; i < totalColumns; i++) {
                        unsigned char b[2];
                        if (fread(b, 1, 2, dfile) != 2) {
                                fprintf(stderr, "FAIL: short read on cumtable\n");
                                fclose(dfile); return 1;
                        }
                        cumTable[i] = (int)(b[0] | ((unsigned short)b[1] << 8));
                }
                /* Sentinel: total SFT count */
                cumTable[totalColumns] = things.squareFirstThingCount;
        }
        fclose(dfile);

        {
                int c = 0;
                for (m = 0; m < (int)state.header.mapCount; m++) {
                        mapFirstCol[m] = c;
                        c += state.maps[m].width;
                }
        }

        /* ---- Open report files ---- */
        snprintf(path_buf, sizeof(path_buf), "%s/dungeon_doors_sensors_probe.md", argv[2]);
        report = fopen(path_buf, "w");
        if (!report) { fprintf(stderr, "Cannot create report\n"); return 1; }

        snprintf(path_buf, sizeof(path_buf), "%s/dungeon_doors_sensors_invariants.md", argv[2]);
        invariants = fopen(path_buf, "w");
        if (!invariants) { fclose(report); return 1; }

        fprintf(report, "# Dungeon Doors & Sensors (Actuators) Probe\n\n");
        fprintf(report, "## Summary\n\n");
        fprintf(report, "- Total doors (header): %d\n", things.doorCount);
        fprintf(report, "- Total sensors (header): %d\n", things.sensorCount);
        fprintf(report, "- Maps: %d\n\n", (int)state.header.mapCount);

        fprintf(invariants, "# Dungeon Doors & Sensors Invariants\n\n");

        /* ---- Invariant 1: counts match header ---- */
        if (things.doorCount == (int)state.header.thingCounts[THING_TYPE_DOOR]) {
                fprintf(invariants, "- PASS: Door count %d matches header thingCounts[0]\n", things.doorCount);
        } else {
                fprintf(invariants, "- FAIL: Door count %d != header thingCounts[0] %d\n",
                        things.doorCount, (int)state.header.thingCounts[THING_TYPE_DOOR]);
                fail_count++;
        }
        if (things.sensorCount == (int)state.header.thingCounts[THING_TYPE_SENSOR]) {
                fprintf(invariants, "- PASS: Sensor count %d matches header thingCounts[3]\n", things.sensorCount);
        } else {
                fprintf(invariants, "- FAIL: Sensor count %d != header thingCounts[3] %d\n",
                        things.sensorCount, (int)state.header.thingCounts[THING_TYPE_SENSOR]);
                fail_count++;
        }

        /* ---- Invariant 2: Door field ranges ---- */
        {
                int bad_type = 0, bad_ornament = 0;
                for (i = 0; i < things.doorCount; i++) {
                        if (things.doors[i].type > 1) bad_type++;
                        if (things.doors[i].ornamentOrdinal > 15) bad_ornament++;
                }
                if (bad_type == 0) {
                        fprintf(invariants, "- PASS: All %d doors have type in {0,1}\n", things.doorCount);
                } else {
                        fprintf(invariants, "- FAIL: %d doors have type > 1\n", bad_type);
                        fail_count++;
                }
                if (bad_ornament == 0) {
                        fprintf(invariants, "- PASS: All %d doors have ornamentOrdinal < 16\n", things.doorCount);
                } else {
                        fprintf(invariants, "- FAIL: %d doors have ornamentOrdinal > 15\n", bad_ornament);
                        fail_count++;
                }
        }

        /* ---- Invariant 3: Sensor field ranges ---- */
        {
                int bad_type = 0, bad_effect = 0;
                for (i = 0; i < things.sensorCount; i++) {
                        if (things.sensors[i].sensorType > 127) bad_type++;
                        if (things.sensors[i].effect > 3) bad_effect++;
                }
                if (bad_type == 0) {
                        fprintf(invariants, "- PASS: All %d sensors have sensorType <= 127\n", things.sensorCount);
                } else {
                        fprintf(invariants, "- FAIL: %d sensors have sensorType > 127\n", bad_type);
                        fail_count++;
                }
                if (bad_effect == 0) {
                        fprintf(invariants, "- PASS: All %d sensors have effect <= 3\n", things.sensorCount);
                } else {
                        fprintf(invariants, "- FAIL: %d sensors have effect > 3\n", bad_effect);
                        fail_count++;
                }
        }

        /* ---- Invariant 4: Per-map door/sensor counts via linked list walk ---- */
        fprintf(report, "## Per-Map Door & Sensor Counts\n\n");
        fprintf(report, "| Map | Level | Size | Doors | Sensors |\n");
        fprintf(report, "|-----|-------|------|-------|---------|\n");

        {
                int totalDoorsWalked = 0, totalSensorsWalked = 0;
                int anyMapBadDoorRange = 0, anyMapBadSensorRange = 0;

                for (m = 0; m < (int)state.header.mapCount; m++) {
                        int w = state.maps[m].width;
                        int mapDoors = 0, mapSensors = 0;

                        for (col = 0; col < w; col++) {
                                int globalCol = mapFirstCol[m] + col;
                                int sftBase = cumTable[globalCol];
                                int sftEnd  = cumTable[globalCol + 1];
                                int s;

                                for (s = sftBase; s < sftEnd; s++) {
                                        if (s >= things.squareFirstThingCount) break;
                                        unsigned short firstThing = things.squareFirstThings[s];
                                        int sd = 0, ss = 0;
                                        count_things_on_square(firstThing, &things, &sd, &ss);
                                        mapDoors += sd;
                                        mapSensors += ss;
                                }
                        }

                        fprintf(report, "| %d | %d | %dx%d | %d | %d |\n",
                                m, state.maps[m].level, w, state.maps[m].height,
                                mapDoors, mapSensors);

                        totalDoorsWalked += mapDoors;
                        totalSensorsWalked += mapSensors;

                        if (mapDoors > 64) anyMapBadDoorRange++;
                        if (mapSensors > 200) anyMapBadSensorRange++;
                }

                fprintf(report, "\n**Total doors walked: %d** (header: %d)\n", totalDoorsWalked, things.doorCount);
                fprintf(report, "**Total sensors walked: %d** (header: %d)\n\n", totalSensorsWalked, things.sensorCount);

                /* Counter invariant: walked totals <= header (some things may be in free list).
                 * For a fresh dungeon (not a save), walked should equal header. */
                if (totalDoorsWalked <= things.doorCount && totalDoorsWalked > 0) {
                        fprintf(invariants, "- PASS: Walked door count %d <= header %d (placed doors)\n",
                                totalDoorsWalked, things.doorCount);
                } else if (things.doorCount == 0 && totalDoorsWalked == 0) {
                        fprintf(invariants, "- PASS: No doors in dungeon\n");
                } else {
                        fprintf(invariants, "- FAIL: Walked door count %d vs header %d\n",
                                totalDoorsWalked, things.doorCount);
                        fail_count++;
                }

                if (totalSensorsWalked <= things.sensorCount && totalSensorsWalked > 0) {
                        fprintf(invariants, "- PASS: Walked sensor count %d <= header %d (placed sensors)\n",
                                totalSensorsWalked, things.sensorCount);
                } else if (things.sensorCount == 0 && totalSensorsWalked == 0) {
                        fprintf(invariants, "- PASS: No sensors in dungeon\n");
                } else {
                        fprintf(invariants, "- FAIL: Walked sensor count %d vs header %d\n",
                                totalSensorsWalked, things.sensorCount);
                        fail_count++;
                }

                if (anyMapBadDoorRange == 0) {
                        fprintf(invariants, "- PASS: All maps have door count in range 0..64\n");
                } else {
                        fprintf(invariants, "- FAIL: %d maps have > 64 doors\n", anyMapBadDoorRange);
                        fail_count++;
                }

                if (anyMapBadSensorRange == 0) {
                        fprintf(invariants, "- PASS: All maps have sensor count in range 0..200\n");
                } else {
                        fprintf(invariants, "- FAIL: %d maps have > 200 sensors\n", anyMapBadSensorRange);
                        fail_count++;
                }
        }

        /* ---- Door detail table ---- */
        fprintf(report, "## Door Details (first 20)\n\n");
        fprintf(report, "| Index | Type | Ornament | Vertical | Button | MagicDest | MeleeDest |\n");
        fprintf(report, "|-------|------|----------|----------|--------|-----------|-----------|\n");
        for (i = 0; i < things.doorCount && i < 20; i++) {
                fprintf(report, "| %d | %d | %d | %d | %d | %d | %d |\n",
                        i,
                        things.doors[i].type,
                        things.doors[i].ornamentOrdinal,
                        things.doors[i].vertical,
                        things.doors[i].button,
                        things.doors[i].magicDestructible,
                        things.doors[i].meleeDestructible);
        }

        /* ---- Sensor detail table ---- */
        fprintf(report, "\n## Sensor Details (first 30)\n\n");
        fprintf(report, "| Index | SType | Data | Effect | OnceOnly | Audible | Value | OrnOrd | TargX | TargY | TargCell | LocalMult |\n");
        fprintf(report, "|-------|-------|------|--------|----------|---------|-------|--------|-------|-------|----------|-----------|\n");
        for (i = 0; i < things.sensorCount && i < 30; i++) {
                fprintf(report, "| %d | %d | %d | %d | %d | %d | %d | %d | %d | %d | %d | %d |\n",
                        i,
                        things.sensors[i].sensorType,
                        things.sensors[i].sensorData,
                        things.sensors[i].effect,
                        things.sensors[i].onceOnly,
                        things.sensors[i].audible,
                        things.sensors[i].value,
                        things.sensors[i].ornamentOrdinal,
                        things.sensors[i].targetMapX,
                        things.sensors[i].targetMapY,
                        things.sensors[i].targetCell,
                        things.sensors[i].localMultiple);
        }

        /* ---- Sensor type histogram ---- */
        fprintf(report, "\n## Sensor Type Histogram\n\n");
        {
                int typeHist[128];
                memset(typeHist, 0, sizeof(typeHist));
                for (i = 0; i < things.sensorCount; i++) {
                        if (things.sensors[i].sensorType < 128)
                                typeHist[things.sensors[i].sensorType]++;
                }
                fprintf(report, "| Type | Count |\n");
                fprintf(report, "|------|-------|\n");
                for (i = 0; i < 128; i++) {
                        if (typeHist[i] > 0)
                                fprintf(report, "| %d | %d |\n", i, typeHist[i]);
                }
        }

        /* ---- Final status ---- */
        fprintf(invariants, "\nStatus: %s\n", fail_count == 0 ? "PASS" : "FAIL");

        fclose(report);
        fclose(invariants);

        printf("Dungeon doors/sensors probe: %d doors, %d sensors, %s\n",
               things.doorCount, things.sensorCount,
               fail_count == 0 ? "PASS" : "FAIL");

        F0504_DUNGEON_FreeThingData_Compat(&things);
        F0502_DUNGEON_FreeTileData_Compat(&state);
        F0500_DUNGEON_FreeDatHeader_Compat(&state);

        free(cumTable);
        free(mapFirstCol);

        return fail_count == 0 ? 0 : 1;
}
