#include "firestaff/dm1/v1/slot_drop_order_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;
static int g_assertions = 0;

static void check(int cond, const char *expr, const char *file, int line)
{
    ++g_assertions;
    if (!cond) {
        ++g_failures;
        fprintf(stderr, "FAIL: %s:%d %s\n", file, line, expr);
    }
}

#define CHECK(c) check((c), #c, __FILE__, __LINE__)

static void test_table_values(void)
{
    const int *t = dm1_v1_slot_drop_order_table_pc34();
    int n = dm1_v1_slot_drop_order_size_pc34();
    int i;
    CHECK(t != 0);
    CHECK(n == 30);
    /* First = C05_SLOT_FEET = 5, Last = C01_SLOT_ACTION_HAND = 1. */
    CHECK(t[0] == 5);
    CHECK(t[29] == 1);
    /* Spot-check key entries. */
    CHECK(t[1] == 4);    /* C04_SLOT_LEGS */
    CHECK(t[8] == 3);    /* C03_SLOT_TORSO */
    CHECK(t[26] == 10);  /* C10_SLOT_NECK */
    CHECK(t[27] == 2);   /* C02_SLOT_HEAD */
    CHECK(t[28] == 0);   /* C00_SLOT_READY_HAND */
    /* All backpack slots 13..29 must appear. */
    for (i = 13; i <= 29; ++i) {
        int j, found = 0;
        for (j = 0; j < n; ++j) {
            if (t[j] == i) { found = 1; break; }
        }
        CHECK(found);
    }
}

static void test_lookup_function(void)
{
    int i;
    for (i = 0; i < 30; ++i) {
        CHECK(dm1_v1_slot_drop_order_get_pc34(i) >= 0);
        CHECK(dm1_v1_slot_drop_order_get_pc34(i) <= 29);
    }
    CHECK(dm1_v1_slot_drop_order_get_pc34(-1) == -1);
    CHECK(dm1_v1_slot_drop_order_get_pc34(30) == -1);
    CHECK(dm1_v1_slot_drop_order_get_pc34(999) == -1);
}

static void test_first_last_specific(void)
{
    CHECK(dm1_v1_slot_drop_order_first_pc34() == 5);
    CHECK(dm1_v1_slot_drop_order_last_pc34() == 1);
}

static void test_run_accepted(void)
{
    DM1_V1_SlotDropOrderResultPc34 r;
    int ok = dm1_v1_slot_drop_order_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 9);
    CHECK(r.tableSize == 30);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.firstEntryFeetSlot5 == 1);
    CHECK(r.lastEntryActionHandSlot1 == 1);
    CHECK(r.allValuesInByteRange == 1);
    CHECK(r.allValuesDistinct == 1);
    CHECK(r.allBackpackSlotsCovered == 1);
    CHECK(r.lookupFunctionCorrect == 1);
    CHECK(r.lookupOutOfRangeReturnsMinusOne == 1);
    for (i = 0; i < 30; ++i) {
        CHECK(r.tableEntries[i] == dm1_v1_slot_drop_order_get_pc34(i));
    }
}

int main(void)
{
    test_table_values();
    test_lookup_function();
    test_first_last_specific();
    test_run_accepted();
    printf("dm1_v1_slot_drop_order: %d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}