#include "dm1_v1_l0151_l0200_f0115_local_owner_audit.h"

#define ROW(number, routine, anchor, owner, kind) \
    { number##u, routine##u, anchor, owner, kind, 1, 1 }

/* ReDMCSB L labels are automatic storage.  This table deliberately records
 * their enclosing live route instead of promoting them to Firestaff globals. */
static const DM1V1L0151L0200LocalOwnerAudit k_audit[] = {
    ROW(151, 115, "DUNVIEW.C:4667", "dm1_v1_f0115_f0219_creature_item_material_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_CREATURE_MATERIAL),
    ROW(152, 115, "DUNVIEW.C:4684", "dm1_v1_f0115_f0219_creature_item_material_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_CREATURE_MATERIAL),
    ROW(153, 115, "DUNVIEW.C:4685", "dm1_v1_f0115_f0219_creature_item_material_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_CREATURE_MATERIAL),
    ROW(154, 115, "DUNVIEW.C:4686", "dm1_v1_f0115_f0219_creature_item_material_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_CREATURE_MATERIAL),
    ROW(155, 115, "DUNVIEW.C:4687", "dm1_v1_f0115_f0219_creature_item_material_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_CREATURE_MATERIAL),
    ROW(156, 115, "DUNVIEW.C:4691", "dm1_v1_f0115_f0219_creature_item_material_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_CREATURE_MATERIAL),
    ROW(157, 115, "DUNVIEW.C:4692", "dm1_v1_f0115_f0219_creature_item_material_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_CREATURE_MATERIAL),
    ROW(158, 115, "DUNVIEW.C:4694", "dm1_v1_f0115_f0219_creature_item_material_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_CREATURE_MATERIAL),
    ROW(159, 115, "DUNVIEW.C:4696", "dm1_v1_f0115_f0219_creature_item_material_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_CREATURE_MATERIAL),
    ROW(160, 115, "DUNVIEW.C:4698", "fail_closed: unreferenced F0115 local", DM1_V1_L0151L0200_OWNER_F0115_CREATURE_MATERIAL),
    ROW(161, 115, "DUNVIEW.C:4700", "dm1_v1_f0115_f0219_creature_item_material_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_CREATURE_MATERIAL),
    ROW(162, 115, "DUNVIEW.C:4701", "dm1_v1_f0115_f0219_creature_item_material_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_CREATURE_MATERIAL),
    ROW(163, 115, "DUNVIEW.C:4703", "dm1_v1_f0115_f0219_creature_item_material_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_CREATURE_MATERIAL),
    ROW(164, 115, "DUNVIEW.C:4704", "dm1_v1_f0115_f0219_creature_item_material_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_CREATURE_MATERIAL),
    ROW(165, 115, "DUNVIEW.C:4705", "dm1_v1_f0115_f0219_creature_item_material_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_CREATURE_MATERIAL),
    ROW(166, 115, "DUNVIEW.C:4706", "fail_closed: unreferenced F0115 local", DM1_V1_L0151L0200_OWNER_F0115_CREATURE_MATERIAL),
    ROW(167, 115, "DUNVIEW.C:4714", "dm1_v1_f0115_f0219_creature_item_material_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_CREATURE_MATERIAL),
    ROW(168, 115, "DUNVIEW.C:4718", "dm1_v1_f0115_square_material_scheduler_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_CREATURE_MATERIAL),
    ROW(169, 115, "DUNVIEW.C:4719", "dm1_v1_f0115_f0219_creature_item_material_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_CREATURE_MATERIAL),
    ROW(170, 115, "DUNVIEW.C:4720", "dm1_v1_f0115_f0219_creature_item_material_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_CREATURE_MATERIAL),
    ROW(171, 115, "DUNVIEW.C:4721", "dm1_v1_f0115_f0219_creature_item_material_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_CREATURE_MATERIAL),
    ROW(172, 115, "DUNVIEW.C:4722", "dm1_v1_f0115_f0219_creature_item_material_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_CREATURE_MATERIAL),
    ROW(173, 115, "DUNVIEW.C:4723", "dm1_v1_f0115_f0219_creature_item_material_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_CREATURE_MATERIAL),
    ROW(174, 115, "DUNVIEW.C:4716", "dm1_v1_f0115_f0219_creature_item_material_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_CREATURE_MATERIAL),
    ROW(175, 115, "DUNVIEW.C:4661", "dm1_v1_f0115_square_material_scheduler_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_PROJECTILE_EFFECT),
    ROW(176, 115, "DUNVIEW.C:4645", "dm1_v1_f0115_square_material_scheduler_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_PROJECTILE_EFFECT),
    ROW(177, 115, "DUNVIEW.C:4728", "dm1_v1_projectile_explosion_render_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_PROJECTILE_EFFECT),
    ROW(178, 115, "DUNVIEW.C:4733", "dm1_v1_projectile_explosion_render_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_PROJECTILE_EFFECT),
    ROW(179, 115, "DUNVIEW.C:4735", "dm1_v1_projectile_explosion_render_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_PROJECTILE_EFFECT),
    ROW(180, 115, "DUNVIEW.C:4736", "dm1_v1_projectile_explosion_render_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_PROJECTILE_EFFECT),
    ROW(181, 115, "DUNVIEW.C:4738", "dm1_v1_projectile_explosion_render_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_PROJECTILE_EFFECT),
    ROW(182, 115, "DUNVIEW.C:4739", "dm1_v1_projectile_explosion_render_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_PROJECTILE_EFFECT),
    ROW(183, 115, "DUNVIEW.C:4740", "dm1_v1_projectile_explosion_render_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_PROJECTILE_EFFECT),
    ROW(184, 115, "DUNVIEW.C:4741", "dm1_v1_projectile_explosion_render_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_PROJECTILE_EFFECT),
    ROW(185, 115, "DUNVIEW.C:4748", "dm1_v1_projectile_explosion_render_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_PROJECTILE_EFFECT),
    ROW(186, 115, "DUNVIEW.C:4749", "dm1_v1_projectile_explosion_render_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_PROJECTILE_EFFECT),
    ROW(187, 115, "DUNVIEW.C:4743", "dm1_v1_projectile_explosion_render_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_PROJECTILE_EFFECT),
    ROW(188, 115, "DUNVIEW.C:4753", "dm1_v1_projectile_explosion_render_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_PROJECTILE_EFFECT),
    ROW(189, 115, "DUNVIEW.C:4754", "dm1_v1_projectile_explosion_render_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_PROJECTILE_EFFECT),
    ROW(190, 115, "DUNVIEW.C:4756", "dm1_v1_projectile_explosion_render_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_PROJECTILE_EFFECT),
    ROW(191, 115, "DUNVIEW.C:4758", "dm1_v1_projectile_explosion_render_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_PROJECTILE_EFFECT),
    ROW(192, 115, "DUNVIEW.C:4760", "dm1_v1_projectile_explosion_render_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_PROJECTILE_EFFECT),
    ROW(193, 115, "DUNVIEW.C:4763", "dm1_v1_projectile_explosion_render_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_PROJECTILE_EFFECT),
    ROW(194, 115, "DUNVIEW.C:4765", "dm1_v1_projectile_explosion_render_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_PROJECTILE_EFFECT),
    ROW(195, 115, "DUNVIEW.C:4769", "dm1_v1_projectile_explosion_render_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_PROJECTILE_EFFECT),
    ROW(196, 115, "DUNVIEW.C:4770", "dm1_v1_projectile_explosion_render_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_PROJECTILE_EFFECT),
    ROW(197, 115, "DUNVIEW.C:4771", "dm1_v1_projectile_explosion_render_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_PROJECTILE_EFFECT),
    ROW(198, 115, "DUNVIEW.C:4779", "dm1_v1_projectile_explosion_render_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_PROJECTILE_EFFECT),
    ROW(199, 115, "DUNVIEW.C:4782", "dm1_v1_projectile_explosion_render_pc34_compat", DM1_V1_L0151L0200_OWNER_F0115_PROJECTILE_EFFECT),
    ROW(200, 116, "DUNVIEW.C:6370", "m11_game_view.c:21467", DM1_V1_L0151L0200_OWNER_F0116_SQUARE_ORDER)
};

#undef ROW

const DM1V1L0151L0200LocalOwnerAudit *
dm1_v1_l0151_l0200_local_owner_audit(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1V1L0151L0200LocalOwnerAudit *
dm1_v1_l0151_l0200_local_owner_find(uint16_t label, uint16_t enclosing_routine)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].label == label &&
            k_audit[index].enclosing_routine == enclosing_routine) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_l0151_l0200_local_owner_evidence(void)
{
    return "ReDMCSB DUNVIEW.C:4645-4782 defines L0151-L0199 inside F0115; "
           "DUNVIEW.C:6370 defines L0200 inside F0116. Firestaff keeps every "
           "label local to the corresponding material, projectile, effect, or "
           "square-order route. No independent ABI, fallback bitmap, or synthetic "
           "local state is permitted.";
}
