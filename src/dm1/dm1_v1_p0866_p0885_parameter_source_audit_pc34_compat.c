#include "dm1_v1_p0866_p0885_parameter_source_audit_pc34_compat.h"

static const DM1_V1_P0866P0885SourceAuditPc34 k_audit[] = {
    { 866u, "STARTUP2.C:440 P0866_pc_CommandLineParameters / F0458", "dm1_v1_f0458_start_get_command_line_parameters_cpsa_pc34_compat", 1, 1, 1, 1 },
    { 867u, "BMPSIZE.C:10 P0867_i_ByteWidth / F0459", "redmcsb_start_bitmap_size_pc34_compat", 1, 1, 1, 1 },
    { 868u, "BMPSIZE.C:10 P0868_i_Height / F0459", "redmcsb_start_bitmap_size_pc34_compat", 1, 1, 1, 1 },
    { 869u, "BMPSIZE.C:10 P0869_i_Scale / F0459", "redmcsb_start_bitmap_size_pc34_compat", 1, 1, 1, 1 },
    { 870u, "COPYPRO9.C:15 P0870_pfv_Function / F0464", "fail_closed: platform copy-protection checksum route", 1, 1, 1, 1 },
    { 871u, "EXPAND.C:15 P0871_puc_Graphic / F0466", "expand_frontend_pc34_compat", 1, 1, 1, 1 },
    { 872u, "EXPAND.C:15 P0872_puc_Bitmap / F0466", "expand_frontend_pc34_compat", 1, 1, 1, 1 },
    { 873u, "EXPAND.C:15 P0873_i_X / F0466", "expand_frontend_pc34_compat", 1, 1, 1, 1 },
    { 874u, "EXPAND.C:15 P0874_i_Y / F0466", "expand_frontend_pc34_compat", 1, 1, 1, 1 },
    { 875u, "MEMORY.C:147 P0875_ui_GraphicIndex / F0467", "memory_graphics_dat_metadata_pc34_compat", 1, 1, 1, 1 },
    { 876u, "MEMORY.C:306 P0876_l_ByteCount / F0468", "memory_cache_allocator_pc34_compat", 1, 1, 1, 1 },
    { 877u, "MEMORY.C:306 P0877_ui_AllocationType / F0468", "memory_cache_allocator_pc34_compat", 1, 1, 1, 1 },
    { 878u, "MEMORY.C:415 P0878_l_ByteCount / F0469", "memory_cache_frontend_pc34_compat", 1, 1, 1, 1 },
    { 879u, "MEMORY.C:417 P0879_l_ByteCount / F0470", "redmcsb_f0470_memory_free_at_heap_bottom_pc34_compat", 1, 1, 1, 1 },
    { 880u, "MEMORY.C:456 P0880_puc_Block / F0471", "redmcsb_f0471_cache_remove_unused_block_pc34_compat", 1, 1, 1, 1 },
    { 881u, "MEMORY.C:488 P0881_puc_Block / F0472", "redmcsb_f0472_cache_add_unused_block_pc34_compat", 1, 1, 1, 1 },
    { 882u, "MEMORY.C:555 P0882_pui_Values / F0473", "redmcsb_f0473_memory_sort_values_pc34_compat", 1, 1, 1, 1 },
    { 883u, "MEMORY.C:555 P0883_ValueCount / F0473", "redmcsb_f0473_memory_sort_values_pc34_compat", 1, 1, 1, 1 },
    { 884u, "MEMORY.C:709 P0884_i_GraphicIndex / F0474", "memory_graphics_dat_pc34_compat", 1, 1, 1, 1 },
    { 885u, "MEMORY.C:709 P0885_puc_Graphic / F0474", "memory_graphics_dat_pc34_compat", 1, 1, 1, 1 }
};

const DM1_V1_P0866P0885SourceAuditPc34 *
dm1_v1_p0866_p0885_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_P0866P0885SourceAuditPc34 *
dm1_v1_p0866_p0885_source_audit_find_pc34(unsigned int parameter_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].parameter_number == parameter_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_p0866_p0885_source_audit_evidence_pc34(void)
{
    return "ReDMCSB labels 0866-0885 as P parameters, not F functions. "
           "STARTUP2.C, BMPSIZE.C, COPYPRO9.C, EXPAND.C, and MEMORY.C bind "
           "them to F0458-F0474. Existing owners require raw source or PC34 "
           "material and fail closed when unavailable. The audit does not render "
           "or synthesize UI or timing paths.";
}
