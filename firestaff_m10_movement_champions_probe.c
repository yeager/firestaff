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
        memcpy(orig.mirrorInventoryText, "AAAAAA          ", 16);
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
        memcpy(origP.champions[0].mirrorInventoryText, "AAAAAA          ", 16);
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
        memcpy(orig.mirrorInventoryText, "AAAAAA          ", 16);
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
        memcpy(origP.champions[0].mirrorInventoryText, "AAAAAA          ", 16);
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
                  "STAMM|BLADECASTER||M|AADFACNAAAAP|DHCGCDCLCNCKCI|AAAAAA", &champ) == 1,
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
        CHECK(memcmp(champ.mirrorInventoryText, "AAAAAA          ", 16) == 0,
              "Champion mirror parser carries encoded source inventory field");
        CHECK(F0606_CHAMPION_ParseMirrorTextIdentity_Compat(
                  "STAMM|BLADECASTER||X|AADFACNAAAAP|DHCGCDCLCNCKCI|AAAAAA", &champ) == 0,
              "Champion mirror parser rejects non-source sex byte");
        CHECK(champ.sex == 0,
              "Champion mirror parser clears stale sex before rejecting malformed record");
        CHECK(F0606_CHAMPION_ParseMirrorTextIdentity_Compat(
                  "STAMM|BLADECASTER|M|AADFACNAAAAP", &champ) == 0,
              "Champion mirror parser rejects missing empty source field before sex");
    }

    /* Test 8: DUNGEON.DAT TextString-to-champion parser integration */
    {
        int tsIndex;
        int foundStamm = 0;
        for (tsIndex = 0; tsIndex < things.textStringCount; ++tsIndex) {
            struct ChampionState_Compat parsed;
            F0600_CHAMPION_InitEmpty_Compat(&parsed);
            if (F0607_CHAMPION_ParseMirrorTextString_Compat(&things, tsIndex, &parsed) &&
                memcmp(parsed.name, "STAMM   ", 8) == 0 &&
                memcmp(parsed.title, "BLADECASTER         ", 20) == 0 &&
                parsed.sex == 'M') {
                foundStamm = 1;
                break;
            }
        }
        CHECK(foundStamm,
              "DUNGEON.DAT TextString parser finds STAMM/BLADECASTER mirror record");
    }

    /* Test 9: DUNGEON.DAT champion mirror enumeration/recruitment helpers */
    {
        unsigned char stammName[CHAMPION_NAME_LENGTH] = { 'S','T','A','M','M',' ',' ',' ' };
        unsigned char halkName[CHAMPION_NAME_LENGTH] = { 'H','A','L','K',' ',' ',' ',' ' };
        unsigned char bogusName[CHAMPION_NAME_LENGTH] = { 'N','O','P','E',' ',' ',' ',' ' };
        struct PartyState_Compat recruited;
        int stammTextIndex = F0609_CHAMPION_FindMirrorTextStringByName_Compat(&things, stammName);
        memcpy(&recruited, &party, sizeof(recruited));

        CHECK(F0608_CHAMPION_CountMirrorTextStrings_Compat(&things) >= 20,
              "DUNGEON.DAT mirror parser counts many champion records");
        CHECK(stammTextIndex >= 0,
              "DUNGEON.DAT mirror parser finds STAMM by packed Name[8]");
        CHECK(F0609_CHAMPION_FindMirrorTextStringByName_Compat(&things, bogusName) < 0,
              "DUNGEON.DAT mirror parser rejects unknown packed Name[8]");
        CHECK(F0610_PARTY_AddChampionFromMirrorTextString_Compat(&things, stammTextIndex, &recruited) == 1,
              "Party can recruit champion identity from DUNGEON.DAT TextString index");
        CHECK(recruited.championCount == 1 && recruited.champions[0].present,
              "Mirror recruitment marks first party slot present and increments count");
        CHECK(recruited.activeChampionIndex == 0,
              "Mirror recruitment selects first recruited champion as active");
        CHECK(memcmp(recruited.champions[0].name, "STAMM   ", 8) == 0 &&
              memcmp(recruited.champions[0].title, "BLADECASTER         ", 20) == 0,
              "Mirror recruitment copies source name/title into party state");
        CHECK(recruited.champions[0].sex == 'M' &&
              memcmp(recruited.champions[0].mirrorStatsText, "AAELADCAAAAA    ", 16) == 0,
              "Mirror recruitment copies source sex and encoded stat field");
        CHECK(F0611_PARTY_AddChampionFromMirrorName_Compat(&things, halkName, &recruited) == 1 &&
              recruited.championCount == 2 &&
              memcmp(recruited.champions[1].name, "HALK    ", 8) == 0,
              "Party can recruit champion identity by packed Name[8]");
        CHECK(F0611_PARTY_AddChampionFromMirrorName_Compat(&things, stammName, &recruited) == 0 &&
              recruited.championCount == 2,
              "Mirror recruitment rejects duplicate champion name");
        CHECK(recruited.mapX == party.mapX && recruited.mapY == party.mapY &&
              recruited.direction == party.direction,
              "Mirror recruitment preserves party map position and facing");
        {
            struct PartyState_Compat fullParty;
            memcpy(&fullParty, &party, sizeof(fullParty));
            fullParty.championCount = CHAMPION_MAX_PARTY;
            CHECK(F0610_PARTY_AddChampionFromMirrorTextString_Compat(&things, stammTextIndex, &fullParty) == 0,
                  "Mirror recruitment rejects full party");
        }
    }

    /* Test 10: DUNGEON.DAT champion mirror collection helpers */
    {
        struct ChampionState_Compat mirrorRecords[32];
        int mirrorIndices[32];
        unsigned char titleBLADECASTER[CHAMPION_TITLE_LENGTH] = {
            'B','L','A','D','E','C','A','S','T','E','R',' ',' ',' ',' ',' ',' ',' ',' ',' '
        };
        unsigned char nameDAROOU[CHAMPION_NAME_LENGTH] = { 'D','A','R','O','O','U',' ',' ' };
        unsigned char nameHALK[CHAMPION_NAME_LENGTH] = { 'H','A','L','K',' ',' ',' ',' ' };
        struct PartyState_Compat byOrdinal;
        struct PartyState_Compat holeParty;
        int collected = F0612_CHAMPION_CollectMirrorTextStrings_Compat(&things, mirrorRecords, mirrorIndices, 32);

        memcpy(&byOrdinal, &party, sizeof(byOrdinal));
        memcpy(&holeParty, &party, sizeof(holeParty));
        holeParty.championCount = 1;
        holeParty.champions[1].present = 1;
        memcpy(holeParty.champions[1].name, "HALK    ", 8);

        CHECK(collected == F0608_CHAMPION_CountMirrorTextStrings_Compat(&things),
              "Mirror collection count matches mirror record counter");
        CHECK(collected == 24,
              "DM1 DUNGEON.DAT exposes 24 champion mirror records");
        CHECK(mirrorIndices[0] == F0613_CHAMPION_GetMirrorTextStringIndexByOrdinal_Compat(&things, 0),
              "Mirror ordinal 0 resolves to collected TextString index");
        CHECK(memcmp(mirrorRecords[0].name, "DAROOU  ", 8) == 0,
              "Mirror ordinal 0 carries first source champion name DAROOU");
        CHECK(F0613_CHAMPION_GetMirrorTextStringIndexByOrdinal_Compat(&things, collected) < 0,
              "Mirror ordinal helper rejects out-of-range ordinal");
        CHECK(F0615_CHAMPION_FindMirrorTextStringByTitle_Compat(&things, titleBLADECASTER) >= 0,
              "Mirror title finder locates BLADECASTER title");
        CHECK(F0616_CHAMPION_CountMirrorTextStringsBySex_Compat(&things, 'M') +
              F0616_CHAMPION_CountMirrorTextStringsBySex_Compat(&things, 'F') == collected,
              "Mirror sex counters sum to total champion records");
        CHECK(F0617_CHAMPION_HasMirrorTextStringByName_Compat(&things, nameDAROOU) == 1 &&
              F0617_CHAMPION_HasMirrorTextStringByName_Compat(&things, titleBLADECASTER) == 0,
              "Mirror name existence helper distinguishes packed names from titles");
        CHECK(F0614_PARTY_AddChampionFromMirrorOrdinal_Compat(&things, 0, &byOrdinal) == 1 &&
              memcmp(byOrdinal.champions[0].name, "DAROOU  ", 8) == 0,
              "Party can recruit champion identity by mirror ordinal");
        CHECK(F0611_PARTY_AddChampionFromMirrorName_Compat(&things, nameHALK, &holeParty) == 0 &&
              holeParty.championCount == 1,
              "Mirror recruitment detects duplicate name outside compact count range");
        {
            unsigned char nameSTAMM[CHAMPION_NAME_LENGTH] = { 'S','T','A','M','M',' ',' ',' ' };
            CHECK(F0611_PARTY_AddChampionFromMirrorName_Compat(&things, nameSTAMM, &holeParty) == 1 &&
                  holeParty.champions[0].present && memcmp(holeParty.champions[0].name, "STAMM   ", 8) == 0,
                  "Mirror recruitment fills first empty slot instead of blindly appending");
        }
    }

    /* Test 11: Champion mirror UI-bridge helpers */
    {
        unsigned char packedName[CHAMPION_NAME_LENGTH];
        unsigned char packedTitle[CHAMPION_TITLE_LENGTH];
        struct PartyState_Compat stringParty;
        struct PartyState_Compat slotProbe;
        memcpy(&stringParty, &party, sizeof(stringParty));
        memcpy(&slotProbe, &party, sizeof(slotProbe));

        CHECK(F0618_CHAMPION_PackName_Compat("STAMM", packedName) == 1 &&
              memcmp(packedName, "STAMM   ", 8) == 0,
              "Champion name pack helper pads source Name[8]");
        CHECK(F0619_CHAMPION_PackTitle_Compat("BLADECASTER", packedTitle) == 1 &&
              memcmp(packedTitle, "BLADECASTER         ", 20) == 0,
              "Champion title pack helper pads source Title[20]");
        CHECK(F0620_CHAMPION_FindMirrorTextStringByNameString_Compat(&things, "STAMM") >= 0,
              "Mirror finder accepts plain C champion name string");
        CHECK(F0621_PARTY_AddChampionFromMirrorNameString_Compat(&things, "STAMM", &stringParty) == 1 &&
              memcmp(stringParty.champions[0].name, "STAMM   ", 8) == 0,
              "Party can recruit champion identity by plain C name string");
        CHECK(F0622_CHAMPION_GetMirrorOrdinalByName_Compat(&things, packedName) >= 0,
              "Mirror ordinal lookup works by packed Name[8]");
        CHECK(F0623_CHAMPION_GetMirrorOrdinalByTitle_Compat(&things, packedTitle) ==
              F0622_CHAMPION_GetMirrorOrdinalByName_Compat(&things, packedName),
              "Mirror ordinal lookup by Title[20] matches same champion");
        CHECK(F0624_PARTY_GetNextFreeChampionSlot_Compat(&slotProbe) == 0,
              "Next-free party slot helper returns slot 0 for empty party");
        slotProbe.champions[0].present = 1;
        slotProbe.championCount = 1;
        CHECK(F0624_PARTY_GetNextFreeChampionSlot_Compat(&slotProbe) == 1,
              "Next-free party slot helper skips occupied slot");
        slotProbe.champions[1].present = 1;
        slotProbe.champions[2].present = 1;
        slotProbe.champions[3].present = 1;
        CHECK(F0625_PARTY_IsFull_Compat(&slotProbe) == 1,
              "Party full helper detects all champion slots occupied");
        CHECK(F0626_PARTY_ContainsChampionName_Compat(&stringParty, packedName) == 1,
              "Public party duplicate helper finds recruited packed Name[8]");
    }

    /* Test 12: Champion mirror presentation/state helpers */
    {
        struct ChampionState_Compat parsed;
        char nameBuf[16];
        char titleBuf[32];
        unsigned char badField[CHAMPION_MIRROR_FIELD_LENGTH] = { 'A','A','a',' ' };
        struct PartyState_Compat helperParty;
        F0600_CHAMPION_InitEmpty_Compat(&parsed);
        F0607_CHAMPION_ParseMirrorTextString_Compat(&things,
            F0620_CHAMPION_FindMirrorTextStringByNameString_Compat(&things, "STAMM"),
            &parsed);
        memcpy(&helperParty, &party, sizeof(helperParty));

        CHECK(F0627_CHAMPION_PackedTrimLength_Compat((const unsigned char*)"STAMM   ", 8) == 5,
              "Packed trim helper returns source name length without pad spaces");
        CHECK(F0628_CHAMPION_UnpackName_Compat(&parsed, nameBuf, sizeof(nameBuf)) == 5 && strcmp(nameBuf, "STAMM") == 0,
              "Champion unpack-name helper returns display name string");
        CHECK(F0629_CHAMPION_UnpackTitle_Compat(&parsed, titleBuf, sizeof(titleBuf)) == 11 && strcmp(titleBuf, "BLADECASTER") == 0,
              "Champion unpack-title helper returns display title string");
        CHECK(F0630_CHAMPION_MirrorStatsTextLength_Compat(&parsed) == 12,
              "Mirror stats encoded field reports trimmed source length");
        CHECK(F0631_CHAMPION_MirrorSkillsTextLength_Compat(&parsed) == 14,
              "Mirror skills encoded field reports trimmed source length");
        CHECK(F0632_CHAMPION_MirrorInventoryTextLength_Compat(&parsed) == 16,
              "Mirror inventory encoded field reports trimmed source length");
        CHECK(F0633_CHAMPION_IsEncodedMirrorField_Compat(parsed.mirrorStatsText, CHAMPION_MIRROR_FIELD_LENGTH) == 1,
              "Encoded mirror field validator accepts uppercase source field");
        CHECK(F0633_CHAMPION_IsEncodedMirrorField_Compat(badField, CHAMPION_MIRROR_FIELD_LENGTH) == 0,
              "Encoded mirror field validator rejects lowercase/non-source field");
        CHECK(F0634_CHAMPION_HasValidEncodedMirrorFields_Compat(&parsed) == 1,
              "Champion mirror record validates all encoded source fields");
        CHECK(F0635_CHAMPION_GetMirrorNameByOrdinal_Compat(&things, 0, nameBuf, sizeof(nameBuf)) > 0 && strcmp(nameBuf, "DAROOU") == 0,
              "Mirror ordinal can produce display name string");
        CHECK(F0636_CHAMPION_GetMirrorTitleByOrdinal_Compat(&things, 11, titleBuf, sizeof(titleBuf)) > 0 && strcmp(titleBuf, "BLADECASTER") == 0,
              "Mirror ordinal can produce display title string");
        CHECK(F0637_CHAMPION_FindMirrorTextStringByTitleString_Compat(&things, "BLADECASTER") >= 0,
              "Mirror finder accepts plain C title string");
        CHECK(F0638_PARTY_CountOccupiedChampionSlots_Compat(&helperParty) == 0,
              "Occupied-slot counter reports empty party");
        CHECK(F0639_PARTY_IsChampionSlotOccupied_Compat(&helperParty, 0) == 0,
              "Slot-occupied helper reports empty slot");
        F0621_PARTY_AddChampionFromMirrorNameString_Compat(&things, "STAMM", &helperParty);
        CHECK(F0640_PARTY_RecountChampionSlots_Compat(&helperParty) == 1,
              "Party recount helper reports recruited champion");
        CHECK(F0641_PARTY_HasActiveChampion_Compat(&helperParty) == 1,
              "Party active helper sees first recruited champion");
        CHECK(F0642_PARTY_SetActiveChampionIfPresent_Compat(&helperParty, 0) == 1,
              "Party active setter accepts present champion slot");
        CHECK(F0644_PARTY_GetChampionSlotByName_Compat(&helperParty, parsed.name) == 0,
              "Party slot lookup by packed name finds recruited champion");
        CHECK(F0645_PARTY_GetChampionSlotByNameString_Compat(&helperParty, "STAMM") == 0,
              "Party slot lookup by C name finds recruited champion");
        CHECK(F0646_PARTY_AddChampionFromMirrorOrdinalIfAbsent_Compat(&things, F0648_CHAMPION_GetMirrorOrdinalByNameString_Compat(&things, "STAMM"), &helperParty) == 1 &&
              F0638_PARTY_CountOccupiedChampionSlots_Compat(&helperParty) == 1,
              "Add-if-absent by mirror ordinal is idempotent for existing champion");
        CHECK(F0647_PARTY_AddChampionFromMirrorNameStringIfAbsent_Compat(&things, "HALK", &helperParty) == 1 &&
              F0638_PARTY_CountOccupiedChampionSlots_Compat(&helperParty) == 2,
              "Add-if-absent by C name recruits missing champion");
        CHECK(F0648_CHAMPION_GetMirrorOrdinalByNameString_Compat(&things, "STAMM") == 11,
              "Mirror ordinal by C name returns STAMM ordinal");
        CHECK(F0649_CHAMPION_GetMirrorOrdinalByTitleString_Compat(&things, "BLADECASTER") == 11,
              "Mirror ordinal by C title returns STAMM ordinal");
        CHECK(F0650_CHAMPION_GetMirrorTextStringIndexByNameString_Compat(&things, "STAMM") ==
              F0620_CHAMPION_FindMirrorTextStringByNameString_Compat(&things, "STAMM"),
              "TextString index by C name matches name finder");
        CHECK(F0651_CHAMPION_GetMirrorTextStringIndexByTitleString_Compat(&things, "BLADECASTER") ==
              F0637_CHAMPION_FindMirrorTextStringByTitleString_Compat(&things, "BLADECASTER"),
              "TextString index by C title matches title finder");
        CHECK(F0643_PARTY_ClearChampionSlot_Compat(&helperParty, 0) == 1 &&
              F0639_PARTY_IsChampionSlotOccupied_Compat(&helperParty, 0) == 0,
              "Clear champion slot helper removes recruited slot");
    }

    /* Test 13: Champion mirror catalog helpers */
    {
        struct ChampionMirrorCatalog_Compat catalog;
        struct PartyState_Compat catalogParty;
        struct ChampionState_Compat copied;
        char nameBuf[16];
        char titleBuf[32];
        int stammOrd;
        int stammTextIndex;
        memcpy(&catalogParty, &party, sizeof(catalogParty));
        CHECK(F0652_CHAMPION_BuildMirrorCatalog_Compat(&things, &catalog) == 24,
              "Mirror catalog builds 24 source champion records");
        CHECK(catalog.count == 24,
              "Mirror catalog count stores 24 records");
        CHECK(F0653_CHAMPION_MirrorCatalogFindByOrdinal_Compat(&catalog, 0) == 0,
              "Mirror catalog finds ordinal 0 at record 0");
        CHECK(F0654_CHAMPION_MirrorCatalogFindByName_Compat(&catalog, "STAMM") >= 0,
              "Mirror catalog finds STAMM by display name");
        CHECK(F0655_CHAMPION_MirrorCatalogFindByTitle_Compat(&catalog, "BLADECASTER") >= 0,
              "Mirror catalog finds BLADECASTER by display title");
        CHECK(F0656_CHAMPION_MirrorCatalogCountBySex_Compat(&catalog, 'M') +
              F0656_CHAMPION_MirrorCatalogCountBySex_Compat(&catalog, 'F') == catalog.count,
              "Mirror catalog sex counts sum to catalog count");
        CHECK(F0657_CHAMPION_MirrorCatalogCountTitled_Compat(&catalog) +
              F0658_CHAMPION_MirrorCatalogCountUntitled_Compat(&catalog) == catalog.count,
              "Mirror catalog titled/untitled counts sum to catalog count");
        CHECK(F0659_CHAMPION_MirrorCatalogNamesUnique_Compat(&catalog) == 1,
              "Mirror catalog packed names are unique");
        CHECK(F0660_CHAMPION_MirrorCatalogGetName_Compat(&catalog, 0, nameBuf, sizeof(nameBuf)) > 0 && strcmp(nameBuf, "DAROOU") == 0,
              "Mirror catalog gets display name by ordinal");
        stammOrd = F0648_CHAMPION_GetMirrorOrdinalByNameString_Compat(&things, "STAMM");
        CHECK(F0661_CHAMPION_MirrorCatalogGetTitle_Compat(&catalog, stammOrd, titleBuf, sizeof(titleBuf)) > 0 && strcmp(titleBuf, "BLADECASTER") == 0,
              "Mirror catalog gets display title by ordinal");
        stammTextIndex = F0662_CHAMPION_MirrorCatalogGetTextStringIndex_Compat(&catalog, stammOrd);
        CHECK(stammTextIndex == F0650_CHAMPION_GetMirrorTextStringIndexByNameString_Compat(&things, "STAMM"),
              "Mirror catalog gets original TextString index by ordinal");
        CHECK(F0663_CHAMPION_MirrorCatalogHasName_Compat(&catalog, "STAMM") == 1 &&
              F0663_CHAMPION_MirrorCatalogHasName_Compat(&catalog, "NOPE") == 0,
              "Mirror catalog has-name helper distinguishes known and unknown names");
        CHECK(F0664_CHAMPION_MirrorCatalogHasTitle_Compat(&catalog, "BLADECASTER") == 1,
              "Mirror catalog has-title helper finds BLADECASTER");
        CHECK(F0665_CHAMPION_MirrorCatalogFirstOrdinalBySex_Compat(&catalog, 'M') >= 0 &&
              F0666_CHAMPION_MirrorCatalogLastOrdinalBySex_Compat(&catalog, 'F') >= 0,
              "Mirror catalog first/last ordinal by sex helpers return records");
        CHECK(F0667_CHAMPION_MirrorCatalogOrdinalAfter_Compat(&catalog, 0) == 1 &&
              F0668_CHAMPION_MirrorCatalogOrdinalBefore_Compat(&catalog, 1) == 0,
              "Mirror catalog before/after ordinal helpers walk records");
        CHECK(F0669_CHAMPION_MirrorCatalogRecordValid_Compat(&catalog.records[0]) == 1,
              "Mirror catalog validates individual source record");
        CHECK(F0670_CHAMPION_MirrorCatalogAllRecordsValid_Compat(&catalog) == 1,
              "Mirror catalog validates all source records");
        CHECK(F0671_CHAMPION_MirrorCatalogRecruitOrdinal_Compat(&catalog, stammOrd, &catalogParty) == 1 &&
              memcmp(catalogParty.champions[0].name, "STAMM   ", 8) == 0,
              "Mirror catalog recruits by ordinal");
        CHECK(F0672_CHAMPION_MirrorCatalogRecruitName_Compat(&catalog, "HALK", &catalogParty) == 1 &&
              F0638_PARTY_CountOccupiedChampionSlots_Compat(&catalogParty) == 2,
              "Mirror catalog recruits by display name");
        CHECK(F0673_CHAMPION_MirrorCatalogRecruitOrdinalIfAbsent_Compat(&catalog, stammOrd, &catalogParty) == 1 &&
              F0638_PARTY_CountOccupiedChampionSlots_Compat(&catalogParty) == 2,
              "Mirror catalog recruit ordinal if-absent is idempotent");
        CHECK(F0674_CHAMPION_MirrorCatalogRecruitNameIfAbsent_Compat(&catalog, "HALK", &catalogParty) == 1 &&
              F0638_PARTY_CountOccupiedChampionSlots_Compat(&catalogParty) == 2,
              "Mirror catalog recruit name if-absent is idempotent");
        CHECK(F0675_CHAMPION_MirrorCatalogCopyRecord_Compat(&catalog, stammOrd, &copied) == 1 &&
              memcmp(copied.title, "BLADECASTER         ", 20) == 0,
              "Mirror catalog copies champion source record by ordinal");
        CHECK(F0676_CHAMPION_MirrorCatalogGetOrdinalForTextStringIndex_Compat(&catalog, stammTextIndex) == stammOrd,
              "Mirror catalog resolves ordinal from TextString index");
    }

    /* Test 14: Sensor identification */
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
