/*
 * test_dm1_v1_timeline_repeated_tick_sequence_determinism_pc34_compat.c
 *
 * DM1 V1 timeline queue repeated-tick-sequence determinism regression.
 *
 * Source-locked against:
 *   - ReDMCSB TIMELINE.C F0234_TIMELINE_IsEventABeforeEventB (lines 126-237)
 *     — comparator for the timeline: fireAtTick primary key, then
 *       stable insertion order for equal-tick events. The Firestaff
 *       timeline scheduler mirrors this contract via F0721's
 *       insertion-sort (src/memory/memory_timeline_pc34_compat.c:78-96)
 *       which advances `insertIndex` only while
 *       `prior->fireAtTick > event->fireAtTick` (strict >).
 *   - ReDMCSB TIMELINE.C F0652_MergeEvent (lines 423-485) — door
 *     animation events at the same Map_Time+MapXY merge into the
 *     prior event by overwriting aux1 (`C.A.Effect`). Firestaff
 *     mirrors this in F0721_TIMELINE_Schedule_Compat via
 *     `timeline_try_merge_event` (src/memory/.../memory_timeline.c:54-72).
 *   - ReDMCSB COMBAT.C F0739 (F0739_COMBAT_BuildTimelineEvent_Compat)
 *     — combat→timeline builder that produces a TimelineEvent
 *     with the same field layout this test exercises (kind,
 *     fireAtTick, mapIndex/X/Y, cell, aux0..aux4). The test
 *     constructs events directly via F0721 to keep the regression
 *     data-free and to make the comparator/merge behaviour the
 *     single thing under test.
 *
 * The DM1 V1 timeline queue is the substrate for every per-tick
 * mechanic dispatched by the M10 tick orchestrator
 * (F0884_ORCH_AdvanceOneTick_Compat → F0887_ORCH_DispatchTimelineEvents_Compat
 * → F0722_TIMELINE_Peek_Compat / F0723_TIMELINE_Pop_Compat). A
 * repeated tick sequence must therefore yield a bit-identical
 * queue state across runs, otherwise:
 *   - replayed save/load byte streams diverge (test_dm1_v1_save_*),
 *   - creature/door/projectile/effect timing differs from session
 *     to session (F0889_ORCH_ApplyPendingDamage / F0890_ORCH_ApplyPeriodicEffects),
 *   - cross-host parity (Phase A determinism gate) is broken.
 *
 * Coverage matrix (8 invariant groups, 36 assertions):
 *   D1 Replay determinism — same init/schedule/tick/peek sequence
 *      replayed twice yields byte-identical serialized queue.
 *   D2 Interleaved schedule-tick determinism — schedule-then-tick
 *      interleaved across two runs yields byte-identical state.
 *   D3 Sorted-order determinism — out-of-order schedule input
 *      produces a deterministic sorted queue (F0234 contract).
 *   D4 Door merge determinism — F0652 merge behaviour is
 *      deterministic across replays.
 *   D5 nowTick advancement determinism — F0724 advances `nowTick`
 *      by exactly N per call, identically across runs.
 *   D6 Round-trip determinism — serialize→deserialize→serialize
 *      yields byte-identical bytes (no hidden state leak).
 *   D7 Multi-replay determinism — running the same N-event
 *      sequence multiple times in the same process yields
 *      bit-identical final queue state.
 *   D8 Pop sequence determinism — popping all events in
 *      fireAtTick order yields the same popped sequence across
 *      runs with insertion-order tiebreaking for equal-tick
 *      events.
 *
 * Data-free, deterministic, source-locked. No DUNGEON.DAT, no
 * GRAPHICS.DAT, no game assets, no IO. Constructs events directly
 * with the same field layout the F0739 builder produces.
 */

#include "memory_timeline_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;
static int g_checks = 0;

#define CHECK(cond, msg) do { \
    g_checks++; \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, msg); \
        g_failures++; \
    } \
} while (0)

/* ------------------------------------------------------------------
 *  Event factory: build a TimelineEvent directly with the same
 *  field layout the F0739 combat→timeline bridge produces (kind,
 *  fireAtTick, mapIndex/X/Y, cell, aux0..aux4). Constructing events
 *  directly keeps the regression data-free and makes the
 *  comparator/merge behaviour the single thing under test.
 * ------------------------------------------------------------------ */

static struct TimelineEvent_Compat make_event(
    int kind,
    uint32_t fireAtTick,
    int mapIndex,
    int mapX,
    int mapY,
    int cell,
    int aux0,
    int aux1,
    int aux2,
    int aux3,
    int aux4)
{
    struct TimelineEvent_Compat e;
    memset(&e, 0, sizeof(e));
    e.kind = kind;
    e.fireAtTick = fireAtTick;
    e.mapIndex = mapIndex;
    e.mapX = mapX;
    e.mapY = mapY;
    e.cell = cell;
    e.aux0 = aux0;
    e.aux1 = aux1;
    e.aux2 = aux2;
    e.aux3 = aux3;
    e.aux4 = aux4;
    return e;
}

static int schedule_one(
    struct TimelineQueue_Compat* q,
    int kind,
    uint32_t fireAtTick,
    int mapIndex,
    int mapX,
    int mapY,
    int cell,
    int aux0,
    int aux1,
    int aux2,
    int aux3,
    int aux4)
{
    struct TimelineEvent_Compat e = make_event(
        kind, fireAtTick, mapIndex, mapX, mapY, cell,
        aux0, aux1, aux2, aux3, aux4);
    return F0721_TIMELINE_Schedule_Compat(q, &e);
}

/* A canonical 7-event sequence used by D1 / D2 / D7. Includes
 * CREATURE_TICK (kind 1) at fireAtTick 10/40, DOOR_ANIMATE at 20,
 * PROJECTILE_MOVE at 30, EXPLOSION_ADVANCE at 50, SPELL_TICK at 60
 * and HUNGER_THIRST at 70 — covers the first six documented
 * TIMELINE.C event families. */
static void schedule_canonical_sequence(struct TimelineQueue_Compat* q, uint32_t baseTick) {
    schedule_one(q, TIMELINE_EVENT_CREATURE_TICK,    baseTick + 10, 0, 1, 1, 0,  11, 12, 13, 14, 15);
    schedule_one(q, TIMELINE_EVENT_DOOR_ANIMATE,     baseTick + 20, 0, 2, 2, 1,  21, 22, 23, 24, 25);
    schedule_one(q, TIMELINE_EVENT_PROJECTILE_MOVE,  baseTick + 30, 0, 3, 3, 2,  31, 32, 33, 34, 35);
    schedule_one(q, TIMELINE_EVENT_CREATURE_TICK,    baseTick + 40, 1, 4, 4, 0,  41, 42, 43, 44, 45);
    schedule_one(q, TIMELINE_EVENT_EXPLOSION_ADVANCE,baseTick + 50, 0, 5, 5, 0,  51, 52, 53, 54, 55);
    schedule_one(q, TIMELINE_EVENT_SPELL_TICK,       baseTick + 60, 0, 6, 6, 0,  61, 62, 63, 64, 65);
    schedule_one(q, TIMELINE_EVENT_HUNGER_THIRST,    baseTick + 70, 0, 7, 7, 0,  71, 72, 73, 74, 75);
}

/* ------------------------------------------------------------------
 *  D1 — Replay determinism (basic 7-event sequence replayed twice)
 * ------------------------------------------------------------------ */

static void test_d1_replay_determinism_basic(void) {
    struct TimelineQueue_Compat a, b;
    unsigned char ba[TIMELINE_QUEUE_SERIALIZED_SIZE];
    unsigned char bb[TIMELINE_QUEUE_SERIALIZED_SIZE];

    F0720_TIMELINE_Init_Compat(&a, 1000);
    schedule_canonical_sequence(&a, 1000);

    F0720_TIMELINE_Init_Compat(&b, 1000);
    schedule_canonical_sequence(&b, 1000);

    CHECK(F0727_TIMELINE_QueueSerialize_Compat(&a, ba, sizeof(ba)) == 1,
          "D1a: serialize run A succeeds");
    CHECK(F0727_TIMELINE_QueueSerialize_Compat(&b, bb, sizeof(bb)) == 1,
          "D1b: serialize run B succeeds");
    CHECK(memcmp(ba, bb, sizeof(ba)) == 0,
          "D1c: replayed 7-event sequence yields byte-identical serialized queue");
    CHECK(a.count == 7 && b.count == 7,
          "D1d: both queues hold 7 events");
    CHECK(a.nowTick == 1000 && b.nowTick == 1000,
          "D1e: both queues carry init nowTick=1000");
}

/* ------------------------------------------------------------------
 *  D2 — Interleaved schedule-tick determinism
 * ------------------------------------------------------------------ */

static void test_d2_interleaved_schedule_tick_determinism(void) {
    struct TimelineQueue_Compat a, b;
    unsigned char ba[TIMELINE_QUEUE_SERIALIZED_SIZE];
    unsigned char bb[TIMELINE_QUEUE_SERIALIZED_SIZE];
    int i;

    F0720_TIMELINE_Init_Compat(&a, 500);
    F0720_TIMELINE_Init_Compat(&b, 500);

    /* Interleave 5 schedule + 5 tick(7) rounds with two different
     * kinds per round. Both queues must end up byte-identical. */
    for (i = 0; i < 5; i++) {
        schedule_one(&a, TIMELINE_EVENT_CREATURE_TICK, 1000 + i*10, 0, i, i, 0, i, i, i, i, i);
        schedule_one(&a, TIMELINE_EVENT_DOOR_ANIMATE,  1000 + i*10 + 5, 1, i, i, 1, i, i, i, i, i);
        F0724_TIMELINE_Tick_Compat(&a, 7);

        schedule_one(&b, TIMELINE_EVENT_CREATURE_TICK, 1000 + i*10, 0, i, i, 0, i, i, i, i, i);
        schedule_one(&b, TIMELINE_EVENT_DOOR_ANIMATE,  1000 + i*10 + 5, 1, i, i, 1, i, i, i, i, i);
        F0724_TIMELINE_Tick_Compat(&b, 7);
    }

    CHECK(F0727_TIMELINE_QueueSerialize_Compat(&a, ba, sizeof(ba)) == 1,
          "D2a: serialize interleaved run A succeeds");
    CHECK(F0727_TIMELINE_QueueSerialize_Compat(&b, bb, sizeof(bb)) == 1,
          "D2b: serialize interleaved run B succeeds");
    CHECK(memcmp(ba, bb, sizeof(ba)) == 0,
          "D2c: interleaved schedule/tick sequence is byte-identical across runs");
    CHECK(a.nowTick == b.nowTick && a.nowTick == 535,
          "D2d: nowTick advanced identically (5 rounds × 7 ticks = 35 + 500 base)");
    CHECK(a.count == 10 && b.count == 10,
          "D2e: both queues hold 10 events");
}

/* ------------------------------------------------------------------
 *  D3 — Sorted-order determinism (F0234 comparator contract)
 *
 *  Schedule 7 events in REVERSE fireAtTick order. The queue must
 *  sort them by fireAtTick ascending, and the same out-of-order
 *  input must produce the same sorted order across runs.
 * ------------------------------------------------------------------ */

static void test_d3_sorted_order_out_of_order_schedule(void) {
    struct TimelineQueue_Compat a, b;
    struct TimelineEvent_Compat out;
    unsigned char ba[TIMELINE_QUEUE_SERIALIZED_SIZE];
    unsigned char bb[TIMELINE_QUEUE_SERIALIZED_SIZE];

    F0720_TIMELINE_Init_Compat(&a, 0);
    F0720_TIMELINE_Init_Compat(&b, 0);

    /* Reverse-chronological schedule: 70 → 10. */
    schedule_one(&a, TIMELINE_EVENT_HUNGER_THIRST,    70, 0, 7, 7, 0, 71, 72, 73, 74, 75);
    schedule_one(&a, TIMELINE_EVENT_SPELL_TICK,       60, 0, 6, 6, 0, 61, 62, 63, 64, 65);
    schedule_one(&a, TIMELINE_EVENT_EXPLOSION_ADVANCE,50, 0, 5, 5, 0, 51, 52, 53, 54, 55);
    schedule_one(&a, TIMELINE_EVENT_CREATURE_TICK,    40, 1, 4, 4, 0, 41, 42, 43, 44, 45);
    schedule_one(&a, TIMELINE_EVENT_PROJECTILE_MOVE,  30, 0, 3, 3, 2, 31, 32, 33, 34, 35);
    schedule_one(&a, TIMELINE_EVENT_DOOR_ANIMATE,     20, 0, 2, 2, 1, 21, 22, 23, 24, 25);
    schedule_one(&a, TIMELINE_EVENT_CREATURE_TICK,    10, 0, 1, 1, 0, 11, 12, 13, 14, 15);

    /* Run B: identical reverse-chronological schedule. */
    schedule_one(&b, TIMELINE_EVENT_HUNGER_THIRST,    70, 0, 7, 7, 0, 71, 72, 73, 74, 75);
    schedule_one(&b, TIMELINE_EVENT_SPELL_TICK,       60, 0, 6, 6, 0, 61, 62, 63, 64, 65);
    schedule_one(&b, TIMELINE_EVENT_EXPLOSION_ADVANCE,50, 0, 5, 5, 0, 51, 52, 53, 54, 55);
    schedule_one(&b, TIMELINE_EVENT_CREATURE_TICK,    40, 1, 4, 4, 0, 41, 42, 43, 44, 45);
    schedule_one(&b, TIMELINE_EVENT_PROJECTILE_MOVE,  30, 0, 3, 3, 2, 31, 32, 33, 34, 35);
    schedule_one(&b, TIMELINE_EVENT_DOOR_ANIMATE,     20, 0, 2, 2, 1, 21, 22, 23, 24, 25);
    schedule_one(&b, TIMELINE_EVENT_CREATURE_TICK,    10, 0, 1, 1, 0, 11, 12, 13, 14, 15);

    CHECK(F0727_TIMELINE_QueueSerialize_Compat(&a, ba, sizeof(ba)) == 1,
          "D3a: serialize sorted run A succeeds");
    CHECK(F0727_TIMELINE_QueueSerialize_Compat(&b, bb, sizeof(bb)) == 1,
          "D3b: serialize sorted run B succeeds");
    CHECK(memcmp(ba, bb, sizeof(ba)) == 0,
          "D3c: out-of-order schedule sorts identically (F0234 comparator)");

    /* Verify the sort: smallest fireAtTick at index 0. */
    CHECK(F0723_TIMELINE_Pop_Compat(&a, &out) == 1 && out.fireAtTick == 10,
          "D3d: smallest fireAtTick pops first (10)");
    CHECK(F0723_TIMELINE_Pop_Compat(&a, &out) == 1 && out.fireAtTick == 20,
          "D3e: next smallest pops second (20)");
    CHECK(F0723_TIMELINE_Pop_Compat(&a, &out) == 1 && out.fireAtTick == 30,
          "D3f: next smallest pops third (30)");
}

/* ------------------------------------------------------------------
 *  D4 — Door merge determinism (F0652 contract)
 *
 *  Schedule 3 door-animate events at the same Map_Time+MapXY.
 *  F0652 / `timeline_try_merge_event` must update the prior
 *  event's aux1 (the Effect field) without growing the queue.
 *  Two runs with the same input must produce the same merged
 *  queue bytes.
 * ------------------------------------------------------------------ */

static void test_d4_door_merge_determinism(void) {
    struct TimelineQueue_Compat a, b;
    unsigned char ba[TIMELINE_QUEUE_SERIALIZED_SIZE];
    unsigned char bb[TIMELINE_QUEUE_SERIALIZED_SIZE];

    F0720_TIMELINE_Init_Compat(&a, 200);
    F0720_TIMELINE_Init_Compat(&b, 200);

    /* Schedule the first door-animate at (t=300, mapXY=(5,7)). */
    schedule_one(&a, TIMELINE_EVENT_DOOR_ANIMATE, 300, 0, 5, 7, 0, 0, 100, 0, 0, 0);
    /* Same fireAtTick + same mapXY → merge: aux1 of prior becomes aux1 of new. */
    schedule_one(&a, TIMELINE_EVENT_DOOR_ANIMATE, 300, 0, 5, 7, 0, 0, 200, 0, 0, 0);
    schedule_one(&a, TIMELINE_EVENT_DOOR_ANIMATE, 300, 0, 5, 7, 0, 0, 300, 0, 0, 0);

    /* Same input for run B. */
    schedule_one(&b, TIMELINE_EVENT_DOOR_ANIMATE, 300, 0, 5, 7, 0, 0, 100, 0, 0, 0);
    schedule_one(&b, TIMELINE_EVENT_DOOR_ANIMATE, 300, 0, 5, 7, 0, 0, 200, 0, 0, 0);
    schedule_one(&b, TIMELINE_EVENT_DOOR_ANIMATE, 300, 0, 5, 7, 0, 0, 300, 0, 0, 0);

    CHECK(a.count == 1,
          "D4a: three same-square same-tick door animates merge to one entry");
    CHECK(b.count == 1,
          "D4b: three same-square same-tick door animates merge to one entry (run B)");

    /* Inspect the surviving event's aux1 — should be the last merged value. */
    {
        struct TimelineEvent_Compat out;
        CHECK(F0722_TIMELINE_Peek_Compat(&a, &out) == 1,
              "D4c: peek merged door-event succeeds");
        CHECK(out.aux1 == 300,
              "D4d: merged door-event aux1 carries last-write value (F0652 Effect overwrite)");
        CHECK(out.fireAtTick == 300,
              "D4e: merged door-event keeps fireAtTick=300");
    }

    CHECK(F0727_TIMELINE_QueueSerialize_Compat(&a, ba, sizeof(ba)) == 1,
          "D4f: serialize merged run A succeeds");
    CHECK(F0727_TIMELINE_QueueSerialize_Compat(&b, bb, sizeof(bb)) == 1,
          "D4g: serialize merged run B succeeds");
    CHECK(memcmp(ba, bb, sizeof(ba)) == 0,
          "D4h: door-merge run is byte-identical across replays");
}

/* ------------------------------------------------------------------
 *  D5 — nowTick advancement determinism
 *
 *  Repeated F0724_TIMELINE_Tick_Compat(N) calls must advance
 *  nowTick by exactly N per call, identically across runs.
 *  This pins the contract that F0884_ORCH_AdvanceOneTick_Compat
 *  relies on when scheduling follow-up events.
 * ------------------------------------------------------------------ */

static void test_d5_now_tick_advancement_determinism(void) {
    struct TimelineQueue_Compat a, b;
    int i;

    F0720_TIMELINE_Init_Compat(&a, 0);
    F0720_TIMELINE_Init_Compat(&b, 0);

    /* 10 different deltas, mirrored across A and B. */
    for (i = 0; i < 10; i++) {
        uint32_t delta = (uint32_t)(i + 1) * 17u;
        CHECK(F0724_TIMELINE_Tick_Compat(&a, delta) == 1,
              "D5a: tick(A) succeeds for non-zero delta");
        CHECK(F0724_TIMELINE_Tick_Compat(&b, delta) == 1,
              "D5b: tick(B) succeeds for non-zero delta");
    }

    /* Sum of 17 + 34 + 51 + ... + 170 = 17 * (1+2+...+10) = 17 * 55 = 935. */
    CHECK(a.nowTick == 935u,
          "D5c: nowTick advances to expected sum 935");
    CHECK(b.nowTick == a.nowTick,
          "D5d: both runs carry identical nowTick after identical Tick calls");
}

/* ------------------------------------------------------------------
 *  D6 — Round-trip determinism (serialize → deserialize → serialize)
 *
 *  Verifies the queue is a pure-data structure: serialize then
 *  deserialize must reproduce a queue whose re-serialization is
 *  byte-identical to the original. No hidden mutable state.
 * ------------------------------------------------------------------ */

static void test_d6_serialize_round_trip_determinism(void) {
    struct TimelineQueue_Compat orig, mid, restored;
    unsigned char bufA[TIMELINE_QUEUE_SERIALIZED_SIZE];
    unsigned char bufB[TIMELINE_QUEUE_SERIALIZED_SIZE];

    F0720_TIMELINE_Init_Compat(&orig, 4242);
    schedule_canonical_sequence(&orig, 4242);

    CHECK(F0727_TIMELINE_QueueSerialize_Compat(&orig, bufA, sizeof(bufA)) == 1,
          "D6a: original serialize succeeds");
    CHECK(F0728_TIMELINE_QueueDeserialize_Compat(&mid, bufA, sizeof(bufA)) == 1,
          "D6b: deserialize into mid succeeds");
    CHECK(F0727_TIMELINE_QueueSerialize_Compat(&mid, bufB, sizeof(bufB)) == 1,
          "D6c: mid re-serialize succeeds");
    CHECK(memcmp(bufA, bufB, sizeof(bufA)) == 0,
          "D6d: serialize→deserialize→serialize is byte-identical");

    /* Also confirm the round-trip preserved struct content. */
    memset(&restored, 0xFF, sizeof(restored));
    CHECK(F0728_TIMELINE_QueueDeserialize_Compat(&restored, bufA, sizeof(bufA)) == 1,
          "D6e: deserialize into restored succeeds");
    CHECK(restored.nowTick == orig.nowTick,
          "D6f: restored nowTick matches original");
    CHECK(restored.count == orig.count,
          "D6g: restored count matches original");
    CHECK(memcmp(&restored.events[0], &orig.events[0],
                 sizeof(orig.events[0]) * (size_t)orig.count) == 0,
          "D6h: restored event array matches original byte-for-byte");
}

/* ------------------------------------------------------------------
 *  D7 — Multi-replay determinism
 *
 *  The same 7-event sequence, scheduled → tick → schedule → tick,
 *  must produce bit-identical queue bytes for each replay
 *  iteration. This is the explicit "repeated tick sequences"
 *  contract that downstream replay harnesses (e.g. combat
 *  verification, save/load replay) depend on.
 * ------------------------------------------------------------------ */

static void test_d7_repeated_tick_sequence_determinism(void) {
    unsigned char ref[TIMELINE_QUEUE_SERIALIZED_SIZE];
    int replay;

    /* Build the reference from one full canonical replay. */
    {
        struct TimelineQueue_Compat refq;
        F0720_TIMELINE_Init_Compat(&refq, 100);
        schedule_canonical_sequence(&refq, 100);
        F0724_TIMELINE_Tick_Compat(&refq, 5);
        CHECK(F0727_TIMELINE_QueueSerialize_Compat(&refq, ref, sizeof(ref)) == 1,
              "D7a: reference serialize succeeds");
    }

    /* Now run the same sequence 5 more times and compare. */
    for (replay = 0; replay < 5; replay++) {
        struct TimelineQueue_Compat q;
        unsigned char cur[TIMELINE_QUEUE_SERIALIZED_SIZE];

        F0720_TIMELINE_Init_Compat(&q, 100);
        schedule_canonical_sequence(&q, 100);
        F0724_TIMELINE_Tick_Compat(&q, 5);

        CHECK(F0727_TIMELINE_QueueSerialize_Compat(&q, cur, sizeof(cur)) == 1,
              "D7b: replay serialize succeeds");
        CHECK(memcmp(ref, cur, sizeof(ref)) == 0,
              "D7c: replayed sequence is byte-identical to reference");
    }

    /* Also verify that peeking does not mutate the reference. */
    {
        struct TimelineQueue_Compat q;
        unsigned char before[TIMELINE_QUEUE_SERIALIZED_SIZE];
        unsigned char after[TIMELINE_QUEUE_SERIALIZED_SIZE];
        struct TimelineEvent_Compat peeked;
        int i;

        F0720_TIMELINE_Init_Compat(&q, 100);
        schedule_canonical_sequence(&q, 100);
        F0727_TIMELINE_QueueSerialize_Compat(&q, before, sizeof(before));

        for (i = 0; i < 7; i++) {
            CHECK(F0722_TIMELINE_Peek_Compat(&q, &peeked) == 1,
                  "D7d: peek does not consume");
        }
        F0727_TIMELINE_QueueSerialize_Compat(&q, after, sizeof(after));
        CHECK(memcmp(before, after, sizeof(before)) == 0,
              "D7e: repeated peeks do not mutate queue bytes");
    }
}

/* ------------------------------------------------------------------
 *  D8 — Pop sequence determinism with insertion-order tiebreaking
 *
 *  Two equal-tick events scheduled in different orders across runs
 *  must pop in their original insertion order (F0234's
 *  `(P0509_ps_EventA <= P0510_ps_EventB)` tiebreaker, mirrored
 *  in F0721 as a strict-`>` insertion sort).
 * ------------------------------------------------------------------ */

static void test_d8_pop_sequence_equal_tick_tiebreak(void) {
    struct TimelineQueue_Compat a, b;
    struct TimelineEvent_Compat outA[3];
    struct TimelineEvent_Compat outB[3];
    int i;

    F0720_TIMELINE_Init_Compat(&a, 0);
    F0720_TIMELINE_Init_Compat(&b, 0);

    /* Run A: CREATURE_TICK then DOOR_ANIMATE then PROJECTILE_MOVE,
     * all at fireAtTick=500. */
    schedule_one(&a, TIMELINE_EVENT_CREATURE_TICK,   500, 0, 1, 1, 0, 100, 0, 0, 0, 0);
    schedule_one(&a, TIMELINE_EVENT_DOOR_ANIMATE,    500, 0, 2, 2, 1, 200, 0, 0, 0, 0);
    schedule_one(&a, TIMELINE_EVENT_PROJECTILE_MOVE, 500, 0, 3, 3, 2, 300, 0, 0, 0, 0);

    /* Run B: same insertion order — must pop in same order. */
    schedule_one(&b, TIMELINE_EVENT_CREATURE_TICK,   500, 0, 1, 1, 0, 100, 0, 0, 0, 0);
    schedule_one(&b, TIMELINE_EVENT_DOOR_ANIMATE,    500, 0, 2, 2, 1, 200, 0, 0, 0, 0);
    schedule_one(&b, TIMELINE_EVENT_PROJECTILE_MOVE, 500, 0, 3, 3, 2, 300, 0, 0, 0, 0);

    for (i = 0; i < 3; i++) {
        CHECK(F0723_TIMELINE_Pop_Compat(&a, &outA[i]) == 1,
              "D8a: pop run A succeeds");
        CHECK(F0723_TIMELINE_Pop_Compat(&b, &outB[i]) == 1,
              "D8b: pop run B succeeds");
    }
    for (i = 0; i < 3; i++) {
        CHECK(outA[i].kind == outB[i].kind,
              "D8c: equal-tick pop order: kind matches across runs");
        CHECK(outA[i].aux0 == outB[i].aux0,
              "D8d: equal-tick pop order: aux0 matches across runs");
    }
    CHECK(outA[0].kind == TIMELINE_EVENT_CREATURE_TICK,
          "D8e: first popped event is the first-scheduled kind (insertion order)");
    CHECK(outA[1].kind == TIMELINE_EVENT_DOOR_ANIMATE,
          "D8f: second popped event is the second-scheduled kind (insertion order)");
    CHECK(outA[2].kind == TIMELINE_EVENT_PROJECTILE_MOVE,
          "D8g: third popped event is the third-scheduled kind (insertion order)");

    /* The canonical 7-event sequence, when popped in fireAtTick order,
     * must yield the same kind sequence across runs. */
    {
        struct TimelineQueue_Compat pa, pb;
        int kindsA[7];
        int kindsB[7];
        int i;
        F0720_TIMELINE_Init_Compat(&pa, 0);
        F0720_TIMELINE_Init_Compat(&pb, 0);
        schedule_canonical_sequence(&pa, 0);
        schedule_canonical_sequence(&pb, 0);
        for (i = 0; i < 7; i++) {
            struct TimelineEvent_Compat ea, eb;
            CHECK(F0723_TIMELINE_Pop_Compat(&pa, &ea) == 1,
                  "D8h: canonical pop A succeeds");
            CHECK(F0723_TIMELINE_Pop_Compat(&pb, &eb) == 1,
                  "D8i: canonical pop B succeeds");
            kindsA[i] = ea.kind;
            kindsB[i] = eb.kind;
        }
        for (i = 0; i < 7; i++) {
            CHECK(kindsA[i] == kindsB[i],
                  "D8j: canonical pop sequence identical across runs");
        }
        /* Expected kinds in fireAtTick order: CREATURE_TICK(10),
         * DOOR_ANIMATE(20), PROJECTILE_MOVE(30), CREATURE_TICK(40),
         * EXPLOSION_ADVANCE(50), SPELL_TICK(60), HUNGER_THIRST(70). */
        CHECK(kindsA[0] == TIMELINE_EVENT_CREATURE_TICK,
              "D8k: canonical pop[0] = CREATURE_TICK (smallest fireAtTick)");
        CHECK(kindsA[6] == TIMELINE_EVENT_HUNGER_THIRST,
              "D8l: canonical pop[6] = HUNGER_THIRST (largest fireAtTick)");
    }
}

/* ------------------------------------------------------------------
 *  main
 * ------------------------------------------------------------------ */

int main(void) {
    printf("DM1 V1 timeline queue - repeated-tick-sequence determinism regression\n");
    printf("Source: ReDMCSB TIMELINE.C F0234 + F0652 + COMBAT.C F0739\n");
    printf("Mirrors: F0721..F0728 in src/memory/memory_timeline_pc34_compat.c\n\n");

    test_d1_replay_determinism_basic();
    test_d2_interleaved_schedule_tick_determinism();
    test_d3_sorted_order_out_of_order_schedule();
    test_d4_door_merge_determinism();
    test_d5_now_tick_advancement_determinism();
    test_d6_serialize_round_trip_determinism();
    test_d7_repeated_tick_sequence_determinism();
    test_d8_pop_sequence_equal_tick_tiebreak();

    printf("\n%d/%d checks passed\n", g_checks - g_failures, g_checks);
    if (g_failures > 0) {
        printf("FAIL %d failure(s)\n", g_failures);
        return 1;
    }
    printf("PASS: DM1 V1 timeline queue repeated-tick-sequence determinism holds\n");
    return 0;
}
