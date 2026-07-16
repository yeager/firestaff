#include "dm1_v22_mac_release_capture_gate_pc34.h"

#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK_EQ(actual, expected, label) do { \
    int a_ = (int)(actual); \
    int e_ = (int)(expected); \
    if (a_ != e_) { \
        fprintf(stderr, "FAIL %s: expected %d got %d\n", label, e_, a_); \
        ++failures; \
    } \
} while (0)

#define CHECK_TRUE(cond, label) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s\n", label); \
        ++failures; \
    } \
} while (0)

static DM1_V22_MacReleaseCaptureInputPc34 good_input(void) {
    DM1_V22_MacReleaseCaptureInputPc34 in;
    memset(&in, 0, sizeof(in));
    in.game_id = "dm1";
    in.presentation_mode = "V22_MODERN";
    in.host_os = "macOS";
    in.app_kind = "release_app";
    in.data_root = "/Users/test/.firestaff/data/dm1";
    in.capture_path = "/Users/test/Library/Application Support/Firestaff/captures/dm1-v22.bmp";
    in.material_gate_finished_real = 1;
    in.screenshot_receipt_finished = 1;
    in.frame_width = 1920;
    in.frame_height = 1080;
    in.frame_hash = 0x12345678u;
    return in;
}

static void expect_state(DM1_V22_MacReleaseCaptureInputPc34 in,
                         DM1_V22_MacReleaseCaptureStatePc34 state,
                         const char* label) {
    DM1_V22_MacReleaseCaptureReceiptPc34 out;
    CHECK_EQ(dm1_v22_mac_release_capture_gate_pc34(&in, &out), 1, label);
    CHECK_EQ(out.state, state, label);
    CHECK_EQ(out.eligible_for_pixel_promotion,
             state == DM1_V22_MAC_CAPTURE_READY, label);
    CHECK_EQ(out.requires_operator_capture,
             state != DM1_V22_MAC_CAPTURE_READY, label);
    CHECK_TRUE(out.source_evidence &&
               strstr(out.source_evidence, "ReDMCSB DUNVIEW.C F0115") != 0,
               "source evidence names ReDMCSB viewport owner");
}

int main(void) {
    DM1_V22_MacReleaseCaptureInputPc34 in = good_input();

    expect_state(in, DM1_V22_MAC_CAPTURE_READY, "ready capture");

    in = good_input();
    in.capture_path = "";
    expect_state(in, DM1_V22_MAC_CAPTURE_NO_RECEIPT, "missing receipt");

    in = good_input();
    in.game_id = "csb";
    expect_state(in, DM1_V22_MAC_CAPTURE_WRONG_GAME, "wrong game");

    in = good_input();
    in.presentation_mode = "V21_UPSCALED";
    expect_state(in, DM1_V22_MAC_CAPTURE_NOT_V22, "wrong presentation");

    in = good_input();
    in.host_os = "Linux";
    expect_state(in, DM1_V22_MAC_CAPTURE_NOT_RELEASE_APP, "wrong host");

    in = good_input();
    in.app_kind = "dev_binary";
    expect_state(in, DM1_V22_MAC_CAPTURE_NOT_RELEASE_APP, "wrong app kind");

    in = good_input();
    in.data_root = "";
    expect_state(in, DM1_V22_MAC_CAPTURE_MISSING_REAL_DATA, "missing data root");

    in = good_input();
    in.material_gate_finished_real = 0;
    expect_state(in, DM1_V22_MAC_CAPTURE_MATERIAL_NOT_FINISHED, "unfinished material");

    in = good_input();
    in.screenshot_receipt_finished = 0;
    expect_state(in, DM1_V22_MAC_CAPTURE_SCREENSHOT_NOT_FINISHED, "unfinished screenshot");

    in = good_input();
    in.frame_hash = 0;
    expect_state(in, DM1_V22_MAC_CAPTURE_BAD_FRAME, "bad frame hash");

    in = good_input();
    CHECK_EQ(dm1_v22_mac_release_capture_gate_pc34(&in, NULL), 0,
             "null output rejected");
    CHECK_TRUE(strcmp(dm1_v22_mac_release_capture_state_name_pc34(
                          DM1_V22_MAC_CAPTURE_READY), "READY") == 0,
               "state name ready");

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }

    puts("ok: DM1 V2.2 Mac release capture gate");
    return 0;
}
