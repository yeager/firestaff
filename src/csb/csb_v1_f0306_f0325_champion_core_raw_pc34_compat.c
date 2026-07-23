#include "csb_v1_f0306_f0325_champion_core_raw_pc34_compat.h"

#include <string.h>

typedef struct ChampionCoreSpec {
    CSB_V1_ChampionCoreFunctionPc34 function_id;
    int graphics;
    int dungeon;
    int timeline;
    int query;
    const char *evidence;
} ChampionCoreSpec;

static const ChampionCoreSpec s_specs[] = {
    {306, 0, 0, 0, 1, "ReDMCSB CHAMPION.C F0306 stamina adjustment"},
    {307, 0, 0, 0, 1, "ReDMCSB CHAMPION.C F0307 statistic attack adjustment"},
    {308, 0, 0, 0, 1, "ReDMCSB CHAMPION.C F0308 luck test"},
    {309, 0, 0, 0, 1, "ReDMCSB CHAMPION.C F0309 maximum load"},
    {310, 0, 0, 0, 1, "ReDMCSB CHAMPION.C F0310 movement ticks"},
    {311, 0, 0, 0, 1, "ReDMCSB CHAMPION.C F0311 dexterity"},
    {312, 0, 1, 0, 1, "ReDMCSB CHAMPION.C F0312 strength/object weight"},
    {313, 0, 1, 0, 1, "ReDMCSB CHAMPION.C F0313 wound defense"},
    {314, 1, 0, 1, 0, "ReDMCSB CHAMPION.C F0314 wake up"},
    {315, 0, 1, 1, 1, "ReDMCSB CHAMPION.C F0315 scent ordinal"},
    {316, 0, 1, 1, 0, "ReDMCSB CHAMPION.C F0316 delete scent"},
    {317, 0, 1, 1, 0, "ReDMCSB CHAMPION.C F0317 add scent strength"},
    {318, 0, 1, 0, 0, "ReDMCSB CHAMPION.C F0318 drop all objects"},
    {319, 1, 1, 1, 0, "ReDMCSB CHAMPION.C F0319 kill champion"},
    {320, 1, 0, 1, 0, "ReDMCSB CHAMPION.C F0320 pending damage draw"},
    {321, 0, 1, 1, 0, "ReDMCSB CHAMPION.C F0321 damage and wounds"},
    {322, 0, 0, 1, 0, "ReDMCSB CHAMPION.C F0322 poison"},
    {323, 0, 0, 1, 0, "ReDMCSB CHAMPION.C F0323 unpoison"},
    {324, 0, 1, 1, 0, "ReDMCSB CHAMPION.C F0324 damage all"},
    {325, 0, 0, 1, 0, "ReDMCSB CHAMPION.C F0325 decrement stamina"}
};

static const ChampionCoreSpec *find_spec(CSB_V1_ChampionCoreFunctionPc34 function_id)
{
    size_t i;
    for (i = 0; i < sizeof(s_specs) / sizeof(s_specs[0]); ++i) {
        if (s_specs[i].function_id == function_id) return &s_specs[i];
    }
    return NULL;
}

static int has_material(const uint8_t *bytes, size_t size, uint32_t identity)
{
    return bytes != NULL && size != 0 && identity != 0;
}

int csb_v1_f0306_f0325_champion_core_audit_pc34(
    const CSB_V1_ChampionCoreRawMaterialPc34 *raw,
    CSB_V1_ChampionCoreFunctionPc34 function_id,
    CSB_V1_ChampionCoreAuditReceiptPc34 *out)
{
    const ChampionCoreSpec *spec;
    CSB_V1_ChampionCoreAuditReceiptPc34 receipt;

    if (!out) return 0;
    memset(&receipt, 0, sizeof(receipt));
    *out = receipt;
    spec = find_spec(function_id);
    if (!spec || !raw || !raw->authenticated_pc34 ||
        !has_material(raw->champion_record, raw->champion_record_size,
                      raw->champion_record_identity) ||
        (spec->graphics && !has_material(raw->graphics_material,
                                         raw->graphics_material_size,
                                         raw->graphics_material_identity)) ||
        (spec->dungeon && !has_material(raw->dungeon_material,
                                        raw->dungeon_material_size,
                                        raw->dungeon_material_identity)) ||
        (spec->timeline && !has_material(raw->timeline_material,
                                         raw->timeline_material_size,
                                         raw->timeline_material_identity))) {
        return 0;
    }
    receipt.raw_material_admitted = 1;
    receipt.graphics_material_required = spec->graphics;
    receipt.dungeon_material_required = spec->dungeon;
    receipt.timeline_material_required = spec->timeline;
    receipt.read_only_query = spec->query;
    receipt.runtime_execution_blocked = 1;
    receipt.platform_behavior_fail_closed = 1;
    receipt.function_id = function_id;
    receipt.champion_record_identity = raw->champion_record_identity;
    receipt.source_evidence = spec->evidence;
    *out = receipt;
    return 1;
}
