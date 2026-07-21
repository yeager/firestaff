/*
 * test_dm2_v1_caii_free_runtime_pc34_compat.c — CAII slot free wired
 * into the runtime (DM2-003/005 follow-up).
 *
 * Verifies the full slot lifecycle over the runtime boundaries:
 *   activation (alloc + first timer)
 *     -> free (DM2_1c9a_0fcb bounded slice: pending timer cancelled
 *        through bound DM2_1c9a_0db0, record byte@5 cleared, slot
 *        marked free)
 *     -> next tick dispatches NOTHING for the freed creature
 *     -> re-activation reuses the freed slot and the think chain
 *        resumes end-to-end.
 */

#include "dm2_v1_runtime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dm2_v1_creature.h"
#include "dm2_v1_delete_creature_full_pc34_compat.h"

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { \
        passed++; \
        printf("  PASS: %s\n", msg); \
    } else { \
        failed++; \
        printf("  FAIL: %s\n", msg); \
    } \
} while (0)

#define MAP_BASE 0
#define COLUMN_BASE 64
#define GROUND_BASE 128
#define TEXT_BASE 256
#define DB4_BASE 256
#define G1_EXT_BASE (DB4_BASE + 2 * 16)
#define RAW_SIZE 512

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
    d->text_data_base = TEXT_BASE;
    d->text_word_count = 0;
    d->g1_extension_base = G1_EXT_BASE;
    d->thing_type_counts[4] = 2;
    d->raw_data = raw;
    d->raw_size = RAW_SIZE;

    raw[MAP_BASE + 0] = 0x10; /* (0,0) */
    raw[MAP_BASE + 1] = 0x10; /* (0,1) */

    wr16(raw + COLUMN_BASE + 0, 0);
    wr16(raw + COLUMN_BASE + 2, 2);

    wr16(raw + GROUND_BASE + 0,
         (int16_t)((2u << 14) | (4u << 10) | 0u));
    wr16(raw + GROUND_BASE + 2, DM2_V1_RECORD_HANDLE_END);

    wr16(raw + DB4_BASE + 0, DM2_V1_RECORD_HANDLE_END);
    raw[DB4_BASE + 4] = 0x0C;
    raw[DB4_BASE + 5] = 0xFF;
    wr16(raw + DB4_BASE + 2, DM2_V1_RECORD_HANDLE_END); /* no possession */
    wr16(raw + DB4_BASE + 8, (int16_t)0xffff);
    /* word@0xc packed message coords for the DELETE_CREATURE_RECORD
     * invoke branch: map 0, y 1, x 1 (c_record.cpp:1389-1405). */
    wr16(raw + DB4_BASE + 0xc, 0x21);
    wr16(raw + DB4_BASE + 16, DM2_V1_RECORD_HANDLE_END);
}

static void test_runtime_caii_slot_lifecycle(void)
{
    static uint8_t raw[RAW_SIZE];
    DM2_V1_DungeonData dungeon;
    DM2_V1_BootProfile boot = {0};
    DM2_V1_CaiiAllocReceipt alloc;
    DM2_V1_CaiiFreeReceipt free_rc;
    DM2_V1_ThinkCreatureReceipt think;

    build_dungeon(&dungeon, raw);
    boot.dungeon_data = &dungeon;

    dm2_v1_runtime_init(&boot);
    dm2_v1_runtime_set_outdoor(0);
    (void)dm2_v1_runtime_caii_init(4);

    /* Free without an allocation fails closed. */
    memset(&free_rc, 0, sizeof(free_rc));
    CHECK(dm2_v1_runtime_free_caii_slot(0, &free_rc) == 0 &&
              free_rc.already_free == 1,
          "free before activation takes the source early return");

    /* Activate, then free before the timer fires. */
    memset(&alloc, 0, sizeof(alloc));
    CHECK(dm2_v1_runtime_alloc_caii_at(0, 0, &alloc) == 1 &&
              alloc.slot_index == 0,
          "activation allocates slot 0 + first timer");

    memset(&free_rc, 0, sizeof(free_rc));
    CHECK(dm2_v1_runtime_free_caii_slot(0, &free_rc) == 1,
          "runtime boundary frees the slot");
    CHECK(free_rc.freed == 1 && free_rc.had_pending_timer == 1 &&
              free_rc.record_delete_unbound == 1,
          "free receipt: pending timer cancelled, record branch unbound");
    CHECK(dm2_v1_runtime_caii_alloc_count() == 0,
          "alloc counter back to zero");

    /* Next tick: nothing dispatches for the freed creature. */
    dm2_v1_runtime_tick();
    CHECK(dm2_v1_runtime_think_creature_receipt(&think) == 1 &&
              think.think_timers == 0,
          "freed creature's timer never reaches the think binding");

    /* Re-activation reuses the freed slot; the chain resumes. */
    memset(&alloc, 0, sizeof(alloc));
    CHECK(dm2_v1_runtime_alloc_caii_at(0, 0, &alloc) == 1 &&
              alloc.slot_index == 0,
          "re-activation reuses the freed slot");
    dm2_v1_runtime_tick();
    CHECK(dm2_v1_runtime_think_creature_receipt(&think) == 1 &&
              think.think_timers == 1 && think.resolved == 1 &&
              think.last_creature_type == 0x0C,
          "think chain resumes end-to-end after reuse");
}

static void test_runtime_caii_free_fail_closed_without_session(void)
{
    DM2_V1_BootProfile boot = {0};
    DM2_V1_CaiiFreeReceipt free_rc;

    dm2_v1_runtime_init(&boot);
    dm2_v1_runtime_set_outdoor(0);

    memset(&free_rc, 0, sizeof(free_rc));
    CHECK(dm2_v1_runtime_free_caii_slot(0, &free_rc) == 0,
          "no CAII session: free boundary stays unready");
}

/* ── 0fcb branch wired to the COMPLETE DELETE_CREATURE_RECORD ───────
 * Session context for the wired full-composition hook: the source call
 * is DM2_DELETE_CREATURE_RECORD(x, y, 0, 1) (c_1c9a.cpp:5956-5957). */
typedef struct {
    DM2_V1_DropRng rng;
    unsigned long game_tick;
    DM2_V1_DeleteCreatureFullReceipt full;
} WiredDeleteCtx;

static int wired_delete_full(DM2_V1_RecordPoolSet *pool_set,
                             DM2_V1_DungeonData *dungeon,
                             DM2_V1_CaiiArray *caii,
                             DM2_V1_SourceTimerQueue *queue,
                             int x, int y, void *context)
{
    WiredDeleteCtx *ctx = (WiredDeleteCtx *)context;

    memset(&ctx->full, 0, sizeof(ctx->full));
    return dm2_v1_delete_creature_record_full(
        pool_set, dungeon, caii, queue, &ctx->rng,
        0, ctx->game_tick, x, y, 0, 1, 0, 0, 1, NULL, &ctx->full);
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

static void test_runtime_caii_free_wired_full_composition(void)
{
    static uint8_t raw[RAW_SIZE];
    DM2_V1_GdatEntry entries[4];
    uint32_t raw_offsets[1] = { 0 };
    uint32_t raw_sizes[1] = { 0 };
    uint8_t raw_data[1] = { 0 };
    DM2_V1_AssetLoader loader;
    DM2_V1_DungeonData dungeon;
    DM2_V1_BootProfile boot = {0};
    DM2_V1_CaiiAllocReceipt alloc;
    DM2_V1_CaiiFreeReceipt free_rc;
    DM2_V1_ThinkCreatureReceipt think;
    WiredDeleteCtx ctx;

    /* Synthetic GDAT session: type 0x0C -> AI row 5 with flags 0x0000
     * (bit0 clear) + CREATURES word@1 = 0 (table1d607e uc0 0x00 — the
     * invoke-message branch runs, c_record.cpp:1387-1388). */
    set_word_entry(&entries[0], DM2_GDAT_CATEGORY_CREATURES, 0x0C, 0x05, 5);
    set_word_entry(&entries[1], DM2_GDAT_CATEGORY_CREATURE_AI, 5, 0, 0x00);
    set_word_entry(&entries[2], DM2_GDAT_CATEGORY_CREATURE_AI, 5, 4, 40);
    set_word_entry(&entries[3], DM2_GDAT_CATEGORY_CREATURES, 0x0C, 0x01, 0);
    memset(&loader, 0, sizeof(loader));
    loader.data = raw_data;
    loader.data_size = 1;
    loader.loaded = 1;
    loader.raw_data_count = 1;
    loader.raw_offsets = raw_offsets;
    loader.raw_sizes = raw_sizes;
    loader.entries = entries;
    loader.entry_count = 4;
    CHECK(dm2_v1_creature_load_ai_table_from_gdat(&loader) == 1,
          "synthetic GDAT session resolves the type-0x0C AI row");

    build_dungeon(&dungeon, raw);
    boot.dungeon_data = &dungeon;
    dm2_v1_runtime_init(&boot);
    dm2_v1_runtime_set_outdoor(0);
    (void)dm2_v1_runtime_caii_init(4);

    memset(&ctx, 0, sizeof(ctx));
    dm2_v1_drops_rng_init(&ctx.rng);
    ctx.game_tick = 1000ul;
    dm2_v1_caii_set_delete_creature_full_fn(wired_delete_full, &ctx);

    memset(&alloc, 0, sizeof(alloc));
    CHECK(dm2_v1_runtime_alloc_caii_at(0, 0, &alloc) == 1 &&
              alloc.slot_index == 0,
          "activation allocates slot 0 + first timer");
    CHECK(dm2_v1_runtime_caii_set_slot_mode_byte(0, 0x13) == 1,
          "slot mode byte set to the 0x13 dying mode");

    memset(&free_rc, 0, sizeof(free_rc));
    CHECK(dm2_v1_runtime_free_caii_slot(0, &free_rc) == 1 &&
              free_rc.record_delete_flag == 1 &&
              free_rc.record_delete_branch == 1,
          "flag + pending timer takes the 0fcb branch data-backed");
    CHECK(free_rc.record_delete_full_ran == 1 &&
              free_rc.record_delete_full_completed == 1 &&
              free_rc.record_delete_head_resolved == 0,
          "the wired COMPLETE composition ran instead of the head");
    CHECK(ctx.full.completed == 1 &&
              ctx.full.creature_type == 0x0C &&
              ctx.full.ai_bit0_clear == 1,
          "composition resolved the creature and opened the gate");
    CHECK(ctx.full.invoke_message_queued == 1 &&
              ctx.full.invoke_ticket > 0u,
          "map-swap/DM2_INVOKE_MESSAGE queued through the boundary");
    CHECK(ctx.full.cut_performed == 1 &&
              ctx.full.cut_head_rewritten == 1 &&
              ctx.full.drop_ran == 1 &&
              ctx.full.dballoc_cleanup_unbound == 1 &&
              ctx.full.dealloc_performed == 1,
          "cut + drop + dealloc bound end-to-end, 0247 receipted");

    /* The freed slot leaves only the queued invoke timer; the next tick
     * dispatches no think timer for the deleted creature. */
    dm2_v1_runtime_tick();
    CHECK(dm2_v1_runtime_think_creature_receipt(&think) == 1 &&
              think.think_timers == 0,
          "no think timer dispatches after the full delete");

    dm2_v1_caii_set_delete_creature_full_fn(0, 0);
    dm2_v1_creature_reset_ai_table();
}

int main(void)
{
    printf("DM2 V1 CAII slot free runtime wiring\n");
    printf("Source: skproject/SKULLWIN/c_1c9a.cpp:5896-5944 (DM2_1c9a_0fcb)\n");
    printf("        skproject/SKULLWIN/c_1c9a.cpp:5734-5763 (DM2_1c9a_0db0)\n\n");

    test_runtime_caii_slot_lifecycle();
    test_runtime_caii_free_fail_closed_without_session();
    test_runtime_caii_free_wired_full_composition();

    printf("\n%d passed, %d failed\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
