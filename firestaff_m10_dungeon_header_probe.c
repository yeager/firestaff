#include <stdio.h>
#include <stdlib.h>
#include "memory_dungeon_dat_pc34_compat.h"

/*
 * M10 Phase 5 probe: read and verify DUNGEON.DAT header + map descriptors.
 *
 * Usage: probe <DUNGEON.DAT path> <output_dir>
 * Writes:
 *   <output_dir>/dungeon_header_probe.md      — human-readable report
 *   <output_dir>/dungeon_header_invariants.md  — machine-checkable PASS/FAIL
 */

static const char* thingTypeNames[DUNGEON_THING_TYPE_COUNT] = {
        "Door", "Teleporter", "TextString", "Sensor",
        "Group", "Weapon", "Armour", "Scroll",
        "Potion", "Container", "Junk", "Unused11",
        "Unused12", "Unused13", "Projectile", "Explosion"
};

static const char* directionNames[4] = { "North", "East", "South", "West" };

int main(int argc, char* argv[]) {
        struct DungeonDatState_Compat state;
        FILE* report;
        FILE* invariants;
        char path_buf[512];
        int dir, py, px;
        int i;
        int fail_count = 0;
        long expectedMinSize;

        if (argc < 3) {
                fprintf(stderr, "Usage: %s <DUNGEON.DAT> <output_dir>\n", argv[0]);
                return 1;
        }

        if (!F0500_DUNGEON_LoadDatHeader_Compat(argv[1], &state)) {
                fprintf(stderr, "FAIL: could not load dungeon header from %s\n", argv[1]);
                return 1;
        }

        /* ---- write human-readable report ---- */

        snprintf(path_buf, sizeof(path_buf), "%s/dungeon_header_probe.md", argv[2]);
        report = fopen(path_buf, "w");
        if (!report) {
                fprintf(stderr, "FAIL: cannot write %s\n", path_buf);
                F0500_DUNGEON_FreeDatHeader_Compat(&state);
                return 1;
        }

        F0501_DUNGEON_DecodePartyLocation_Compat(
                state.header.initialPartyLocation, &dir, &py, &px);

        fprintf(report, "# DUNGEON.DAT Header Probe\n\n");
        fprintf(report, "File size: %ld bytes\n\n", state.fileSize);
        fprintf(report, "## DUNGEON_HEADER\n\n");
        fprintf(report, "| Field | Value |\n");
        fprintf(report, "|-------|-------|\n");
        fprintf(report, "| OrnamentRandomSeed | %u (0x%04x) |\n",
                state.header.ornamentRandomSeed, state.header.ornamentRandomSeed);
        fprintf(report, "| RawMapDataByteCount | %u |\n", state.header.rawMapDataByteCount);
        fprintf(report, "| MapCount | %u |\n", state.header.mapCount);
        fprintf(report, "| TextDataWordCount | %u |\n", state.header.textDataWordCount);
        fprintf(report, "| InitialPartyLocation | 0x%04x (dir=%s, y=%d, x=%d) |\n",
                state.header.initialPartyLocation, directionNames[dir], py, px);
        fprintf(report, "| SquareFirstThingCount | %u |\n", state.header.squareFirstThingCount);
        fprintf(report, "\n## Thing Counts\n\n");
        fprintf(report, "| Type | Name | Count |\n");
        fprintf(report, "|------|------|-------|\n");
        for (i = 0; i < DUNGEON_THING_TYPE_COUNT; i++) {
                fprintf(report, "| %d | %s | %u |\n",
                        i, thingTypeNames[i], state.header.thingCounts[i]);
        }

        fprintf(report, "\n## Map Descriptors\n\n");
        fprintf(report, "| Map | Level | Width | Height | OffsetXY | RawDataOff |\n");
        fprintf(report, "|-----|-------|-------|--------|----------|------------|\n");
        for (i = 0; i < (int)state.header.mapCount; i++) {
                struct DungeonMapDesc_Compat* m = &state.maps[i];
                fprintf(report, "| %d | %d | %d | %d | (%d,%d) | %u |\n",
                        i, m->level, m->width, m->height,
                        m->offsetMapX, m->offsetMapY,
                        m->rawMapDataByteOffset);
        }
        fclose(report);

        /* ---- invariant checks ---- */

        snprintf(path_buf, sizeof(path_buf), "%s/dungeon_header_invariants.md", argv[2]);
        invariants = fopen(path_buf, "w");
        if (!invariants) {
                fprintf(stderr, "FAIL: cannot write %s\n", path_buf);
                F0500_DUNGEON_FreeDatHeader_Compat(&state);
                return 1;
        }

        fprintf(invariants, "# Dungeon Header Invariants\n\n");

#define CHECK(cond, msg) do { \
        if (cond) { \
                fprintf(invariants, "- PASS: %s\n", msg); \
        } else { \
                fprintf(invariants, "- FAIL: %s\n", msg); \
                fail_count++; \
        } \
} while(0)

        CHECK(state.header.mapCount >= 1 && state.header.mapCount <= 20,
              "MapCount in range 1..20");
        CHECK(state.header.rawMapDataByteCount > 0,
              "RawMapDataByteCount > 0");
        CHECK(state.header.squareFirstThingCount > 0,
              "SquareFirstThingCount > 0");

        /* Party location direction must be 0-3 */
        CHECK(dir >= 0 && dir <= 3,
              "InitialPartyLocation direction 0..3");

        /* Map levels should be sequential 0..MapCount-1 */
        {
                int levels_ok = 1;
                for (i = 0; i < (int)state.header.mapCount; i++) {
                        if (state.maps[i].level != (unsigned char)i) {
                                levels_ok = 0;
                                break;
                        }
                }
                CHECK(levels_ok, "Map levels sequential 0..MapCount-1");
        }

        /* Map dimensions: width and height in 1..32 */
        {
                int dims_ok = 1;
                for (i = 0; i < (int)state.header.mapCount; i++) {
                        if (state.maps[i].width < 1 || state.maps[i].width > 32 ||
                            state.maps[i].height < 1 || state.maps[i].height > 32) {
                                dims_ok = 0;
                                break;
                        }
                }
                CHECK(dims_ok, "All map dimensions in 1..32");
        }

        /* RawMapDataByteOffset should be monotonically increasing */
        {
                int mono_ok = 1;
                for (i = 1; i < (int)state.header.mapCount; i++) {
                        if (state.maps[i].rawMapDataByteOffset <=
                            state.maps[i-1].rawMapDataByteOffset) {
                                mono_ok = 0;
                                break;
                        }
                }
                CHECK(mono_ok, "Map RawMapDataByteOffset monotonically increasing");
        }

        /* Unused thing types (11-13) should have count 0 */
        CHECK(state.header.thingCounts[11] == 0 &&
              state.header.thingCounts[12] == 0 &&
              state.header.thingCounts[13] == 0,
              "Unused thing types 11-13 have count 0");

        /* File size sanity: at least header + maps */
        expectedMinSize = DUNGEON_HEADER_SIZE +
                (long)state.header.mapCount * DUNGEON_MAP_DESC_SIZE;
        CHECK(state.fileSize >= expectedMinSize,
              "File size >= header + map descriptors");

        /* Not a compressed dungeon */
        CHECK(state.header.ornamentRandomSeed != DUNGEON_COMPRESSED_SIGNATURE,
              "Not a compressed dungeon (signature != 0x8104)");

        fprintf(invariants, "\nStatus: %s\n", fail_count == 0 ? "PASS" : "FAIL");
        fclose(invariants);

        printf("Dungeon header probe: %d maps, %s\n",
               state.header.mapCount, fail_count == 0 ? "PASS" : "FAIL");

        F0500_DUNGEON_FreeDatHeader_Compat(&state);
        return fail_count == 0 ? 0 : 1;
}
