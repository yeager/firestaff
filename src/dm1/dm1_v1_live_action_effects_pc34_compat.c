#include "dm1_v1_live_action_effects_pc34_compat.h"

#include "firestaff/dm1/v1/box_action_area_pc34_compat.h"
#include "firestaff/dm1/v1/box_spell_area_pc34_compat.h"

#include <string.h>

enum {
    /* ReDMCSB MELEE.C C014 / G2093-G2096. */
    kGraphicCreatureDamage = 14,
    /* ReDMCSB GRAPHICS.DAT M653 source-font entries. */
    kGraphicHudFontPrimary = 695,
    kGraphicHudFontAlternate = 557
};

static const DM1_V1_ActionSpellHudSurfacePc34 *
dm1_v1_live_action_find_surface_pc34(
    const DM1_V1_ActionSpellHudMaterialSetPc34 *materials,
    int graphicId,
    int expectedWidth,
    int expectedHeight)
{
    int i;
    if (!materials || !materials->surfaces || materials->surfaceCount <= 0) {
        return 0;
    }
    for (i = 0; i < materials->surfaceCount; ++i) {
        const DM1_V1_ActionSpellHudSurfacePc34 *surface =
            &materials->surfaces[i];
        if (surface->graphicId != graphicId || !surface->sourceOwned ||
            !surface->pixels || surface->pixelCount <= 0) {
            continue;
        }
        if (expectedWidth > 0 && surface->width != expectedWidth) continue;
        if (expectedHeight > 0 && surface->height != expectedHeight) continue;
        if (expectedWidth > 0 && expectedHeight > 0 &&
            surface->pixelCount < expectedWidth * expectedHeight) {
            continue;
        }
        return surface;
    }
    return 0;
}

static int
dm1_v1_live_action_bind_surface_pc34(
    const DM1_V1_ActionSpellHudMaterialSetPc34 *materials,
    int graphicId,
    int width,
    int height,
    DM1_V1_ActionSpellHudMaterialReceiptPc34 *receipt,
    int slot)
{
    if (!dm1_v1_live_action_find_surface_pc34(
            materials, graphicId, width, height)) {
        return 0;
    }
    if (slot == 0) receipt->primaryGraphicId = graphicId;
    else if (slot == 1) receipt->secondaryGraphicId = graphicId;
    else receipt->fontGraphicId = graphicId;
    ++receipt->sourceSurfaceCount;
    return 1;
}

const char *dm1_v1_live_action_effects_source_evidence_pc34(void)
{
    return "ReDMCSB MENU.C F0407:1053-1056,1270,1398-1541,1613-1628; "
           "MENU.C F0412:1817-2039; PROJEXPL.C F0231:1416-1550; "
           "TIMELINE.C F0253:1574-1605; PANEL.C F0346/F0347 spell/action HUD redraw";
}

void dm1_v1_live_action_effects_reset_pc34(DM1_V1_LiveActionEffectsPc34 *effects)
{
    if (effects) memset(effects, 0, sizeof(*effects));
}

int dm1_v1_live_action_effect_materialize_pc34(
    DM1_V1_LiveActionEffectsPc34 *effects,
    const DM1_V1_LiveActionEffectInputPc34 *input,
    DM1_V1_LiveActionEffectReceiptPc34 *outReceipt)
{
    int i;
    int slot = -1;
    if (outReceipt) memset(outReceipt, 0, sizeof(*outReceipt));
    if (!effects || !input || input->kind <= 0) return 0;
    /* F0407 has one live action-lock per champion.  Replace it in-place so
     * a later action in the same tick cannot leave an older timer alive. */
    if (input->kind == DM1_V1_LIVE_ACTION_EFFECT_ACTION_LOCK_PC34) {
        for (i = 0; i < effects->count; ++i) {
            if (effects->entries[i].kind == input->kind &&
                effects->entries[i].championIndex == input->championIndex) {
                slot = i;
                break;
            }
        }
    }
    if (slot < 0) {
        if (effects->count < DM1_V1_LIVE_ACTION_EFFECT_CAPACITY_PC34) {
            slot = effects->count++;
        } else {
            slot = 0;
            for (i = 1; i < effects->count; ++i) {
                if (effects->entries[i].serial < effects->entries[slot].serial) slot = i;
            }
        }
    }
    effects->entries[slot].kind = input->kind;
    effects->entries[slot].championIndex = input->championIndex;
    effects->entries[slot].actionIndex = input->actionIndex;
    effects->entries[slot].damage = input->damage;
    effects->entries[slot].combatOutcome = input->combatOutcome;
    effects->entries[slot].defenseDelta = input->defenseDelta;
    effects->entries[slot].doorAffected = input->doorAffected;
    effects->entries[slot].remainingTicks = (unsigned char)(input->disabledTicks > 255 ? 255 : input->disabledTicks);
    effects->entries[slot].sourceTick = input->sourceTick;
    effects->entries[slot].serial = ++effects->nextSerial;
    if (outReceipt) {
        outReceipt->valid = 1;
        outReceipt->materialized = 1;
        outReceipt->replaced = slot < effects->count - 1;
        outReceipt->slot = slot;
    }
    return 1;
}

static int dm1_v1_live_action_champion_index_valid(int championIndex)
{
    return championIndex >= 0 && championIndex < 4;
}

static void dm1_v1_live_action_receipt_base(
    const DM1_V1_LiveActionEffectPc34 *effect,
    DM1_V1_ActionSpellHudPresentationReceiptPc34 *receipt)
{
    memset(receipt, 0, sizeof(*receipt));
    receipt->sourceEffectKind = effect->kind;
    receipt->championIndex = effect->championIndex;
    receipt->actionIndex = effect->actionIndex;
    receipt->damage = effect->damage;
    receipt->combatOutcome = effect->combatOutcome;
    receipt->remainingTicks = effect->remainingTicks;
    receipt->sourceTick = effect->sourceTick;
    receipt->serial = effect->serial;
    receipt->requiresSourceFont = 1;
    receipt->suppressSyntheticFallback = 1;
    receipt->requiredFontGraphicId = kGraphicHudFontPrimary;
    receipt->sourceAnchor = dm1_v1_live_action_effects_source_evidence_pc34();
}

int dm1_v1_live_action_effect_hud_presentation_pc34(
    const DM1_V1_LiveActionEffectPc34 *effect,
    DM1_V1_ActionSpellHudPresentationReceiptPc34 *outReceipt)
{
    if (!effect || !outReceipt) return 0;
    dm1_v1_live_action_receipt_base(effect, outReceipt);
    switch (effect->kind) {
        case DM1_V1_LIVE_ACTION_EFFECT_DAMAGE_PC34:
            if (!dm1_v1_live_action_champion_index_valid(effect->championIndex) ||
                effect->damage <= 0) {
                return 0;
            }
            outReceipt->valid = 1;
            outReceipt->drawable = 1;
            outReceipt->presentationKind = DM1_V1_ACTION_HUD_PRESENTATION_DAMAGE_PC34;
            outReceipt->layoutKind = DM1_V1_ACTION_HUD_LAYOUT_CREATURE_DAMAGE_PC34;
            outReceipt->textColor = 4;
            outReceipt->requiredPrimaryGraphicId = kGraphicCreatureDamage;
            outReceipt->requiredPrimaryZoneId =
                DM1_V1_ACTION_RESULT_ZONE_ID_PC34;
            return 1;
        case DM1_V1_LIVE_ACTION_EFFECT_MISS_PC34:
            if (!dm1_v1_live_action_champion_index_valid(effect->championIndex)) return 0;
            outReceipt->valid = 1;
            outReceipt->drawable = 1;
            outReceipt->presentationKind = DM1_V1_ACTION_HUD_PRESENTATION_MISS_PC34;
            outReceipt->layoutKind = DM1_V1_ACTION_HUD_LAYOUT_MESSAGE_LINE_PC34;
            outReceipt->textColor = 4;
            /* MENU.C owns the message text; no host label is emitted here. */
            return 1;
        case DM1_V1_LIVE_ACTION_EFFECT_DOOR_PC34:
            if (!effect->doorAffected) return 0;
            outReceipt->valid = 1;
            outReceipt->drawable = 1;
            outReceipt->presentationKind = DM1_V1_ACTION_HUD_PRESENTATION_DOOR_PC34;
            outReceipt->layoutKind = DM1_V1_ACTION_HUD_LAYOUT_MESSAGE_LINE_PC34;
            /* F0407 owns the message text; no local replacement is valid. */
            return 1;
        case DM1_V1_LIVE_ACTION_EFFECT_ACTION_LOCK_PC34:
            if (!dm1_v1_live_action_champion_index_valid(effect->championIndex) ||
                effect->actionIndex < 0 || effect->remainingTicks == 0) {
                return 0;
            }
            outReceipt->valid = 1;
            outReceipt->drawable = 1;
            outReceipt->presentationKind = DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34;
            outReceipt->layoutKind = DM1_V1_ACTION_HUD_LAYOUT_ACTION_MENU_ROW_PC34;
            outReceipt->requiresRealActionMenuLayout = 1;
            outReceipt->requiredPrimaryGraphicId =
                DM1_V1_ACTION_AREA_GRAPHIC_ID_PC34;
            outReceipt->requiredPrimaryZoneId =
                DM1_V1_ACTION_AREA_ZONE_ID_PC34;
            return 1;
        case DM1_V1_LIVE_ACTION_EFFECT_SPELL_PC34:
            if (!dm1_v1_live_action_champion_index_valid(effect->championIndex) ||
                effect->damage <= 0) {
                return 0;
            }
            outReceipt->valid = 1;
            outReceipt->drawable = 1;
            outReceipt->spellPowerOrdinal = effect->damage;
            outReceipt->spellKind = effect->combatOutcome;
            outReceipt->layoutKind = DM1_V1_ACTION_HUD_LAYOUT_SPELL_AREA_PC34;
            outReceipt->requiresRealSpellAreaLayout = 1;
            outReceipt->requiredPrimaryGraphicId =
                DM1_V1_SPELL_AREA_BACKGROUND_GRAPHIC_ID_PC34;
            outReceipt->requiredSecondaryGraphicId =
                DM1_V1_SPELL_AREA_LINES_GRAPHIC_ID_PC34;
            outReceipt->requiredPrimaryZoneId = DM1_V1_SPELL_AREA_ZONE_ID_PC34;
            if (effect->combatOutcome == 1) {
                outReceipt->presentationKind = DM1_V1_ACTION_HUD_PRESENTATION_SPELL_POTION_PC34;
            } else if (effect->combatOutcome == 2) {
                outReceipt->presentationKind = DM1_V1_ACTION_HUD_PRESENTATION_SPELL_PROJECTILE_PC34;
            } else if (effect->combatOutcome == 3 || effect->combatOutcome == 4) {
                outReceipt->presentationKind = DM1_V1_ACTION_HUD_PRESENTATION_SPELL_EFFECT_PC34;
            } else {
                memset(outReceipt, 0, sizeof(*outReceipt));
                return 0;
            }
            return 1;
        default:
            memset(outReceipt, 0, sizeof(*outReceipt));
            return 0;
    }
}

int dm1_v1_live_action_spell_failure_hud_presentation_f0412_pc34(
    const DM1_V1_SpellFailureHudFeedbackPc34 *feedback,
    int championIndex,
    DM1_V1_ActionSpellHudPresentationReceiptPc34 *outReceipt)
{
    if (!feedback || !outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!dm1_v1_live_action_champion_index_valid(championIndex) ||
        !feedback->messageBeforeSkill || !feedback->messageAfterSkill ||
        feedback->messageColor < 0) {
        return 0;
    }
    outReceipt->valid = 1;
    outReceipt->drawable = 1;
    outReceipt->sourceEffectKind = DM1_V1_LIVE_ACTION_EFFECT_SPELL_PC34;
    outReceipt->presentationKind = DM1_V1_ACTION_HUD_PRESENTATION_SPELL_FAILURE_PC34;
    outReceipt->layoutKind = DM1_V1_ACTION_HUD_LAYOUT_SPELL_AREA_PC34;
    outReceipt->championIndex = championIndex;
    outReceipt->textColor = feedback->messageColor;
    outReceipt->requiresSourceFont = 1;
    outReceipt->requiredPrimaryGraphicId =
        DM1_V1_SPELL_AREA_BACKGROUND_GRAPHIC_ID_PC34;
    outReceipt->requiredSecondaryGraphicId =
        DM1_V1_SPELL_AREA_LINES_GRAPHIC_ID_PC34;
    outReceipt->requiredPrimaryZoneId = DM1_V1_SPELL_AREA_ZONE_ID_PC34;
    outReceipt->requiredFontGraphicId = kGraphicHudFontAlternate;
    outReceipt->requiresRealSpellAreaLayout = 1;
    outReceipt->suppressSyntheticFallback = 1;
    outReceipt->printsLineFeed = feedback->printsLineFeed;
    outReceipt->printsChampionName = feedback->printsChampionName;
    outReceipt->appendsBaseSkillName = feedback->appendsBaseSkillName;
    outReceipt->clearsSymbolsOnCastClick = feedback->clearsSymbolsOnCastClick;
    outReceipt->redrawsAvailableSymbols = feedback->redrawsAvailableSymbols;
    outReceipt->redrawsChampionSymbols = feedback->redrawsChampionSymbols;
    outReceipt->messageBeforeSkill = feedback->messageBeforeSkill;
    outReceipt->messageAfterSkill = feedback->messageAfterSkill;
    outReceipt->sourceAnchor = dm1_v1_live_action_effects_source_evidence_pc34();
    return 1;
}

int
dm1_v1_live_action_effect_hud_bind_materials_pc34(
    const DM1_V1_ActionSpellHudPresentationReceiptPc34 *presentation,
    const DM1_V1_ActionSpellHudMaterialSetPc34 *materials,
    DM1_V1_ActionSpellHudMaterialReceiptPc34 *outReceipt)
{
    int actionGraphic;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!presentation || !presentation->valid || !presentation->drawable ||
        !presentation->suppressSyntheticFallback || !materials) {
        return 0;
    }
    outReceipt->presentationKind = presentation->presentationKind;

    switch (presentation->presentationKind) {
        case DM1_V1_ACTION_HUD_PRESENTATION_DAMAGE_PC34:
            if (presentation->requiredPrimaryGraphicId !=
                    kGraphicCreatureDamage ||
                presentation->requiredPrimaryZoneId !=
                    DM1_V1_ACTION_RESULT_ZONE_ID_PC34 ||
                !dm1_v1_live_action_bind_surface_pc34(
                    materials, kGraphicCreatureDamage, 88, 45, outReceipt, 0) ||
                !dm1_v1_live_action_bind_surface_pc34(
                    materials, kGraphicHudFontPrimary, 0, 0, outReceipt, 2)) {
                return 0;
            }
            outReceipt->primaryZoneId = DM1_V1_ACTION_RESULT_ZONE_ID_PC34;
            break;

        case DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34:
            if (!presentation->requiresRealActionMenuLayout ||
                presentation->requiredPrimaryGraphicId !=
                    DM1_V1_ACTION_AREA_GRAPHIC_ID_PC34 ||
                presentation->requiredPrimaryZoneId !=
                    DM1_V1_ACTION_AREA_ZONE_ID_PC34 ||
                materials->actionMenuRowCount < 1 ||
                materials->actionMenuRowCount > 3) {
                return 0;
            }
            /* C079/C077/C011 are F0387 destination zones. The only
             * GRAPHICS.DAT bitmap in this route is C010. */
            actionGraphic = dm1_v1_action_menu_graphic_zone_id_pc34(
                materials->actionMenuRowCount);
            if (!dm1_v1_live_action_bind_surface_pc34(
                    materials, DM1_V1_ACTION_AREA_GRAPHIC_ID_PC34,
                    87, 45, outReceipt, 0) ||
                !dm1_v1_live_action_bind_surface_pc34(
                    materials, kGraphicHudFontPrimary, 0, 0, outReceipt, 2)) {
                return 0;
            }
            outReceipt->primaryZoneId = DM1_V1_ACTION_AREA_ZONE_ID_PC34;
            outReceipt->secondaryZoneId = actionGraphic;
            break;

        case DM1_V1_ACTION_HUD_PRESENTATION_SPELL_POTION_PC34:
        case DM1_V1_ACTION_HUD_PRESENTATION_SPELL_PROJECTILE_PC34:
        case DM1_V1_ACTION_HUD_PRESENTATION_SPELL_EFFECT_PC34:
        case DM1_V1_ACTION_HUD_PRESENTATION_SPELL_FAILURE_PC34:
            if (!presentation->requiresRealSpellAreaLayout ||
                presentation->requiredPrimaryGraphicId !=
                    DM1_V1_SPELL_AREA_BACKGROUND_GRAPHIC_ID_PC34 ||
                presentation->requiredSecondaryGraphicId !=
                    DM1_V1_SPELL_AREA_LINES_GRAPHIC_ID_PC34 ||
                presentation->requiredPrimaryZoneId !=
                    DM1_V1_SPELL_AREA_ZONE_ID_PC34 ||
                !dm1_v1_live_action_bind_surface_pc34(
                    materials, DM1_V1_SPELL_AREA_BACKGROUND_GRAPHIC_ID_PC34,
                    87, 25, outReceipt, 0) ||
                !dm1_v1_live_action_bind_surface_pc34(
                    materials, DM1_V1_SPELL_AREA_LINES_GRAPHIC_ID_PC34,
                    14, 39, outReceipt, 1) ||
                !dm1_v1_live_action_bind_surface_pc34(
                    materials, presentation->requiredFontGraphicId, 0, 0,
                    outReceipt, 2)) {
                return 0;
            }
            outReceipt->primaryZoneId = DM1_V1_SPELL_AREA_ZONE_ID_PC34;
            break;

        /* These messages are source TEXT2/QuePrintLines output.  They have no
         * authenticated GRAPHICS.DAT material in this live-effect boundary. */
        case DM1_V1_ACTION_HUD_PRESENTATION_MISS_PC34:
        case DM1_V1_ACTION_HUD_PRESENTATION_DOOR_PC34:
        default:
            return 0;
    }

    outReceipt->accepted = 1;
    outReceipt->drawable = 1;
    return 1;
}

int dm1_v1_live_action_effects_advance_pc34(
    DM1_V1_LiveActionEffectsPc34 *effects,
    uint32_t tick,
    DM1_V1_LiveActionEffectsAdvancePlanPc34 *outPlan)
{
    int i;
    if (outPlan) memset(outPlan, 0, sizeof(*outPlan));
    if (!effects || !outPlan || tick == effects->lastAdvancedTick) return 0;
    effects->lastAdvancedTick = tick;
    outPlan->valid = 1;
    outPlan->advanced = 1;
    for (i = 0; i < effects->count; ) {
        DM1_V1_LiveActionEffectPc34 *entry = &effects->entries[i];
        if (entry->sourceTick >= tick) { ++i; continue; }
        if (entry->remainingTicks > 0) --entry->remainingTicks;
        if (entry->remainingTicks > 0) { ++i; continue; }
        if (entry->kind == DM1_V1_LIVE_ACTION_EFFECT_ACTION_LOCK_PC34 &&
            outPlan->expiredCount < DM1_V1_LIVE_ACTION_EFFECT_CAPACITY_PC34) {
            outPlan->expiredChampionIndex[outPlan->expiredCount] = entry->championIndex;
            outPlan->expiredActionIndex[outPlan->expiredCount] = entry->actionIndex;
            ++outPlan->expiredCount;
        }
        effects->entries[i] = effects->entries[effects->count - 1];
        --effects->count;
    }
    return 1;
}
