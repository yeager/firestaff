#include "dm1_v1_action_spell_final_capture_m11_gate_pc34_compat.h"

#include "firestaff/dm1/v1/box_action_area_pc34_compat.h"
#include "firestaff/dm1/v1/box_spell_area_pc34_compat.h"

#include <string.h>

static int configured_proof(int g, int z, int c, int n)
{
    return (g == DM1_V1_ACTION_AREA_GRAPHIC_ID_PC34 && z == DM1_V1_ACTION_AREA_ZONE_ID_PC34 && c == 0 && n == 1) ||
           (g == DM1_V1_SPELL_AREA_BACKGROUND_GRAPHIC_ID_PC34 && z == DM1_V1_SPELL_AREA_ZONE_ID_PC34 &&
            c == DM1_V1_SPELL_AREA_LINES_GRAPHIC_ID_PC34 && n == 2);
}

int dm1_v1_action_spell_final_capture_m11_gate_build_pc34(
    const DM1_V1_ActionSpellM11CaptureLifecycleReceiptPc34 *capture,
    DM1_V1_ActionSpellFinalCaptureM11GateReceiptPc34 *outReceipt)
{
    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!capture || !capture->accepted || !capture->finalCaptureCurrent ||
        !capture->suppressSyntheticFallback || capture->sourceCommandCount <= 0 ||
        capture->frameTick == 0 || capture->sourceTick == 0 || capture->serial == 0 ||
        capture->commandFingerprint == 0 || capture->orderingFingerprint == 0 ||
        !configured_proof(capture->graphicId, capture->zoneId,
                          capture->companionGraphicId, capture->assetCount)) return 0;
    if ((capture->presentationKind == DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34 &&
         capture->graphicId != 10) ||
        ((capture->presentationKind < DM1_V1_ACTION_HUD_PRESENTATION_SPELL_PROJECTILE_PC34 ||
          capture->presentationKind > DM1_V1_ACTION_HUD_PRESENTATION_SPELL_POTION_PC34) &&
         capture->presentationKind != DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34)) return 0;
    outReceipt->accepted = 1; outReceipt->m11CaptureGateOpen = 1;
    outReceipt->presentationKind = capture->presentationKind;
    outReceipt->originalGraphicId = capture->graphicId; outReceipt->originalZoneId = capture->zoneId;
    outReceipt->companionGraphicId = capture->companionGraphicId;
    outReceipt->sourceAssetCount = capture->assetCount; outReceipt->sourceCommandCount = capture->sourceCommandCount;
    outReceipt->suppressSyntheticFallback = 1; outReceipt->frameTick = capture->frameTick;
    outReceipt->sourceTick = capture->sourceTick; outReceipt->serial = capture->serial;
    outReceipt->commandFingerprint = capture->commandFingerprint;
    outReceipt->orderingFingerprint = capture->orderingFingerprint;
    return 1;
}
