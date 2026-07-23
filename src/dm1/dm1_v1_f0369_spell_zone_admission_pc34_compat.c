#include "dm1_v1_f0369_spell_zone_admission_pc34_compat.h"

#include "touch_click_zone_matrix_pc34_compat.h"

#include <string.h>

enum {
    kGraphicC009 = 9,
    kGraphicC011 = 11,
    kZoneC013 = 13,
    kFontM653 = 695,
    kFontM653Legacy = 557
};

static const char s_source_evidence[] =
    "ReDMCSB COMMAND.C:392 C100 parent -> C013; COMMAND.C:475-482 "
    "G0454 maps C101..C108 to C245..C252/C254; CLIKMENU.C "
    "F0369_COMMAND_ProcessTypes101To108_ClickInSpellSymbolsArea; "
    "CLIKMENU.C F0370 parent dispatch; MENUDRAW.C F0396 C011; "
    "CASTER.C F0394 C009; GRAPHICS.DAT M653. "
    "The shared touch matrix supplies only original layout-696 zones.";

static int point_in_zone(int x, int y, const TouchClickZonePc34Compat *zone)
{
    return zone && zone->coordMode ==
               TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT &&
           x >= zone->x && y >= zone->y &&
           x < zone->x + zone->w && y < zone->y + zone->h;
}

static int find_command_zone(unsigned int commandId,
                             TouchClickZonePc34Compat *outZone)
{
    unsigned int i;
    for (i = 0; i < TOUCHCLICK_Compat_GetZoneCount(); ++i) {
        TouchClickZonePc34Compat zone;
        if (!TOUCHCLICK_Compat_GetZone(i, &zone) ||
            zone.commandId != commandId ||
            zone.coordMode != TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT ||
            zone.buttonMask != TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT) {
            continue;
        }
        if (outZone) *outZone = zone;
        return 1;
    }
    return 0;
}

static int material_is_source_bound(
    const DM1_V1_ActionSpellHudMaterialReceiptPc34 *materials)
{
    return materials && materials->accepted && materials->drawable &&
           materials->sourceSurfaceCount == 3 &&
           materials->primaryGraphicId == kGraphicC009 &&
           materials->secondaryGraphicId == kGraphicC011 &&
           materials->primaryZoneId == kZoneC013 &&
           (materials->fontGraphicId == kFontM653 ||
            materials->fontGraphicId == kFontM653Legacy);
}

int dm1_v1_f0369_spell_zone_admit_pc34(
    const DM1_V1_F0369SpellZoneRequestPc34 *request,
    const DM1_V1_ActionSpellHudMaterialReceiptPc34 *materials,
    DM1_V1_F0369SpellZoneReceiptPc34 *outReceipt)
{
    TouchClickZonePc34Compat parent;
    TouchClickZonePc34Compat child;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!request || !material_is_source_bound(materials) ||
        request->sourceTick == 0 || request->candidatePanelActive ||
        !request->magicCasterLive ||
        !find_command_zone(DM1_V1_F0369_C100_SPELL_PARENT_PC34, &parent) ||
        !point_in_zone(request->screenX, request->screenY, &parent) ||
        !TOUCHCLICK_Compat_HitTestWithButton(
            request->screenX, request->screenY,
            TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &child) ||
        child.commandId < DM1_V1_F0369_C101_RUNE_FIRST_PC34 ||
        child.commandId > DM1_V1_F0369_C108_CAST_PC34) {
        return 0;
    }

    outReceipt->accepted = 1;
    outReceipt->commandId = (int)child.commandId;
    outReceipt->zoneIndex = (int)child.zoneIndex;
    outReceipt->runeIndex = child.commandId >= DM1_V1_F0369_C101_RUNE_FIRST_PC34 &&
                            child.commandId <= DM1_V1_F0369_C106_RUNE_LAST_PC34
        ? (int)child.commandId - DM1_V1_F0369_C101_RUNE_FIRST_PC34 : -1;
    outReceipt->recant = child.commandId == DM1_V1_F0369_C107_RECANT_PC34;
    outReceipt->cast = child.commandId == DM1_V1_F0369_C108_CAST_PC34;
    outReceipt->parentGraphicId = materials->primaryGraphicId;
    outReceipt->linesGraphicId = materials->secondaryGraphicId;
    outReceipt->fontGraphicId = materials->fontGraphicId;
    outReceipt->sourceTick = request->sourceTick;
    outReceipt->suppressSyntheticFallback = 1;
    return 1;
}

const char *dm1_v1_f0369_spell_zone_source_evidence_pc34(void)
{
    return s_source_evidence;
}
