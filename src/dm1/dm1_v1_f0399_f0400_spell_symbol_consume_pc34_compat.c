#include "dm1_v1_f0399_f0400_spell_symbol_consume_pc34_compat.h"

#include <string.h>

enum {
    kGraphicC009 = 9,
    kGraphicC011 = 11,
    kZoneC013 = 13,
    kFontM653 = 695,
    kFontM653Legacy = 557
};

static const char s_source_evidence[] =
    "ReDMCSB COMMAND.C:392,475-482 C100/C101..C108; CLIKMENU.C "
    "F0369/F0370; SYMBOL.C F0399_MENUS_AddChampionSymbol:17-39; "
    "SYMBOL.C F0400_MENUS_DeleteChampionSymbol:85-103; CASTER.C "
    "F0394 C009; MENUDRAW.C F0396 C011; GRAPHICS.DAT M653. "
    "C108 dispatches to F0412 and is not consumed here.";

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

int dm1_v1_f0399_f0400_spell_symbol_consume_pc34(
    const DM1_V1_F0399F0400SpellSymbolRequestPc34 *request,
    const DM1_V1_F0369SpellZoneReceiptPc34 *zoneReceipt,
    const DM1_V1_ActionSpellHudMaterialReceiptPc34 *materials,
    DM1_SpellCastingState *spellState,
    DM1_ChampionSpellStats *casterStats,
    DM1_V1_F0399F0400SpellSymbolReceiptPc34 *outReceipt)
{
    DM1_ChampionSpellInput *input;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!request || !zoneReceipt || !material_is_source_bound(materials) ||
        !spellState || !casterStats || !request->magicCasterLive ||
        request->casterIndex < 0 || request->casterIndex >= 4 ||
        request->sourceTick == 0 || !zoneReceipt->accepted ||
        zoneReceipt->sourceTick != request->sourceTick ||
        !zoneReceipt->suppressSyntheticFallback ||
        zoneReceipt->parentGraphicId != kGraphicC009 ||
        zoneReceipt->linesGraphicId != kGraphicC011 ||
        (zoneReceipt->fontGraphicId != kFontM653 &&
         zoneReceipt->fontGraphicId != kFontM653Legacy)) {
        return 0;
    }

    input = &spellState->input[request->casterIndex];
    outReceipt->commandId = zoneReceipt->commandId;
    outReceipt->symbolIndex = zoneReceipt->runeIndex;
    outReceipt->symbolStepBefore = input->symbolStep;
    outReceipt->symbolStepAfter = input->symbolStep;
    outReceipt->manaBefore = casterStats->currentMana;
    outReceipt->manaAfter = casterStats->currentMana;
    outReceipt->sourceTick = request->sourceTick;

    if (zoneReceipt->commandId >= DM1_V1_F0369_C101_RUNE_FIRST_PC34 &&
        zoneReceipt->commandId <= DM1_V1_F0369_C106_RUNE_LAST_PC34 &&
        zoneReceipt->runeIndex >= 0 && zoneReceipt->runeIndex < 6) {
        if (!dm1_spell_addSymbol(spellState, request->casterIndex,
                                 casterStats, zoneReceipt->runeIndex)) {
            return 0;
        }
        outReceipt->symbolAdded = 1;
    } else if (zoneReceipt->commandId == DM1_V1_F0369_C107_RECANT_PC34 &&
               zoneReceipt->recant) {
        if (input->symbols[0] != '\0') {
            dm1_spell_deleteSymbol(spellState, request->casterIndex);
            outReceipt->symbolDeleted = 1;
        }
    } else {
        return 0;
    }

    outReceipt->accepted = 1;
    outReceipt->symbolStepAfter = input->symbolStep;
    outReceipt->manaAfter = casterStats->currentMana;
    outReceipt->suppressSyntheticFallback = 1;
    return 1;
}

const char *dm1_v1_f0399_f0400_spell_symbol_source_evidence_pc34(void)
{
    return s_source_evidence;
}
