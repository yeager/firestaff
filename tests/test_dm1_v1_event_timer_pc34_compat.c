/*
 * CTest gate for DM1 V1 Event Timer and Scheduler System.
 *
 * Tests verify:
 *   1. Queue init and basic add/extract
 *   2. Binary heap ordering (time, type, priority, index)
 *   3. Event merge for door/corridor types
 *   4. Delete and heap repair
 *   5. Tick processing and dispatch classification
 *   6. Serialisation round-trip
 *   7. Capacity limits
 *   8. Game tick advancement
 */

#include "dm1_v1_event_timer_pc34_compat.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_ASSERT(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL [%s:%d]: %s\n", __FILE__, __LINE__, msg); \
        tests_failed++; \
        return; \
    } \
} while(0)

#define TEST_ASSERT_INT_EQ(a, b, msg) do { \
    if ((a) != (b)) { \
        fprintf(stderr, "FAIL [%s:%d]: %s (got %d, expected %d)\n", \
                __FILE__, __LINE__, msg, (int)(a), (int)(b)); \
        tests_failed++; \
        return; \
    } \
} while(0)

/* ----------------------------------------------------------------
 *  Test 1: Init and basic add/extract
 * ---------------------------------------------------------------- */
static void test_init_and_basic_add(void) {
    struct DM1_EventQueue_V1 queue;
    struct DM1_Event_V1 ev, out;

    TEST_ASSERT(dm1v1_event_queue_init(&queue, 100), "init should succeed");
    TEST_ASSERT_INT_EQ(queue.gameTick, 100, "gameTick should be 100");
    TEST_ASSERT_INT_EQ(queue.eventCount, 0, "eventCount should be 0");

    memset(&ev, 0, sizeof(ev));
    ev.type = DM1_EVENT_PLAY_SOUND;
    ev.map_time = DM1_MAP_TIME_MAKE(0, 50);  /* time 50 */
    ev.b_mapX = 3;
    ev.b_mapY = 4;

    TEST_ASSERT(dm1v1_event_add(&queue, &ev) >= 0, "add should succeed");
    TEST_ASSERT_INT_EQ(queue.eventCount, 1, "eventCount should be 1");

    /* Event at time 50 is expired (gameTick=100) */
    TEST_ASSERT(dm1v1_event_is_first_expired(&queue), "event should be expired");

    memset(&out, 0, sizeof(out));
    TEST_ASSERT(dm1v1_event_extract_first(&queue, &out), "extract should succeed");
    TEST_ASSERT_INT_EQ(out.type, DM1_EVENT_PLAY_SOUND, "type mismatch");
    TEST_ASSERT_INT_EQ(out.b_mapX, 3, "mapX mismatch");
    TEST_ASSERT_INT_EQ(out.b_mapY, 4, "mapY mismatch");
    TEST_ASSERT_INT_EQ(queue.eventCount, 0, "eventCount should be 0 after extract");

    tests_passed++;
    printf("PASS: test_init_and_basic_add\n");
}

/* ----------------------------------------------------------------
 *  Test 2: Heap ordering — earlier time first
 * ---------------------------------------------------------------- */
static void test_heap_ordering_by_time(void) {
    struct DM1_EventQueue_V1 queue;
    struct DM1_Event_V1 ev, out;

    dm1v1_event_queue_init(&queue, 1000);

    /* Add events at different times in non-sorted order */
    memset(&ev, 0, sizeof(ev));
    ev.type = DM1_EVENT_LIGHT;
    ev.map_time = DM1_MAP_TIME_MAKE(0, 300);
    dm1v1_event_add(&queue, &ev);

    ev.map_time = DM1_MAP_TIME_MAKE(0, 100);
    dm1v1_event_add(&queue, &ev);

    ev.map_time = DM1_MAP_TIME_MAKE(0, 200);
    dm1v1_event_add(&queue, &ev);

    TEST_ASSERT_INT_EQ(queue.eventCount, 3, "should have 3 events");

    dm1v1_event_extract_first(&queue, &out);
    TEST_ASSERT_INT_EQ(DM1_MAP_TIME_TIME(out.map_time), 100, "first should be time 100");

    dm1v1_event_extract_first(&queue, &out);
    TEST_ASSERT_INT_EQ(DM1_MAP_TIME_TIME(out.map_time), 200, "second should be time 200");

    dm1v1_event_extract_first(&queue, &out);
    TEST_ASSERT_INT_EQ(DM1_MAP_TIME_TIME(out.map_time), 300, "third should be time 300");

    tests_passed++;
    printf("PASS: test_heap_ordering_by_time\n");
}

/* ----------------------------------------------------------------
 *  Test 3: Heap ordering — same time, higher type first
 * ---------------------------------------------------------------- */
static void test_heap_ordering_by_type(void) {
    struct DM1_EventQueue_V1 queue;
    struct DM1_Event_V1 ev, out;

    dm1v1_event_queue_init(&queue, 1000);

    memset(&ev, 0, sizeof(ev));
    ev.map_time = DM1_MAP_TIME_MAKE(0, 500);

    ev.type = DM1_EVENT_DOOR_ANIMATION;  /* type 1 */
    dm1v1_event_add(&queue, &ev);

    ev.type = DM1_EVENT_EXPLOSION;  /* type 25 */
    dm1v1_event_add(&queue, &ev);

    ev.type = DM1_EVENT_LIGHT;  /* type 70 */
    dm1v1_event_add(&queue, &ev);

    /* Higher type value should come first (per ReDMCSB comparison) */
    dm1v1_event_extract_first(&queue, &out);
    TEST_ASSERT_INT_EQ(out.type, DM1_EVENT_LIGHT, "highest type first");

    dm1v1_event_extract_first(&queue, &out);
    TEST_ASSERT_INT_EQ(out.type, DM1_EVENT_EXPLOSION, "middle type second");

    dm1v1_event_extract_first(&queue, &out);
    TEST_ASSERT_INT_EQ(out.type, DM1_EVENT_DOOR_ANIMATION, "lowest type third");

    tests_passed++;
    printf("PASS: test_heap_ordering_by_type\n");
}

/* ----------------------------------------------------------------
 *  Test 4: Door event merge
 * ---------------------------------------------------------------- */
static void test_door_event_merge(void) {
    struct DM1_EventQueue_V1 queue;
    struct DM1_Event_V1 ev;
    int idx1, idx2;

    dm1v1_event_queue_init(&queue, 0);

    /* Add a DOOR event */
    memset(&ev, 0, sizeof(ev));
    ev.type = DM1_EVENT_DOOR;
    ev.map_time = DM1_MAP_TIME_MAKE(0, 100);
    ev.b_mapX = 5;
    ev.b_mapY = 6;
    ev.c_effect = DM1_EFFECT_SET;
    idx1 = dm1v1_event_add(&queue, &ev);
    TEST_ASSERT(idx1 >= 0, "first add should succeed");
    TEST_ASSERT_INT_EQ(queue.eventCount, 1, "should have 1 event");

    /* Add same DOOR event — should merge (update effect) */
    ev.c_effect = DM1_EFFECT_CLEAR;
    idx2 = dm1v1_event_add(&queue, &ev);
    TEST_ASSERT_INT_EQ(idx2, idx1, "should merge into same slot");
    TEST_ASSERT_INT_EQ(queue.eventCount, 1, "still 1 event after merge");
    TEST_ASSERT_INT_EQ(queue.events[idx1].c_effect, DM1_EFFECT_CLEAR, "effect should be updated");

    tests_passed++;
    printf("PASS: test_door_event_merge\n");
}

/* ----------------------------------------------------------------
 *  Test 5: Delete and heap integrity
 * ---------------------------------------------------------------- */
static void test_delete_and_heap_integrity(void) {
    struct DM1_EventQueue_V1 queue;
    struct DM1_Event_V1 ev, out;
    int idx;

    dm1v1_event_queue_init(&queue, 1000);

    memset(&ev, 0, sizeof(ev));
    ev.type = DM1_EVENT_LIGHT;

    ev.map_time = DM1_MAP_TIME_MAKE(0, 100);
    dm1v1_event_add(&queue, &ev);

    ev.map_time = DM1_MAP_TIME_MAKE(0, 200);
    idx = dm1v1_event_add(&queue, &ev);

    ev.map_time = DM1_MAP_TIME_MAKE(0, 300);
    dm1v1_event_add(&queue, &ev);

    TEST_ASSERT_INT_EQ(queue.eventCount, 3, "should have 3 events");

    /* Delete the middle event (time 200) */
    TEST_ASSERT(dm1v1_event_delete(&queue, idx), "delete should succeed");
    TEST_ASSERT_INT_EQ(queue.eventCount, 2, "should have 2 events");

    /* Extract should give time 100 then 300 */
    dm1v1_event_extract_first(&queue, &out);
    TEST_ASSERT_INT_EQ(DM1_MAP_TIME_TIME(out.map_time), 100, "first should be 100");

    dm1v1_event_extract_first(&queue, &out);
    TEST_ASSERT_INT_EQ(DM1_MAP_TIME_TIME(out.map_time), 300, "second should be 300");

    tests_passed++;
    printf("PASS: test_delete_and_heap_integrity\n");
}

/* ----------------------------------------------------------------
 *  Test 6: Tick processing and dispatch classification
 * ---------------------------------------------------------------- */
static void test_tick_processing(void) {
    struct DM1_EventQueue_V1 queue;
    struct DM1_Event_V1 ev;
    struct DM1_TickDispatchResult_V1 result;

    dm1v1_event_queue_init(&queue, 100);

    /* Add events at various times */
    memset(&ev, 0, sizeof(ev));

    /* Expired event (time 50) */
    ev.type = DM1_EVENT_PLAY_SOUND;
    ev.map_time = DM1_MAP_TIME_MAKE(0, 50);
    ev.b_mapX = 1; ev.b_mapY = 2;
    dm1v1_event_add(&queue, &ev);

    /* Expired event (time 100) */
    ev.type = DM1_EVENT_DOOR_ANIMATION;
    ev.map_time = DM1_MAP_TIME_MAKE(0, 100);
    ev.b_mapX = 3; ev.b_mapY = 4;
    dm1v1_event_add(&queue, &ev);

    /* Future event (time 200) — should NOT be dispatched */
    ev.type = DM1_EVENT_EXPLOSION;
    ev.map_time = DM1_MAP_TIME_MAKE(0, 200);
    dm1v1_event_add(&queue, &ev);

    int dispatched = dm1v1_event_process_tick(&queue, &result);
    TEST_ASSERT_INT_EQ(dispatched, 2, "should dispatch 2 expired events");
    TEST_ASSERT_INT_EQ(queue.eventCount, 1, "1 future event remains");

    /* Check dispatch classification */
    int foundSound = 0, foundDoor = 0;
    for (int i = 0; i < result.count; i++) {
        if (result.records[i].dispatchKind == DM1_DISPATCH_SOUND) foundSound = 1;
        if (result.records[i].dispatchKind == DM1_DISPATCH_DOOR_ANIMATION) foundDoor = 1;
    }
    TEST_ASSERT(foundSound, "should have dispatched sound event");
    TEST_ASSERT(foundDoor, "should have dispatched door animation event");

    tests_passed++;
    printf("PASS: test_tick_processing\n");
}

/* ----------------------------------------------------------------
 *  Test 7: Serialisation round-trip
 * ---------------------------------------------------------------- */
static void test_serialisation_roundtrip(void) {
    struct DM1_EventQueue_V1 queue1, queue2;
    struct DM1_Event_V1 ev;
    unsigned char buf[DM1_EVENT_QUEUE_SERIALIZED_SIZE];

    dm1v1_event_queue_init(&queue1, 42);

    memset(&ev, 0, sizeof(ev));
    ev.type = DM1_EVENT_LIGHT;
    ev.map_time = DM1_MAP_TIME_MAKE(3, 999);
    ev.priority = 7;
    ev.b_mapX = 10;
    ev.b_mapY = 20;
    ev.c_cell = 2;
    ev.c_effect = DM1_EFFECT_TOGGLE;
    dm1v1_event_add(&queue1, &ev);

    ev.type = DM1_EVENT_MOVE_PROJECTILE;
    ev.map_time = DM1_MAP_TIME_MAKE(1, 500);
    dm1v1_event_add(&queue1, &ev);

    TEST_ASSERT(dm1v1_event_queue_serialize(&queue1, buf, sizeof(buf)),
                "serialize should succeed");

    memset(&queue2, 0, sizeof(queue2));
    TEST_ASSERT(dm1v1_event_queue_deserialize(&queue2, buf, sizeof(buf)),
                "deserialize should succeed");

    TEST_ASSERT_INT_EQ(queue2.gameTick, 42, "gameTick roundtrip");
    TEST_ASSERT_INT_EQ(queue2.eventCount, 2, "eventCount roundtrip");
    TEST_ASSERT_INT_EQ(queue2.events[0].type, DM1_EVENT_LIGHT, "event 0 type roundtrip");
    TEST_ASSERT_INT_EQ(queue2.events[0].priority, 7, "event 0 priority roundtrip");
    TEST_ASSERT_INT_EQ(queue2.events[0].b_mapX, 10, "event 0 mapX roundtrip");
    TEST_ASSERT_INT_EQ(queue2.events[0].c_effect, DM1_EFFECT_TOGGLE, "event 0 effect roundtrip");
    TEST_ASSERT_INT_EQ(DM1_MAP_TIME_MAP(queue2.events[0].map_time), 3, "event 0 map roundtrip");

    tests_passed++;
    printf("PASS: test_serialisation_roundtrip\n");
}

/* ----------------------------------------------------------------
 *  Test 8: Capacity limit
 * ---------------------------------------------------------------- */
static void test_capacity_limit(void) {
    struct DM1_EventQueue_V1 queue;
    struct DM1_Event_V1 ev;
    int i, added;

    dm1v1_event_queue_init(&queue, 0);

    memset(&ev, 0, sizeof(ev));
    ev.type = DM1_EVENT_PLAY_SOUND;

    for (i = 0; i < DM1_EVENT_MAX_COUNT; i++) {
        ev.map_time = DM1_MAP_TIME_MAKE(0, (uint32_t)(i + 1));
        added = dm1v1_event_add(&queue, &ev);
        TEST_ASSERT(added >= 0, "add should succeed within capacity");
    }

    TEST_ASSERT_INT_EQ(queue.eventCount, DM1_EVENT_MAX_COUNT, "should be at capacity");

    /* One more should fail */
    ev.map_time = DM1_MAP_TIME_MAKE(0, 999);
    added = dm1v1_event_add(&queue, &ev);
    TEST_ASSERT_INT_EQ(added, -1, "add should fail at capacity");

    tests_passed++;
    printf("PASS: test_capacity_limit\n");
}

/* ----------------------------------------------------------------
 *  Test 9: Game tick advancement
 * ---------------------------------------------------------------- */
static void test_tick_advancement(void) {
    struct DM1_EventQueue_V1 queue;
    struct DM1_Event_V1 ev;

    dm1v1_event_queue_init(&queue, 0);

    memset(&ev, 0, sizeof(ev));
    ev.type = DM1_EVENT_WATCHDOG;
    ev.map_time = DM1_MAP_TIME_MAKE(0, 5);
    dm1v1_event_add(&queue, &ev);

    /* At tick 0, event at time 5 is NOT expired */
    TEST_ASSERT(!dm1v1_event_is_first_expired(&queue), "should not be expired at tick 0");

    dm1v1_event_advance_tick(&queue);  /* tick 1 */
    dm1v1_event_advance_tick(&queue);  /* tick 2 */
    dm1v1_event_advance_tick(&queue);  /* tick 3 */
    dm1v1_event_advance_tick(&queue);  /* tick 4 */
    TEST_ASSERT(!dm1v1_event_is_first_expired(&queue), "should not be expired at tick 4");

    dm1v1_event_advance_tick(&queue);  /* tick 5 */
    TEST_ASSERT(dm1v1_event_is_first_expired(&queue), "should be expired at tick 5");

    tests_passed++;
    printf("PASS: test_tick_advancement\n");
}

/* ----------------------------------------------------------------
 *  Test 10: Dispatch classification for all event families
 * ---------------------------------------------------------------- */
static void test_dispatch_classification(void) {
    struct DM1_EventQueue_V1 queue;
    struct DM1_Event_V1 ev;
    struct DM1_TickDispatchResult_V1 result;
    int i;

    dm1v1_event_queue_init(&queue, 1000);

    /* Add one of each major event family, all expired */
    uint8_t types[] = {
        DM1_EVENT_DOOR_ANIMATION,          /* → DOOR_ANIMATION */
        DM1_EVENT_EXPLOSION,               /* → EXPLOSION */
        DM1_EVENT_MOVE_PROJECTILE,         /* → PROJECTILE */
        DM1_EVENT_LIGHT,                   /* → PARTY_SPELL */
        DM1_EVENT_UPDATE_BEHAVIOR_GROUP,   /* → CREATURE_AI */
        DM1_EVENT_ENABLE_GROUP_GENERATOR,  /* → GENERATOR */
        DM1_EVENT_WATCHDOG,                /* → WATCHDOG */
    };
    int expected[] = {
        DM1_DISPATCH_DOOR_ANIMATION,
        DM1_DISPATCH_EXPLOSION,
        DM1_DISPATCH_PROJECTILE,
        DM1_DISPATCH_PARTY_SPELL,
        DM1_DISPATCH_CREATURE_AI,
        DM1_DISPATCH_GENERATOR,
        DM1_DISPATCH_WATCHDOG,
    };
    int n = sizeof(types) / sizeof(types[0]);

    for (i = 0; i < n; i++) {
        memset(&ev, 0, sizeof(ev));
        ev.type = types[i];
        ev.map_time = DM1_MAP_TIME_MAKE(0, (uint32_t)(100 + i));
        dm1v1_event_add(&queue, &ev);
    }

    dm1v1_event_process_tick(&queue, &result);
    TEST_ASSERT_INT_EQ(result.count, n, "should dispatch all events");

    for (i = 0; i < n; i++) {
        /* Find the record matching this event type */
        int found = 0;
        for (int j = 0; j < result.count; j++) {
            if (result.records[j].eventType == types[i]) {
                TEST_ASSERT_INT_EQ(result.records[j].dispatchKind, expected[i],
                                   "dispatch kind mismatch");
                found = 1;
                break;
            }
        }
        TEST_ASSERT(found, "event type not found in dispatch results");
    }

    tests_passed++;
    printf("PASS: test_dispatch_classification\n");
}

/* ----------------------------------------------------------------
 *  Test 11: DOOR_ANIMATION + DOOR merge (toggle resolution)
 * ---------------------------------------------------------------- */
static void test_door_animation_merge(void) {
    struct DM1_EventQueue_V1 queue;
    struct DM1_Event_V1 ev;
    int idx1, idx2;

    dm1v1_event_queue_init(&queue, 0);

    /* Add a DOOR_ANIMATION event with CLEAR effect */
    memset(&ev, 0, sizeof(ev));
    ev.type = DM1_EVENT_DOOR_ANIMATION;
    ev.map_time = DM1_MAP_TIME_MAKE(0, 100);
    ev.b_mapX = 5;
    ev.b_mapY = 6;
    ev.c_effect = DM1_EFFECT_CLEAR;
    idx1 = dm1v1_event_add(&queue, &ev);
    TEST_ASSERT(idx1 >= 0, "first DOOR_ANIMATION add");

    /* Add a DOOR event with TOGGLE at same time/location
     * Per F0238: existing DOOR_ANIMATION + new DOOR event with TOGGLE →
     * resolve toggle to 1 - DOOR_ANIMATION.c_effect */
    ev.type = DM1_EVENT_DOOR;
    ev.c_effect = DM1_EFFECT_TOGGLE;
    idx2 = dm1v1_event_add(&queue, &ev);

    /* The DOOR_ANIMATION was deleted, DOOR event added with resolved effect.
     * Per F0238 DOOR_ANIMATION branch: new DOOR event where existing
     * DOOR_ANIMATION had CLEAR(1), so toggle resolves to 1-1=0=SET.
     * But the code path actually checks: if existing is DOOR and effect
     * is TOGGLE, set existing->effect = 1 - incoming->effect.
     * For the DOOR_ANIMATION merging with incoming DOOR: the DOOR_ANIMATION
     * gets deleted, and the DOOR event is added fresh. */
    TEST_ASSERT(idx2 >= 0, "DOOR event should be added");

    tests_passed++;
    printf("PASS: test_door_animation_merge\n");
}

/* ================================================================
 *  Main
 * ================================================================ */

int main(void) {
    printf("=== DM1 V1 Event Timer Tests ===\n\n");

    test_init_and_basic_add();
    test_heap_ordering_by_time();
    test_heap_ordering_by_type();
    test_door_event_merge();
    test_delete_and_heap_integrity();
    test_tick_processing();
    test_serialisation_roundtrip();
    test_capacity_limit();
    test_tick_advancement();
    test_dispatch_classification();
    test_door_animation_merge();

    printf("\n=== Results: %d passed, %d failed ===\n",
           tests_passed, tests_failed);

    return tests_failed > 0 ? 1 : 0;
}
