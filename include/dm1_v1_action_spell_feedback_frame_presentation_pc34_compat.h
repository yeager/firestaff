#ifndef FIRESTAFF_DM1_V1_ACTION_SPELL_FEEDBACK_FRAME_PRESENTATION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ACTION_SPELL_FEEDBACK_FRAME_PRESENTATION_PC34_COMPAT_H

#include "dm1_v1_action_spell_result_feedback_pc34_compat.h"
#include "dm1_v1_action_spell_presentation_lifecycle_pc34_compat.h"

/*
 * ReDMCSB F0407/F0412 result feedback may reach the frame only through an
 * existing original command order and its current clear/repaint lifecycle.
 * This receipt holds no text, bitmap, color, or host fallback state.
 */
typedef struct {
    int accepted;
    int resultKind;
    int presentationKind;
    int championIndex;
    int inputZoneId;
    int commandRepaintCurrent;
    int suppressSyntheticFallback;
    unsigned int frameTick;
    unsigned int sourceTick;
    unsigned int serial;
    unsigned int commandFingerprint;
    unsigned int orderingFingerprint;
    unsigned int lifecycleGeneration;
} DM1_V1_ActionSpellFeedbackFramePresentationReceiptPc34;

/*
 * Admits feedback only when it describes the currently repainted original
 * command frame, including the exact command and order identities.
 */
int dm1_v1_action_spell_feedback_frame_presentation_build_pc34(
    const DM1_V1_ActionSpellResultFeedbackReceiptPc34 *feedback,
    const DM1_V1_ActionSpellCommandFrameOrderReceiptPc34 *order,
    const DM1_V1_ActionSpellPresentationLifecycleReceiptPc34 *lifecycle,
    DM1_V1_ActionSpellFeedbackFramePresentationReceiptPc34 *outReceipt);

#endif
