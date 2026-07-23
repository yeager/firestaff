#include "dm1_v1_f1906_f1945_hint_source_audit_pc34_compat.h"

static const DM1_V1_F1906F1945SourceAuditPc34 k_audit[] = {
    { 1906u, "HINT004.C:219 F1906_", "fail_closed: optional HINT utility command", 1, 1, 1, 1 },
    { 1907u, "no numbered F1907 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1908u, "HINTLOAD.C:103 F1908_Checksum_CPSX", "fail_closed: HINT copy-protection checksum", 1, 1, 1, 1 },
    { 1909u, "HINTLOAD.C:119 F1909_CopyStringUntilCharacter", "dm1_v1_hint_string_helpers_pc34_compat", 1, 1, 1, 1 },
    { 1910u, "HINTLOAD.C:135 F1910_LoadSavedGamePart", "redmcsb_f1918_hintload_pc34_compat (CSB owner)", 1, 1, 1, 1 },
    { 1911u, "no numbered F1911 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1912u, "no numbered F1912 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1913u, "HINTLOAD.C:184 F1913_LoadAndDeobfuscateSavedGamePart", "redmcsb_f1918_hintload_pc34_compat (CSB owner)", 1, 1, 1, 1 },
    { 1914u, "HINTLOAD.C:189 F1914_LoadAndDeobfuscateSavedGameHeader", "redmcsb_f1918_hintload_pc34_compat (CSB owner)", 1, 1, 1, 1 },
    { 1915u, "HINTLOAD.C:221 F1915_UnloadGame", "fail_closed: optional HINT save teardown", 1, 1, 1, 1 },
    { 1916u, "HINTLOAD.C:281 F1916_Pre_F1918_LoadGame_CPSX", "fail_closed: optional HINT load admission", 1, 1, 1, 1 },
    { 1917u, "no numbered F1917 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1918u, "HINTLOAD.C:289 F1918_LoadGame_CPSX", "csb_v1_f1918_hintload_initial_load_pc34_compat", 1, 1, 1, 1 },
    { 1919u, "HINTLOAD.C:469 F1919_Post_F1918_LoadGame_CPSX", "csb_v1_f1918_hintload_initial_load_pc34_compat", 1, 1, 1, 1 },
    { 1920u, "HINTHTC.C:39 F1920_AllocateMemory", "fail_closed: optional HINT HTC allocation", 1, 1, 1, 1 },
    { 1921u, "HINTHTC.C:56 F1921_FreeMemory", "fail_closed: optional HINT HTC allocation", 1, 1, 1, 1 },
    { 1922u, "HINTHTC.C:68 F1922_ReadFromHTCFile", "fail_closed: optional HINT HTC file route", 1, 1, 1, 1 },
    { 1923u, "HINTHTC.C:79 F1923_Pre_F1925_CPSX", "fail_closed: optional HINT HTC admission", 1, 1, 1, 1 },
    { 1924u, "no numbered F1924 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1925u, "HINTHTC.C:86 F1925_CPSX", "fail_closed: optional HINT HTC dispatcher", 1, 1, 1, 1 },
    { 1926u, "HINTHTC.C:480 F1926_Post_F1925_CPSX", "fail_closed: optional HINT HTC continuation", 1, 1, 1, 1 },
    { 1927u, "no numbered F1927 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1928u, "HINTFLOP.C:32 F1928_ForceMediaChangeDetection", "fail_closed: original floppy media change route", 1, 1, 1, 1 },
    { 1929u, "no numbered F1929 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1930u, "no numbered F1930 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1931u, "HINTFLOP.C:131 F1931_", "fail_closed: optional HINT floppy dispatcher", 1, 1, 1, 1 },
    { 1932u, "HINTERR.C:5 F1932_", "fail_closed: optional HINT disk error UI", 1, 1, 1, 1 },
    { 1933u, "HINTERR.C:9 F1933_", "fail_closed: optional HINT error callback", 1, 1, 1, 1 },
    { 1934u, "HINTHINT.C:34 F1934_Checksum_CPSX", "fail_closed: HINT copy-protection checksum", 1, 1, 1, 1 },
    { 1935u, "HINTHINT.C:46 F1935_Pre_F1937_Hints_CPSX", "fail_closed: optional HINT selection admission", 1, 1, 1, 1 },
    { 1936u, "no numbered F1936 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1937u, "HINTHINT.C:54 F1937_Hints_CPSX", "fail_closed: optional HINT selection dispatcher", 1, 1, 1, 1 },
    { 1938u, "HINTHINT.C:283 F1938_Post_F1937_Hints_CPSX", "fail_closed: optional HINT selection continuation", 1, 1, 1, 1 },
    { 1939u, "no numbered F1939 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1940u, "HINTHINT.C:291 F1940_CPSX", "fail_closed: optional HINT text dispatcher", 1, 1, 1, 1 },
    { 1941u, "HINTHINT.C:429 F1941_", "fail_closed: optional HINT command path", 1, 1, 1, 1 },
    { 1942u, "HINTMAIN.C:16 F1942_Pre_F1944_ChecksumAdd_main", "fail_closed: HINT copy-protection preamble", 1, 1, 1, 1 },
    { 1943u, "no numbered F1943 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1944u, "HINTMAIN.C:22 F1944_ChecksumAdd", "fail_closed: HINT copy-protection checksum", 1, 1, 1, 1 },
    { 1945u, "HINTMAIN.C:247 F1945_Pre_F1944_ChecksumAdd_main", "fail_closed: HINT copy-protection preamble", 1, 1, 1, 1 }
};

const DM1_V1_F1906F1945SourceAuditPc34 *
dm1_v1_f1906_f1945_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_F1906F1945SourceAuditPc34 *
dm1_v1_f1906_f1945_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_f1906_f1945_source_audit_evidence_pc34(void)
{
    return "ReDMCSB HINT004.C, HINTLOAD.C, HINTHTC.C, HINTFLOP.C, HINTERR.C, "
           "HINTHINT.C, and HINTMAIN.C are the authority for F1906-F1945. "
           "Existing F1909 and CSB HINTLOAD owners are retained. Remaining HINT, "
           "HTC, floppy, copy-protection, and disk-error routes remain fail closed "
           "without authentic raw PC34 material. The audit does not render or "
           "synthesize UI or timing paths.";
}
