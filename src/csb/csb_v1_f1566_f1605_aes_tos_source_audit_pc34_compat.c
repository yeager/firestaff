#include "csb_v1_f1566_f1605_aes_tos_source_audit_pc34_compat.h"

#define AES(number) { number##u, "UTSTAES.C AES platform vector F" #number, "fail_closed: no authenticated CSB PC34 AES/TOS platform owner", 1, 1, 1, 1 }
#define NONE(number) { number##u, "no numbered F" #number " body in ReDMCSB callable inventory", "fail_closed: no source owner", 0, 1, 1, 1 }

static const CSB_V1_F1566F1605SourceAuditPc34 k_audit[] = {
    AES(1566), AES(1567), AES(1568), AES(1569), AES(1570), AES(1571), AES(1572), AES(1573),
    AES(1574), AES(1575), AES(1576), AES(1577), AES(1578), AES(1579), AES(1580), AES(1581),
    AES(1582), AES(1583), AES(1584), AES(1585), AES(1586), AES(1587), AES(1588), AES(1589),
    AES(1590), AES(1591), AES(1592), AES(1593), AES(1594), AES(1595), AES(1596), AES(1597),
    AES(1598), AES(1599), AES(1600), AES(1601), AES(1602),
    { 1603u, "CEDTINCI.C; HINTGRAP.C; TOS.C F1603_TerminateTOSProgram", "fail_closed: no authenticated CSB PC34 TOS termination owner", 1, 1, 1, 1 },
    NONE(1604), NONE(1605)
};

#undef NONE
#undef AES

const CSB_V1_F1566F1605SourceAuditPc34 *
csb_v1_f1566_f1605_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const CSB_V1_F1566F1605SourceAuditPc34 *
csb_v1_f1566_f1605_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;

    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *csb_v1_f1566_f1605_source_audit_evidence_pc34(void)
{
    return "ReDMCSB UTSTAES.C owns the AES form, graphics, window, resource, "
           "shell, and file-selector vectors F1566-F1602; CEDTINCI.C, HINTGRAP.C, "
           "and TOS.C own F1603. No CSB PC34 owner is present, so every route "
           "fails closed without authenticated PC34 material. This audit does not "
           "render or synthesize UI, graphics, timing, input, files, or termination.";
}
