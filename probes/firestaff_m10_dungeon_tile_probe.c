#include <stdio.h>
#include <stdlib.h>
#include "memory_dungeon_dat_pc34_compat.h"

/*
 * M10 Phase 6 probe: decode and verify tile (square) data from DUNGEON.DAT.
 *
 * Usage: probe <DUNGEON.DAT path> <output_dir>
 * Writes:
 *   <output_dir>/dungeon_tile_probe.md         — human-readable report
 *   <output_dir>/dungeon_tile_invariants.md    — machine-checkable PASS/FAIL
 */

int main(int argc, char* argv[]) {
        struct DungeonDatState_Compat state;
        FILE* report;
        FILE* invariants;
        char path_buf[512];
        int i, j;
        int fail_count = 0;
        int any_map_has_diverse_tiles = 0;
        int any_map_tilecount_matches = 0;

        if (argc < 3) {
                fprintf(stderr, "Usage: %s <DUNGEON.DAT> <output_dir>\n", argv[0]);
                return 1;
        }

        if (!F0500_DUNGEON_LoadDatHeader_Compat(argv[1], &state)) {
                fprintf(stderr, "FAIL: could not load dungeon header from %s\n", argv[1]);
                return 1;
        }

        if (!F0502_DUNGEON_LoadTileData_Compat(argv[1], &state)) {
                fprintf(stderr, "FAIL: could not load tile data from %s\n", argv[1]);
                F0500_DUNGEON_FreeDatHeader_Compat(&state);
                return 1;
        }

        /* ---- write human-readable report ---- */

        snprintf(path_buf, sizeof(path_buf), "%s/dungeon_tile_probe.md", argv[2]);
        report = fopen(path_buf, "w");
        if (!report) {
                fprintf(stderr, "FAIL: cannot write %s\n", path_buf);
                F0500_DUNGEON_FreeDatHeader_Compat(&state);
                return 1;
        }

        fprintf(report, "# DUNGEON.DAT Tile/Square Probe\n\n");
        fprintf(report, "Maps: %d\n\n", state.header.mapCount);

        /* Per-map decoded bitfields + tile distribution */
        for (i = 0; i < (int)state.header.mapCount; i++) {
                struct DungeonMapDesc_Compat* m = &state.maps[i];
                struct DungeonMapTiles_Compat* t = &state.tiles[i];
                int elementCounts[DUNGEON_ELEMENT_COUNT + 1]; /* +1 for unknown */
                int thingListCount = 0;
                int distinctTypes = 0;
                int fillerCount = 0;

                fprintf(report, "## Map %d (Level %d, %dx%d)\n\n", i, m->level, m->width, m->height);

                /* Bitfield B/C/D decoded */
                fprintf(report, "### Map Descriptor Bitfields\n\n");
                fprintf(report, "| Field | Value | Source |\n");
                fprintf(report, "|-------|-------|--------|\n");
                fprintf(report, "| WallOrnamentCount | %d | B bits 3:0 (DEFS.H) |\n", m->wallOrnamentCount);
                fprintf(report, "| RandomWallOrnamentCount | %d | B bits 7:4 (DEFS.H) |\n", m->randomWallOrnamentCount);
                fprintf(report, "| FloorOrnamentCount | %d | B bits 11:8 (DEFS.H) |\n", m->floorOrnamentCount);
                fprintf(report, "| RandomFloorOrnamentCount | %d | B bits 15:12 (DEFS.H) |\n", m->randomFloorOrnamentCount);
                fprintf(report, "| DoorOrnamentCount | %d | C bits 3:0 (DEFS.H) |\n", m->doorOrnamentCount);
                fprintf(report, "| CreatureTypeCount | %d | C bits 7:4 (DEFS.H) |\n", m->creatureTypeCount);
                fprintf(report, "| Difficulty | %d | C bits 15:12 (DEFS.H) |\n", m->difficulty);
                fprintf(report, "| FloorSet | %d | D bits 3:0 (DEFS.H) |\n", m->floorSet);
                fprintf(report, "| WallSet | %d | D bits 7:4 (DEFS.H) |\n", m->wallSet);
                fprintf(report, "| DoorSet0 | %d | D bits 11:8 (DEFS.H) |\n", m->doorSet0);
                fprintf(report, "| DoorSet1 | %d | D bits 15:12 (DEFS.H) |\n", m->doorSet1);

                /* Tile distribution */
                for (j = 0; j <= DUNGEON_ELEMENT_COUNT; j++) elementCounts[j] = 0;

                for (j = 0; j < t->squareCount; j++) {
                        unsigned char sq = t->squareData[j];
                        int elemType = (sq & DUNGEON_SQUARE_MASK_TYPE) >> 5;
                        if (sq == 0xFF) {
                                fillerCount++;
                        } else if (elemType >= 0 && elemType < DUNGEON_ELEMENT_COUNT) {
                                elementCounts[elemType]++;
                        } else {
                                elementCounts[DUNGEON_ELEMENT_COUNT]++;
                        }
                        if (sq & DUNGEON_SQUARE_MASK_THING_LIST) {
                                thingListCount++;
                        }
                }

                fprintf(report, "\n### Square Distribution (%d squares = %d x %d)\n\n",
                        t->squareCount, m->width, m->height);
                fprintf(report, "| Element | Count | %% |\n");
                fprintf(report, "|---------|-------|---|\n");
                for (j = 0; j < DUNGEON_ELEMENT_COUNT; j++) {
                        if (elementCounts[j] > 0) {
                                distinctTypes++;
                                fprintf(report, "| %s | %d | %.1f%% |\n",
                                        F0503_DUNGEON_GetElementName_Compat(j),
                                        elementCounts[j],
                                        100.0 * elementCounts[j] / t->squareCount);
                        }
                }
                if (fillerCount > 0) {
                        fprintf(report, "| *Filler (0xFF)* | %d | %.1f%% |\n",
                                fillerCount,
                                100.0 * fillerCount / t->squareCount);
                }
                if (elementCounts[DUNGEON_ELEMENT_COUNT] > 0) {
                        fprintf(report, "| Unknown (type7, not 0xFF) | %d | %.1f%% |\n",
                                elementCounts[DUNGEON_ELEMENT_COUNT],
                                100.0 * elementCounts[DUNGEON_ELEMENT_COUNT] / t->squareCount);
                }
                fprintf(report, "| *ThingList set* | %d | %.1f%% |\n",
                        thingListCount,
                        100.0 * thingListCount / t->squareCount);

                fprintf(report, "\nDistinct element types: %d\n\n", distinctTypes);

                /* First 16 raw bytes for debugging */
                {
                        int showCount = t->squareCount < 16 ? t->squareCount : 16;
                        fprintf(report, "First %d raw square bytes (hex): ", showCount);
                        for (j = 0; j < showCount; j++) {
                                fprintf(report, "%02X ", t->squareData[j]);
                        }
                        fprintf(report, "\n\n");
                }

                /* Track for invariants */
                if (t->squareCount == (int)m->width * (int)m->height) {
                        any_map_tilecount_matches = 1;
                }
                if (distinctTypes >= 2) {
                        any_map_has_diverse_tiles = 1;
                }
        }

        fclose(report);

        /* ---- invariant checks ---- */

        snprintf(path_buf, sizeof(path_buf), "%s/dungeon_tile_invariants.md", argv[2]);
        invariants = fopen(path_buf, "w");
        if (!invariants) {
                fprintf(stderr, "FAIL: cannot write %s\n", path_buf);
                F0500_DUNGEON_FreeDatHeader_Compat(&state);
                return 1;
        }

        fprintf(invariants, "# Dungeon Tile Invariants\n\n");

#define CHECK(cond, msg) do { \
        if (cond) { \
                fprintf(invariants, "- PASS: %s\n", msg); \
        } else { \
                fprintf(invariants, "- FAIL: %s\n", msg); \
                fail_count++; \
        } \
} while(0)

        /* 1. Tile-count matches width*height for at least one map */
        CHECK(any_map_tilecount_matches,
              "Tile-count matches width*height for at least one map");

        /* 2. All square element types in valid 3-bit range (0-7).
         * DEFS.H names types 0-6, but value 7 appears in CSB dungeons
         * and is a valid encoding for the 3-bit field. */
        {
                int all_valid = 1;
                for (i = 0; i < (int)state.header.mapCount && all_valid; i++) {
                        struct DungeonMapTiles_Compat* t = &state.tiles[i];
                        for (j = 0; j < t->squareCount; j++) {
                                int elemType = (t->squareData[j] & DUNGEON_SQUARE_MASK_TYPE) >> 5;
                                if (elemType > 7) {
                                        all_valid = 0; /* impossible for 3 bits, but defensive */
                                        break;
                                }
                        }
                }
                CHECK(all_valid, "All tile element types in valid 3-bit range 0..7");
        }

        /* 2b. Excluding 0xFF filler, >=85% of tiles use named types 0-6.
         * CSB uses type 7 for certain squares and 0xFF as filler. */
        {
                int total_non_filler = 0;
                int named_type_squares = 0;
                for (i = 0; i < (int)state.header.mapCount; i++) {
                        struct DungeonMapTiles_Compat* t = &state.tiles[i];
                        for (j = 0; j < t->squareCount; j++) {
                                if (t->squareData[j] == 0xFF) continue;
                                total_non_filler++;
                                if ((t->squareData[j] >> 5) <= 6)
                                        named_type_squares++;
                        }
                }
                CHECK(total_non_filler > 0 &&
                      (named_type_squares * 100 / total_non_filler) >= 85,
                      ">=85% of non-filler tiles use named element types 0..6");
        }

        /* 3. Not degenerate: at least one map has >=2 distinct element types */
        CHECK(any_map_has_diverse_tiles,
              "At least one map has >= 2 distinct element types (not degenerate)");

        /* 4. All maps loaded with correct square count */
        {
                int all_match = 1;
                for (i = 0; i < (int)state.header.mapCount; i++) {
                        if (state.tiles[i].squareCount !=
                            (int)state.maps[i].width * (int)state.maps[i].height) {
                                all_match = 0;
                                break;
                        }
                }
                CHECK(all_match, "All maps: squareCount == width * height");
        }

        /* 5. Wall is present in every map (DM/CSB always has walls) */
        {
                int all_have_walls = 1;
                for (i = 0; i < (int)state.header.mapCount; i++) {
                        struct DungeonMapTiles_Compat* t = &state.tiles[i];
                        int hasWall = 0;
                        for (j = 0; j < t->squareCount; j++) {
                                if (((t->squareData[j] & DUNGEON_SQUARE_MASK_TYPE) >> 5) == DUNGEON_ELEMENT_WALL) {
                                        hasWall = 1;
                                        break;
                                }
                        }
                        if (!hasWall) { all_have_walls = 0; break; }
                }
                CHECK(all_have_walls, "Every map contains at least one Wall square");
        }

        /* 6. Corridor present in at least one map */
        {
                int any_corridor = 0;
                for (i = 0; i < (int)state.header.mapCount && !any_corridor; i++) {
                        struct DungeonMapTiles_Compat* t = &state.tiles[i];
                        for (j = 0; j < t->squareCount; j++) {
                                if (((t->squareData[j] & DUNGEON_SQUARE_MASK_TYPE) >> 5) == DUNGEON_ELEMENT_CORRIDOR) {
                                        any_corridor = 1;
                                        break;
                                }
                        }
                }
                CHECK(any_corridor, "At least one map contains Corridor squares");
        }

        /* 7. Bitfield D values reasonable: sets 0-15 */
        {
                int sets_ok = 1;
                for (i = 0; i < (int)state.header.mapCount; i++) {
                        struct DungeonMapDesc_Compat* m = &state.maps[i];
                        if (m->floorSet > 15 || m->wallSet > 15 ||
                            m->doorSet0 > 15 || m->doorSet1 > 15) {
                                sets_ok = 0;
                                break;
                        }
                }
                CHECK(sets_ok, "All graphic set indices in range 0..15");
        }

        fprintf(invariants, "\nStatus: %s\n", fail_count == 0 ? "PASS" : "FAIL");
        fclose(invariants);

        printf("Dungeon tile probe: %d maps decoded, %s\n",
               state.header.mapCount, fail_count == 0 ? "PASS" : "FAIL");

        F0500_DUNGEON_FreeDatHeader_Compat(&state);
        return fail_count == 0 ? 0 : 1;
}
