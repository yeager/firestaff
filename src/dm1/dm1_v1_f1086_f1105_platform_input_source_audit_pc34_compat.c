#include "dm1_v1_f1086_f1105_platform_input_source_audit_pc34_compat.h"

static const DM1_V1_F1086F1105SourceAuditPc34 k_audit[] = {
    { 1086u, "AMIGINIT.C:283 F1086_ReplaceIntuitionVectors", "redmcsb_f1086_replace_intuition_vectors_pc34_compat", 1, 1, 1, 1 },
    { 1087u, "AMIGINIT.C:293 F1087_RestoreIntuitionVectors", "redmcsb_f1087_restore_intuition_vectors_pc34_compat", 1, 1, 1, 1 },
    { 1088u, "AMIGINIT.C:333 F1088_OpenAmigaStuff", "redmcsb_f1088_open_amiga_stuff_pc34_compat", 1, 1, 1, 1 },
    { 1089u, "AMIGINIT.C:363 F1089_CloseAmigaStuff", "redmcsb_f1089_close_amiga_stuff_pc34_compat", 1, 1, 1, 1 },
    { 1090u, "AMIGINIT.C:392 F1090_GetCSBInternalErrorMessage", "redmcsb_f1090_get_csb_internal_error_message_pc34_compat", 1, 1, 1, 1 },
    { 1091u, "AMIGINIT.C:411 F1091_GetCSBSystemErrorMessage", "redmcsb_f1091_get_csb_system_error_message_pc34_compat", 1, 1, 1, 1 },
    { 1092u, "AMIGINIT.C:430 F1092_GetHexadecimalDigits", "fail_closed: CSB-only alert template", 1, 1, 1, 1 },
    { 1093u, "AMIGINIT.C:442 F1093_DisplayAlertCSBInternalError", "fail_closed: Amiga DisplayAlert and reset", 1, 1, 1, 1 },
    { 1094u, "AMIGINIT.C:459 F1094_DisplayAlertCSBSystemError", "fail_closed: Amiga DisplayAlert and reset", 1, 1, 1, 1 },
    { 1095u, "no numbered F1095 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1096u, "no numbered F1096 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1097u, "INPUT.C:822 F1097_StoreKeyInBuffer", "dm1_v1_input_poll_pc34_compat ring insert", 1, 1, 1, 1 },
    { 1098u, "INPUT.C:836 F1098_GetFirstKeyFromBuffer", "dm1_v1_input_poll_pc34_compat ring extract", 1, 1, 1, 1 },
    { 1099u, "INPUT.C:849 F1099_IsKeyBufferNotEmpty", "dm1_v1_input_poll_pc34_compat ring availability", 1, 1, 1, 1 },
    { 1100u, "INPUT.C:861 F1100_Unreferenced", "fail_closed: source unreferenced input helper", 1, 1, 1, 1 },
    { 1101u, "INPUT.C:866 F1101_DetectDiskChange", "fail_closed: Amiga floppy disk-change route", 1, 1, 1, 1 },
    { 1102u, "INPUT.C:880 F1102_Unreferenced", "fail_closed: source unreferenced input helper", 1, 1, 1, 1 },
    { 1103u, "no numbered F1103 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1104u, "no numbered F1104 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1105u, "no numbered F1105 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 }
};

const DM1_V1_F1086F1105SourceAuditPc34 *
dm1_v1_f1086_f1105_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_F1086F1105SourceAuditPc34 *
dm1_v1_f1086_f1105_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_f1086_f1105_source_audit_evidence_pc34(void)
{
    return "ReDMCSB AMIGINIT.C and INPUT.C are the authority for F1086-F1105. "
           "F1095-F1096 and F1103-F1105 have no numbered source body in the "
           "audited corpus. Amiga Intuition, DisplayAlert, and disk-change "
           "routes remain fail closed without authentic PC34 material. The audit "
           "does not render or synthesize UI or timing paths.";
}
