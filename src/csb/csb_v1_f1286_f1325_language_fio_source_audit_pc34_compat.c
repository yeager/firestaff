#include "csb_v1_f1286_f1325_language_fio_source_audit_pc34_compat.h"

#define NONE(number) { number##u, "no numbered F" #number " body in ReDMCSB callable inventory", "fail_closed: no source owner", 0, 1, 1, 1 }
#define BLOCK(number, anchor, reason) { number##u, anchor, reason, 1, 1, 1, 1 }

static const CSB_V1_F1286F1325SourceAuditPc34 k_audit[] = {
    NONE(1286), NONE(1287),
    BLOCK(1288, "SWITCH.C:520 F1288_GetChosenOption", "fail_closed: no authenticated CSB PC34 option-selection owner"),
    BLOCK(1289, "APPBLANG.C:16 F1289_GetFullFileName", "fail_closed: no authenticated CSB PC34 language-file owner"),
    BLOCK(1290, "APPBLANG.C F1290_GetVolumeState", "fail_closed: no authenticated CSB PC34 language-volume owner"),
    BLOCK(1291, "APPBLANG.C F1291_GetLanguageIndexFromLanguageFileName", "fail_closed: no authenticated CSB PC34 language-file owner"),
    BLOCK(1292, "APPBLANG.C F1292_DeleteLanguageFilesFromGameDisk", "fail_closed: no CSB PC34 destructive language-file route"),
    BLOCK(1293, "APPBLANG.C F1293_CreateLanguageFile", "fail_closed: no CSB PC34 language-file creation route"),
    BLOCK(1294, "APPBLANG.C F1294_GetKeyboardMatrix", "fail_closed: no authenticated CSB PC34 keyboard-matrix owner"),
    BLOCK(1295, "APPBLANG.C F1295_IsAltKeyPressed", "fail_closed: no authenticated CSB PC34 modifier-key owner"),
    NONE(1296), NONE(1297),
    BLOCK(1298, "APPBLANG.C; SWITCH.C F1298_GetAutomaticStartupLanguage", "fail_closed: no authenticated CSB PC34 startup-language receipt"),
    BLOCK(1299, "APPBLANG.C; SWITCH.C F1299_SaveLanguageChoice", "fail_closed: no CSB PC34 language-choice write route"),
    BLOCK(1300, "APPBLANG.C; SWITCH.C F1300_Initialize", "fail_closed: no authenticated CSB PC34 language subsystem owner"),
    BLOCK(1301, "APPBLANG.C; SWITCH.C F1301_Cleanup", "fail_closed: no authenticated CSB PC34 language subsystem owner"),
    BLOCK(1302, "STRING.C; CNFG.C F1302_strncmp", "fail_closed: no separate CSB PC34 callable owner"),
    BLOCK(1303, "CEDT019.C; FIO1STUB.C F1303_OpenFIO1", "fail_closed: no CSB PC34 FIO1 library-open substitute"),
    BLOCK(1304, "CEDT019.C; FIO1STUB.C F1304_CloseFIO1", "fail_closed: no CSB PC34 FIO1 library-close substitute"),
    BLOCK(1305, "FIO1MAIN.C F1305_OpenFTLLibrary", "csb_v1_f1168_f1170_f1171_f1305_f1307_usio_fio1_boundaries_pc34_compat"),
    BLOCK(1306, "FIO1MAIN.C F1306_Unreferenced", "fail_closed: source marks this FIO1 route unreferenced"),
    BLOCK(1307, "FIO1MAIN.C F1307_FIO1_03_Expunge", "csb_v1_f1168_f1170_f1171_f1305_f1307_usio_fio1_boundaries_pc34_compat"),
    NONE(1308), NONE(1309), NONE(1310), NONE(1311), NONE(1312), NONE(1313), NONE(1314), NONE(1315),
    BLOCK(1316, "FIO1MAIN.C; FIO1.C F1316_InitializeLibrary", "fail_closed: no CSB PC34 FIO1 library initializer"),
    BLOCK(1317, "FIO1MAIN.C; FIO1.C F1317_UninitializeLibrary", "fail_closed: no CSB PC34 FIO1 library teardown"),
    BLOCK(1318, "FIO1MAIN.C; FIO1.C F1318_FIO1_29_", "fail_closed: no authenticated CSB PC34 FIO1 owner"),
    BLOCK(1319, "FIO1MAIN.C; FIO1.C F1319_FIO1_28_", "fail_closed: no authenticated CSB PC34 FIO1 owner"),
    BLOCK(1320, "FIO1MAIN.C; FIO1.C F1320_FIO1_27_", "fail_closed: no authenticated CSB PC34 FIO1 owner"),
    BLOCK(1321, "FIO1MAIN.C; FIO1.C F1321_FIO1_07_SubstituteStringInFileName", "redmcsb_fio1_file_handle_pc34_compat caller-owned boundary preserved separately"),
    BLOCK(1322, "FIO1MAIN.C; FIO1.C F1322_FIO1_26_", "fail_closed: no authenticated CSB PC34 FIO1 owner"),
    BLOCK(1323, "FIO1MAIN.C; FIO1.C F1323_FIO1_25_", "redmcsb_fio1_file_handle_pc34_compat caller-owned boundary preserved separately"),
    BLOCK(1324, "FIO1MAIN.C; FIO1.C F1324_FIO1_24_", "fail_closed: no authenticated CSB PC34 FIO1 owner"),
    BLOCK(1325, "FIO1MAIN.C; FIO1.C F1325_FIO1_04_IsDiskWriteable", "fail_closed: no CSB PC34 drive-writeability substitute")
};

#undef BLOCK
#undef NONE

const CSB_V1_F1286F1325SourceAuditPc34 *
csb_v1_f1286_f1325_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const CSB_V1_F1286F1325SourceAuditPc34 *
csb_v1_f1286_f1325_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;

    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *csb_v1_f1286_f1325_source_audit_evidence_pc34(void)
{
    return "ReDMCSB SWITCH.C, APPBLANG.C, STRING.C, CEDT019.C, FIO1STUB.C, "
           "FIO1MAIN.C, and FIO1.C are the authority for F1286-F1325. Existing "
           "CSB F1305/F1307 and shared caller-owned F1321/F1323 owners remain "
           "separate. Language, library, disk, and unowned routes fail closed "
           "without authenticated PC34 material; this audit does not render or "
           "synthesize UI, timing, file, or input behavior.";
}
