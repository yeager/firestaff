#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory_dungeon_dat_pc34_compat.h"

/*
 * M10 Phase 9 probe: decode and verify Monster Groups and Item things
 * from DUNGEON.DAT.
 *
 * Usage: probe <DUNGEON.DAT path> <output_dir>
 */

/* Walk linked list from a SFT entry, counting each thing type
 * and marking visited entries. */
static void walk_linked_list(
        unsigned short firstThing,
        const struct DungeonThings_Compat* things,
        int localCounts[16],
        int* visitedArrays[16])
{
        unsigned short t = firstThing;
        int safety = 10000;

        while (t != THING_NONE && t != THING_ENDOFLIST && safety-- > 0) {
                int type = THING_GET_TYPE(t);
                int idx  = THING_GET_INDEX(t);

                if (type >= 16 || idx >= things->thingCounts[type]) break;

                localCounts[type]++;
                if (visitedArrays[type]) visitedArrays[type][idx] = 1;

                /* Follow next pointer (first 2 bytes of any thing) */
                int byteCount = (int)s_thingDataByteCount[type];
                if (byteCount >= 2 && things->rawThingData[type]) {
                        const unsigned char* raw = things->rawThingData[type] + idx * byteCount;
                        t = (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8));
                } else {
                        break;
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
        int i, m, col, s;
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

        /* Allocate visited arrays for all 16 types */
        int* visitedArrays[16];
        for (i = 0; i < 16; i++) {
                if (things.thingCounts[i] > 0)
                        visitedArrays[i] = (int*)calloc(things.thingCounts[i], sizeof(int));
                else
                        visitedArrays[i] = NULL;
        }

        /* ---- Open report ---- */
        snprintf(path_buf, sizeof(path_buf), "%s/dungeon_monsters_items_probe.md", argv[2]);
        report = fopen(path_buf, "w");
        if (!report) { fprintf(stderr, "Cannot write report\n"); return 1; }

        fprintf(report, "# Phase 9: Monsters & Items Probe\n\n");

        /* ---- Summary counts ---- */
        fprintf(report, "## Thing Counts (from header)\n\n");
        fprintf(report, "| Type | Name | Count |\n|------|------|-------|\n");
        for (i = 0; i < 16; i++) {
                fprintf(report, "| %d | %s | %d |\n", i, s_thingTypeNames[i], things.thingCounts[i]);
        }
        fprintf(report, "\n");

        /* ---- Verify monster groups ---- */
        fprintf(report, "## Monster Groups (type 4)\n\n");
        fprintf(report, "Total groups: %d\n\n", things.groupCount);

        int badCreatureType = 0;
        int maxCreatureType = 0;
        for (i = 0; i < things.groupCount; i++) {
                if (things.groups[i].creatureType > DUNGEON_CREATURE_TYPE_MAX &&
                    things.groups[i].next != THING_NONE) {
                        badCreatureType++;
                }
                if (things.groups[i].creatureType > maxCreatureType)
                        maxCreatureType = things.groups[i].creatureType;
        }
        fprintf(report, "- Max creature type index: %d\n", maxCreatureType);
        fprintf(report, "- Bad creature type (> %d, non-free): %d\n\n", DUNGEON_CREATURE_TYPE_MAX, badCreatureType);
        if (badCreatureType > 0) fail_count++;

        /* ---- Verify weapons ---- */
        fprintf(report, "## Weapons (type 5)\n\n");
        fprintf(report, "Total weapons: %d\n\n", things.weaponCount);
        int badWeaponType = 0;
        for (i = 0; i < things.weaponCount; i++) {
                if (things.weapons[i].type > DUNGEON_WEAPON_TYPE_MAX &&
                    things.weapons[i].next != THING_NONE)
                        badWeaponType++;
        }
        fprintf(report, "- Bad weapon type (> %d, non-free): %d\n\n", DUNGEON_WEAPON_TYPE_MAX, badWeaponType);
        if (badWeaponType > 0) fail_count++;

        /* ---- Verify armour ---- */
        fprintf(report, "## Armour (type 6)\n\n");
        fprintf(report, "Total armour: %d\n\n", things.armourCount);
        int badArmourType = 0;
        for (i = 0; i < things.armourCount; i++) {
                if (things.armours[i].type > DUNGEON_ARMOUR_TYPE_MAX &&
                    things.armours[i].next != THING_NONE)
                        badArmourType++;
        }
        fprintf(report, "- Bad armour type (> %d, non-free): %d\n\n", DUNGEON_ARMOUR_TYPE_MAX, badArmourType);
        if (badArmourType > 0) fail_count++;

        /* ---- Verify potions ---- */
        fprintf(report, "## Potions (type 8)\n\n");
        fprintf(report, "Total potions: %d\n\n", things.potionCount);
        int badPotionType = 0;
        for (i = 0; i < things.potionCount; i++) {
                if (things.potions[i].type > DUNGEON_POTION_TYPE_MAX &&
                    things.potions[i].next != THING_NONE)
                        badPotionType++;
        }
        fprintf(report, "- Bad potion type (> %d, non-free): %d\n\n", DUNGEON_POTION_TYPE_MAX, badPotionType);
        if (badPotionType > 0) fail_count++;

        /* ---- Verify junk ---- */
        fprintf(report, "## Junk (type 10)\n\n");
        fprintf(report, "Total junk: %d\n\n", things.junkCount);
        int badJunkType = 0;
        for (i = 0; i < things.junkCount; i++) {
                if (things.junks[i].type > DUNGEON_JUNK_TYPE_MAX &&
                    things.junks[i].next != THING_NONE)
                        badJunkType++;
        }
        fprintf(report, "- Bad junk type (> %d, non-free): %d\n\n", DUNGEON_JUNK_TYPE_MAX, badJunkType);
        if (badJunkType > 0) fail_count++;

        /* ---- Verify containers ---- */
        fprintf(report, "## Containers (type 9)\n\n");
        fprintf(report, "Total containers: %d\n\n", things.containerCount);
        int badContainerType = 0;
        for (i = 0; i < things.containerCount; i++) {
                if (things.containers[i].type > DUNGEON_CONTAINER_TYPE_MAX &&
                    things.containers[i].next != THING_NONE)
                        badContainerType++;
        }
        fprintf(report, "- Bad container type (> %d, non-free): %d\n\n", DUNGEON_CONTAINER_TYPE_MAX, badContainerType);
        if (badContainerType > 0) fail_count++;

        /* ---- Linked-list walk: count reachable things per type per map ---- */
        fprintf(report, "## Linked-List Reachability\n\n");

        int totalReachable[16];
        memset(totalReachable, 0, sizeof(totalReachable));
        int mapGroupCounts[32];
        memset(mapGroupCounts, 0, sizeof(mapGroupCounts));

        for (m = 0; m < (int)state.header.mapCount; m++) {
                int mapReachable[16];
                memset(mapReachable, 0, sizeof(mapReachable));
                int w = state.maps[m].width;

                for (col = 0; col < w; col++) {
                        int globalCol = mapFirstCol[m] + col;
                        int sftBase = cumTable[globalCol];
                        int sftEnd  = cumTable[globalCol + 1];

                        for (s = sftBase; s < sftEnd; s++) {
                                if (s >= things.squareFirstThingCount) break;
                                unsigned short ft = things.squareFirstThings[s];
                                int localCounts[16];
                                memset(localCounts, 0, sizeof(localCounts));
                                walk_linked_list(ft, &things, localCounts, visitedArrays);
                                for (i = 0; i < 16; i++) mapReachable[i] += localCounts[i];
                        }
                }
                mapGroupCounts[m] = mapReachable[THING_TYPE_GROUP];
                for (i = 0; i < 16; i++) totalReachable[i] += mapReachable[i];
        }

        /* Report per-map group counts */
        fprintf(report, "### Monster Groups per Map\n\n");
        fprintf(report, "| Map | Level | Groups |\n|-----|-------|--------|\n");
        int groupRangeOk = 1;
        for (m = 0; m < (int)state.header.mapCount; m++) {
                fprintf(report, "| %d | %d | %d |\n", m, state.maps[m].level, mapGroupCounts[m]);
                if (mapGroupCounts[m] > 100) groupRangeOk = 0; /* generous cap */
        }
        fprintf(report, "\n");
        if (!groupRangeOk) {
                fprintf(report, "**FAIL**: some map has > 100 groups\n\n");
                fail_count++;
        }

        /* Total items reachable (from SFT chains only) */
        int totalItems = totalReachable[THING_TYPE_WEAPON] +
                         totalReachable[THING_TYPE_ARMOUR] +
                         totalReachable[THING_TYPE_SCROLL] +
                         totalReachable[THING_TYPE_POTION] +
                         totalReachable[THING_TYPE_CONTAINER] +
                         totalReachable[THING_TYPE_JUNK];
        fprintf(report, "### Reachable Things Summary\n\n");
        fprintf(report, "| Type | Reachable |\n|------|----------|\n");
        for (i = 0; i < 16; i++) {
                if (totalReachable[i] > 0)
                        fprintf(report, "| %s | %d |\n", s_thingTypeNames[i], totalReachable[i]);
        }
        fprintf(report, "\nTotal object items (weapon+armour+scroll+potion+container+junk): %d\n\n", totalItems);

        /* Count visited entries per type */
        fprintf(report, "## Visited vs Total (free-list accounting)\n\n");
        fprintf(report, "| Type | Total | Visited | Free |\n|------|-------|---------|------|\n");
        int totalVisited = 0, totalFree = 0;
        for (i = 0; i < 16; i++) {
                if (things.thingCounts[i] == 0) continue;
                int vis = 0;
                if (visitedArrays[i]) {
                        for (int j = 0; j < things.thingCounts[i]; j++)
                                if (visitedArrays[i][j]) vis++;
                }
                int freeCount = things.thingCounts[i] - vis;
                fprintf(report, "| %s | %d | %d | %d |\n", s_thingTypeNames[i],
                        things.thingCounts[i], vis, freeCount);
                totalVisited += vis;
                totalFree += freeCount;
                if (freeCount < 0) {
                        fail_count++;
                        fprintf(report, "**FAIL**: negative free count for %s\n", s_thingTypeNames[i]);
                }
        }
        fprintf(report, "\nTotal visited: %d, total free: %d\n\n", totalVisited, totalFree);

        /* Verify no free entry's next points to a visited entry (structural integrity) */
        int doubleCount = 0;
        for (i = 0; i < 16; i++) {
                if (!visitedArrays[i]) continue;
                int byteCount = (int)s_thingDataByteCount[i];
                if (byteCount < 2 || !things.rawThingData[i]) continue;
                for (int j = 0; j < things.thingCounts[i]; j++) {
                        if (!visitedArrays[i][j]) {
                                const unsigned char* raw = things.rawThingData[i] + j * byteCount;
                                unsigned short nx = (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8));
                                if (nx != THING_NONE && nx != THING_ENDOFLIST) {
                                        int nxIdx = THING_GET_INDEX(nx);
                                        int nxType = THING_GET_TYPE(nx);
                                        if (nxType == i && nxIdx < things.thingCounts[i] &&
                                            visitedArrays[i][nxIdx]) {
                                                doubleCount++;
                                        }
                                }
                        }
                }
        }
        if (doubleCount > 0) {
                fprintf(report, "**FAIL**: %d free-list entries point into visited entries\n\n", doubleCount);
                fail_count++;
        }

        fclose(report);

        /* ---- Write invariants ---- */
        snprintf(path_buf, sizeof(path_buf), "%s/dungeon_monsters_items_invariants.md", argv[2]);
        invariants = fopen(path_buf, "w");
        if (!invariants) { fprintf(stderr, "Cannot write invariants\n"); return 1; }

        fprintf(invariants, "# Phase 9: Monsters & Items Invariants\n\n");
        fprintf(invariants, "Status: %s\n\n", fail_count == 0 ? "PASS" : "FAIL");
        fprintf(invariants, "- Groups decoded: %d\n", things.groupCount);
        fprintf(invariants, "- Weapons decoded: %d\n", things.weaponCount);
        fprintf(invariants, "- Armour decoded: %d\n", things.armourCount);
        fprintf(invariants, "- Scrolls decoded: %d\n", things.scrollCount);
        fprintf(invariants, "- Potions decoded: %d\n", things.potionCount);
        fprintf(invariants, "- Containers decoded: %d\n", things.containerCount);
        fprintf(invariants, "- Junk decoded: %d\n", things.junkCount);
        fprintf(invariants, "- Projectiles decoded: %d\n", things.projectileCount);
        fprintf(invariants, "- Explosions decoded: %d\n", things.explosionCount);
        fprintf(invariants, "- Total reachable items: %d\n", totalItems);
        fprintf(invariants, "- Total visited things: %d\n", totalVisited);
        fprintf(invariants, "- Total free things: %d\n", totalFree);
        fprintf(invariants, "- Bad creature types: %d\n", badCreatureType);
        fprintf(invariants, "- Bad weapon types: %d\n", badWeaponType);
        fprintf(invariants, "- Bad armour types: %d\n", badArmourType);
        fprintf(invariants, "- Bad potion types: %d\n", badPotionType);
        fprintf(invariants, "- Bad junk types: %d\n", badJunkType);
        fprintf(invariants, "- Bad container types: %d\n", badContainerType);
        fprintf(invariants, "- Free-list double-counts: %d\n", doubleCount);
        fprintf(invariants, "- Failures: %d\n", fail_count);

        fclose(invariants);

        /* Cleanup */
        for (i = 0; i < 16; i++) if (visitedArrays[i]) free(visitedArrays[i]);
        free(cumTable);
        free(mapFirstCol);
        F0504_DUNGEON_FreeThingData_Compat(&things);
        F0502_DUNGEON_FreeTileData_Compat(&state);
        F0500_DUNGEON_FreeDatHeader_Compat(&state);

        if (fail_count > 0) {
                fprintf(stderr, "Phase 9: %d failures\n", fail_count);
                return 1;
        }
        printf("Phase 9: PASS\n");
        return 0;
}
