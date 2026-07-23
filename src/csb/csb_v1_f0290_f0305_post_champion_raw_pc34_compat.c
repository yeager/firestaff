#include "csb_v1_f0290_f0305_post_champion_raw_pc34_compat.h"

#include <string.h>

typedef struct PostChampionSourceSpec {
    CSB_V1_PostChampionFunctionPc34 function_id;
    int needs_graphics;
    int needs_dungeon;
    int read_only_query;
    const char *evidence;
} PostChampionSourceSpec;

static const PostChampionSourceSpec s_specs[] = {
    {CSB_V1_POST_CHAMPION_F0290, 1, 0, 0, "ReDMCSB CHAMDRAW.C F0290 health/stamina/mana draw"},
    {CSB_V1_POST_CHAMPION_F0291, 1, 1, 0, "ReDMCSB CHAMDRAW.C F0291 slot draw"},
    {CSB_V1_POST_CHAMPION_F0292, 1, 0, 0, "ReDMCSB CHAMDRAW.C F0292 champion-state draw"},
    {CSB_V1_POST_CHAMPION_F0293, 1, 0, 0, "ReDMCSB CHAMDRAW.C F0293 all-state draw"},
    {CSB_V1_POST_CHAMPION_F0294, 0, 1, 1, "ReDMCSB AMMO.C F0294 ammunition compatibility"},
    {CSB_V1_POST_CHAMPION_F0296, 1, 1, 0, "ReDMCSB CHAMDRAW.C F0296 changed object icons"},
    {CSB_V1_POST_CHAMPION_F0297, 1, 1, 0, "ReDMCSB CHAMPION.C F0297 leader-hand put"},
    {CSB_V1_POST_CHAMPION_F0298, 1, 1, 0, "ReDMCSB CHAMPION.C F0298 leader-hand remove"},
    {CSB_V1_POST_CHAMPION_F0299, 0, 1, 0, "ReDMCSB CHAMPION.C F0299 object modifiers"},
    {CSB_V1_POST_CHAMPION_F0300, 1, 1, 0, "ReDMCSB CHAMPION.C F0300 slot remove"},
    {CSB_V1_POST_CHAMPION_F0301, 1, 1, 0, "ReDMCSB CHAMPION.C F0301 slot add"},
    {CSB_V1_POST_CHAMPION_F0302, 1, 1, 0, "ReDMCSB CHAMPION.C F0302 slot command"},
    {CSB_V1_POST_CHAMPION_F0303, 0, 0, 1, "ReDMCSB CHAMPION.C F0303 skill-level query"},
    {CSB_V1_POST_CHAMPION_F0304, 1, 0, 0, "ReDMCSB CHAMPION.C F0304 skill experience"},
    {CSB_V1_POST_CHAMPION_F0305, 0, 1, 1, "ReDMCSB CHAMPION.C F0305 throwing stamina cost"}
};

static const PostChampionSourceSpec *find_spec(CSB_V1_PostChampionFunctionPc34 function_id)
{
    size_t i;
    for (i = 0; i < sizeof(s_specs) / sizeof(s_specs[0]); ++i) {
        if (s_specs[i].function_id == function_id) return &s_specs[i];
    }
    return NULL;
}

static int authenticated_material(const uint8_t *bytes, size_t size, uint32_t identity)
{
    return bytes && size != 0 && identity != 0;
}

int csb_v1_f0290_f0305_post_champion_audit_pc34(
    const CSB_V1_PostChampionRawMaterialPc34 *raw,
    CSB_V1_PostChampionFunctionPc34 function_id,
    CSB_V1_PostChampionAuditReceiptPc34 *out)
{
    const PostChampionSourceSpec *spec;
    CSB_V1_PostChampionAuditReceiptPc34 receipt;

    if (!out) return 0;
    memset(&receipt, 0, sizeof(receipt));
    *out = receipt;
    spec = find_spec(function_id);
    if (!spec || !raw || !raw->authenticated_pc34 ||
        !authenticated_material(raw->champion_record, raw->champion_record_size,
                                raw->champion_record_identity) ||
        (spec->needs_graphics && !authenticated_material(
            raw->graphics_material, raw->graphics_material_size,
            raw->graphics_material_identity)) ||
        (spec->needs_dungeon && !authenticated_material(
            raw->dungeon_material, raw->dungeon_material_size,
            raw->dungeon_material_identity))) {
        return 0;
    }

    receipt.raw_material_admitted = 1;
    receipt.graphics_material_required = spec->needs_graphics;
    receipt.dungeon_material_required = spec->needs_dungeon;
    receipt.read_only_query = spec->read_only_query;
    receipt.runtime_execution_blocked = 1;
    receipt.platform_behavior_fail_closed = 1;
    receipt.function_id = function_id;
    receipt.champion_record_identity = raw->champion_record_identity;
    receipt.source_evidence = spec->evidence;
    *out = receipt;
    return 1;
}
