#include "dm1_v1_f1186_f1205_anim_step_source_audit_pc34_compat.h"

static const DM1_V1_F1186F1205SourceAuditPc34 k_audit[] = {
    { 1186u, "ANIM.C:652 F1186_Process_SO", "fail_closed: raw ANIM sound step not authenticated", 1, 1, 1, 1 },
    { 1187u, "ANIM.C:664 F1187_Process_MI", "fail_closed: raw ANIM music step not authenticated", 1, 1, 1, 1 },
    { 1188u, "ANIM.C:672 F1188_Process_SF", "fail_closed: raw ANIM sound-file step not authenticated", 1, 1, 1, 1 },
    { 1189u, "ANIM.C:705 F1189_Process_MF", "fail_closed: raw ANIM music-file step not authenticated", 1, 1, 1, 1 },
    { 1190u, "ANIM.C:738 F1190_Process_FO", "fail_closed: raw ANIM fade-out step not authenticated", 1, 1, 1, 1 },
    { 1191u, "ANIM.C:747 F1191_Process_NE", "fail_closed: raw ANIM next-event step not authenticated", 1, 1, 1, 1 },
    { 1192u, "ANIM.C:759 F1192_Process_BN", "fail_closed: raw ANIM bitmap-name step not authenticated", 1, 1, 1, 1 },
    { 1193u, "ANIM.C:766 F1193_Process_DO", "fail_closed: raw ANIM display-option step not authenticated", 1, 1, 1, 1 },
    { 1194u, "ANIM.C:775 F1194_Process_WA", "fail_closed: raw ANIM wait step not authenticated", 1, 1, 1, 1 },
    { 1195u, "ANIM.C:783 F1195_Process_TR", "fail_closed: raw ANIM transition step not authenticated", 1, 1, 1, 1 },
    { 1196u, "ANIM.C:792 F1196_ dispatches ANIM step tags", "fail_closed: raw ANIM dispatch stream not authenticated", 1, 1, 1, 1 },
    { 1197u, "no numbered F1197 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1198u, "ANIM.C:936 F1198_", "fail_closed: source longjmp error exit", 1, 1, 1, 1 },
    { 1199u, "ANIM.C:942 F1199_", "fail_closed: raw ANIM playback route not authenticated", 1, 1, 1, 1 },
    { 1200u, "ANIM.C:973 F1200_", "fail_closed: source USIO queue drain route", 1, 1, 1, 1 },
    { 1201u, "ANIM.C:982 F1201_", "fail_closed: raw ANIM breakable-input route not authenticated", 1, 1, 1, 1 },
    { 1202u, "ANIM.C:997 F1202_", "fail_closed: raw ANIM allocation route not authenticated", 1, 1, 1, 1 },
    { 1203u, "ANIM.C:1008 F1203_", "fail_closed: raw ANIM release/music-stop route", 1, 1, 1, 1 },
    { 1204u, "ANIM.C:1019 F1204_", "fail_closed: raw ANIM expand/palette route not authenticated", 1, 1, 1, 1 },
    { 1205u, "ANIM.C:1070 F1205_", "fail_closed: raw ANIM double-buffer route not authenticated", 1, 1, 1, 1 }
};

const DM1_V1_F1186F1205SourceAuditPc34 *
dm1_v1_f1186_f1205_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_F1186F1205SourceAuditPc34 *
dm1_v1_f1186_f1205_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_f1186_f1205_source_audit_evidence_pc34(void)
{
    return "ReDMCSB ANIM.C is the authority for F1186-F1205. F1197 has no "
           "numbered source body in the audited corpus. Sound, music, palette, "
           "bitmap, wait, and buffer routes remain fail closed without authentic "
           "raw PC34 ANIM material. The audit does not render or synthesize UI or "
           "timing paths.";
}
