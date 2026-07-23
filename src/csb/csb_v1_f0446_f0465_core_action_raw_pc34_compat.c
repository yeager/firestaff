#include "csb_v1_f0446_f0465_core_action_raw_pc34_compat.h"

#include <string.h>

typedef struct CoreActionSpec {
    CSB_V1_CoreActionFunctionPc34 id;
    int platform, memory, floppy, save, dungeon, graphics, timeline, input, audio, query, existing_owner;
    const char *evidence;
} CoreActionSpec;

static const CoreActionSpec s_specs[] = {
    {446,0,0,0,0,1,1,1,0,1,0,0,"ReDMCSB ENDGAME.C F0446 fuse sequence"},
    {447,1,0,0,0,0,0,0,0,0,0,0,"ReDMCSB STARTUP1.C F0447 platform hang"},
    {448,1,1,0,0,0,0,0,0,0,0,0,"ReDMCSB STARTUP1.C F0448 memory manager"},
    {449,1,0,1,0,0,0,0,0,0,1,0,"ReDMCSB FLOPPY.C F0449 write protection"},
    {450,1,0,1,0,0,0,0,0,0,0,1,"ReDMCSB FLOPPY.C F0450 media change"},
    {451,1,1,1,0,0,0,0,0,0,0,0,"ReDMCSB FLOPPY.C F0451 initialize"},
    {452,1,0,1,1,0,0,0,0,0,1,0,"ReDMCSB FLOPPY.C F0452 disk type"},
    {453,1,0,1,1,0,0,0,0,0,1,0,"ReDMCSB FLOPPY.C F0453 format result"},
    {454,1,0,1,1,0,0,0,0,0,1,0,"ReDMCSB FLOPPY.C F0454 save disk type"},
    {455,0,1,0,1,1,0,0,0,0,0,0,"ReDMCSB DECOMPDU.C F0455 decompress dungeon"},
    {456,0,0,0,1,0,1,0,1,0,0,0,"ReDMCSB STARTUP2.C F0456 disabled menus"},
    {457,0,0,0,1,0,1,0,1,0,0,0,"ReDMCSB STARTUP2.C F0457 enabled menus"},
    {458,1,0,0,0,0,0,0,0,0,1,0,"ReDMCSB STARTUP1.C F0458 command line"},
    {459,0,1,0,0,0,1,0,0,0,1,0,"ReDMCSB BMPSIZE.C F0459 bitmap size"},
    {460,0,1,0,0,0,1,0,0,0,0,0,"ReDMCSB STARTUP1.C F0460 graphics init"},
    {461,0,1,0,0,1,1,0,0,0,0,0,"ReDMCSB MEMORY.C F0461 flipped walls"},
    {462,0,1,0,1,1,1,1,1,0,0,0,"ReDMCSB STARTUP1.C F0462 start game"},
    {463,1,1,0,1,1,1,1,1,0,0,0,"ReDMCSB STARTUP2.C F0463 initialize game"},
    {464,1,0,1,0,0,0,0,0,0,1,0,"ReDMCSB COPYPRO9.C F0464 checksum"},
    {465,1,0,1,0,0,0,0,0,0,1,0,"ReDMCSB DEFS.H S0465 sector 247"}
};

static const CoreActionSpec *find_spec(CSB_V1_CoreActionFunctionPc34 id)
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

int csb_v1_f0446_f0465_core_action_audit_pc34(
    const CSB_V1_CoreActionRawMaterialPc34 *raw,
    CSB_V1_CoreActionFunctionPc34 function_id,
    CSB_V1_CoreActionAuditReceiptPc34 *out)
{
    const CoreActionSpec *spec;
    CSB_V1_CoreActionAuditReceiptPc34 receipt;
    if (!out) return 0;
    memset(&receipt, 0, sizeof(receipt));
    *out = receipt;
    spec = find_spec(function_id);
    if (!spec || !raw || !raw->authenticated_pc34 ||
        (spec->platform && !has(raw->platform_material, raw->platform_material_size, raw->platform_material_identity)) ||
        (spec->memory && !has(raw->memory_material, raw->memory_material_size, raw->memory_material_identity)) ||
        (spec->floppy && !has(raw->floppy_material, raw->floppy_material_size, raw->floppy_material_identity)) ||
        (spec->save && !has(raw->save_material, raw->save_material_size, raw->save_material_identity)) ||
        (spec->dungeon && !has(raw->dungeon_material, raw->dungeon_material_size, raw->dungeon_material_identity)) ||
        (spec->graphics && !has(raw->graphics_material, raw->graphics_material_size, raw->graphics_material_identity)) ||
        (spec->timeline && !has(raw->timeline_material, raw->timeline_material_size, raw->timeline_material_identity)) ||
        (spec->input && !has(raw->input_material, raw->input_material_size, raw->input_material_identity)) ||
        (spec->audio && !has(raw->audio_material, raw->audio_material_size, raw->audio_material_identity))) return 0;
    receipt.raw_material_admitted = 1;
    receipt.existing_runtime_owner_preserved = spec->existing_owner;
    receipt.platform_material_required = spec->platform; receipt.memory_material_required = spec->memory;
    receipt.floppy_material_required = spec->floppy; receipt.save_material_required = spec->save;
    receipt.dungeon_material_required = spec->dungeon; receipt.graphics_material_required = spec->graphics;
    receipt.timeline_material_required = spec->timeline; receipt.input_material_required = spec->input;
    receipt.audio_material_required = spec->audio; receipt.read_only_query = spec->query;
    receipt.runtime_execution_blocked = 1; receipt.platform_behavior_fail_closed = 1;
    receipt.function_id = function_id; receipt.source_evidence = spec->evidence;
    *out = receipt;
    return 1;
}
