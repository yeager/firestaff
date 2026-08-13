/*
 * test_dm2_v1_drop_possession_pc34_compat.c — DM2_DROP_CREATURE_POSSESSION
 * bounded slice.
 *
 * Verifies the skproject boundary:
 *   c_record.cpp:1562-1563  mode == 2 immediate return
 *   c_record.cpp:1568-1634  generated-drops loop (via the proven slots
 *                           binding) with the tile-rooted destination
 *   c_record.cpp:1682-1751  possession walk: next-link prefetch, AI
 *                           flags bit0 direction randomization folded
 *                           into the handle, DB != 0x0e tile append,
 *                           DB 0x0e dealloc (word@0 = 0xffff)
 *   DM2_QUEUE_NOISE_GEN2 stays receipted, never simulated
 */

#include "dm2_v1_drop_possession_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

/* 2x2 byte-square map.  Flagged: (0,0) -> idx 0 (ground item DB5 rec0),
 * (0,1) -> idx 1 (empty chain).  (1,1) carries no object flag. */
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

    wr16(raw + GROUND_BASE + 0, mk_handle(5, 0));
    wr16(raw + GROUND_BASE + 2, DM2_V1_RECORD_HANDLE_END);
}

static void build_pools(DM2_V1_RecordPoolSet *set)
{
    memset(set, 0, sizeof(*set));

    /* DB4 creature: possession head word@2 -> DB5 rec1, type 7. */
    set->pools[4].record_size = 16;
    set->pools[4].record_count = 1;
    set->pools[4].source_base = 0;
    set->pools[4].bytes = calloc(1, 16);
    wr16(set->pools[4].bytes + 0, DM2_V1_RECORD_HANDLE_END);
    set->pools[4].bytes[4] = 7;
    wr16(set->pools[4].bytes + 2, mk_handle(5, 1));

    /* DB5 weapons (4-byte): rec0 ground item (next END), rec1
     * possession item (next -> DB10 rec0), rec2/rec3 free
     * (w0 = OBJECT_NULL) for the generated-drops allocations. */
    set->pools[5].record_size = 4;
    set->pools[5].record_count = 4;
    set->pools[5].source_base = 16;
    set->pools[5].bytes = calloc(4, 4);
    wr16(set->pools[5].bytes + 0, DM2_V1_RECORD_HANDLE_END);
    wr16(set->pools[5].bytes + 4, mk_handle(10, 0));
    wr16(set->pools[5].bytes + 8, DM2_V1_RECORD_HANDLE_NULL);
    wr16(set->pools[5].bytes + 12, DM2_V1_RECORD_HANDLE_NULL);

    /* DB10 misc (4-byte): rec0 possession item (next -> DB14 rec0). */
    set->pools[10].record_size = 4;
    set->pools[10].record_count = 1;
    set->pools[10].source_base = 32;
    set->pools[10].bytes = calloc(1, 4);
    wr16(set->pools[10].bytes + 0, mk_handle(14, 0));

    /* DB14 (8-byte): rec0 possession item (next END). */
    set->pools[14].record_size = 8;
    set->pools[14].record_count = 1;
    set->pools[14].source_base = 36;
    set->pools[14].bytes = calloc(1, 8);
    wr16(set->pools[14].bytes + 0, DM2_V1_RECORD_HANDLE_END);

    set->valid = 1;
}

static int flags_bit0_clear(int creature_type, uint16_t *out_flags)
{
    (void)creature_type;
    *out_flags = 0x0000;
    return 1;
}

static int flags_bit0_set(int creature_type, uint16_t *out_flags)
{
    (void)creature_type;
    *out_flags = 0x0001;
    return 1;
}

static int flags_from_context(void *context, int creature_type,
                              uint16_t *out_flags)
{
    (void)creature_type;
    if (!context || !out_flags) return 0;
    *out_flags = *(const uint16_t *)context;
    return 1;
}

/* Slot 0: (w & 0xf) = 1 -> base 2 items, no extra roll; itemspec
 * w >> 7 = 5 -> dbWeapon type 5.  All other slots empty. */
static const uint16_t k_drop_slots[DM2_DROP_SLOT_COUNT] = {
    (uint16_t)((5u << 7) | 1u), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

int main(void)
{
    static uint8_t raw[RAW_SIZE];
    DM2_V1_DungeonData dungeon;
    DM2_V1_RecordPoolSet set;
    DM2_V1_DropRng rng;

    /* ── full slice: generated drops + possession walk ───────────── */
    build_dungeon(&dungeon, raw);
    build_pools(&set);
    dm2_v1_drops_rng_init(&rng);
    {
        DM2_V1_DropPossessionReceipt rc;
        DM2_V1_DropPlacedItem items[4];
        memset(&rc, 0, sizeof(rc));
        memset(items, 0, sizeof(items));
        CHECK(dm2_v1_drop_creature_possession(
                  &set, &dungeon, 0, &rng, flags_bit0_clear,
                  mk_handle(4, 0), 0, 0, 0, 0, 0, 0, 1,
                  k_drop_slots, items, 4, &rc) == 1,
              "full drop runs to the source end");
        CHECK(rc.generated_drops_ran == 1 && rc.drops_placed == 2,
              "generated-drops loop placed both slot items");
        CHECK(rc.drops_iterations == 2, "per-item iterations receipted");
        CHECK(items[0].record != DM2_V1_RECORD_HANDLE_NULL &&
                  items[0].placed == 1,
              "first generated item allocated and placed");
        CHECK(rc.possession_walk_ran == 1 && rc.possession_items == 3,
              "possession walk visited all three chain records");
        CHECK(rc.possession_dropped == 2 && rc.possession_dealloced == 1,
              "DB != 0x0e dropped, DB 0x0e deallocated");
        CHECK(rc.dir_draws == 3,
              "AI bit0 clear randomized every item's direction");
        CHECK(rc.noise_would_queue == 2,
              "noise receipted for the dropped items only");
        CHECK(rd16(set.pools[14].bytes + 0) == DM2_V1_RECORD_HANDLE_NULL,
              "DB 0x0e record carries the 0xffff free marker");
        /* Ground chain: rec0 -> gen1 -> gen2 -> DB5 rec1 -> DB10 rec0,
         * each possession item's link word carrying direction bits. */
        {
            int16_t l0 = rd16(set.pools[5].bytes + 0);
            int16_t l1 = rd16(set.pools[5].bytes + 8);
            int16_t l2 = rd16(set.pools[5].bytes + 12);
            int16_t l3 = rd16(set.pools[5].bytes + 4);
            int16_t l4 = rd16(set.pools[10].bytes + 0);
            CHECK(dm2_v1_record_handle_pool(l0) == 5 &&
                      dm2_v1_record_handle_index(l0) == 2,
                  "ground item links to the first generated record");
            CHECK(dm2_v1_record_handle_pool(l1) == 5 &&
                      dm2_v1_record_handle_index(l1) == 3,
                  "first generated links to the second");
            CHECK(dm2_v1_record_handle_pool(l2) == 5 &&
                      dm2_v1_record_handle_index(l2) == 1,
                  "second generated links to the first possession item");
            CHECK(dm2_v1_record_handle_pool(l3) == 10 &&
                      dm2_v1_record_handle_index(l3) == 0,
                  "first possession item links to the second");
            CHECK(l4 == DM2_V1_RECORD_HANDLE_END,
                  "last possession item terminates the chain");
            CHECK((l3 & 0xC000) != 0 || (l2 & 0xC000) != 0 ||
                      (l0 & 0xC000) != 0 || (l1 & 0xC000) != 0,
                  "direction draws folded into the link words");
            (void)l4;
        }
        CHECK(dm2_v1_dungeon_get_first_thing(&dungeon, 0, 0, 0) ==
                  mk_handle(5, 0),
              "ground-stack head unchanged (chain was non-empty)");
    }
    dm2_v1_record_pool_set_free(&set);

    /* ── private AI-owner context does not use the global callback ───── */
    build_dungeon(&dungeon, raw);
    build_pools(&set);
    dm2_v1_drops_rng_init(&rng);
    {
        const uint16_t private_flags = 0x0001u;
        DM2_V1_DropPossessionReceipt rc;
        memset(&rc, 0, sizeof(rc));
        CHECK(dm2_v1_drop_creature_possession_with_context(
                  &set, &dungeon, 0, &rng, flags_from_context,
                  (void *)&private_flags, mk_handle(4, 0), 0, 0, 1, -1,
                  5, 5, 0, NULL, NULL, 0, &rc) == 1 &&
                  rc.ai_flags_known == 1 && rc.dir_draws == 0,
              "context-bound AI flags owner drives possession walk");
    }
    dm2_v1_record_pool_set_free(&set);

    /* ── malformed possession tail: preflight before generated drops ─── */
    build_dungeon(&dungeon, raw);
    build_pools(&set);
    wr16(set.pools[5].bytes + 4u, DM2_V1_RECORD_HANDLE_NULL);
    dm2_v1_drops_rng_init(&rng);
    {
        DM2_V1_DropRng rng_before = rng;
        DM2_V1_DropPossessionReceipt rc;
        memset(&rc, 0, sizeof(rc));
        CHECK(dm2_v1_drop_creature_possession(
                  &set, &dungeon, 0, &rng, flags_bit0_clear,
                  mk_handle(4, 0), 0, 0, 0, 0, 0, 0, 1,
                  k_drop_slots, NULL, 0, &rc) == 0 &&
                  rc.walk_corrupt == 1 &&
                  rc.generated_drops_ran == 0,
              "malformed possession tail fails before generated drops");
        CHECK(rng.random == rng_before.random &&
                  dm2_v1_dungeon_get_first_thing(&dungeon, 0, 0, 0) ==
                      mk_handle(5, 0) &&
                  rd16(set.pools[5].bytes + 8u) ==
                      DM2_V1_RECORD_HANDLE_NULL,
              "malformed possession tail leaves RNG, ground and pools intact");
    }
    dm2_v1_record_pool_set_free(&set);

    /* ── authenticated non-zero map: never fall back to map 0 ───── */
    build_dungeon(&dungeon, raw);
    build_pools(&set);
    dungeon.level_count = 2;
    dungeon.level_widths[1] = 2;
    dungeon.level_heights[1] = 2;
    dungeon.level_offsets[1] = 4;
    dungeon.square_first_thing_count = 4;
    raw[MAP_BASE + 4] = 0x10;
    raw[MAP_BASE + 5] = 0x10;
    wr16(raw + COLUMN_BASE + 4, 2);
    wr16(raw + COLUMN_BASE + 6, 4);
    wr16(raw + GROUND_BASE + 4, DM2_V1_RECORD_HANDLE_END);
    wr16(raw + GROUND_BASE + 6, DM2_V1_RECORD_HANDLE_END);
    dm2_v1_drops_rng_init(&rng);
    {
        DM2_V1_DropPossessionReceipt rc;
        memset(&rc, 0, sizeof(rc));
        CHECK(dm2_v1_drop_creature_possession(
                  &set, &dungeon, 1, &rng, flags_bit0_set,
                  mk_handle(4, 0), 0, 0, 0, -1, 5, 5, 0,
                  NULL, NULL, 0, &rc) == 1 &&
                  dm2_v1_dungeon_get_first_thing(&dungeon, 1, 0, 0) !=
                      mk_handle(5, 0) &&
                  dm2_v1_dungeon_get_first_thing(&dungeon, 0, 0, 0) ==
                      mk_handle(5, 0),
              "drop possession writes the authenticated non-zero map only");
    }
    dm2_v1_record_pool_set_free(&set);

    /* ── AI bit0 set: no direction draws ─────────────────────────── */
    build_dungeon(&dungeon, raw);
    build_pools(&set);
    dm2_v1_drops_rng_init(&rng);
    {
        DM2_V1_DropPossessionReceipt rc;
        memset(&rc, 0, sizeof(rc));
        CHECK(dm2_v1_drop_creature_possession(
                  &set, &dungeon, 0, &rng, flags_bit0_set,
                  mk_handle(4, 0), 0, 0, 0, -1, 5, 5, 0,
                  NULL, NULL, 0, &rc) == 1,
              "bit0-set walk runs without draws");
        CHECK(rc.drop_slots_unloaded == 1,
              "unloaded GDAT drop fields receipted, generated part skipped");
        CHECK(rc.dir_draws == 0 && rc.possession_dropped == 2 &&
                  rc.possession_dealloced == 1,
              "bit0 set keeps the handles, drops unchanged");
        CHECK(rc.noise_would_queue == 0,
              "negative noise arg suppresses the noise receipt");
        CHECK(rd16(set.pools[5].bytes + 4) == mk_handle(10, 0) ||
              rd16(set.pools[5].bytes + 4) == DM2_V1_RECORD_HANDLE_END,
              "possession link spliced without direction bits");
    }
    dm2_v1_record_pool_set_free(&set);

    /* ── empty drop cell: the generated head is written back ─────── */
    build_dungeon(&dungeon, raw);
    build_pools(&set);
    set.pools[4].bytes[2] = 0xFE; /* word@2 = END: empty possession */
    set.pools[4].bytes[3] = 0xFF;
    dm2_v1_drops_rng_init(&rng);
    {
        DM2_V1_DropPossessionReceipt rc;
        memset(&rc, 0, sizeof(rc));
        CHECK(dm2_v1_drop_creature_possession(
                  &set, &dungeon, 0, &rng, flags_bit0_clear,
                  mk_handle(4, 0), 0, 1, 0, 0, 9, 9, 2,
                  k_drop_slots, NULL, 0, &rc) == 1,
              "generated drops onto an empty cell chain complete");
        CHECK(rc.drops_placed == 2 && rc.possession_walk_ran == 0,
              "empty possession chain: generated part only");
        {
            int head = dm2_v1_dungeon_get_first_thing(&dungeon, 0, 0, 1);
            CHECK(dm2_v1_record_handle_pool((int16_t)head) == 5 &&
                      dm2_v1_record_handle_index((int16_t)head) == 2,
                  "ground-stack head rewritten to the first generated item");
        }
    }
    dm2_v1_record_pool_set_free(&set);

    /* ── mode 2: source immediate return ─────────────────────────── */
    build_dungeon(&dungeon, raw);
    build_pools(&set);
    {
        DM2_V1_DropPossessionReceipt rc;
        memset(&rc, 0, sizeof(rc));
        CHECK(dm2_v1_drop_creature_possession(
                  &set, &dungeon, 0, NULL, NULL,
                  mk_handle(4, 0), 0, 0, 2, 0, 0, 0, 0,
                  k_drop_slots, NULL, 0, &rc) == 1,
              "mode 2 returns immediately");
        CHECK(rc.mode_return == 1 && rc.generated_drops_ran == 0 &&
                  rc.possession_walk_ran == 0,
              "nothing ran before the source return");
        CHECK(rd16(set.pools[5].bytes + 0) == DM2_V1_RECORD_HANDLE_END,
              "no mutation on the mode-2 path");
    }
    dm2_v1_record_pool_set_free(&set);

    /* ── fail-closed paths ───────────────────────────────────────── */
    build_dungeon(&dungeon, raw);
    build_pools(&set);
    {
        DM2_V1_DropPossessionReceipt rc;
        memset(&rc, 0, sizeof(rc));
        CHECK(dm2_v1_drop_creature_possession(
                  &set, &dungeon, 0, &rng, flags_bit0_clear,
                  mk_handle(4, 0), 1, 1, 0, 0, 0, 0, 0,
                  k_drop_slots, NULL, 0, &rc) == 0,
              "flag-less drop cell fails closed");
        CHECK(rc.drops_cell_unrooted == 1, "unrooted cell receipted");

        memset(&rc, 0, sizeof(rc));
        CHECK(dm2_v1_drop_creature_possession(
                  &set, &dungeon, 0, NULL, flags_bit0_clear,
                  mk_handle(4, 0), 0, 0, 0, 0, 0, 0, 0,
                  k_drop_slots, NULL, 0, &rc) == 0,
              "NULL rng fails closed before any mutation");
        CHECK(rc.rng_unbound == 1, "rng_unbound receipted");

        memset(&rc, 0, sizeof(rc));
        dm2_v1_drops_rng_init(&rng);
        CHECK(dm2_v1_drop_creature_possession(
                  &set, &dungeon, 0, &rng, NULL,
                  mk_handle(4, 0), 0, 0, 1, 0, 0, 0, 0,
                  NULL, NULL, 0, &rc) == 0,
              "unwired AI flags fail closed before the first draw");
        CHECK(rc.ai_flags_unknown == 1, "ai_flags_unknown receipted");
        CHECK(rd16(set.pools[5].bytes + 0) == DM2_V1_RECORD_HANDLE_END,
              "no mutation when the walk cannot start");
    }
    dm2_v1_record_pool_set_free(&set);

    CHECK(strstr(dm2_v1_drop_possession_source_evidence(),
                 "c_record.cpp") != NULL,
          "source evidence cites c_record.cpp");

    if (g_failures != 0) {
        fprintf(stderr,
                "dm2_v1_drop_possession_pc34_compat: %d failure(s)\n",
                g_failures);
        return 1;
    }
    printf("dm2_v1_drop_possession_pc34_compat: all checks passed\n");
    return 0;
}
