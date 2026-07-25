#include "dm1_v1_dungeon_loader_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static DM1_V1_DungeonStatePc34 g_state;

static void test_init(void) {
    DM1_V1_DungeonLoader_InitPc34Compat(&g_state);
    assert(g_state.loaded == false);
    assert(g_state.header.level_count == 0);
    /* Step tables: dir 0=North, 1=East, 2=South, 3=West */
    assert(g_state.step_east[0] == 0);
    assert(g_state.step_east[1] == 1);
    assert(g_state.step_east[2] == 0);
    assert(g_state.step_east[3] == -1);
    assert(g_state.step_north[0] == -1);
    assert(g_state.step_north[1] == 0);
    assert(g_state.step_north[2] == 1);
    assert(g_state.step_north[3] == 0);
    /* Thing byte counts */
    assert(g_state.thing_byte_count[0] == 4);  /* door */
    assert(g_state.thing_byte_count[1] == 6);  /* teleporter */
    assert(g_state.thing_byte_count[4] == 16); /* group */
    assert(g_state.thing_byte_count[14] == 8); /* projectile */
}

static void test_load_nonexistent(void) {
    DM1_V1_DungeonLoader_InitPc34Compat(&g_state);
    bool ok = DM1_V1_DungeonLoader_LoadFromFilePc34Compat(&g_state, "/no/such/file.dat");
    assert(!ok);
    assert(!g_state.loaded);
}

static void test_get_tile_unloaded(void) {
    DM1_V1_DungeonLoader_InitPc34Compat(&g_state);
    const DM1_V1_DungeonTilePc34 *t = DM1_V1_DungeonLoader_GetTilePc34Compat(&g_state, 0, 0, 0);
    assert(t == NULL);
}

static void test_get_tile_out_of_bounds(void) {
    /* Even if loaded were true, out-of-range coords should return NULL */
    DM1_V1_DungeonLoader_InitPc34Compat(&g_state);
    g_state.loaded = true;
    const DM1_V1_DungeonTilePc34 *t;
    t = DM1_V1_DungeonLoader_GetTilePc34Compat(&g_state, DM1_MAX_LEVELS, 0, 0);
    assert(t == NULL);
    t = DM1_V1_DungeonLoader_GetTilePc34Compat(&g_state, 0, DM1_MAX_MAP_W, 0);
    assert(t == NULL);
    t = DM1_V1_DungeonLoader_GetTilePc34Compat(&g_state, 0, 0, DM1_MAX_MAP_H);
    assert(t == NULL);
    (void)t;
}

static void test_step_forward(void) {
    int x, y;
    /* dir 0 = North: east+0, north-1 */
    x = 5; y = 5;
    DM1_V1_DungeonLoader_StepForwardPc34Compat(&x, &y, 0);
    assert(x == 5); assert(y == 4);
    /* dir 1 = East: east+1, north+0 */
    x = 5; y = 5;
    DM1_V1_DungeonLoader_StepForwardPc34Compat(&x, &y, 1);
    assert(x == 6); assert(y == 5);
    /* dir 2 = South: east+0, north+1 */
    x = 5; y = 5;
    DM1_V1_DungeonLoader_StepForwardPc34Compat(&x, &y, 2);
    assert(x == 5); assert(y == 6);
    /* dir 3 = West: east-1, north+0 */
    x = 5; y = 5;
    DM1_V1_DungeonLoader_StepForwardPc34Compat(&x, &y, 3);
    assert(x == 4); assert(y == 5);
    (void)x; (void)y;
}

static void test_step_forward_invalid_dir(void) {
    int x = 5, y = 5;
    DM1_V1_DungeonLoader_StepForwardPc34Compat(&x, &y, 4);
    assert(x == 5); assert(y == 5);
    (void)x; (void)y;
}

static void test_cleanup(void) {
    DM1_V1_DungeonLoader_InitPc34Compat(&g_state);
    g_state.loaded = true;
    DM1_V1_DungeonLoader_CleanupPc34Compat(&g_state);
    assert(!g_state.loaded);
    assert(g_state.header.level_count == 0);
}

static void test_enum_values(void) {
    assert(DM1_V1_DUNGEON_THING_DOOR_PC34 == 0);
    assert(DM1_V1_DUNGEON_THING_WEAPON_PC34 == 5);
    assert(DM1_V1_DUNGEON_THING_ARMOUR_PC34 == 6);
    assert(DM1_V1_DUNGEON_THING_PROJECTILE_PC34 == 14);
}

int main(void) {
    test_init();
    test_load_nonexistent();
    test_get_tile_unloaded();
    test_get_tile_out_of_bounds();
    test_step_forward();
    test_step_forward_invalid_dir();
    test_cleanup();
    test_enum_values();
    puts("ok: dm1_v1_dungeon_loader_pc34_compat 8 tests passed");
    return 0;
}
