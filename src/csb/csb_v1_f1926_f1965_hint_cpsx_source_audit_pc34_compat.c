#include "csb_v1_f1926_f1965_hint_cpsx_source_audit_pc34_compat.h"

#define NONE(number) { number##u, "no numbered F" #number " body in ReDMCSB callable inventory", "fail_closed: no source owner", 0, 1, 1, 1 }
#define BLOCK(number, anchor, reason) { number##u, anchor, reason, 1, 1, 1, 1 }

static const CSB_V1_F1926F1965SourceAuditPc34 k_audit[] = {
    BLOCK(1926, "HINTHTC.C; HINTSCR.C F1926_Post_F1925_CPSX", "fail_closed: no authenticated CSB PC34 CPSX hint owner"),
    NONE(1927),
    BLOCK(1928, "CEDTINCF.C; UTIO.C; HINTFLOP.C F1928_ForceMediaChangeDetection", "fail_closed: no authenticated CSB PC34 media-change owner"),
    NONE(1929), NONE(1930), NONE(1931), NONE(1932), NONE(1933),
    BLOCK(1934, "HINTMAIN.C; HINTHINT.C; HINTSCR.C F1934_Checksum_CPSX", "fail_closed: no authenticated CSB PC34 hint checksum owner"),
    BLOCK(1935, "HINTHINT.C F1935_Pre_F1937_Hints_CPSX", "fail_closed: no authenticated CSB PC34 hint owner"),
    NONE(1936),
    BLOCK(1937, "HINTMAIN.C; HINTHINT.C; HINTSCR.C F1937_Hints_CPSX", "fail_closed: no authenticated CSB PC34 hint owner"),
    BLOCK(1938, "HINTHINT.C F1938_Post_F1937_Hints_CPSX", "fail_closed: no authenticated CSB PC34 hint owner"),
    NONE(1939),
    BLOCK(1940, "HINTMAIN.C; HINTHINT.C; HINTSCR.C F1940_CPSX", "fail_closed: no authenticated CSB PC34 CPSX owner"),
    NONE(1941),
    BLOCK(1942, "HINTMAIN.C; HINTINPT.C; HINT007.C F1942_Pre_F1944_ChecksumAdd_main", "fail_closed: no authenticated CSB PC34 checksum owner"),
    NONE(1943),
    BLOCK(1944, "HINTLOAD.C; HINTMAIN.C; HINTSCR.C; HINTINPT.C F1944_ChecksumAdd", "fail_closed: no authenticated CSB PC34 checksum owner"),
    BLOCK(1945, "HINTMAIN.C; HINTINPT.C; HINT007.C F1945_Pre_F1944_ChecksumAdd_main", "fail_closed: no authenticated CSB PC34 checksum owner"),
    NONE(1946),
    BLOCK(1947, "HINTLOAD.C; HINT005.C; HINT004.C; HINTSCR.C F1947_BlitBitmapToOrFromScreen", "fail_closed: no authenticated CSB PC34 bitmap owner"),
    NONE(1948), NONE(1949),
    BLOCK(1950, "HINTHTC.C; HINT006.C F1950_Pre_F1952_CheckCopyProtectionSector_CPSX", "fail_closed: no authenticated CSB PC34 copy-protection owner"),
    NONE(1951),
    BLOCK(1952, "HINTHTC.C; HINT006.C; HINTMAIN.C F1952_CheckCopyProtectionSector_CPSX", "fail_closed: no authenticated CSB PC34 copy-protection owner"),
    BLOCK(1953, "HINTHTC.C; HINT006.C; HINT007.C F1953_Post_F1952_CheckCopyProtectionSector_CPSX", "fail_closed: no authenticated CSB PC34 copy-protection owner"),
    NONE(1954),
    BLOCK(1955, "HINTHTC.C; HINTMAIN.C; HINTTEXT.C; HINTHINT.C; HINTFLOP.C; HINTSCR.C; HINTINPT.C F1955_Input_CPSX", "fail_closed: no authenticated CSB PC34 CPSX input owner"),
    BLOCK(1956, "HINT002.C; HINTLOAD.C; HINTEND.C; HINTHTC.C; HINTMAIN.C; HINTTEXT.C; HINTSCR.C F1956_EndProgram", "fail_closed: no authenticated CSB PC34 CPSX end-program owner"),
    NONE(1957), NONE(1958), NONE(1959), NONE(1960), NONE(1961), NONE(1962), NONE(1963), NONE(1964), NONE(1965)
};

#undef BLOCK
#undef NONE

const CSB_V1_F1926F1965SourceAuditPc34 *
csb_v1_f1926_f1965_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const CSB_V1_F1926F1965SourceAuditPc34 *
csb_v1_f1926_f1965_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;

    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *csb_v1_f1926_f1965_source_audit_evidence_pc34(void)
{
    return "ReDMCSB HINTHTC.C, HINTSCR.C, CEDTINCF.C, UTIO.C, HINTFLOP.C, "
           "HINTMAIN.C, HINTHINT.C, HINTINPT.C, HINT007.C, HINTLOAD.C, HINT005.C, "
           "HINT004.C, HINT006.C, HINTTEXT.C, HINT002.C, and HINTEND.C own the "
           "identified F1926-F1965 CPSX routes. Existing CSB F1918 handoff remains "
           "separate. No CSB PC34 owner is present, so all routes fail closed without "
           "authenticated material. This audit does not render or synthesize graphics, "
           "text, UI, timing, input, files, or copy-protection behavior.";
}
