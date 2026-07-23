#include "csb_v1_f0346_f0365_panel_input_raw_pc34_compat.h"

#include <string.h>

typedef struct PanelInputSpec {
    CSB_V1_PanelInputFunctionPc34 id;
    int champion, graphics, dungeon, input, platform, query;
    const char *evidence;
} PanelInputSpec;

static const PanelInputSpec s_specs[] = {
    {346,1,1,1,0,0,0,"ReDMCSB PANEL.C F0346 resurrect/reincarnate panel"},
    {347,1,1,1,0,0,0,"ReDMCSB PANEL.C F0347 panel router"},
    {348,1,0,0,0,0,0,"ReDMCSB PANEL.C F0348 statistic adjustment"},
    {349,1,1,1,1,0,0,"ReDMCSB PANEL.C F0349 mouth command"},
    {350,1,1,0,1,0,0,"ReDMCSB PANEL.C F0350 mouth release"},
    {351,1,1,0,0,0,0,"ReDMCSB PANEL.C F0351 skill/stat draw"},
    {352,1,1,1,1,0,0,"ReDMCSB PANEL.C F0352 eye command"},
    {353,1,1,0,1,0,0,"ReDMCSB PANEL.C F0353 eye release"},
    {354,1,1,0,0,0,0,"ReDMCSB PANEL.C F0354 status portrait"},
    {355,1,1,1,1,0,0,"ReDMCSB PANEL.C F0355 inventory toggle"},
    {356,0,0,0,0,1,0,"ReDMCSB COPYPRO7.C F0356 fuzzy-sector validation"},
    {357,0,0,0,1,0,0,"ReDMCSB COMMAND.C F0357 discard input"},
    {358,0,0,0,1,0,1,"ReDMCSB COMMAND.C F0358 mouse command lookup"},
    {359,1,1,1,1,0,0,"ReDMCSB COMMAND.C F0359 click dispatch"},
    {360,0,0,0,1,0,0,"ReDMCSB COMMAND.C F0360 pending click"},
    {361,1,1,1,1,0,0,"ReDMCSB COMMAND.C F0361 key dispatch"},
    {362,0,1,0,1,0,0,"ReDMCSB CLIKMENU.C F0362 highlight enable"},
    {363,0,1,0,1,0,0,"ReDMCSB CLIKMENU.C F0363 highlight disable"},
    {364,1,1,1,1,0,0,"ReDMCSB CLIKMENU.C F0364 stairs"},
    {365,1,1,1,1,0,0,"ReDMCSB CLIKMENU.C F0365 party turn"}
};

static const PanelInputSpec *find_spec(CSB_V1_PanelInputFunctionPc34 id)
{
    size_t i;
    for (i = 0; i < sizeof(s_specs) / sizeof(s_specs[0]); ++i)
        if (s_specs[i].id == id) return &s_specs[i];
    return NULL;
}

static int has(const uint8_t *p, size_t size, uint32_t identity)
{
    return p != NULL && size != 0 && identity != 0;
}

int csb_v1_f0346_f0365_panel_input_audit_pc34(
    const CSB_V1_PanelInputRawMaterialPc34 *raw,
    CSB_V1_PanelInputFunctionPc34 function_id,
    CSB_V1_PanelInputAuditReceiptPc34 *out)
{
    const PanelInputSpec *spec;
    CSB_V1_PanelInputAuditReceiptPc34 receipt;
    if (!out) return 0;
    memset(&receipt, 0, sizeof(receipt)); *out = receipt;
    spec = find_spec(function_id);
    if (!spec || !raw || !raw->authenticated_pc34 ||
        (spec->champion && !has(raw->champion_record, raw->champion_record_size, raw->champion_record_identity)) ||
        (spec->graphics && !has(raw->graphics_material, raw->graphics_material_size, raw->graphics_material_identity)) ||
        (spec->dungeon && !has(raw->dungeon_material, raw->dungeon_material_size, raw->dungeon_material_identity)) ||
        (spec->input && !has(raw->input_material, raw->input_material_size, raw->input_material_identity)) ||
        (spec->platform && !has(raw->platform_material, raw->platform_material_size, raw->platform_material_identity))) return 0;
    receipt.raw_material_admitted = 1;
    receipt.champion_material_required = spec->champion;
    receipt.graphics_material_required = spec->graphics;
    receipt.dungeon_material_required = spec->dungeon;
    receipt.input_material_required = spec->input;
    receipt.platform_material_required = spec->platform;
    receipt.read_only_query = spec->query;
    receipt.runtime_execution_blocked = 1;
    receipt.platform_behavior_fail_closed = 1;
    receipt.function_id = function_id;
    receipt.source_evidence = spec->evidence;
    *out = receipt;
    return 1;
}
