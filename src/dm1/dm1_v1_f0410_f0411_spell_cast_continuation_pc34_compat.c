#include "dm1_v1_f0410_f0411_spell_cast_continuation_pc34_compat.h"

#include <string.h>

enum {
    kGraphicC009 = 9,
    kGraphicC011 = 11,
    kZoneC013 = 13,
    kFontM653 = 695,
    kFontM653Legacy = 557
};

static const char s_source_evidence[] =
    "ReDMCSB COMMAND.C:392,481 C100/C108; CLIKMENU.C:484-497 F0408; "
    "MENU.C:1817-1849 F0412 -> F0410 and MENU.C:1721-1749 F0411; "
    "SPELFAIL.C:2-168 F0410 failure text metadata; CASTER.C F0394 C009; "
    "MENUDRAW.C F0396 C011; GRAPHICS.DAT M653. F0412 remains the sole "
    "owner of spell results and mutations.";

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

static int cast_chain_is_source_bound(
    const DM1_V1_F0410F0411SpellCastRequestPc34 *request,
    const DM1_V1_F0369SpellZoneReceiptPc34 *zoneReceipt,
    const DM1_V1_F0408F0409SpellCastReceiptPc34 *castReceipt,
    const DM1_V1_ActionSpellHudMaterialReceiptPc34 *materials)
{
    return request && zoneReceipt && castReceipt && material_is_source_bound(materials) &&
           request->magicCasterLive && request->casterIndex >= 0 &&
           request->casterIndex < 4 && request->sourceTick != 0 &&
           zoneReceipt->accepted && zoneReceipt->cast &&
           zoneReceipt->commandId == DM1_V1_F0369_C108_CAST_PC34 &&
           zoneReceipt->sourceTick == request->sourceTick &&
           zoneReceipt->suppressSyntheticFallback &&
           zoneReceipt->parentGraphicId == kGraphicC009 &&
           zoneReceipt->linesGraphicId == kGraphicC011 &&
           (zoneReceipt->fontGraphicId == kFontM653 ||
            zoneReceipt->fontGraphicId == kFontM653Legacy) &&
           castReceipt->accepted && castReceipt->commandId ==
               DM1_V1_F0369_C108_CAST_PC34 &&
           castReceipt->f0412DispatchRequired &&
           castReceipt->sourceTick == request->sourceTick &&
           castReceipt->suppressSyntheticFallback;
}

static int find_empty_flask(const DM1_SpellPotionInventory *inventory,
                            int *outSlotIndex,
                            unsigned short *outThing)
{
    static const int kSlotOrder[DM1_SPELL_HAND_SLOT_COUNT] = {
        DM1_SPELL_SLOT_ACTION_HAND,
        DM1_SPELL_SLOT_READY_HAND
    };
    int orderIndex;

    if (outSlotIndex) *outSlotIndex = -1;
    if (outThing) *outThing = DM1_SPELL_THING_NONE_PC34;
    if (!inventory || inventory->potionCount < 0 || inventory->potionCount > 4) {
        return 0;
    }
    for (orderIndex = 0; orderIndex < DM1_SPELL_HAND_SLOT_COUNT; ++orderIndex) {
        int slotIndex = kSlotOrder[orderIndex];
        unsigned short thing = inventory->slots[slotIndex];
        int potionIndex;
        if (thing == DM1_SPELL_THING_NONE_PC34) continue;
        for (potionIndex = 0; potionIndex < inventory->potionCount; ++potionIndex) {
            const DM1_SpellPotionObject *potion = &inventory->potions[potionIndex];
            if (potion->thing == thing &&
                potion->iconIndex == DM1_SPELL_ICON_EMPTY_FLASK_PC34 &&
                potion->type == DM1_SPELL_POTION_EMPTY_FLASK_PC34) {
                if (outSlotIndex) *outSlotIndex = slotIndex;
                if (outThing) *outThing = thing;
                return 1;
            }
        }
    }
    return 0;
}

int dm1_v1_f0410_f0411_spell_cast_continue_pc34(
    const DM1_V1_F0410F0411SpellCastRequestPc34 *request,
    const DM1_V1_F0369SpellZoneReceiptPc34 *zoneReceipt,
    const DM1_V1_F0408F0409SpellCastReceiptPc34 *castReceipt,
    const DM1_V1_ActionSpellHudMaterialReceiptPc34 *materials,
    const DM1_SpellF0412RuntimeReceipt *runtimeReceipt,
    const DM1_SpellPotionInventory *inventory,
    DM1_V1_F0410F0411SpellCastReceiptPc34 *outReceipt)
{
    const DM1_SpellFailureFeedback *feedback;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    outReceipt->flaskSlotIndex = -1;
    outReceipt->flaskThing = DM1_SPELL_THING_NONE_PC34;
    if (!cast_chain_is_source_bound(request, zoneReceipt, castReceipt, materials) ||
        !runtimeReceipt) {
        return 0;
    }

    outReceipt->accepted = 1;
    outReceipt->failureType = runtimeReceipt->failureType;
    outReceipt->sourceTick = request->sourceTick;
    outReceipt->suppressSyntheticFallback = 1;

    if (runtimeReceipt->castResult != DM1_SPELL_CAST_SUCCESS) {
        feedback = dm1_spell_failureFeedback(runtimeReceipt->failureType);
        if (!feedback) {
            memset(outReceipt, 0, sizeof(*outReceipt));
            outReceipt->flaskSlotIndex = -1;
            outReceipt->flaskThing = DM1_SPELL_THING_NONE_PC34;
            return 0;
        }
        outReceipt->failureFeedbackAdmitted = 1;
        outReceipt->messageColor = feedback->messageColor;
        outReceipt->printsChampionName = feedback->printsChampionName;
        outReceipt->appendsBaseSkillName = feedback->appendsBaseSkillName;
    }

    if (runtimeReceipt->spellKind == DM1_SPELL_KIND_POTION &&
        find_empty_flask(inventory, &outReceipt->flaskSlotIndex,
                         &outReceipt->flaskThing)) {
        outReceipt->flaskFound = 1;
    }
    return 1;
}

const char *dm1_v1_f0410_f0411_spell_cast_source_evidence_pc34(void)
{
    return s_source_evidence;
}
