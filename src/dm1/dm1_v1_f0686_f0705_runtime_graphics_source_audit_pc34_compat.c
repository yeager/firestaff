#include "dm1_v1_f0686_f0705_runtime_graphics_source_audit_pc34_compat.h"

static const DM1_V1_F0686F0705SourceAuditPc34 k_audit[] = {
    { 686u, "IMAGE2.C:19 F0686_IMG_CopyFromPreviousLine", "redmcsb_f0686_copy_previous_line_pc34_compat", 1, 1, 1 },
    { 687u, "IMAGE2.C:91 F0687_IMG3_GetNibble", "image_backend_pc34_compat", 1, 1, 1 },
    { 688u, "IMAGE2.C:101 F0688_IMG3_GetPixelCount", "image_backend_pc34_compat", 1, 1, 1 },
    { 689u, "IMAGE2.C:120 F0689_IMG_ExpandGraphicToBitmap", "redmcsb_f0689_img3_expand_pc34_compat", 1, 1, 1 },
    { 690u, "IMAGE3.C:1124 F0690_CopyPixelLineToScreenWithoutTransparency", "redmcsb_f0690_copy_pixel_line_to_screen_pc34_compat", 1, 1, 1 },
    { 691u, "IMAGE3.C:1030 F0691_IMG_ExpandGraphicToScreen", "redmcsb_f0691_draw_compressed_img3_pc34_compat", 1, 1, 1 },
    { 692u, "IMAGE3.C:1166 F0692_FillBox", "redmcsb_f0692_fill_box_pc34_compat", 1, 1, 1 },
    { 693u, "IMAGE.C:40 F0693_WaitVerticalBlank", "image_frontend_pc34", 1, 1, 1 },
    { 694u, "DRAWVIEW.C:640 F0694_SetMultipleColorsInPalette", "image_frontend_pc34", 1, 1, 1 },
    { 695u, "DRAWVIEW.C:682 F0695_SetCreatureReplacementColors", "redmcsb_f0695_set_creature_replacement_colors_pc34_compat", 1, 1, 1 },
    { 696u, "DRAWMSGA.C F0696_UpdateMessageArea", "redmcsb_f0696_update_message_area_pc34_compat", 1, 1, 1 },
    { 697u, "IMAGE.C:160 F0697_HatchScreenBox", "redmcsb_f0697_hatch_screen_box_pc34_compat", 1, 1, 1 },
    { 698u, "IMAGE.C:182 F0698_InvertBox", "redmcsb_f0698_invert_box_pc34_compat", 1, 1, 1 },
    { 699u, "IMAGE.C:393 F0699_InitVideoInterrupt", "redmcsb_f0699_video_interrupt_pc34_compat", 1, 1, 1 },
    { 700u, "IO.C:680 F0700_TriggerImmediateMouseEvent", "redmcsb_f0700_trigger_immediate_mouse_event_pc34_compat", 1, 1, 1 },
    { 701u, "DUNGEON.C:1318 F0150 step-delta tables consumed by F0701", "memory_movement_pc34_compat", 1, 1, 1 },
    { 702u, "IO.C:1694 F0702_BuildObjectMousePointerIcon", "redmcsb_f0702_build_object_mouse_pointer_icon_pc34_compat", 1, 1, 1 },
    { 703u, "IO.C:3298 F0703_ReleaseChampionIcon", "redmcsb_f0703_release_champion_icon_pc34_compat", 1, 1, 1 },
    { 704u, "IO.C:3725 F0704 I/O driver slot 03", "redmcsb_f0704_io_driver_slot03_pc34_compat", 1, 1, 1 },
    { 705u, "IO.C:3732 F0705 I/O driver slot 04", "redmcsb_f0705_invoke_io_driver_04_pc34_compat", 1, 1, 1 }
};

const DM1_V1_F0686F0705SourceAuditPc34 *
dm1_v1_f0686_f0705_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_F0686F0705SourceAuditPc34 *
dm1_v1_f0686_f0705_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_f0686_f0705_source_audit_evidence_pc34(void)
{
    return "ReDMCSB IMAGE2.C, IMAGE3.C, IMAGE.C, DRAWVIEW.C, DRAWMSGA.C, "
           "and IO.C are the authority for F0686-F0705. This audit records "
           "existing owners only; they require raw source or PC34 material and "
           "fail closed when unavailable. The audit does not render or synthesize "
           "UI or timing paths.";
}
