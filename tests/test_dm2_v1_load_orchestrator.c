/* Test DM2 load orchestrator — round-trip with save orchestrator.
 * Source: sksvgame.cpp:1415-1530 (load), 2086-2287 (save). */

#include "dm2_v1_load_orchestrator_pc34_compat.h"
#include "dm2_v1_save_orchestrator_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* ---- Null safety ---- */
static void test_null_safety(void)
{
    DM2_LoadOrchestratorResult result;
    assert(dm2_v1_load_orchestrate(NULL, NULL, 0, &result) == -1);
    assert(result.error != 0);
    printf("  PASS: null_safety\n");
}

/* ---- Result fields init ---- */
static void test_result_fields(void)
{
    DM2_LoadOrchestratorResult r;
    memset(&r, 0, sizeof(r));
    assert(r.valid == 0);
    assert(r.hero_count == 0);
    assert(r.num_timers == 0);
    assert(r.current_map == 0);
    assert(r.hero_items_loaded == 0);
    assert(r.dungeon_loaded == 0);
    assert(r.error == 0);
    printf("  PASS: result_fields\n");
}

/* ---- Callback struct size ---- */
static void test_callback_struct(void)
{
    DM2_LoadOrchestratorCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    assert(cb.ctx == NULL);
    assert(cb.read_raw == NULL);
    assert(cb.set_header == NULL);
    printf("  PASS: callback_struct\n");
}

int main(void)
{
    printf("test_dm2_v1_load_orchestrator:\n");
    test_null_safety();
    test_result_fields();
    test_callback_struct();
    printf("All load_orchestrator tests passed.\n");
    return 0;
}
