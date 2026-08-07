/*
 * test_dm2_v1_record_pool_pc34_compat.c — DM2-002 c_record pool ownership.
 *
 * Verifies the source-ordered record-pool layer against skproject anchors:
 *   c_record.cpp:28-31  table_recordsizes
 *   c_record.cpp:44-52  DM2_GET_ADDRESS_OF_RECORD decode
 *   c_record.cpp:54-57  DM2_GET_NEXT_RECORD_LINK
 *   c_record.cpp:60-170 APPEND/CUT list paths
 *   c_moverec.cpp       DM2_MOVE_RECORD_TO relocation boundary
 */

#include "dm2_v1_record_pool_pc34_compat.h"

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

static void wr16(uint8_t *p, int16_t v)
{
    uint16_t u = (uint16_t)v;
    p[0] = (uint8_t)(u & 0xffu);
    p[1] = (uint8_t)((u >> 8) & 0xffu);
}

/* Build a synthetic pool set: DB0 (4-byte records) x3, DB3 (8-byte) x2. */
static void build_synthetic(DM2_V1_RecordPoolSet *set)
{
    memset(set, 0, sizeof(*set));
    set->pools[0].record_size = 4;
    set->pools[0].record_count = 3;
    set->pools[0].source_base = 0;
    set->pools[0].bytes = calloc(3, 4);
    set->pools[3].record_size = 8;
    set->pools[3].record_count = 2;
    set->pools[3].source_base = 12;
    set->pools[3].bytes = calloc(2, 8);
    set->pools[3].extension_count = 2;
    set->pools[3].extension_base = 28;
    set->pools[3].extension_bytes = calloc(2, 8);
    set->valid = 1;
}

static int16_t mk_handle(int pool, int index)
{
    return (int16_t)((pool << 10) | (index & 0x3ff));
}

int main(void)
{
    DM2_V1_RecordPoolSet set;
    int16_t next;
    int16_t head_a;
    int16_t head_b;

    /* ── table_recordsizes (c_record.cpp:28-31) ─────────────────── */
    {
        static const int expect[16] = {
            4, 6, 4, 8, 16, 4, 4, 4, 4, 8, 4, 0, 0, 0, 8, 4
        };
        for (int i = 0; i < 16; ++i) {
            CHECK(dm2_v1_record_pool_record_size(i) == expect[i],
                  "record size table matches skproject table_recordsizes");
        }
        CHECK(dm2_v1_record_pool_record_size(-1) == 0, "pool -1 size 0");
        CHECK(dm2_v1_record_pool_record_size(16) == 0, "pool 16 size 0");
    }

    /* ── handle decode (c_record.cpp:44-52) ─────────────────────── */
    CHECK(dm2_v1_record_handle_pool(mk_handle(4, 173)) == 4, "pool decode");
    CHECK(dm2_v1_record_handle_index(mk_handle(4, 173)) == 173,
          "index decode");
    /* Direction bits 14-15 shift out exactly like the source. */
    CHECK(dm2_v1_record_handle_pool((int16_t)(0xC000u | (4u << 10) | 7u)) == 4,
          "direction bits masked from pool");
    CHECK(dm2_v1_record_handle_index((int16_t)(0xC000u | (4u << 10) | 7u)) == 7,
          "direction bits masked from index");

    /* ── address / link semantics on synthetic pools ────────────── */
    build_synthetic(&set);
    CHECK(dm2_v1_record_pool_address(&set, mk_handle(0, 0)) == set.pools[0].bytes,
          "DB0 record 0 address");
    CHECK(dm2_v1_record_pool_address(&set, mk_handle(0, 2)) ==
              set.pools[0].bytes + 8,
          "DB0 record 2 address (ofs = size * index)");
    CHECK(dm2_v1_record_pool_address(&set, mk_handle(0, 3)) == NULL,
          "out-of-range index fails closed");
    CHECK(dm2_v1_record_pool_address(&set, mk_handle(3, 2)) ==
              set.pools[3].extension_bytes,
          "G1 continuation starts at the declared pool count");
    CHECK(dm2_v1_record_pool_address(&set, mk_handle(3, 3)) ==
              set.pools[3].extension_bytes + 8,
          "G1 continuation preserves record stride");
    CHECK(dm2_v1_record_pool_address(&set, mk_handle(3, 4)) == NULL,
          "G1 continuation bound fails closed");
    CHECK(dm2_v1_record_pool_address(&set, mk_handle(11, 0)) == NULL,
          "zero-sized DB11 fails closed");
    CHECK(dm2_v1_record_pool_address(&set, mk_handle(1, 0)) == NULL,
          "absent DB1 pool fails closed");
    CHECK(dm2_v1_record_pool_address(&set, DM2_V1_RECORD_HANDLE_NULL) == NULL,
          "OBJECT_NULL fails closed");
    CHECK(dm2_v1_record_pool_address(&set, DM2_V1_RECORD_HANDLE_END) == NULL,
          "OBJECT_END_MARKER fails closed");

    /* Link walk: chain 0 -> 1 -> END within DB0. */
    wr16(set.pools[0].bytes + 0, mk_handle(0, 1));
    wr16(set.pools[0].bytes + 4, DM2_V1_RECORD_HANDLE_END);
    CHECK(dm2_v1_record_pool_next_link(&set, mk_handle(0, 0), &next) &&
              next == mk_handle(0, 1),
          "next link follows first word");
    CHECK(dm2_v1_record_pool_next_link(&set, mk_handle(0, 1), &next) &&
              next == DM2_V1_RECORD_HANDLE_END,
          "chain terminates at OBJECT_END_MARKER");
    CHECK(!dm2_v1_record_pool_next_link(&set, mk_handle(11, 0), &next),
          "next link on zero-sized pool fails closed");

    /* ── APPEND/CUT list paths (c_record.cpp:60-170) ────────────── */
    head_a = DM2_V1_RECORD_HANDLE_END;
    head_b = DM2_V1_RECORD_HANDLE_END;
    CHECK(dm2_v1_record_pool_append_to_list(&set, &head_a, mk_handle(3, 0)),
          "append into empty list");
    CHECK(head_a == mk_handle(3, 0), "list head updated");
    CHECK(dm2_v1_record_pool_next_link(&set, mk_handle(3, 0), &next) &&
              next == DM2_V1_RECORD_HANDLE_END,
          "appended record terminates");
    CHECK(dm2_v1_record_pool_append_to_list(&set, &head_a, mk_handle(3, 1)),
          "append second record");
    CHECK(dm2_v1_record_pool_next_link(&set, mk_handle(3, 0), &next) &&
              next == mk_handle(3, 1),
          "append chains at list end");
    CHECK(!dm2_v1_record_pool_append_to_list(&set, &head_a,
                                             DM2_V1_RECORD_HANDLE_NULL),
          "append of OBJECT_NULL rejected");
    CHECK(!dm2_v1_record_pool_append_to_list(&set, &head_a, mk_handle(0, 9)),
          "append of unresolvable record rejected");

    /* Relocate record (3,0) from list A to list B (c_moverec.cpp). */
    CHECK(dm2_v1_record_pool_relocate(&set, &head_a, &head_b, mk_handle(3, 0)),
          "relocate across lists");
    CHECK(head_a == mk_handle(3, 1), "source list spliced");
    CHECK(head_b == mk_handle(3, 0), "destination list owns record");
    CHECK(dm2_v1_record_pool_next_link(&set, mk_handle(3, 0), &next) &&
              next == DM2_V1_RECORD_HANDLE_END,
          "relocated record terminates destination");

    /* Cut the remaining record from list A. */
    CHECK(dm2_v1_record_pool_cut_from_list(&set, &head_a, mk_handle(3, 1)),
          "cut last record");
    CHECK(head_a == DM2_V1_RECORD_HANDLE_END, "list head becomes END");
    CHECK(!dm2_v1_record_pool_cut_from_list(&set, &head_a, mk_handle(3, 1)),
          "cut of absent record fails");

    dm2_v1_record_pool_set_free(&set);
    CHECK(set.valid == 0 && set.pools[0].bytes == NULL,
          "free releases owned pools");

    /* init_from_world without a verified world fails closed. */
    memset(&set, 0, sizeof(set));
    CHECK(!dm2_v1_record_pool_set_init_from_world(&set, NULL),
          "init from NULL world fails closed");

    CHECK(strstr(dm2_v1_record_pool_source_evidence(), "c_record.cpp") != NULL,
          "source evidence cites c_record.cpp");

    if (g_failures != 0) {
        fprintf(stderr, "dm2_v1_record_pool_pc34_compat: %d failure(s)\n",
                g_failures);
        return 1;
    }
    printf("dm2_v1_record_pool_pc34_compat: all checks passed\n");
    return 0;
}
