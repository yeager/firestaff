#include "firestaff/dm1/v1/f0035_object_clear_leader_hand_object_name_redux_20260714_pc34_compat.h"

#include <stdio.h>

static int failures;

typedef struct FillReceipt {
    int calls;
    int zone_index;
    int color;
} FillReceipt;

static void expect_int(int actual, int expected, const char *message)
{
    if (actual != expected) {
        fprintf(stderr, "FAIL: %s (got %d, expected %d)\n", message, actual,
                expected);
        ++failures;
    }
}

static void record_fill(void *context, int zone_index, int color)
{
    FillReceipt *receipt = context;

    ++receipt->calls;
    receipt->zone_index = zone_index;
    receipt->color = color;
}

static void test_dispatches_the_source_zone_fill(void)
{
    FillReceipt receipt = {0, -1, -1};
    DM1_V1_F0035ObjectClearBackendRedux20260714Pc34 backend = {
        &receipt,
        record_fill
    };

    DM1_V1_F0035_OBJECT_ClearLeaderHandObjectNameRedux20260714Pc34Compat(
        &backend);

    expect_int(receipt.calls, 1, "F0035 performs one zone fill");
    expect_int(receipt.zone_index,
               DM1_V1_F0035_LEADER_HAND_OBJECT_NAME_ZONE_REDUX_20260714_PC34,
               "F0035 fills C017 leader-hand object-name zone");
    expect_int(receipt.color, DM1_V1_F0035_BLACK_REDUX_20260714_PC34,
               "F0035 fills with C00 black");
}

static void test_missing_backend_is_a_no_op(void)
{
    FillReceipt receipt = {0, -1, -1};
    DM1_V1_F0035ObjectClearBackendRedux20260714Pc34 no_fill = {
        &receipt,
        NULL
    };

    DM1_V1_F0035_OBJECT_ClearLeaderHandObjectNameRedux20260714Pc34Compat(NULL);
    DM1_V1_F0035_OBJECT_ClearLeaderHandObjectNameRedux20260714Pc34Compat(
        &no_fill);

    expect_int(receipt.calls, 0, "missing backend does not synthesize a fill");
}

int main(void)
{
    test_dispatches_the_source_zone_fill();
    test_missing_backend_is_a_no_op();
    return failures != 0;
}
