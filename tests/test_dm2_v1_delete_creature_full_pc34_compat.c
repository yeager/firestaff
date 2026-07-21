/*
 * test_dm2_v1_delete_creature_full_pc34_compat.c — the COMPLETE
 * DM2_DELETE_CREATURE_RECORD composition plus the DM2_INVOKE_MESSAGE
 * bounded slice.
 *
 * Source-lock anchors:
 *   skproject/SKULLWIN/c_record.cpp:1357-1425  DM2_DELETE_CREATURE_RECORD:
 *     GET_CREATURE_AT early return, jz_test8 AI gate, table1d607e &4
 *     probe, word@0xc decode + map swap + DM2_INVOKE_MESSAGE, CAII slot
 *     byte@1a clear, tile-rooted cut, DROP_CREATURE_POSSESSION,
 *     1c9a_0247 (receipted), DEALLOC_RECORD
 *   skproject/SKULLWIN/c_tim_proc.cpp:4332-4367 DM2_INVOKE_MESSAGE:
 *     setmticks/settype 0x4/actor RG3UW 0->1,1->3,2->2/setxyA/setxyB +
 *     DM2_QUEUE_TIMER
 *   skproject/SKULLWIN/c_timer.h:66,82,90     the field encodings
 *
 * The fixture reuses the synthetic dtWordValue GDAT session pattern
 * from the AI-spec/record-delete tests: type 12 -> AI row 5 -> flags
 * 0x0001 (bit0 set), type 7/9/10 -> AI row 9 -> flags 0x0200 (bit0
 * clear); CREATURES word@1: type 7 -> 0 (table1d607e uc0 0x00, invoke
 * runs), type 9 -> 7 (uc0 0x8c, &4 set, invoke skipped), type 10 ->
 * not loaded (fail-closed).
 */

#include "dm2_v1_delete_creature_full_pc34_compat.h"

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

#define RAW_SIZE 256
#define MAP_BASE 0
#define COLUMN_BASE 64
#define GROUND_BASE 128

#define TYPE_BIT0_SET 12     /* flags 0x0001 */
#define TYPE_INVOKE 7        /* bit0 clear, word@1 = 0 (uc0 &4 == 0) */
#define TYPE_NO_INVOKE 9     /* bit0 clear, word@1 = 7 (uc0 &4 set) */
#define TYPE_W1_UNKNOWN 10   /* bit0 clear, word@1 not loaded */

static int16_t mk_handle(int pool, int index)
{
    return (int16_t)((pool << 10) | (index & 0x3ff));
}

static void wr16(uint8_t *p, int16_t v)
{
    uint16_t u = (uint16_t)v;
    p[0] = (uint8_t)(u & 0xffu);
    p[1] = (uint8_t)((u >> 8) & 0xffu);
}

static int16_t rd16(const uint8_t *p)
{
    return (int16_t)(p[0] | ((uint16_t)p[1] << 8));
}

/* 2x2 byte-square map.  Flagged: (0,0) -> ground idx 0, (0,1) -> idx 1.
 * (1,1) carries no object flag. */
static void build_dungeon(DM2_V1_DungeonData *d, uint8_t *raw)
{
    memset(d, 0, sizeof(*d));
    memset(raw, 0, RAW_SIZE);
    d->level_count = 1;
    d->level_widths[0] = 2;
    d->level_heights[0] = 2;
    d->level_offsets[0] = 0;
    d->square_bytes = 1;
    d->raw_map_data_base = MAP_BASE;
    d->column_index_base = COLUMN_BASE;
    d->square_first_thing_base = GROUND_BASE;
    d->square_first_thing_count = 2;
    d->raw_data = raw;
    d->raw_size = RAW_SIZE;

    raw[MAP_BASE + 0] = 0x10; /* (0,0) */
    raw[MAP_BASE + 1] = 0x10; /* (0,1) */
    wr16(raw + COLUMN_BASE + 0, 0);
    wr16(raw + COLUMN_BASE + 2, 2);

    wr16(raw + GROUND_BASE + 0, DM2_V1_RECORD_HANDLE_END);
    wr16(raw + GROUND_BASE + 2, DM2_V1_RECORD_HANDLE_END);
}

/* DB4 creature records (16 bytes), link words set per case:
 *   rec0 type 7  (invoke), owns CAII slot 0, possession -> DB5 rec1,
 *        word@0xc = (0 << 10) | (1 << 5) | 1 = 0x21;
 *   rec1 type 9  (no invoke), no slot, no possession;
 *   rec2 type 12 (gate closed), owns CAII slot 0, no possession;
 *   rec3 type 10 (word@1 unknown), no slot, no possession.
 * DB5 weapons (4 bytes): rec0 ground item, rec1 possession item,
 * rec2/rec3 free for the generated-drops allocations. */
static void build_pools(DM2_V1_RecordPoolSet *set)
{
    memset(set, 0, sizeof(*set));

    set->pools[4].record_size = 16;
    set->pools[4].record_count = 4;
    set->pools[4].source_base = 0;
    set->pools[4].bytes = calloc(4, 16);
    for (int i = 0; i < 4; i++) {
        wr16(set->pools[4].bytes + i * 16 + 0, DM2_V1_RECORD_HANDLE_END);
        wr16(set->pools[4].bytes + i * 16 + 2, DM2_V1_RECORD_HANDLE_END);
        set->pools[4].bytes[i * 16 + 5] = 0xFF;
    }
    set->pools[4].bytes[0 * 16 + 4] = TYPE_INVOKE;
    set->pools[4].bytes[0 * 16 + 5] = 0; /* CAII slot 0 */
    wr16(set->pools[4].bytes + 0 * 16 + 2, mk_handle(5, 1));
    wr16(set->pools[4].bytes + 0 * 16 + 0xc, 0x21);
    set->pools[4].bytes[1 * 16 + 4] = TYPE_NO_INVOKE;
    set->pools[4].bytes[2 * 16 + 4] = TYPE_BIT0_SET;
    set->pools[4].bytes[2 * 16 + 5] = 0; /* CAII slot 0 */
    set->pools[4].bytes[3 * 16 + 4] = TYPE_W1_UNKNOWN;

    set->pools[5].record_size = 4;
    set->pools[5].record_count = 4;
    set->pools[5].source_base = 64;
    set->pools[5].bytes = calloc(4, 4);
    wr16(set->pools[5].bytes + 0, DM2_V1_RECORD_HANDLE_END);
    wr16(set->pools[5].bytes + 4, DM2_V1_RECORD_HANDLE_END);
    wr16(set->pools[5].bytes + 8, DM2_V1_RECORD_HANDLE_NULL);
    wr16(set->pools[5].bytes + 12, DM2_V1_RECORD_HANDLE_NULL);

    set->valid = 1;
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

static void build_ai_loader(DM2_V1_AssetLoader *loader,
                            DM2_V1_GdatEntry *entries,
                            uint32_t *raw_offsets,
                            uint32_t *raw_sizes,
                            uint8_t *raw_data)
{
    set_word_entry(&entries[0], DM2_GDAT_CATEGORY_CREATURES,
                   TYPE_BIT0_SET, 0x05, 5);
    set_word_entry(&entries[1], DM2_GDAT_CATEGORY_CREATURES,
                   TYPE_INVOKE, 0x05, 9);
    set_word_entry(&entries[2], DM2_GDAT_CATEGORY_CREATURE_AI, 5, 0, 0x01);
    set_word_entry(&entries[3], DM2_GDAT_CATEGORY_CREATURE_AI, 5, 4, 40);
    set_word_entry(&entries[4], DM2_GDAT_CATEGORY_CREATURE_AI, 9, 1, 0x02);
    set_word_entry(&entries[5], DM2_GDAT_CATEGORY_CREATURE_AI, 9, 4, 24);
    set_word_entry(&entries[6], DM2_GDAT_CATEGORY_CREATURES,
                   TYPE_INVOKE, 0x01, 0);
    set_word_entry(&entries[7], DM2_GDAT_CATEGORY_CREATURES,
                   TYPE_NO_INVOKE, 0x05, 9);
    set_word_entry(&entries[8], DM2_GDAT_CATEGORY_CREATURES,
                   TYPE_NO_INVOKE, 0x01, 7);
    set_word_entry(&entries[9], DM2_GDAT_CATEGORY_CREATURES,
                   TYPE_W1_UNKNOWN, 0x05, 9);

    memset(loader, 0, sizeof(*loader));
    loader->data = raw_data;
    loader->data_size = 1;
    loader->loaded = 1;
    loader->raw_data_count = 1;
    loader->raw_offsets = raw_offsets;
    loader->raw_sizes = raw_sizes;
    loader->entries = entries;
    loader->entry_count = 10;
}

/* Slot 0: (w & 0xf) = 1 -> base 2 items, no extra roll; itemspec
 * w >> 7 = 5 -> dbWeapon type 5.  All other slots empty. */
static const uint16_t k_drop_slots[DM2_DROP_SLOT_COUNT] = {
    (uint16_t)((5u << 7) | 1u), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static void test_invoke_message_unit(void)
{
    DM2_V1_SourceTimerQueue queue;
    DM2_V1_InvokeMessageReceipt rc;

    /* actor mapping + field decoding (c_tim_proc.cpp:4340-4360) */
    dm2_v1_source_timer_queue_init(&queue);
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_invoke_message(&queue, 5, 0x123, 0x1ff, 0x77, 0,
                                100, &rc) == 1 &&
              rc.valid == 1 && rc.type == 4 && rc.actor == 1 &&
              rc.value_a == (int16_t)((0xffu << 8) | 0x23u) &&
              rc.value_b == (int16_t)((0u << 8) | 0x77u) &&
              rc.ticks_and_map == ((5u << 24) | 100u) &&
              rc.ticket > 0u && queue.count == 1,
          "sel 0: actor 1, byte-masked setxyA/setxyB, setmticks word");

    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_invoke_message(&queue, 0, 1, 2, 3, 1, 7, &rc) == 1 &&
              rc.actor == 3 && rc.value_b == (int16_t)((1u << 8) | 3u),
          "sel 1: actor 3");
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_invoke_message(&queue, 0, 0, 0, 0, 2, 7, &rc) == 1 &&
              rc.actor == 2,
          "sel 2: actor 2");
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_invoke_message(&queue, 0, 0, 0, 0, 3, 7, &rc) == 1 &&
              rc.actor == 0,
          "sel 3: the c_tim init default actor 0");
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_invoke_message(&queue, 0, 0, 0, 0, 0xffff, 7, &rc) == 1 &&
              rc.actor == 0,
          "sel 0xffff: actor 0 (RG3UW mapping)");

    /* distinct stable tickets */
    {
        DM2_V1_InvokeMessageReceipt first;
        DM2_V1_InvokeMessageReceipt second;
        memset(&first, 0, sizeof(first));
        memset(&second, 0, sizeof(second));
        CHECK(dm2_v1_invoke_message(&queue, 0, 0, 0, 0, 0, 1,
                                    &first) == 1 &&
                  dm2_v1_invoke_message(&queue, 0, 0, 0, 0, 0, 2,
                                        &second) == 1 &&
                  first.ticket != second.ticket,
              "each queue issues a distinct stable ticket");
    }

    /* fail-closed paths */
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_invoke_message(NULL, 0, 0, 0, 0, 0, 1, &rc) == 0,
          "NULL queue fails closed");
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_invoke_message(&queue, -1, 0, 0, 0, 0, 1, &rc) == 0 &&
              dm2_v1_invoke_message(&queue, 0x100, 0, 0, 0, 0, 1,
                                    &rc) == 0,
          "out-of-range map byte fails closed");

    /* queue full: DM2_QUEUE_TIMER rejects, receipted queue_rejected */
    {
        DM2_V1_SourceTimerQueue full;
        DM2_V1_InvokeMessageReceipt fill;
        dm2_v1_source_timer_queue_init(&full);
        for (int i = 0; i < (int)DM2_V1_SOURCE_TIMER_MAX; i++) {
            memset(&fill, 0, sizeof(fill));
            if (dm2_v1_invoke_message(&full, 0, 0, 0, 0, 0, i + 1,
                                      &fill) != 1) {
                break;
            }
        }
        memset(&rc, 0, sizeof(rc));
        CHECK(dm2_v1_invoke_message(&full, 0, 0, 0, 0, 0, 999,
                                    &rc) == 0 &&
                  rc.queue_rejected == 1 && rc.ticket == 0u,
              "a full queue rejects fail-closed without a ticket");
    }

    CHECK(strstr(dm2_v1_invoke_message_source_evidence(),
                 "c_tim_proc.cpp:4332-4367") != NULL,
          "invoke evidence cites DM2_INVOKE_MESSAGE");
}

int main(void)
{
    static uint8_t raw[RAW_SIZE];
    DM2_V1_GdatEntry entries[10];
    uint32_t raw_offsets[1] = { 0 };
    uint32_t raw_sizes[1] = { 0 };
    uint8_t raw_data[1] = { 0 };
    DM2_V1_AssetLoader loader;
    DM2_V1_DungeonData dungeon;
    DM2_V1_RecordPoolSet set;
    DM2_V1_CaiiArray caii;
    DM2_V1_SourceTimerQueue queue;
    DM2_V1_DropRng rng;

    test_invoke_message_unit();

    build_ai_loader(&loader, entries, raw_offsets, raw_sizes, raw_data);
    CHECK(dm2_v1_creature_load_ai_table_from_gdat(&loader) == 4,
          "synthetic GDAT session resolves four creature AI rows");
    dm2_v1_caii_set_ai_spec_flags_fn(dm2_v1_creature_ai_spec_flags);
    dm2_v1_caii_set_gdat_word1_fn(dm2_v1_creature_gdat_word1);

    /* ── (A) full flow: invoke queued, cut, drop, dealloc ──────────── */
    build_dungeon(&dungeon, raw);
    build_pools(&set);
    wr16(raw + GROUND_BASE + 0, mk_handle(4, 0));
    wr16(set.pools[4].bytes + 0, mk_handle(5, 0)); /* rec0 -> ground item */
    dm2_v1_caii_array_init(&caii, 1);
    dm2_v1_source_timer_queue_init(&queue);
    dm2_v1_drops_rng_init(&rng);
    caii.slots[0x1a] = 0x13u;
    {
        DM2_V1_DeleteCreatureFullReceipt rc;
        memset(&rc, 0, sizeof(rc));
        CHECK(dm2_v1_delete_creature_record_full(
                  &set, &dungeon, &caii, &queue, &rng,
                  0, 1000ul, 0, 0, 0, 0, 0, 0, 1,
                  k_drop_slots, &rc) == 1 &&
                  rc.valid == 1 && rc.completed == 1 &&
                  rc.creature_not_found == 0,
              "full composition runs to the dealloc");
        CHECK(dm2_v1_record_handle_pool(rc.record_handle) == 4 &&
                  dm2_v1_record_handle_index(rc.record_handle) == 0 &&
                  rc.creature_type == TYPE_INVOKE,
              "the type-7 creature resolved at (0, 0)");
        CHECK(rc.ai_flags_known == 1 && rc.ai_bit0_clear == 1 &&
                  rc.gdat_w1_unknown == 0,
              "the jz_test8 gate opened data-backed");
        CHECK(rc.invoke_message_queued == 1 &&
                  rc.map_swap_receipted == 1 && rc.invoke_ticket > 0u &&
                  rc.invoke_queue_rejected == 0,
              "the map-swap/DM2_INVOKE_MESSAGE branch ran bound");
        CHECK(queue.count == 1 &&
                  queue.timers[0].type == 4u &&
                  queue.timers[0].actor == 1u &&
                  queue.timers[0].value_a == (int16_t)0x0101 &&
                  queue.timers[0].value_b == 0 &&
                  queue.timers[0].ticks_and_map == 1001u,
              "queued timer: type 4, actor 1, valueA (y<<8)|x, "
              "gametick+1 on map 0");
        CHECK(rc.slot_mode_cleared == 1 && caii.slots[0x1a] == 0,
              "owning CAII slot's mode byte cleared (c_record.cpp:1408-1413)");
        CHECK(rc.cut_performed == 1 && rc.cut_head_rewritten == 1 &&
                  rc.cut_side_effects_unbound == 1,
              "tile-rooted cut bound, 3CE7D side effects receipted");
        CHECK(rc.drop_ran == 1 && rc.drop_failed == 0 &&
                  rc.drop.drops_placed == 2 &&
                  rc.drop.possession_dropped == 1 &&
                  rc.drop.dir_draws == 1,
              "bound DROP_CREATURE_POSSESSION ran (2 generated + 1 "
              "possession item)");
        CHECK(rc.dballoc_cleanup_unbound == 1,
              "1c9a_0247 receipted host-owned");
        CHECK(rc.dealloc_performed == 1 &&
                  rd16(set.pools[4].bytes + 0) ==
                      DM2_V1_RECORD_HANDLE_NULL,
              "dealloc wrote the 0xffff free marker");
        /* chain: DB5 rec0 -> gen1(rec2) -> gen2(rec3) -> possession rec1 */
        CHECK(dm2_v1_record_handle_index(rd16(set.pools[5].bytes + 0)) == 2 &&
                  dm2_v1_record_handle_index(rd16(set.pools[5].bytes + 8)) == 3 &&
                  dm2_v1_record_handle_index(rd16(set.pools[5].bytes + 12)) == 1 &&
                  rd16(set.pools[5].bytes + 4) == DM2_V1_RECORD_HANDLE_END,
              "ground chain: ground item -> generated -> possession -> END");
        CHECK(dm2_v1_dungeon_get_first_thing(&dungeon, 0, 0, 0) ==
                  mk_handle(5, 0),
              "ground-stack head rewritten to the ground item");
    }
    dm2_v1_caii_array_free(&caii);
    dm2_v1_record_pool_set_free(&set);

    /* ── (B) table1d607e &4 set: the invoke branch is skipped ───────── */
    build_dungeon(&dungeon, raw);
    build_pools(&set);
    wr16(raw + GROUND_BASE + 0, mk_handle(4, 1));
    dm2_v1_source_timer_queue_init(&queue);
    dm2_v1_drops_rng_init(&rng);
    {
        DM2_V1_DeleteCreatureFullReceipt rc;
        memset(&rc, 0, sizeof(rc));
        CHECK(dm2_v1_delete_creature_record_full(
                  &set, &dungeon, NULL, &queue, &rng,
                  0, 2000ul, 0, 0, 0, -1, 5, 5, 0,
                  NULL, &rc) == 1 &&
                  rc.ai_bit0_clear == 1 &&
                  rc.invoke_message_queued == 0 &&
                  rc.map_swap_receipted == 0 &&
                  queue.count == 0,
              "&4 probe set: no message queued");
        CHECK(rc.slot_mode_cleared == 0 &&
                  rc.slot_mode_clear_unbound == 0,
              "byte@5 == 0xff skips the slot-mode clear");
        CHECK(rc.cut_performed == 1 && rc.drop_ran == 1 &&
                  rc.drop.possession_walk_ran == 0 &&
                  rc.dealloc_performed == 1,
              "cut + drop (empty possession) + dealloc still bound");
    }
    dm2_v1_record_pool_set_free(&set);

    /* ── (C) bit0 set: the whole gate block is skipped ─────────────── */
    build_dungeon(&dungeon, raw);
    build_pools(&set);
    wr16(raw + GROUND_BASE + 0, mk_handle(4, 2));
    wr16(set.pools[4].bytes + 2 * 16, mk_handle(5, 0));
    dm2_v1_caii_array_init(&caii, 1);
    dm2_v1_source_timer_queue_init(&queue);
    dm2_v1_drops_rng_init(&rng);
    caii.slots[0x1a] = 0x13u;
    {
        DM2_V1_DeleteCreatureFullReceipt rc;
        memset(&rc, 0, sizeof(rc));
        CHECK(dm2_v1_delete_creature_record_full(
                  &set, &dungeon, &caii, &queue, &rng,
                  0, 3000ul, 0, 0, 2, 0, 0, 0, 0,
                  NULL, &rc) == 1 &&
                  rc.ai_flags_known == 1 && rc.ai_bit0_clear == 0 &&
                  rc.invoke_message_queued == 0 &&
                  rc.slot_mode_cleared == 0 &&
                  caii.slots[0x1a] == 0x13u,
              "bit0-set flags skip the gate block (mode 2 drop return)");
        CHECK(rc.cut_performed == 1 && rc.drop_ran == 1 &&
                  rc.drop.mode_return == 1 &&
                  rc.dealloc_performed == 1,
              "mode 2: the drop's source immediate return, then dealloc");
    }
    dm2_v1_caii_array_free(&caii);
    dm2_v1_record_pool_set_free(&set);

    /* ── (D) gate open but word@1 unknown: fail-closed, no mutation ── */
    build_dungeon(&dungeon, raw);
    build_pools(&set);
    wr16(raw + GROUND_BASE + 0, mk_handle(4, 3));
    dm2_v1_source_timer_queue_init(&queue);
    dm2_v1_drops_rng_init(&rng);
    {
        DM2_V1_DeleteCreatureFullReceipt rc;
        memset(&rc, 0, sizeof(rc));
        CHECK(dm2_v1_delete_creature_record_full(
                  &set, &dungeon, NULL, &queue, &rng,
                  0, 4000ul, 0, 0, 0, 0, 0, 0, 0,
                  NULL, &rc) == 0 &&
                  rc.gdat_w1_unknown == 1 &&
                  rc.invoke_message_queued == 0 &&
                  rc.cut_performed == 0 &&
                  rc.dealloc_performed == 0,
              "unknown GDAT word@1 fails closed before any mutation");
        CHECK(queue.count == 0 &&
                  rd16(set.pools[4].bytes + 3 * 16) ==
                      DM2_V1_RECORD_HANDLE_END &&
                  dm2_v1_dungeon_get_first_thing(&dungeon, 0, 0, 0) ==
                      mk_handle(4, 3),
              "queue, record and tile chain untouched");
    }
    dm2_v1_record_pool_set_free(&set);

    /* ── (E) no creature at the cell: source early return ───────────── */
    build_dungeon(&dungeon, raw);
    build_pools(&set);
    dm2_v1_source_timer_queue_init(&queue);
    {
        DM2_V1_DeleteCreatureFullReceipt rc;
        memset(&rc, 0, sizeof(rc));
        CHECK(dm2_v1_delete_creature_record_full(
                  &set, &dungeon, NULL, &queue, NULL,
                  0, 5000ul, 1, 1, 0, 0, 0, 0, 0,
                  NULL, &rc) == 1 &&
                  rc.creature_not_found == 1 && rc.completed == 1 &&
                  rc.cut_performed == 0,
              "creature_not_found takes the source early return");
    }
    dm2_v1_record_pool_set_free(&set);

    /* ── (F) fail-closed drop: the dealloc is skipped ───────────────── */
    build_dungeon(&dungeon, raw);
    build_pools(&set);
    wr16(raw + GROUND_BASE + 0, mk_handle(4, 0));
    wr16(set.pools[4].bytes + 0, mk_handle(5, 0));
    dm2_v1_caii_array_init(&caii, 1);
    dm2_v1_source_timer_queue_init(&queue);
    {
        DM2_V1_DeleteCreatureFullReceipt rc;
        memset(&rc, 0, sizeof(rc));
        CHECK(dm2_v1_delete_creature_record_full(
                  &set, &dungeon, &caii, &queue, NULL,
                  0, 6000ul, 0, 0, 0, 0, 0, 0, 1,
                  k_drop_slots, &rc) == 0 &&
                  rc.drop_failed == 1 && rc.drop.rng_unbound == 1 &&
                  rc.dealloc_performed == 0,
              "unbound RNG fails the drop closed; dealloc skipped");
        CHECK(rd16(set.pools[4].bytes + 0) != DM2_V1_RECORD_HANDLE_NULL,
              "no free marker on the fail-closed path");
        CHECK(rc.cut_performed == 1 && rc.invoke_message_queued == 1,
              "source-ordered effects before the drop still applied");
    }
    dm2_v1_caii_array_free(&caii);
    dm2_v1_record_pool_set_free(&set);

    /* ── (G) invalid input ──────────────────────────────────────────── */
    build_dungeon(&dungeon, raw);
    build_pools(&set);
    dm2_v1_source_timer_queue_init(&queue);
    {
        DM2_V1_DeleteCreatureFullReceipt rc;
        memset(&rc, 0, sizeof(rc));
        CHECK(dm2_v1_delete_creature_record_full(
                  NULL, &dungeon, NULL, &queue, NULL,
                  0, 0ul, 0, 0, 0, 0, 0, 0, 0, NULL, &rc) == 0 &&
              dm2_v1_delete_creature_record_full(
                  &set, NULL, NULL, &queue, NULL,
                  0, 0ul, 0, 0, 0, 0, 0, 0, 0, NULL, &rc) == 0 &&
              dm2_v1_delete_creature_record_full(
                  &set, &dungeon, NULL, NULL, NULL,
                  0, 0ul, 0, 0, 0, 0, 0, 0, 0, NULL, &rc) == 0,
              "NULL pool/dungeon/queue rejected fail-closed");
    }
    dm2_v1_record_pool_set_free(&set);

    CHECK(strstr(dm2_v1_delete_creature_full_source_evidence(),
                 "c_record.cpp:1357-1425") != NULL,
          "composition evidence cites DM2_DELETE_CREATURE_RECORD");

    dm2_v1_creature_reset_ai_table();

    if (g_failures != 0) {
        fprintf(stderr,
                "dm2_v1_delete_creature_full_pc34_compat: %d failure(s)\n",
                g_failures);
        return 1;
    }
    printf("dm2_v1_delete_creature_full_pc34_compat: all checks passed\n");
    return 0;
}
