#include "dm1_v1_f0381_f0400_core_viewport_source_audit_pc34_compat.h"

static const DM1_V1_F0381F0400SourceAuditPc34 k_audit[] = {
    { 381u, "MENU.C:555 F0381_MENUS_PrintMessageAfterReplacements", "dm1_v1_text_message_pc34_compat", 1, 1, 1 },
    { 382u, "MENU.C:607 F0382_MENUS_GetActionObjectChargeCount", "dm1_v1_action_list_pc34_compat", 1, 1, 1 },
    { 383u, "MENU.C:635 F0383_MENUS_SetActionList", "dm1_v1_action_list_pc34_compat", 1, 1, 1 },
    { 384u, "ACTIDRAW.C:10 F0384_MENUS_GetActionName", "dm1_v1_g0490_pc34_compat", 1, 1, 1 },
    { 385u, "ACTIDRAW.C:34 F0385_MENUS_DrawActionDamage", "dm1_v1_action_damage_render_plan_pc34_compat", 1, 1, 1 },
    { 386u, "ACTIDRAW.C:201 F0386_MENUS_DrawActionIcon", "dm1_v1_champion_panel_disabled_icon_state_pc34_compat", 1, 1, 1 },
    { 387u, "ACTIDRAW.C:302 F0387_MENUS_DrawActionArea", "dm1_v1_action_spell_render_command_admission_pc34_compat", 1, 1, 1 },
    { 388u, "MENU.C:682 F0388_MENUS_ClearActingChampion", "dm1_v1_action_spell_presentation_sequence_pc34_compat", 1, 1, 1 },
    { 389u, "MENU.C:696 F0389_MENUS_ProcessCommands116To119_SetActingChampion", "dm1_v1_action_spell_input_command_admission_pc34_compat", 1, 1, 1 },
    { 390u, "MENU.C:734 F0390_MENUS_RefreshActionAreaAndSetChampionDirectionMaximumDamageReceived", "dm1_v1_live_action_effects_pc34_compat", 1, 1, 1 },
    { 391u, "MENU.C:803 F0391_MENUS_DidClickTriggerAction", "dm1_v1_action_spell_input_execution_bridge_pc34_compat", 1, 1, 1 },
    { 392u, "MENU.C:844 F0392_MENUS_BuildSpellAreaLine", "dm1_v1_champion_panel_spell_area_overlay_pc34_compat", 1, 1, 1 },
    { 393u, "SPELDRAW.C:2 F0393_MENUS_DrawSpellAreaControls", "dm1_v1_spell_effect_render_pc34_compat", 1, 1, 1 },
    { 394u, "CASTER.C:2 F0394_MENUS_SetMagicCasterAndDrawSpellArea", "dm1_v1_champion_panel_spell_area_overlay_pc34_compat", 1, 1, 1 },
    { 395u, "MENUDRAW.C:5 F0395_MENUS_DrawMovementArrows", "dm1_v1_menu_render_pc34_compat", 1, 1, 1 },
    { 396u, "MENUDRAW.C:31 F0396_MENUS_LoadSpellAreaLinesBitmap", "dm1_v1_champion_panel_spell_area_overlay_pc34_compat", 1, 1, 1 },
    { 397u, "MENUDRAW.C:47 F0397_MENUS_DrawAvailableSymbols", "dm1_v1_champion_panel_spell_area_overlay_pc34_compat", 1, 1, 1 },
    { 398u, "MENUDRAW.C:83 F0398_MENUS_DrawChampionSymbols", "dm1_v1_champion_panel_spell_area_overlay_pc34_compat", 1, 1, 1 },
    { 399u, "SYMBOL.C:2 F0399_MENUS_AddChampionSymbol", "dm1_v1_f0399_f0400_spell_symbol_consume_pc34_compat", 1, 1, 1 },
    { 400u, "SYMBOL.C:69 F0400_MENUS_DeleteChampionSymbol", "dm1_v1_f0399_f0400_spell_symbol_consume_pc34_compat", 1, 1, 1 }
};

const DM1_V1_F0381F0400SourceAuditPc34 *
dm1_v1_f0381_f0400_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_F0381F0400SourceAuditPc34 *
dm1_v1_f0381_f0400_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_f0381_f0400_source_audit_evidence_pc34(void)
{
    return "ReDMCSB MENU.C, ACTIDRAW.C, CASTER.C, SPELDRAW.C, MENUDRAW.C, "
           "and SYMBOL.C are the authority for F0381-F0400. This audit records "
           "existing owners only; they require raw PC34 source material and fail "
           "closed without it. The audit does not render or synthesize UI.";
}
