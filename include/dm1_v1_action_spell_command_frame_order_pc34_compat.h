#ifndef FIRESTAFF_DM1_V1_ACTION_SPELL_COMMAND_FRAME_ORDER_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ACTION_SPELL_COMMAND_FRAME_ORDER_PC34_COMPAT_H

#include "dm1_v1_action_spell_presentation_apply_pc34_compat.h"

/*
 * ReDMCSB frame ordering for F0407/F0412 presentation.  This is an ordering
 * receipt only: it never renders, adds text, creates a font, or substitutes a
 * missing surface.
 */
typedef struct {
    int accepted;
    int readyForPresentation;
    int presentationKind;
    int commandCount;
    unsigned int frameTick;
    unsigned int sourceTick;
    unsigned int serial;
    unsigned int commandFingerprint;
    unsigned int orderingFingerprint;
    int orderedSurfaceIndices[DM1_V1_ACTION_SPELL_RENDER_COMMAND_MAX_PC34];
} DM1_V1_ActionSpellCommandFrameOrderReceiptPc34;

/*
 * Revalidates the applied frame state and source material, then proves that
 * the command order is the original F0407/F0412 material order.
 */
int dm1_v1_action_spell_command_frame_order_build_pc34(
    const DM1_V1_ActionSpellPresentationFrameStatePc34 *state,
    const DM1_V1_ActionSpellPresentationApplyReceiptPc34 *applyReceipt,
    const DM1_V1_ActionSpellHudMaterialSetPc34 *materials,
    DM1_V1_ActionSpellCommandFrameOrderReceiptPc34 *outReceipt);

#endif
