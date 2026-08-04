/*
 * test_dm2_v1_ccm_loop_pc34_compat.c — the CCM stream owner/grammar
 * DM2_13e4_0982 bounded slice (the message loop).
 *
 * Source-lock anchors:
 *   skproject/SKULLWIN/c_ai.cpp:5341-5647   DM2_13e4_0982
 *   skproject/SKULLWIN/c_ai.cpp:2340-2430   DM2_13e4_01a3
 *   skproject/SKULLWIN/c_ai.cpp:5275-5338   DM2_50CB
 *   skproject/SKULLWIN/c_ai.cpp:5602-5604   continue/break grammar
 *   skproject/SKULLWIN/c_timer.h:66-68      setmticks / adddata
 *
 * Synthetic dtWordValue/RAW GDAT fixture (no game data).
 *
 * Fixture map:
 *   type 12 -> AI row 5 -> flags 0x0000 (dynamic), BaseHP 40, jitter 5
 *     attribution: [0x05->2] [0x13->2] [0x07->6] [0xffff->0]
 *     info row 2 = {0x05,0x1f,0x41,0x33}  (GAF stop, 50CB advance +1)
 *     info row 3 = {0x00,0x1f,0x42,0x10}  (50CB advance +2, ret 1)
 *     info row 5 = {0x00,0x01,0x40,0x00}  (50CB no-advance -> 2 BREAK)
 *     info row 6 = {0x00,0x1f,0x80,0x10}  (handler-boundary row)
 *     GDAT word@1 = 0x0d (table1d607e uc[0] & 8 clear -> cloud path)
 *   type  7 -> AI row 9 -> flags 0x0001 (static), BaseHP 24
 *     attribution: [0x05->1] [0xffff->0]; info rows all zero
 */

#include "dm2_v1_ccm_loop_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dm2_v1_creature.h"

static int g_failures = 0;

#define CHECK(cond, msg)                                              \
    do {                                                              \
        if (!(cond)) {                                                \
            fprintf(stderr, "FAIL: %s (line %d)\n", msg, __LINE__);   \
            ++g_failures;                                             \
        }                                                             \
    } while (0)

#define TYPE_DYN 12
#define TYPE_STA 7

/* raw layout inside data[]:
 *   raw 0 @0    16B attribution type 12
 *   raw 1 @16   32B info type 12
 *   raw 2 @48    8B attribution type 7
 *   raw 3 @56    8B info type 7
 */
#define RAW_COUNT 4
#define DATA_SIZE 64
#define ENTRY_COUNT 13

static void wr16(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v & 0xffu);
    p[1] = (uint8_t)((v >> 8) & 0xffu);
}

static uint16_t rd16(const uint8_t *p)
{
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static int16_t rec_handle(int index)
{
    return (int16_t)((4 << 10) | (index & 0x3ff));
}

static void set_word_entry(DM2_V1_GdatEntry *e, int category, int index,
                           int field, uint16_t value)
{
    memset(e, 0, sizeof(*e));
    e->cls1 = (uint8_t)category;
    e->cls2 = (uint8_t)index;
    e->cls3 = (uint8_t)DM2_GDAT_ENTRY_TYPE_WORD_VALUE;
    e->cls4 = (uint8_t)field;
    e->data_index = value;
}

static void set_raw_entry(DM2_V1_GdatEntry *e, int category, int index,
                          int type, int field, uint16_t raw_index)
{
    memset(e, 0, sizeof(*e));
    e->cls1 = (uint8_t)category;
    e->cls2 = (uint8_t)index;
    e->cls3 = (uint8_t)type;
    e->cls4 = (uint8_t)field;
    e->data_index = raw_index;
}

static void build_loader(DM2_V1_AssetLoader *loader,
                         DM2_V1_GdatEntry *entries,
                         uint32_t *raw_offsets, uint32_t *raw_sizes,
                         uint8_t *data)
{
    int n = 0;

    memset(data, 0, DATA_SIZE);

    /* type 12 attribution @0 */
    wr16(data + 0, 0x05); wr16(data + 2, 2);
    wr16(data + 4, 0x13); wr16(data + 6, 2);
    wr16(data + 8, 0x07); wr16(data + 10, 6);
    wr16(data + 12, 0xffff); wr16(data + 14, 0);
    /* type 12 info @16 */
    data[16 + 8] = 0x05; data[16 + 9] = 0x1f; data[16 + 10] = 0x41;
    data[16 + 11] = 0x33;                          /* row 2 */
    data[16 + 13] = 0x1f; data[16 + 14] = 0x42; data[16 + 15] = 0x10;
                                                    /* row 3 */
    data[16 + 21] = 0x01; data[16 + 22] = 0x40;    /* row 5 */
    data[16 + 25] = 0x1f; data[16 + 26] = 0x80; data[16 + 27] = 0x10;
                                                 /* row 6 */
    /* type 7 attribution @48 / info @56 (zeros) */
    wr16(data + 48, 0x05); wr16(data + 50, 1);
    wr16(data + 52, 0xffff); wr16(data + 54, 0);

    raw_offsets[0] = 0;  raw_sizes[0] = 16;
    raw_offsets[1] = 16; raw_sizes[1] = 32;
    raw_offsets[2] = 48; raw_sizes[2] = 8;
    raw_offsets[3] = 56; raw_sizes[3] = 8;

    set_word_entry(&entries[n++], DM2_GDAT_CATEGORY_CREATURES, TYPE_DYN,
                   0x05, 5);
    set_word_entry(&entries[n++], DM2_GDAT_CATEGORY_CREATURES, TYPE_STA,
                   0x05, 9);
    set_word_entry(&entries[n++], DM2_GDAT_CATEGORY_CREATURES, TYPE_DYN,
                   0x01, 0x0d);
    set_word_entry(&entries[n++], DM2_GDAT_CATEGORY_CREATURES, TYPE_STA,
                   0x01, 0x0d);
    set_word_entry(&entries[n++], DM2_GDAT_CATEGORY_CREATURE_AI, 5, 0, 0x00);
    set_word_entry(&entries[n++], DM2_GDAT_CATEGORY_CREATURE_AI, 5, 4, 40);
    set_word_entry(&entries[n++], DM2_GDAT_CATEGORY_CREATURE_AI, 5, 9, 0x05);
    set_word_entry(&entries[n++], DM2_GDAT_CATEGORY_CREATURE_AI, 9, 0, 0x01);
    set_word_entry(&entries[n++], DM2_GDAT_CATEGORY_CREATURE_AI, 9, 4, 24);
    set_raw_entry(&entries[n++], DM2_GDAT_CATEGORY_CREATURES, TYPE_DYN,
                  DM2_GDAT_ENTRY_TYPE_RAW8,
                  DM2_GDAT_CREATURE_ANIM_ATTRIBUTION, 0);
    set_raw_entry(&entries[n++], DM2_GDAT_CATEGORY_CREATURES, TYPE_DYN,
                  DM2_GDAT_ENTRY_TYPE_RAW7,
                  DM2_GDAT_CREATURE_ANIM_INFO_SEQUENCE, 1);
    set_raw_entry(&entries[n++], DM2_GDAT_CATEGORY_CREATURES, TYPE_STA,
                  DM2_GDAT_ENTRY_TYPE_RAW8,
                  DM2_GDAT_CREATURE_ANIM_ATTRIBUTION, 2);
    set_raw_entry(&entries[n++], DM2_GDAT_CATEGORY_CREATURES, TYPE_STA,
                  DM2_GDAT_ENTRY_TYPE_RAW7,
                  DM2_GDAT_CREATURE_ANIM_INFO_SEQUENCE, 3);

    memset(loader, 0, sizeof(*loader));
    loader->data = data;
    loader->data_size = DATA_SIZE;
    loader->loaded = 1;
    loader->raw_data_count = RAW_COUNT;
    loader->raw_offsets = raw_offsets;
    loader->raw_sizes = raw_sizes;
    loader->entries = entries;
    loader->entry_count = n;
}

static void build_pools(DM2_V1_RecordPoolSet *set)
{
    int i;

    memset(set, 0, sizeof(*set));
    set->pools[4].record_size = 16;
    set->pools[4].record_count = 2;
    set->pools[4].source_base = 0;
    set->pools[4].bytes = calloc(2, 16);
    for (i = 0; i < 2; i++) {
        wr16(set->pools[4].bytes + i * 16, DM2_V1_RECORD_HANDLE_END);
        set->pools[4].bytes[i * 16 + 5] = 0xFF;
    }
    /* rec 0: dynamic, owns slot 0, facing bits 2 (word@0xe = 0x0200) */
    set->pools[4].bytes[0 * 16 + 4] = TYPE_DYN;
    set->pools[4].bytes[0 * 16 + 5] = 0;
    wr16(set->pools[4].bytes + 0 * 16 + 0xe, 0x0200);
    /* rec 1: static, owns slot 1 */
    set->pools[4].bytes[1 * 16 + 4] = TYPE_STA;
    set->pools[4].bytes[1 * 16 + 5] = 1;
    set->valid = 1;
}

static void reset_slot(DM2_V1_CaiiArray *caii, int index)
{
    uint8_t *slot = caii->slots + (size_t)index * DM2_V1_CAII_SLOT_SIZE;
    memset(slot, 0, DM2_V1_CAII_SLOT_SIZE);
    wr16(slot + 2, 0xffffu);   /* no pending timer */
    slot[0x12] = 0xffu;        /* the bound 14cd_062e head */
    slot[0x17] = 0xffu;
    slot[0x1a] = 0xffu;
}

static DM2_V1_SourceTimer make_timer(uint8_t type, uint32_t ticks_and_map)
{
    DM2_V1_SourceTimer t;
    memset(&t, 0, sizeof(t));
    t.type = type;
    t.actor = 0;
    t.ticks_and_map = ticks_and_map;
    t.value_a = (int16_t)(5 | (7 << 8)); /* xA = 5, yA = 7 */
    return t;
}

static int g_mock_proceed_called;
static uint8_t g_mock_proceed_command;
static int32_t g_mock_proceed_result;
static int32_t mock_proceed_fn(void *ctx, uint8_t command,
    uint16_t ct, uint8_t *s, uint16_t a0, uint16_t a1,
    unsigned long gt) {
    (void)ctx; (void)ct; (void)s; (void)a0; (void)a1; (void)gt;
    g_mock_proceed_called = 1;
    g_mock_proceed_command = command;
    return g_mock_proceed_result;
}

int main(void)
{
    static uint8_t data[DATA_SIZE];
    DM2_V1_GdatEntry entries[ENTRY_COUNT];
    uint32_t raw_offsets[RAW_COUNT];
    uint32_t raw_sizes[RAW_COUNT];
    DM2_V1_AssetLoader loader;
    DM2_V1_RecordPoolSet set;
    DM2_V1_CaiiArray caii;
    DM2_V1_SourceTimerQueue queue;
    DM2_V1_DropRng rng;
    DM2_V1_CcmLoopReceipt rc;
    DM2_V1_SourceTimer timer;
    const uint8_t *anim;
    int16_t adj[2];
    int v1e0584;
    uint8_t *slot0;
    uint8_t *slot1;

    build_loader(&loader, entries, raw_offsets, raw_sizes, data);
    build_pools(&set);
    dm2_v1_caii_array_init(&caii, 2);
    CHECK(dm2_v1_creature_load_ai_table_from_gdat(&loader) == 2,
          "AI table loaded from the fixture");
    slot0 = caii.slots;
    slot1 = caii.slots + DM2_V1_CAII_SLOT_SIZE;

    /* ── (a) !flag full path: 4FCC + two 50CB steps + end requeue ── */
    dm2_v1_source_timer_queue_init(&queue);
    reset_slot(&caii, 0);
    slot0[0x1a] = 0x05;          /* the CCM command */
    slot0[0x21] = 1;             /* pending handler-result marker */
    rng.random = 0;
    adj[0] = 2; adj[1] = 0;      /* base 2, frame 0 */
    anim = NULL;
    v1e0584 = -1;
    timer = make_timer(0x21, (3u << 24) | 900u);
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_ccm_message_loop(&set, &caii, &queue, &loader, &rng,
                                  rec_handle(0), &timer, 7, adj, &anim,
                                  &v1e0584, 0, 3, 0, 0, 0, 0, 1000,
                                  NULL, NULL, NULL, NULL, NULL, NULL, &rc) == 1,
          "(a) loop completes");
    CHECK(rc.valid && rc.body_entered && !rc.payload_skip && !rc.type_0x22,
          "(a) body entered on the !flag branch");
    CHECK(rc.gaf_return == 1 && rc.loop_entered &&
              rc.walk_50cb_calls == 2 && rc.walk_50cb_last == 2 &&
              rc.loop_result == 2,
          "(a) 4FCC ret 1, 50CB walks 2 steps and BREAKs on 2");
    CHECK(adj[0] == 2 && adj[1] == 3,
          "(a) frame word advanced 0 -> 1 (4FCC) -> 3 (50CB +2)");
    CHECK(anim != NULL && anim[2] == 0x40,
          "(a) the stream rests on the no-advance row");
    CHECK(rc.requeue_end_completed && queue.count == 1 &&
              queue.timers[0].type == 0x22,
          "(a) end requeue issued the 0x22 timer (loop_result != 1)");
    CHECK(rd16(slot0 + 2) == queue.tickets[0] && rc.mticks_delta == 1000,
          "(a) ticket stored; delta is gametick + 0 for the zero row");

    /* ── (b) payload skip: adddata(4) + the m_15843 tail ────────── */
    dm2_v1_source_timer_queue_init(&queue);
    reset_slot(&caii, 0);
    slot0[0x1a] = 0x05;
    slot0[0x17] = 0x05;          /* no 0x13 anywhere */
    adj[0] = 2; adj[1] = 0;
    anim = NULL;
    v1e0584 = -1;
    timer = make_timer(0x21, (3u << 24) | 1000u);
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_ccm_message_loop(&set, &caii, &queue, &loader, &rng,
                                  rec_handle(0), &timer, 7, adj, &anim,
                                  &v1e0584, 0, 3, 1, 0, 0, 0, 1000,
                                  NULL, NULL, NULL, NULL, NULL, NULL, &rc) == 1,
          "(b) payload-skip completes");
    CHECK(rc.valid && rc.payload_skip && !rc.body_entered &&
              !rc.loop_entered && rc.gaf_return == -1,
          "(b) the body never ran");
    CHECK(timer.ticks_and_map == ((3u << 24) | 1004u),
          "(b) adddata(4) bumped the payload long");
    CHECK(rc.tail_enqueued && queue.count == 1 &&
              queue.timers[0].ticks_and_map == ((3u << 24) | 1004u) &&
              queue.timers[0].type == 0x21,
          "(b) the m_15843 tail re-queued the timer as-is");

    /* ── (c) 0x32..0x34 special: setmticks, no loop ─────────────── */
    dm2_v1_source_timer_queue_init(&queue);
    reset_slot(&caii, 0);
    slot0[0x17] = 0x33;
    adj[0] = 2; adj[1] = 0;
    anim = NULL;
    v1e0584 = -1;
    timer = make_timer(0x22, (3u << 24) | 1000u);
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_ccm_message_loop(&set, &caii, &queue, &loader, &rng,
                                  rec_handle(0), &timer, 7, adj, &anim,
                                  &v1e0584, 0, 9, 0, 0, 0, 0, 1000,
                                  NULL, NULL, NULL, NULL, NULL, NULL, &rc) == 1,
          "(c) special-mticks completes");
    CHECK(rc.valid && rc.special_mticks && rc.type_0x22 &&
              !rc.loop_entered && rc.gaf_return == -1,
          "(c) setmticks without the loop");
    CHECK(timer.ticks_and_map == ((9u << 24) | (uint32_t)(0x33 + 1000 - 50)),
          "(c) setmticks(v1e0571, b_1a + gametick - 50)");
    CHECK(rc.tail_enqueued && queue.count == 1 &&
              slot0[0x1a] == 0x33 && slot0[0x17] == 0xff,
          "(c) tail enqueued; b_1a taken from b_17, b_17 cleared");

    /* ── (d) dying branch: cloud receipt + 0x21 requeue ─────────── */
    dm2_v1_source_timer_queue_init(&queue);
    reset_slot(&caii, 0);
    slot0[0x17] = 0x13;
    rng.random = 0;
    adj[0] = 2; adj[1] = 0;
    anim = NULL;
    v1e0584 = -1;
    timer = make_timer(0x22, (3u << 24) | 1000u);
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_ccm_message_loop(&set, &caii, &queue, &loader, &rng,
                                  rec_handle(0), &timer, 7, adj, &anim,
                                  &v1e0584, 0, 3, 0, 0, 0, 0, 1000,
                                  NULL, NULL, NULL, NULL, NULL, NULL, &rc) == 1,
          "(d) dying branch completes");
    CHECK(rc.valid && rc.dying_branch && rc.cloud_would_create &&
              rc.cloud_id == 0x6e,
          "(d) cloud receipted with the aidef byte@0x23 == 0 selector");
    CHECK(rd16(slot0 + 0xe) == 2 && rd16(slot0 + 0x10) == 0,
          "(d) the adj pair landed in slot words 0xe/0x10");
    CHECK(rc.gaf_return == 1 && rc.loop_result == 1 &&
              rc.requeue_end_completed && queue.timers[0].type == 0x21,
          "(d) loop_result 1 re-arms the 0x21 think timer");

    /* ── (e) handler boundary: bound 01a3, receipted PROCEED_CCM ── */
    dm2_v1_source_timer_queue_init(&queue);
    reset_slot(&caii, 0);
    slot0[0x17] = 0x07;
    rng.random = 0;
    adj[0] = 2; adj[1] = 0;
    anim = NULL;
    v1e0584 = -1;
    timer = make_timer(0x22, (3u << 24) | 1000u);
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_ccm_message_loop(&set, &caii, &queue, &loader, &rng,
                                  rec_handle(0), &timer, 7, adj, &anim,
                                  &v1e0584, 0, 3, 0, 0, 0, 0, 1000,
                                  NULL, NULL, NULL, NULL, NULL, NULL, &rc) == 0,
          "(e) the handler boundary fails closed");
    CHECK(!rc.valid && rc.ccm_handler_unbound && rc.ccm_handler == 0x07 &&
              rc.ccm_handler_group == DM2_V1_CCM_SRC_CCM06,
          "(e) the dispatch matrix names the reached handler");
    CHECK(rc.init_01a3_runs == 1 && rc.v1e0584_set && v1e0584 == 0x0d,
          "(e) the bound 01a3 set v1e0584 lazily before the boundary");
    CHECK(rc.dir_written == 3 && slot0[0x1d] == 3,
          "(e) mode 7 wrote the facing ((2 + 1) & 3) to byte@0x1d");

    /* ── (f) b_17 == -1: the AI goal picker stays host-owned ────── */
    dm2_v1_source_timer_queue_init(&queue);
    reset_slot(&caii, 0);
    slot0[0x17] = 0xff;
    adj[0] = 2; adj[1] = 0;
    anim = NULL;
    v1e0584 = -1;
    timer = make_timer(0x22, (3u << 24) | 1000u);
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_ccm_message_loop(&set, &caii, &queue, &loader, &rng,
                                  rec_handle(0), &timer, 7, adj, &anim,
                                  &v1e0584, 0, 3, 0, 0, 0, 0, 1000,
                                  NULL, NULL, NULL, NULL, NULL, NULL, &rc) == 0 &&
              rc.ai_goal_unbound && slot0[0x1a] == 0xff,
          "(f) DM2_14cd_09e2 boundary after the b_1a write");

    /* ── (g) slot byte@0x12 != 0xff: the s_seven chain is unproven ─ */
    dm2_v1_source_timer_queue_init(&queue);
    reset_slot(&caii, 0);
    slot0[0x17] = 0x05;
    slot0[0x12] = 0x00;
    adj[0] = 2; adj[1] = 0;
    anim = NULL;
    v1e0584 = -1;
    timer = make_timer(0x22, (3u << 24) | 1000u);
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_ccm_message_loop(&set, &caii, &queue, &loader, &rng,
                                  rec_handle(0), &timer, 7, adj, &anim,
                                  &v1e0584, 0, 3, 0, 0, 0, 0, 1000,
                                  NULL, NULL, NULL, NULL, NULL, NULL, &rc) == 0 &&
              rc.s_seven_unbound && slot0[0x1a] == 0x05 &&
              slot0[0x17] == 0xff,
          "(g) 14cd_062e table path fails closed after b_17 clear");

    /* ── (h) static creature: the NULL animation row fails closed ── */
    dm2_v1_source_timer_queue_init(&queue);
    reset_slot(&caii, 1);
    slot1[0x17] = 0x05;
    adj[0] = 1; adj[1] = -1;
    anim = NULL;
    v1e0584 = -1;
    timer = make_timer(0x22, (3u << 24) | 1000u);
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_ccm_message_loop(&set, &caii, &queue, &loader, &rng,
                                  rec_handle(1), &timer, 7, adj, &anim,
                                  &v1e0584, 0, 3, 0, 0, 0, 0, 1000,
                                  NULL, NULL, NULL, NULL, NULL, NULL, &rc) == 0 &&
              rc.anim_row_null && rc.gaf_return == 1 &&
              rc.bitmap_state_reset,
          "(h) static GAF leaves the row NULL; table1d613a[5] & 4 receipted");

    /* ── (i) mode 6 facing write + full pass through the end ────── */
    dm2_v1_source_timer_queue_init(&queue);
    reset_slot(&caii, 0);
    slot0[0x17] = 0x06;
    rng.random = 0;
    adj[0] = 0; adj[1] = -1;
    anim = NULL;
    v1e0584 = -1;
    timer = make_timer(0x22, (3u << 24) | 1000u);
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_ccm_message_loop(&set, &caii, &queue, &loader, &rng,
                                  rec_handle(0), &timer, 7, adj, &anim,
                                  &v1e0584, 0, 3, 0, 0, 0, 0, 1000,
                                  NULL, NULL, NULL, NULL, NULL, NULL, &rc) == 1,
          "(i) mode 6 completes");
    CHECK(rc.valid && rc.dir_written == 1 && slot0[0x1d] == 1,
          "(i) mode 6 wrote the facing ((2 - 1) & 3) to byte@0x1d");
    CHECK(rc.gaf_return == 0 && rc.loop_result == 0 &&
              rc.walk_50cb_calls == 0 && queue.timers[0].type == 0x22,
          "(i) sequence-end GAF; loop_result 0 re-arms 0x22");

    /* ── (j) proceed_ccm callback dispatches the handler ────────── */
    dm2_v1_source_timer_queue_init(&queue);
    reset_slot(&caii, 0);
    slot0[0x1a] = 0x07;  /* any command; 4FCC uses adj_base not attribution */
    slot0[0x21] = 0;     /* no pending — rg1 stays 1, handler fires on 0x80 */
    rng.random = 0;
    adj[0] = 6; adj[1] = -1;  /* base 6 + frame -1 -> 4FCC starts fresh at 0 */
    anim = NULL;
    v1e0584 = -1;
    timer = make_timer(0x21, (3u << 24) | 900u);
    memset(&rc, 0, sizeof(rc));
    g_mock_proceed_called = 0;
    g_mock_proceed_command = 0;
    g_mock_proceed_result = 42;
    CHECK(dm2_v1_ccm_message_loop(&set, &caii, &queue, &loader, &rng,
                                  rec_handle(0), &timer, 7, adj, &anim,
                                  &v1e0584, 0, 3, 0, 0, 0, 0, 1000,
                                  NULL, NULL, NULL, NULL,
                                  mock_proceed_fn, NULL, &rc) == 1,
          "(j) callback dispatch completes");
    CHECK(g_mock_proceed_called && g_mock_proceed_command == 0x07,
          "(j) callback received command 0x07");
    CHECK(rc.ccm_handler_dispatched && rc.ccm_handler_result == 42,
          "(j) receipt records dispatch result 42");
    CHECK(!rc.ccm_handler_unbound,
          "(j) handler is NOT unbound when callback provided");

    dm2_v1_caii_array_free(&caii);
    free(set.pools[4].bytes);

    if (g_failures == 0) {
        puts("PASS: DM2_13e4_0982 CCM message loop bounded slice");
        return 0;
    }
    fprintf(stderr, "FAILURES: %d\n", g_failures);
    return 1;
}
