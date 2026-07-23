#include "dm1_v1_l0251_l0300_local_owner_audit.h"

/* ReDMCSB L0251-L0300 are automatic DUNGEON.C storage.  They must remain in
 * their live owner route, never become a separately persisted port state. */
static const DM1V1L0251L0300LocalOwnerAudit k_audit[] = {
    {251u, 154u, "DUNGEON.C:1522", "csb_v1_runtime_pc34_compat level handoff", 1, 1},
    {252u, 154u, "DUNGEON.C:1524", "csb_v1_runtime_pc34_compat level handoff", 1, 1},
    {253u, 154u, "DUNGEON.C:1519", "csb_v1_runtime_pc34_compat level handoff", 1, 1},
    {254u, 154u, "DUNGEON.C:1517", "csb_v1_runtime_pc34_compat level handoff", 1, 1},
    {255u, 154u, "DUNGEON.C:1526", "csb_v1_runtime_pc34_compat level handoff", 1, 1},
    {256u, 155u, "DUNGEON.C:1568", "csb_v1_runtime_pc34_compat stairs direction", 1, 1},
    {257u, 155u, "DUNGEON.C:1570", "csb_v1_runtime_pc34_compat stairs direction", 1, 1},
    {258u, 158u, "DUNGEON.C:1657", "fail_closed: weapon-info local", 1, 1},
    {259u, 159u, "DUNGEON.C:1671", "dm1_v1_chest_close_stack_merge_pc34_compat", 1, 1},
    {260u, 160u, "DUNGEON.C:1709", "m11_game_view square-first-thing lookup", 1, 1},
    {261u, 160u, "DUNGEON.C:1710", "m11_game_view square-first-thing lookup", 1, 1},
    {262u, 160u, "DUNGEON.C:1707", "m11_game_view square-first-thing lookup", 1, 1},
    {263u, 161u, "DUNGEON.C:1737", "m11_game_view square-first-thing traversal", 1, 1},
    {264u, 162u, "DUNGEON.C:1759", "dm1_v1_viewport_3d_pc34_compat square-first-object", 1, 1},
    {265u, 163u, "DUNGEON.C:1783", "m11_game_view thing-list linking", 1, 1},
    {266u, 163u, "DUNGEON.C:1784", "m11_game_view thing-list linking", 1, 1},
    {267u, 163u, "DUNGEON.C:1778", "m11_game_view thing-list linking", 1, 1},
    {268u, 163u, "DUNGEON.C:1779", "m11_game_view thing-list linking", 1, 1},
    {269u, 163u, "DUNGEON.C:1781", "m11_game_view thing-list linking", 1, 1},
    {270u, 163u, "DUNGEON.C:1789", "m11_game_view thing-list linking", 1, 1},
    {271u, 164u, "DUNGEON.C:1862", "dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_pc34_compat", 1, 1},
    {272u, 164u, "DUNGEON.C:1868", "dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_pc34_compat", 1, 1},
    {273u, 164u, "DUNGEON.C:1866", "dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_pc34_compat", 1, 1},
    {274u, 164u, "DUNGEON.C:1856", "dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_pc34_compat", 1, 1},
    {275u, 164u, "DUNGEON.C:1858", "dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_pc34_compat", 1, 1},
    {276u, 165u, "DUNGEON.C:1944", "fail_closed: discarded-thing local", 1, 1},
    {277u, 165u, "DUNGEON.C:1945", "fail_closed: discarded-thing local", 1, 1},
    {278u, 165u, "DUNGEON.C:1941", "fail_closed: discarded-thing local", 1, 1},
    {279u, 165u, "DUNGEON.C:1940", "fail_closed: discarded-thing local", 1, 1},
    {280u, 165u, "DUNGEON.C:1934", "fail_closed: discarded-thing local", 1, 1},
    {281u, 165u, "DUNGEON.C:1938", "fail_closed: discarded-thing local", 1, 1},
    {282u, 165u, "DUNGEON.C:1936", "fail_closed: discarded-thing local", 1, 1},
    {283u, 165u, "DUNGEON.C:1955", "fail_closed: discarded-thing local", 1, 1},
    {284u, 165u, "DUNGEON.C:1956", "fail_closed: discarded-thing local", 1, 1},
    {285u, 165u, "DUNGEON.C:1958", "fail_closed: discarded-thing local", 1, 1},
    {286u, 165u, "DUNGEON.C:1959", "fail_closed: discarded-thing local", 1, 1},
    {287u, 165u, "DUNGEON.C:1965", "fail_closed: discarded-thing local", 1, 1},
    {288u, 166u, "DUNGEON.C:2084", "csb_v1_runtime_pc34_compat unused-thing allocation", 1, 1},
    {289u, 166u, "DUNGEON.C:2086", "csb_v1_runtime_pc34_compat unused-thing allocation", 1, 1},
    {290u, 166u, "DUNGEON.C:2088", "csb_v1_runtime_pc34_compat unused-thing allocation", 1, 1},
    {291u, 166u, "DUNGEON.C:2083", "csb_v1_runtime_pc34_compat unused-thing allocation", 1, 1},
    {292u, 166u, "DUNGEON.C:2090", "csb_v1_runtime_pc34_compat unused-thing allocation", 1, 1},
    {293u, 167u, "DUNGEON.C:2149", "csb_v1_runtime_pc34_compat launcher/generator object", 1, 1},
    {294u, 167u, "DUNGEON.C:2151", "csb_v1_runtime_pc34_compat launcher/generator object", 1, 1},
    {295u, 167u, "DUNGEON.C:2153", "csb_v1_runtime_pc34_compat launcher/generator object", 1, 1},
    {296u, 167u, "DUNGEON.C:2147", "csb_v1_runtime_pc34_compat launcher/generator object", 1, 1},
    {297u, 168u, "DUNGEON.C:2219", "m11_game_view dungeon text decode", 1, 1},
    {298u, 168u, "DUNGEON.C:2224", "m11_game_view dungeon text decode", 1, 1},
    {299u, 168u, "DUNGEON.C:2222", "m11_game_view dungeon text decode", 1, 1},
    {300u, 168u, "DUNGEON.C:2217", "m11_game_view dungeon text decode", 1, 1}
};

const DM1V1L0251L0300LocalOwnerAudit *
dm1_v1_l0251_l0300_local_owner_find(uint16_t label)
{
    if (label < 251u || label > 300u) return 0;
    return &k_audit[label - 251u];
}

const char *dm1_v1_l0251_l0300_local_owner_evidence(void)
{
    return "ReDMCSB DUNGEON.C:1517-2224 defines L0251-L0300 as function-local "
           "storage within F0154-F0168. Firestaff keeps them in their live "
           "dungeon or viewport owner route, or fails closed where no owner "
           "was evidenced. No independent ABI, fallback bitmap, or synthetic "
           "local state is permitted.";
}
