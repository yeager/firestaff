#include "dm1_v1_f1166_f1185_usio_anim_source_audit_pc34_compat.h"

static const DM1_V1_F1166F1185SourceAuditPc34 k_audit[] = {
    { 1166u, "USIO2.C:75 F1166_USIO_16_ExtractFirstUsioDataFromQueue", "csb_v1_f1164_f1165_f1166_f1167_usio_queue_boundaries_pc34_compat", 1, 1, 1, 1 },
    { 1167u, "USIO2.C:89/124 F1167_USIO_14_GetMouseStatus", "csb_v1_f1164_f1165_f1166_f1167_usio_queue_boundaries_pc34_compat", 1, 1, 1, 1 },
    { 1168u, "USIO2.C:116 F1168_USIO_18_Empty", "csb_v1_f1168_f1170_f1171_f1305_f1307_usio_fio1_boundaries_pc34_compat", 1, 1, 1, 1 },
    { 1169u, "USIO2.C:133 F1169_", "fail_closed: Amiga USIO library resident route", 1, 1, 1, 1 },
    { 1170u, "USIO2.C:176 F1170_USIO_03_Expunge", "csb_v1_f1168_f1170_f1171_f1305_f1307_usio_fio1_boundaries_pc34_compat", 1, 1, 1, 1 },
    { 1171u, "USIO2.C:205 F1171_USIO_19_LockDF0", "csb_v1_f1168_f1170_f1171_f1305_f1307_usio_fio1_boundaries_pc34_compat", 1, 1, 1, 1 },
    { 1172u, "USIO2.C:274 F1172_QueueMouseAndKeyboardInput", "dm1_v1_input_command_queue_pc34_compat", 1, 1, 1, 1 },
    { 1173u, "USIO1.C:177; USIO2.C:235/344 F1173_AddUsioDataToInputQueue", "dm1_v1_input_command_queue_pc34_compat", 1, 1, 1, 1 },
    { 1174u, "USIO2.C:264/362 F1174_AddPendingUsioDataToInputQueue", "dm1_v1_input_command_queue_pc34_compat", 1, 1, 1, 1 },
    { 1175u, "USIO2.C:368 F1175_GetFirstQueuedUsioDataIndex", "csb_v1_f1164_f1165_f1166_f1167_usio_queue_boundaries_pc34_compat", 1, 1, 1, 1 },
    { 1176u, "USIO2.C:384 F1176_ExtractFirstUsioDataFromQueue", "csb_v1_f1164_f1165_f1166_f1167_usio_queue_boundaries_pc34_compat", 1, 1, 1, 1 },
    { 1177u, "ANIM.C:529 F1177_", "fail_closed: raw ANIM file descriptor not authenticated", 1, 1, 1, 1 },
    { 1178u, "ANIM.C:570 F1178_", "fail_closed: raw ANIM descriptor release route", 1, 1, 1, 1 },
    { 1179u, "ANIM.C:586 F1179_Process_AN", "fail_closed: raw ANIM step stream not authenticated", 1, 1, 1, 1 },
    { 1180u, "ANIM.C:593 F1180_Process_BR", "fail_closed: raw ANIM step stream not authenticated", 1, 1, 1, 1 },
    { 1181u, "ANIM.C:601 F1181_Process_PL", "fail_closed: raw ANIM palette step not authenticated", 1, 1, 1, 1 },
    { 1182u, "ANIM.C:620 F1182_Process_EN", "fail_closed: raw ANIM bitmap step not authenticated", 1, 1, 1, 1 },
    { 1183u, "ANIM.C:628 F1183_Process_DL", "fail_closed: raw ANIM bitmap step not authenticated", 1, 1, 1, 1 },
    { 1184u, "ANIM.C:636 F1184_Process_SD", "fail_closed: raw ANIM sound descriptor not authenticated", 1, 1, 1, 1 },
    { 1185u, "ANIM.C:644 F1185_Process_MD", "fail_closed: raw ANIM music descriptor not authenticated", 1, 1, 1, 1 }
};

const DM1_V1_F1166F1185SourceAuditPc34 *
dm1_v1_f1166_f1185_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_F1166F1185SourceAuditPc34 *
dm1_v1_f1166_f1185_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_f1166_f1185_source_audit_evidence_pc34(void)
{
    return "ReDMCSB USIO1.C, USIO2.C, and ANIM.C are the authority for "
           "F1166-F1185. Existing PC34 queue owners consume only caller-owned "
           "input. The Amiga resident, floppy, and all ANIM routes remain fail "
           "closed without authentic raw PC34 material. The audit does not render "
           "or synthesize UI or timing paths.";
}
