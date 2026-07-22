#ifndef FIRESTAFF_DM1_V1_ACTION_SPELL_RUNTIME_CAPTURE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ACTION_SPELL_RUNTIME_CAPTURE_PC34_COMPAT_H

#include "dm1_v1_action_spell_source_frame_m11_lifecycle_pc34_compat.h"

/* Final source-owned capture proof for a live action/spell runtime frame. */
typedef struct {
    int accepted;
    int runtimeCaptureCurrent;
    int presentationKind;
    int originalGraphicId;
    int originalZoneId;
    int companionGraphicId;
    int sourceAssetCount;
    int sourceCommandCount;
    int revokeStaleCapture;
    int staleOriginalGraphicId;
    int staleOriginalZoneId;
    int staleCompanionGraphicId;
    int suppressSyntheticFallback;
    unsigned int frameTick;
    unsigned int sourceTick;
    unsigned int serial;
    unsigned int commandFingerprint;
    unsigned int orderingFingerprint;
} DM1_V1_ActionSpellRuntimeCaptureReceiptPc34;

/*
 * Converts current M11-facing source evidence into final runtime capture
 * evidence. The receipt never captures substitute material or invokes M11.
 */
int dm1_v1_action_spell_runtime_capture_build_pc34(
    const DM1_V1_ActionSpellSourceFrameM11LifecycleReceiptPc34 *lifecycle,
    DM1_V1_ActionSpellRuntimeCaptureReceiptPc34 *outReceipt);

#endif
