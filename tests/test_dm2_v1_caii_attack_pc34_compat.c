/*
 * test_dm2_v1_caii_attack_pc34_compat.c — DM2_ATTACK_CREATURE bounded
 * slice (c_creature.cpp:318-649).
 *
 * Source-lock anchors:
 *   skproject/SKULLWIN/c_creature.cpp:345-352  handle -1 resolve + early return
 *   skproject/SKULLWIN/c_creature.cpp:353-369  vol_00 bit 0x4000/0x2000 dance
 *   skproject/SKULLWIN/c_creature.cpp:374-385  vl_18 gate + ALLOC_CAII
 *   skproject/SKULLWIN/c_creature.cpp:389-393  slot word@0x14 HP add
 *   skproject/SKULLWIN/c_creature.cpp:394-435  aggro block (bands + BaseHP pct)
 *   skproject/SKULLWIN/c_creature.cpp:438-536  c_ai turn block (host-owned;
 *     entry gate table1d607e uc[0] & 0x80 data-backed)
 *   skproject/SKULLWIN/c_creature.cpp:539-563  reaction roll + champion bit
 *   skproject/SKULLWIN/c_creature.cpp:566-648  reschedule gate + 0db0/0cf7
 *   skproject/SKULLWIN/mdata.c:1564-1639       table1d607e + table1d613a
 *   skproject/SKULLWIN/c_random.cpp:13-47      session LCG draws
 *
 * Synthetic dtWordValue GDAT fixture (no game data):
 *   type 12 -> AI row 5  -> flags 0x0001 (bit0 set),   BaseHP 40, w1 = 33
 *             (table1d607e[33].uc[0] = 0x1b: c_ai gate PASSES, t6 & 0x410)
 *   type  7 -> AI row 9  -> flags 0x0200 (bit0 clear), BaseHP 24, w1 = 3
 *             (table1d607e[3].uc[0] = 0x80: c_ai gate CLOSED, t6 & 0x410 == 0)
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

#define TYPE_FLAGGED 12 /* flags bit0 set */
#define TYPE_PLAIN 7    /* flags bit0 clear */

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
    static const uint8_t types[2] = { TYPE_FLAGGED, TYPE_PLAIN };
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
                   TYPE_FLAGGED, 0x05, 5);
    set_word_entry(&entries[1], DM2_GDAT_CATEGORY_CREATURES,
                   TYPE_PLAIN, 0x05, 9);
    set_word_entry(&entries[2], DM2_GDAT_CATEGORY_CREATURE_AI, 5, 0, 0x01);
    set_word_entry(&entries[3], DM2_GDAT_CATEGORY_CREATURE_AI, 5, 4, 40);
    set_word_entry(&entries[4], DM2_GDAT_CATEGORY_CREATURE_AI, 9, 1, 0x02);
    set_word_entry(&entries[5], DM2_GDAT_CATEGORY_CREATURE_AI, 9, 4, 24);
    set_word_entry(&entries[6], DM2_GDAT_CATEGORY_CREATURES,
                   TYPE_FLAGGED, 0x01, 33);
    set_word_entry(&entries[7], DM2_GDAT_CATEGORY_CREATURES,
                   TYPE_PLAIN, 0x01, 3);

    memset(loader, 0, sizeof(*loader));
    loader->data = raw_data;
    loader->data_size = 1;
    loader->loaded = 1;
    loader->raw_data_count = 1;
    loader->raw_offsets = raw_offsets;
    loader->raw_sizes = raw_sizes;
    loader->entries = entries;
    loader->entry_count = 8;
}

/* Fresh session state per scenario. */
static void reset_world(DM2_V1_RecordPoolSet *set, DM2_V1_CaiiArray *caii,
                        DM2_V1_SourceTimerQueue *queue)
{
    dm2_v1_record_pool_set_free(set);
    dm2_v1_caii_array_free(caii);
    build_pools(set);
    dm2_v1_caii_array_init(caii, 3);
    dm2_v1_source_timer_queue_init(queue);
}

/* Activate a creature directly through the bound allocator (the
 * allocator itself carries no vl_18 gate — that gate lives in
 * ATTACK_CREATURE). */
static int activate(DM2_V1_RecordPoolSet *set, DM2_V1_DungeonData *dungeon,
                    DM2_V1_CaiiArray *caii, DM2_V1_SourceTimerQueue *queue,
                    int index, int x, int y, DM2_V1_CaiiAllocReceipt *alloc)
{
    memset(alloc, 0, sizeof(*alloc));
    return dm2_v1_caii_alloc_to_creature(set, dungeon, caii, queue,
                                         0, 100ul, rec_handle(index),
                                         x, y, alloc);
}

int main(void)
{
    static uint8_t raw[RAW_SIZE];
    DM2_V1_GdatEntry entries[8];
    uint32_t raw_offsets[1] = { 0 };
    uint32_t raw_sizes[1] = { 0 };
    uint8_t raw_data[1] = { 0 };
    DM2_V1_AssetLoader loader;
    DM2_V1_DungeonData dungeon;
    DM2_V1_RecordPoolSet set;
    DM2_V1_CaiiArray caii;
    DM2_V1_SourceTimerQueue queue;
    DM2_V1_CaiiAllocReceipt alloc;
    DM2_V1_CaiiAttackReceipt rc;
    DM2_V1_CaiiAiTurnReceipt tr;
    DM2_V1_CaiiCcmEndRequeueReceipt cr;
    DM2_V1_SourceTimer tim;
    DM2_V1_SourceTimer peeked;
    DM2_V1_CaiiDeleteCreatureRecordReceipt head;
    DM2_V1_DropRng rng;
    uint8_t *rec0;
    uint8_t *rec1;
    uint8_t *slot;

    build_dungeon(&dungeon, raw);
    build_pools(&set);
    build_ai_loader(&loader, entries, raw_offsets, raw_sizes, raw_data);
    dm2_v1_caii_array_init(&caii, 3);
    dm2_v1_source_timer_queue_init(&queue);
    dm2_v1_creature_reset_ai_table();

    /* ── (a) guard paths ───────────────────────────────────────────── */
    rng.random = 0;
    CHECK(dm2_v1_caii_attack_creature(NULL, &dungeon, &caii, &queue, &rng,
                                      0, 100ul, rec_handle(0), 0, 0, 0, 0,
                                      0, 10, 5, &rc) == 0 &&
              dm2_v1_caii_attack_creature(&set, NULL, &caii, &queue, &rng,
                                          0, 100ul, rec_handle(0), 0, 0, 0, 0,
                                          0, 10, 5, &rc) == 0,
          "NULL pool/dungeon rejected");
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_caii_attack_creature(&set, &dungeon, &caii, &queue, &rng,
                                      0, 100ul, DM2_V1_RECORD_HANDLE_NULL,
                                      1, 1, 1, 1, 0, 10, 5, &rc) == 0 &&
              rc.creature_not_found == 1,
          "handle -1 with an empty cell takes the source early return");

    /* ── (b) no provider: the body fails closed ────────────────────── */
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_caii_attack_creature(&set, &dungeon, &caii, &queue, &rng,
                                      0, 100ul, rec_handle(0), 0, 0, 0, 0,
                                      0, 10, 5, &rc) == 0 &&
              rc.ai_flags_unknown == 1,
          "unwired flags provider fails the body closed");

    /* ── (c) GDAT session, flags provider only: head probe unknown ─── */
    CHECK(dm2_v1_creature_load_ai_table_from_gdat(&loader) == 2,
          "synthetic GDAT session loads two AI rows");
    dm2_v1_caii_set_ai_spec_flags_fn(dm2_v1_creature_ai_spec_flags);
    memset(&head, 0, sizeof(head));
    CHECK(dm2_v1_caii_delete_creature_record_head(&set, &dungeon, &caii,
                                                  0, 1, &head) == 1 &&
              head.ai_bit0_clear == 1 &&
              head.invoke_message_would_run == -1,
          "invoke-message probe unknown without the word@1 provider");

    /* ── (d) word@1 + BaseHP providers wired: probe data-backed ────── */
    dm2_v1_caii_set_ai_base_hp_fn(dm2_v1_creature_ai_base_hp);
    dm2_v1_caii_set_gdat_word1_fn(dm2_v1_creature_gdat_word1);
    memset(&head, 0, sizeof(head));
    CHECK(dm2_v1_caii_delete_creature_record_head(&set, &dungeon, &caii,
                                                  0, 1, &head) == 1 &&
              head.invoke_message_would_run == 1,
          "table1d607e[3].uc[0] & 4 == 0 -> the branch would run");

    /* ── (e) denied: bit0-clear creature without a slot ────────────── */
    reset_world(&set, &caii, &queue);
    rng.random = 0;
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_caii_attack_creature(&set, &dungeon, &caii, &queue, &rng,
                                      0, 100ul, rec_handle(1), 0, 1, 0, 1,
                                      0, 10, 5, &rc) == 0 &&
              rc.denied_static_no_slot == 1 &&
              rc.aggro_evaluated == 0,
          "vl_18 == 0 without a slot takes the source early return");

    /* ── (f) full completion: bit0-set creature, alloc through the
     * gate, HP add, champion bit set, reschedule issued ────────────── */
    reset_world(&set, &caii, &queue);
    rng.random = 0;
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_caii_attack_creature(&set, &dungeon, &caii, &queue, &rng,
                                      0, 200ul, rec_handle(0), 0, 0, 0, 0,
                                      0x0002, 50, 10, &rc) == 1 &&
              rc.completed == 1 &&
              rc.alloc_performed == 1 &&
              rc.hp_word_after == 10 &&
              rc.aggro_evaluated == 0 &&
              rc.reaction_roll == 0 &&
              rc.reaction_success == 1 &&
              rc.champion_bit_set == 1 &&
              rc.final_rg1 == 0 &&
              rc.rescheduled == 1 &&
              rc.timer_ticket != 0u,
          "type-12 attack completes end-to-end");
    rec0 = set.pools[4].bytes;
    CHECK(rd16(rec0 + 0xa) == 0x0004u,
          "champion bit (1 << 2) OR-ed into record word@0xa");
    CHECK(rd16(rec0 + 0xa) == 0x0004u && queue.count == 1,
          "think timer re-queued after the attack");

    /* ── (g) aggro high band + closed c_ai gate: stream stays
     * aligned, body completes with the block receipted ─────────────── */
    reset_world(&set, &caii, &queue);
    CHECK(activate(&set, &dungeon, &caii, &queue, 1, 0, 1, &alloc) == 1,
          "type-7 creature pre-activated");
    rng.random = 0;
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_caii_attack_creature(&set, &dungeon, &caii, &queue, &rng,
                                      0, 300ul, rec_handle(1), 0, 1, 0, 1,
                                      0x0001, 60, 40, &rc) == 1 &&
              rc.aggro_evaluated == 1 &&
              rc.aggro_set == 1 &&
              rc.ai_turn_unbound == 1 &&
              rc.ai_turn_gate_passed == 0 &&
              rc.rng_stream_diverged == 0 &&
              rc.completed == 1,
          "aggro > 0x1e sets the bit; closed c_ai gate keeps the stream");
    rec1 = set.pools[4].bytes + 16;
    CHECK((rd16(rec1 + 0xa) & 0x0006u) == 0x0006u,
          "aggro bit 2 and champion bit 0 both set in word@0xa");

    /* ── (h) bound turn block: vol 0x4000 survives the coin flip, the
     * c_ai gate passes, the direction dance runs on the session stream
     * and DM2_ai_13e4_0360 (argl0 == 0) writes slot byte@0x17 ──────── */
    reset_world(&set, &caii, &queue);
    rng.random = 0; /* draws: survival bit 0, entry bit 1, RANDDIR 2,
                     * reaction r16 60 */
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_caii_attack_creature(&set, &dungeon, &caii, &queue, &rng,
                                      0, 400ul, rec_handle(0), 0, 0, 2, 0,
                                      0x4001, 50, 10, &rc) == 1 &&
              rc.alloc_performed == 1 &&
              rc.hp_word_after == 10 &&
              rc.ai_turn_unbound == 0 &&
              rc.ai_turn_gate_passed == 1 &&
              rc.rng_stream_diverged == 0 &&
              rc.ai_turn_ran == 1 &&
              rc.ai_turn_entry_roll == 1 &&
              rc.ai_turn_vector_dir == 1 &&
              rc.ai_turn_facing == 0 &&
              rc.ai_turn_dir == 7 &&
              rc.ai_turn_applied == 1 &&
              rc.reaction_roll == 60 &&
              rc.reaction_success == 0 &&
              rc.completed == 1,
          "bound c_ai turn block turns the creature toward the attack");
    rec0 = set.pools[4].bytes;
    slot = caii.slots + (size_t)rec0[5] * DM2_V1_CAII_SLOT_SIZE;
    CHECK(rd16(rec0 + 0xa) == 0u,
          "reaction roll 60 >= strength 50 leaves word@0xa untouched");
    CHECK(slot[0x17] == 7u,
          "slot byte@0x17 holds the bound turn direction (c_ai.cpp:5946)");

    /* ── (i) aggro low band via the BaseHP percentage probe ────────── */
    reset_world(&set, &caii, &queue);
    CHECK(activate(&set, &dungeon, &caii, &queue, 1, 0, 1, &alloc) == 1,
          "type-7 creature pre-activated for the low band");
    rng.random = 0;
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_caii_attack_creature(&set, &dungeon, &caii, &queue, &rng,
                                      0, 500ul, rec_handle(1), 0, 1, 0, 1,
                                      0x0000, 50, 3, &rc) == 1 &&
              rc.aggro_evaluated == 1 &&
              rc.aggro_set == 0 &&
              rc.aggro_undecided == 0 &&
              rc.completed == 1,
          "hp 3 <= 4: 100*3/24 = 12 <= 15 leaves aggro clear");
    reset_world(&set, &caii, &queue);
    CHECK(activate(&set, &dungeon, &caii, &queue, 1, 0, 1, &alloc) == 1,
          "type-7 creature pre-activated for the setting probe");
    rng.random = 0;
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_caii_attack_creature(&set, &dungeon, &caii, &queue, &rng,
                                      0, 500ul, rec_handle(1), 0, 1, 0, 1,
                                      0x0000, 50, 4, &rc) == 1 &&
              rc.aggro_set == 1,
          "hp 4 <= 4: 100*4/24 = 16 > 15 sets aggro deterministically");

    /* ── (j) middle band consumes RANDDIR from the session stream ──── */
    reset_world(&set, &caii, &queue);
    CHECK(activate(&set, &dungeon, &caii, &queue, 1, 0, 1, &alloc) == 1,
          "type-7 creature pre-activated for the RNG band");
    rng.random = 0; /* RANDDIR draw is 0 -> aggro directly */
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_caii_attack_creature(&set, &dungeon, &caii, &queue, &rng,
                                      0, 600ul, rec_handle(1), 0, 1, 0, 1,
                                      0x0000, 50, 10, &rc) == 1 &&
              rc.aggro_evaluated == 1 &&
              rc.aggro_set == 1,
          "band 5..30: RANDDIR == 0 sets aggro without the probe");
    reset_world(&set, &caii, &queue);
    CHECK(activate(&set, &dungeon, &caii, &queue, 1, 0, 1, &alloc) == 1,
          "type-7 creature pre-activated for the rng-less band");
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_caii_attack_creature(&set, &dungeon, &caii, &queue, NULL,
                                      0, 600ul, rec_handle(1), 0, 1, 0, 1,
                                      0x0000, 50, 10, &rc) == 0 &&
              rc.aggro_evaluated == 1 &&
              rc.aggro_set == 0 &&
              rc.aggro_undecided == 1 &&
              rc.rng_unbound == 1,
          "band 5..30 without a stream receipts rng_unbound");

    /* ── (k) vl_10 + strength 0 forces rg1 = 1 ─────────────────────── */
    reset_world(&set, &caii, &queue);
    CHECK(activate(&set, &dungeon, &caii, &queue, 1, 0, 1, &alloc) == 1,
          "type-7 creature pre-activated for the vl_10 path");
    rng.random = 0;
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_caii_attack_creature(&set, &dungeon, &caii, &queue, &rng,
                                      0, 700ul, rec_handle(1), 0, 1, 0, 1,
                                      0x2000, 0, 5, &rc) == 1 &&
              rc.reaction_success == 0 &&
              rc.champion_bit_set == 0 &&
              rc.final_rg1 == 1 &&
              rc.completed == 1,
          "vl_18 == 0 && vl_10 != 0 && strength == 0 -> rg1 = 1");

    /* ── (l) dying mode and below-threshold early returns ──────────── */
    reset_world(&set, &caii, &queue);
    CHECK(activate(&set, &dungeon, &caii, &queue, 1, 0, 1, &alloc) == 1,
          "type-7 creature pre-activated for dying mode");
    slot = caii.slots + (size_t)alloc.slot_index * DM2_V1_CAII_SLOT_SIZE;
    slot[0x1a] = 0x13u;
    rng.random = 0;
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_caii_attack_creature(&set, &dungeon, &caii, &queue, &rng,
                                      0, 800ul, rec_handle(1), 0, 1, 0, 1,
                                      0x0000, 50, 5, &rc) == 0 &&
              rc.dying_mode == 1 &&
              rc.rescheduled == 0 &&
              queue.count == 1,
          "slot byte@1a == 0x13 returns without rescheduling");

    reset_world(&set, &caii, &queue);
    CHECK(activate(&set, &dungeon, &caii, &queue, 1, 0, 1, &alloc) == 1,
          "type-7 creature pre-activated for the threshold");
    rec1 = set.pools[4].bytes + 16;
    wr16(rec1 + 6, 100); /* record word@6 threshold */
    rng.random = 0;
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_caii_attack_creature(&set, &dungeon, &caii, &queue, &rng,
                                      0, 900ul, rec_handle(1), 0, 1, 0, 1,
                                      0x0000, 50, 3, &rc) == 0 &&
              rc.final_rg1 == 0 &&
              rc.below_threshold == 1 &&
              rc.rescheduled == 0,
          "rg1 == 0 && hp 3 < word@6 100 keeps the current timer");

    /* ── (m) champion bit cleared when vol bit 0x8000 is set ────────── */
    reset_world(&set, &caii, &queue);
    CHECK(activate(&set, &dungeon, &caii, &queue, 0, 0, 0, &alloc) == 1,
          "type-12 creature pre-activated for the clear path");
    rec0 = set.pools[4].bytes;
    wr16(rec0 + 0xa, (int16_t)0x0006); /* bits 1+2 preset */
    rng.random = 0;
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_caii_attack_creature(&set, &dungeon, &caii, &queue, &rng,
                                      0, 950ul, rec_handle(0), 0, 0, 0, 0,
                                      0x8001, 50, 5, &rc) == 1 &&
              rc.champion_bit_cleared == 1 &&
              rc.champion_bit_set == 0 &&
              rd16(rec0 + 0xa) == 0x0004u,
          "vw_20 set: champion bit AND-ed out of record word@0xa");

    /* ── (n) table1d613a chain reaches rg1 = 1 via & 2 ─────────────── */
    reset_world(&set, &caii, &queue);
    CHECK(activate(&set, &dungeon, &caii, &queue, 0, 0, 0, &alloc) == 1,
          "type-12 creature pre-activated for the chain path");
    slot = caii.slots + (size_t)alloc.slot_index * DM2_V1_CAII_SLOT_SIZE;
    slot[0x1a] = 0x0eu; /* table1d613a[0x0e] = 0x12: & 0x10 and & 2 */
    rec0 = set.pools[4].bytes;
    wr16(rec0 + 0xa, 0x0004); /* aggro bit preset: keeps rg7 == 0 so the
                               * c_ai gate stay out of this probe */
    rng.random = 0;
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_caii_attack_creature(&set, &dungeon, &caii, &queue, &rng,
                                      0, 980ul, rec_handle(0), 0, 0, 0, 0,
                                      0x0000, 50, 5, &rc) == 1 &&
              rc.final_rg1 == 1 &&
              rc.completed == 1,
          "t613a & 0x10 -> w1 33 -> t6 & 0x410 -> t613a & 2 -> rg1 = 1");

    /* ── (o) mode byte beyond the proven table span fails closed ────── */
    reset_world(&set, &caii, &queue);
    CHECK(activate(&set, &dungeon, &caii, &queue, 0, 0, 0, &alloc) == 1,
          "type-12 creature pre-activated for the span guard");
    slot = caii.slots + (size_t)alloc.slot_index * DM2_V1_CAII_SLOT_SIZE;
    slot[0x1a] = 0x60u; /* > 0x55: outside table1d613a's proven span */
    rng.random = 0;
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_caii_attack_creature(&set, &dungeon, &caii, &queue, &rng,
                                      0, 990ul, rec_handle(0), 0, 0, 0, 0,
                                      0x0000, 50, 5, &rc) == 0 &&
              rc.hp_applied == 1 &&
              rc.mode_b1a_out_of_span == 1 &&
              rc.rescheduled == 0,
          "byte@1a > 0x55 fails closed after the source HP write (source would read OOB)");

    /* ── (p) entry RANDBIT 0: the bound turn block runs but turns
     * nothing; the stream stays aligned for the reaction roll ──────── */
    reset_world(&set, &caii, &queue);
    rng.random = 12; /* draws: survival bit 0, entry bit 0,
                      * reaction r16 34 */
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_caii_attack_creature(&set, &dungeon, &caii, &queue, &rng,
                                      0, 991ul, rec_handle(0), 0, 0, 2, 0,
                                      0x4001, 50, 10, &rc) == 1 &&
              rc.ai_turn_ran == 1 &&
              rc.ai_turn_entry_roll == 0 &&
              rc.ai_turn_vector_dir == -1 &&
              rc.ai_turn_dir == -1 &&
              rc.ai_turn_applied == 0 &&
              rc.reaction_roll == 34 &&
              rc.reaction_success == 1 &&
              rc.champion_bit_set == 1 &&
              rc.completed == 1,
          "entry flip 0 skips the turn, stream stays aligned");
    rec0 = set.pools[4].bytes;
    CHECK(rd16(rec0 + 0xa) == 0x0002u,
          "champion bit (1 << 1) OR-ed after the aligned reaction roll");

    /* ── (q) skip00248 reversal: RANDDIR 0 reverses the vector before
     * the final dance (seed 23: survival 0, entry 1, RANDDIR 0) ────── */
    reset_world(&set, &caii, &queue);
    rng.random = 23; /* draws: survival bit 0, entry bit 1, RANDDIR 0,
                      * reaction r16 40 */
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_caii_attack_creature(&set, &dungeon, &caii, &queue, &rng,
                                      0, 992ul, rec_handle(0), 0, 0, 2, 0,
                                      0x4001, 50, 10, &rc) == 1 &&
              rc.ai_turn_ran == 1 &&
              rc.ai_turn_vector_dir == 1 &&
              rc.ai_turn_dir == 6 &&
              rc.ai_turn_applied == 1 &&
              rc.reaction_roll == 40 &&
              rc.completed == 1,
          "skip00248 reversal path lands on relative turn 6");

    /* ── (r) the DM2_ai_13e4_0360 guard: slot byte@1a == 0x13 denies
     * the byte@0x17 write (and takes the dying-mode return later) ──── */
    reset_world(&set, &caii, &queue);
    CHECK(activate(&set, &dungeon, &caii, &queue, 0, 0, 0, &alloc) == 1,
          "type-12 creature pre-activated for the turn guard");
    slot = caii.slots + (size_t)alloc.slot_index * DM2_V1_CAII_SLOT_SIZE;
    slot[0x1a] = 0x13u; /* c_ai.cpp:5943-5944 guard + c_creature.cpp:638 */
    rng.random = 0;
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_caii_attack_creature(&set, &dungeon, &caii, &queue, &rng,
                                      0, 993ul, rec_handle(0), 0, 0, 2, 0,
                                      0x4001, 50, 10, &rc) == 0 &&
              rc.ai_turn_ran == 1 &&
              rc.ai_turn_dir == 7 &&
              rc.ai_turn_applied == 0 &&
              rc.ai_turn_guard_denied == 1 &&
              rc.dying_mode == 1 &&
              rc.rescheduled == 0,
          "0x13 mode denies the turn write and stops the body");

    /* ── (s) passing gate without a session stream still stops
     * fail-closed BEFORE the reaction roll ─────────────────────────── */
    reset_world(&set, &caii, &queue);
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_caii_attack_creature(&set, &dungeon, &caii, &queue, NULL,
                                      0, 994ul, rec_handle(0), 0, 0, 2, 0,
                                      0x4001, 50, 10, &rc) == 0 &&
              rc.ai_turn_gate_passed == 1 &&
              rc.ai_turn_ran == 0 &&
              rc.rng_stream_diverged == 1 &&
              rc.reaction_roll == -1 &&
              rc.completed == 0,
          "gate passed but unbound stream: diverged stop, fail-closed");

    /* ── (t) DM2_ai_13e4_0360 direct, argl0 == 0: the direction write
     * behind the guards (the ATTACK_CREATURE caller's mode) ────────── */
    reset_world(&set, &caii, &queue);
    CHECK(activate(&set, &dungeon, &caii, &queue, 0, 0, 0, &alloc) == 1,
          "type-12 creature pre-activated for the direct turn");
    memset(&tr, 0, sizeof(tr));
    CHECK(dm2_v1_caii_ai_13e4_0360(&set, &dungeon, &caii, &queue,
                                   0, 1000ul, rec_handle(0), 0, 0,
                                   2, 0, &tr) == 1 &&
              tr.dir_written == 1 &&
              tr.argl0_tail == 0 &&
              tr.completed == 1,
          "argl0 == 0 writes slot byte@0x17 and returns");
    slot = caii.slots + (size_t)alloc.slot_index * DM2_V1_CAII_SLOT_SIZE;
    CHECK(slot[0x17] == 2u,
          "slot byte@0x17 holds the direct direction (c_ai.cpp:5946)");

    /* ── (u) argl0 == 1, mode with table1d613a & 0x10: the byte@0x21
     * flag path; a follow-up call hits the 0x13 guard ──────────────── */
    reset_world(&set, &caii, &queue);
    CHECK(activate(&set, &dungeon, &caii, &queue, 0, 0, 0, &alloc) == 1,
          "type-12 creature pre-activated for the flag path");
    slot = caii.slots + (size_t)alloc.slot_index * DM2_V1_CAII_SLOT_SIZE;
    /* activation mode is 0x11 -> table1d613a[0x11] = 0x10: & 0x10 set */
    memset(&tr, 0, sizeof(tr));
    CHECK(dm2_v1_caii_ai_13e4_0360(&set, &dungeon, &caii, &queue,
                                   0, 1010ul, rec_handle(0), 0, 0,
                                   0x13, 1, &tr) == 1 &&
              tr.dir_written == 1 &&
              tr.argl0_tail == 1 &&
              tr.flag_set == 1 &&
              tr.timer_cancelled == 0 &&
              tr.rescheduled == 0 &&
              tr.completed == 1,
          "t613a & 0x10 sets byte@0x21 instead of requeuing");
    CHECK(slot[0x17] == 0x13u && slot[0x21] == 1u,
          "AI-stop marker and byte@0x21 flag in the slot");
    memset(&tr, 0, sizeof(tr));
    CHECK(dm2_v1_caii_ai_13e4_0360(&set, &dungeon, &caii, &queue,
                                   0, 1020ul, rec_handle(0), 0, 0,
                                   1, 0, &tr) == 0 &&
              tr.guard_denied == 1 &&
              tr.dir_written == 0,
          "byte@0x17 == 0x13 blocks further turns (c_ai.cpp:5941)");

    /* ── (v) argl0 == 1, mode without & 0x10: the bound 0db0 + 0cf7
     * cancel-and-requeue tail ──────────────────────────────────────── */
    reset_world(&set, &caii, &queue);
    CHECK(activate(&set, &dungeon, &caii, &queue, 0, 0, 0, &alloc) == 1,
          "type-12 creature pre-activated for the requeue path");
    slot = caii.slots + (size_t)alloc.slot_index * DM2_V1_CAII_SLOT_SIZE;
    slot[0x1a] = 0x00u; /* table1d613a[0] = 0x08: & 0x10 clear */
    CHECK(queue.count == 1, "activation queued the think timer");
    memset(&tr, 0, sizeof(tr));
    CHECK(dm2_v1_caii_ai_13e4_0360(&set, &dungeon, &caii, &queue,
                                   0, 1030ul, rec_handle(0), 0, 0,
                                   0x13, 1, &tr) == 1 &&
              tr.dir_written == 1 &&
              tr.argl0_tail == 1 &&
              tr.flag_set == 0 &&
              tr.timer_cancelled == 1 &&
              tr.rescheduled == 1 &&
              tr.timer_ticket != 0u &&
              tr.completed == 1 &&
              queue.count == 1,
          "t613a & 0x10 clear: cancel + requeue at the creature tile");

    /* ── (w) handle -1 resolution and the empty-cell early return ───── */
    reset_world(&set, &caii, &queue);
    CHECK(activate(&set, &dungeon, &caii, &queue, 1, 0, 1, &alloc) == 1,
          "type-7 creature pre-activated for the resolve path");
    memset(&tr, 0, sizeof(tr));
    CHECK(dm2_v1_caii_ai_13e4_0360(&set, &dungeon, &caii, &queue,
                                   0, 1040ul, DM2_V1_RECORD_HANDLE_NULL,
                                   0, 1, 3, 0, &tr) == 1 &&
              tr.record_handle == (int16_t)((1u << 14) | (4u << 10) | 1u) &&
              tr.dir_written == 1,
          "handle -1 resolves the creature at (x, y)");
    memset(&tr, 0, sizeof(tr));
    CHECK(dm2_v1_caii_ai_13e4_0360(&set, &dungeon, &caii, &queue,
                                   0, 1050ul, DM2_V1_RECORD_HANDLE_NULL,
                                   1, 1, 3, 0, &tr) == 0 &&
              tr.creature_not_found == 1,
          "handle -1 on an empty cell takes the source early return");

    /* ── (x) record without a CAII slot: the byte@5 guard ───────────── */
    reset_world(&set, &caii, &queue);
    memset(&tr, 0, sizeof(tr));
    CHECK(dm2_v1_caii_ai_13e4_0360(&set, &dungeon, &caii, &queue,
                                   0, 1060ul, rec_handle(0), 0, 0,
                                   1, 0, &tr) == 0 &&
              tr.no_slot == 1 &&
              tr.dir_written == 0,
          "byte@5 == 0xff takes the source early return");

    /* ── (y) argl0 == 1 with byte@1a beyond the proven span: the dir
     * write already happened, the table read fails closed ──────────── */
    reset_world(&set, &caii, &queue);
    CHECK(activate(&set, &dungeon, &caii, &queue, 0, 0, 0, &alloc) == 1,
          "type-12 creature pre-activated for the tail span guard");
    slot = caii.slots + (size_t)alloc.slot_index * DM2_V1_CAII_SLOT_SIZE;
    slot[0x1a] = 0x60u; /* > 0x55: outside table1d613a's proven span */
    memset(&tr, 0, sizeof(tr));
    CHECK(dm2_v1_caii_ai_13e4_0360(&set, &dungeon, &caii, &queue,
                                   0, 1070ul, rec_handle(0), 0, 0,
                                   0x13, 1, &tr) == 0 &&
              tr.dir_written == 1 &&
              tr.argl0_tail == 1 &&
              tr.mode_b1a_out_of_span == 1 &&
              tr.flag_set == 0 &&
              tr.rescheduled == 0,
          "byte@1a > 0x55 fails the tail closed after the dir write");

    /* ── (z) CCM-end requeue: pending timer cancelled, rebuilt timer
     * enqueued with the loop-result type and setmticks word, ticket
     * stored in slot word@2 ─────────────────────────────────────────── */
    reset_world(&set, &caii, &queue);
    CHECK(activate(&set, &dungeon, &caii, &queue, 0, 0, 0, &alloc) == 1,
          "type-12 creature pre-activated for the CCM requeue");
    slot = caii.slots + (size_t)alloc.slot_index * DM2_V1_CAII_SLOT_SIZE;
    memset(&tim, 0, sizeof(tim));
    tim.actor = 5;
    tim.value_a = 0x1234;
    tim.value_b = 0x0077;
    memset(&cr, 0, sizeof(cr));
    CHECK(dm2_v1_caii_ccm_end_requeue(&set, &caii, &queue, rec_handle(0),
                                      &tim, 0, 2, 0, 3, 1000, &cr) == 1 &&
              cr.timer_type == 0x22 &&
              cr.timer_cancelled == 1 &&
              cr.enqueued == 1 &&
              cr.timer_ticket != 0u &&
              cr.completed == 1 &&
              queue.count == 1,
          "loop result != 1 re-queues with type 0x22 (c_ai.cpp:5609-46)");
    CHECK(rd16(slot + 2) == (uint16_t)cr.timer_ticket,
          "slot word@2 holds the issued ticket (c_ai.cpp:5646)");
    memset(&peeked, 0, sizeof(peeked));
    CHECK(dm2_v1_source_timer_peek_ticket(&queue, cr.timer_ticket,
                                          &peeked) == 1 &&
              peeked.type == 0x22u &&
              peeked.ticks_and_map == ((3u << 24) | 1000u) &&
              peeked.actor == 5u &&
              peeked.value_a == 0x1234 &&
              peeked.value_b == 0x0077,
          "rebuilt timer: type + setmticks word, loop payload intact");

    /* ── (aa) loop result == 1 keeps type 0x21 ───────────────────────── */
    reset_world(&set, &caii, &queue);
    CHECK(activate(&set, &dungeon, &caii, &queue, 0, 0, 0, &alloc) == 1,
          "type-12 creature pre-activated for the 0x21 type");
    memset(&cr, 0, sizeof(cr));
    CHECK(dm2_v1_caii_ccm_end_requeue(&set, &caii, &queue, rec_handle(0),
                                      &tim, 0, 1, 0, 0, 42, &cr) == 1 &&
              cr.timer_type == 0x21 &&
              cr.enqueued == 1,
          "loop result == 1 re-queues with type 0x21");
    memset(&peeked, 0, sizeof(peeked));
    CHECK(dm2_v1_source_timer_peek_ticket(&queue, cr.timer_ticket,
                                          &peeked) == 1 &&
              peeked.type == 0x21u &&
              peeked.ticks_and_map == 42u,
          "map 0 with delta 42 in the setmticks word");

    /* ── (bb) s350.v1e0570 suppresses the requeue entirely ───────────── */
    reset_world(&set, &caii, &queue);
    CHECK(activate(&set, &dungeon, &caii, &queue, 0, 0, 0, &alloc) == 1,
          "type-12 creature pre-activated for the suppress path");
    memset(&cr, 0, sizeof(cr));
    CHECK(dm2_v1_caii_ccm_end_requeue(&set, &caii, &queue, rec_handle(0),
                                      &tim, 0, 2, 1, 3, 1000, &cr) == 0 &&
              cr.suppressed == 1 &&
              cr.timer_type == 0x22 &&
              cr.enqueued == 0 &&
              queue.count == 1,
          "v1e0570 set: no cancel, no enqueue (c_ai.cpp:5612-5613)");

    /* ── (cc) record without a CAII slot fails closed ────────────────── */
    reset_world(&set, &caii, &queue);
    memset(&cr, 0, sizeof(cr));
    CHECK(dm2_v1_caii_ccm_end_requeue(&set, &caii, &queue, rec_handle(0),
                                      &tim, 0, 2, 0, 3, 1000, &cr) == 0 &&
              cr.no_slot == 1 &&
              cr.enqueued == 0,
          "byte@5 == 0xff fails the requeue closed");

    CHECK(strstr(rc.source_evidence, "c_creature.cpp:318-649") != NULL,
          "evidence cites DM2_ATTACK_CREATURE");

    dm2_v1_caii_array_free(&caii);
    dm2_v1_record_pool_set_free(&set);
    dm2_v1_creature_reset_ai_table();

    if (g_failures != 0) {
        fprintf(stderr,
                "dm2_v1_caii_attack_pc34_compat: %d failure(s)\n",
                g_failures);
        return 1;
    }
    printf("dm2_v1_caii_attack_pc34_compat: all checks passed\n");
    return 0;
}
