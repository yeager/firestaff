/*
 * test_dm2_v1_caii_record_delete_pc34_compat.c — DM2_1c9a_0fcb
 * DM2_DELETE_CREATURE_RECORD branch head, taken data-backed.
 *
 * Source-lock anchors:
 *   skproject/SKULLWIN/c_1c9a.cpp:5936-5957  branch head: flag set AND a
 *     pending timer -> payload read BEFORE DM2_1c9a_0db0; no pending
 *     timer forces RG3L = 0; DELETE_CREATURE_RECORD runs after the slot
 *     is marked free
 *   skproject/SKULLWIN/c_timer.h:80-82       getxA/getyA = valueA lo/hi
 *     bytes (the branch's (x, y) arguments)
 *   skproject/SKULLWIN/c_record.cpp:1357-1425 DM2_DELETE_CREATURE_RECORD
 *     decision head: GET_CREATURE_AT early return, jz_test8 AI gate,
 *     CAII slot byte@1a clear; mutating tail receipted unbound
 *
 * The fixture reuses the synthetic dtWordValue GDAT session from the
 * AI-spec gate test: type 12 -> row 5 -> 0x0001 (bit0 set), type 7 ->
 * row 9 -> 0x0200 (bit0 clear), type 3 -> no row.
 */

#include "dm2_v1_caii_alloc_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dm2_v1_creature.h"
#include "dm2_v1_proceed_timers_pc34_compat.h"

static int g_failures = 0;

#define CHECK(cond, msg)                                              \
    do {                                                              \
        if (!(cond)) {                                                \
            fprintf(stderr, "FAIL: %s (line %d)\n", msg, __LINE__);   \
            ++g_failures;                                             \
        }                                                             \
    } while (0)

#define MAP_BASE 0
#define COLUMN_BASE 64
#define GROUND_BASE 128
#define RAW_SIZE 160

#define TYPE_BIT0_SET 12   /* flags 0x0001 */
#define TYPE_BIT0_CLEAR 7  /* flags 0x0200 */

static void wr16(uint8_t *p, int16_t v)
{
    uint16_t u = (uint16_t)v;
    p[0] = (uint8_t)(u & 0xffu);
    p[1] = (uint8_t)((u >> 8) & 0xffu);
}

static uint16_t rd16(const uint8_t *p)
{
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static void build_dungeon(DM2_V1_DungeonData *d, uint8_t *raw)
{
    memset(d, 0, sizeof(*d));
    memset(raw, 0, RAW_SIZE);
    d->level_count = 1;
    d->level_widths[0] = 2;
    d->level_heights[0] = 3;
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

    wr16(raw + GROUND_BASE + 0,
         (int16_t)((2u << 14) | (4u << 10) | 0u));
    wr16(raw + GROUND_BASE + 2,
         (int16_t)((1u << 14) | (4u << 10) | 1u));
}

static void build_pools(DM2_V1_RecordPoolSet *set)
{
    static const uint8_t types[2] = { TYPE_BIT0_SET, TYPE_BIT0_CLEAR };
    int i;

    memset(set, 0, sizeof(*set));

    set->pools[4].record_size = 16;
    set->pools[4].record_count = 2;
    set->pools[4].source_base = 0;
    set->pools[4].bytes = calloc(2, 16);
    for (i = 0; i < 2; i++) {
        wr16(set->pools[4].bytes + i * 16, DM2_V1_RECORD_HANDLE_END);
        set->pools[4].bytes[i * 16 + 4] = types[i];
        set->pools[4].bytes[i * 16 + 5] = 0xFF;
        wr16(set->pools[4].bytes + i * 16 + 8, (int16_t)0xffff);
    }

    set->valid = 1;
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

static void build_ai_loader(DM2_V1_AssetLoader *loader,
                            DM2_V1_GdatEntry *entries,
                            uint32_t *raw_offsets,
                            uint32_t *raw_sizes,
                            uint8_t *raw_data)
{
    set_word_entry(&entries[0], DM2_GDAT_CATEGORY_CREATURES,
                   TYPE_BIT0_SET, 0x05, 5);
    set_word_entry(&entries[1], DM2_GDAT_CATEGORY_CREATURES,
                   TYPE_BIT0_CLEAR, 0x05, 9);
    set_word_entry(&entries[2], DM2_GDAT_CATEGORY_CREATURE_AI, 5, 0, 0x01);
    set_word_entry(&entries[3], DM2_GDAT_CATEGORY_CREATURE_AI, 5, 4, 40);
    set_word_entry(&entries[4], DM2_GDAT_CATEGORY_CREATURE_AI, 9, 1, 0x02);
    set_word_entry(&entries[5], DM2_GDAT_CATEGORY_CREATURE_AI, 9, 4, 24);

    memset(loader, 0, sizeof(*loader));
    loader->data = raw_data;
    loader->data_size = 1;
    loader->loaded = 1;
    loader->raw_data_count = 1;
    loader->raw_offsets = raw_offsets;
    loader->raw_sizes = raw_sizes;
    loader->entries = entries;
    loader->entry_count = 6;
}

int main(void)
{
    static uint8_t raw[RAW_SIZE];
    DM2_V1_GdatEntry entries[6];
    uint32_t raw_offsets[1] = { 0 };
    uint32_t raw_sizes[1] = { 0 };
    uint8_t raw_data[1] = { 0 };
    DM2_V1_AssetLoader loader;
    DM2_V1_DungeonData dungeon;
    DM2_V1_RecordPoolSet set;
    DM2_V1_CaiiArray caii;
    DM2_V1_SourceTimerQueue queue;
    DM2_V1_CaiiAllocReceipt alloc;
    DM2_V1_CaiiFreeReceipt rc;
    DM2_V1_CaiiDeleteCreatureRecordReceipt head;
    DM2_V1_SourceTimer pending;
    uint8_t *slot;

    build_dungeon(&dungeon, raw);
    build_pools(&set);
    build_ai_loader(&loader, entries, raw_offsets, raw_sizes, raw_data);
    dm2_v1_caii_array_init(&caii, 3);
    dm2_v1_source_timer_queue_init(&queue);

    /* ── (a) no provider wired: the branch can never be taken ──────── */
    dm2_v1_creature_reset_ai_table();
    memset(&alloc, 0, sizeof(alloc));
    CHECK(dm2_v1_caii_alloc_to_creature(&set, &dungeon, &caii, &queue,
                                        0, 100ul, rec_handle(1),
                                        0, 1, &alloc) == 1,
          "type-7 creature activates without a provider");
    slot = caii.slots + (size_t)alloc.slot_index * DM2_V1_CAII_SLOT_SIZE;
    slot[0x1a] = 0x13u;
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_caii_free_slot(&set, &dungeon, &caii, &queue,
                                alloc.slot_index, &rc) == 1 &&
              rc.record_delete_flag == -1 &&
              rc.record_delete_branch == 0 &&
              rc.record_delete_head_resolved == 0,
          "unknown flags keep the branch closed");

    /* ── (b) GDAT session + provider wired ──────────────────────────── */
    CHECK(dm2_v1_creature_load_ai_table_from_gdat(&loader) == 2,
          "synthetic GDAT session loads two AI rows");
    dm2_v1_caii_set_ai_spec_flags_fn(dm2_v1_creature_ai_spec_flags);

    /* ── (c) ticket peek accessor (c_1c9a.cpp:5943-5944 slot read) ─── */
    memset(&alloc, 0, sizeof(alloc));
    CHECK(dm2_v1_caii_alloc_to_creature(&set, &dungeon, &caii, &queue,
                                        0, 200ul, rec_handle(1),
                                        0, 1, &alloc) == 1,
          "type-7 creature reactivates in the freed slot");
    slot = caii.slots + (size_t)alloc.slot_index * DM2_V1_CAII_SLOT_SIZE;
    {
        uint32_t ticket = (uint32_t)rd16(slot + 2);
        memset(&pending, 0, sizeof(pending));
        CHECK(ticket != 0u && ticket != 0xffffu &&
                  dm2_v1_source_timer_peek_ticket(&queue, ticket,
                                                  &pending) == 1 &&
                  ((uint16_t)pending.value_a & 0xffu) == 0u &&
                  (((uint16_t)pending.value_a >> 8) & 0xffu) == 1u,
              "peek returns the pending timer with packed x|y payload");
        CHECK(dm2_v1_source_timer_peek_ticket(&queue, ticket + 99u,
                                              &pending) == 0 &&
                  dm2_v1_source_timer_peek_ticket(&queue, 0u,
                                                  &pending) == 0 &&
                  dm2_v1_source_timer_peek_ticket(NULL, ticket,
                                                  &pending) == 0 &&
                  dm2_v1_source_timer_peek_ticket(&queue, ticket,
                                                  NULL) == 0,
              "peek fails closed for unknown/zero tickets and NULLs");
    }

    /* ── (d) branch taken end-to-end (c_1c9a.cpp:5936-5957) ────────── */
    slot[0x1a] = 0x13u;
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_caii_free_slot(&set, &dungeon, &caii, &queue,
                                alloc.slot_index, &rc) == 1 &&
              rc.record_delete_flag == 1 &&
              rc.record_delete_branch == 1 &&
              rc.record_delete_no_timer == 0 &&
              rc.record_delete_x == 0 && rc.record_delete_y == 1 &&
              rc.record_delete_head_resolved == 1 &&
              rc.record_delete_unbound == 1,
          "flag + pending timer takes the branch with payload coords");
    CHECK(queue.count == 0,
          "pending timer consumed through the bound 0db0 path");

    /* ── (e) flag set but timer pre-deleted: RG3L = 0 path ─────────── */
    memset(&alloc, 0, sizeof(alloc));
    CHECK(dm2_v1_caii_alloc_to_creature(&set, &dungeon, &caii, &queue,
                                        0, 300ul, rec_handle(1),
                                        0, 1, &alloc) == 1,
          "type-7 creature activates for the no-timer path");
    slot = caii.slots + (size_t)alloc.slot_index * DM2_V1_CAII_SLOT_SIZE;
    slot[0x1a] = 0x13u;
    {
        DM2_V1_CaiiDeleteTimerReceipt del;
        memset(&del, 0, sizeof(del));
        CHECK(dm2_v1_caii_delete_timer(&set, &caii, &queue,
                                       rec_handle(1), &del) == 1,
              "pending timer deleted before the free");
    }
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_caii_free_slot(&set, &dungeon, &caii, &queue,
                                alloc.slot_index, &rc) == 1 &&
              rc.record_delete_flag == 1 &&
              rc.record_delete_branch == 0 &&
              rc.record_delete_no_timer == 1 &&
              rc.record_delete_head_resolved == 0,
          "no pending timer forces the source's RG3L = 0 outcome");

    /* ── (f) bit0-set flags: branch closed even with a timer ────────── */
    memset(&alloc, 0, sizeof(alloc));
    CHECK(dm2_v1_caii_alloc_to_creature(&set, &dungeon, &caii, &queue,
                                        0, 400ul, rec_handle(0),
                                        0, 0, &alloc) == 1,
          "type-12 creature activates");
    slot = caii.slots + (size_t)alloc.slot_index * DM2_V1_CAII_SLOT_SIZE;
    slot[0x1a] = 0x13u;
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_caii_free_slot(&set, &dungeon, &caii, &queue,
                                alloc.slot_index, &rc) == 1 &&
              rc.record_delete_flag == 0 &&
              rc.record_delete_branch == 0 &&
              rc.record_delete_no_timer == 0,
          "bit0-set flags keep the branch closed with a live timer");

    /* ── (g) decision head, direct callers (c_record.cpp:1357-1425) ── */
    memset(&head, 0, sizeof(head));
    CHECK(dm2_v1_caii_delete_creature_record_head(&set, &dungeon, &caii,
                                                  1, 1, &head) == 0 &&
              head.creature_not_found == 1,
          "no creature at the cell takes the source early return");
    CHECK(dm2_v1_caii_delete_creature_record_head(&set, NULL, &caii,
                                                  0, 0, &head) == 0 &&
              dm2_v1_caii_delete_creature_record_head(NULL, &dungeon,
                                                      &caii, 0, 0,
                                                      &head) == 0,
          "NULL dungeon/pool rejected fail-closed");

    /* type-12 record (bit0 set): gate not taken, tails still unbound */
    memset(&head, 0, sizeof(head));
    CHECK(dm2_v1_caii_delete_creature_record_head(&set, &dungeon, &caii,
                                                  0, 0, &head) == 1 &&
              head.resolved == 1 &&
              dm2_v1_record_handle_pool(head.record_handle) == 4 &&
              dm2_v1_record_handle_index(head.record_handle) == 0 &&
              head.creature_type == TYPE_BIT0_SET &&
              head.ai_flags_known == 1 &&
              head.ai_bit0_clear == 0 &&
              head.invoke_message_unbound == 0 &&
              head.slot_mode_cleared == 0 &&
              head.move_record_unbound == 1 &&
              head.drop_possession_unbound == 1 &&
              head.dballoc_cleanup_unbound == 1 &&
              head.dealloc_record_unbound == 1,
          "bit0-set head: gate closed, mutating tail receipted unbound");

    /* activated type-7 creature (bit0 clear, owns a CAII slot): the
     * slot mode byte clear is BOUND (c_record.cpp:1408-1413) */
    memset(&alloc, 0, sizeof(alloc));
    CHECK(dm2_v1_caii_alloc_to_creature(&set, &dungeon, &caii, &queue,
                                        0, 500ul, rec_handle(1),
                                        0, 1, &alloc) == 1,
          "type-7 creature activates for the direct head call");
    slot = caii.slots + (size_t)alloc.slot_index * DM2_V1_CAII_SLOT_SIZE;
    slot[0x1a] = 0x13u;
    memset(&head, 0, sizeof(head));
    CHECK(dm2_v1_caii_delete_creature_record_head(&set, &dungeon, &caii,
                                                  0, 1, &head) == 1 &&
              head.ai_bit0_clear == 1 &&
              head.invoke_message_unbound == 1 &&
              head.slot_mode_cleared == 1 &&
              slot[0x1a] == 0,
          "bit0-clear head clears the owning slot's mode byte");
    memset(&head, 0, sizeof(head));
    slot[0x1a] = 0x13u;
    CHECK(dm2_v1_caii_delete_creature_record_head(&set, &dungeon, NULL,
                                                  0, 1, &head) == 1 &&
              head.ai_bit0_clear == 1 &&
              head.slot_mode_cleared == 0 &&
              slot[0x1a] == 0x13u,
          "NULL CAII array skips the mode-byte clear fail-closed");
    CHECK(strstr(head.source_evidence, "c_record.cpp:1357-1425") != NULL,
          "head evidence cites DM2_DELETE_CREATURE_RECORD");

    dm2_v1_caii_array_free(&caii);
    dm2_v1_record_pool_set_free(&set);
    dm2_v1_creature_reset_ai_table();

    if (g_failures != 0) {
        fprintf(stderr,
                "dm2_v1_caii_record_delete_pc34_compat: %d failure(s)\n",
                g_failures);
        return 1;
    }
    printf("dm2_v1_caii_record_delete_pc34_compat: all checks passed\n");
    return 0;
}
