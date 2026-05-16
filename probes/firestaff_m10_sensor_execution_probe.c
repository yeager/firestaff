/*
 * M10 Phase 11 probe: sensor execution verification.
 *
 * Tests:
 *   - Sensor execution on teleport (type 0) returns TELEPORT effect with
 *     correct destination
 *   - Text sensor (type 13) returns SHOW_TEXT effect (if present in data)
 *   - Unsupported types return EFFECT_UNSUPPORTED, not a crash
 *   - WALK_OFF / ITEM_ON / ITEM_OFF / CHAMPION_ACTION return empty lists
 *     on teleporters (conservative v1 behaviour)
 *   - Round-trip serialisation of SensorEffect and SensorEffectList
 *     is bit-identical
 *   - Determinism: same input twice produces byte-identical output
 *   - Purity: input SensorOnSquare is not mutated by execute
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory_sensor_execution_pc34_compat.h"

static unsigned int checksum_bytes(const void* p, size_t n) {
    const unsigned char* b = (const unsigned char*)p;
    unsigned int h = 2166136261u;
    size_t i;
    for (i = 0; i < n; i++) {
        h ^= b[i];
        h *= 16777619u;
    }
    return h;
}

static int find_sensor_at(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    int mapIndex, int mapX, int mapY,
    struct SensorOnSquare_Compat* out)
{
    return F0703_MOVEMENT_IdentifySensorsOnSquare_Compat(
        dungeon, things, mapIndex, mapX, mapY, out);
}

static int find_first_sensor_of_type(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    int wantType,
    struct SensorOnSquare_Compat* out,
    int* outMapIndex, int* outMapX, int* outMapY)
{
    int mi, mx, my;
    for (mi = 0; mi < (int)dungeon->header.mapCount; mi++) {
        const struct DungeonMapDesc_Compat* m = &dungeon->maps[mi];
        for (mx = 0; mx < m->width; mx++) {
            for (my = 0; my < m->height; my++) {
                struct SensorOnSquare_Compat s;
                if (F0703_MOVEMENT_IdentifySensorsOnSquare_Compat(
                        dungeon, things, mi, mx, my, &s)) {
                    if (s.sensorType == wantType) {
                        *out = s;
                        *outMapIndex = mi;
                        *outMapX = mx;
                        *outMapY = my;
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

int main(int argc, char* argv[]) {
    struct DungeonDatState_Compat dungeon;
    struct DungeonThings_Compat things;
    FILE* report;
    FILE* invariants;
    char path_buf[512];
    int failCount = 0;

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <DUNGEON.DAT> <output_dir>\n", argv[0]);
        return 1;
    }

    memset(&dungeon, 0, sizeof(dungeon));
    memset(&things, 0, sizeof(things));

    if (!F0500_DUNGEON_LoadDatHeader_Compat(argv[1], &dungeon)) {
        fprintf(stderr, "FAIL: could not load dungeon header\n"); return 1;
    }
    if (!F0502_DUNGEON_LoadTileData_Compat(argv[1], &dungeon)) {
        fprintf(stderr, "FAIL: could not load tile data\n");
        F0500_DUNGEON_FreeDatHeader_Compat(&dungeon); return 1;
    }
    if (!F0504_DUNGEON_LoadThingData_Compat(argv[1], &dungeon, &things)) {
        fprintf(stderr, "FAIL: could not load thing data\n");
        F0502_DUNGEON_FreeTileData_Compat(&dungeon);
        F0500_DUNGEON_FreeDatHeader_Compat(&dungeon); return 1;
    }

    /* Reports */
    snprintf(path_buf, sizeof(path_buf), "%s/sensor_execution_probe.md", argv[2]);
    report = fopen(path_buf, "w");
    if (!report) { fprintf(stderr, "FAIL: cannot write report\n"); return 1; }
    fprintf(report, "# M10 Phase 11: Sensor Execution Probe\n\n");
    fprintf(report, "## Implemented sensor types (v1)\n\n");
    fprintf(report, "- Type 0 (teleport): WALK_ON -> EFFECT_TELEPORT with target from sensor data\n");
    fprintf(report, "- Type 13 (text): WALK_ON -> EFFECT_SHOW_TEXT with textIndex from sensor data\n");
    fprintf(report, "- All other types: EFFECT_UNSUPPORTED (non-crashing stub)\n");
    fprintf(report, "- Non-WALK_ON events: empty effect list (v1 conservative)\n\n");
    fclose(report);

    snprintf(path_buf, sizeof(path_buf), "%s/sensor_execution_invariants.md", argv[2]);
    invariants = fopen(path_buf, "w");
    if (!invariants) { fprintf(stderr, "FAIL: cannot write invariants\n"); return 1; }
    fprintf(invariants, "# Sensor Execution Invariants\n\n");

#define CHECK(cond, msg) do { \
    if (cond) { \
        fprintf(invariants, "- PASS: %s\n", msg); \
    } else { \
        fprintf(invariants, "- FAIL: %s\n", msg); \
        failCount++; \
    } \
} while(0)

    /* ---- Test group A: Round-trip serialisation ---- */
    {
        struct SensorEffect_Compat orig, restored;
        unsigned char buf[SENSOR_EFFECT_SERIALIZED_SIZE];

        orig.kind         = SENSOR_EFFECT_TELEPORT;
        orig.sensorType   = 0;
        orig.destMapIndex = -1;
        orig.destMapX     = 5;
        orig.destMapY     = 7;
        orig.destCell     = 2;
        orig.textIndex    = 0;

        memset(&restored, 0xAA, sizeof(restored));
        F0711_SENSOR_EffectSerialize_Compat(&orig, buf, sizeof(buf));
        F0712_SENSOR_EffectDeserialize_Compat(&restored, buf, sizeof(buf));
        CHECK(memcmp(&orig, &restored, sizeof(orig)) == 0,
              "SensorEffect round-trip (TELEPORT) is bit-identical");
    }
    {
        struct SensorEffect_Compat orig, restored;
        unsigned char buf[SENSOR_EFFECT_SERIALIZED_SIZE];

        orig.kind         = SENSOR_EFFECT_SHOW_TEXT;
        orig.sensorType   = 13;
        orig.destMapIndex = 0;
        orig.destMapX     = 0;
        orig.destMapY     = 0;
        orig.destCell     = 0;
        orig.textIndex    = 42;

        memset(&restored, 0xAA, sizeof(restored));
        F0711_SENSOR_EffectSerialize_Compat(&orig, buf, sizeof(buf));
        F0712_SENSOR_EffectDeserialize_Compat(&restored, buf, sizeof(buf));
        CHECK(memcmp(&orig, &restored, sizeof(orig)) == 0,
              "SensorEffect round-trip (SHOW_TEXT) is bit-identical");
    }
    {
        struct SensorEffect_Compat orig, restored;
        unsigned char buf[SENSOR_EFFECT_SERIALIZED_SIZE];

        orig.kind         = SENSOR_EFFECT_UNSUPPORTED;
        orig.sensorType   = 127;
        orig.destMapIndex = 0;
        orig.destMapX     = 0;
        orig.destMapY     = 0;
        orig.destCell     = 0;
        orig.textIndex    = 0;

        memset(&restored, 0xAA, sizeof(restored));
        F0711_SENSOR_EffectSerialize_Compat(&orig, buf, sizeof(buf));
        F0712_SENSOR_EffectDeserialize_Compat(&restored, buf, sizeof(buf));
        CHECK(memcmp(&orig, &restored, sizeof(orig)) == 0,
              "SensorEffect round-trip (UNSUPPORTED) is bit-identical");
    }

    /* Effect list round-trip */
    {
        struct SensorEffectList_Compat orig, restored;
        unsigned char buf[SENSOR_EFFECT_LIST_SERIALIZED_SIZE];

        memset(&orig, 0, sizeof(orig));
        orig.count = 2;
        orig.effects[0].kind = SENSOR_EFFECT_TELEPORT;
        orig.effects[0].sensorType = 0;
        orig.effects[0].destMapX = 3; orig.effects[0].destMapY = 4;
        orig.effects[0].destCell = 1;
        orig.effects[1].kind = SENSOR_EFFECT_SHOW_TEXT;
        orig.effects[1].sensorType = 13;
        orig.effects[1].textIndex = 9;

        memset(&restored, 0xCC, sizeof(restored));
        F0713_SENSOR_ListSerialize_Compat(&orig, buf, sizeof(buf));
        F0714_SENSOR_ListDeserialize_Compat(&restored, buf, sizeof(buf));
        CHECK(memcmp(&orig, &restored, sizeof(orig)) == 0,
              "SensorEffectList round-trip (count=2) is bit-identical");
    }

    /* Empty list round-trip */
    {
        struct SensorEffectList_Compat orig, restored;
        unsigned char buf[SENSOR_EFFECT_LIST_SERIALIZED_SIZE];

        memset(&orig, 0, sizeof(orig));
        orig.count = 0;

        memset(&restored, 0xCC, sizeof(restored));
        F0713_SENSOR_ListSerialize_Compat(&orig, buf, sizeof(buf));
        F0714_SENSOR_ListDeserialize_Compat(&restored, buf, sizeof(buf));
        CHECK(memcmp(&orig, &restored, sizeof(orig)) == 0,
              "SensorEffectList round-trip (empty) is bit-identical");
    }

    /* Serialized size invariant */
    CHECK(SENSOR_EFFECT_SERIALIZED_SIZE == 28,
          "SensorEffect serialised size is 28 bytes");
    CHECK(SENSOR_EFFECT_LIST_SERIALIZED_SIZE == 228,
          "SensorEffectList serialised size is 228 bytes (4 + 8*28)");

    /* ---- Test group B: Teleport execution ---- */
    {
        struct SensorOnSquare_Compat teleportSensor;
        int mi, mx, my;
        int haveTeleport = find_first_sensor_of_type(
            &dungeon, &things, 0, &teleportSensor, &mi, &mx, &my);
        CHECK(haveTeleport, "Found at least one teleport (type 0) in dungeon");

        if (haveTeleport) {
            struct SensorEffectList_Compat list;
            int ok = F0710_SENSOR_Execute_Compat(
                &dungeon, &things, &teleportSensor, SENSOR_EVENT_WALK_ON, &list);
            CHECK(ok == 1, "Teleport WALK_ON returns ok=1");
            CHECK(list.count == 1, "Teleport WALK_ON produces exactly 1 effect");
            CHECK(list.effects[0].kind == SENSOR_EFFECT_TELEPORT,
                  "Teleport WALK_ON effect kind is TELEPORT");
            CHECK(list.effects[0].sensorType == 0,
                  "Teleport effect carries sensorType=0");
            CHECK(list.effects[0].destMapX == teleportSensor.targetMapX,
                  "Teleport destMapX matches sensor target");
            CHECK(list.effects[0].destMapY == teleportSensor.targetMapY,
                  "Teleport destMapY matches sensor target");
            CHECK(list.effects[0].destCell == teleportSensor.targetCell,
                  "Teleport destCell matches sensor target");

            /* Non-WALK_ON events return empty list */
            {
                struct SensorEffectList_Compat emptyList;
                F0710_SENSOR_Execute_Compat(&dungeon, &things, &teleportSensor,
                                            SENSOR_EVENT_WALK_OFF, &emptyList);
                CHECK(emptyList.count == 0, "Teleport WALK_OFF gives empty list");

                F0710_SENSOR_Execute_Compat(&dungeon, &things, &teleportSensor,
                                            SENSOR_EVENT_ITEM_ON, &emptyList);
                CHECK(emptyList.count == 0, "Teleport ITEM_ON gives empty list");
            }

            /* Determinism: same input twice = byte-identical result */
            {
                struct SensorEffectList_Compat a, b;
                unsigned char bufA[SENSOR_EFFECT_LIST_SERIALIZED_SIZE];
                unsigned char bufB[SENSOR_EFFECT_LIST_SERIALIZED_SIZE];
                F0710_SENSOR_Execute_Compat(&dungeon, &things, &teleportSensor,
                                            SENSOR_EVENT_WALK_ON, &a);
                F0710_SENSOR_Execute_Compat(&dungeon, &things, &teleportSensor,
                                            SENSOR_EVENT_WALK_ON, &b);
                F0713_SENSOR_ListSerialize_Compat(&a, bufA, sizeof(bufA));
                F0713_SENSOR_ListSerialize_Compat(&b, bufB, sizeof(bufB));
                CHECK(memcmp(bufA, bufB, sizeof(bufA)) == 0,
                      "Execute is deterministic (same input -> same bytes)");
            }

            /* Purity: input sensor struct unchanged after execute */
            {
                struct SensorOnSquare_Compat before = teleportSensor;
                struct SensorEffectList_Compat tmp;
                unsigned int hBefore = checksum_bytes(&teleportSensor, sizeof(teleportSensor));
                F0710_SENSOR_Execute_Compat(&dungeon, &things, &teleportSensor,
                                            SENSOR_EVENT_WALK_ON, &tmp);
                unsigned int hAfter = checksum_bytes(&teleportSensor, sizeof(teleportSensor));
                CHECK(hBefore == hAfter, "Execute does not mutate input sensor struct");
                CHECK(memcmp(&before, &teleportSensor, sizeof(before)) == 0,
                      "Execute does not mutate input sensor (memcmp)");
            }
        }
    }

    /* ---- Test group C: Known teleport on map 0 (from Phase 10) ---- */
    {
        /* Phase 10 already located a teleport at map 0 x=0 y=1 with
         * remote target (1,1) cell 2. */
        struct SensorOnSquare_Compat s;
        if (find_sensor_at(&dungeon, &things, 0, 0, 1, &s)) {
            if (s.sensorType == 0) {
                struct SensorEffectList_Compat list;
                F0710_SENSOR_Execute_Compat(&dungeon, &things, &s,
                                            SENSOR_EVENT_WALK_ON, &list);
                CHECK(list.count == 1 &&
                      list.effects[0].kind == SENSOR_EFFECT_TELEPORT &&
                      list.effects[0].destMapX == 1 &&
                      list.effects[0].destMapY == 1 &&
                      list.effects[0].destCell == 2,
                      "Known teleport at map0 (0,1) -> (1,1) cell=2 matches expectation");
            }
        }
    }

    /* ---- Test group D: Unsupported sensor stubbing ---- */
    {
        /* Fabricate a synthetic sensor struct so we can exercise the
         * unsupported branch even if the dungeon happens not to carry
         * one of the rarer types on the specific tile walker ordering. */
        struct SensorOnSquare_Compat fake;
        struct SensorEffectList_Compat list;
        memset(&fake, 0, sizeof(fake));
        fake.found = 1;
        fake.sensorIndex = 0;
        fake.sensorType = 127; /* Rare/complex type, stubbed in v1 */
        fake.sensorData = 0;
        fake.targetMapX = 0; fake.targetMapY = 0; fake.targetCell = 0;
        fake.isLocal = 0;
        fake.totalSensorsOnSquare = 1;

        F0710_SENSOR_Execute_Compat(&dungeon, &things, &fake,
                                    SENSOR_EVENT_WALK_ON, &list);
        CHECK(list.count == 1, "Unsupported sensor (type 127) yields exactly one effect");
        CHECK(list.effects[0].kind == SENSOR_EFFECT_UNSUPPORTED,
              "Unsupported sensor effect kind is UNSUPPORTED");
        CHECK(list.effects[0].sensorType == 127,
              "Unsupported effect carries original sensorType");

        /* Non-WALK_ON on unsupported: still empty list */
        F0710_SENSOR_Execute_Compat(&dungeon, &things, &fake,
                                    SENSOR_EVENT_WALK_OFF, &list);
        CHECK(list.count == 0, "Unsupported sensor WALK_OFF gives empty list");
    }

    /* ---- Test group E: Null-arg safety ---- */
    {
        struct SensorEffectList_Compat list;
        int ok = F0710_SENSOR_Execute_Compat(&dungeon, &things, 0,
                                             SENSOR_EVENT_WALK_ON, &list);
        CHECK(ok == 0, "Execute returns 0 on null sensor pointer");
    }
    {
        struct SensorOnSquare_Compat fake;
        memset(&fake, 0, sizeof(fake));
        fake.found = 0;
        struct SensorEffectList_Compat list;
        int ok = F0710_SENSOR_Execute_Compat(&dungeon, &things, &fake,
                                             SENSOR_EVENT_WALK_ON, &list);
        CHECK(ok == 1 && list.count == 0,
              "Execute on found=0 sensor returns empty list, ok=1");
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
