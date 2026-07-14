#include "firestaff/dm1/v1/f0035_object_clear_leader_hand_object_name_isolated_c91f_pc34_compat.h"

#include <stdio.h>

static int failures;

typedef struct ZoneCall {
    int invocation_count;
    int zone_index;
    int color;
} ZoneCall;

static void expect_int(int actual, int expected, const char *message)
{
    if (actual != expected) {
        fprintf(stderr, "FAIL: %s (got %d, expected %d)\n", message, actual,
                expected);
        ++failures;
    }
}

static void record_zone_fill(void *context, int zone_index, int color)
{
    ZoneCall *call = context;

    ++call->invocation_count;
    call->zone_index = zone_index;
    call->color = color;
}

static void test_source_zone_and_color_are_forwarded(void)
{
    ZoneCall call = {0, -1, -1};
    DM1_V1_F0035ZoneBackendIsolatedC91fPc34 backend = {
        &call,
        record_zone_fill
    };

    DM1_V1_F0035_OBJECT_ClearLeaderHandObjectNameIsolatedC91fPc34Compat(
        &backend);

    expect_int(call.invocation_count, 1, "F0035 calls the zone fill once");
    expect_int(call.zone_index,
               DM1_V1_F0035_LEADER_HAND_NAME_ZONE_ISOLATED_C91F_PC34,
               "F0035 selects C017");
    expect_int(call.color, DM1_V1_F0035_BLACK_ISOLATED_C91F_PC34,
               "F0035 selects C00 black");
}

static void test_absent_zone_backend_does_nothing(void)
{
    ZoneCall call = {0, -1, -1};
    DM1_V1_F0035ZoneBackendIsolatedC91fPc34 backend = {&call, NULL};

    DM1_V1_F0035_OBJECT_ClearLeaderHandObjectNameIsolatedC91fPc34Compat(NULL);
    DM1_V1_F0035_OBJECT_ClearLeaderHandObjectNameIsolatedC91fPc34Compat(
        &backend);

    expect_int(call.invocation_count, 0, "no callback yields no UI surrogate");
}

int main(void)
{
    test_source_zone_and_color_are_forwarded();
    test_absent_zone_backend_does_nothing();
    return failures != 0;
}
