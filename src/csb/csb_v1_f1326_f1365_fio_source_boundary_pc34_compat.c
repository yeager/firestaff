#include "csb_v1_f1326_f1365_fio_source_boundary_pc34_compat.h"

static const CSB_V1_F1326F1365FioSourceBoundaryReceiptPc34 k_receipts[] = {
    { 1326u, "FIO1.C:285 FIO1_05_FormatSaveDisk", "fail_closed: floppy format is non-PC34", 1, 1, 1 },
    { 1327u, "FIO1.C:50 FIO1_23", "fail_closed: library vector is non-PC34", 1, 1, 1 },
    { 1328u, "FIO1.C:362 FIO1_22", "shared F1328 callback owner; no CSB content route", 1, 1, 1 },
    { 1329u, "FIO1.C:384 FIO1_21_Open", "shared F1329 callback owner; no CSB content route", 1, 1, 1 },
    { 1330u, "FIO1.C:417 FIO1_20_Create", "Nexus-only owner; no CSB content route", 1, 1, 1 },
    { 1331u, "FIO1.C:453 FIO1_19_Close", "shared F1331 callback owner; no CSB content route", 1, 1, 1 },
    { 1332u, "FIO1.C:479 FIO1_18_GetSize", "shared F1332 callback owner; no CSB content route", 1, 1, 1 },
    { 1333u, "FIO1.C:506 FIO1_17_Tell", "shared F1333 callback owner; no CSB content route", 1, 1, 1 },
    { 1334u, "FIO1.C:531 FIO1_16_Seek", "shared F1334 callback owner; no CSB content route", 1, 1, 1 },
    { 1335u, "FIO1.C:560 FIO1_15_Read", "shared F1335 callback owner; no CSB content route", 1, 1, 1 },
    { 1336u, "FIO1.C:592 FIO1_14_Write", "shared F1336 callback owner; no CSB content route", 1, 1, 1 },
    { 1337u, "FIO1.C:634 FIO1_13_Lock", "fail_closed: lock primitive is non-PC34", 1, 1, 1 },
    { 1338u, "FIO1.C:668 FIO1_12_Delete", "shared F1338 callback owner; no CSB content route", 1, 1, 1 },
    { 1339u, "FIO1.C:697 FIO1_11_Rename", "shared F1339 callback owner; no CSB content route", 1, 1, 1 },
    { 1340u, "FIO1.C:728 FIO1_09", "fail_closed: no portable PC34 owner", 1, 1, 1 },
    { 1341u, "FIO1.C:745 FIO1_10", "shared F1341 callback owner; no CSB content route", 1, 1, 1 },
    { 1342u, "FIO1.C:762 FIO1_08", "shared F1342 callback owner; no CSB content route", 1, 1, 1 },
    { 1343u, "FIO1.C:791 FIO1_06", "fail_closed: no portable PC34 owner", 1, 1, 1 },
    { 1344u, "FIO1.C:26 UninitializeData", "fail_closed: library-global storage", 1, 1, 1 },
    { 1345u, "FIO1.C:27 InitializeData", "fail_closed: library-global storage", 1, 1, 1 },
    { 1346u, "no numbered callable body in ReDMCSB corpus", "fail_closed: absent source", 1, 1, 1 },
    { 1347u, "no numbered callable body in ReDMCSB corpus", "fail_closed: absent source", 1, 1, 1 },
    { 1348u, "FIO1.C:329 IsFloppyDiskInDrive", "fail_closed: floppy hardware", 1, 1, 1 },
    { 1349u, "FIO1.C:278 IsDiskWriteProtected", "fail_closed: floppy hardware", 1, 1, 1 },
    { 1350u, "no numbered callable body in ReDMCSB corpus", "fail_closed: absent source", 1, 1, 1 },
    { 1351u, "no numbered callable body in ReDMCSB corpus", "fail_closed: absent source", 1, 1, 1 },
    { 1352u, "no numbered callable body in ReDMCSB corpus", "fail_closed: absent source", 1, 1, 1 },
    { 1353u, "no numbered callable body in ReDMCSB corpus", "fail_closed: absent source", 1, 1, 1 },
    { 1354u, "no numbered callable body in ReDMCSB corpus", "fail_closed: absent source", 1, 1, 1 },
    { 1355u, "no numbered callable body in ReDMCSB corpus", "fail_closed: absent source", 1, 1, 1 },
    { 1356u, "no numbered callable body in ReDMCSB corpus", "fail_closed: absent source", 1, 1, 1 },
    { 1357u, "no numbered callable body in ReDMCSB corpus", "fail_closed: absent source", 1, 1, 1 },
    { 1358u, "no numbered callable body in ReDMCSB corpus", "fail_closed: absent source", 1, 1, 1 },
    { 1359u, "no numbered callable body in ReDMCSB corpus", "fail_closed: absent source", 1, 1, 1 },
    { 1360u, "no numbered callable body in ReDMCSB corpus", "fail_closed: absent source", 1, 1, 1 },
    { 1361u, "no numbered callable body in ReDMCSB corpus", "fail_closed: absent source", 1, 1, 1 },
    { 1362u, "FIO1.C:21 IsFloppyDriveEmpty", "fail_closed: floppy hardware", 1, 1, 1 },
    { 1363u, "FIO1.C:24 IsFloppyDiskWriteProtected", "fail_closed: floppy hardware", 1, 1, 1 },
    { 1364u, "no numbered callable body in ReDMCSB corpus", "fail_closed: absent source", 1, 1, 1 },
    { 1365u, "no numbered callable body in ReDMCSB corpus", "fail_closed: absent source", 1, 1, 1 }
};

const CSB_V1_F1326F1365FioSourceBoundaryReceiptPc34 *
csb_v1_f1326_f1365_fio_source_boundary_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_receipts) / sizeof(k_receipts[0]);
    return k_receipts;
}

const CSB_V1_F1326F1365FioSourceBoundaryReceiptPc34 *
csb_v1_f1326_f1365_fio_source_boundary_find_pc34(unsigned int number)
{
    size_t index;

    for (index = 0u; index < sizeof(k_receipts) / sizeof(k_receipts[0]); ++index) {
        if (k_receipts[index].function_number == number) return &k_receipts[index];
    }
    return 0;
}

const char *csb_v1_f1326_f1365_fio_source_boundary_evidence_pc34(void)
{
    return "ReDMCSB FIO1.C is the authority for F1326-F1365. Existing shared "
           "FIO callback owners require real host-backed PC34 material but do "
           "not prove a CSB game-content route. Every CSB route fails closed; "
           "this receipt does not perform file I/O or synthesize UI, graphics, or timing.";
}
