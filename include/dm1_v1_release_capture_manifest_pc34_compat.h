#ifndef FIRESTAFF_DM1_V1_RELEASE_CAPTURE_MANIFEST_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_RELEASE_CAPTURE_MANIFEST_PC34_COMPAT_H

#include "dm1_v1_action_spell_runtime_capture_pc34_compat.h"
#include "dm1_v1_f0437_f0438_f0439_startup_visual_admission_pc34_compat.h"
#include "firestaff/dm1/v1/startup_sequence_pc34_compat.h"

/*
 * A release capture is evidence, not a render plan.  Each supplied frame must
 * already have been presented by the app from an authenticated DM1 receipt.
 */
typedef struct DM1_V1_ReleaseCaptureFramePc34 {
    int capturedFromMacWindow;
    int capturedFromReleaseApp;
    int width;
    int height;
    int byteCount;
    unsigned int framebufferHash;
} DM1_V1_ReleaseCaptureFramePc34;

typedef struct DM1_V1_ReleaseCaptureManifestInputPc34 {
    const DM1_V1_F0437TitleMaterialReceiptPc34 *titleReceipt;
    const DM1_V1_StartupHoCPresentedCaptureHostExportReceipt_PC34 *hocReceipt;
    const DM1_V1_ActionSpellRuntimeCaptureReceiptPc34 *hudReceipt;
    DM1_V1_ReleaseCaptureFramePc34 titleFrame;
    DM1_V1_ReleaseCaptureFramePc34 hocFrame;
    DM1_V1_ReleaseCaptureFramePc34 hudFrame;
} DM1_V1_ReleaseCaptureManifestInputPc34;

typedef struct DM1_V1_ReleaseCaptureManifestPc34 {
    int handled;
    int ready;
    int titleReceiptConsumed;
    int hocReceiptConsumed;
    int hudReceiptConsumed;
    int titleFrameCaptured;
    int hocFrameCaptured;
    int hudFrameCaptured;
    unsigned int titleFrameHash;
    unsigned int hocFrameHash;
    unsigned int hudFrameHash;
    unsigned int manifestHash;
    const char *sourceEvidence;
} DM1_V1_ReleaseCaptureManifestPc34;

int dm1_v1_release_capture_manifest_build_pc34(
    const DM1_V1_ReleaseCaptureManifestInputPc34 *input,
    DM1_V1_ReleaseCaptureManifestPc34 *outManifest);

#endif
