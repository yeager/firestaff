#ifndef FIRESTAFF_DM1_V1_ACTION_SPELL_PRESENTATION_LIFECYCLE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ACTION_SPELL_PRESENTATION_LIFECYCLE_PC34_COMPAT_H

#include "dm1_v1_action_spell_command_frame_order_pc34_compat.h"

/*
 * ReDMCSB F0407/F0412 redraw lifecycle.  The state expresses only original
 * clear-before-repaint ordering.  It owns no bitmap, text, fallback, or host
 * presentation state.
 */
typedef struct {
    unsigned int frameTick;
    int frameOpen;
    int active;
    int clearRequired;
    int presentationKind;
    unsigned int sourceTick;
    unsigned int serial;
    unsigned int commandFingerprint;
    unsigned int lifecycleGeneration;
} DM1_V1_ActionSpellPresentationLifecycleStatePc34;

typedef struct {
    int accepted;
    int clearPrevious;
    int repainted;
    int alreadyCurrent;
    int presentationKind;
    unsigned int frameTick;
    unsigned int sourceTick;
    unsigned int serial;
    unsigned int commandFingerprint;
    unsigned int lifecycleGeneration;
} DM1_V1_ActionSpellPresentationLifecycleReceiptPc34;

/* Opens a frame and marks the previous source presentation for clear. */
void dm1_v1_action_spell_presentation_lifecycle_begin_frame_pc34(
    DM1_V1_ActionSpellPresentationLifecycleStatePc34 *state,
    unsigned int frameTick);

/*
 * Consumes one already ordered source command frame. A prior frame is cleared
 * before this order is repainted; stale or divergent order/frame facts reject.
 */
int dm1_v1_action_spell_presentation_lifecycle_apply_pc34(
    DM1_V1_ActionSpellPresentationLifecycleStatePc34 *state,
    const DM1_V1_ActionSpellPresentationFrameStatePc34 *frameState,
    const DM1_V1_ActionSpellCommandFrameOrderReceiptPc34 *order,
    DM1_V1_ActionSpellPresentationLifecycleReceiptPc34 *outReceipt);

#endif
