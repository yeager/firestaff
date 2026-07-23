#include "csb_v1_f0486_f0505_graphics_lzw_raw_pc34_compat.h"

#include <string.h>

typedef struct GraphicsLzwSpec {
    CSB_V1_GraphicsLzwFunctionPc34 id;
    int memory, graphics, cache, compressed, audio, platform, query, existing_owner, unavailable;
    const char *evidence;
} GraphicsLzwSpec;

static const GraphicsLzwSpec s_specs[] = {
    {486,1,0,1,0,0,0,0,0,0,"ReDMCSB MEMORY.C F0486 used list"},
    {487,1,0,1,0,0,0,1,0,0,"ReDMCSB MEMORY.C F0487 cache lookup"},
    {488,1,1,1,0,0,0,0,1,0,"ReDMCSB MEMORY.C F0488 expand graphic"},
    {489,1,1,1,0,0,0,1,0,0,"ReDMCSB MEMORY.C F0489 native bitmap"},
    {490,1,1,1,1,0,0,0,1,0,"ReDMCSB MEMORY.C F0490 load/decompress"},
    {491,1,0,1,0,0,0,1,0,0,"ReDMCSB MEMORY.C F0491 derived bitmap cache"},
    {492,1,1,1,0,0,0,1,0,0,"ReDMCSB MEMORY.C F0492 derived bitmap"},
    {493,1,1,1,0,0,0,0,0,0,"ReDMCSB MEMORY.C F0493 add derived bitmap"},
    {494,1,1,0,0,0,0,1,0,0,"ReDMCSB MEMORY.C F0494 graphic byte count"},
    {495,1,0,0,1,0,0,1,0,0,"ReDMCSB LZW.C F0495 next input code"},
    {496,1,0,0,1,0,0,0,1,0,"ReDMCSB LZW.C F0496 output character"},
    {497,1,0,0,1,0,0,0,1,0,"ReDMCSB LZW.C F0497 decompress"},
    {500,0,0,0,0,1,1,0,0,1,"ReDMCSB _MAIN.C F0500 Amiga-only"},
    {501,0,0,0,0,1,1,0,0,1,"ReDMCSB SOUND.C F0501 Amiga-only"},
    {502,0,0,0,0,1,1,0,0,1,"ReDMCSB SOUND.C F0502 Amiga-only"},
    {503,0,0,0,0,1,1,0,0,1,"ReDMCSB SOUND.C F0503 Amiga-only"},
    {504,0,1,0,0,1,1,0,0,1,"ReDMCSB SOUND.C F0504 Amiga-only"},
    {505,0,0,0,0,1,1,0,0,1,"ReDMCSB SOUND.C F0505 Amiga-only"}
};

static const GraphicsLzwSpec *find_spec(CSB_V1_GraphicsLzwFunctionPc34 id)
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

int csb_v1_f0486_f0505_graphics_lzw_audit_pc34(
    const CSB_V1_GraphicsLzwRawMaterialPc34 *raw,
    CSB_V1_GraphicsLzwFunctionPc34 function_id,
    CSB_V1_GraphicsLzwAuditReceiptPc34 *out)
{
    const GraphicsLzwSpec *spec;
    CSB_V1_GraphicsLzwAuditReceiptPc34 receipt;
    if (!out) return 0;
    memset(&receipt, 0, sizeof(receipt));
    *out = receipt;
    spec = find_spec(function_id);
    if (!spec || !raw || !raw->authenticated_pc34 || spec->unavailable ||
        (spec->memory && !has(raw->memory_material, raw->memory_material_size, raw->memory_material_identity)) ||
        (spec->graphics && !has(raw->graphics_material, raw->graphics_material_size, raw->graphics_material_identity)) ||
        (spec->cache && !has(raw->cache_material, raw->cache_material_size, raw->cache_material_identity)) ||
        (spec->compressed && !has(raw->compressed_material, raw->compressed_material_size, raw->compressed_material_identity)) ||
        (spec->audio && !has(raw->audio_material, raw->audio_material_size, raw->audio_material_identity)) ||
        (spec->platform && !has(raw->platform_material, raw->platform_material_size, raw->platform_material_identity))) return 0;
    receipt.raw_material_admitted = 1; receipt.existing_runtime_owner_preserved = spec->existing_owner;
    receipt.memory_material_required = spec->memory; receipt.graphics_material_required = spec->graphics;
    receipt.cache_material_required = spec->cache; receipt.compressed_material_required = spec->compressed;
    receipt.audio_material_required = spec->audio; receipt.platform_material_required = spec->platform;
    receipt.read_only_query = spec->query; receipt.runtime_execution_blocked = 1;
    receipt.platform_behavior_fail_closed = 1; receipt.function_id = function_id;
    receipt.source_evidence = spec->evidence; *out = receipt;
    return 1;
}
