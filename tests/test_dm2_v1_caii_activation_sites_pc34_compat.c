/*
 * test_dm2_v1_caii_activation_sites_pc34_compat.c — the two direct
 * DM2_ALLOC_CAII_TO_CREATURE activation call sites, bound at the source's
 * event places.
 *
 * Source-lock anchors:
 *   skproject/SKULLWIN/c_tim_proc.cpp:2859-2900  DM2_ANIMATE_CREATURE:
 *     GET_CREATURE_AT early return; flags bit0 set AND record byte@5 ==
 *     0xff -> ALLOC_CAII_TO_CREATURE; CCM tail host-owned
 *   skproject/SKULLWIN/c_tim_proc.cpp:3177-3184  floor-mecha walk reaching
 *     DM2_ANIMATE_CREATURE for tile record type 0x3a
 *   skproject/SKULLWIN/c_moverec.cpp:960-985     DM2_moverec_3CE7D:
 *     byte@5 != 0xff -> in-place setxyA/setmticks payload update of the
 *     pending think timer; byte@5 == 0xff AND flags bit0 CLEAR -> ALLOC
 *     (the OPPOSITE gate of the c_tim_proc.cpp:2887 site)
 *   skproject/SKULLWIN/c_timer.h:66,80-82        setmticks(map, ticks) /
 *     setxyA payload byte semantics
 *
 * The fixture reuses the synthetic dtWordValue GDAT session from the
 * AI-spec gate test: type 12 -> row 5 -> 0x0001 (bit0 set), type 7 ->
 * row 9 -> 0x0200 (bit0 clear).  Record 0 (type 12) sits at cell (0,0),
 * record 1 (type 7) at cell (0,1).
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
    DM2_V1_CaiiAnimateActivationReceipt anim;
    DM2_V1_CaiiMoverecActivationReceipt move;
    DM2_V1_SourceTimer peeked;
    uint8_t *record0;
    uint8_t *record1;
    uint8_t *slot;
    uint32_t ticket;
    uint32_t ticks_before;
    size_t count_before;

    build_dungeon(&dungeon, raw);
    build_pools(&set);
    build_ai_loader(&loader, entries, raw_offsets, raw_sizes, raw_data);
    dm2_v1_caii_array_init(&caii, 3);
    dm2_v1_source_timer_queue_init(&queue);
    record0 = set.pools[4].bytes;
    record1 = set.pools[4].bytes + 16;

    /* ── (a) NULL/invalid arguments fail closed ─────────────────────── */
    CHECK(dm2_v1_caii_animate_activation(NULL, &dungeon, &caii, &queue,
                                         0, 100ul, 0, 0, &anim) == 0 &&
              dm2_v1_caii_animate_activation(&set, NULL, &caii, &queue,
                                             0, 100ul, 0, 0, &anim) == 0 &&
              dm2_v1_caii_moverec_activation(&set, &dungeon, NULL, &queue,
                                             0, 100ul, rec_handle(0),
                                             0, 0, &move) == 0 &&
              dm2_v1_caii_moverec_activation(&set, &dungeon, &caii, NULL,
                                             0, 100ul, rec_handle(0),
                                             0, 0, &move) == 0,
          "NULL arguments rejected fail-closed");

    /* ── (b) no provider wired: unknown provenance fails closed ─────── */
    dm2_v1_creature_reset_ai_table();
    memset(&anim, 0, sizeof(anim));
    CHECK(dm2_v1_caii_animate_activation(&set, &dungeon, &caii, &queue,
                                         0, 100ul, 0, 0, &anim) == 0 &&
              anim.resolved == 1 &&
              anim.ai_flags_known == 0 &&
              anim.gate_taken == 0 &&
              anim.alloc_performed == 0 &&
              record0[5] == 0xffu,
          "animate site without a provider never allocs");
    memset(&move, 0, sizeof(move));
    CHECK(dm2_v1_caii_moverec_activation(&set, &dungeon, &caii, &queue,
                                         0, 100ul, rec_handle(1),
                                         0, 1, &move) == 0 &&
              move.ai_flags_known == 0 &&
              move.alloc_performed == 0 &&
              record1[5] == 0xffu,
          "moverec site without a provider never allocs");

    /* ── (c) GDAT session + provider wired ──────────────────────────── */
    CHECK(dm2_v1_creature_load_ai_table_from_gdat(&loader) == 2,
          "synthetic GDAT session loads two AI rows");
    dm2_v1_caii_set_ai_spec_flags_fn(dm2_v1_creature_ai_spec_flags);

    /* ── (d) animate site: bit0-set type allocates (c_tim_proc.cpp:2886-2891) */
    memset(&anim, 0, sizeof(anim));
    CHECK(dm2_v1_caii_animate_activation(&set, &dungeon, &caii, &queue,
                                         0, 200ul, 0, 0, &anim) == 1 &&
              anim.resolved == 1 &&
              anim.creature_type == TYPE_BIT0_SET &&
              anim.ai_flags_known == 1 &&
              anim.gate_taken == 1 &&
              anim.alloc_performed == 1 &&
              anim.ccm_tail_unbound == 1 &&
              record0[5] != 0xffu,
          "bit0-set creature without a slot activates through the animate gate");
    CHECK(queue.count == 1,
          "the bound allocator queued the creature's first think timer");

    /* ── (e) animate site: already-allocated creature skips the gate ── */
    memset(&anim, 0, sizeof(anim));
    count_before = queue.count;
    CHECK(dm2_v1_caii_animate_activation(&set, &dungeon, &caii, &queue,
                                         0, 210ul, 0, 0, &anim) == 1 &&
              anim.gate_taken == 0 &&
              anim.alloc_performed == 0 &&
              anim.ccm_tail_unbound == 1 &&
              queue.count == count_before,
          "an owned slot keeps the animate gate closed");

    /* ── (f) animate site: bit0-clear type never allocs here ────────── */
    memset(&anim, 0, sizeof(anim));
    CHECK(dm2_v1_caii_animate_activation(&set, &dungeon, &caii, &queue,
                                         0, 220ul, 0, 1, &anim) == 1 &&
              anim.creature_type == TYPE_BIT0_CLEAR &&
              anim.gate_taken == 0 &&
              anim.alloc_performed == 0 &&
              record1[5] == 0xffu,
          "bit0-clear creature skips the animate gate (opposite of moverec)");

    /* ── (g) animate site: no creature at the cell ──────────────────── */
    memset(&anim, 0, sizeof(anim));
    CHECK(dm2_v1_caii_animate_activation(&set, &dungeon, &caii, &queue,
                                         0, 230ul, 1, 1, &anim) == 0 &&
              anim.creature_not_found == 1,
          "no creature at the cell takes the source early return");

    /* ── (h) moverec site: bit0-clear type allocates (c_moverec.cpp:980-984) */
    memset(&move, 0, sizeof(move));
    CHECK(dm2_v1_caii_moverec_activation(&set, &dungeon, &caii, &queue,
                                         0, 300ul, rec_handle(1),
                                         0, 1, &move) == 1 &&
              move.had_slot == 0 &&
              move.creature_type == TYPE_BIT0_CLEAR &&
              move.ai_flags_known == 1 &&
              move.gate_taken == 1 &&
              move.alloc_performed == 1 &&
              move.minion_door_unbound == 1 &&
              record1[5] != 0xffu,
          "bit0-clear creature without a slot activates through the moverec gate");

    /* ── (i) moverec site: pending timer payload updated in place ─────
     * (c_moverec.cpp:975-978 setxyA + setmticks over the session ticket) */
    slot = caii.slots + (size_t)record1[5] * DM2_V1_CAII_SLOT_SIZE;
    ticket = (uint32_t)rd16(slot + 2);
    CHECK(ticket != 0u && ticket != 0xffffu,
          "the allocated creature owns a pending think timer");
    memset(&peeked, 0, sizeof(peeked));
    CHECK(dm2_v1_source_timer_peek_ticket(&queue, ticket, &peeked) == 1,
          "pending timer peeks before the move");
    ticks_before = peeked.ticks_and_map & DM2_V1_SOURCE_TIMER_TICK_MASK;
    memset(&move, 0, sizeof(move));
    count_before = queue.count;
    CHECK(dm2_v1_caii_moverec_activation(&set, &dungeon, &caii, &queue,
                                         5, 310ul, rec_handle(1),
                                         1, 0, &move) == 1 &&
              move.had_slot == 1 &&
              move.timer_updated == 1 &&
              move.alloc_performed == 0 &&
              queue.count == count_before,
          "owned slot moves the pending timer payload in place");
    memset(&peeked, 0, sizeof(peeked));
    CHECK(dm2_v1_source_timer_peek_ticket(&queue, ticket, &peeked) == 1 &&
              ((uint16_t)peeked.value_a & 0xffu) == 1u &&
              (((uint16_t)peeked.value_a >> 8) & 0xffu) == 0u &&
              (peeked.ticks_and_map >> 24) == 5u &&
              (peeked.ticks_and_map & DM2_V1_SOURCE_TIMER_TICK_MASK) ==
                  ticks_before,
          "setxyA wrote the destination, setmticks the map, ticks preserved");

    /* ── (j) moverec site: slot word@2 == -1 is the source no-op ────── */
    wr16(slot + 2, (int16_t)0xffff);
    memset(&move, 0, sizeof(move));
    CHECK(dm2_v1_caii_moverec_activation(&set, &dungeon, &caii, &queue,
                                         0, 320ul, rec_handle(1),
                                         1, 0, &move) == 0 &&
              move.had_slot == 1 &&
              move.no_pending_timer == 1 &&
              move.timer_updated == 0,
          "no pending timer takes the source no-op");

    /* ── (k) moverec site: a dead ticket fails closed ────────────────── */
    wr16(slot + 2, (int16_t)0x7777);
    memset(&move, 0, sizeof(move));
    CHECK(dm2_v1_caii_moverec_activation(&set, &dungeon, &caii, &queue,
                                         0, 330ul, rec_handle(1),
                                         1, 0, &move) == 0 &&
              move.stale_ticket == 1 &&
              move.timer_updated == 0,
          "a dead session ticket fails closed without mutation");

    /* ── (l) moverec site: bit0-set type without a slot does nothing ── */
    record0[5] = 0xffu; /* undo the (d) alloc to re-enter the gate */
    memset(&move, 0, sizeof(move));
    CHECK(dm2_v1_caii_moverec_activation(&set, &dungeon, &caii, &queue,
                                         0, 340ul, rec_handle(0),
                                         0, 0, &move) == 0 &&
              move.ai_flags_known == 1 &&
              move.gate_taken == 0 &&
              move.alloc_performed == 0 &&
              record0[5] == 0xffu,
          "bit0-set creature skips the moverec gate (opposite of animate)");

    /* ── (m) timeline primitive: direct contract ────────────────────── */
    CHECK(dm2_v1_source_timer_update_payload(NULL, ticket, 0, 0, 0) == 0 &&
              dm2_v1_source_timer_update_payload(&queue, 0u, 0, 0, 0) == 0 &&
              dm2_v1_source_timer_update_payload(&queue, 0x7777u, 0, 0,
                                                 0) == 0,
          "update payload fails closed for NULL/zero/unknown tickets");

    CHECK(strstr(anim.source_evidence, "c_tim_proc.cpp:2859-2900") != NULL,
          "animate evidence cites DM2_ANIMATE_CREATURE");
    CHECK(strstr(move.source_evidence, "c_moverec.cpp:960-985") != NULL,
          "moverec evidence cites DM2_moverec_3CE7D");

    dm2_v1_caii_array_free(&caii);
    dm2_v1_record_pool_set_free(&set);
    dm2_v1_creature_reset_ai_table();

    if (g_failures != 0) {
        fprintf(stderr,
                "dm2_v1_caii_activation_sites_pc34_compat: %d failure(s)\n",
                g_failures);
        return 1;
    }
    printf("dm2_v1_caii_activation_sites_pc34_compat: all checks passed\n");
    return 0;
}
