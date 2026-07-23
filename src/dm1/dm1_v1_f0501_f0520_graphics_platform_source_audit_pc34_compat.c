#include "dm1_v1_f0501_f0520_graphics_platform_source_audit_pc34_compat.h"

static const DM1_V1_F0501F0520SourceAuditPc34 k_audit[] = {
    { 501u, "SOUND.C:181 F0501_SOUND_Deinitialize", "dm1_v1_sound_pc34_compat", 1, 1, 1 },
    { 502u, "SOUND.C:227 F0502_SOUND_CompareVolumes", "dm1_v1_sound_pc34_compat", 1, 1, 1 },
    { 503u, "SOUND.C:1423 F0503_SOUND_LoadAll", "dm1_v1_sound_pc34_compat", 1, 1, 1 },
    { 504u, "SOUND.C:556/1450 F0504_SOUND_ReadSecondSector_CPSD", "dm1_v1_sound_pc34_compat", 1, 1, 1 },
    { 505u, "SOUND.C:1871 F0505_SOUND_GetVolume", "dm1_v1_sound_pc34_compat", 1, 1, 1 },
    { 506u, "STARTUP1.C:409 F0506_AMIGA_AllocateData", "dm1_v1_amiga_platform_boundary_pc34_compat", 1, 1, 1 },
    { 507u, "STARTUP1.C:417 F0507_AMIGA_DeinitializeAndInfiniteLoop", "dm1_v1_amiga_platform_boundary_pc34_compat", 1, 1, 1 },
    { 508u, "STARTUP1.C:436 F0508_AMIGA_BuildPaletteChangeCopperList", "dm1_v1_palette_font_pc34_compat", 1, 1, 1 },
    { 509u, "STARTUP1.C:467 F0509_AMIGA_FillScreen", "dm1_v1_viewport_3d_pc34_compat", 1, 1, 1 },
    { 510u, "STARTUP1.C:475 F0510_AMIGA_WaitBottomOfViewPort", "dm1_v1_viewport_3d_pc34_compat", 1, 1, 1 },
    { 511u, "STARTUP1.C:481 F0511_AMIGA_EmptyFunction", "dm1_v1_amiga_platform_boundary_pc34_compat", 1, 1, 1 },
    { 512u, "STARTUP1.C:489 F0512_STARTUP1_IsAltKeyPressed", "dm1_v1_amiga_platform_boundary_pc34_compat", 1, 1, 1 },
    { 513u, "DIALOG.C:1086 F0513_DIALOG_DrawGameReadyToPlay_Unreferenced", "dm1_v1_amiga_platform_boundary_pc34_compat", 1, 1, 1 },
    { 514u, "MOVESENS.C:911 F0514_MOVE_GetSound", "dm1_v1_creature_sound_pc34_compat", 1, 1, 1 },
    { 515u, "PORTRAIT.C:5 F0515_CHAMPION_ConvertPortraitsToAtariSTPlanar", "dm1_v1_save_load_system_pc34_compat", 1, 1, 1 },
    { 516u, "PORTRAIT.C:67 F0516_CHAMPION_ConvertPortraitsFromAtariSTPlanar", "dm1_v1_save_load_system_pc34_compat", 1, 1, 1 },
    { 517u, "MENUDRAW.C:22 F0517_MENUS_AllocateSpellAreaBitmaps", "dm1_v1_menu_render_pc34_compat", 1, 1, 1 },
    { 518u, "FLOPPYAM.C:110 F0518_FLOPPY_Deinitialize_CPSX", "dm1_v1_amiga_platform_boundary_pc34_compat", 1, 1, 1 },
    { 519u, "FLOPPYAM.C:169/594 F0519_FLOPPY_GetFormatResult", "dm1_v1_amiga_platform_boundary_pc34_compat", 1, 1, 1 },
    { 520u, "FLOPPYAM.C:299/579 F0520_FLOPPY_GetSectorChecksum", "dm1_v1_amiga_platform_boundary_pc34_compat", 1, 1, 1 }
};

const DM1_V1_F0501F0520SourceAuditPc34 *
dm1_v1_f0501_f0520_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_F0501F0520SourceAuditPc34 *
dm1_v1_f0501_f0520_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_f0501_f0520_source_audit_evidence_pc34(void)
{
    return "ReDMCSB SOUND.C, STARTUP1.C, PORTRAIT.C, MENUDRAW.C, MOVESENS.C, "
           "and FLOPPYAM.C are the authority for F0501-F0520. Existing owners "
           "require raw original material and fail closed for unavailable Amiga or "
           "floppy paths. The audit does not render or synthesize UI.";
}
