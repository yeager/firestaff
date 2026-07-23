#include "dm1_v1_f1286_f1305_language_fio_source_audit_pc34_compat.h"

static const DM1_V1_F1286F1305SourceAuditPc34 k_audit[] = {
    { 1286u, "no numbered F1286 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1287u, "no numbered F1287 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1288u, "SWITCH.C:520 F1288_GetChosenOption", "fail_closed: raw selection graphics and USIO input not authenticated", 1, 1, 1, 1 },
    { 1289u, "APPBLANG.C:16 F1289_GetFullFileName", "fail_closed: Amiga game-volume language path", 1, 1, 1, 1 },
    { 1290u, "APPBLANG.C:30 F1290_GetVolumeState", "fail_closed: Amiga volume and write-protection route", 1, 1, 1, 1 },
    { 1291u, "APPBLANG.C:65 F1291_GetLanguageIndexFromLanguageFileName", "fail_closed: original language-file probe", 1, 1, 1, 1 },
    { 1292u, "APPBLANG.C:84 F1292_DeleteLanguageFilesFromGameDisk", "fail_closed: original game-volume mutation", 1, 1, 1, 1 },
    { 1293u, "APPBLANG.C:97 F1293_CreateLanguageFile", "fail_closed: original game-volume mutation", 1, 1, 1, 1 },
    { 1294u, "APPBLANG.C:113 F1294_GetKeyboardMatrix", "fail_closed: Amiga keyboard.device matrix", 1, 1, 1, 1 },
    { 1295u, "APPBLANG.C:123 F1295_IsAltKeyPressed", "fail_closed: Amiga keyboard-matrix route", 1, 1, 1, 1 },
    { 1296u, "APPBLANG.C:130 F1296_", "fail_closed: Amiga keyboard.device setup", 1, 1, 1, 1 },
    { 1297u, "APPBLANG.C:145 F1297_", "fail_closed: Amiga keyboard.device teardown", 1, 1, 1, 1 },
    { 1298u, "APPBLANG.C:162 F1298_GetAutomaticStartupLanguage", "fail_closed: original volume language detection", 1, 1, 1, 1 },
    { 1299u, "APPBLANG.C:175 F1299_SaveLanguageChoice", "fail_closed: original language-file write", 1, 1, 1, 1 },
    { 1300u, "APPBLANG.C:185 F1300_Initialize", "fail_closed: Amiga language selector initialization", 1, 1, 1, 1 },
    { 1301u, "APPBLANG.C:192 F1301_Cleanup", "fail_closed: Amiga language selector cleanup", 1, 1, 1, 1 },
    { 1302u, "STRING.C:307 F1302_strncmp", "fail_closed: CNFG-specific string comparator route", 1, 1, 1, 1 },
    { 1303u, "FIO1STUB.C:270 F1303_OpenFIO1", "fail_closed: FIO1 library-open route", 1, 1, 1, 1 },
    { 1304u, "FIO1STUB.C:279 F1304_CloseFIO1", "fail_closed: FIO1 library-close route", 1, 1, 1, 1 },
    { 1305u, "FIO1MAIN.C:91 F1305_OpenFTLLibrary", "csb_v1_f1168_f1170_f1171_f1305_f1307_usio_fio1_boundaries_pc34_compat", 1, 1, 1, 1 }
};

const DM1_V1_F1286F1305SourceAuditPc34 *
dm1_v1_f1286_f1305_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_F1286F1305SourceAuditPc34 *
dm1_v1_f1286_f1305_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_f1286_f1305_source_audit_evidence_pc34(void)
{
    return "ReDMCSB SWITCH.C, APPBLANG.C, STRING.C, FIO1STUB.C, and "
           "FIO1MAIN.C are the authority for F1286-F1305. F1286-F1287 have no "
           "numbered source bodies. Language, volume, keyboard-device, and FIO1 "
           "routes remain fail closed without authentic raw PC34 material. The audit "
           "does not render or synthesize UI or timing paths.";
}
