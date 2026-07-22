#ifndef FIRESTAFF_DM1_V1_ACTION_SPELL_FINAL_CAPTURE_M11_GATE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ACTION_SPELL_FINAL_CAPTURE_M11_GATE_PC34_COMPAT_H

#include "dm1_v1_action_spell_m11_capture_lifecycle_pc34_compat.h"

/* Current configured-original capture route allowed to reach an M11 consumer. */
typedef struct {
    int accepted;
    int m11CaptureGateOpen;
    int presentationKind;
    int originalGraphicId;
    int originalZoneId;
    int companionGraphicId;
    int sourceAssetCount;
    int sourceCommandCount;
    int suppressSyntheticFallback;
    unsigned int frameTick;
    unsigned int sourceTick;
    unsigned int serial;
    unsigned int commandFingerprint;
    unsigned int orderingFingerprint;
} DM1_V1_ActionSpellFinalCaptureM11GateReceiptPc34;

/* Opens only for current configured C009/C010/C011/C013 capture evidence. */
int dm1_v1_action_spell_final_capture_m11_gate_build_pc34(
    const DM1_V1_ActionSpellM11CaptureLifecycleReceiptPc34 *capture,
    DM1_V1_ActionSpellFinalCaptureM11GateReceiptPc34 *outReceipt);

#endif
