#include "csb_v1_f0526_f0545_platform_input_raw_pc34_compat.h"

#include <string.h>

typedef struct PlatformInputSpec {
    CSB_V1_PlatformInputFunctionPc34 id;
    int platform, floppy, input, memory, graphics, query, existing_owner, unavailable;
    const char *evidence;
} PlatformInputSpec;

static const PlatformInputSpec s_specs[] = {
    {526,1,0,0,0,0,0,0,1,"ReDMCSB FLOPPYAM.C F0526 Amiga-only"},
    {527,1,1,0,1,0,1,0,0,"ReDMCSB IO2.C F0527 fuzzy sector"},
    {528,1,1,0,0,0,0,0,0,"ReDMCSB FLOPPY.C F0528 eject"},
    {529,1,1,0,0,0,1,0,0,"ReDMCSB FLOPPY.C F0529 floppy present"},
    {530,1,1,0,0,0,1,0,0,"ReDMCSB FLOPPY.C F0530 directory exists"},
    {531,1,1,0,0,0,1,0,0,"ReDMCSB FLOPPY.C F0531 volume name"},
    {532,1,1,0,0,0,1,0,0,"ReDMCSB FLOPPY.C F0532 volume info"},
    {533,1,1,0,0,0,0,0,0,"ReDMCSB FLOPPY.C F0533 save info files"},
    {534,1,1,0,0,0,1,0,0,"ReDMCSB FLOPPY.C F0534 file size"},
    {535,1,1,0,1,1,1,0,0,"ReDMCSB MEMORY.C F0535 GRAPHICS.DAT size"},
    {536,1,0,1,1,0,0,0,0,"ReDMCSB INPUT.C F0536 initialize"},
    {537,1,0,1,0,0,0,0,0,"ReDMCSB INPUT.C F0537 release input"},
    {538,1,0,1,0,0,0,0,0,"ReDMCSB INPUT.C F0538 deinitialize"},
    {539,1,0,1,0,0,1,1,0,"ReDMCSB INPUT.C F0539 keyboard pending"},
    {540,1,0,1,0,0,1,1,0,"ReDMCSB INPUT.C F0540 read character"},
    {541,1,0,1,0,0,0,0,0,"ReDMCSB INPUT.C F0541 wait activity"},
    {543,1,0,1,0,0,0,0,1,"ReDMCSB INPUT.C F0543 interrupt handler"},
    {544,1,0,1,0,0,0,0,0,"ReDMCSB INPUT.C F0544 reset eye/mouth"},
    {545,1,0,1,1,0,0,0,0,"ReDMCSB IO.C F0545 mouse allocation"}
};

static const PlatformInputSpec *find_spec(CSB_V1_PlatformInputFunctionPc34 id)
{ size_t i; for (i=0;i<sizeof(s_specs)/sizeof(s_specs[0]);++i) if(s_specs[i].id==id) return &s_specs[i]; return NULL; }
static int has(const uint8_t *bytes,size_t size,uint32_t identity) { return bytes != NULL && size != 0 && identity != 0; }

int csb_v1_f0526_f0545_platform_input_audit_pc34(const CSB_V1_PlatformInputRawMaterialPc34 *raw,
    CSB_V1_PlatformInputFunctionPc34 function_id, CSB_V1_PlatformInputAuditReceiptPc34 *out)
{
    const PlatformInputSpec *spec; CSB_V1_PlatformInputAuditReceiptPc34 receipt;
    if (!out) return 0;
    memset(&receipt,0,sizeof(receipt)); *out=receipt; spec=find_spec(function_id);
    if(!spec || !raw || !raw->authenticated_pc34 || spec->unavailable ||
       (spec->platform && !has(raw->platform_material,raw->platform_material_size,raw->platform_material_identity)) ||
       (spec->floppy && !has(raw->floppy_material,raw->floppy_material_size,raw->floppy_material_identity)) ||
       (spec->input && !has(raw->input_material,raw->input_material_size,raw->input_material_identity)) ||
       (spec->memory && !has(raw->memory_material,raw->memory_material_size,raw->memory_material_identity)) ||
       (spec->graphics && !has(raw->graphics_material,raw->graphics_material_size,raw->graphics_material_identity))) return 0;
    receipt.raw_material_admitted=1; receipt.existing_runtime_owner_preserved=spec->existing_owner;
    receipt.platform_material_required=spec->platform; receipt.floppy_material_required=spec->floppy;
    receipt.input_material_required=spec->input; receipt.memory_material_required=spec->memory;
    receipt.graphics_material_required=spec->graphics; receipt.read_only_query=spec->query;
    receipt.runtime_execution_blocked=1; receipt.platform_behavior_fail_closed=1;
    receipt.function_id=function_id; receipt.source_evidence=spec->evidence; *out=receipt; return 1;
}
