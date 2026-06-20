#include "firestaff/dm1/v1/square_type_to_event_type_pc34_compat.h"

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
    const unsigned char *t = dm1_v1_square_type_to_event_type_table_pc34();
    int n = dm1_v1_square_type_to_event_type_size_pc34();
    static const unsigned char kExpected[7] = { 6, 5, 9, 0, 10, 8, 7 };
    int i;
    CHECK(t != 0);
    CHECK(n == 7);
    for (i = 0; i < 7; ++i) {
        CHECK(t[i] == kExpected[i]);
    }
}

static void test_lookup_function(void)
{
    int i;
    for (i = 0; i < 7; ++i) {
        CHECK(dm1_v1_square_type_to_event_type_get_pc34(i) >= 0);
        CHECK(dm1_v1_square_type_to_event_type_get_pc34(i) <= 255);
    }
    CHECK(dm1_v1_square_type_to_event_type_get_pc34(-1) == 0);
    CHECK(dm1_v1_square_type_to_event_type_get_pc34(7) == 0);
    CHECK(dm1_v1_square_type_to_event_type_get_pc34(999) == 0);
}

static void test_first_last_specific(void)
{
    CHECK(dm1_v1_square_type_to_event_type_get_pc34(0) == 6);   /* WALL */
    CHECK(dm1_v1_square_type_to_event_type_get_pc34(6) == 7);   /* FAKEWALL */
}

static void test_run_accepted(void)
{
    DM1_V1_SquareTypeToEventTypeResultPc34 r;
    int ok = dm1_v1_square_type_to_event_type_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 12);
    CHECK(r.tableSize == 7);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.square0WallEvent6 == 1);
    CHECK(r.square1CorridorEvent5 == 1);
    CHECK(r.square2PitEvent9 == 1);
    CHECK(r.square3NoneEvent0 == 1);
    CHECK(r.square4DoorEvent10 == 1);
    CHECK(r.square5TeleporterEvent8 == 1);
    CHECK(r.square6FakewallEvent7 == 1);
    CHECK(r.allEventsInValidRange == 1);
    CHECK(r.lookupFunctionCorrect == 1);
    CHECK(r.lookupOutOfRangeReturnsZero == 1);
    for (i = 0; i < 7; ++i) {
        CHECK(r.tableEntries[i] == dm1_v1_square_type_to_event_type_get_pc34(i));
    }
}

int main(void)
{
    test_table_values();
    test_lookup_function();
    test_first_last_specific();
    test_run_accepted();
    printf("dm1_v1_square_type_to_event_type: %d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}