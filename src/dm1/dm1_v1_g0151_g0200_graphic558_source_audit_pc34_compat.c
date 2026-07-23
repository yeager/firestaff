#include "dm1_v1_g0151_g0200_graphic558_source_audit_pc34_compat.h"

#define ROW(number, owner) { number##u, "DUNVIEW.C:426-477 Graphic558 global", owner, 1, 1, 1 }

static const DM1_V1_G0151G0200SourceAuditPc34 k_audit[] = {
    ROW(151, "dm1_v1_g0151_pc34_compat"), ROW(152, "dm1_v1_g0152_pc34_compat"),
    ROW(153, "dm1_v1_g0153_pc34_compat"), ROW(154, "dm1_v1_g0154_pc34_compat"),
    ROW(155, "dm1_v1_g0155_pc34_compat"), ROW(156, "dm1_v1_g0156_pc34_compat"),
    ROW(157, "dm1_v1_g0157_pc34_compat"), ROW(158, "dm1_v1_g0158_pc34_compat"),
    ROW(159, "dm1_v1_g0159_pc34_compat"), ROW(160, "dm1_v1_g0160_pc34_compat"),
    ROW(161, "dm1_v1_g0161_pc34_compat"), ROW(162, "dm1_v1_g0162_pc34_compat"),
    ROW(163, "dm1_v1_g0163_pc34_compat"), ROW(164, "dm1_v1_g0164_pc34_compat"),
    ROW(165, "dm1_v1_g0165_pc34_compat"), ROW(166, "dm1_v1_g0166_pc34_compat"),
    ROW(167, "dm1_v1_g0167_pc34_compat"), ROW(168, "dm1_v1_g0168_pc34_compat"),
    ROW(169, "dm1_v1_g0169_pc34_compat"), ROW(170, "dm1_v1_g0170_pc34_compat"),
    ROW(171, "dm1_v1_g0171_pc34_compat"), ROW(172, "dm1_v1_g0172_pc34_compat"),
    ROW(173, "dm1_v1_g0173_pc34_compat"), ROW(174, "dm1_v1_g0174_pc34_compat"),
    ROW(175, "dm1_v1_g0175_pc34_compat"), ROW(176, "dm1_v1_g0176_pc34_compat"),
    ROW(177, "dm1_v1_g0177_pc34_compat"), ROW(178, "dm1_v1_g0178_pc34_compat"),
    ROW(179, "dm1_v1_g0179_pc34_compat"), ROW(180, "dm1_v1_g0180_pc34_compat"),
    ROW(181, "dm1_v1_g0181_pc34_compat"), ROW(182, "dm1_v1_g0182_pc34_compat"),
    ROW(183, "dm1_v1_g0183_pc34_compat"), ROW(184, "dm1_v1_g0184_pc34_compat"),
    ROW(185, "dm1_v1_g0185_pc34_compat"), ROW(186, "dm1_v1_g0186_pc34_compat"),
    ROW(187, "dm1_v1_g0187_pc34_compat"), ROW(188, "dm1_v1_g0188_pc34_compat"),
    ROW(189, "fail_closed: source CPSE event-stop state"),
    ROW(190, "dm1_v1_g0190_pc34_compat"), ROW(191, "dm1_v1_g0191_pc34_compat"),
    ROW(192, "dm1_v1_g0192_pc34_compat"), ROW(193, "dm1_v1_g0193_pc34_compat"),
    ROW(194, "dm1_v1_g0194_pc34_compat"), ROW(195, "dm1_v1_g0195_pc34_compat"),
    ROW(196, "dm1_v1_g0196_pc34_compat"), ROW(197, "dm1_v1_g0197_pc34_compat"),
    ROW(198, "dm1_v1_g0198_pc34_compat"), ROW(199, "dm1_v1_g0199_pc34_compat"),
    ROW(200, "dm1_v1_g0200_pc34_compat")
};

#undef ROW

const DM1_V1_G0151G0200SourceAuditPc34 *
dm1_v1_g0151_g0200_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_G0151G0200SourceAuditPc34 *
dm1_v1_g0151_g0200_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_g0151_g0200_source_audit_evidence_pc34(void)
{
    return "ReDMCSB DUNVIEW.C:426-477 is the authority for G0151-G0200. "
           "Existing source-named Graphic558 owner modules are retained; G0189 "
           "remains fail closed without authentic raw PC34 material. The audit does "
           "not render or synthesize behavior.";
}
