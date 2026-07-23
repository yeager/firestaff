#include "dm1_v1_l0201_l0250_local_owner_audit.h"

/* These ReDMCSB labels are automatic storage. Keep their lifetime inside the
 * corresponding viewport, blit, or dungeon routine; no standalone port state. */
static const DM1V1L0201L0250LocalOwnerAudit k_audit[] = {
    {201u, 116u, "DUNVIEW.C:6369", "m11_game_view D3L square order", 1, 1},
    {202u, 117u, "DUNVIEW.C:6509", "m11_game_view D3R square order", 1, 1},
    {203u, 117u, "DUNVIEW.C:6508", "m11_game_view D3R square aspect", 1, 1},
    {204u, 118u, "DUNVIEW.C:6661", "dm1_v1_viewport_d3c_f0111_door_front_pair_pc34_compat", 1, 1},
    {205u, 118u, "DUNVIEW.C:6652", "dm1_v1_viewport_d3c_f0111_door_front_pair_pc34_compat", 1, 1},
    {206u, 118u, "DUNVIEW.C:6660", "dm1_v1_viewport_d3c_f0111_door_front_pair_pc34_compat", 1, 1},
    {207u, 119u, "DUNVIEW.C:6909", "m11_game_view D2L square order", 1, 1},
    {208u, 119u, "DUNVIEW.C:6908", "m11_game_view D2L square aspect", 1, 1},
    {209u, 120u, "DUNVIEW.C:7060", "dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_pc34_compat", 1, 1},
    {210u, 120u, "DUNVIEW.C:7059", "dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_pc34_compat", 1, 1},
    {211u, 121u, "DUNVIEW.C:7253", "m11_game_view D2C square order", 1, 1},
    {212u, 121u, "DUNVIEW.C:7252", "m11_game_view D2C square aspect", 1, 1},
    {213u, 122u, "DUNVIEW.C:7400", "m11_game_view D1L square order", 1, 1},
    {214u, 122u, "DUNVIEW.C:7399", "m11_game_view D1L square aspect", 1, 1},
    {215u, 123u, "DUNVIEW.C:7568", "dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_pc34_compat", 1, 1},
    {216u, 123u, "DUNVIEW.C:7567", "dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_pc34_compat", 1, 1},
    {217u, 124u, "DUNVIEW.C:7736", "m11_game_view D1C square order", 1, 1},
    {218u, 124u, "DUNVIEW.C:7738", "m11_game_view D1C square aspect", 1, 1},
    {219u, 124u, "DUNVIEW.C:7740", "m11_game_view D1C bitmap", 1, 1},
    {220u, 125u, "DUNVIEW.C:7973", "dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_pc34_compat", 1, 1},
    {221u, 126u, "DUNVIEW.C:8077", "dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_pc34_compat", 1, 1},
    {222u, 127u, "DUNVIEW.C:8177", "dm1_v1_viewport_d0c_stairs_pit_dispatch_pc34_compat", 1, 1},
    {223u, 128u, "DUNVIEW.C:8327", "m11_game_view viewport orientation", 1, 1},
    {224u, 128u, "DUNVIEW.C:8329", "m11_game_view viewport map x", 1, 1},
    {225u, 128u, "DUNVIEW.C:8330", "m11_game_view viewport map y", 1, 1},
    {226u, 129u, "BLTSHRNK.C:239", "palette_changes_mouse_pointer_icon_shadow_pc34_compat", 1, 1},
    {227u, 129u, "BLTSHRNK.C:240", "palette_changes_mouse_pointer_icon_shadow_pc34_compat", 1, 1},
    {228u, 129u, "BLTSHRNK.C:241", "palette_changes_mouse_pointer_icon_shadow_pc34_compat", 1, 1},
    {229u, 132u, "BLIT.C:114", "fail_closed: platform blit local", 1, 1},
    {230u, 132u, "BLIT.C:115", "fail_closed: platform blit local", 1, 1},
    {231u, 132u, "BLIT.C:116", "fail_closed: platform blit local", 1, 1},
    {232u, 133u, "BLITMASK.C:74", "fail_closed: platform masked-blit local", 1, 1},
    {233u, 137u, "COPYPRO3.C:34", "fail_closed: copy-protection local", 1, 1},
    {234u, 139u, "DUNGEON.C:1061", "fail_closed: creature-map local", 1, 1},
    {235u, 139u, "DUNGEON.C:1062", "fail_closed: creature-map local", 1, 1},
    {236u, 139u, "DUNGEON.C:1057", "fail_closed: creature-map local", 1, 1},
    {237u, 139u, "DUNGEON.C:1060", "fail_closed: creature-map local", 1, 1},
    {238u, 140u, "DUNGEON.C:1091", "m11_game_view object weight", 1, 1},
    {239u, 140u, "DUNGEON.C:1089", "m11_game_view object weight", 1, 1},
    {240u, 141u, "CEDT004.C:214", "csb_v1_runtime_pc34_compat object info", 1, 1},
    {241u, 142u, "DUNGEON.C:1174", "m11_game_view projectile aspect", 1, 1},
    {242u, 142u, "DUNGEON.C:1175", "m11_game_view projectile aspect", 1, 1},
    {243u, 142u, "DUNGEON.C:1172", "m11_game_view projectile aspect", 1, 1},
    {244u, 143u, "DUNGEON.C:1237", "m11_game_view armour defense", 1, 1},
    {245u, 144u, "DUNGEON.C:1257", "dm1_v1_melee_action_f0402_pc34_compat", 1, 1},
    {246u, 145u, "DUNGEON.C:1271", "fail_closed: group-cell local", 1, 1},
    {247u, 149u, "DUNGEON.C:1336", "dm1_v1_viewport_d1c_f0107_wall_ornament_pc34_compat", 1, 1},
    {248u, 151u, "DUNGEON.C:1430", "m11_game_view square lookup", 1, 1},
    {249u, 151u, "DUNGEON.C:1433", "m11_game_view square lookup", 1, 1},
    {250u, 154u, "DUNGEON.C:1521", "csb_v1_runtime_pc34_compat level handoff", 1, 1}
};

const DM1V1L0201L0250LocalOwnerAudit *
dm1_v1_l0201_l0250_local_owner_find(uint16_t label)
{
    if (label < 201u || label > 250u) return 0;
    return &k_audit[label - 201u];
}

const char *dm1_v1_l0201_l0250_local_owner_evidence(void)
{
    return "ReDMCSB DUNVIEW.C:6369-8330, BLTSHRNK.C:239-241, BLIT.C:114-116, "
           "and DUNGEON.C:1057-1521 define L0201-L0250 as function-local "
           "storage. Firestaff keeps them in their live owner route or fails "
           "closed at a platform boundary; No independent ABI, fallback bitmap, "
           "or synthetic local state is permitted.";
}
