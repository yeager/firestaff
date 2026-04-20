#include <stdio.h>
#include <stdlib.h>
#include "memory_dungeon_dat_pc34_compat.h"

/*
 * M10 Phase 7 probe: decode and verify "things" from DUNGEON.DAT.
 *
 * Decodes Door, TextString, and Teleporter things.
 * Verifies field ranges, linked-list consistency, count invariants,
 * and cross-references with SquareFirstThings + tile data.
 *
 * Usage: probe <DUNGEON.DAT path> <output_dir>
 */

int main(int argc, char* argv[]) {
        struct DungeonDatState_Compat state;
        struct DungeonThings_Compat things;
        FILE* report;
        FILE* invariants;
        char path_buf[512];
        int i;
        int fail_count = 0;

        if (argc < 3) {
                fprintf(stderr, "Usage: %s <DUNGEON.DAT> <output_dir>\n", argv[0]);
                return 1;
        }

        /* Load header + tiles */
        if (!F0500_DUNGEON_LoadDatHeader_Compat(argv[1], &state)) {
                fprintf(stderr, "FAIL: could not load dungeon header from %s\n", argv[1]);
                return 1;
        }
        if (!F0502_DUNGEON_LoadTileData_Compat(argv[1], &state)) {
                fprintf(stderr, "FAIL: could not load tile data from %s\n", argv[1]);
                F0500_DUNGEON_FreeDatHeader_Compat(&state);
                return 1;
        }

        /* Load things */
        if (!F0504_DUNGEON_LoadThingData_Compat(argv[1], &state, &things)) {
                fprintf(stderr, "FAIL: could not load thing data from %s\n", argv[1]);
                F0500_DUNGEON_FreeDatHeader_Compat(&state);
                return 1;
        }

        /* ---- write human-readable report ---- */

        snprintf(path_buf, sizeof(path_buf), "%s/dungeon_things_probe.md", argv[2]);
        report = fopen(path_buf, "w");
        if (!report) {
                fprintf(stderr, "FAIL: cannot write %s\n", path_buf);
                goto cleanup;
        }

        fprintf(report, "# DUNGEON.DAT Things Probe\n\n");

        /* Summary of thing counts */
        fprintf(report, "## Thing Counts from Header\n\n");
        fprintf(report, "| Type | Name | Count | BytesEach | TotalBytes |\n");
        fprintf(report, "|------|------|-------|-----------|------------|\n");
        {
                int totalThingBytes = 0;
                for (i = 0; i < 16; i++) {
                        int bytes = things.thingCounts[i] * (int)s_thingDataByteCount[i];
                        totalThingBytes += bytes;
                        if (things.thingCounts[i] > 0) {
                                fprintf(report, "| %d | %s | %d | %d | %d |\n",
                                        i, F0505_DUNGEON_GetThingTypeName_Compat(i),
                                        things.thingCounts[i], (int)s_thingDataByteCount[i], bytes);
                        }
                }
                fprintf(report, "| | **Total** | | | **%d** |\n\n", totalThingBytes);
        }

        fprintf(report, "SquareFirstThingCount: %d\n", things.squareFirstThingCount);
        fprintf(report, "TextDataWordCount: %d (%d bytes)\n\n",
                things.textDataWordCount, things.textDataWordCount * 2);

        /* SquareFirstThings analysis */
        fprintf(report, "## SquareFirstThings Analysis\n\n");
        {
                int noneCount = 0, endCount = 0, validCount = 0;
                int typeCounts[16] = {0};
                for (i = 0; i < things.squareFirstThingCount; i++) {
                        unsigned short t = things.squareFirstThings[i];
                        if (t == THING_NONE) {
                                noneCount++;
                        } else if (t == THING_ENDOFLIST) {
                                endCount++;
                        } else {
                                validCount++;
                                int type = (int)THING_GET_TYPE(t);
                                typeCounts[type]++;
                        }
                }
                fprintf(report, "- THING_NONE (0xFFFF): %d\n", noneCount);
                fprintf(report, "- THING_ENDOFLIST (0xFFFE): %d\n", endCount);
                fprintf(report, "- Valid thing refs: %d\n", validCount);
                fprintf(report, "\nBreakdown by type:\n\n");
                fprintf(report, "| Type | Name | Count |\n");
                fprintf(report, "|------|------|-------|\n");
                for (i = 0; i < 16; i++) {
                        if (typeCounts[i] > 0) {
                                fprintf(report, "| %d | %s | %d |\n",
                                        i, F0505_DUNGEON_GetThingTypeName_Compat(i), typeCounts[i]);
                        }
                }
                fprintf(report, "\n");
        }

        /* Door things detail */
        fprintf(report, "## Door Things (%d total)\n\n", things.doorCount);
        if (things.doorCount > 0) {
                int usedCount = 0, unusedCount = 0;
                int typeDistrib[2] = {0};
                int vertCount = 0, buttonCount = 0, magicDestCount = 0, meleeDestCount = 0;
                int maxOrnament = 0;

                fprintf(report, "| # | Next | Type | Ornament | Vert | Btn | MagicDest | MeleeDest |\n");
                fprintf(report, "|---|------|------|----------|------|-----|-----------|----------|\n");
                for (i = 0; i < things.doorCount && i < 30; i++) {
                        struct DungeonDoor_Compat* d = &things.doors[i];
                        const char* nextStr = (d->next == THING_NONE) ? "NONE" :
                                              (d->next == THING_ENDOFLIST) ? "END" : "ref";
                        fprintf(report, "| %d | %s (0x%04X) | %d | %d | %d | %d | %d | %d |\n",
                                i, nextStr, d->next, d->type, d->ornamentOrdinal,
                                d->vertical, d->button, d->magicDestructible, d->meleeDestructible);
                }
                if (things.doorCount > 30) {
                        fprintf(report, "| ... | (%d more) | | | | | | |\n\n", things.doorCount - 30);
                }

                for (i = 0; i < things.doorCount; i++) {
                        struct DungeonDoor_Compat* d = &things.doors[i];
                        if (d->next == THING_NONE) {
                                unusedCount++;
                        } else {
                                usedCount++;
                        }
                        if (d->type < 2) typeDistrib[d->type]++;
                        if (d->vertical) vertCount++;
                        if (d->button) buttonCount++;
                        if (d->magicDestructible) magicDestCount++;
                        if (d->meleeDestructible) meleeDestCount++;
                        if (d->ornamentOrdinal > maxOrnament) maxOrnament = d->ornamentOrdinal;
                }

                fprintf(report, "\n### Door Statistics\n\n");
                fprintf(report, "- Used (in linked list): %d\n", usedCount);
                fprintf(report, "- Unused (THING_NONE next): %d\n", unusedCount);
                fprintf(report, "- Type 0 (DoorSet0): %d\n", typeDistrib[0]);
                fprintf(report, "- Type 1 (DoorSet1): %d\n", typeDistrib[1]);
                fprintf(report, "- Vertical: %d\n", vertCount);
                fprintf(report, "- Has button: %d\n", buttonCount);
                fprintf(report, "- Magic destructible: %d\n", magicDestCount);
                fprintf(report, "- Melee destructible: %d\n", meleeDestCount);
                fprintf(report, "- Max ornament ordinal: %d\n\n", maxOrnament);
        }

        /* TextString things detail */
        fprintf(report, "## TextString Things (%d total)\n\n", things.textStringCount);
        if (things.textStringCount > 0) {
                int usedCount = 0, unusedCount = 0, visibleCount = 0;
                int maxOffset = 0;

                fprintf(report, "| # | Next | Visible | TextDataWordOffset |\n");
                fprintf(report, "|---|------|---------|--------------------|\n");
                for (i = 0; i < things.textStringCount && i < 30; i++) {
                        struct DungeonTextString_Compat* ts = &things.textStrings[i];
                        const char* nextStr = (ts->next == THING_NONE) ? "NONE" :
                                              (ts->next == THING_ENDOFLIST) ? "END" : "ref";
                        fprintf(report, "| %d | %s (0x%04X) | %d | %d |\n",
                                i, nextStr, ts->next, ts->visible, ts->textDataWordOffset);
                }
                if (things.textStringCount > 30) {
                        fprintf(report, "| ... | (%d more) | | |\n", things.textStringCount - 30);
                }

                for (i = 0; i < things.textStringCount; i++) {
                        struct DungeonTextString_Compat* ts = &things.textStrings[i];
                        if (ts->next == THING_NONE) unusedCount++;
                        else usedCount++;
                        if (ts->visible) visibleCount++;
                        if (ts->textDataWordOffset > maxOffset) maxOffset = ts->textDataWordOffset;
                }

                fprintf(report, "\n### TextString Statistics\n\n");
                fprintf(report, "- Used: %d\n", usedCount);
                fprintf(report, "- Unused: %d\n", unusedCount);
                fprintf(report, "- Visible: %d\n", visibleCount);
                fprintf(report, "- Max TextDataWordOffset: %d (textDataWordCount: %d)\n\n",
                        maxOffset, things.textDataWordCount);
        }

        /* Teleporter things detail */
        fprintf(report, "## Teleporter Things (%d total)\n\n", things.teleporterCount);
        if (things.teleporterCount > 0) {
                int usedCount = 0, unusedCount = 0;

                fprintf(report, "| # | Next | TargetMap | TargetX | TargetY | Rot | AbsRot | Scope | Audible |\n");
                fprintf(report, "|---|------|-----------|---------|---------|-----|--------|-------|--------|\n");
                for (i = 0; i < things.teleporterCount && i < 30; i++) {
                        struct DungeonTeleporter_Compat* tp = &things.teleporters[i];
                        const char* nextStr = (tp->next == THING_NONE) ? "NONE" :
                                              (tp->next == THING_ENDOFLIST) ? "END" : "ref";
                        fprintf(report, "| %d | %s (0x%04X) | %d | %d | %d | %d | %d | %d | %d |\n",
                                i, nextStr, tp->next, tp->targetMapIndex,
                                tp->targetMapX, tp->targetMapY,
                                tp->rotation, tp->absoluteRotation, tp->scope, tp->audible);
                }
                if (things.teleporterCount > 30) {
                        fprintf(report, "| ... | (%d more) | | | | | | | |\n", things.teleporterCount - 30);
                }

                for (i = 0; i < things.teleporterCount; i++) {
                        if (things.teleporters[i].next == THING_NONE) unusedCount++;
                        else usedCount++;
                }
                fprintf(report, "\n### Teleporter Statistics\n\n");
                fprintf(report, "- Used: %d\n", usedCount);
                fprintf(report, "- Unused: %d\n\n", unusedCount);
        }

        fclose(report);

        /* ---- invariant checks ---- */

        snprintf(path_buf, sizeof(path_buf), "%s/dungeon_things_invariants.md", argv[2]);
        invariants = fopen(path_buf, "w");
        if (!invariants) {
                fprintf(stderr, "FAIL: cannot write invariants\n");
                goto cleanup;
        }

        fprintf(invariants, "# Dungeon Things Invariants\n\n");

#define CHECK(cond, msg) do { \
        if (cond) { \
                fprintf(invariants, "- PASS: %s\n", msg); \
        } else { \
                fprintf(invariants, "- FAIL: %s\n", msg); \
                fail_count++; \
        } \
} while(0)

        /* 1. Thing data loaded successfully */
        CHECK(things.loaded, "Thing data loaded successfully");

        /* 2. Door count > 0 (DM/CSB always has doors) */
        CHECK(things.doorCount > 0, "Door count > 0");

        /* 3. All Door fields in valid ranges */
        {
                int allValid = 1;
                for (i = 0; i < things.doorCount; i++) {
                        struct DungeonDoor_Compat* d = &things.doors[i];
                        if (d->type > 1) { allValid = 0; break; }
                        if (d->ornamentOrdinal > 15) { allValid = 0; break; }
                }
                CHECK(allValid, "All Door fields in valid ranges (type 0-1, ornament 0-15)");
        }

        /* 4. At least some doors are used (not all THING_NONE) */
        {
                int usedDoors = 0;
                for (i = 0; i < things.doorCount; i++) {
                        if (things.doors[i].next != THING_NONE) usedDoors++;
                }
                CHECK(usedDoors > 0, "At least some doors are used (non-degenerate)");
        }

        /* 5. SquareFirstThings: all valid refs have index < thingCounts[type] */
        {
                int allRefsValid = 1;
                for (i = 0; i < things.squareFirstThingCount; i++) {
                        unsigned short t = things.squareFirstThings[i];
                        if (t == THING_NONE || t == THING_ENDOFLIST) continue;
                        int type = (int)THING_GET_TYPE(t);
                        int idx = (int)THING_GET_INDEX(t);
                        if (type < 0 || type >= 16 || idx >= things.thingCounts[type]) {
                                allRefsValid = 0;
                                break;
                        }
                }
                CHECK(allRefsValid, "All SquareFirstThing refs have valid type/index");
        }

        /* 6. Door things referenced from SquareFirstThings match Door square tiles */
        /* We count squares with Door element type vs Door-type SquareFirstThings */
        {
                int doorSquares = 0;
                int doorFirstThings = 0;
                for (i = 0; i < (int)state.header.mapCount; i++) {
                        struct DungeonMapTiles_Compat* t = &state.tiles[i];
                        int j;
                        for (j = 0; j < t->squareCount; j++) {
                                int elemType = (t->squareData[j] >> 5) & 0x07;
                                if (elemType == DUNGEON_ELEMENT_DOOR) doorSquares++;
                        }
                }
                for (i = 0; i < things.squareFirstThingCount; i++) {
                        unsigned short t = things.squareFirstThings[i];
                        if (t != THING_NONE && t != THING_ENDOFLIST) {
                                if (THING_GET_TYPE(t) == THING_TYPE_DOOR) doorFirstThings++;
                        }
                }
                /* Door squares should have a Door as first thing — approximate match */
                char msg[128];
                snprintf(msg, sizeof(msg),
                        "Door squares (%d) >= Door first-things (%d) [doors referenced from SFT]",
                        doorSquares, doorFirstThings);
                CHECK(doorSquares >= doorFirstThings, msg);
        }

        /* 7. TextString things: all used have TextDataWordOffset < textDataWordCount */
        {
                int allValid = 1;
                for (i = 0; i < things.textStringCount; i++) {
                        struct DungeonTextString_Compat* ts = &things.textStrings[i];
                        if (ts->next == THING_NONE) continue; /* unused */
                        if (ts->textDataWordOffset >= (unsigned short)things.textDataWordCount) {
                                allValid = 0;
                                break;
                        }
                }
                CHECK(allValid, "All used TextString offsets < textDataWordCount");
        }

        /* 8. Teleporter targets: targetMapIndex < mapCount for used teleporters */
        {
                int allValid = 1;
                for (i = 0; i < things.teleporterCount; i++) {
                        struct DungeonTeleporter_Compat* tp = &things.teleporters[i];
                        if (tp->next == THING_NONE) continue;
                        if (tp->targetMapIndex >= state.header.mapCount) {
                                allValid = 0;
                                break;
                        }
                }
                CHECK(allValid, "All used Teleporter targetMapIndex < mapCount");
        }

        /* 9. Teleporter target coords within target map bounds */
        {
                int allValid = 1;
                for (i = 0; i < things.teleporterCount; i++) {
                        struct DungeonTeleporter_Compat* tp = &things.teleporters[i];
                        if (tp->next == THING_NONE) continue;
                        if (tp->targetMapIndex >= state.header.mapCount) { allValid = 0; break; }
                        struct DungeonMapDesc_Compat* m = &state.maps[tp->targetMapIndex];
                        if (tp->targetMapX >= m->width || tp->targetMapY >= m->height) {
                                allValid = 0;
                                break;
                        }
                }
                CHECK(allValid, "All used Teleporter target coords within target map bounds");
        }

        /* 10. Non-degenerate: multiple thing types present in SquareFirstThings */
        {
                int typesPresent = 0;
                int seen[16] = {0};
                for (i = 0; i < things.squareFirstThingCount; i++) {
                        unsigned short t = things.squareFirstThings[i];
                        if (t == THING_NONE || t == THING_ENDOFLIST) continue;
                        int type = (int)THING_GET_TYPE(t);
                        if (type >= 0 && type < 16 && !seen[type]) {
                                seen[type] = 1;
                                typesPresent++;
                        }
                }
                char msg[128];
                snprintf(msg, sizeof(msg),
                        "Multiple thing types in SquareFirstThings (%d types)", typesPresent);
                CHECK(typesPresent >= 2, msg);
        }

        /* 11. Linked list consistency: Door Next fields point to valid things or ENDOFLIST */
        {
                int allConsistent = 1;
                for (i = 0; i < things.doorCount; i++) {
                        unsigned short next = things.doors[i].next;
                        if (next == THING_NONE || next == THING_ENDOFLIST) continue;
                        int type = (int)THING_GET_TYPE(next);
                        int idx = (int)THING_GET_INDEX(next);
                        if (type < 0 || type >= 16 || idx >= things.thingCounts[type]) {
                                allConsistent = 0;
                                break;
                        }
                }
                CHECK(allConsistent, "Door Next fields: all refs valid or NONE/ENDOFLIST");
        }

        /* 12. File size sanity: header + maps + cumtable + rawdata + sft + things + text <= filesize */
        {
                long totalColumns = 0;
                for (i = 0; i < (int)state.header.mapCount; i++)
                        totalColumns += state.maps[i].width;
                long expected = DUNGEON_HEADER_SIZE +
                        (long)state.header.mapCount * DUNGEON_MAP_DESC_SIZE +
                        (totalColumns + 1) * 2 +
                        (long)state.header.rawMapDataByteCount +
                        (long)state.header.squareFirstThingCount * 2;
                for (i = 0; i < 16; i++)
                        expected += (long)things.thingCounts[i] * (long)s_thingDataByteCount[i];
                expected += (long)things.textDataWordCount * 2;
                char msg[128];
                snprintf(msg, sizeof(msg),
                        "Computed file size %ld <= actual %ld", expected, state.fileSize);
                CHECK(expected <= state.fileSize, msg);
        }

        fprintf(invariants, "\nStatus: %s\n", fail_count == 0 ? "PASS" : "FAIL");
        fclose(invariants);

        printf("Dungeon things probe: Door=%d TextString=%d Teleporter=%d, %s\n",
               things.doorCount, things.textStringCount, things.teleporterCount,
               fail_count == 0 ? "PASS" : "FAIL");

cleanup:
        F0504_DUNGEON_FreeThingData_Compat(&things);
        F0500_DUNGEON_FreeDatHeader_Compat(&state);
        return fail_count == 0 ? 0 : 1;
}
