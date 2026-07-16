#include "firestaff/dm1/v1/f0035_object_clear_leader_hand_object_name_bridge_b17e_pc34_compat.h"

#include <stdio.h>

static int failures;

typedef struct ZoneFillCall {
    int count;
    int zone_index;
    int color;
} ZoneFillCall;

static void expect_equal(int actual, int expected, const char *description)
{
    if (actual != expected) {
        fprintf(stderr, "FAIL: %s (got %d; expected %d)\n", description,
                actual, expected);
        ++failures;
    }
}

static void capture_zone_fill(void *context, int zone_index, int color)
{
    ZoneFillCall *call = context;

    ++call->count;
    call->zone_index = zone_index;
    call->color = color;
}

static void test_f0035_delegates_to_the_leader_hand_name_zone(void)
{
    ZoneFillCall call = {0, -1, -1};
    DM1_V1_F0035ZoneBackendBridgeB17ePc34 backend = {
        &call,
        capture_zone_fill
    };

    DM1_V1_F0035_OBJECT_ClearLeaderHandObjectNameBridgeB17ePc34Compat(
        &backend);

    expect_equal(call.count, 1, "one F0733-equivalent zone fill");
    expect_equal(call.zone_index,
                 DM1_V1_F0035_LEADER_HAND_NAME_ZONE_BRIDGE_B17E_PC34,
                 "C017 leader-hand object-name zone");
    expect_equal(call.color, DM1_V1_F0035_BLACK_BRIDGE_B17E_PC34,
                 "C00 black fill color");
}

static void test_unavailable_backend_is_inert(void)
{
    ZoneFillCall call = {0, -1, -1};
    DM1_V1_F0035ZoneBackendBridgeB17ePc34 backend = {&call, NULL};

    DM1_V1_F0035_OBJECT_ClearLeaderHandObjectNameBridgeB17ePc34Compat(NULL);
    DM1_V1_F0035_OBJECT_ClearLeaderHandObjectNameBridgeB17ePc34Compat(
        &backend);

    expect_equal(call.count, 0, "no backend means no synthetic fill");
}

int main(void)
{
    test_f0035_delegates_to_the_leader_hand_name_zone();
    test_unavailable_backend_is_inert();
    return failures != 0;
}
