#include "dm1_v1_f0408_f0409_spell_cast_admission_pc34_compat.h"

#include <string.h>

enum {
    kGraphicC009 = 9,
    kGraphicC011 = 11,
    kZoneC013 = 13,
    kFontM653 = 695,
    kFontM653Legacy = 557
};

static const char s_source_evidence[] =
    "ReDMCSB COMMAND.C:392,481 C100/C108; CLIKMENU.C F0369/F0370; "
    "CLIKMENU.C:484-497 F0408_MENUS_GetClickOnSpellCastResult; MENU.C "
    "F0409_MENUS_GetSpellFromSymbols:1685-1705; CASTER.C F0394 C009; "
    "MENUDRAW.C F0396 C011; GRAPHICS.DAT M653. F0412 owns cast result "
    "and all state mutation after this admission.";

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

static int spell_index_from_pointer(const DM1_Spell *spell)
{
    int index;

    if (!spell) return -1;
    for (index = 0; index < DM1_SPELL_COUNT; ++index) {
        if (spell == &dm1_spells[index]) return index;
    }
    return -1;
}

int dm1_v1_f0408_f0409_spell_cast_admit_pc34(
    const DM1_V1_F0408F0409SpellCastRequestPc34 *request,
    const DM1_V1_F0369SpellZoneReceiptPc34 *zoneReceipt,
    const DM1_V1_ActionSpellHudMaterialReceiptPc34 *materials,
    const DM1_SpellCastingState *spellState,
    DM1_V1_F0408F0409SpellCastReceiptPc34 *outReceipt)
{
    const DM1_Spell *spell;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    outReceipt->spellIndex = -1;
    if (!request || !zoneReceipt || !spellState ||
        !material_is_source_bound(materials) || !request->magicCasterLive ||
        request->casterIndex < 0 || request->casterIndex >= 4 ||
        request->sourceTick == 0 || !zoneReceipt->accepted ||
        zoneReceipt->commandId != DM1_V1_F0369_C108_CAST_PC34 ||
        !zoneReceipt->cast || zoneReceipt->sourceTick != request->sourceTick ||
        !zoneReceipt->suppressSyntheticFallback ||
        zoneReceipt->parentGraphicId != kGraphicC009 ||
        zoneReceipt->linesGraphicId != kGraphicC011 ||
        (zoneReceipt->fontGraphicId != kFontM653 &&
         zoneReceipt->fontGraphicId != kFontM653Legacy)) {
        return 0;
    }

    spell = dm1_spell_lookup(spellState, request->casterIndex);
    outReceipt->accepted = 1;
    outReceipt->commandId = zoneReceipt->commandId;
    outReceipt->lookupMatched = spell != NULL;
    outReceipt->spellIndex = spell_index_from_pointer(spell);
    outReceipt->powerOrdinal =
        spellState->input[request->casterIndex].symbols[0] != '\0'
            ? (int)(unsigned char)spellState->input[request->casterIndex].symbols[0] - 95
            : 0;
    outReceipt->f0412DispatchRequired = 1;
    outReceipt->sourceTick = request->sourceTick;
    outReceipt->suppressSyntheticFallback = 1;
    return 1;
}

const char *dm1_v1_f0408_f0409_spell_cast_source_evidence_pc34(void)
{
    return s_source_evidence;
}
