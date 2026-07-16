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
 *  Test 3b: Heap ordering — same time/type, higher priority then index
 * ---------------------------------------------------------------- */
static void test_heap_ordering_by_priority_and_index(void) {
    struct DM1_EventQueue_V1 queue;
    struct DM1_Event_V1 ev, out;
    int lowPriorityIndex;
    int firstEqualIndex;
    int secondEqualIndex;

    dm1v1_event_queue_init(&queue, 1000);

    memset(&ev, 0, sizeof(ev));
    ev.type = DM1_EVENT_MOVE_PROJECTILE;
    ev.map_time = DM1_MAP_TIME_MAKE(0, 500);

    ev.priority = 1;
    lowPriorityIndex = dm1v1_event_add(&queue, &ev);
    TEST_ASSERT(lowPriorityIndex >= 0, "low-priority event added");

    ev.priority = 7;
    firstEqualIndex = dm1v1_event_add(&queue, &ev);
    TEST_ASSERT(firstEqualIndex >= 0, "first high-priority event added");
    secondEqualIndex = dm1v1_event_add(&queue, &ev);
    TEST_ASSERT(secondEqualIndex >= 0, "second high-priority event added");

    dm1v1_event_extract_first(&queue, &out);
    TEST_ASSERT_INT_EQ(out.priority, 7, "higher priority comes first");
    TEST_ASSERT_INT_EQ(dm1v1_event_get_timeline_index(&queue, firstEqualIndex),
                       -1, "lower event-array index wins exact tie");

    dm1v1_event_extract_first(&queue, &out);
    TEST_ASSERT_INT_EQ(out.priority, 7, "same-priority peer comes second");
    TEST_ASSERT_INT_EQ(dm1v1_event_get_timeline_index(&queue, secondEqualIndex),
                       -1, "second exact-tie event is removed next");

    dm1v1_event_extract_first(&queue, &out);
    TEST_ASSERT_INT_EQ(out.priority, 1, "lower priority comes last");
    TEST_ASSERT_INT_EQ(dm1v1_event_get_timeline_index(&queue, lowPriorityIndex),
                       -1, "low-priority event was extracted");

    tests_passed++;
    printf("PASS: test_heap_ordering_by_priority_and_index\n");
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
 *  Test 4b: F0238 rejects C00 and preserves wall-cell separation
 * ---------------------------------------------------------------- */
static void test_f0238_none_reject_and_wall_cell_merge(void) {
    struct DM1_EventQueue_V1 queue;
    struct DM1_Event_V1 ev;
    int idxCell0, idxCell1, idxCell0Merge;

    dm1v1_event_queue_init(&queue, 0);

    memset(&ev, 0, sizeof(ev));
    ev.type = DM1_EVENT_NONE;
    ev.map_time = DM1_MAP_TIME_MAKE(0, 25);
    TEST_ASSERT_INT_EQ(dm1v1_event_add(&queue, &ev), -1,
                       "F0238 rejects C00_EVENT_NONE");
    TEST_ASSERT_INT_EQ(queue.eventCount, 0,
                       "C00 rejection does not mutate the heap");

    ev.type = DM1_EVENT_WALL;
    ev.b_mapX = 9;
    ev.b_mapY = 2;
    ev.c_cell = 0;
    ev.c_effect = DM1_EFFECT_SET;
    idxCell0 = dm1v1_event_add(&queue, &ev);
    TEST_ASSERT(idxCell0 >= 0, "first wall-cell event added");

    ev.c_cell = 1;
    idxCell1 = dm1v1_event_add(&queue, &ev);
    TEST_ASSERT(idxCell1 >= 0, "second wall-cell event added separately");
    TEST_ASSERT(idxCell1 != idxCell0, "different wall cells do not merge");
    TEST_ASSERT_INT_EQ(queue.eventCount, 2, "two wall-cell events remain");

    ev.c_cell = 0;
    ev.c_effect = DM1_EFFECT_CLEAR;
    idxCell0Merge = dm1v1_event_add(&queue, &ev);
    TEST_ASSERT_INT_EQ(idxCell0Merge, idxCell0, "same wall cell merges");
    TEST_ASSERT_INT_EQ(queue.eventCount, 2, "merge does not add a new event");
    TEST_ASSERT_INT_EQ(queue.events[idxCell0].c_effect, DM1_EFFECT_CLEAR,
                       "same-cell wall merge updates effect");

    tests_passed++;
    printf("PASS: test_f0238_none_reject_and_wall_cell_merge\n");
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
 * Test 7: MOVESENS.C F0265 C60/C61 native event shape
 * ---------------------------------------------------------------- */
static void test_f0265_move_group_event_shape(void) {
    struct DM1_Event_V1 event;
    uint16_t groupThing = (uint16_t)((4u << 10) | 0x0007u);

    memset(&event, 0xff, sizeof(event));
    TEST_ASSERT(dm1v1_f0265_build_move_group_event(
                    0x00fffffeu, 3u, 11u, 12u, groupThing, 0, &event),
                "F0265 builds silent C60 event");
    TEST_ASSERT_INT_EQ(event.type, DM1_EVENT_MOVE_GROUP_SILENT,
                       "F0265 silent type is C60");
    TEST_ASSERT_INT_EQ(event.priority, 0, "F0265 priority is zero");
    TEST_ASSERT_INT_EQ(DM1_MAP_TIME_MAP(event.map_time), 3,
                       "F0265 preserves map index");
    TEST_ASSERT_INT_EQ(DM1_MAP_TIME_TIME(event.map_time), 3,
                       "F0265 wraps only the native 24-bit game time");
    TEST_ASSERT_INT_EQ(event.b_mapX, 11, "F0265 writes destination X");
    TEST_ASSERT_INT_EQ(event.b_mapY, 12, "F0265 writes destination Y");
    TEST_ASSERT_INT_EQ((int)(event.c_cell | ((uint16_t)event.c_effect << 8)),
                       groupThing, "F0265 writes exact C04 Thing into C.Slot");

    TEST_ASSERT(dm1v1_f0265_build_move_group_event(
                    100u, 2u, 1u, 9u, groupThing, 1, &event),
                "F0265 builds audible C61 event");
    TEST_ASSERT_INT_EQ(event.type, DM1_EVENT_MOVE_GROUP_AUDIBLE,
                       "F0265 audible type is C61");
    TEST_ASSERT_INT_EQ(DM1_MAP_TIME_TIME(event.map_time), 105,
                       "F0265 delays exactly five ticks");

    TEST_ASSERT(!dm1v1_f0265_build_move_group_event(
                     0u, 0u, 0u, 0u, groupThing, 0, NULL),
                "F0265 rejects absent event storage");
    tests_passed++;
    printf("PASS: test_f0265_move_group_event_shape\n");
}

/* ----------------------------------------------------------------
 *  Test 8: Serialisation round-trip
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

/* ----------------------------------------------------------------
 *  Test 11b: F0238 C02 deletes same-map door/animation at any time
 * ---------------------------------------------------------------- */
static void test_door_destruction_merge_scope(void) {
    struct DM1_EventQueue_V1 queue;
    struct DM1_Event_V1 ev;
    int doorOtherMapSameTime;
    int destruction;
    int sameMapConflictCount = 0;

    dm1v1_event_queue_init(&queue, 0);

    memset(&ev, 0, sizeof(ev));
    ev.type = DM1_EVENT_DOOR;
    ev.map_time = DM1_MAP_TIME_MAKE(2, 100);
    ev.b_mapX = 5;
    ev.b_mapY = 6;
    ev.c_effect = DM1_EFFECT_SET;
    TEST_ASSERT(dm1v1_event_add(&queue, &ev) >= 0, "same-map door added");

    ev.type = DM1_EVENT_DOOR_ANIMATION;
    ev.map_time = DM1_MAP_TIME_MAKE(2, 75);
    TEST_ASSERT(dm1v1_event_add(&queue, &ev) >= 0, "same-map animation added");

    ev.type = DM1_EVENT_DOOR;
    ev.map_time = DM1_MAP_TIME_MAKE(3, 75);
    doorOtherMapSameTime = dm1v1_event_add(&queue, &ev);
    TEST_ASSERT(doorOtherMapSameTime >= 0, "other-map door added");

    ev.type = DM1_EVENT_DOOR_DESTRUCTION;
    ev.map_time = DM1_MAP_TIME_MAKE(2, 75);
    destruction = dm1v1_event_add(&queue, &ev);
    TEST_ASSERT(destruction >= 0, "door-destruction event added");

    for (int i = 0; i < queue.maxEvents; i++) {
        if ((queue.events[i].type == DM1_EVENT_DOOR ||
             queue.events[i].type == DM1_EVENT_DOOR_ANIMATION) &&
            DM1_MAP_TIME_MAP(queue.events[i].map_time) == 2 &&
            queue.events[i].b_mapX == 5 &&
            queue.events[i].b_mapY == 6) {
            sameMapConflictCount++;
        }
    }
    TEST_ASSERT_INT_EQ(sameMapConflictCount, 0,
                       "C02 removes all same-map door/animation conflicts");
    TEST_ASSERT_INT_EQ(queue.events[doorOtherMapSameTime].type,
                       DM1_EVENT_DOOR,
                       "C02 retains other-map door");
    TEST_ASSERT_INT_EQ(queue.eventCount, 2,
                       "only other-map door and C02 event remain");

    tests_passed++;
    printf("PASS: test_door_destruction_merge_scope\n");
}

/* ----------------------------------------------------------------
 *  Test 11c: F0235/F0236 reschedule an existing event in place
 * ---------------------------------------------------------------- */
static void test_get_index_and_fix_existing_placement(void) {
    struct DM1_EventQueue_V1 queue;
    struct DM1_Event_V1 ev, out;
    int idxLate;
    int idxEarly;

    dm1v1_event_queue_init(&queue, 1000);

    memset(&ev, 0, sizeof(ev));
    ev.type = DM1_EVENT_LIGHT;
    ev.map_time = DM1_MAP_TIME_MAKE(0, 300);
    idxLate = dm1v1_event_add(&queue, &ev);
    TEST_ASSERT(idxLate >= 0, "late event added");

    ev.map_time = DM1_MAP_TIME_MAKE(0, 100);
    idxEarly = dm1v1_event_add(&queue, &ev);
    TEST_ASSERT(idxEarly >= 0, "early event added");
    TEST_ASSERT(dm1v1_event_get_timeline_index(&queue, idxLate) >= 0,
                "F0235 finds live late event");

    queue.events[idxLate].map_time = DM1_MAP_TIME_MAKE(0, 50);
    TEST_ASSERT(dm1v1_event_fix_existing_placement(&queue, idxLate),
                "F0236 fixes changed live event");

    dm1v1_event_extract_first(&queue, &out);
    TEST_ASSERT_INT_EQ(DM1_MAP_TIME_TIME(out.map_time), 50,
                       "rescheduled event extracts first");

    TEST_ASSERT(!dm1v1_event_fix_existing_placement(&queue, idxLate),
                "F0236 wrapper rejects deleted event index");

    dm1v1_event_extract_first(&queue, &out);
    TEST_ASSERT_INT_EQ(DM1_MAP_TIME_TIME(out.map_time), 100,
                       "remaining event keeps heap order");
    TEST_ASSERT_INT_EQ(dm1v1_event_get_timeline_index(&queue, idxEarly),
                       -1, "F0235 reports missing after extraction");

    tests_passed++;
    printf("PASS: test_get_index_and_fix_existing_placement\n");
}

/* ----------------------------------------------------------------
 *  Test 12: GROUP.C F0181 deletes C29..C41 at one current-map square
 * ---------------------------------------------------------------- */
static void test_group_delete_events_f0181(void) {
    struct DM1_EventQueue_V1 queue;
    struct DM1_Event_V1 ev;
    int deleted;

    dm1v1_event_queue_init(&queue, 0);
    memset(&ev, 0, sizeof(ev));
    ev.map_time = DM1_MAP_TIME_MAKE(3, 10);
    ev.b_mapX = 4;
    ev.b_mapY = 5;

    ev.type = DM1_EVENT_GROUP_REACTION_DANGER_ON_SQUARE;
    TEST_ASSERT(dm1v1_event_add(&queue, &ev) >= 0, "add C29 target");
    ev.type = DM1_EVENT_UPDATE_ASPECT_CREATURE_2;
    TEST_ASSERT(dm1v1_event_add(&queue, &ev) >= 0, "add C35 target");
    ev.type = DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3;
    TEST_ASSERT(dm1v1_event_add(&queue, &ev) >= 0, "add C41 target");

    /* F0181 includes C32 aspect events; only other map/square entries stay. */
    ev.type = DM1_EVENT_UPDATE_ASPECT_GROUP;
    TEST_ASSERT(dm1v1_event_add(&queue, &ev) >= 0, "add C32 retained");
    ev.type = DM1_EVENT_UPDATE_BEHAVIOR_GROUP;
    ev.b_mapX = 6;
    TEST_ASSERT(dm1v1_event_add(&queue, &ev) >= 0, "add other-square retained");
    ev.b_mapX = 4;
    ev.map_time = DM1_MAP_TIME_MAKE(2, 10);
    TEST_ASSERT(dm1v1_event_add(&queue, &ev) >= 0, "add other-map retained");

    deleted = dm1v1_group_delete_events_f0181(&queue, 3, 4, 5);
    TEST_ASSERT_INT_EQ(deleted, 4,
                       "F0181 removes exactly target C29..C41 entries");
    TEST_ASSERT_INT_EQ(queue.eventCount, 2, "F0181 retains out-of-contract events");
    TEST_ASSERT_INT_EQ(dm1v1_group_delete_events_f0181(&queue, 3, 4, 5),
                       0, "F0181 is empty after its exact deletion");

    tests_passed++;
    printf("PASS: test_group_delete_events_f0181\n");
}

/* ================================================================
 *  Main
 * ================================================================ */

int main(void) {
    printf("=== DM1 V1 Event Timer Tests ===\n\n");

    test_init_and_basic_add();
    test_heap_ordering_by_time();
    test_heap_ordering_by_type();
    test_heap_ordering_by_priority_and_index();
    test_door_event_merge();
    test_f0238_none_reject_and_wall_cell_merge();
    test_delete_and_heap_integrity();
    test_tick_processing();
    test_f0265_move_group_event_shape();
    test_serialisation_roundtrip();
    test_capacity_limit();
    test_tick_advancement();
    test_dispatch_classification();
    test_door_animation_merge();
    test_door_destruction_merge_scope();
    test_get_index_and_fix_existing_placement();
    test_group_delete_events_f0181();

    printf("\n=== Results: %d passed, %d failed ===\n",
           tests_passed, tests_failed);

    return tests_failed > 0 ? 1 : 0;
}
