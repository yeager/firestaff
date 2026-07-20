/*
 * test_dm2_v1_caii_ai_spec_pc34_compat.c — data-backed AI-spec flag gate
 * for the CAII module.
 *
 * Source-lock anchors:
 *   skproject/SKULLWIN/c_record.cpp:1346-1349 DM2_QUERY_CREATURE_AI_SPEC_FLAGS
 *   skproject/SKULLWIN/c_record.cpp:1351-1354 DM2_QUERY_CREATURE_AI_SPEC_FROM_RECORD
 *     (CREATURES[type & 0xff] word@5 -> table1d296c AIDefinition row -> word@0)
 *   skproject/SKULLWIN/c_1c9a.cpp:5917-5929 DM2_1c9a_0fcb record-delete flag
 *     ((flags & 1) == 0 && slot byte@1a == 0x13)
 *   skproject/SKULLWIN/c_creature.cpp:370-385 DM2_ATTACK_CREATURE vl_18 gate
 *     (AIDefinition word@0 & 1; alloc only when set, byte@5 == 0xff)
 *
 * The fixture drives the PROVEN GDAT extended-mode path
 * (EXTENDED_LOAD_AI_DEFINITION, SkWinCore.cpp:233-400) with synthetic
 * dtWordValue entries, so no game data is required:
 *   creature type 12 -> AI row 5 -> w0AIFlags = 0x0001 (bit0 set)
 *   creature type  7 -> AI row 9 -> w0AIFlags = 0x0200 (bit0 clear)
 *   creature type  3 -> no AI row (unknown provenance)
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

#define TYPE_GATED_ALLOC 12 /* flags bit0 set: ATTACK_CREATURE allocs */
#define TYPE_GATED_SKIP 7   /* flags bit0 clear: source returns early */
#define TYPE_UNKNOWN 3      /* no AI row in the session */

static void wr16(uint8_t *p, int16_t v)
{
    uint16_t u = (uint16_t)v;
    p[0] = (uint8_t)(u & 0xffu);
    p[1] = (uint8_t)((u >> 8) & 0xffu);
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
    static const uint8_t types[3] = { TYPE_GATED_ALLOC, TYPE_GATED_SKIP,
                                      TYPE_UNKNOWN };
    int i;

    memset(set, 0, sizeof(*set));

    set->pools[4].record_size = 16;
    set->pools[4].record_count = 3;
    set->pools[4].source_base = 0;
    set->pools[4].bytes = calloc(3, 16);
    for (i = 0; i < 3; i++) {
        wr16(set->pools[4].bytes + i * 16, DM2_V1_RECORD_HANDLE_END);
        set->pools[4].bytes[i * 16 + 4] = types[i]; /* record byte@4 */
        set->pools[4].bytes[i * 16 + 5] = 0xFF;     /* record byte@5 */
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
    /* CREATURES word@5 indirection (c_record.cpp:1351-1354). */
    set_word_entry(&entries[0], DM2_GDAT_CATEGORY_CREATURES,
                   TYPE_GATED_ALLOC, 0x05, 5);
    set_word_entry(&entries[1], DM2_GDAT_CATEGORY_CREATURES,
                   TYPE_GATED_SKIP, 0x05, 9);
    /* AI row 5: w0AIFlags = 0x0001, BaseHP low byte 40 (admission
     * probe, loader field 4).  Absent fields decode as zero. */
    set_word_entry(&entries[2], DM2_GDAT_CATEGORY_CREATURE_AI, 5, 0, 0x01);
    set_word_entry(&entries[3], DM2_GDAT_CATEGORY_CREATURE_AI, 5, 4, 40);
    /* AI row 9: w0AIFlags = 0x0200 (bit0 clear), BaseHP low byte 24. */
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
    uint16_t flags = 0;
    uint8_t *slot0;
    uint8_t *slot1;

    build_dungeon(&dungeon, raw);
    build_pools(&set);
    build_ai_loader(&loader, entries, raw_offsets, raw_sizes, raw_data);
    dm2_v1_caii_array_init(&caii, 3);
    dm2_v1_source_timer_queue_init(&queue);

    /* ── (a) fail-closed before any GDAT session loads the table ────── */
    dm2_v1_creature_reset_ai_table();
    CHECK(dm2_v1_creature_ai_spec_flags(TYPE_GATED_ALLOC, &flags) == 0 &&
              flags == 0,
          "accessor fails closed with an unloaded AI table");
    CHECK(dm2_v1_creature_ai_spec_flags(TYPE_GATED_ALLOC, NULL) == 0,
          "NULL out-param rejected");
    CHECK(dm2_v1_caii_attack_guard_allows_alloc(&set, rec_handle(0)) == -1,
          "attack guard reports unknown provenance before GDAT import");

    memset(&alloc, 0, sizeof(alloc));
    CHECK(dm2_v1_caii_alloc_to_creature(&set, &dungeon, &caii, &queue,
                                        0, 100ul, rec_handle(0),
                                        0, 0, &alloc) == 1,
          "first creature activates without the AI table");
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_caii_free_slot(&set, &caii, &queue, 0, &rc) == 1 &&
              rc.record_delete_flag == -1 &&
              rc.record_delete_unbound == 1,
          "0fcb record-delete flag is -1 without loaded AI flags");

    /* ── (b) GDAT import binds the source's word@5 indirection ──────── */
    CHECK(dm2_v1_creature_load_ai_table_from_gdat(&loader) == 2,
          "synthetic GDAT session loads two AI rows");
    /* The session that owns the AI table wires the proven provider. */
    dm2_v1_caii_set_ai_spec_flags_fn(dm2_v1_creature_ai_spec_flags);
    CHECK(dm2_v1_creature_ai_spec_flags(TYPE_GATED_ALLOC, &flags) == 1 &&
              flags == 0x0001u,
          "type 12 flags come from AI row 5 word@0 (bit0 set)");
    CHECK(dm2_v1_creature_ai_spec_flags(TYPE_GATED_SKIP, &flags) == 1 &&
              flags == 0x0200u,
          "type 7 flags come from AI row 9 word@0 (bit0 clear)");
    CHECK(dm2_v1_creature_ai_spec_flags(TYPE_UNKNOWN, &flags) == 0 &&
              flags == 0,
          "type without a CREATURES word@5 row fails closed");
    CHECK(dm2_v1_creature_ai_spec_flags(-1, &flags) == 0 &&
              dm2_v1_creature_ai_spec_flags(DM2_AI_TABLE_SIZE, &flags) == 0,
          "out-of-range types fail closed");

    /* ── (c) ATTACK_CREATURE vl_18 gate (c_creature.cpp:370-385) ────── */
    CHECK(dm2_v1_caii_attack_guard_allows_alloc(&set, rec_handle(0)) == 1,
          "gate permits alloc when flags bit0 is set (vl_18 != 0)");
    CHECK(dm2_v1_caii_attack_guard_allows_alloc(&set, rec_handle(1)) == 0,
          "gate denies alloc when flags bit0 is clear (source returns)");
    CHECK(dm2_v1_caii_attack_guard_allows_alloc(&set, rec_handle(2)) == -1,
          "gate reports unknown for a type without a loaded AI row");
    CHECK(dm2_v1_caii_attack_guard_allows_alloc(&set, 0x0001) == 0,
          "non-creature DB handle rejected fail-closed");
    CHECK(dm2_v1_caii_attack_guard_allows_alloc(NULL, rec_handle(0)) == 0,
          "NULL pool set rejected");

    /* ── (d) 0fcb record-delete flag, full source matrix ──────────────
     * flag = ((flags & 1) == 0 && slot byte@1a == 0x13)
     * (c_1c9a.cpp:5917-5929). */
    memset(&alloc, 0, sizeof(alloc));
    CHECK(dm2_v1_caii_alloc_to_creature(&set, &dungeon, &caii, &queue,
                                        0, 200ul, rec_handle(0),
                                        0, 0, &alloc) == 1,
          "type-12 creature reactivates");
    slot0 = caii.slots + (size_t)alloc.slot_index * DM2_V1_CAII_SLOT_SIZE;
    slot0[0x1a] = 0x13u;
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_caii_free_slot(&set, &caii, &queue, alloc.slot_index,
                                &rc) == 1 &&
              rc.record_delete_flag == 0 &&
              rc.record_delete_unbound == 1,
          "bit0-set flags suppress the record-delete flag even at 0x13");

    memset(&alloc, 0, sizeof(alloc));
    CHECK(dm2_v1_caii_alloc_to_creature(&set, &dungeon, &caii, &queue,
                                        0, 300ul, rec_handle(1),
                                        0, 1, &alloc) == 1,
          "type-7 creature activates");
    slot1 = caii.slots + (size_t)alloc.slot_index * DM2_V1_CAII_SLOT_SIZE;
    slot1[0x1a] = 0x13u;
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_caii_free_slot(&set, &caii, &queue, alloc.slot_index,
                                &rc) == 1 &&
              rc.record_delete_flag == 1 &&
              rc.record_delete_unbound == 1,
          "bit0-clear flags plus byte@1a == 0x13 raise the delete flag");

    memset(&alloc, 0, sizeof(alloc));
    CHECK(dm2_v1_caii_alloc_to_creature(&set, &dungeon, &caii, &queue,
                                        0, 400ul, rec_handle(1),
                                        0, 1, &alloc) == 1,
          "type-7 creature reactivates in the freed slot");
    /* alloc leaves byte@1a at 0x11 (ungrouped record) — not 0x13. */
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_caii_free_slot(&set, &caii, &queue, alloc.slot_index,
                                &rc) == 1 &&
              rc.record_delete_flag == 0,
          "byte@1a != 0x13 keeps the delete flag clear");

    dm2_v1_caii_array_free(&caii);
    dm2_v1_record_pool_set_free(&set);
    dm2_v1_creature_reset_ai_table();

    if (g_failures != 0) {
        fprintf(stderr,
                "dm2_v1_caii_ai_spec_pc34_compat: %d failure(s)\n",
                g_failures);
        return 1;
    }
    printf("dm2_v1_caii_ai_spec_pc34_compat: all checks passed\n");
    return 0;
}
