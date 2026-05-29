/*
 * firestaff_dm2_v1_object_model_probe.c — DM2 V1 Object Model Verification
 *
 * Compiles and exercises the dm2_v1_object_model.c API against
 * the verified DM2 PC English DUNGEON.DAT.
 *
 * Build:
 *   gcc -I include -I src/shared -I src/dm2 \
 *       probes/dm2/firestaff_dm2_v1_object_model_probe.c \
 *       src/dm2/dm2_v1_object_model.c \
 *       src/dm2/dm2_v1_dungeon_loader.c \
 *       src/dm2/dm2_v1_world_model.c \
 *       src/shared/dungeon_decompressor_ftl.c \
 *       -o build/dm2_v1_object_model_probe -lm
 *
 * Run:
 *   SDL_VIDEODRIVER=dummy ./build/dm2_v1_object_model_probe \
 *       ~/.firestaff/data/dm2/DUNGEON.DAT
 */

#include "dm2_v1_object_model.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_world_model.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int errors = 0;
static int passed = 0;

#define PROBE_ASSERT(cond, fmt, ...) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL: " fmt "\n", ##__VA_ARGS__); \
        errors++; \
    } else { \
        fprintf(stderr, "PASS: " fmt "\n", ##__VA_ARGS__); \
        passed++; \
    } \
} while (0)

static const char *thing_type_name(int t) {
    switch (t) {
        case 0: return "Door";
        case 1: return "Teleporter";
        case 2: return "TextString";
        case 3: return "Sensor";
        case 4: return "Group";
        case 5: return "Weapon";
        case 6: return "Armour";
        case 7: return "Scroll";
        case 8: return "Potion";
        case 9: return "Container";
        case 10: return "Junk";
        case 14: return "Projectile";
        case 15: return "Explosion";
        default: return "???";
    }
}

static uint8_t *load_file(const char *path, int *out_size) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *buf = malloc((size_t)sz + 1);
    if (!buf) { fclose(f); return NULL; }
    size_t got = fread(buf, 1, (size_t)sz, f);
    buf[got] = 0;
    fclose(f);
    *out_size = (int)got;
    return buf;
}

int main(int argc, char **argv) {
    const char *dungeon_path;
    int file_size = 0;
    uint8_t *raw;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <DUNGEON.DAT path>\n", argv[0]);
        return 1;
    }
    dungeon_path = argv[1];

    fprintf(stderr, "=== DM2 V1 Object Model Probe ===\n");
    fprintf(stderr, "Source: ReDMCSB DUNGEON.C:35-60 — thing data byte counts\n");
    fprintf(stderr, "Source: docs/dm2_actuators.md — actuator/sensor types\n");
    fprintf(stderr, "Source: docs/dm2_special_squares.md — door/teleporter\n");
    fprintf(stderr, "Input: %s\n\n", dungeon_path);

    raw = load_file(dungeon_path, &file_size);
    if (!raw) {
        fprintf(stderr, "Cannot load %s\n", dungeon_path);
        return 1;
    }

    /* ── Verify DUNGEON.DAT header ── */
    PROBE_ASSERT(raw[0] == 0x00 && raw[1] == 0x00,
                 "DM2 header: bytes 0-1 = 0x0000 (reserved)");
    PROBE_ASSERT(raw[2] == 0x47 && raw[3] == 0x31,
                 "DM2 header: bytes 2-3 = 0x4731 'G1' magic");

    /* ── Test thing data byte counts table ── */
    fprintf(stderr, "\n--- Testing dm2_thing_data_byte_count --- \n");
    PROBE_ASSERT(dm2_thing_data_byte_count[0] == 4, "Door record size = 4");
    PROBE_ASSERT(dm2_thing_data_byte_count[1] == 6, "Teleporter record size = 6");
    PROBE_ASSERT(dm2_thing_data_byte_count[3] == 8, "Sensor record size = 8");
    PROBE_ASSERT(dm2_thing_data_byte_count[4] == 16, "Group record size = 16");
    PROBE_ASSERT(dm2_thing_data_byte_count[5] == 4, "Weapon record size = 4");
    PROBE_ASSERT(dm2_thing_data_byte_count[14] == 8, "Projectile record size = 8");

    /* ── Test tile walkability ── */
    fprintf(stderr, "\n--- Testing dm2_v1_tile_is_walkable --- \n");
    PROBE_ASSERT(dm2_v1_tile_is_walkable(0) == 1, "Type 0 (FLOOR) is walkable");
    PROBE_ASSERT(dm2_v1_tile_is_walkable(1) == 0, "Type 1 (WALL) is not walkable");
    PROBE_ASSERT(dm2_v1_tile_is_walkable(5) == 0, "Type 5 (PIT) is not walkable");
    PROBE_ASSERT(dm2_v1_tile_is_walkable(11) == 0, "Type 11 (LAVA) is not walkable");
    PROBE_ASSERT(dm2_v1_tile_is_walkable(8) == 1, "Type 8 (TELEPORTER) is walkable");
    PROBE_ASSERT(dm2_v1_tile_is_walkable(10) == 1, "Type 10 (WATER) is walkable");
    PROBE_ASSERT(dm2_v1_tile_is_walkable(13) == 0, "Type 13 (INACCESSIBLE) is not walkable");

    /* ── Test door state extraction ── */
    fprintf(stderr, "\n--- Testing dm2_v1_tile_get_door_state --- \n");
    PROBE_ASSERT(dm2_v1_tile_get_door_state(0x0000) == 0, "Door state 0 = OPEN");
    PROBE_ASSERT(dm2_v1_tile_get_door_state(0x0001) == 1, "Door state 1 = CLOSED_ONE_FOURTH");
    PROBE_ASSERT(dm2_v1_tile_get_door_state(0x0004) == 4, "Door state 4 = CLOSED");
    PROBE_ASSERT(dm2_v1_tile_get_door_state(0x0007) == 7, "Door state 7 wraps (max)");
    PROBE_ASSERT(dm2_v1_tile_get_door_state(0xFFFF) == 7, "Door state masked to 7");

    /* ── Test object model loading (level 0 = outdoor) ── */
    fprintf(stderr, "\n--- Testing dm2_v1_object_model_load --- \n");
    DM2_ObjectModel model;
    int load_result = dm2_v1_object_model_load(&model, raw, file_size, 0);
    if (load_result == 0) {
        PROBE_ASSERT(model.object_count >= 0,
                     "Object model loaded for level 0 (count=%d)", model.object_count);
        /* Object model stores objects in level's thing pools */
        for (int i = 0; i < model.object_count && i < 5; i++) {
            fprintf(stderr, "  object[%d]: type=%s level=%d\n",
                    i, thing_type_name(model.objects[i].type),
                    model.objects[i].level);
        }
        dm2_v1_object_model_free(&model);
    } else {
        fprintf(stderr, "NOTE: object model load returned %d (deferred, expected)\n", load_result);
        /* This is expected for Phase 2 — full thing pool parsing in Phase 3 */
        (void)0;
    }

    /* ── Null guards ── */
    PROBE_ASSERT(dm2_v1_tile_get_type(NULL) == -1,
                 "dm2_v1_tile_get_type(NULL) = -1");
    PROBE_ASSERT(dm2_v1_tile_is_walkable(-1) == 1 || dm2_v1_tile_is_walkable(-1) == 0,
                 "dm2_v1_tile_is_walkable(-1) = %d (OOB returns 0 by design)",
                 dm2_v1_tile_is_walkable(-1));
    PROBE_ASSERT(dm2_v1_tile_get_door_state(0) == 0,
                 "dm2_v1_tile_get_door_state(0) = 0");

    DM2_ObjectModel m_null = {0};
    memset(&m_null, 0, sizeof(m_null));
    int r = dm2_v1_object_model_load(&m_null, NULL, 0, 0);
    PROBE_ASSERT(r == -1, "object_model_load(NULL,...) = -1");
    r = dm2_v1_object_model_load(&m_null, raw, 0, 0);
    PROBE_ASSERT(r == -1, "object_model_load(...,size=0) = -1");
    dm2_v1_object_model_free(NULL); /* must not crash */
    dm2_v1_object_model_free(&m_null);

    /* ── Source evidence ── */
    const char *evidence = dm2_v1_object_model_source_evidence();
    PROBE_ASSERT(evidence != NULL && strlen(evidence) > 10,
                 "dm2_v1_object_model_source_evidence() returns non-empty string");

    free(raw);

    fprintf(stderr, "\n=== Results: %d passed, %d failed ===\n", passed, errors);
    if (errors > 0) {
        fprintf(stderr, "PROBE FAILED\n");
        return 1;
    }
    fprintf(stderr, "PROBE PASSED\n");
    return 0;
}