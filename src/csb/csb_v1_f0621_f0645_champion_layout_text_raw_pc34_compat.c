#include "csb_v1_f0621_f0645_champion_layout_text_raw_pc34_compat.h"

#include <string.h>

typedef struct ChampionLayoutTextSpec {
    CSB_V1_ChampionLayoutTextFunctionPc34 id;
    int champion, zone, graphics, font, text, input, memory, query, existing_owner;
    const char *evidence;
} ChampionLayoutTextSpec;

static const ChampionLayoutTextSpec s_specs[] = {
    {621,1,1,1,0,0,0,0,0,0,"ReDMCSB CHAMDRAW.C F0621 clear champion icon"},
    {622,1,1,1,0,0,0,1,0,0,"ReDMCSB CHAMDRAW.C F0622 prepare champion icon"},
    {623,1,1,1,0,0,0,0,0,0,"ReDMCSB CHAMDRAW.C F0623 champion damage"},
    {625,0,1,0,0,0,0,0,1,0,"ReDMCSB COORD.C F0625 zone from dimensions"},
    {626,0,1,0,0,0,0,0,1,0,"ReDMCSB COORD.C F0626 temporary zone coordinates"},
    {627,0,1,0,0,0,0,0,1,0,"ReDMCSB COORD.C F0627 temporary zone dimensions"},
    {628,0,1,0,0,0,0,0,1,0,"ReDMCSB COORD.C F0628 add zone margin"},
    {629,0,1,0,0,0,1,0,1,0,"ReDMCSB COORD.C F0629 point in zone"},
    {630,0,1,1,0,0,0,1,0,0,"ReDMCSB COORD.C F0630 bitmap struct"},
    {631,0,1,1,0,0,0,1,1,0,"ReDMCSB COORD.C F0631 bitmap pointer"},
    {632,0,1,1,0,0,0,1,0,0,"ReDMCSB COORD.C F0632 negative bitmap pointer"},
    {633,0,1,1,0,0,0,1,0,0,"ReDMCSB COORD.C F0633 negative bitmap index"},
    {635,0,1,1,0,0,0,0,1,0,"ReDMCSB COORD.C F0635 bitmap coordinates"},
    {636,0,1,0,0,0,0,0,1,0,"ReDMCSB COORD.C F0636 zone top-left"},
    {637,0,1,0,0,0,0,0,1,0,"ReDMCSB COORD.C F0637 proportional zone"},
    {638,0,1,0,0,0,0,0,1,0,"ReDMCSB COORD.C F0638 get zone"},
    {641,0,1,1,0,0,0,1,0,0,"ReDMCSB COORD.C F0641 layout initialize"},
    {642,0,0,1,1,0,0,1,0,0,"ReDMCSB FONT.C F0642 font allocation"},
    {643,0,0,1,1,1,0,1,0,0,"ReDMCSB FONT.C/TEXT2.C F0643 font data"},
    {644,0,1,1,1,1,0,0,0,0,"ReDMCSB TEXT.C F0644 alternate text"},
    {645,0,0,0,1,1,0,0,1,0,"ReDMCSB TEXT.C F0645 string dimensions"}
};

static const ChampionLayoutTextSpec *find_spec(CSB_V1_ChampionLayoutTextFunctionPc34 id)
{ size_t i; for(i=0;i<sizeof(s_specs)/sizeof(s_specs[0]);++i)if(s_specs[i].id==id)return &s_specs[i];return NULL; }
static int has(const uint8_t *bytes,size_t size,uint32_t identity){return bytes!=NULL&&size!=0&&identity!=0;}

int csb_v1_f0621_f0645_champion_layout_text_audit_pc34(const CSB_V1_ChampionLayoutTextRawMaterialPc34 *raw,
    CSB_V1_ChampionLayoutTextFunctionPc34 function_id, CSB_V1_ChampionLayoutTextAuditReceiptPc34 *out)
{
    const ChampionLayoutTextSpec *spec; CSB_V1_ChampionLayoutTextAuditReceiptPc34 receipt;
    if(!out)return 0;
    memset(&receipt,0,sizeof(receipt));*out=receipt;spec=find_spec(function_id);
    if(!spec||!raw||!raw->authenticated_pc34||
       (spec->champion&&!has(raw->champion_material,raw->champion_material_size,raw->champion_material_identity))||
       (spec->zone&&!has(raw->zone_layout_material,raw->zone_layout_material_size,raw->zone_layout_material_identity))||
       (spec->graphics&&!has(raw->graphics_material,raw->graphics_material_size,raw->graphics_material_identity))||
       (spec->font&&!has(raw->font_material,raw->font_material_size,raw->font_material_identity))||
       (spec->text&&!has(raw->text_material,raw->text_material_size,raw->text_material_identity))||
       (spec->input&&!has(raw->input_material,raw->input_material_size,raw->input_material_identity))||
       (spec->memory&&!has(raw->memory_material,raw->memory_material_size,raw->memory_material_identity)))return 0;
    receipt.raw_material_admitted=1;receipt.existing_runtime_owner_preserved=spec->existing_owner;
    receipt.champion_material_required=spec->champion;receipt.zone_layout_material_required=spec->zone;receipt.graphics_material_required=spec->graphics;
    receipt.font_material_required=spec->font;receipt.text_material_required=spec->text;receipt.input_material_required=spec->input;receipt.memory_material_required=spec->memory;
    receipt.read_only_query=spec->query;receipt.runtime_execution_blocked=1;receipt.platform_behavior_fail_closed=1;receipt.function_id=function_id;receipt.source_evidence=spec->evidence;*out=receipt;return 1;
}
