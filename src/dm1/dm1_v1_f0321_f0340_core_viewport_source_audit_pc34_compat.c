#include "dm1_v1_f0321_f0340_core_viewport_source_audit_pc34_compat.h"

static const DM1_V1_F0321F0340SourceAuditPc34 k_audit[] = {
    { 321u, "CHAMPION.C:1803 F0321_CHAMPION_AddPendingDamageAndWounds_GetDamage", "dm1_v1_combat_pc34_compat", 1, 1, 1 },
    { 322u, "CHAMPION.C:1926 F0322_CHAMPION_Poison", "dm1_v1_combat_pc34_compat", 1, 1, 1 },
    { 323u, "CHAMPION.C:1965 F0323_CHAMPION_Unpoison", "dm1_v1_champion_unpoison_pc34_compat", 1, 1, 1 },
    { 324u, "CHAMPION.C:1991 F0324_CHAMPION_DamageAll_GetDamagedChampionCount", "dm1_v1_combat_pc34_compat", 1, 1, 1 },
    { 325u, "CHAMPION.C:2025 F0325_CHAMPION_DecrementStamina", "dm1_v1_action_xp_graphic560_pc34_compat", 1, 1, 1 },
    { 326u, "CHAMPION.C:2051 F0326_CHAMPION_ShootProjectile", "dm1_v1_throw_shoot_pc34_compat", 1, 1, 1 },
    { 327u, "CHAMPION.C:2073 F0327_CHAMPION_IsProjectileSpellCast", "dm1_v1_throw_shoot_pc34_compat", 1, 1, 1 },
    { 328u, "CHAMPION.C:2109 F0328_CHAMPION_IsObjectThrown", "dm1_v1_throw_shoot_pc34_compat", 1, 1, 1 },
    { 329u, "CHAMPION.C:2196 F0329_CHAMPION_AddObjectInLeaderHand", "dm1_v1_leader_hand_throw_admission_f0329_pc34_compat", 1, 1, 1 },
    { 330u, "CHAMPION.C:2208 F0330_CHAMPION_DisableAction", "dm1_v1_champion_panel_disabled_icon_state_pc34_compat", 1, 1, 1 },
    { 331u, "CHAMPION.C:2254 F0331_CHAMPION_ApplyTimeEffects_CPSF", "dm1_v1_champion_needs_pc34_compat", 1, 1, 1 },
    { 332u, "PANEL.C:120 F0332_INVENTORY_DrawIconToViewport", "dm1_v1_object_draw_icon_to_screen_pc34_compat", 1, 1, 1 },
    { 333u, "PANEL.C CHEST.C F0333_INVENTORY_OpenAndDrawChest", "dm1_v1_chest_admission_f0333_f0334_pc34_compat", 1, 1, 1 },
    { 334u, "PANEL.C CHEST.C F0334_INVENTORY_CloseChest", "dm1_v1_chest_admission_f0333_f0334_pc34_compat", 1, 1, 1 },
    { 335u, "PANEL.C:172 F0335_INVENTORY_DrawPanel_ObjectDescriptionString", "dm1_v1_text_message_pc34_compat", 1, 1, 1 },
    { 336u, "PANEL.C:235 F0336_INVENTORY_DrawPanel_BuildObjectAttributesString", "inventory_item_identification_pc34_compat", 1, 1, 1 },
    { 337u, "PANEL.C:329 F0337_INVENTORY_SetDungeonViewPalette", "dm1_v1_dungeon_light_admission_f0337_pc34_compat", 1, 1, 1 },
    { 338u, "PANEL.C:434 F0338_INVENTORY_DecreaseTorchesLightPower_CPSE", "dm1_v1_torch_drain_f0338_pc34_compat", 1, 1, 1 },
    { 339u, "PANEL.C:504 F0339_INVENTORY_DrawPanel_ArrowOrEye", "dm1_v1_f0352_eye_material_pc34_compat", 1, 1, 1 },
    { 340u, "PANEL.C:754 F0340_INVENTORY_DrawPanel_ScrollTextLine", "dm1_v1_f0341_scroll_material_pc34_compat", 1, 1, 1 }
};

const DM1_V1_F0321F0340SourceAuditPc34 *
dm1_v1_f0321_f0340_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_F0321F0340SourceAuditPc34 *
dm1_v1_f0321_f0340_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_f0321_f0340_source_audit_evidence_pc34(void)
{
    return "ReDMCSB CHAMPION.C and PANEL.C are the authority for F0321-F0340. "
           "This audit records existing owners only; they require raw PC34 source "
           "material and fail closed without it. The audit does not render or "
           "synthesize UI.";
}
