/*
 * test_dm2_v1_dballoc_pc34_compat.c — Tests for DM2 database allocation module.
 *
 * Covers: CPX linked list, preserved GFX binary search, W-table lookup,
 * SET_PPPW_ENTRY, GFX table compaction, CPX heap allocator, ID generator.
 */

#include "dm2_v1_dballoc_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) static void name(void)
#define RUN(name) do { \
    printf("  %-60s", #name); \
    name(); \
    printf(" PASS\n"); \
    tests_passed++; \
} while (0)
#define ASSERT(cond) do { \
    if (!(cond)) { \
        printf(" FAIL [%s:%d] %s\n", __FILE__, __LINE__, #cond); \
        tests_failed++; \
        return; \
    } \
} while (0)

/* ========================================================================
 * CPX linked list tests
 * ======================================================================== */

TEST(test_cpx_linklist_init)
{
    DM2_V1_CpxLinklist list;
    dm2_v1_dballoc_cpx_linklist_init(&list);
    ASSERT(dm2_v1_dballoc_cpx_linklist_is_empty(&list));
    ASSERT(list.ptrlist == NULL);
}

TEST(test_cpx_link_single_node)
{
    DM2_V1_CpxLinklist list;
    dm2_v1_dballoc_cpx_linklist_init(&list);

    DM2_V1_DballocLpp node;
    node.amount = 100;
    dm2_v1_dballoc_cpx_link_node(&list, &node);

    ASSERT(!dm2_v1_dballoc_cpx_linklist_is_empty(&list));
    ASSERT(list.ptrlist == &node);
    ASSERT(node.prev == NULL);
    ASSERT(node.next == NULL);
}

TEST(test_cpx_link_descending_order)
{
    DM2_V1_CpxLinklist list;
    dm2_v1_dballoc_cpx_linklist_init(&list);

    DM2_V1_DballocLpp a, b, c;
    a.amount = 50;
    b.amount = 100;
    c.amount = 200;

    /* Insert in ascending order — list should reorder to descending */
    dm2_v1_dballoc_cpx_link_node(&list, &a);
    dm2_v1_dballoc_cpx_link_node(&list, &b);
    dm2_v1_dballoc_cpx_link_node(&list, &c);

    ASSERT(list.ptrlist == &c);
    ASSERT(c.next == &b);
    ASSERT(b.next == &a);
    ASSERT(a.next == NULL);
    ASSERT(a.prev == &b);
    ASSERT(b.prev == &c);
    ASSERT(c.prev == NULL);
}

TEST(test_cpx_link_equal_amounts)
{
    DM2_V1_CpxLinklist list;
    dm2_v1_dballoc_cpx_linklist_init(&list);

    DM2_V1_DballocLpp a, b;
    a.amount = 100;
    b.amount = 100;

    dm2_v1_dballoc_cpx_link_node(&list, &a);
    dm2_v1_dballoc_cpx_link_node(&list, &b);

    /* b inserted at head (amount >= head) */
    ASSERT(list.ptrlist == &b);
    ASSERT(b.next == &a);
}

TEST(test_cpx_unlink_head)
{
    DM2_V1_CpxLinklist list;
    dm2_v1_dballoc_cpx_linklist_init(&list);

    DM2_V1_DballocLpp a, b;
    a.amount = 50;
    b.amount = 100;

    dm2_v1_dballoc_cpx_link_node(&list, &a);
    dm2_v1_dballoc_cpx_link_node(&list, &b);
    /* list: b(100) -> a(50) */

    dm2_v1_dballoc_cpx_unlink_node(&list, &b);
    ASSERT(list.ptrlist == &a);
    ASSERT(a.prev == NULL);
}

TEST(test_cpx_unlink_tail)
{
    DM2_V1_CpxLinklist list;
    dm2_v1_dballoc_cpx_linklist_init(&list);

    DM2_V1_DballocLpp a, b;
    a.amount = 50;
    b.amount = 100;

    dm2_v1_dballoc_cpx_link_node(&list, &a);
    dm2_v1_dballoc_cpx_link_node(&list, &b);

    dm2_v1_dballoc_cpx_unlink_node(&list, &a);
    ASSERT(list.ptrlist == &b);
    ASSERT(b.next == NULL);
}

TEST(test_cpx_unlink_middle)
{
    DM2_V1_CpxLinklist list;
    dm2_v1_dballoc_cpx_linklist_init(&list);

    DM2_V1_DballocLpp a, b, c;
    a.amount = 50;
    b.amount = 100;
    c.amount = 200;

    dm2_v1_dballoc_cpx_link_node(&list, &a);
    dm2_v1_dballoc_cpx_link_node(&list, &b);
    dm2_v1_dballoc_cpx_link_node(&list, &c);
    /* list: c(200) -> b(100) -> a(50) */

    dm2_v1_dballoc_cpx_unlink_node(&list, &b);
    ASSERT(list.ptrlist == &c);
    ASSERT(c.next == &a);
    ASSERT(a.prev == &c);
}

TEST(test_cpx_unlink_only_node)
{
    DM2_V1_CpxLinklist list;
    dm2_v1_dballoc_cpx_linklist_init(&list);

    DM2_V1_DballocLpp a;
    a.amount = 100;
    dm2_v1_dballoc_cpx_link_node(&list, &a);
    dm2_v1_dballoc_cpx_unlink_node(&list, &a);
    ASSERT(dm2_v1_dballoc_cpx_linklist_is_empty(&list));
}

/* CPX1 tests using heap-allocated buffer to embed nodes */
TEST(test_cpx1_exact_match)
{
    /* Create a buffer large enough for a free block */
    uint8_t buffer[256];
    memset(buffer, 0, sizeof(buffer));

    DM2_V1_CpxLinklist list;
    dm2_v1_dballoc_cpx_linklist_init(&list);

    DM2_V1_DballocLpp *node = (DM2_V1_DballocLpp *)buffer;
    node->amount = 100;
    dm2_v1_dballoc_cpx_link_node(&list, node);

    DM2_V1_Cpx1Receipt r = dm2_v1_dballoc_cpx1(&list, 100);
    ASSERT(r.allocated_amount == 100);
    ASSERT(r.block_ptr == buffer);
    /* Node was unlinked */
    ASSERT(dm2_v1_dballoc_cpx_linklist_is_empty(&list));
}

TEST(test_cpx1_no_split_small_remainder)
{
    uint8_t buffer[256];
    memset(buffer, 0, sizeof(buffer));

    DM2_V1_CpxLinklist list;
    dm2_v1_dballoc_cpx_linklist_init(&list);

    DM2_V1_DballocLpp *node = (DM2_V1_DballocLpp *)buffer;
    node->amount = 120;  /* Request 100, remainder 20 < 30 */
    dm2_v1_dballoc_cpx_link_node(&list, node);

    DM2_V1_Cpx1Receipt r = dm2_v1_dballoc_cpx1(&list, 100);
    ASSERT(r.allocated_amount == 120);  /* Uses entire block */
    ASSERT(dm2_v1_dballoc_cpx_linklist_is_empty(&list));
}

TEST(test_cpx1_split_large_remainder)
{
    uint8_t buffer[256];
    memset(buffer, 0, sizeof(buffer));

    DM2_V1_CpxLinklist list;
    dm2_v1_dballoc_cpx_linklist_init(&list);

    DM2_V1_DballocLpp *node = (DM2_V1_DballocLpp *)buffer;
    node->amount = 200;  /* Request 100, remainder 100 >= 30 */
    dm2_v1_dballoc_cpx_link_node(&list, node);

    DM2_V1_Cpx1Receipt r = dm2_v1_dballoc_cpx1(&list, 100);
    ASSERT(r.allocated_amount == 100);  /* Only requested amount */
    ASSERT(!dm2_v1_dballoc_cpx_linklist_is_empty(&list));

    /* The remainder node should be in the list */
    DM2_V1_DballocLpp *remainder = list.ptrlist;
    ASSERT(remainder != NULL);
    ASSERT(remainder->amount == 100);
}

/* ========================================================================
 * Preserved GFX binary search tests
 * ======================================================================== */

TEST(test_find_preserved_empty)
{
    uint32_t table[8];
    int16_t  idxtable[8];

    DM2_V1_PreservedGfxFindResult r = dm2_v1_dballoc_find_in_preserved_gfx(
        42, table, idxtable, 0);
    ASSERT(!r.found);
    ASSERT(r.index == 0);
}

TEST(test_find_preserved_single_match)
{
    uint32_t table[8];
    int16_t  idxtable[8];

    table[0] = 42;
    idxtable[0] = 0;

    DM2_V1_PreservedGfxFindResult r = dm2_v1_dballoc_find_in_preserved_gfx(
        42, table, idxtable, 1);
    ASSERT(r.found);
    ASSERT(r.index == 0);
}

TEST(test_find_preserved_single_miss_before)
{
    uint32_t table[8];
    int16_t  idxtable[8];

    table[0] = 100;
    idxtable[0] = 0;

    DM2_V1_PreservedGfxFindResult r = dm2_v1_dballoc_find_in_preserved_gfx(
        50, table, idxtable, 1);
    ASSERT(!r.found);
    ASSERT(r.index == 0);
}

TEST(test_find_preserved_single_miss_after)
{
    uint32_t table[8];
    int16_t  idxtable[8];

    table[0] = 100;
    idxtable[0] = 0;

    DM2_V1_PreservedGfxFindResult r = dm2_v1_dballoc_find_in_preserved_gfx(
        200, table, idxtable, 1);
    ASSERT(!r.found);
    ASSERT(r.index == 1);
}

TEST(test_find_preserved_multiple)
{
    uint32_t table[8];
    int16_t  idxtable[8];

    /* Sorted by value: 10, 20, 30, 40 */
    table[0] = 10; table[1] = 20; table[2] = 30; table[3] = 40;
    idxtable[0] = 0; idxtable[1] = 1; idxtable[2] = 2; idxtable[3] = 3;

    /* Find each */
    DM2_V1_PreservedGfxFindResult r;
    r = dm2_v1_dballoc_find_in_preserved_gfx(10, table, idxtable, 4);
    ASSERT(r.found && r.index == 0);

    r = dm2_v1_dballoc_find_in_preserved_gfx(30, table, idxtable, 4);
    ASSERT(r.found && r.index == 2);

    r = dm2_v1_dballoc_find_in_preserved_gfx(40, table, idxtable, 4);
    ASSERT(r.found && r.index == 3);

    /* Miss between entries */
    r = dm2_v1_dballoc_find_in_preserved_gfx(25, table, idxtable, 4);
    ASSERT(!r.found);
    ASSERT(r.index == 2);  /* insertion point */

    /* Miss after all */
    r = dm2_v1_dballoc_find_in_preserved_gfx(50, table, idxtable, 4);
    ASSERT(!r.found);
    ASSERT(r.index == 4);
}

/* ========================================================================
 * W-table lookup tests
 * ======================================================================== */

TEST(test_wtable_lookup_table1)
{
    int16_t w_table1[4] = {10, 20, 30, 40};
    int16_t w_table2[4] = {50, 60, 70, 80};

    /* Bit 15 set -> use w_table1, index = dbidx & 0x7fff */
    int16_t r = dm2_v1_dballoc_get_wtable_entry((int16_t)(0x8000 | 2),
                                                  w_table1, w_table2);
    ASSERT(r == 30);
}

TEST(test_wtable_lookup_table2)
{
    int16_t w_table1[4] = {10, 20, 30, 40};
    int16_t w_table2[4] = {50, 60, 70, 80};

    /* Bit 15 clear -> use w_table2 */
    int16_t r = dm2_v1_dballoc_get_wtable_entry(1, w_table1, w_table2);
    ASSERT(r == 60);
}

/* ========================================================================
 * SET_PPPW_ENTRY tests
 * ======================================================================== */

TEST(test_set_pppw_entry_null)
{
    DM2_V1_DballocPppw entry;
    dm2_v1_dballoc_set_pppw_entry(&entry, NULL, 0, 42);
    ASSERT(entry.wp_00 == NULL);
    ASSERT(entry.wp_04 == NULL);
    ASSERT(entry.wp_08 == NULL);
    ASSERT(entry.w_0c == 42);
}

TEST(test_set_pppw_entry_with_offset)
{
    int16_t buffer[32];
    int16_t *ptr = &buffer[16];

    DM2_V1_DballocPppw entry;
    dm2_v1_dballoc_set_pppw_entry(&entry, ptr, 10, 7);

    ASSERT(entry.wp_00 == ptr);
    ASSERT(entry.wp_04 == ptr - 5);  /* offset/2 = 10/2 = 5 */
    ASSERT(entry.wp_08 == ptr);
    ASSERT(entry.w_0c == 7);
}

TEST(test_set_pppw_entry_zero_offset)
{
    int16_t buffer[8];
    DM2_V1_DballocPppw entry;
    dm2_v1_dballoc_set_pppw_entry(&entry, buffer, 0, 3);

    ASSERT(entry.wp_00 == buffer);
    ASSERT(entry.wp_04 == buffer);  /* offset 0 -> no adjustment */
    ASSERT(entry.wp_08 == buffer);
    ASSERT(entry.w_0c == 3);
}

/* ========================================================================
 * Generate ID tests
 * ======================================================================== */

TEST(test_generate_id_no_collision)
{
    int16_t counter = 0;
    uint32_t table[4] = {0};
    int16_t  idxtable[4] = {0};

    uint32_t id = dm2_v1_dballoc_generate_id(&counter, table, idxtable, 0);
    ASSERT(id == 0xFFFF0000u);
    ASSERT(counter == 1);
}

TEST(test_generate_id_skips_collision)
{
    int16_t counter = 5;
    /* Pre-populate table with ID 0xFFFF0005 */
    uint32_t table[4];
    int16_t  idxtable[4];
    table[0] = 0xFFFF0005u;
    idxtable[0] = 0;

    uint32_t id = dm2_v1_dballoc_generate_id(&counter, table, idxtable, 1);
    /* Should skip 0xFFFF0005 and return 0xFFFF0006 */
    ASSERT(id == 0xFFFF0006u);
    ASSERT(counter == 7);
}

/* ========================================================================
 * CPX heap create_index tests
 * ======================================================================== */

TEST(test_cpxheap_create_index_basic)
{
    DM2_V1_DballocXmalloc *gfx_table[8];
    memset(gfx_table, 0, sizeof(gfx_table));

    DM2_V1_CpxHeapState heap;
    memset(&heap, 0, sizeof(heap));
    heap.gfx_table = gfx_table;
    heap.mallocm = 0;
    heap.mallock = 0;
    heap.num_gfx_tableentries = 8;

    int16_t idx = dm2_v1_dballoc_cpxheap_create_index(&heap, NULL, NULL);
    ASSERT(idx == 0);
    ASSERT(heap.mallock == 1);
    ASSERT(heap.mallocm == 1);  /* advanced to next free slot */
}

TEST(test_cpxheap_create_index_skips_occupied)
{
    DM2_V1_DballocXmalloc dummy;
    DM2_V1_DballocXmalloc *gfx_table[8];
    memset(gfx_table, 0, sizeof(gfx_table));
    gfx_table[1] = &dummy;  /* slot 1 occupied */

    DM2_V1_CpxHeapState heap;
    memset(&heap, 0, sizeof(heap));
    heap.gfx_table = gfx_table;
    heap.mallocm = 0;
    heap.mallock = 0;
    heap.num_gfx_tableentries = 8;

    int16_t idx = dm2_v1_dballoc_cpxheap_create_index(&heap, NULL, NULL);
    ASSERT(idx == 0);
    ASSERT(heap.mallocm == 2);  /* skipped slot 1 */
}

TEST(test_cpxheap_create_index_table_full)
{
    DM2_V1_DballocXmalloc *gfx_table[4];
    memset(gfx_table, 0, sizeof(gfx_table));

    DM2_V1_CpxHeapState heap;
    memset(&heap, 0, sizeof(heap));
    heap.gfx_table = gfx_table;
    heap.mallocm = 0;
    heap.mallock = 3;  /* one less than num entries */
    heap.num_gfx_tableentries = 4;

    int16_t idx = dm2_v1_dballoc_cpxheap_create_index(&heap, NULL, NULL);
    ASSERT(idx == 0);
    ASSERT(heap.mallocm == DM2_DBALLOC_NODATA);  /* no more slots */
}

/* ========================================================================
 * GFX table compaction tests
 * ======================================================================== */

static int16_t test_compact_get_wtable(void *ctx, int16_t dbidx3)
{
    (void)ctx;
    /* Simple: dbidx3 with bit 15 cleared is the gfx_table index */
    return dbidx3 & 0x7fff;
}

TEST(test_gfx_compact_empty_list)
{
    DM2_V1_CpxLinklist list;
    dm2_v1_dballoc_cpx_linklist_init(&list);

    DM2_V1_GfxCompactState state;
    memset(&state, 0, sizeof(state));

    DM2_V1_GfxCompactCallbacks cb = { test_compact_get_wtable };

    DM2_V1_GfxCompactReceipt r = dm2_v1_dballoc_init_gfx_table(
        &list, &state, NULL, NULL, &cb, NULL);
    ASSERT(!r.compacted);
}

/* ========================================================================
 * Add to preserved GFX tests
 * ======================================================================== */

static int16_t test_alloc_slot_counter = 0;
static int16_t test_alloc_slot(void *ctx)
{
    (void)ctx;
    return test_alloc_slot_counter++;
}

TEST(test_add_preserved_gfx_basic)
{
    uint32_t table[8];
    int16_t  idxtable[8];
    int16_t  count = 0;
    memset(table, 0, sizeof(table));
    memset(idxtable, 0, sizeof(idxtable));

    test_alloc_slot_counter = 0;
    DM2_V1_AddPreservedGfxCallbacks cb = { test_alloc_slot, NULL };

    DM2_V1_AddPreservedGfxReceipt r = dm2_v1_dballoc_add_to_preserved_gfx(
        42, 0, idxtable, table, &count, 8, &cb, NULL);

    ASSERT(r.dbidx == 0);
    ASSERT(!r.evicted);
    ASSERT(count == 1);
    ASSERT(table[0] == 42);
    ASSERT(idxtable[0] == 0);
}

TEST(test_add_preserved_gfx_insert_middle)
{
    uint32_t table[8];
    int16_t  idxtable[8];
    int16_t  count = 0;
    memset(table, 0, sizeof(table));

    test_alloc_slot_counter = 0;
    DM2_V1_AddPreservedGfxCallbacks cb = { test_alloc_slot, NULL };

    /* Add 10 at pos 0 */
    dm2_v1_dballoc_add_to_preserved_gfx(10, 0, idxtable, table, &count, 8, &cb, NULL);
    /* Add 30 at pos 1 */
    dm2_v1_dballoc_add_to_preserved_gfx(30, 1, idxtable, table, &count, 8, &cb, NULL);
    /* Add 20 at pos 1 (between 10 and 30) */
    dm2_v1_dballoc_add_to_preserved_gfx(20, 1, idxtable, table, &count, 8, &cb, NULL);

    ASSERT(count == 3);
    /* Verify sorted order via indirection */
    ASSERT(table[idxtable[0]] == 10);
    ASSERT(table[idxtable[1]] == 20);
    ASSERT(table[idxtable[2]] == 30);
}

/* ========================================================================
 * Main
 * ======================================================================== */

int main(void)
{
    printf("dm2_v1_dballoc_pc34_compat tests:\n");

    /* CPX linked list */
    RUN(test_cpx_linklist_init);
    RUN(test_cpx_link_single_node);
    RUN(test_cpx_link_descending_order);
    RUN(test_cpx_link_equal_amounts);
    RUN(test_cpx_unlink_head);
    RUN(test_cpx_unlink_tail);
    RUN(test_cpx_unlink_middle);
    RUN(test_cpx_unlink_only_node);
    RUN(test_cpx1_exact_match);
    RUN(test_cpx1_no_split_small_remainder);
    RUN(test_cpx1_split_large_remainder);

    /* Preserved GFX */
    RUN(test_find_preserved_empty);
    RUN(test_find_preserved_single_match);
    RUN(test_find_preserved_single_miss_before);
    RUN(test_find_preserved_single_miss_after);
    RUN(test_find_preserved_multiple);

    /* W-table lookup */
    RUN(test_wtable_lookup_table1);
    RUN(test_wtable_lookup_table2);

    /* SET_PPPW_ENTRY */
    RUN(test_set_pppw_entry_null);
    RUN(test_set_pppw_entry_with_offset);
    RUN(test_set_pppw_entry_zero_offset);

    /* Generate ID */
    RUN(test_generate_id_no_collision);
    RUN(test_generate_id_skips_collision);

    /* CPX heap create_index */
    RUN(test_cpxheap_create_index_basic);
    RUN(test_cpxheap_create_index_skips_occupied);
    RUN(test_cpxheap_create_index_table_full);

    /* GFX table compaction */
    RUN(test_gfx_compact_empty_list);

    /* Add preserved GFX */
    RUN(test_add_preserved_gfx_basic);
    RUN(test_add_preserved_gfx_insert_middle);

    printf("\nResults: %d passed, %d failed\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
