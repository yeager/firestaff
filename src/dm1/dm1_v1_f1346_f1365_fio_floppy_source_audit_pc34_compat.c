#include "dm1_v1_f1346_f1365_fio_floppy_source_audit_pc34_compat.h"

static const DM1_V1_F1346F1365SourceAuditPc34 k_audit[] = {
    { 1346u, "FIO1.C:844 F1346_", "fail_closed: raw FIO1_FILE signature route", 1, 1, 1, 1 },
    { 1347u, "FIO1.C:861 F1347_", "fail_closed: Amiga device-name and floppy route", 1, 1, 1, 1 },
    { 1348u, "FIO1.C:949 F1348_IsFloppyDiskInDrive", "fail_closed: Amiga floppy/device probe", 1, 1, 1, 1 },
    { 1349u, "FIO1.C:989 F1349_IsDiskWriteProtected", "fail_closed: raw FIO1/trackdisk write-protection route", 1, 1, 1, 1 },
    { 1350u, "no numbered F1350 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1351u, "no numbered F1351 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1352u, "no numbered F1352 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1353u, "no numbered F1353 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1354u, "no numbered F1354 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1355u, "no numbered F1355 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1356u, "no numbered F1356 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1357u, "no numbered F1357 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1358u, "no numbered F1358 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1359u, "no numbered F1359 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1360u, "no numbered F1360 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1361u, "no numbered F1361 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1362u, "FLOPPYAM.C:781 F1362_IsFloppyDriveEmpty", "fail_closed: Amiga trackdisk TD_CHANGESTATE", 1, 1, 1, 1 },
    { 1363u, "FLOPPYAM.C:796 F1363_IsFloppyDiskWriteProtected", "fail_closed: Amiga trackdisk TD_PROTSTATUS", 1, 1, 1, 1 },
    { 1364u, "no numbered F1364 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1365u, "no numbered F1365 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 }
};

const DM1_V1_F1346F1365SourceAuditPc34 *
dm1_v1_f1346_f1365_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_F1346F1365SourceAuditPc34 *
dm1_v1_f1346_f1365_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_f1346_f1365_source_audit_evidence_pc34(void)
{
    return "ReDMCSB FIO1.C and FLOPPYAM.C are the authority for F1346-F1365. "
           "F1350-F1361 and F1364-F1365 have no numbered source bodies in the "
           "audited corpus. FIO1, device-name, and trackdisk routes remain fail "
           "closed without authentic raw PC34 material. The audit does not render "
           "or synthesize UI or timing paths.";
}
