#include "csb_v1_f0386_f0405_action_viewport_raw_pc34_compat.h"

#include <string.h>

typedef struct ActionViewportSpec {
    CSB_V1_ActionViewportFunctionPc34 id;
    int champion, graphics, dungeon, input, timeline, query;
    const char *evidence;
} ActionViewportSpec;

static const ActionViewportSpec s_specs[] = {
    {386,1,1,0,0,0,0,"ReDMCSB ACTIDRAW.C F0386 action icon"},
    {387,1,1,1,0,0,0,"ReDMCSB ACTIDRAW.C F0387 action area"},
    {388,1,1,0,0,0,0,"ReDMCSB MENU.C F0388 clear acting champion"},
    {389,1,1,1,1,0,0,"ReDMCSB MENU.C F0389 set acting champion"},
    {390,1,1,0,0,0,0,"ReDMCSB MENU.C F0390 refresh action area"},
    {391,1,0,1,1,1,0,"ReDMCSB MENU.C F0391 trigger action"},
    {392,1,1,0,0,0,0,"ReDMCSB MENU.C F0392 build spell line"},
    {393,1,1,0,0,0,0,"ReDMCSB SPELDRAW.C F0393 spell controls"},
    {394,1,1,0,1,0,0,"ReDMCSB CASTER.C F0394 set magic caster"},
    {395,0,1,0,0,0,0,"ReDMCSB MENUDRAW.C F0395 movement arrows"},
    {396,0,1,0,0,0,0,"ReDMCSB MENUDRAW.C F0396 spell line bitmap"},
    {397,1,1,0,0,0,0,"ReDMCSB MENUDRAW.C F0397 available symbols"},
    {398,1,1,0,0,0,0,"ReDMCSB MENUDRAW.C F0398 champion symbols"},
    {399,1,1,0,1,0,0,"ReDMCSB SYMBOL.C F0399 add champion symbol"},
    {400,1,1,0,1,0,0,"ReDMCSB SYMBOL.C F0400 delete champion symbol"},
    {401,1,0,1,0,1,0,"ReDMCSB MENU.C F0401 fright action"},
    {402,1,0,1,0,1,0,"ReDMCSB MENU.C F0402 melee action"},
    {403,1,0,1,0,1,0,"ReDMCSB MENU.C F0403 party shield action"},
    {404,1,0,1,0,1,0,"ReDMCSB MENU.C F0404 light event"},
    {405,1,0,1,0,0,0,"ReDMCSB MENU.C F0405 decrement charges"}
};

static const ActionViewportSpec *find_spec(CSB_V1_ActionViewportFunctionPc34 id)
{
    size_t i;
    for (i = 0; i < sizeof(s_specs) / sizeof(s_specs[0]); ++i)
        if (s_specs[i].id == id) return &s_specs[i];
    return NULL;
}

static int has(const uint8_t *bytes, size_t size, uint32_t identity)
{
    return bytes != NULL && size != 0 && identity != 0;
}

int csb_v1_f0386_f0405_action_viewport_audit_pc34(
    const CSB_V1_ActionViewportRawMaterialPc34 *raw,
    CSB_V1_ActionViewportFunctionPc34 function_id,
    CSB_V1_ActionViewportAuditReceiptPc34 *out)
{
    const ActionViewportSpec *spec;
    CSB_V1_ActionViewportAuditReceiptPc34 receipt;

    if (!out) return 0;
    memset(&receipt, 0, sizeof(receipt));
    *out = receipt;
    spec = find_spec(function_id);
    if (!spec || !raw || !raw->authenticated_pc34 ||
        (spec->champion && !has(raw->champion_record, raw->champion_record_size, raw->champion_record_identity)) ||
        (spec->graphics && !has(raw->graphics_material, raw->graphics_material_size, raw->graphics_material_identity)) ||
        (spec->dungeon && !has(raw->dungeon_material, raw->dungeon_material_size, raw->dungeon_material_identity)) ||
        (spec->input && !has(raw->input_material, raw->input_material_size, raw->input_material_identity)) ||
        (spec->timeline && !has(raw->timeline_material, raw->timeline_material_size, raw->timeline_material_identity))) return 0;

    receipt.raw_material_admitted = 1;
    receipt.champion_material_required = spec->champion;
    receipt.graphics_material_required = spec->graphics;
    receipt.dungeon_material_required = spec->dungeon;
    receipt.input_material_required = spec->input;
    receipt.timeline_material_required = spec->timeline;
    receipt.read_only_query = spec->query;
    receipt.runtime_execution_blocked = 1;
    receipt.platform_behavior_fail_closed = 1;
    receipt.function_id = function_id;
    receipt.source_evidence = spec->evidence;
    *out = receipt;
    return 1;
}
