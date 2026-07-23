#include "dm1_v1_f0421_f0440_save_endgame_source_audit_pc34_compat.h"

static const DM1_V1_F0421F0440SourceAuditPc34 k_audit[] = {
    { 421u, "DEFS.H:8482; LOADSAVE.C:1937 F0421 running-checksum call sites", "dm1_v1_original_save_pc34_handoff", 1, 1, 1 },
    { 422u, "DEFS.H:8489; LOADSAVE.C:1661 F0422 running-checksum call sites", "dm1_v1_original_save_pc34_handoff", 1, 1, 1 },
    { 423u, "DEFS.H:8496; LOADSAVE.C:2177 F0423 clone-fix handoff", "dm1_v1_original_save_pc34_handoff", 1, 1, 1 },
    { 424u, "DIALOG.C:293 F0424_DIALOG_GetChoice", "dm1_v1_f0424_f0427_dialog_admission_pc34_compat", 1, 1, 1 },
    { 425u, "DIALOG.C:546 F0425_DIALOG_PrintCenteredChoice", "dm1_v1_f0424_f0427_dialog_admission_pc34_compat", 1, 1, 1 },
    { 426u, "DIALOG.C:561 F0426_DIALOG_IsMessageOnTwoLines", "dm1_v1_f0424_f0427_dialog_admission_pc34_compat", 1, 1, 1 },
    { 427u, "DIALOG.C:592 F0427_DIALOG_Draw", "dm1_v1_f0424_f0427_dialog_admission_pc34_compat", 1, 1, 1 },
    { 428u, "DEFS.H:8533; DIALOG.C/LOADSAVE.C F0428 disk-media call sites", "dm1_v1_dialog_layout_pc34_compat", 1, 1, 1 },
    { 429u, "SAVEHEAD.C:13 F0429_STARTEND_IsReadSaveHeaderSuccessful", "dm1_v1_original_save_pc34_handoff", 1, 1, 1 },
    { 430u, "SAVEHEAD.C:57 F0430_STARTEND_IsWriteObfuscatedSaveHeaderSuccessful", "dm1_v1_original_save_pc34_handoff", 1, 1, 1 },
    { 431u, "DEFS.H:8544 F0431_STARTEND_GetDarkenedColor source-bound palette boundary", "dm1_v1_f0431_f0436_palette_step_pc34_compat", 1, 1, 1 },
    { 432u, "LOADSAVE.C:278 F0432_STARTEND_FormatDiskMenu", "dm1_v1_save_load_system_pc34_compat", 1, 1, 1 },
    { 433u, "LOADSAVE.C:550 F0433_STARTEND_ProcessCommand140_SaveGame_CPSCDF", "dm1_v1_original_save_pc34_handoff", 1, 1, 1 },
    { 434u, "LOADSAVE.C:1803 F0434_STARTEND_IsLoadDungeonSuccessful_CPSC", "dm1_v1_original_save_pc34_handoff", 1, 1, 1 },
    { 435u, "LOADSAVE.C:2192 F0435_STARTEND_LoadGame", "dm1_v1_original_save_pc34_handoff", 1, 1, 1 },
    { 436u, "DEFS.H:8566; TITLE.C/ENTRANCE.C F0436 palette call sites", "dm1_v1_f0431_f0436_palette_step_pc34_compat", 1, 1, 1 },
    { 437u, "TITLE.C:12 F0437_STARTEND_DrawTitle", "dm1_v1_f0437_f0438_f0439_startup_visual_admission_pc34_compat", 1, 1, 1 },
    { 438u, "ENTRANCE.C:93 F0438_STARTEND_OpenEntranceDoors", "dm1_v1_f0437_f0438_f0439_startup_visual_admission_pc34_compat", 1, 1, 1 },
    { 439u, "ENTRANCE.C:371 F0439_STARTEND_DrawEntrance", "dm1_v1_f0437_f0438_f0439_startup_visual_admission_pc34_compat", 1, 1, 1 },
    { 440u, "ENTRANCE.C:600 F0440_STARTEND_GetTemporarilyLoadedGraphicByteCount", "dm1_v1_f0440_f0441_entrance_asset_flow_pc34_compat", 1, 1, 1 }
};

const DM1_V1_F0421F0440SourceAuditPc34 *
dm1_v1_f0421_f0440_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_F0421F0440SourceAuditPc34 *
dm1_v1_f0421_f0440_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_f0421_f0440_source_audit_evidence_pc34(void)
{
    return "ReDMCSB SAVEHEAD.C, LOADSAVE.C, DIALOG.C, TITLE.C, and ENTRANCE.C "
           "are the authority for F0421-F0440. The archive omits some platform "
           "bodies, so existing owners require raw PC34 source material and fail "
           "closed without it. The audit does not save, render, synthesize endgame, "
           "or synthesize UI.";
}
