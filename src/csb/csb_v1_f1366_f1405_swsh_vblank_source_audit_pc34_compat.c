#include "csb_v1_f1366_f1405_swsh_vblank_source_audit_pc34_compat.h"

#define NONE(number) { number##u, "no numbered F" #number " body in ReDMCSB callable inventory", "fail_closed: no source owner", 0, 1, 1, 1 }
#define BLOCK(number, anchor, reason) { number##u, anchor, reason, 1, 1, 1, 1 }

static const CSB_V1_F1366F1405SourceAuditPc34 k_audit[] = {
    NONE(1366), NONE(1367),
    BLOCK(1368, "SWSH.C:2407 F1368_Graphic_FTLLogo", "fail_closed: no authenticated CSB PC34 FTL-logo bitmap owner"),
    NONE(1369), NONE(1370), NONE(1371),
    BLOCK(1372, "ANIM.C:105; VBLANK.C F1372_SetVerticalBlankInterrupt", "fail_closed: no authenticated CSB PC34 VBlank interrupt owner"),
    NONE(1373),
    BLOCK(1374, "FMTOWNS.H:392; SWSH.C F1374_ApplyPaletteAndWaitForDelay", "fail_closed: no authenticated CSB PC34 palette/timing receipt"),
    NONE(1375), NONE(1376),
    BLOCK(1377, "SWSH.C:2717 F1377_PlaySilentSound", "fail_closed: no authenticated CSB PC34 silent-sound owner"),
    NONE(1378), NONE(1379), NONE(1380), NONE(1381), NONE(1382), NONE(1383), NONE(1384), NONE(1385),
    NONE(1386), NONE(1387), NONE(1388), NONE(1389), NONE(1390), NONE(1391), NONE(1392), NONE(1393),
    NONE(1394), NONE(1395), NONE(1396), NONE(1397), NONE(1398), NONE(1399), NONE(1400), NONE(1401),
    NONE(1402), NONE(1403), NONE(1404), NONE(1405)
};

#undef BLOCK
#undef NONE

const CSB_V1_F1366F1405SourceAuditPc34 *
csb_v1_f1366_f1405_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const CSB_V1_F1366F1405SourceAuditPc34 *
csb_v1_f1366_f1405_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;

    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *csb_v1_f1366_f1405_source_audit_evidence_pc34(void)
{
    return "ReDMCSB SWSH.C:2407,2717, ANIM.C:105, VBLANK.C, and FMTOWNS.H:392 "
           "are the only identified source bodies for F1366-F1405. Existing CSB "
           "swoosh and runtime owners remain separate. FTL-logo, VBlank, palette, "
           "and sound routes fail closed without authenticated PC34 material; this "
           "audit does not render or synthesize UI or timing paths.";
}
