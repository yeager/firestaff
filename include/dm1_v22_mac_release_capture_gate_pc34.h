/*
 * dm1_v22_mac_release_capture_gate_pc34.h
 *
 * DM1 V2.2 packaged Mac/release-app capture admission gate.
 *
 * Source-lock boundary:
 *   - ReDMCSB DUNVIEW.C F0115/F0128/F0129/F0130: the source viewport is
 *     still the original DM1 draw order.
 *   - ReDMCSB PANEL.C/COMMAND.C: HUD, action and spell command surfaces
 *     remain source-owned before V2.2 presentation consumes them.
 *   - include/dm1_v22_finished_art_material_gate_pc34.h: finished-art and
 *     runtime screenshot material receipts must already be positive.
 *
 * This gate does not inspect pixels. It is the final fail-closed handoff that
 * says a packaged Mac/release-app frame is eligible to be compared/promoted.
 */

#ifndef FIRESTAFF_DM1_V22_MAC_RELEASE_CAPTURE_GATE_PC34_H
#define FIRESTAFF_DM1_V22_MAC_RELEASE_CAPTURE_GATE_PC34_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM1_V22_MAC_CAPTURE_NOT_PROBED = 0,
    DM1_V22_MAC_CAPTURE_NO_RECEIPT = 1,
    DM1_V22_MAC_CAPTURE_WRONG_GAME = 2,
    DM1_V22_MAC_CAPTURE_NOT_V22 = 3,
    DM1_V22_MAC_CAPTURE_NOT_RELEASE_APP = 4,
    DM1_V22_MAC_CAPTURE_MISSING_REAL_DATA = 5,
    DM1_V22_MAC_CAPTURE_MATERIAL_NOT_FINISHED = 6,
    DM1_V22_MAC_CAPTURE_SCREENSHOT_NOT_FINISHED = 7,
    DM1_V22_MAC_CAPTURE_BAD_FRAME = 8,
    DM1_V22_MAC_CAPTURE_READY = 9
} DM1_V22_MacReleaseCaptureStatePc34;

typedef struct {
    const char* game_id;              /* must be "dm1" */
    const char* presentation_mode;    /* must be "V22_MODERN" */
    const char* host_os;              /* must name macOS/Mac */
    const char* app_kind;             /* "release_app" or "packaged_app" */
    const char* data_root;            /* real DM1 data root used by capture */
    const char* capture_path;         /* on-disk frame/receipt artifact */
    int material_gate_finished_real;  /* dm1_v22_famg_is_finished_real() */
    int screenshot_receipt_finished;  /* dm1_v22_famg_has_finished_real_receipt() */
    int frame_width;
    int frame_height;
    unsigned int frame_hash;          /* non-zero receipt hash */
} DM1_V22_MacReleaseCaptureInputPc34;

typedef struct {
    DM1_V22_MacReleaseCaptureStatePc34 state;
    int eligible_for_pixel_promotion;
    int requires_operator_capture;
    const char* blocker;
    const char* source_evidence;
} DM1_V22_MacReleaseCaptureReceiptPc34;

int dm1_v22_mac_release_capture_gate_pc34(
    const DM1_V22_MacReleaseCaptureInputPc34* in,
    DM1_V22_MacReleaseCaptureReceiptPc34* out);

const char* dm1_v22_mac_release_capture_state_name_pc34(
    DM1_V22_MacReleaseCaptureStatePc34 state);

const char* dm1_v22_mac_release_capture_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V22_MAC_RELEASE_CAPTURE_GATE_PC34_H */
