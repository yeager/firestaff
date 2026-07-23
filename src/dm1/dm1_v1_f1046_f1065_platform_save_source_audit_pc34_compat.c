#include "dm1_v1_f1046_f1065_platform_save_source_audit_pc34_compat.h"

static const DM1_V1_F1046F1065SourceAuditPc34 k_audit[] = {
    { 1046u, "MAINLIB.C:19 F1046_OpenLibraries", "redmcsb_f1046_open_libraries_pc34_compat", 1, 1, 1, 1 },
    { 1047u, "MAINLIB.C:49 F1047_CloseLibraries", "redmcsb_f1047_close_libraries_pc34_compat", 1, 1, 1, 1 },
    { 1048u, "DEFS.H:3209 commented F1048_setjmp alias", "fail_closed: no callable source body", 0, 1, 1, 1 },
    { 1049u, "DEFS.H:3210 commented F1049_longjmp alias", "fail_closed: no callable source body", 0, 1, 1, 1 },
    { 1050u, "AMIGINIT.C:480 F1050_AlertCSBSystemError", "redmcsb_f1050_alert_csb_system_error_pc34_compat", 1, 1, 1, 1 },
    { 1051u, "SOUND.C:272 F1051_ Amiga sound helper", "fail_closed: Amiga sound route", 1, 1, 1, 1 },
    { 1052u, "FILLBOX.C:17 F1052_WaitForScanLine", "redmcsb_f1052_wait_for_scan_line_pc34_compat", 1, 1, 1, 1 },
    { 1053u, "COMMAND.C:2029 F1053_Pre_F0380_COMMAND_ProcessQueue_CPSC", "redmcsb_f1053_pre_f0380_command_process_queue_pc34_compat", 1, 1, 1, 1 },
    { 1054u, "no numbered F1054 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1055u, "COMMAND.C:2481 F1055_Post_F0380_COMMAND_ProcessQueue_CPSC", "redmcsb_f1055_post_f0380_command_process_queue_pc34_compat", 1, 1, 1, 1 },
    { 1056u, "no numbered F1056 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1057u, "LOADSAVE.C:533 F1057_Pre_F0433_STARTEND_ProcessCommand140_SaveGame_CPSCDF", "redmcsb_f1057_pre_f0433_startend_process_command140_save_game_pc34_compat", 1, 1, 1, 1 },
    { 1058u, "no numbered F1058 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1059u, "LOADSAVE.C:1784 F1059_Post_F0433_STARTEND_ProcessCommand140_SaveGame_CPSCDF", "redmcsb_f1059_post_f0433_startend_process_command140_save_game_pc34_compat", 1, 1, 1, 1 },
    { 1060u, "no numbered F1060 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1061u, "READWRIT.C:75 F1061_Pre_Unreferenced", "redmcsb_f1061_pre_unreferenced_pc34_compat", 1, 1, 1, 1 },
    { 1062u, "no numbered F1062 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1063u, "READWRIT.C:90 F1063_ChecksumEor_CPSX", "redmcsb_f1063_checksum_eor_cpsx_pc34_compat", 1, 1, 1, 1 },
    { 1064u, "no numbered F1064 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1065u, "AMIGALIB.C:838 F1065_SetExecBase", "redmcsb_f1065_set_exec_base_pc34_compat", 1, 1, 1, 1 }
};

const DM1_V1_F1046F1065SourceAuditPc34 *
dm1_v1_f1046_f1065_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_F1046F1065SourceAuditPc34 *
dm1_v1_f1046_f1065_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_f1046_f1065_source_audit_evidence_pc34(void)
{
    return "ReDMCSB MAINLIB.C, AMIGINIT.C, SOUND.C, FILLBOX.C, COMMAND.C, "
           "LOADSAVE.C, READWRIT.C, and AMIGALIB.C are the authority for "
           "F1046-F1065. The source bodies require Amiga libraries, scan-line "
           "access, code injection, or Exec state; no authentic PC34 route is "
           "proved. All boundaries remain fail closed. The audit does not render "
           "or synthesize UI or timing paths.";
}
