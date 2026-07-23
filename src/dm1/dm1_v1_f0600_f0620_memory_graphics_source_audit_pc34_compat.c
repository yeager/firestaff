#include "dm1_v1_f0600_f0620_memory_graphics_source_audit_pc34_compat.h"

static const DM1_V1_F0600F0620SourceAuditPc34 k_audit[] = {
    { 600u, "DIALOG.C:271 F0600_DIALOG_subroutine", "dialog_frontend_pc34_compat", 1, 1, 1 },
    { 601u, "DUNGEON.C:1350 F0601_IsWallOrnamentAFountain", "dm1_v1_fountain_interaction_pc34_compat", 1, 1, 1 },
    { 602u, "MEMORY.C:222 F0602_GetMinimumOverheadMemoryChunk", "memory_cache_allocator_pc34_compat", 1, 1, 1 },
    { 603u, "MEMORY.C:255 F0603_AllocateFromTopMemoryChunk", "memory_cache_allocator_pc34_compat", 1, 1, 1 },
    { 604u, "MEMORY.C:263 F0604_BackupCurrentMemoryChunkInfo", "memory_cache_allocator_pc34_compat", 1, 1, 1 },
    { 605u, "MEMORY.C:283 F0605_RestoreMemoryChunkInfo", "memory_cache_allocator_pc34_compat", 1, 1, 1 },
    { 606u, "MEMORY.C:427 F0606_AllocateMemForGraphic", "memory_cache_frontend_pc34_compat", 1, 1, 1 },
    { 607u, "MEMORY.C:447 F0607_FreeMemForGraphic", "memory_cache_frontend_pc34_compat", 1, 1, 1 },
    { 608u, "MEMORY.C:666 F0608_GetLanguageSpecificGraphicIndex", "memory_load_pipeline_pc34_compat", 1, 1, 1 },
    { 609u, "MEMORY.C:1723 F0609_InitializeMemoryArea", "memory_cache_index_pc34_compat", 1, 1, 1 },
    { 610u, "MEMORY.C:1735 F0610_DefragmentMemoryArea", "memory_cache_index_pc34_compat", 1, 1, 1 },
    { 611u, "MEMORY.C:1767 F0611_MarkGraphicAsUnused", "memory_cache_index_pc34_compat", 1, 1, 1 },
    { 612u, "MEMORY.C:1778 F0612_RemoveGraphic", "memory_cache_index_pc34_compat", 1, 1, 1 },
    { 613u, "MEMORY.C:1801 F0613_AllocateGraphicInMemoryArea", "memory_cache_index_pc34_compat", 1, 1, 1 },
    { 614u, "MEMORY.C:1809 F0614_CopyToMemoryArea", "memory_cache_index_pc34_compat", 1, 1, 1 },
    { 615u, "MEMORY.C:2715 F0615_CopyBitmapDimensions", "bitmap_copy_pc34_compat", 1, 1, 1 },
    { 616u, "MEMORY.C:2722 F0616_CopyBitmap", "bitmap_copy_pc34_compat", 1, 1, 1 },
    { 617u, "MEMORY.C:2777 F0617_PrintMemoryUsage", "memory_cache_usage_pc34_compat", 1, 1, 1 },
    { 618u, "OBJECT.C:365 F0618_LoadZone", "dm1_v1_slot_boxes_pc34_compat", 1, 1, 1 },
    { 619u, "OBJECT.C:378 F0619_GetSlotBoxBorderCoordinates", "dm1_v1_slot_boxes_pc34_compat", 1, 1, 1 },
    { 620u, "MENU.C:543 F0620_LoadActionNames", "dm1_v1_action_list_pc34_compat", 1, 1, 1 }
};

const DM1_V1_F0600F0620SourceAuditPc34 *
dm1_v1_f0600_f0620_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_F0600F0620SourceAuditPc34 *
dm1_v1_f0600_f0620_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_f0600_f0620_source_audit_evidence_pc34(void)
{
    return "ReDMCSB DIALOG.C, DUNGEON.C, MEMORY.C, OBJECT.C, and MENU.C are "
           "the authority for F0600-F0620. This audit records existing owners "
           "only; they require raw source or PC34 material and fail closed when "
           "unavailable. The audit does not render or synthesize UI.";
}
