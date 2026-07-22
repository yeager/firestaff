#ifndef FIRESTAFF_DM1_V1_ACTION_SPELL_M11_CAPTURE_LIFECYCLE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ACTION_SPELL_M11_CAPTURE_LIFECYCLE_PC34_COMPAT_H

#include "dm1_v1_action_spell_source_frame_m11_lifecycle_pc34_compat.h"

typedef struct {
    int active;
    int graphicId;
    int zoneId;
    int companionGraphicId;
    int assetCount;
    unsigned int frameTick;
    unsigned int sourceTick;
    unsigned int serial;
    unsigned int commandFingerprint;
    unsigned int orderingFingerprint;
} DM1_V1_ActionSpellM11CaptureLifecycleStatePc34;

/* Final source-only capture evidence produced from the live M11 bridge route. */
typedef struct {
    int accepted;
    int finalCaptureCurrent;
    int clearStaleCapture;
    int revokeStaleCapture;
    int alreadyCurrent;
    int presentationKind;
    int graphicId;
    int zoneId;
    int companionGraphicId;
    int assetCount;
    int sourceCommandCount;
    int staleGraphicId;
    int staleZoneId;
    int staleCompanionGraphicId;
    int suppressSyntheticFallback;
    unsigned int frameTick;
    unsigned int sourceTick;
    unsigned int serial;
    unsigned int commandFingerprint;
    unsigned int orderingFingerprint;
} DM1_V1_ActionSpellM11CaptureLifecycleReceiptPc34;

/* Advances final source capture only to a newer configured original route. */
int dm1_v1_action_spell_m11_capture_lifecycle_apply_pc34(
    DM1_V1_ActionSpellM11CaptureLifecycleStatePc34 *state,
    const DM1_V1_ActionSpellSourceFrameM11LifecycleReceiptPc34 *m11,
    DM1_V1_ActionSpellM11CaptureLifecycleReceiptPc34 *outReceipt);

#endif
