#include "dm1_v1_live_action_effects_pc34_compat.h"
#include "firestaff/dm1/v1/box_action_area_pc34_compat.h"
#include "firestaff/dm1/v1/box_spell_area_pc34_compat.h"
#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(x, s) do { if (!(x)) { fprintf(stderr, "FAIL %s\n", s); ++failures; } } while (0)

int main(void)
{
    DM1_V1_LiveActionEffectsPc34 effects;
    DM1_V1_LiveActionEffectInputPc34 input;
    DM1_V1_ActionSpellHudPresentationReceiptPc34 hud;
    DM1_V1_SpellFailureHudFeedbackPc34 failureFeedback;
    DM1_V1_LiveActionEffectsAdvancePlanPc34 advance;
    dm1_v1_live_action_effects_reset_pc34(&effects);
    memset(&input, 0, sizeof(input));
    input.kind = DM1_V1_LIVE_ACTION_EFFECT_ACTION_LOCK_PC34;
    input.championIndex = 1;
    input.actionIndex = 2;
    input.disabledTicks = 2;
    input.sourceTick = 40;
    CHECK(dm1_v1_live_action_effect_materialize_pc34(&effects, &input, NULL), "materialize lock");
    input.actionIndex = 7;
    input.disabledTicks = 3;
    CHECK(dm1_v1_live_action_effect_materialize_pc34(&effects, &input, NULL), "replace lock");
    CHECK(effects.count == 1 && effects.entries[0].actionIndex == 7, "one current lock per champion");
    CHECK(dm1_v1_live_action_effects_advance_pc34(&effects, 40, &advance), "advance source tick");
    CHECK(effects.count == 1 && effects.entries[0].remainingTicks == 3, "source tick does not age new effect");
    CHECK(dm1_v1_live_action_effects_advance_pc34(&effects, 41, &advance), "advance tick 41");
    CHECK(effects.entries[0].remainingTicks == 2, "lock ages once");
    CHECK(!dm1_v1_live_action_effects_advance_pc34(&effects, 41, &advance), "same tick cannot age twice");
    CHECK(dm1_v1_live_action_effects_advance_pc34(&effects, 42, &advance), "advance tick 42");
    CHECK(dm1_v1_live_action_effects_advance_pc34(&effects, 43, &advance), "advance expiry");
    CHECK(advance.expiredCount == 1 && advance.expiredChampionIndex[0] == 1 &&
          advance.expiredActionIndex[0] == 7, "F0253 receipt on expiry");
    CHECK(effects.count == 0, "expired lock removed");

    memset(&input, 0, sizeof(input));
    input.kind = DM1_V1_LIVE_ACTION_EFFECT_DAMAGE_PC34;
    input.championIndex = 0;
    input.damage = 17;
    input.combatOutcome = 1;
    input.disabledTicks = 2;
    input.sourceTick = 88;
    CHECK(dm1_v1_live_action_effect_materialize_pc34(&effects, &input, NULL), "materialize F0231 damage");
    CHECK(dm1_v1_live_action_effect_hud_presentation_pc34(&effects.entries[0], &hud), "damage HUD receipt");
    CHECK(hud.valid && hud.drawable, "damage receipt drawable");
    CHECK(hud.presentationKind == DM1_V1_ACTION_HUD_PRESENTATION_DAMAGE_PC34, "damage presentation kind");
    CHECK(hud.layoutKind == DM1_V1_ACTION_HUD_LAYOUT_CREATURE_DAMAGE_PC34, "damage layout");
    CHECK(hud.damage == 17 && hud.text[0] == '\0', "damage has no host label");
    CHECK(hud.requiredPrimaryGraphicId == 14 &&
          hud.requiredPrimaryZoneId == DM1_V1_ACTION_RESULT_ZONE_ID_PC34 &&
          hud.requiredFontGraphicId == 695,
          "damage requires C014/result-zone/M653 material");
    CHECK(hud.requiresSourceFont && hud.suppressSyntheticFallback, "damage no synthetic fallback");

    memset(&input, 0, sizeof(input));
    input.kind = DM1_V1_LIVE_ACTION_EFFECT_SPELL_PC34;
    input.championIndex = 2;
    input.damage = 3;
    input.combatOutcome = 2;
    input.disabledTicks = 1;
    input.sourceTick = 89;
    CHECK(dm1_v1_live_action_effect_materialize_pc34(&effects, &input, NULL), "materialize F0412 projectile spell");
    CHECK(dm1_v1_live_action_effect_hud_presentation_pc34(&effects.entries[1], &hud), "spell HUD receipt");
    CHECK(hud.presentationKind == DM1_V1_ACTION_HUD_PRESENTATION_SPELL_PROJECTILE_PC34, "projectile spell presentation");
    CHECK(hud.layoutKind == DM1_V1_ACTION_HUD_LAYOUT_SPELL_AREA_PC34, "spell area layout");
    CHECK(hud.spellKind == 2 && hud.spellPowerOrdinal == 3, "spell kind and power");
    CHECK(hud.text[0] == '\0', "projectile spell has no host label");
    CHECK(hud.requiredPrimaryGraphicId == DM1_V1_SPELL_AREA_BACKGROUND_GRAPHIC_ID_PC34 &&
          hud.requiredSecondaryGraphicId == DM1_V1_SPELL_AREA_LINES_GRAPHIC_ID_PC34 &&
          hud.requiredPrimaryZoneId == DM1_V1_SPELL_AREA_ZONE_ID_PC34,
          "projectile spell requires C009/C011/C013 material");
    CHECK(hud.requiresRealSpellAreaLayout && hud.suppressSyntheticFallback, "spell route requires source layout");

    memset(&input, 0, sizeof(input));
    input.kind = DM1_V1_LIVE_ACTION_EFFECT_ACTION_LOCK_PC34;
    input.championIndex = 3;
    input.actionIndex = 20;
    input.disabledTicks = 4;
    input.sourceTick = 90;
    CHECK(dm1_v1_live_action_effect_materialize_pc34(&effects, &input, NULL), "materialize F0407 action lock");
    CHECK(dm1_v1_live_action_effect_hud_presentation_pc34(&effects.entries[2], &hud), "action lock HUD receipt");
    CHECK(hud.presentationKind == DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34, "action lock presentation");
    CHECK(hud.layoutKind == DM1_V1_ACTION_HUD_LAYOUT_ACTION_MENU_ROW_PC34, "action menu layout");
    CHECK(hud.actionIndex == 20 && hud.remainingTicks == 4, "action lock state");
    CHECK(hud.requiresRealActionMenuLayout, "action route requires source layout");
    CHECK(hud.text[0] == '\0' &&
          hud.requiredPrimaryGraphicId == DM1_V1_ACTION_AREA_GRAPHIC_ID_PC34 &&
          hud.requiredPrimaryZoneId == DM1_V1_ACTION_AREA_ZONE_ID_PC34,
          "action lock requires C010/C011 material");

    memset(&failureFeedback, 0, sizeof(failureFeedback));
    failureFeedback.failureType = 0;
    failureFeedback.messageColor = 4;
    failureFeedback.printsLineFeed = 1;
    failureFeedback.printsChampionName = 1;
    failureFeedback.appendsBaseSkillName = 1;
    failureFeedback.clearsSymbolsOnCastClick = 1;
    failureFeedback.redrawsAvailableSymbols = 1;
    failureFeedback.redrawsChampionSymbols = 1;
    failureFeedback.messageBeforeSkill = " NEEDS MORE PRACTICE WITH THIS ";
    failureFeedback.messageAfterSkill = " SPELL.";
    CHECK(dm1_v1_live_action_spell_failure_hud_presentation_f0412_pc34(&failureFeedback, 1, &hud),
          "F0412 failure HUD receipt");
    CHECK(hud.presentationKind == DM1_V1_ACTION_HUD_PRESENTATION_SPELL_FAILURE_PC34, "failure presentation");
    CHECK(hud.layoutKind == DM1_V1_ACTION_HUD_LAYOUT_SPELL_AREA_PC34, "failure spell layout");
    CHECK(hud.textColor == 4 && hud.printsChampionName && hud.appendsBaseSkillName, "failure text flags");
    CHECK(hud.clearsSymbolsOnCastClick && hud.redrawsAvailableSymbols && hud.redrawsChampionSymbols,
          "failure redraw flags");
    CHECK(hud.text[0] == '\0' &&
          hud.requiredPrimaryGraphicId == DM1_V1_SPELL_AREA_BACKGROUND_GRAPHIC_ID_PC34 &&
          hud.requiredSecondaryGraphicId == DM1_V1_SPELL_AREA_LINES_GRAPHIC_ID_PC34 &&
          hud.requiredFontGraphicId == 557,
          "failure requires C009/C011/M653 material");
    CHECK(strcmp(hud.messageBeforeSkill, " NEEDS MORE PRACTICE WITH THIS ") == 0 &&
          strcmp(hud.messageAfterSkill, " SPELL.") == 0, "failure source text");
    CHECK(!dm1_v1_live_action_spell_failure_hud_presentation_f0412_pc34(&failureFeedback, 4, &hud),
          "reject invalid champion failure receipt");

    effects.entries[1].combatOutcome = 99;
    CHECK(!dm1_v1_live_action_effect_hud_presentation_pc34(&effects.entries[1], &hud),
          "reject non-source spell kind");
    printf("%s\n", failures ? "failed" : "ok: DM1 live action effects");
    return failures ? 1 : 0;
}
