#include "dm1_v1_f0786_f0805_runtime_panel_source_audit_pc34_compat.h"

static const DM1_V1_F0786F0805SourceAuditPc34 k_audit[] = {
    { 786u, "IO.C:3818 F0786_ IODRV_24 gate", "fail_closed: PC34 IODRV_24 unavailable", 1, 1, 1, 1 },
    { 787u, "COORD.C:1840 F0787_GetZoneInitializedFromCoordinates", "redmcsb_f0787_get_zone_initialized_from_coordinates_pc34_compat", 1, 1, 1, 1 },
    { 788u, "COORD.C:2415 F0788_ zone-coordinate request", "fail_closed: F0635 layout-coordinate contract unavailable", 1, 1, 1, 1 },
    { 789u, "COORD.C:2536 F0789_AllocateLayoutRange", "redmcsb_f0789_allocate_layout_range_pc34_compat", 1, 1, 1, 1 },
    { 790u, "IMAGE4.C:7 F0790_ packed-nibble write", "image_backend_pc34_compat", 1, 1, 1, 1 },
    { 791u, "DUNVIEW.C:3394 F0791_DUNGEONVIEW_DrawBitmapXX", "redmcsb_f0791_dungeonview_draw_bitmap_xx_pc34_compat", 1, 1, 1, 1 },
    { 792u, "DUNVIEW.C:3288 F0792_DUNGEONVIEW_DrawBitmapYYY", "redmcsb_f0792_dungeonview_draw_bitmap_yyy_pc34_compat", 1, 1, 1, 1 },
    { 793u, "no numbered F0793 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 794u, "no numbered F0794 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 795u, "no numbered F0795 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 796u, "no numbered F0796 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 797u, "ENTRANCE.C:58 F0797_STARTEND_DrawEntranceMicroDungeon", "dm1_v1_startup_sequence_pc34_compat", 1, 1, 1, 1 },
    { 798u, "COORD.C:1915 F0798_COMMAND_IsPointInZone", "redmcsb_f0798_command_is_point_in_zone_pc34_compat", 1, 1, 1, 1 },
    { 799u, "SOUND.C:1397 F0799_SOUND_DisableUnavailableSounds", "redmcsb_f0799_sound_disable_unavailable_sounds_pc34_compat", 1, 1, 1, 1 },
    { 800u, "no numbered F0800 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 801u, "no numbered F0801 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 802u, "PANEL.C:519 F0802_IsMagicMap", "redmcsb_f0802_is_magic_map_pc34_compat", 1, 1, 1, 1 },
    { 803u, "PANEL.C:534 F0803_DrawMagicMapIcon", "redmcsb_f0803_draw_magic_map_icon_pc34_compat", 1, 1, 1, 1 },
    { 804u, "PANEL.C:545 F0804_DrawMagicMap", "redmcsb_f0804_draw_magic_map_pc34_compat", 1, 1, 1, 1 },
    { 805u, "PANEL.C:802 F0805_CreatureNameScroll", "redmcsb_f0805_creature_name_scroll_pc34_compat", 1, 1, 1, 1 }
};

const DM1_V1_F0786F0805SourceAuditPc34 *
dm1_v1_f0786_f0805_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_F0786F0805SourceAuditPc34 *
dm1_v1_f0786_f0805_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_f0786_f0805_source_audit_evidence_pc34(void)
{
    return "ReDMCSB IO.C, COORD.C, IMAGE4.C, DUNVIEW.C, ENTRANCE.C, SOUND.C, "
           "and PANEL.C are the authority for F0786-F0805. F0793-F0796 and "
           "F0800-F0801 have no numbered source body in the audited corpus and "
           "remain fail closed. This audit does not render or synthesize UI or "
           "timing paths.";
}
