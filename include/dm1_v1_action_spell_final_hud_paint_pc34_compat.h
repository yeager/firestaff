#ifndef FIRESTAFF_DM1_V1_ACTION_SPELL_FINAL_HUD_PAINT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ACTION_SPELL_FINAL_HUD_PAINT_PC34_COMPAT_H

#include "dm1_v1_action_spell_feedback_frame_presentation_pc34_compat.h"

/* Explicit physical HUD target rectangle owned by the original PC34 route. */
typedef struct {
    int x;
    int y;
    int w;
    int h;
} DM1_V1_ActionSpellHudPaintRectPc34;

/*
 * The final host-facing command contains only source-owned clear/blit facts.
 * No label, font, generated bitmap, or fallback paint can enter here.
 */
typedef struct {
    int accepted;
    int presentationKind;
    int resultKind;
    int championIndex;
    int clearColor;
    int sourceGraphicId;
    int sourceZoneId;
    int clearBeforeRender;
    int suppressSyntheticFallback;
    DM1_V1_ActionSpellHudPaintRectPc34 clearRect;
    DM1_V1_ActionSpellHudPaintRectPc34 renderRect;
    unsigned int frameTick;
    unsigned int sourceTick;
    unsigned int serial;
    unsigned int commandFingerprint;
    unsigned int orderingFingerprint;
    unsigned int lifecycleGeneration;
} DM1_V1_ActionSpellFinalHudPaintReceiptPc34;

/*
 * Converts one current feedback-frame receipt to its original clear/blit HUD
 * command.  It fails closed for unknown presentation identities.
 */
int dm1_v1_action_spell_final_hud_paint_build_pc34(
    const DM1_V1_ActionSpellFeedbackFramePresentationReceiptPc34 *feedbackFrame,
    DM1_V1_ActionSpellFinalHudPaintReceiptPc34 *outReceipt);

#endif
