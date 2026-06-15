/**
 * test_dm1_v1_sft_overflow_bug0_08_pc34_compat.c
 *
 * Regression gate for F0502b_DUNGEON_CheckBug0_08SftOverfill_Compat
 * (DUN-05 in docs/dm1-v1-functional-divergence-report.md).
 *
 * ReDMCSB DUNGEON.C:F0163_DUNGEON_LinkThingToList can overfill the
 * G0283_pT_SquareFirstThings buffer when the dungeon contains more
 * thing-bearing squares than free slots. Original can corrupt memory;
 * Firestaff refuses to overfill silently (defensive) and the helper
 * F0502b makes the divergence observable by emitting a one-shot
 * warning to stderr.
 *
 * This test pins the helper's API contract:
 *   - returns 0 when thing-list squares fit within the SFT buffer
 *   - returns the overfill count when thing-list squares exceed it
 *   - returns 0 on a NULL or uninitialised state
 */
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { \
        ++g_pass; \
    } else { \
        ++g_fail; \
        fprintf(stderr, "FAIL: %s\n", msg); \
    } \
} while (0)

/* Synthetic state with one map of 4x4 squares. */
static void build_state_with_thing_list_squares(
    struct DungeonDatState_Compat* state,
    int thingListSquareCount,
    int sftCapacity)
{
    int i;
    int mapIndex = 0;
    const int w = 4;
    const int h = 4;
    const int n = w * h;

    memset(state, 0, sizeof(*state));
    state->header.mapCount = 1;
    state->header.squareFirstThingCount = (unsigned short)sftCapacity;
    state->maps = (struct DungeonMapDesc_Compat*)calloc(1, sizeof(state->maps[0]));
    state->maps[0].width = (unsigned char)w;
    state->maps[0].height = (unsigned char)h;

    state->tiles = (struct DungeonMapTiles_Compat*)calloc(1, sizeof(state->tiles[0]));
    state->tiles[mapIndex].squareCount = n;
    state->tiles[mapIndex].squareData = (unsigned char*)calloc((size_t)n, 1);
    for (i = 0; i < thingListSquareCount && i < n; ++i) {
        state->tiles[mapIndex].squareData[i] = DUNGEON_SQUARE_MASK_THING_LIST;
    }
    state->tilesLoaded = 1;
}

static void free_state(struct DungeonDatState_Compat* state) {
    if (state->tiles) {
        if (state->tiles[0].squareData) free(state->tiles[0].squareData);
        free(state->tiles);
        state->tiles = NULL;
    }
    if (state->maps) {
        free(state->maps);
        state->maps = NULL;
    }
}

static void test_no_overfill(void) {
    /* 4 thing-list squares, SFT capacity 16 - fits. */
    struct DungeonDatState_Compat state;
    struct DungeonThings_Compat things;

    memset(&things, 0, sizeof(things));
    things.squareFirstThingCount = 16;
    things.squareFirstThings = (unsigned short*)calloc(16, sizeof(unsigned short));

    build_state_with_thing_list_squares(&state, 4, 16);

    CHECK(F0502b_DUNGEON_CheckBug0_08SftOverfill_Compat(&state, &things) == 0,
          "no overfill when thing-list squares fit within SFT");

    free(things.squareFirstThings);
    free_state(&state);
}

static void test_overfill(void) {
    /* 12 thing-list squares, SFT capacity 8 - overfill = 4. */
    struct DungeonDatState_Compat state;
    struct DungeonThings_Compat things;
    int overfill;

    memset(&things, 0, sizeof(things));
    things.squareFirstThingCount = 8;
    things.squareFirstThings = (unsigned short*)calloc(8, sizeof(unsigned short));

    build_state_with_thing_list_squares(&state, 12, 8);

    overfill = F0502b_DUNGEON_CheckBug0_08SftOverfill_Compat(&state, &things);
    CHECK(overfill == 4,
          "overfill count equals thing-list squares minus SFT capacity");

    free(things.squareFirstThings);
    free_state(&state);
}

static void test_exact_fit(void) {
    /* 4 thing-list squares, SFT capacity 4 - exact fit, not overfill. */
    struct DungeonDatState_Compat state;
    struct DungeonThings_Compat things;

    memset(&things, 0, sizeof(things));
    things.squareFirstThingCount = 4;
    things.squareFirstThings = (unsigned short*)calloc(4, sizeof(unsigned short));

    build_state_with_thing_list_squares(&state, 4, 4);

    CHECK(F0502b_DUNGEON_CheckBug0_08SftOverfill_Compat(&state, &things) == 0,
          "exact fit reports no overfill");

    free(things.squareFirstThings);
    free_state(&state);
}

static void test_null_safety(void) {
    struct DungeonDatState_Compat state;
    struct DungeonThings_Compat things;

    memset(&state, 0, sizeof(state));
    memset(&things, 0, sizeof(things));

    /* All inputs NULL: 0 (no crash, no false-positive). */
    CHECK(F0502b_DUNGEON_CheckBug0_08SftOverfill_Compat(NULL, NULL) == 0,
          "NULL inputs return 0");

    /* Tiles not loaded: 0. */
    CHECK(F0502b_DUNGEON_CheckBug0_08SftOverfill_Compat(&state, &things) == 0,
          "tiles-not-loaded state returns 0");
}

int main(void) {
    printf("[dm1_v1_sft_overflow_bug0_08_pc34_compat]\n");
    test_null_safety();
    test_no_overfill();
    test_exact_fit();
    test_overfill();
    printf("  %d passed, %d failed\n", g_pass, g_fail);
    return (g_fail == 0) ? 0 : 1;
}
