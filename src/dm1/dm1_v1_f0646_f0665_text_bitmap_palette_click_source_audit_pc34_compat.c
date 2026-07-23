#include "dm1_v1_f0646_f0665_text_bitmap_palette_click_source_audit_pc34_compat.h"

static const DM1_V1_F0646F0665SourceAuditPc34 k_audit[] = {
    { 646u, "TEXT.C:179 F0646_GetLargestPrintableSubString", "dm1_v1_text_message_pc34_compat", 1, 1, 1 },
    { 647u, "TEXT.C:1838 F0647_PrintLongForDebugMemory", "memory_cache_usage_pc34_compat", 1, 1, 1 },
    { 648u, "TEXT.C:1937 F0648_PrintTextInViewportZone", "dm1_v1_layout_zones_pc34_compat", 1, 1, 1 },
    { 649u, "TEXT.C:1954 F0649_PrintCenteredTextToViewportZone", "dm1_v1_text_message_pc34_compat", 1, 1, 1 },
    { 650u, "TEXT.C:1972 F0650_PrintCenteredTextToScreenZone", "dm1_v1_champion_panel_name_box_clip_pc34_compat", 1, 1, 1 },
    { 651u, "TIMELINE.C:100 F0651_TIMELINE_InitializeOptimizedManagement", "redmcsb_f0651_timeline_free_list_pc34_compat", 1, 1, 1 },
    { 652u, "TIMELINE.C:423 F0652_MergeEvent", "redmcsb_f0652_merge_event_pc34_compat", 1, 1, 1 },
    { 653u, "BASE.C:1171 F0653_GetBitmapByteCount", "base_frontend_pc34", 1, 1, 1 },
    { 654u, "BASE.C:1181 F0654_Call_F0132_VIDEO_Blit", "base_frontend_pc34", 1, 1, 1 },
    { 655u, "BASE.C:1216 F0655_CopyBitmapAndFlip", "redmcsb_f0655_f0656_viewport_bitmap_pc34_compat", 1, 1, 1 },
    { 656u, "BASE.C:1292 F0656_BlitBitmapToViewportZoneIndexWithTransparency", "redmcsb_f0655_f0656_viewport_bitmap_pc34_compat", 1, 1, 1 },
    { 657u, "BASE.C:1320 F0657_BlitBitmapIndexToViewportZoneWithTransparency", "redmcsb_f0657_f0658_viewport_bitmap_index_pc34_compat", 1, 1, 1 },
    { 658u, "BASE.C:1341 F0658_BlitBitmapIndexToZoneIndexWithTransparency", "redmcsb_f0657_f0658_viewport_bitmap_index_pc34_compat", 1, 1, 1 },
    { 659u, "BASE.C:1449 F0659 screen-zone bitmap path", "dm1_v1_f0659_shield_material_pc34_compat", 1, 1, 1 },
    { 660u, "BASE.C:1473 F0660 screen-zone bitmap path", "dm1_v1_champion_panel_damage_indicator_pc34_compat", 1, 1, 1 },
    { 661u, "BASE.C:1526 F0661_GetShrinkedBitmap", "redmcsb_f0661_get_shrinked_bitmap_pc34_compat", 1, 1, 1 },
    { 662u, "BASE.C:1548 F0662_ApplyPaletteChanges", "dm1_v1_f0662_invisibility_material_pc34_compat", 1, 1, 1 },
    { 663u, "BASE.C:1555 F0663_CopyBitmapWithPaletteChanges", "dm1_v1_f0663_smoke_material_pc34_compat", 1, 1, 1 },
    { 664u, "CLIKVIEW.C:30 F0664_COMMAND_ProcessType80_ClickInDungeonView_KnockOnFrontWall", "redmcsb_f0664_knock_front_wall_pc34_compat", 1, 1, 1 },
    { 665u, "CLIKMENU.C:10 F0665_F0362_sub highlight path", "redmcsb_f0665_highlight_box_pc34_compat", 1, 1, 1 }
};

const DM1_V1_F0646F0665SourceAuditPc34 *
dm1_v1_f0646_f0665_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_F0646F0665SourceAuditPc34 *
dm1_v1_f0646_f0665_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_f0646_f0665_source_audit_evidence_pc34(void)
{
    return "ReDMCSB TEXT.C, TIMELINE.C, BASE.C, CLIKVIEW.C, and CLIKMENU.C "
           "are the authority for F0646-F0665. This audit records existing "
           "owners only; they require raw source or PC34 material and fail closed "
           "when unavailable. The audit does not render or synthesize click paths.";
}
