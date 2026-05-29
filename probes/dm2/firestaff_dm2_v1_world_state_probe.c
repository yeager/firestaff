/*
 * firestaff_dm2_v1_world_state_probe.c — DM2 V1 World State Verification
 *
 * Compiles and exercises the dm2_v1_world_state.c API against
 * the verified DM2 PC English DUNGEON.DAT.
 *
 * Build:
 *   gcc -I include -I src/shared -I src/dm2 \
 *       probes/dm2/firestaff_dm2_v1_world_state_probe.c \
 *       src/dm2/dm2_v1_world_state.c \
 *       src/dm2/dm2_v1_dungeon_loader.c \
 *       src/dm2/dm2_v1_world_model.c \
 *       src/shared/dungeon_decompressor_ftl.c \
 *       -o build/dm2_v1_world_state_probe -lm
 *
 * Run:
 *   SDL_VIDEODRIVER=dummy ./build/dm2_v1_world_state_probe \
 *       ~/.firestaff/data/dm2/DUNGEON.DAT
 */

#include "dm2_v1_world_state.h"
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

    fprintf(stderr, "=== DM2 V1 World State Probe ===\n");
    fprintf(stderr, "Source: docs/dm2_save_format.md — SUPPRESS compression\n");
    fprintf(stderr, "Source: docs/dm2_quest.md — quest flags and phases\n");
    fprintf(stderr, "Source: docs/dm2_party_state.md — party/champion state\n");
    fprintf(stderr, "Source: SKULL.ASM T520 — party placement from dungeon\n");
    fprintf(stderr, "Input: %s\n\n", dungeon_path);

    raw = load_file(dungeon_path, &file_size);
    if (!raw) {
        fprintf(stderr, "Cannot load %s\n", dungeon_path);
        return 1;
    }

    PROBE_ASSERT(file_size == 39437,
                 "DM2 PC English DUNGEON.DAT size 39437 (got %d)", file_size);

    /* ── Test world state from dungeon ── */
    fprintf(stderr, "\n--- Testing dm2_v1_world_state_new_from_dungeon --- \n");
    DM2_WorldState *state = dm2_v1_world_state_new_from_dungeon(raw, (size_t)file_size);
    PROBE_ASSERT(state != NULL,
                 "dm2_v1_world_state_new_from_dungeon returns non-NULL");
    if (state) {
        PROBE_ASSERT(state->party.leader_index == 0,
                     "Default leader index = 0");
        PROBE_ASSERT(state->quest_count == 0,
                     "New game: quest_count = 0");
        PROBE_ASSERT(state->game_tick == 0,
                     "New game: game_tick = 0");
        PROBE_ASSERT(state->timer_count == 0,
                     "New game: timer_count = 0");
        PROBE_ASSERT(state->current_level >= 0,
                     "current_level >= 0 (got %d)", state->current_level);

        /* Initial party position from DUNGEON.DAT */
        fprintf(stderr, "  outdoor_level_count=%d current_level=%d\n",
                state->outdoor_level_count, state->current_level);
    }

    /* ── Test quest flag accessors ── */
    fprintf(stderr, "\n--- Testing quest flag accessors --- \n");
    if (state) {
        dm2_v1_world_state_set_quest_flag(state, 0, 1);
        dm2_v1_world_state_set_quest_flag(state, 5, 42);
        PROBE_ASSERT(dm2_v1_world_state_get_quest_flag(state, 0) == 1,
                     "Quest flag 0 = 1 (set)");
        PROBE_ASSERT(dm2_v1_world_state_get_quest_flag(state, 5) == 42,
                     "Quest flag 5 = 42 (set)");
        PROBE_ASSERT(dm2_v1_world_state_get_quest_flag(state, 99) == 0,
                     "Quest flag 99 = 0 (unset)");
        dm2_v1_world_state_set_quest_flag(state, 0, 0);

        /* Out-of-bounds accessors */
        int bad_flag = dm2_v1_world_state_get_quest_flag(state, -1);
        PROBE_ASSERT(bad_flag == 0, "Quest flag -1 = 0 (OOB)");
        int bad_hp = dm2_v1_world_state_get_champion_hp(state, -1);
        PROBE_ASSERT(bad_hp == -1, "Champion HP -1 = -1 (OOB)");
        int bad_hp2 = dm2_v1_world_state_get_champion_hp(state, 99);
        PROBE_ASSERT(bad_hp2 == -1, "Champion HP 99 = -1 (OOB)");
    }

    /* ── Test SUPPRESS serialize (stub) ── */
    fprintf(stderr, "\n--- Testing dm2_v1_world_state_serialize --- \n");
    if (state) {
        size_t ser_size = 0;
        uint8_t *ser_buf = dm2_v1_world_state_serialize(state, &ser_size);
        PROBE_ASSERT(ser_buf != NULL,
                     "dm2_v1_world_state_serialize returns non-NULL");
        PROBE_ASSERT(ser_size > 0,
                     "Serialized size > 0 (got %zu)", ser_size);
        if (ser_buf) {
            /* Check save slot magic markers (BEET/DEAD) */
            PROBE_ASSERT(ser_buf[38] == 0xBE && ser_buf[39] == 0xEF,
                         "Save slot magic BEET present at offset 38");
            PROBE_ASSERT(ser_buf[40] == 0xDE && ser_buf[41] == 0xAD,
                         "Save slot magic DEAD present at offset 40");
            free(ser_buf);
        }
    }

    /* ── Null guards ── */
    DM2_WorldState *null_state = dm2_v1_world_state_new_from_dungeon(NULL, 0);
    PROBE_ASSERT(null_state == NULL,
                 "dm2_v1_world_state_new_from_dungeon(NULL,0) = NULL");
    null_state = dm2_v1_world_state_new_from_dungeon(raw, 0);
    PROBE_ASSERT(null_state == NULL,
                 "dm2_v1_world_state_new_from_dungeon(...,size=0) = NULL");
    null_state = dm2_v1_world_state_load_from_file("/nonexistent/path");
    PROBE_ASSERT(null_state == NULL,
                 "dm2_v1_world_state_load_from_file(bad_path) = NULL");
    null_state = dm2_v1_world_state_load_from_mem(NULL, 0);
    PROBE_ASSERT(null_state == NULL,
                 "dm2_v1_world_state_load_from_mem(NULL,0) = NULL");

    dm2_v1_world_state_free(NULL); /* must not crash */
    dm2_v1_world_state_free(state);

    /* ── Source evidence ── */
    const char *evidence = dm2_v1_world_state_source_evidence();
    PROBE_ASSERT(evidence != NULL && strlen(evidence) > 10,
                 "dm2_v1_world_state_source_evidence() returns non-empty string");

    free(raw);

    fprintf(stderr, "\n=== Results: %d passed, %d failed ===\n", passed, errors);
    if (errors > 0) {
        fprintf(stderr, "PROBE FAILED\n");
        return 1;
    }
    fprintf(stderr, "PROBE PASSED\n");
    return 0;
}