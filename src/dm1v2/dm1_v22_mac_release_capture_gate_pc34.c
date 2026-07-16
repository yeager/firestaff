#include "dm1_v22_mac_release_capture_gate_pc34.h"

#include <string.h>

static const char* const k_source_evidence =
    "ReDMCSB DUNVIEW.C F0115/F0128/F0129/F0130 original viewport draw order; "
    "ReDMCSB PANEL.C/COMMAND.C source HUD/action/spell surfaces; "
    "DM1 V2.2 finished-art material and screenshot receipt gates";

static int streq(const char* a, const char* b) {
    return a && b && strcmp(a, b) == 0;
}

static int contains_case_token(const char* text, const char* a, const char* b) {
    if (!text) return 0;
    return strstr(text, a) != 0 || strstr(text, b) != 0;
}

static int has_text(const char* text) {
    return text && text[0] != '\0';
}

const char* dm1_v22_mac_release_capture_state_name_pc34(
    DM1_V22_MacReleaseCaptureStatePc34 state) {
    switch (state) {
        case DM1_V22_MAC_CAPTURE_NOT_PROBED: return "NOT_PROBED";
        case DM1_V22_MAC_CAPTURE_NO_RECEIPT: return "NO_RECEIPT";
        case DM1_V22_MAC_CAPTURE_WRONG_GAME: return "WRONG_GAME";
        case DM1_V22_MAC_CAPTURE_NOT_V22: return "NOT_V22";
        case DM1_V22_MAC_CAPTURE_NOT_RELEASE_APP: return "NOT_RELEASE_APP";
        case DM1_V22_MAC_CAPTURE_MISSING_REAL_DATA: return "MISSING_REAL_DATA";
        case DM1_V22_MAC_CAPTURE_MATERIAL_NOT_FINISHED: return "MATERIAL_NOT_FINISHED";
        case DM1_V22_MAC_CAPTURE_SCREENSHOT_NOT_FINISHED: return "SCREENSHOT_NOT_FINISHED";
        case DM1_V22_MAC_CAPTURE_BAD_FRAME: return "BAD_FRAME";
        case DM1_V22_MAC_CAPTURE_READY: return "READY";
        default: return "UNKNOWN";
    }
}

const char* dm1_v22_mac_release_capture_source_evidence_pc34(void) {
    return k_source_evidence;
}

int dm1_v22_mac_release_capture_gate_pc34(
    const DM1_V22_MacReleaseCaptureInputPc34* in,
    DM1_V22_MacReleaseCaptureReceiptPc34* out) {
    DM1_V22_MacReleaseCaptureReceiptPc34 receipt;

    if (!out) {
        return 0;
    }

    memset(&receipt, 0, sizeof(receipt));
    receipt.state = DM1_V22_MAC_CAPTURE_NOT_PROBED;
    receipt.requires_operator_capture = 1;
    receipt.blocker = "gate not evaluated";
    receipt.source_evidence = k_source_evidence;

    if (!in || !has_text(in->capture_path)) {
        receipt.state = DM1_V22_MAC_CAPTURE_NO_RECEIPT;
        receipt.blocker = "missing packaged Mac/release-app capture receipt";
    } else if (!streq(in->game_id, "dm1")) {
        receipt.state = DM1_V22_MAC_CAPTURE_WRONG_GAME;
        receipt.blocker = "capture is not for DM1";
    } else if (!streq(in->presentation_mode, "V22_MODERN")) {
        receipt.state = DM1_V22_MAC_CAPTURE_NOT_V22;
        receipt.blocker = "capture is not DM1 V2.2 modern presentation";
    } else if (!contains_case_token(in->host_os, "mac", "Mac")) {
        receipt.state = DM1_V22_MAC_CAPTURE_NOT_RELEASE_APP;
        receipt.blocker = "capture is not from macOS";
    } else if (!streq(in->app_kind, "release_app") &&
               !streq(in->app_kind, "packaged_app")) {
        receipt.state = DM1_V22_MAC_CAPTURE_NOT_RELEASE_APP;
        receipt.blocker = "capture is not from packaged release app";
    } else if (!has_text(in->data_root)) {
        receipt.state = DM1_V22_MAC_CAPTURE_MISSING_REAL_DATA;
        receipt.blocker = "capture lacks real DM1 data root";
    } else if (!in->material_gate_finished_real) {
        receipt.state = DM1_V22_MAC_CAPTURE_MATERIAL_NOT_FINISHED;
        receipt.blocker = "finished-art material gate is not FINISHED_REAL";
    } else if (!in->screenshot_receipt_finished) {
        receipt.state = DM1_V22_MAC_CAPTURE_SCREENSHOT_NOT_FINISHED;
        receipt.blocker = "runtime screenshot material receipt is not FINISHED_REAL";
    } else if (in->frame_width <= 0 || in->frame_height <= 0 ||
               in->frame_hash == 0u) {
        receipt.state = DM1_V22_MAC_CAPTURE_BAD_FRAME;
        receipt.blocker = "capture frame dimensions or hash are invalid";
    } else {
        receipt.state = DM1_V22_MAC_CAPTURE_READY;
        receipt.eligible_for_pixel_promotion = 1;
        receipt.requires_operator_capture = 0;
        receipt.blocker = "";
    }

    *out = receipt;
    return 1;
}
