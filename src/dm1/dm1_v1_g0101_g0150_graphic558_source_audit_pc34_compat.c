#include "dm1_v1_g0101_g0150_graphic558_source_audit_pc34_compat.h"

#define ROW(number, owner) { number##u, "DUNVIEW.C:359-425 Graphic558 global", owner, 1, 1, 1 }

static const DM1_V1_G0101G0150SourceAuditPc34 k_audit[] = {
    ROW(101, "fail_closed: no verified current-map ornament owner"),
    ROW(102, "fail_closed: no verified current-map ornament owner"),
    ROW(103, "fail_closed: no verified current-map ornament owner"),
    {104u, "DEFS.H:5459 G0104", "fail_closed: source code-patch state", 1, 1, 1},
    ROW(105, "dm1_v1_g0105_pc34_compat"), ROW(106, "dm1_v1_g0106_pc34_compat"),
    ROW(107, "dm1_v1_g0107_pc34_compat"), ROW(108, "dm1_v1_g0108_pc34_compat"),
    ROW(109, "dm1_v1_g0109_pc34_compat"), ROW(110, "dm1_v1_g0110_pc34_compat"),
    ROW(111, "dm1_v1_g0111_pc34_compat"), ROW(112, "dm1_v1_g0112_pc34_compat"),
    ROW(113, "dm1_v1_g0113_pc34_compat"), ROW(114, "dm1_v1_g0114_pc34_compat"),
    ROW(115, "dm1_v1_g0115_pc34_compat"), ROW(116, "dm1_v1_g0116_pc34_compat"),
    ROW(117, "dm1_v1_g0117_pc34_compat"), ROW(118, "dm1_v1_g0118_pc34_compat"),
    ROW(119, "dm1_v1_g0119_pc34_compat"), ROW(120, "dm1_v1_g0120_pc34_compat"),
    ROW(121, "dm1_v1_g0121_pc34_compat"), ROW(122, "dm1_v1_g0122_pc34_compat"),
    ROW(123, "dm1_v1_g0123_pc34_compat"), ROW(124, "dm1_v1_g0124_pc34_compat"),
    ROW(125, "dm1_v1_g0125_pc34_compat"), ROW(126, "dm1_v1_g0126_pc34_compat"),
    ROW(127, "dm1_v1_g0127_pc34_compat"), ROW(128, "dm1_v1_g0128_pc34_compat"),
    ROW(129, "dm1_v1_g0129_pc34_compat"), ROW(130, "dm1_v1_g0130_pc34_compat"),
    ROW(131, "dm1_v1_g0131_pc34_compat"), ROW(132, "dm1_v1_g0132_pc34_compat"),
    ROW(133, "dm1_v1_g0133_pc34_compat"), ROW(134, "dm1_v1_g0134_pc34_compat"),
    ROW(135, "dm1_v1_g0135_pc34_compat"), ROW(136, "dm1_v1_g0136_pc34_compat"),
    ROW(137, "dm1_v1_g0137_pc34_compat"), ROW(138, "dm1_v1_g0138_pc34_compat"),
    ROW(139, "dm1_v1_g0139_pc34_compat"), ROW(140, "dm1_v1_g0140_pc34_compat"),
    ROW(141, "dm1_v1_g0141_pc34_compat"), ROW(142, "dm1_v1_g0142_pc34_compat"),
    ROW(143, "dm1_v1_g0143_pc34_compat"), ROW(144, "dm1_v1_g0144_pc34_compat"),
    ROW(145, "dm1_v1_g0145_pc34_compat"), ROW(146, "dm1_v1_g0146_pc34_compat"),
    ROW(147, "dm1_v1_g0147_pc34_compat"), ROW(148, "dm1_v1_g0148_pc34_compat"),
    ROW(149, "dm1_v1_g0149_pc34_compat"), ROW(150, "dm1_v1_g0150_pc34_compat")
};

#undef ROW

const DM1_V1_G0101G0150SourceAuditPc34 *
dm1_v1_g0101_g0150_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_G0101G0150SourceAuditPc34 *
dm1_v1_g0101_g0150_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_g0101_g0150_source_audit_evidence_pc34(void)
{
    return "ReDMCSB DUNVIEW.C:359-425 and DEFS.H:5459 are the authority for "
           "G0101-G0150. Existing source-named Graphic558 owner modules are retained; "
           "current-map ornament and code-patch state remains fail closed without "
           "authentic raw PC34 material. The audit does not render or synthesize behavior.";
}
