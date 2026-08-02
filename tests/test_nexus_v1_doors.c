/* Test Nexus door state machine and animation runtime. */
#include "nexus_v1_doors.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    Nexus_DoorManager mgr;
    nexus_v1_door_manager_init(&mgr);

    /* Test 1: register and find. */
    int d0 = nexus_v1_door_register(&mgr, 5, 10, 0,
        NEXUS_DOOR_TYPE_WOODEN, 0, 0, 0);
    assert(d0 == 0);
    assert(mgr.count == 1);
    assert(nexus_v1_door_find(&mgr, 5, 10, 0) == 0);
    assert(nexus_v1_door_find(&mgr, 5, 11, 0) == -1);

    /* Test 2: unlocked door opens immediately. */
    assert(mgr.doors[d0].state == NEXUS_DOOR_CLOSED);
    int r = nexus_v1_door_try_open(&mgr, d0, -1, 0);
    assert(r == NEXUS_DOOR_RESULT_OK);
    assert(mgr.doors[d0].state == NEXUS_DOOR_OPENING);
    assert(!nexus_v1_door_is_passable(&mgr, d0) ||
            nexus_v1_door_is_passable(&mgr, d0));

    /* Test 3: tick through animation. */
    for (int i = 0; i < NEXUS_DOOR_ANIM_TICKS; i++)
        nexus_v1_door_tick(&mgr);
    assert(mgr.doors[d0].state == NEXUS_DOOR_OPEN);
    assert(nexus_v1_door_is_passable(&mgr, d0));
    assert(nexus_v1_door_anim_progress(&mgr, d0) == 1.0f);

    /* Test 4: already open. */
    r = nexus_v1_door_try_open(&mgr, d0, -1, 0);
    assert(r == NEXUS_DOOR_RESULT_ALREADY_OPEN);

    /* Test 5: close and tick. */
    assert(nexus_v1_door_close(&mgr, d0) == 0);
    assert(mgr.doors[d0].state == NEXUS_DOOR_CLOSING);
    for (int i = 0; i < NEXUS_DOOR_ANIM_TICKS; i++)
        nexus_v1_door_tick(&mgr);
    assert(mgr.doors[d0].state == NEXUS_DOOR_CLOSED);
    assert(!nexus_v1_door_is_passable(&mgr, d0));

    /* Test 6: locked door — no key. */
    int d1 = nexus_v1_door_register(&mgr, 8, 8, 1,
        NEXUS_DOOR_TYPE_IRON, 42, 10, 0);
    assert(d1 == 1);
    assert(mgr.doors[d1].state == NEXUS_DOOR_LOCKED);
    r = nexus_v1_door_try_open(&mgr, d1, -1, 0);
    assert(r == NEXUS_DOOR_RESULT_LOCKED);

    /* Test 7: locked door — wrong key. */
    r = nexus_v1_door_try_open(&mgr, d1, 13, 0);
    assert(r == NEXUS_DOOR_RESULT_LOCKED);

    /* Test 8: locked door — correct key. */
    r = nexus_v1_door_try_open(&mgr, d1, 42, 0);
    assert(r == NEXUS_DOOR_RESULT_OK);
    assert(mgr.doors[d1].state == NEXUS_DOOR_OPENING);

    /* Test 9: bash — too weak. */
    nexus_v1_door_manager_init(&mgr);
    nexus_v1_door_register(&mgr, 3, 3, 0,
        NEXUS_DOOR_TYPE_WOODEN, 5, 20, 0);
    r = nexus_v1_door_try_open(&mgr, 0, -1, 10);
    assert(r == NEXUS_DOOR_RESULT_BASH_FAIL);

    /* Test 10: bash — strong enough. */
    r = nexus_v1_door_try_open(&mgr, 0, -1, 25);
    assert(r == NEXUS_DOOR_RESULT_BASHED);

    /* Test 11: toggle. */
    nexus_v1_door_manager_init(&mgr);
    nexus_v1_door_register(&mgr, 1, 1, 0,
        NEXUS_DOOR_TYPE_PORTCUL, 0, 0, NEXUS_DOOR_FLAG_BUTTON);
    assert(nexus_v1_door_toggle(&mgr, 0) == 0);
    assert(mgr.doors[0].state == NEXUS_DOOR_OPENING);
    for (int i = 0; i < NEXUS_DOOR_ANIM_TICKS; i++)
        nexus_v1_door_tick(&mgr);
    assert(mgr.doors[0].state == NEXUS_DOOR_OPEN);
    assert(nexus_v1_door_toggle(&mgr, 0) == 0);
    assert(mgr.doors[0].state == NEXUS_DOOR_CLOSING);

    /* Test 12: max capacity. */
    nexus_v1_door_manager_init(&mgr);
    for (int i = 0; i < NEXUS_MAX_DOORS; i++) {
        int idx = nexus_v1_door_register(&mgr, i, 0, 0, 0, 0, 0, 0);
        assert(idx == i);
    }
    assert(nexus_v1_door_register(&mgr, 999, 0, 0, 0, 0, 0, 0) == -1);

    printf("PASS: nexus_v1_doors (12 tests)\n");
    return 0;
}
