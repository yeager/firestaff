#include "dm2_v1_timer_queue_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define MAX_TIMERS 32

static DM2_V1_TimerEntry entries[MAX_TIMERS];
static int16_t indices[MAX_TIMERS];
static DM2_V1_TimerQueue q;

static void reset_queue(void)
{
    memset(entries, 0, sizeof(entries));
    memset(indices, 0, sizeof(indices));
    dm2_v1_timer_queue_init(&q, entries, indices, MAX_TIMERS);
}

static DM2_V1_TimerEntry make_timer(int32_t tick, uint8_t type, uint8_t actor)
{
    DM2_V1_TimerEntry t;
    dm2_v1_timer_entry_init(&t);
    dm2_v1_timer_set_mticks(&t, 0, tick);
    t.ttype = type;
    t.actor = actor;
    return t;
}

static void test_init(void)
{
    reset_queue();
    assert(q.num_timers == 0);
    assert(q.deferred_sift == -1);
    assert(q.max_timers == MAX_TIMERS);
    printf("test_init OK\n");
}

static void test_queue_single(void)
{
    reset_queue();
    DM2_V1_TimerEntry t = make_timer(100, 1, 0);
    int16_t slot = dm2_v1_timer_queue(&q, &t);
    assert(slot >= 0);
    assert(q.num_timers == 1);
    assert(dm2_v1_timer_get_ticks(&q.entries[slot]) == 100);
    printf("test_queue_single OK\n");
}

static void test_queue_notype_rejected(void)
{
    reset_queue();
    DM2_V1_TimerEntry t = make_timer(100, 0, 0);
    int16_t slot = dm2_v1_timer_queue(&q, &t);
    assert(slot == -1);
    assert(q.num_timers == 0);
    printf("test_queue_notype_rejected OK\n");
}

static void test_min_heap_order(void)
{
    reset_queue();
    DM2_V1_TimerEntry t1 = make_timer(300, 1, 0);
    DM2_V1_TimerEntry t2 = make_timer(100, 1, 0);
    DM2_V1_TimerEntry t3 = make_timer(200, 1, 0);
    dm2_v1_timer_queue(&q, &t1);
    dm2_v1_timer_queue(&q, &t2);
    dm2_v1_timer_queue(&q, &t3);
    assert(q.num_timers == 3);

    DM2_V1_TimerEntry out;
    dm2_v1_timer_get_and_delete_next(&q, &out);
    assert(dm2_v1_timer_get_ticks(&out) == 100);
    dm2_v1_timer_get_and_delete_next(&q, &out);
    assert(dm2_v1_timer_get_ticks(&out) == 200);
    dm2_v1_timer_get_and_delete_next(&q, &out);
    assert(dm2_v1_timer_get_ticks(&out) == 300);
    assert(q.num_timers == 0);
    printf("test_min_heap_order OK\n");
}

static void test_is_due(void)
{
    reset_queue();
    DM2_V1_TimerEntry t = make_timer(50, 1, 0);
    dm2_v1_timer_queue(&q, &t);

    q.gametick = 49;
    assert(!dm2_v1_timer_is_due(&q));
    q.gametick = 50;
    assert(dm2_v1_timer_is_due(&q));
    q.gametick = 100;
    assert(dm2_v1_timer_is_due(&q));
    printf("test_is_due OK\n");
}

static void test_is_due_empty(void)
{
    reset_queue();
    q.gametick = 100;
    assert(!dm2_v1_timer_is_due(&q));
    printf("test_is_due_empty OK\n");
}

static void test_delete_middle(void)
{
    reset_queue();
    DM2_V1_TimerEntry t1 = make_timer(100, 1, 0);
    DM2_V1_TimerEntry t2 = make_timer(200, 2, 0);
    DM2_V1_TimerEntry t3 = make_timer(300, 3, 0);
    dm2_v1_timer_queue(&q, &t1);
    int16_t s2 = dm2_v1_timer_queue(&q, &t2);
    dm2_v1_timer_queue(&q, &t3);

    dm2_v1_timer_delete(&q, s2);
    assert(q.num_timers == 2);

    DM2_V1_TimerEntry out;
    dm2_v1_timer_get_and_delete_next(&q, &out);
    assert(dm2_v1_timer_get_ticks(&out) == 100);
    dm2_v1_timer_get_and_delete_next(&q, &out);
    assert(dm2_v1_timer_get_ticks(&out) == 300);
    printf("test_delete_middle OK\n");
}

static void test_tiebreak_by_type(void)
{
    reset_queue();
    DM2_V1_TimerEntry t1 = make_timer(100, 5, 0);
    DM2_V1_TimerEntry t2 = make_timer(100, 10, 0);
    dm2_v1_timer_queue(&q, &t1);
    dm2_v1_timer_queue(&q, &t2);

    DM2_V1_TimerEntry out;
    dm2_v1_timer_get_and_delete_next(&q, &out);
    assert(out.ttype == 10);
    dm2_v1_timer_get_and_delete_next(&q, &out);
    assert(out.ttype == 5);
    printf("test_tiebreak_by_type OK\n");
}

static void test_tiebreak_by_actor(void)
{
    reset_queue();
    DM2_V1_TimerEntry t1 = make_timer(100, 5, 3);
    DM2_V1_TimerEntry t2 = make_timer(100, 5, 7);
    dm2_v1_timer_queue(&q, &t1);
    dm2_v1_timer_queue(&q, &t2);

    DM2_V1_TimerEntry out;
    dm2_v1_timer_get_and_delete_next(&q, &out);
    assert(out.actor == 7);
    dm2_v1_timer_get_and_delete_next(&q, &out);
    assert(out.actor == 3);
    printf("test_tiebreak_by_actor OK\n");
}

static void test_map_packing(void)
{
    DM2_V1_TimerEntry t;
    dm2_v1_timer_entry_init(&t);
    dm2_v1_timer_set_mticks(&t, 5, 0x123456);
    assert(dm2_v1_timer_get_map(&t) == 5);
    assert(dm2_v1_timer_get_ticks(&t) == 0x123456);
    printf("test_map_packing OK\n");
}

static void test_inc_data(void)
{
    DM2_V1_TimerEntry t;
    dm2_v1_timer_entry_init(&t);
    dm2_v1_timer_set_mticks(&t, 0, 99);
    dm2_v1_timer_inc_data(&t);
    assert(dm2_v1_timer_get_ticks(&t) == 100);
    printf("test_inc_data OK\n");
}

static void test_sort(void)
{
    reset_queue();
    /* Manually place 3 timers in unsorted order */
    entries[0].ttype = 1;
    dm2_v1_timer_set_mticks(&entries[0], 0, 300);
    entries[1].ttype = 2;
    dm2_v1_timer_set_mticks(&entries[1], 0, 100);
    entries[2].ttype = 3;
    dm2_v1_timer_set_mticks(&entries[2], 0, 200);
    q.num_timers = 3;

    dm2_v1_timer_sort(&q);

    DM2_V1_TimerEntry out;
    dm2_v1_timer_get_and_delete_next(&q, &out);
    assert(dm2_v1_timer_get_ticks(&out) == 100);
    dm2_v1_timer_get_and_delete_next(&q, &out);
    assert(dm2_v1_timer_get_ticks(&out) == 200);
    dm2_v1_timer_get_and_delete_next(&q, &out);
    assert(dm2_v1_timer_get_ticks(&out) == 300);
    printf("test_sort OK\n");
}

static void test_reheapify(void)
{
    reset_queue();
    DM2_V1_TimerEntry t1 = make_timer(100, 1, 0);
    DM2_V1_TimerEntry t2 = make_timer(200, 2, 0);
    DM2_V1_TimerEntry t3 = make_timer(300, 3, 0);
    int16_t s1 = dm2_v1_timer_queue(&q, &t1);
    dm2_v1_timer_queue(&q, &t2);
    dm2_v1_timer_queue(&q, &t3);

    /* Change earliest timer to latest */
    dm2_v1_timer_set_mticks(&q.entries[s1], 0, 500);
    dm2_v1_timer_reheapify(&q, s1);

    DM2_V1_TimerEntry out;
    dm2_v1_timer_get_and_delete_next(&q, &out);
    assert(dm2_v1_timer_get_ticks(&out) == 200);
    dm2_v1_timer_get_and_delete_next(&q, &out);
    assert(dm2_v1_timer_get_ticks(&out) == 300);
    dm2_v1_timer_get_and_delete_next(&q, &out);
    assert(dm2_v1_timer_get_ticks(&out) == 500);
    printf("test_reheapify OK\n");
}

static void test_many_timers(void)
{
    reset_queue();
    for (int i = MAX_TIMERS - 1; i >= 0; i--) {
        DM2_V1_TimerEntry t = make_timer(i * 10, 1, 0);
        dm2_v1_timer_queue(&q, &t);
    }
    assert(q.num_timers == MAX_TIMERS);

    for (int i = 0; i < MAX_TIMERS; i++) {
        DM2_V1_TimerEntry out;
        dm2_v1_timer_get_and_delete_next(&q, &out);
        assert(dm2_v1_timer_get_ticks(&out) == i * 10);
    }
    assert(q.num_timers == 0);
    printf("test_many_timers OK\n");
}

static void test_queue_after_delete_reuses_slots(void)
{
    reset_queue();
    DM2_V1_TimerEntry t1 = make_timer(100, 1, 0);
    DM2_V1_TimerEntry t2 = make_timer(200, 2, 0);
    int16_t s1 = dm2_v1_timer_queue(&q, &t1);
    dm2_v1_timer_queue(&q, &t2);

    dm2_v1_timer_delete(&q, s1);
    assert(q.num_timers == 1);

    DM2_V1_TimerEntry t3 = make_timer(50, 3, 0);
    int16_t s3 = dm2_v1_timer_queue(&q, &t3);
    assert(s3 == s1);
    assert(q.num_timers == 2);

    DM2_V1_TimerEntry out;
    dm2_v1_timer_get_and_delete_next(&q, &out);
    assert(dm2_v1_timer_get_ticks(&out) == 50);
    printf("test_queue_after_delete_reuses_slots OK\n");
}

static void test_get_heap_index(void)
{
    reset_queue();
    DM2_V1_TimerEntry t = make_timer(100, 1, 0);
    int16_t slot = dm2_v1_timer_queue(&q, &t);
    int16_t pos = dm2_v1_timer_get_heap_index(&q, slot);
    assert(pos == 0);
    assert(dm2_v1_timer_get_heap_index(&q, 99) == -1);
    printf("test_get_heap_index OK\n");
}

int main(void)
{
    test_init();
    test_queue_single();
    test_queue_notype_rejected();
    test_min_heap_order();
    test_is_due();
    test_is_due_empty();
    test_delete_middle();
    test_tiebreak_by_type();
    test_tiebreak_by_actor();
    test_map_packing();
    test_inc_data();
    test_sort();
    test_reheapify();
    test_many_timers();
    test_queue_after_delete_reuses_slots();
    test_get_heap_index();
    printf("All dm2_v1_timer_queue tests passed.\n");
    return 0;
}
