#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory_movement_pc34_compat.h"

/*
 * M10 Phase 10 probe: movement + champion state verification.
 *
 * Tests:
 *   1. Initial party position matches DUNGEON.DAT header
 *   2. Movement in all 4 directions from start
 *   3. Turn left/right direction rotation
 *   4. Champion struct round-trip serialisation
 *   5. Party struct round-trip serialisation
 *   6. Sensor identification on a square (find first sensor, don't execute)
 */

static const char* dirNames[4] = { "North", "East", "South", "West" };
static const char* moveNames[6] = {
    "Forward", "Right", "Backward", "Left", "TurnRight", "TurnLeft"
};

int main(int argc, char* argv[]) {
    struct DungeonDatState_Compat dungeon;
    struct DungeonThings_Compat things;
    struct PartyState_Compat party;
    struct MovementResult_Compat result;
    FILE* report;
    FILE* invariants;
    char path_buf[512];
    int failCount = 0;
    int dir, py, px;
    int i;

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <DUNGEON.DAT> <output_dir>\n", argv[0]);
        return 1;
    }

    /* ---- Load dungeon ---- */
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&things, 0, sizeof(things));

    if (!F0500_DUNGEON_LoadDatHeader_Compat(argv[1], &dungeon)) {
        fprintf(stderr, "FAIL: could not load dungeon header\n");
        return 1;
    }
    if (!F0502_DUNGEON_LoadTileData_Compat(argv[1], &dungeon)) {
        fprintf(stderr, "FAIL: could not load tile data\n");
        F0500_DUNGEON_FreeDatHeader_Compat(&dungeon);
        return 1;
    }
    if (!F0504_DUNGEON_LoadThingData_Compat(argv[1], &dungeon, &things)) {
        fprintf(stderr, "FAIL: could not load thing data\n");
        F0502_DUNGEON_FreeTileData_Compat(&dungeon);
        F0500_DUNGEON_FreeDatHeader_Compat(&dungeon);
        return 1;
    }

    /* ---- Init party ---- */
    if (!F0601_CHAMPION_InitPartyFromDungeon_Compat(&dungeon, &party)) {
        fprintf(stderr, "FAIL: could not init party\n");
        F0504_DUNGEON_FreeThingData_Compat(&things);
        F0502_DUNGEON_FreeTileData_Compat(&dungeon);
        F0500_DUNGEON_FreeDatHeader_Compat(&dungeon);
        return 1;
    }

    /* Decode expected values */
    F0501_DUNGEON_DecodePartyLocation_Compat(
        dungeon.header.initialPartyLocation, &dir, &py, &px);

    /* ---- Write report ---- */
    snprintf(path_buf, sizeof(path_buf), "%s/movement_champions_probe.md", argv[2]);
    report = fopen(path_buf, "w");
    if (!report) { fprintf(stderr, "FAIL: cannot write report\n"); return 1; }

    fprintf(report, "# M10 Phase 10: Movement + Champions Probe\n\n");
    fprintf(report, "## Initial Party Position\n\n");
    fprintf(report, "| Field | Header | Party |\n");
    fprintf(report, "|-------|--------|-------|\n");
    fprintf(report, "| mapX | %d | %d |\n", px, party.mapX);
    fprintf(report, "| mapY | %d | %d |\n", py, party.mapY);
    fprintf(report, "| direction | %s (%d) | %s (%d) |\n",
            dirNames[dir], dir, dirNames[party.direction], party.direction);
    fprintf(report, "| mapIndex | 0 | %d |\n", party.mapIndex);
    fprintf(report, "| championCount | 0 | %d |\n", party.championCount);

    /* ---- Movement tests ---- */
    fprintf(report, "\n## Movement Tests (from start position)\n\n");
    fprintf(report, "| Action | Result | NewX | NewY | NewDir |\n");
    fprintf(report, "|--------|--------|------|------|--------|\n");

    for (i = 0; i < MOVE_ACTION_COUNT; i++) {
        F0702_MOVEMENT_TryMove_Compat(&dungeon, &party, i, &result);
        fprintf(report, "| %s | %s | %d | %d | %s |\n",
                moveNames[i],
                result.resultCode == MOVE_OK ? "OK" :
                result.resultCode == MOVE_TURN_ONLY ? "TURN" :
                result.resultCode == MOVE_BLOCKED_WALL ? "WALL" :
                result.resultCode == MOVE_BLOCKED_BOUNDS ? "BOUNDS" : "DOOR",
                result.newMapX, result.newMapY,
                dirNames[result.newDirection]);
    }

    /* ---- Turn rotation tests ---- */
    fprintf(report, "\n## Direction Rotation Tests\n\n");
    fprintf(report, "| Start | TurnRight | TurnLeft |\n");
    fprintf(report, "|-------|-----------|----------|\n");
    for (i = 0; i < 4; i++) {
        int r = F0700_MOVEMENT_TurnDirection_Compat(i, 1);
        int l = F0700_MOVEMENT_TurnDirection_Compat(i, 0);
        fprintf(report, "| %s | %s | %s |\n", dirNames[i], dirNames[r], dirNames[l]);
    }

    /* ---- Champion serialisation round-trip ---- */
    fprintf(report, "\n## Champion Serialisation Round-Trip\n\n");
    {
        struct ChampionState_Compat orig, restored;
        unsigned char buf[CHAMPION_SERIALIZED_SIZE];
        int rc1, rc2;

        F0600_CHAMPION_InitEmpty_Compat(&orig);
        orig.present = 1;
        orig.portraitIndex = 3;
        memcpy(orig.name, "TESTNAME", 8);
        memcpy(orig.title, "SOURCE TITLE        ", 20);
        orig.sex = 'F';
        memcpy(orig.mirrorStatsText, "AADFACNAAAAP    ", 16);
        memcpy(orig.mirrorSkillsText, "DHCGCDCLCNCKCI  ", 16);
        orig.attributes[0] = 50;
        orig.attributes[1] = 40;
        orig.attributes[2] = 30;
        orig.attributes[3] = 60;
        orig.attributes[4] = 20;
        orig.attributes[5] = 15;
        orig.skillLevels[0] = 5;
        orig.skillLevels[1] = 3;
        orig.skillLevels[2] = 7;
        orig.skillLevels[3] = 2;
        orig.hp.current = 100; orig.hp.maximum = 120; orig.hp.shifted = 240;
        orig.stamina.current = 80; orig.stamina.maximum = 100; orig.stamina.shifted = 200;
        orig.mana.current = 50; orig.mana.maximum = 60; orig.mana.shifted = 120;
        orig.inventory[CHAMPION_SLOT_HAND_LEFT] = 0x1400;  /* some weapon thing */
        orig.load = 500;
        orig.maxLoad = 2000;
        orig.wounds = 0x03;
        orig.poisonDose = 10;
        orig.food = 200;
        orig.water = 180;
        orig.direction = DIR_SOUTH;

        rc1 = F0602_CHAMPION_Serialize_Compat(&orig, buf, sizeof(buf));
        rc2 = F0603_CHAMPION_Deserialize_Compat(&restored, buf, sizeof(buf));

        fprintf(report, "- Serialize returned: %d\n", rc1);
        fprintf(report, "- Deserialize returned: %d\n", rc2);
        fprintf(report, "- Bit-identical: %s\n",
                memcmp(&orig, &restored, sizeof(orig)) == 0 ? "YES" : "NO");

        if (memcmp(&orig, &restored, sizeof(orig)) != 0) {
            /* Detail what differs */
            unsigned char* a = (unsigned char*)&orig;
            unsigned char* b = (unsigned char*)&restored;
            int j;
            int diffCount = 0;
            for (j = 0; j < (int)sizeof(orig); j++) {
                if (a[j] != b[j]) {
                    if (diffCount < 10)
                        fprintf(report, "  - Byte %d: orig=0x%02x restored=0x%02x\n", j, a[j], b[j]);
                    diffCount++;
                }
            }
            fprintf(report, "  - Total differing bytes: %d / %d\n", diffCount, (int)sizeof(orig));
        }
    }

    /* ---- Party serialisation round-trip ---- */
    fprintf(report, "\n## Party Serialisation Round-Trip\n\n");
    {
        struct PartyState_Compat origP, restoredP;
        unsigned char buf[PARTY_SERIALIZED_SIZE];
        int rc1, rc2;

        memcpy(&origP, &party, sizeof(origP));
        /* Add a test champion */
        origP.championCount = 1;
        origP.champions[0].present = 1;
        origP.champions[0].portraitIndex = 5;
        memcpy(origP.champions[0].name, "HALK    ", 8);
        memcpy(origP.champions[0].title, "THE BARBARIAN       ", 20);
        origP.champions[0].sex = 'M';
        memcpy(origP.champions[0].mirrorStatsText, "AADFACNAAAAP    ", 16);
        memcpy(origP.champions[0].mirrorSkillsText, "DHCGCDCLCNCKCI  ", 16);
        origP.champions[0].hp.current = 90;
        origP.champions[0].hp.maximum = 100;
        origP.champions[0].hp.shifted = 200;

        rc1 = F0604_PARTY_Serialize_Compat(&origP, buf, sizeof(buf));
        rc2 = F0605_PARTY_Deserialize_Compat(&restoredP, buf, sizeof(buf));

        fprintf(report, "- Serialize returned: %d\n", rc1);
        fprintf(report, "- Deserialize returned: %d\n", rc2);
        fprintf(report, "- Bit-identical: %s\n",
                memcmp(&origP, &restoredP, sizeof(origP)) == 0 ? "YES" : "NO");
    }

    /* ---- Sensor identification ---- */
    fprintf(report, "\n## Sensor Identification\n\n");
    {
        struct SensorOnSquare_Compat sensorResult;
        int foundAnySensor = 0;
        int mx, my, mi;

        /* Scan all squares on map 0 looking for the first sensor */
        if (dungeon.header.mapCount > 0) {
            const struct DungeonMapDesc_Compat* map0 = &dungeon.maps[0];
            for (mx = 0; mx < map0->width && !foundAnySensor; mx++) {
                for (my = 0; my < map0->height && !foundAnySensor; my++) {
                    if (F0703_MOVEMENT_IdentifySensorsOnSquare_Compat(
                            &dungeon, &things, 0, mx, my, &sensorResult)) {
                        foundAnySensor = 1;
                        fprintf(report, "First sensor found at map=0, x=%d, y=%d:\n", mx, my);
                        fprintf(report, "- sensorIndex: %d\n", sensorResult.sensorIndex);
                        fprintf(report, "- sensorType: %d\n", sensorResult.sensorType);
                        fprintf(report, "- sensorData: %u\n", sensorResult.sensorData);
                        fprintf(report, "- isLocal: %d\n", sensorResult.isLocal);
                        fprintf(report, "- targetMapX: %d\n", sensorResult.targetMapX);
                        fprintf(report, "- targetMapY: %d\n", sensorResult.targetMapY);
                        fprintf(report, "- targetCell: %d\n", sensorResult.targetCell);
                        fprintf(report, "- totalSensorsOnSquare: %d\n",
                                sensorResult.totalSensorsOnSquare);
                    }
                }
            }
        }

        if (!foundAnySensor) {
            /* Try all maps */
            for (mi = 1; mi < (int)dungeon.header.mapCount && !foundAnySensor; mi++) {
                const struct DungeonMapDesc_Compat* mapN = &dungeon.maps[mi];
                for (mx = 0; mx < mapN->width && !foundAnySensor; mx++) {
                    for (my = 0; my < mapN->height && !foundAnySensor; my++) {
                        if (F0703_MOVEMENT_IdentifySensorsOnSquare_Compat(
                                &dungeon, &things, mi, mx, my, &sensorResult)) {
                            foundAnySensor = 1;
                            fprintf(report, "First sensor found at map=%d, x=%d, y=%d:\n", mi, mx, my);
                            fprintf(report, "- sensorIndex: %d\n", sensorResult.sensorIndex);
                            fprintf(report, "- sensorType: %d\n", sensorResult.sensorType);
                            fprintf(report, "- sensorData: %u\n", sensorResult.sensorData);
                            fprintf(report, "- isLocal: %d\n", sensorResult.isLocal);
                            fprintf(report, "- targetMapX: %d\n", sensorResult.targetMapX);
                            fprintf(report, "- targetMapY: %d\n", sensorResult.targetMapY);
                            fprintf(report, "- targetCell: %d\n", sensorResult.targetCell);
                            fprintf(report, "- totalSensorsOnSquare: %d\n",
                                    sensorResult.totalSensorsOnSquare);
                        }
                    }
                }
            }
        }

        if (!foundAnySensor) {
            fprintf(report, "WARNING: No sensors found in any map square.\n");
        }
    }

    fclose(report);

    /* ---- Write invariants ---- */
    snprintf(path_buf, sizeof(path_buf), "%s/movement_champions_invariants.md", argv[2]);
    invariants = fopen(path_buf, "w");
    if (!invariants) { fprintf(stderr, "FAIL: cannot write invariants\n"); return 1; }

    fprintf(invariants, "# Movement + Champions Invariants\n\n");

#define CHECK(cond, msg) do { \
    if (cond) { \
        fprintf(invariants, "- PASS: %s\n", msg); \
    } else { \
        fprintf(invariants, "- FAIL: %s\n", msg); \
        failCount++; \
    } \
} while(0)

    /* Test 1: Initial party position */
    CHECK(party.mapX == px, "Party mapX matches header");
    CHECK(party.mapY == py, "Party mapY matches header");
    CHECK(party.direction == dir, "Party direction matches header");
    CHECK(party.mapIndex == 0, "Party starts on map 0");
    CHECK(party.championCount == 0, "No champions initially");
    CHECK(party.activeChampionIndex == -1, "No active champion initially");

    /* Test 2: Direction rotation */
    CHECK(F0700_MOVEMENT_TurnDirection_Compat(DIR_NORTH, 1) == DIR_EAST,
          "North + TurnRight = East");
    CHECK(F0700_MOVEMENT_TurnDirection_Compat(DIR_EAST, 1) == DIR_SOUTH,
          "East + TurnRight = South");
    CHECK(F0700_MOVEMENT_TurnDirection_Compat(DIR_SOUTH, 1) == DIR_WEST,
          "South + TurnRight = West");
    CHECK(F0700_MOVEMENT_TurnDirection_Compat(DIR_WEST, 1) == DIR_NORTH,
          "West + TurnRight = North");
    CHECK(F0700_MOVEMENT_TurnDirection_Compat(DIR_NORTH, 0) == DIR_WEST,
          "North + TurnLeft = West");
    CHECK(F0700_MOVEMENT_TurnDirection_Compat(DIR_EAST, 0) == DIR_NORTH,
          "East + TurnLeft = North");
    CHECK(F0700_MOVEMENT_TurnDirection_Compat(DIR_SOUTH, 0) == DIR_EAST,
          "South + TurnLeft = East");
    CHECK(F0700_MOVEMENT_TurnDirection_Compat(DIR_WEST, 0) == DIR_SOUTH,
          "West + TurnLeft = South");

    /* Test 3: Turn actions from party */
    {
        struct MovementResult_Compat tr, tl;
        F0702_MOVEMENT_TryMove_Compat(&dungeon, &party, MOVE_TURN_RIGHT, &tr);
        F0702_MOVEMENT_TryMove_Compat(&dungeon, &party, MOVE_TURN_LEFT, &tl);

        CHECK(tr.resultCode == MOVE_TURN_ONLY, "Turn right result is TURN_ONLY");
        CHECK(tl.resultCode == MOVE_TURN_ONLY, "Turn left result is TURN_ONLY");
        CHECK(tr.newDirection == ((party.direction + 1) & 3),
              "Turn right gives correct direction");
        CHECK(tl.newDirection == ((party.direction + 3) & 3),
              "Turn left gives correct direction");
        CHECK(tr.newMapX == party.mapX && tr.newMapY == party.mapY,
              "Turn right does not change position");
    }

    /* Test 4: Movement stays in bounds */
    {
        int action;
        int allInBounds = 1;
        for (action = MOVE_FORWARD; action <= MOVE_LEFT; action++) {
            struct MovementResult_Compat mr;
            F0702_MOVEMENT_TryMove_Compat(&dungeon, &party, action, &mr);
            if (mr.resultCode == MOVE_OK) {
                const struct DungeonMapDesc_Compat* m = &dungeon.maps[party.mapIndex];
                if (mr.newMapX < 0 || mr.newMapX >= m->width ||
                    mr.newMapY < 0 || mr.newMapY >= m->height) {
                    allInBounds = 0;
                }
            }
        }
        CHECK(allInBounds, "All movement results stay within dungeon bounds");
    }

    /* Test 5: Champion serialisation round-trip */
    {
        struct ChampionState_Compat orig, restored;
        unsigned char buf[CHAMPION_SERIALIZED_SIZE];

        F0600_CHAMPION_InitEmpty_Compat(&orig);
        orig.present = 1;
        orig.portraitIndex = 3;
        memcpy(orig.name, "TESTNAME", 8);
        memcpy(orig.title, "SOURCE TITLE        ", 20);
        orig.sex = 'F';
        memcpy(orig.mirrorStatsText, "AADFACNAAAAP    ", 16);
        memcpy(orig.mirrorSkillsText, "DHCGCDCLCNCKCI  ", 16);
        orig.attributes[0] = 50; orig.attributes[1] = 40;
        orig.attributes[2] = 30; orig.attributes[3] = 60;
        orig.attributes[4] = 20; orig.attributes[5] = 15;
        orig.skillLevels[0] = 5; orig.skillLevels[1] = 3;
        orig.skillLevels[2] = 7; orig.skillLevels[3] = 2;
        orig.hp.current = 100; orig.hp.maximum = 120; orig.hp.shifted = 240;
        orig.stamina.current = 80; orig.stamina.maximum = 100; orig.stamina.shifted = 200;
        orig.mana.current = 50; orig.mana.maximum = 60; orig.mana.shifted = 120;
        orig.inventory[CHAMPION_SLOT_HAND_LEFT] = 0x1400;
        orig.load = 500; orig.maxLoad = 2000;
        orig.wounds = 0x03; orig.poisonDose = 10;
        orig.food = 200; orig.water = 180;
        orig.direction = DIR_SOUTH;

        F0602_CHAMPION_Serialize_Compat(&orig, buf, sizeof(buf));
        F0603_CHAMPION_Deserialize_Compat(&restored, buf, sizeof(buf));

        CHECK(memcmp(&orig, &restored, sizeof(orig)) == 0,
              "Champion round-trip serialisation is bit-identical");
    }

    /* Test 6: Party serialisation round-trip */
    {
        struct PartyState_Compat origP, restoredP;
        unsigned char buf[PARTY_SERIALIZED_SIZE];

        memcpy(&origP, &party, sizeof(origP));
        origP.championCount = 1;
        origP.champions[0].present = 1;
        origP.champions[0].portraitIndex = 5;
        memcpy(origP.champions[0].name, "HALK    ", 8);
        memcpy(origP.champions[0].title, "THE BARBARIAN       ", 20);
        origP.champions[0].sex = 'M';
        memcpy(origP.champions[0].mirrorStatsText, "AADFACNAAAAP    ", 16);
        memcpy(origP.champions[0].mirrorSkillsText, "DHCGCDCLCNCKCI  ", 16);
        origP.champions[0].hp.current = 90;
        origP.champions[0].hp.maximum = 100;
        origP.champions[0].hp.shifted = 200;

        F0604_PARTY_Serialize_Compat(&origP, buf, sizeof(buf));
        F0605_PARTY_Deserialize_Compat(&restoredP, buf, sizeof(buf));

        CHECK(memcmp(&origP, &restoredP, sizeof(origP)) == 0,
              "Party round-trip serialisation is bit-identical");
    }

    /* Test 7: DUNGEON.DAT champion mirror text identity parse */
    {
        struct ChampionState_Compat champ;
        F0600_CHAMPION_InitEmpty_Compat(&champ);
        CHECK(F0606_CHAMPION_ParseMirrorTextIdentity_Compat(
                  "STAMM|BLADECASTER||M|AADFACNAAAAP|DHCGCDCLCNCKCI|", &champ) == 1,
              "Champion mirror text identity parser accepts NAME|TITLE||... source format");
        CHECK(memcmp(champ.name, "STAMM   ", 8) == 0,
              "Champion mirror parser packs source Name[8]");
        CHECK(memcmp(champ.title, "BLADECASTER         ", 20) == 0,
              "Champion mirror parser packs source Title[20]");
        CHECK(champ.sex == 'M',
              "Champion mirror parser carries source sex byte");
        CHECK(memcmp(champ.mirrorStatsText, "AADFACNAAAAP    ", 16) == 0,
              "Champion mirror parser carries encoded source stat field");
        CHECK(memcmp(champ.mirrorSkillsText, "DHCGCDCLCNCKCI  ", 16) == 0,
              "Champion mirror parser carries encoded source skill field");
    }

    /* Test 8: Sensor identification */
    {
        struct SensorOnSquare_Compat sensorResult;
        int foundAnySensor = 0;
        int mi, mx, my;

        for (mi = 0; mi < (int)dungeon.header.mapCount && !foundAnySensor; mi++) {
            const struct DungeonMapDesc_Compat* mapN = &dungeon.maps[mi];
            for (mx = 0; mx < mapN->width && !foundAnySensor; mx++) {
                for (my = 0; my < mapN->height && !foundAnySensor; my++) {
                    if (F0703_MOVEMENT_IdentifySensorsOnSquare_Compat(
                            &dungeon, &things, mi, mx, my, &sensorResult)) {
                        foundAnySensor = 1;
                    }
                }
            }
        }

        CHECK(foundAnySensor, "At least one sensor found in dungeon");
        if (foundAnySensor) {
            CHECK(sensorResult.sensorIndex >= 0, "Sensor index is non-negative");
            CHECK(sensorResult.totalSensorsOnSquare >= 1, "At least 1 sensor on square");
            CHECK(sensorResult.targetMapX >= 0, "Sensor target X is non-negative");
            CHECK(sensorResult.targetMapY >= 0, "Sensor target Y is non-negative");
        }
    }

    fprintf(invariants, "\n");
    if (failCount == 0) {
        fprintf(invariants, "Status: PASS\n");
    } else {
        fprintf(invariants, "Status: FAIL (%d failures)\n", failCount);
    }

    fclose(invariants);

    /* Cleanup */
    F0504_DUNGEON_FreeThingData_Compat(&things);
    F0502_DUNGEON_FreeTileData_Compat(&dungeon);
    F0500_DUNGEON_FreeDatHeader_Compat(&dungeon);

    return failCount > 0 ? 1 : 0;
}
