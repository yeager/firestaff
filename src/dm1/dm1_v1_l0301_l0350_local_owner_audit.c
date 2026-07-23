#include "dm1_v1_l0301_l0350_local_owner_audit.h"

/* ReDMCSB L0301-L0350 are automatic DUNGEON.C/GROUP.C storage. They belong
 * to their live routines; a separately persisted substitute is forbidden. */
static const DM1V1L0301L0350LocalOwnerAudit k_audit[] = {
    {301u, 168u, "DUNGEON.C:2227", "m11_game_view dungeon text decode", 1, 1},
    {302u, 168u, "DUNGEON.C:2228", "m11_game_view dungeon text decode", 1, 1},
    {303u, 168u, "DUNGEON.C:2215", "m11_game_view dungeon text decode", 1, 1},
    {304u, 168u, "DUNGEON.C:2236", "m11_game_view dungeon text decode", 1, 1},
    {305u, 170u, "DUNGEON.C:2392", "m11_game_view random wall ornament", 1, 1},
    {306u, 171u, "DUNGEON.C:2428", "m11_game_view square ornament setup", 1, 1},
    {307u, 172u, "DUNGEON.C:2487", "dm1_v1_hoc_f0115_material_order", 1, 1},
    {308u, 172u, "DUNGEON.C:2486", "dm1_v1_hoc_f0115_material_order", 1, 1},
    {309u, 172u, "DUNGEON.C:2495", "dm1_v1_hoc_f0115_material_order", 1, 1},
    {310u, 172u, "DUNGEON.C:2493", "dm1_v1_hoc_f0115_material_order", 1, 1},
    {311u, 172u, "DUNGEON.C:2501", "dm1_v1_hoc_f0115_material_order", 1, 1},
    {312u, 172u, "DUNGEON.C:2503", "dm1_v1_hoc_f0115_material_order", 1, 1},
    {313u, 172u, "DUNGEON.C:2508", "dm1_v1_hoc_f0115_material_order", 1, 1},
    {314u, 172u, "DUNGEON.C:2492", "dm1_v1_hoc_f0115_material_order", 1, 1},
    {315u, 174u, "DUNGEON.C:2752", "m11_game_view current-map setup", 1, 1},
    {316u, 174u, "DUNGEON.C:2751", "m11_game_view current-map setup", 1, 1},
    {317u, 175u, "GROUP.C:59", "csb_v1_dungeon_world_pc34_compat", 1, 1},
    {318u, 176u, "GROUP.C:77", "dm1_v1_combat_pc34_compat", 1, 1},
    {319u, 176u, "GROUP.C:79", "dm1_v1_combat_pc34_compat", 1, 1},
    {320u, 176u, "GROUP.C:83", "dm1_v1_combat_pc34_compat", 1, 1},
    {321u, 177u, "GROUP.C:120", "m11_game_view melee target selection", 1, 1},
    {322u, 177u, "GROUP.C:121", "m11_game_view melee target selection", 1, 1},
    {323u, 177u, "GROUP.C:122", "m11_game_view melee target selection", 1, 1},
    {324u, 177u, "GROUP.C:119", "m11_game_view melee target selection", 1, 1},
    {325u, 177u, "GROUP.C:123", "m11_game_view melee target selection", 1, 1},
    {326u, 179u, "GROUP.C:196", "dm1_v1_creature_render_pc34_compat", 1, 1},
    {327u, 179u, "GROUP.C:200", "dm1_v1_creature_render_pc34_compat", 1, 1},
    {328u, 179u, "GROUP.C:202", "dm1_v1_creature_render_pc34_compat", 1, 1},
    {329u, 179u, "GROUP.C:195", "dm1_v1_creature_render_pc34_compat", 1, 1},
    {330u, 179u, "GROUP.C:207", "dm1_v1_creature_render_pc34_compat", 1, 1},
    {331u, 179u, "GROUP.C:205", "dm1_v1_creature_render_pc34_compat", 1, 1},
    {332u, 180u, "GROUP.C:318", "csb_v1_runtime_pc34_compat", 1, 1},
    {333u, 180u, "GROUP.C:319", "csb_v1_runtime_pc34_compat", 1, 1},
    {334u, 181u, "GROUP.C:348", "m11_game_view group event removal", 1, 1},
    {335u, 181u, "GROUP.C:349", "m11_game_view group event removal", 1, 1},
    {336u, 181u, "GROUP.C:347", "m11_game_view group event removal", 1, 1},
    {337u, 182u, "GROUP.C:382", "fail_closed: group stop-attacking local", 1, 1},
    {338u, 183u, "GROUP.C:398", "csb_v1_runtime_pc34_compat", 1, 1},
    {339u, 183u, "GROUP.C:399", "csb_v1_runtime_pc34_compat", 1, 1},
    {340u, 183u, "GROUP.C:400", "csb_v1_runtime_pc34_compat", 1, 1},
    {341u, 183u, "GROUP.C:401", "csb_v1_runtime_pc34_compat", 1, 1},
    {342u, 183u, "GROUP.C:402", "csb_v1_runtime_pc34_compat", 1, 1},
    {343u, 183u, "GROUP.C:403", "csb_v1_runtime_pc34_compat", 1, 1},
    {344u, 183u, "GROUP.C:404", "csb_v1_runtime_pc34_compat", 1, 1},
    {345u, 184u, "GROUP.C:462", "csb_v1_runtime_pc34_compat", 1, 1},
    {346u, 184u, "GROUP.C:463", "csb_v1_runtime_pc34_compat", 1, 1},
    {347u, 184u, "GROUP.C:459", "csb_v1_runtime_pc34_compat", 1, 1},
    {348u, 184u, "GROUP.C:460", "csb_v1_runtime_pc34_compat", 1, 1},
    {349u, 185u, "GROUP.C:503", "dm1_v1_creature_render_pc34_compat", 1, 1},
    {350u, 185u, "GROUP.C:504", "dm1_v1_creature_render_pc34_compat", 1, 1}
};

const DM1V1L0301L0350LocalOwnerAudit *
dm1_v1_l0301_l0350_local_owner_find(uint16_t label)
{
    if (label < 301u || label > 350u) return 0;
    return &k_audit[label - 301u];
}

const char *dm1_v1_l0301_l0350_local_owner_evidence(void)
{
    return "ReDMCSB DUNGEON.C:2215-2752 and GROUP.C:59-504 define "
           "L0301-L0350 as automatic function-local storage in F0168-F0185. "
           "Firestaff keeps each item in its live owner route or fails closed "
           "where no owner was evidenced. No independent ABI, fallback bitmap, "
           "or synthetic local state is permitted.";
}
