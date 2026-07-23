#include "csb_v1_f1266_f1285_swsh_platform_source_audit_pc34_compat.h"

static const CSB_V1_F1266F1285SourceAuditPc34 k_audit[] = {
    { 1266u, "no numbered F1266 body in ReDMCSB callable inventory", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1267u, "no numbered F1267 body in ReDMCSB callable inventory", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1268u, "no numbered F1268 body in ReDMCSB callable inventory", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1269u, "no numbered F1269 body in ReDMCSB callable inventory", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1270u, "no numbered F1270 body in ReDMCSB callable inventory", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1271u, "no numbered F1271 body in ReDMCSB callable inventory", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1272u, "SWSH.C:915 F1272_Initialize_CPSX", "fail_closed: platform CPSX initialization has no authenticated CSB PC34 owner", 1, 1, 1, 1 },
    { 1273u, "SWSH.C:2870 F1273_Cleanup", "fail_closed: platform swoosh cleanup has no authenticated CSB PC34 owner", 1, 1, 1, 1 },
    { 1274u, "SWSH.C:2878 F1274_Unreferenced", "fail_closed: source marks this swoosh route unreferenced", 1, 1, 1, 1 },
    { 1275u, "no numbered F1275 body in ReDMCSB callable inventory", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1276u, "no numbered F1276 body in ReDMCSB callable inventory", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1277u, "no numbered F1277 body in ReDMCSB callable inventory", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1278u, "no numbered F1278 body in ReDMCSB callable inventory", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1279u, "no numbered F1279 body in ReDMCSB callable inventory", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1280u, "no numbered F1280 body in ReDMCSB callable inventory", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1281u, "no numbered F1281 body in ReDMCSB callable inventory", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1282u, "AMIGINIT.C:308 F1282_WriteString", "fail_closed: Amiga platform text output has no CSB PC34 owner", 1, 1, 1, 1 },
    { 1283u, "AMIGINIT.C:312 F1283_WriteCharacter_Unreferenced", "fail_closed: source marks this Amiga route unreferenced", 1, 1, 1, 1 },
    { 1284u, "no numbered F1284 body in ReDMCSB callable inventory", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1285u, "no numbered F1285 body in ReDMCSB callable inventory", "fail_closed: no source owner", 0, 1, 1, 1 }
};

const CSB_V1_F1266F1285SourceAuditPc34 *
csb_v1_f1266_f1285_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const CSB_V1_F1266F1285SourceAuditPc34 *
csb_v1_f1266_f1285_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;

    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *csb_v1_f1266_f1285_source_audit_evidence_pc34(void)
{
    return "ReDMCSB SWSH.C:915,2870,2878 and AMIGINIT.C:308,312 are the "
           "identified source bodies for F1266-F1285. Existing CSB swoosh "
           "owners remain separate; CPSX and Amiga platform paths fail closed "
           "without authenticated PC34 material. The audit does not render or "
           "synthesize UI or timing paths.";
}
