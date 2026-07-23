#include "dm1_v1_l0351_l0400_local_owner_audit.h"

/* ReDMCSB L0351-L0400 are automatic GROUP.C storage. They remain owned by
 * the live creature/group route; standalone compatibility state is forbidden. */
static const DM1V1L0351L0400LocalOwnerAudit k_audit[] = {
    {351u, 185u, "GROUP.C:501", "dm1_v1_creature_render_pc34_compat", 1, 1},
    {352u, 185u, "GROUP.C:500", "dm1_v1_creature_render_pc34_compat", 1, 1},
    {353u, 185u, "GROUP.C:497", "dm1_v1_creature_render_pc34_compat", 1, 1},
    {354u, 185u, "GROUP.C:498", "dm1_v1_creature_render_pc34_compat", 1, 1},
    {355u, 185u, "GROUP.C:509", "dm1_v1_creature_render_pc34_compat", 1, 1},
    {356u, 186u, "GROUP.C:562", "csb_v1_runtime_pc34_compat", 1, 1},
    {357u, 186u, "GROUP.C:564", "csb_v1_runtime_pc34_compat", 1, 1},
    {358u, 186u, "GROUP.C:565", "csb_v1_runtime_pc34_compat", 1, 1},
    {359u, 186u, "GROUP.C:560", "csb_v1_runtime_pc34_compat", 1, 1},
    {360u, 186u, "GROUP.C:561", "csb_v1_runtime_pc34_compat", 1, 1},
    {361u, 186u, "GROUP.C:566", "csb_v1_runtime_pc34_compat", 1, 1},
    {362u, 186u, "GROUP.C:567", "csb_v1_runtime_pc34_compat", 1, 1},
    {363u, 187u, "GROUP.C:657", "dm1_v1_melee_action_f0402_pc34_compat", 1, 1},
    {364u, 187u, "GROUP.C:659", "dm1_v1_melee_action_f0402_pc34_compat", 1, 1},
    {365u, 188u, "GROUP.C:695", "m11_game_view group possessions", 1, 1},
    {366u, 188u, "GROUP.C:701", "m11_game_view group possessions", 1, 1},
    {367u, 188u, "GROUP.C:694", "m11_game_view group possessions", 1, 1},
    {368u, 188u, "GROUP.C:702", "m11_game_view group possessions", 1, 1},
    {369u, 188u, "GROUP.C:697", "m11_game_view group possessions", 1, 1},
    {370u, 188u, "GROUP.C:698", "m11_game_view group possessions", 1, 1},
    {371u, 188u, "GROUP.C:699", "m11_game_view group possessions", 1, 1},
    {372u, 189u, "GROUP.C:747", "m11_game_view group deletion", 1, 1},
    {373u, 189u, "GROUP.C:746", "m11_game_view group deletion", 1, 1},
    {374u, 190u, "GROUP.C:791", "m11_game_view creature damage", 1, 1},
    {375u, 190u, "GROUP.C:796", "m11_game_view creature damage", 1, 1},
    {376u, 190u, "GROUP.C:786", "m11_game_view creature damage", 1, 1},
    {377u, 190u, "GROUP.C:788", "m11_game_view creature damage", 1, 1},
    {378u, 190u, "GROUP.C:789", "m11_game_view creature damage", 1, 1},
    {379u, 190u, "GROUP.C:803", "m11_game_view creature damage", 1, 1},
    {380u, 190u, "GROUP.C:804", "m11_game_view creature damage", 1, 1},
    {381u, 190u, "GROUP.C:805", "m11_game_view creature damage", 1, 1},
    {382u, 190u, "GROUP.C:806", "m11_game_view creature damage", 1, 1},
    {383u, 190u, "GROUP.C:807", "m11_game_view creature damage", 1, 1},
    {384u, 190u, "GROUP.C:817", "m11_game_view creature damage", 1, 1},
    {385u, 191u, "GROUP.C:942", "csb_v1_runtime_pc34_compat", 1, 1},
    {386u, 191u, "GROUP.C:944", "csb_v1_runtime_pc34_compat", 1, 1},
    {387u, 191u, "GROUP.C:951", "csb_v1_runtime_pc34_compat", 1, 1},
    {388u, 191u, "GROUP.C:948", "csb_v1_runtime_pc34_compat", 1, 1},
    {389u, 191u, "GROUP.C:949", "csb_v1_runtime_pc34_compat", 1, 1},
    {390u, 192u, "GROUP.C:998", "fail_closed: poison attack local", 1, 1},
    {391u, 193u, "GROUP.C:1021", "dm1_v1_mirror_candidate_teleporter_survival_pc34_compat", 1, 1},
    {392u, 193u, "GROUP.C:1022", "dm1_v1_mirror_candidate_teleporter_survival_pc34_compat", 1, 1},
    {393u, 193u, "GROUP.C:1023", "dm1_v1_mirror_candidate_teleporter_survival_pc34_compat", 1, 1},
    {394u, 193u, "GROUP.C:1024", "dm1_v1_mirror_candidate_teleporter_survival_pc34_compat", 1, 1},
    {395u, 193u, "GROUP.C:1020", "dm1_v1_mirror_candidate_teleporter_survival_pc34_compat", 1, 1},
    {396u, 193u, "GROUP.C:1026", "dm1_v1_mirror_candidate_teleporter_survival_pc34_compat", 1, 1},
    {397u, 194u, "GROUP.C:1088", "dm1_v1_group_map_lifecycle_pc34_compat", 1, 1},
    {398u, 195u, "GROUP.C:1109", "dm1_v1_group_map_lifecycle_pc34_compat", 1, 1},
    {399u, 195u, "GROUP.C:1110", "dm1_v1_group_map_lifecycle_pc34_compat", 1, 1},
    {400u, 195u, "GROUP.C:1107", "dm1_v1_group_map_lifecycle_pc34_compat", 1, 1}
};

const DM1V1L0351L0400LocalOwnerAudit *
dm1_v1_l0351_l0400_local_owner_find(uint16_t label)
{
    if (label < 351u || label > 400u) return 0;
    return &k_audit[label - 351u];
}

const char *dm1_v1_l0351_l0400_local_owner_evidence(void)
{
    return "ReDMCSB GROUP.C:497-1110 defines L0351-L0400 as automatic "
           "function-local storage in F0185-F0195. Firestaff keeps each item "
           "in its live owner route or fails closed where no owner was "
           "evidenced. No independent ABI, fallback bitmap, or synthetic "
           "local state is permitted.";
}
