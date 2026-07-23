#include "csb_v1_f0646_f0665_text_bitmap_click_raw_pc34_compat.h"

#include <string.h>

typedef struct TextBitmapClickSpec {
    CSB_V1_TextBitmapClickFunctionPc34 id;
    int text,font,graphics,zone,timeline,palette,dungeon,input,memory,query,existing_owner;
    const char *evidence;
} TextBitmapClickSpec;

static const TextBitmapClickSpec s_specs[] = {
    {646,1,1,0,1,0,0,0,0,0,1,0,"ReDMCSB TEXT.C F0646 printable substring"},
    {647,1,1,0,0,0,0,0,0,1,0,0,"ReDMCSB TEXT.C F0647 debug text"},
    {648,1,1,1,1,0,0,0,0,0,0,0,"ReDMCSB TEXT.C F0648 viewport text"},
    {649,1,1,1,1,0,0,0,0,0,0,0,"ReDMCSB TEXT.C F0649 centered viewport text"},
    {650,1,1,1,1,0,0,0,0,0,0,0,"ReDMCSB TEXT.C F0650 centered screen text"},
    {651,0,0,0,0,1,0,0,0,1,0,1,"ReDMCSB TIMELINE.C F0651 timeline initialize"},
    {652,0,0,0,0,1,0,1,0,1,0,1,"ReDMCSB TIMELINE.C F0652 merge event"},
    {653,0,0,1,0,0,0,0,0,1,1,0,"ReDMCSB BASE.C F0653 bitmap byte count"},
    {654,0,0,1,1,0,0,0,0,0,0,0,"ReDMCSB BASE.C F0654 video blit"},
    {655,0,0,1,0,0,0,0,0,0,0,1,"ReDMCSB BASE.C F0655 copy/flip bitmap"},
    {656,0,0,1,1,0,0,0,0,0,0,1,"ReDMCSB BASE.C F0656 viewport bitmap"},
    {657,0,0,1,1,0,0,0,0,0,0,1,"ReDMCSB BASE.C F0657 bitmap index viewport"},
    {658,0,0,1,1,0,0,0,0,0,0,1,"ReDMCSB BASE.C F0658 bitmap index zone"},
    {659,0,0,1,1,0,0,0,0,0,0,0,"ReDMCSB BASE.C F0659 bitmap primitive"},
    {660,0,0,1,1,0,0,0,0,0,0,0,"ReDMCSB BASE.C F0660 bitmap zone"},
    {661,0,0,1,0,0,1,0,0,1,1,1,"ReDMCSB BASE.C F0661 shrink bitmap"},
    {662,0,0,1,0,0,1,0,0,0,0,1,"ReDMCSB BASE.C F0662 palette changes"},
    {663,0,0,1,0,0,1,0,0,0,0,1,"ReDMCSB BASE.C F0663 bitmap palette copy"},
    {664,0,0,1,1,0,0,1,1,0,0,1,"ReDMCSB CLIKVIEW.C F0664 knock wall"},
    {665,0,0,1,1,0,0,0,1,0,0,1,"ReDMCSB CLIKMENU.C F0665 highlight box"}
};
static const TextBitmapClickSpec *find_spec(CSB_V1_TextBitmapClickFunctionPc34 id){size_t i;for(i=0;i<sizeof(s_specs)/sizeof(s_specs[0]);++i)if(s_specs[i].id==id)return &s_specs[i];return NULL;}
static int has(const uint8_t *b,size_t s,uint32_t id){return b!=NULL&&s!=0&&id!=0;}

int csb_v1_f0646_f0665_text_bitmap_click_audit_pc34(const CSB_V1_TextBitmapClickRawMaterialPc34 *raw,
    CSB_V1_TextBitmapClickFunctionPc34 function_id, CSB_V1_TextBitmapClickAuditReceiptPc34 *out)
{
 const TextBitmapClickSpec *spec;CSB_V1_TextBitmapClickAuditReceiptPc34 r;if(!out)return 0;memset(&r,0,sizeof(r));*out=r;spec=find_spec(function_id);
 if(!spec||!raw||!raw->authenticated_pc34||(spec->text&&!has(raw->text_material,raw->text_material_size,raw->text_material_identity))||(spec->font&&!has(raw->font_material,raw->font_material_size,raw->font_material_identity))||(spec->graphics&&!has(raw->graphics_material,raw->graphics_material_size,raw->graphics_material_identity))||(spec->zone&&!has(raw->zone_material,raw->zone_material_size,raw->zone_material_identity))||(spec->timeline&&!has(raw->timeline_material,raw->timeline_material_size,raw->timeline_material_identity))||(spec->palette&&!has(raw->palette_material,raw->palette_material_size,raw->palette_material_identity))||(spec->dungeon&&!has(raw->dungeon_material,raw->dungeon_material_size,raw->dungeon_material_identity))||(spec->input&&!has(raw->input_material,raw->input_material_size,raw->input_material_identity))||(spec->memory&&!has(raw->memory_material,raw->memory_material_size,raw->memory_material_identity)))return 0;
 r.raw_material_admitted=1;r.existing_runtime_owner_preserved=spec->existing_owner;r.text_material_required=spec->text;r.font_material_required=spec->font;r.graphics_material_required=spec->graphics;r.zone_material_required=spec->zone;r.timeline_material_required=spec->timeline;r.palette_material_required=spec->palette;r.dungeon_material_required=spec->dungeon;r.input_material_required=spec->input;r.memory_material_required=spec->memory;r.read_only_query=spec->query;r.runtime_execution_blocked=1;r.platform_behavior_fail_closed=1;r.function_id=function_id;r.source_evidence=spec->evidence;*out=r;return 1;
}
