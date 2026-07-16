#include "firestaff/dm1/v1/object_clear_leader_hand_object_name_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void expect_true(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static void test_full_source_box_and_state_preservation(void)
{
    enum { kWidth = 320, kHeight = 200, kStride = 320 };
    unsigned char screen[kStride * kHeight];
    DM1_V1_F0035LeaderHandStatePc34 state;
    uint16_t saved_thing;
    int saved_present;
    char saved_name[DM1_V1_F0035_LEADER_HAND_OBJECT_NAME_CAP_PC34];
    int x;
    int y;

    memset(screen, 0x5a, sizeof(screen));
    memset(&state, 0, sizeof(state));
    state.logical_screen = screen;
    state.logical_screen_size = sizeof(screen);
    state.logical_screen_stride = kStride;
    state.logical_screen_width = kWidth;
    state.logical_screen_height = kHeight;
    state.leader_hand_thing = 0x1234;
    state.leader_hand_present = 1;
    strcpy(state.leader_hand_object_name, "FURY");
    saved_thing = state.leader_hand_thing;
    saved_present = state.leader_hand_present;
    memcpy(saved_name, state.leader_hand_object_name, sizeof(saved_name));

    expect_true(DM1_V1_F0035_OBJECT_ClearLeaderHandObjectNamePc34Compat(&state),
                "F0035 accepts a complete logical screen");
    for (y = 0; y < kHeight; ++y) {
        for (x = 0; x < kWidth; ++x) {
            int in_box = x >= DM1_V1_F0035_LEADER_HAND_NAME_X_PC34 &&
                         x < DM1_V1_F0035_LEADER_HAND_NAME_X_PC34 +
                                 DM1_V1_F0035_LEADER_HAND_NAME_WIDTH_PC34 &&
                         y >= DM1_V1_F0035_LEADER_HAND_NAME_Y_PC34 &&
                         y < DM1_V1_F0035_LEADER_HAND_NAME_Y_PC34 +
                                 DM1_V1_F0035_LEADER_HAND_NAME_HEIGHT_PC34;
            expect_true(screen[y * kStride + x] == (in_box ? 0 : 0x5a),
                        "F0035 clears exactly the source box");
        }
    }
    expect_true(state.leader_hand_thing == saved_thing,
                "F0035 preserves the held object");
    expect_true(state.leader_hand_present == saved_present,
                "F0035 preserves leader-hand presence");
    expect_true(memcmp(state.leader_hand_object_name, saved_name, sizeof(saved_name)) == 0,
                "F0035 preserves the source object name");
}

static void test_clips_to_complete_surface(void)
{
    enum { kWidth = 250, kHeight = 36, kStride = 256 };
    unsigned char screen[kStride * kHeight];
    DM1_V1_F0035LeaderHandStatePc34 state;
    int x;
    int y;

    memset(screen, 0x7c, sizeof(screen));
    memset(&state, 0, sizeof(state));
    state.logical_screen = screen;
    state.logical_screen_size = sizeof(screen);
    state.logical_screen_stride = kStride;
    state.logical_screen_width = kWidth;
    state.logical_screen_height = kHeight;

    expect_true(DM1_V1_F0035_OBJECT_ClearLeaderHandObjectNamePc34Compat(&state),
                "F0035 accepts a clipped complete surface");
    for (y = 0; y < kHeight; ++y) {
        for (x = 0; x < kStride; ++x) {
            int in_box = x >= DM1_V1_F0035_LEADER_HAND_NAME_X_PC34 && x < kWidth &&
                         y >= DM1_V1_F0035_LEADER_HAND_NAME_Y_PC34 && y < kHeight;
            expect_true(screen[y * kStride + x] == (in_box ? 0 : 0x7c),
                        "F0035 writes only the clipped visible rectangle");
        }
    }
}

static void test_rejects_truncated_surface_without_writing(void)
{
    unsigned char screen[320 * 200];
    unsigned char before[320 * 200];
    DM1_V1_F0035LeaderHandStatePc34 state;

    memset(screen, 0x3d, sizeof(screen));
    memcpy(before, screen, sizeof(screen));
    memset(&state, 0, sizeof(state));
    state.logical_screen = screen;
    state.logical_screen_size = sizeof(screen) - 1;
    state.logical_screen_stride = 320;
    state.logical_screen_width = 320;
    state.logical_screen_height = 200;

    expect_true(!DM1_V1_F0035_OBJECT_ClearLeaderHandObjectNamePc34Compat(&state),
                "F0035 rejects truncated logical-screen storage");
    expect_true(memcmp(screen, before, sizeof(screen)) == 0,
                "F0035 does not write after storage validation fails");
}

int main(void)
{
    test_full_source_box_and_state_preservation();
    test_clips_to_complete_surface();
    test_rejects_truncated_surface_without_writing();
    if (failures != 0) {
        fprintf(stderr, "%d test assertion(s) failed\n", failures);
        return 1;
    }
    return 0;
}
