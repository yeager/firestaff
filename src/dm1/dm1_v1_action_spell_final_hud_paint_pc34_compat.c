#include "dm1_v1_action_spell_final_hud_paint_pc34_compat.h"

#include "firestaff/dm1/v1/box_action_area_pc34_compat.h"
#include "firestaff/dm1/v1/box_spell_area_pc34_compat.h"

#include <string.h>

static void
dm1_v1_action_spell_final_hud_paint_set_rect_pc34(
    DM1_V1_ActionSpellHudPaintRectPc34 *outRect,
    int x, int y, int w, int h)
{
    outRect->x = x;
    outRect->y = y;
    outRect->w = w;
    outRect->h = h;
}

int
dm1_v1_action_spell_final_hud_paint_build_pc34(
    const DM1_V1_ActionSpellFeedbackFramePresentationReceiptPc34 *feedbackFrame,
    DM1_V1_ActionSpellFinalHudPaintReceiptPc34 *outReceipt)
{
    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!feedbackFrame || !feedbackFrame->accepted ||
        !feedbackFrame->commandRepaintCurrent ||
        !feedbackFrame->suppressSyntheticFallback ||
        feedbackFrame->frameTick == 0 || feedbackFrame->sourceTick == 0 ||
        feedbackFrame->serial == 0 || feedbackFrame->commandFingerprint == 0 ||
        feedbackFrame->orderingFingerprint == 0 ||
        feedbackFrame->lifecycleGeneration == 0) {
        return 0;
    }

    outReceipt->presentationKind = feedbackFrame->presentationKind;
    outReceipt->resultKind = feedbackFrame->resultKind;
    outReceipt->championIndex = feedbackFrame->championIndex;
    outReceipt->clearColor = DM1_V1_ACTION_AREA_CLEAR_COLOR_PC34;
    outReceipt->clearBeforeRender = 1;
    outReceipt->suppressSyntheticFallback = 1;
    switch (feedbackFrame->presentationKind) {
        case DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34:
            /* DATA.C G0001 and ACTIDRAW.C F0387: clear physical box, C010 blit. */
            dm1_v1_action_spell_final_hud_paint_set_rect_pc34(
                &outReceipt->clearRect, 224, 77, 96, 45);
            dm1_v1_action_spell_final_hud_paint_set_rect_pc34(
                &outReceipt->renderRect, 233, 77, 87, 45);
            outReceipt->sourceGraphicId = DM1_V1_ACTION_AREA_GRAPHIC_ID_PC34;
            outReceipt->sourceZoneId = DM1_V1_ACTION_AREA_ZONE_ID_PC34;
            break;
        case DM1_V1_ACTION_HUD_PRESENTATION_SPELL_POTION_PC34:
        case DM1_V1_ACTION_HUD_PRESENTATION_SPELL_PROJECTILE_PC34:
        case DM1_V1_ACTION_HUD_PRESENTATION_SPELL_EFFECT_PC34:
            /* DATA.C G0000 and CASTER.C F0394: clear physical box, C009 blit. */
            dm1_v1_action_spell_final_hud_paint_set_rect_pc34(
                &outReceipt->clearRect, 224, 42, 96, 33);
            dm1_v1_action_spell_final_hud_paint_set_rect_pc34(
                &outReceipt->renderRect, 233, 42, 87, 25);
            outReceipt->sourceGraphicId = DM1_V1_SPELL_AREA_BACKGROUND_GRAPHIC_ID_PC34;
            outReceipt->sourceZoneId = DM1_V1_SPELL_AREA_ZONE_ID_PC34;
            break;
        default:
            return 0;
    }

    outReceipt->accepted = 1;
    outReceipt->frameTick = feedbackFrame->frameTick;
    outReceipt->sourceTick = feedbackFrame->sourceTick;
    outReceipt->serial = feedbackFrame->serial;
    outReceipt->commandFingerprint = feedbackFrame->commandFingerprint;
    outReceipt->orderingFingerprint = feedbackFrame->orderingFingerprint;
    outReceipt->lifecycleGeneration = feedbackFrame->lifecycleGeneration;
    return 1;
}
