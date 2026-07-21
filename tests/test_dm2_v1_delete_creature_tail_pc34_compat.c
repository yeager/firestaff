/*
 * test_dm2_v1_delete_creature_tail_pc34_compat.c — the
 * DM2_DELETE_CREATURE_RECORD mutating tail over the DM2-002 record
 * pool.
 *
 * Verifies the skproject boundary:
 *   c_record.cpp:1419        tile-rooted cut (the DM2_MOVE_RECORD_TO
 *                            x == -4 skip00823 path, c_moverec.cpp:630-683
 *                            observable end state): mid-chain splice and
 *                            head rewrite into the dungeon ground-stack
 *                            table via dm2_v1_dungeon_set_first_thing
 *   c_record.cpp:1205-1208   DM2_DEALLOC_RECORD: record word@0 = 0xffff
 *   c_record.cpp:1422-1423   DROP_CREATURE_POSSESSION and the 1c9a_0247
 *                            tagged-dballoc cleanup stay receipted
 *                            unbound, never simulated
 *   corrupt chains are bounded (membership pre-walk), a record not
 *   chained on the tile fails closed (cut_miss)
 */

#include "dm2_v1_caii_alloc_pc34_compat.h"

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

/* 2x2 byte-square map.  Flagged cells: (0,0) -> idx 0, (0,1) -> idx 1,
 * (1,1) -> idx 2.
 * idx 0: DB5 rec0 -> DB4 rec0 -> END   (mid-chain creature)
 * idx 1: DB4 rec1 -> DB5 rec1 -> END   (head creature)
 * idx 2: DB3 rec0 -> itself            (corrupt self-loop) */
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
    d->square_first_thing_count = 3;
    d->raw_data = raw;
    d->raw_size = RAW_SIZE;

    raw[MAP_BASE + 0] = 0x10; /* (0,0) */
    raw[MAP_BASE + 1] = 0x10; /* (0,1) */
    raw[MAP_BASE + 3] = 0x10; /* (1,1) */
    wr16(raw + COLUMN_BASE + 0, 0); /* column 0 base */
    wr16(raw + COLUMN_BASE + 2, 2); /* column 1 base */

    wr16(raw + GROUND_BASE + 0, mk_handle(5, 0));
    wr16(raw + GROUND_BASE + 2, mk_handle(4, 1));
    wr16(raw + GROUND_BASE + 4, mk_handle(3, 0));
}

static void build_pools(DM2_V1_RecordPoolSet *set)
{
    memset(set, 0, sizeof(*set));

    /* DB3 (4-byte): rec0 loops onto itself. */
    set->pools[3].record_size = 4;
    set->pools[3].record_count = 1;
    set->pools[3].source_base = 0;
    set->pools[3].bytes = calloc(1, 4);
    wr16(set->pools[3].bytes + 0, mk_handle(3, 0));

    /* DB4 creatures (16-byte): rec0 terminates, rec1 chains to
     * DB5 rec1; neither owns a CAII slot (byte@5 = 0xff). */
    set->pools[4].record_size = 16;
    set->pools[4].record_count = 2;
    set->pools[4].source_base = 4;
    set->pools[4].bytes = calloc(2, 16);
    wr16(set->pools[4].bytes + 0, DM2_V1_RECORD_HANDLE_END);
    set->pools[4].bytes[4] = 0x0C;
    set->pools[4].bytes[5] = 0xFF;
    wr16(set->pools[4].bytes + 16, mk_handle(5, 1));
    set->pools[4].bytes[16 + 4] = 0x0C;
    set->pools[4].bytes[16 + 5] = 0xFF;

    /* DB5 weapons (4-byte): rec0 -> DB4 rec0, rec1 terminates. */
    set->pools[5].record_size = 4;
    set->pools[5].record_count = 2;
    set->pools[5].source_base = 36;
    set->pools[5].bytes = calloc(2, 4);
    wr16(set->pools[5].bytes + 0, mk_handle(4, 0));
    wr16(set->pools[5].bytes + 4, DM2_V1_RECORD_HANDLE_END);

    set->valid = 1;
}

int main(void)
{
    static uint8_t raw[RAW_SIZE];
    DM2_V1_DungeonData dungeon;
    DM2_V1_RecordPoolSet set;

    build_dungeon(&dungeon, raw);
    build_pools(&set);

    /* ── dm2_v1_dungeon_set_first_thing unit discipline ──────────── */
    CHECK(dm2_v1_dungeon_set_first_thing(&dungeon, 0, 0, 0,
                                         (uint16_t)mk_handle(5, 0)) == 0,
          "setter accepts a flagged byte-square cell");
    CHECK(dm2_v1_dungeon_set_first_thing(&dungeon, 0, 1, 0,
                                         (uint16_t)mk_handle(4, 0)) == -1,
          "setter rejects a cell without the 0x10 flag");
    CHECK(dm2_v1_dungeon_set_first_thing(&dungeon, 0, 9, 0, 0) == -1,
          "setter rejects an out-of-bounds cell");
    CHECK(dm2_v1_dungeon_set_first_thing(NULL, 0, 0, 0, 0) == -1,
          "setter fails closed on NULL data");

    /* ── head + tail composition: the mid-chain cut ──────────────── */
    {
        DM2_V1_CaiiDeleteCreatureRecordReceipt head_rc;
        DM2_V1_CaiiDeleteCreatureTailReceipt tail_rc;
        memset(&head_rc, 0, sizeof(head_rc));
        memset(&tail_rc, 0, sizeof(tail_rc));

        CHECK(dm2_v1_caii_delete_creature_record_head(
                  &set, &dungeon, NULL, 0, 0, &head_rc) == 1,
              "decision head resolves the creature at the cell");
        CHECK(head_rc.record_handle == mk_handle(4, 0),
              "head receipts the resolved record");

        CHECK(dm2_v1_caii_delete_creature_record_tail(
                  &set, &dungeon, head_rc.record_handle, 0, 0,
                  &tail_rc) == 1,
              "tail runs to the source end");
        CHECK(tail_rc.cut_performed == 1 && tail_rc.cut_head_rewritten == 0,
              "mid-chain cut splices the predecessor, head untouched");
        CHECK(rd16(set.pools[5].bytes + 0) == DM2_V1_RECORD_HANDLE_END,
              "DB5 predecessor link spliced to the successor");
        CHECK(dm2_v1_dungeon_get_first_thing(&dungeon, 0, 0, 0) ==
                  mk_handle(5, 0),
              "ground-stack head still roots the remaining chain");
        CHECK(rd16(set.pools[4].bytes + 0) == DM2_V1_RECORD_HANDLE_NULL,
              "DEALLOC_RECORD wrote the 0xffff free marker");
        CHECK(tail_rc.dealloc_performed == 1, "dealloc receipted");
        CHECK(tail_rc.cut_side_effects_unbound == 1 &&
                  tail_rc.drop_possession_unbound == 1 &&
                  tail_rc.dballoc_cleanup_unbound == 1,
              "3CE7D side effects, possession drop and dballoc cleanup "
              "receipted unbound, never simulated");
    }

    /* ── the head cut: ground-stack word rewritten ───────────────── */
    {
        DM2_V1_CaiiDeleteCreatureTailReceipt tail_rc;
        memset(&tail_rc, 0, sizeof(tail_rc));
        CHECK(dm2_v1_caii_delete_creature_record_tail(
                  &set, &dungeon, mk_handle(4, 1), 0, 1, &tail_rc) == 1,
              "head cut runs to the source end");
        CHECK(tail_rc.cut_performed == 1 && tail_rc.cut_head_rewritten == 1,
              "head cut rewrites the ground-stack word");
        CHECK(dm2_v1_dungeon_get_first_thing(&dungeon, 0, 0, 1) ==
                  mk_handle(5, 1),
              "chain head is now the successor record");
        CHECK(rd16(set.pools[4].bytes + 16) == DM2_V1_RECORD_HANDLE_NULL,
              "second creature deallocated");
    }

    /* ── fail-closed paths ───────────────────────────────────────── */
    {
        DM2_V1_CaiiDeleteCreatureTailReceipt tail_rc;
        memset(&tail_rc, 0, sizeof(tail_rc));
        CHECK(dm2_v1_caii_delete_creature_record_tail(
                  &set, &dungeon, mk_handle(4, 0), 0, 0, &tail_rc) == 0,
              "re-cutting a record no longer chained fails closed");
        CHECK(tail_rc.cut_miss == 1, "cut_miss receipted");

        memset(&tail_rc, 0, sizeof(tail_rc));
        CHECK(dm2_v1_caii_delete_creature_record_tail(
                  &set, &dungeon, mk_handle(5, 0), 1, 1, &tail_rc) == 0,
              "corrupt self-loop chain is bounded, fails closed");
        CHECK(tail_rc.cut_miss == 1, "bounded pre-walk receipted");

        memset(&tail_rc, 0, sizeof(tail_rc));
        CHECK(dm2_v1_caii_delete_creature_record_tail(
                  &set, &dungeon, mk_handle(4, 9), 0, 0, &tail_rc) == 0,
              "unresolvable record fails closed");

        memset(&tail_rc, 0, sizeof(tail_rc));
        CHECK(dm2_v1_caii_delete_creature_record_tail(
                  &set, &dungeon, DM2_V1_RECORD_HANDLE_END, 0, 0,
                  &tail_rc) == 0,
              "end-marker handle fails closed");
    }

    {
        DM2_V1_CaiiDeleteCreatureTailReceipt tail_rc;
        memset(&tail_rc, 0, sizeof(tail_rc));
        (void)dm2_v1_caii_delete_creature_record_tail(
            &set, &dungeon, mk_handle(5, 1), 0, 1, &tail_rc);
        CHECK(strstr(tail_rc.source_evidence, "c_record.cpp") != NULL,
              "source evidence cites c_record.cpp");
    }

    dm2_v1_record_pool_set_free(&set);

    if (g_failures != 0) {
        fprintf(stderr,
                "dm2_v1_delete_creature_tail_pc34_compat: %d failure(s)\n",
                g_failures);
        return 1;
    }
    printf("dm2_v1_delete_creature_tail_pc34_compat: all checks passed\n");
    return 0;
}
