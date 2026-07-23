#include "dm1_v1_f1486_f1505_switch_source_audit_pc34_compat.h"

static const DM1_V1_F1486F1505SourceAuditPc34 k_audit[] = {
    { 1486u, "no F1486 symbol in audited ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1487u, "no F1487 symbol in audited ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1488u, "no F1488 symbol in audited ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1489u, "no F1489 symbol in audited ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1490u, "no F1490 symbol in audited ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1491u, "no F1491 symbol in audited ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1492u, "no F1492 symbol in audited ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1493u, "no F1493 symbol in audited ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1494u, "no F1494 symbol in audited ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1495u, "no F1495 symbol in audited ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1496u, "no F1496 symbol in audited ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1497u, "no F1497 symbol in audited ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1498u, "no F1498 symbol in audited ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1499u, "no F1499 symbol in audited ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1500u, "no F1500 symbol in audited ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1501u, "SWITCH.C:53 F1501_GetSwitchOptionBitmapByteCount", "fail_closed: Atari ST switch bitmap descriptor", 1, 1, 1, 1 },
    { 1502u, "no numbered F1502 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1503u, "SWITCH.C:78 F1503_ReadFTLHeaders", "fail_closed: Atari ST FTL header/container loader", 1, 1, 1, 1 },
    { 1504u, "SWITCH.C:151 F1504_LoadSegment", "fail_closed: Atari ST FTL segment loader", 1, 1, 1, 1 },
    { 1505u, "no numbered F1505 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 }
};

const DM1_V1_F1486F1505SourceAuditPc34 *
dm1_v1_f1486_f1505_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_F1486F1505SourceAuditPc34 *
dm1_v1_f1486_f1505_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_f1486_f1505_source_audit_evidence_pc34(void)
{
    return "The audited ReDMCSB corpus contains only F1501, F1503, and F1504 "
           "in F1486-F1505; they are Atari ST switch/FTL-container routes. No "
           "authentic PC34 owner or material exists, so every row remains fail "
           "closed. The audit does not render or synthesize UI or timing paths.";
}
