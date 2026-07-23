#include "dm1_v1_m151_m200_source_audit_pc34_compat.h"

#define ABSENT(number) {number, "M" #number "_absent", "ReDMCSB label inventory: no assigned module symbol"}

static const Dm1V1M151M200SourceAuditPc34 kSymbols[] = {
    ABSENT(151), ABSENT(152), ABSENT(153), ABSENT(154), ABSENT(155),
    ABSENT(156), ABSENT(157), ABSENT(158), ABSENT(159), ABSENT(160),
    ABSENT(161), ABSENT(162), ABSENT(163), ABSENT(164), ABSENT(165),
    ABSENT(166), ABSENT(167), ABSENT(168), ABSENT(169), ABSENT(170),
    ABSENT(171), ABSENT(172), ABSENT(173), ABSENT(174), ABSENT(175),
    ABSENT(176), ABSENT(177), ABSENT(178), ABSENT(179), ABSENT(180),
    ABSENT(181), ABSENT(182), ABSENT(183), ABSENT(184), ABSENT(185),
    ABSENT(186), ABSENT(187), ABSENT(188), ABSENT(189), ABSENT(190),
    ABSENT(191), ABSENT(192), ABSENT(193), ABSENT(194), ABSENT(195),
    ABSENT(196), ABSENT(197), ABSENT(198), ABSENT(199), ABSENT(200),
};

const Dm1V1M151M200SourceAuditPc34 *
dm1_v1_m151_m200_source_audit_pc34(unsigned int number)
{
    if (number < 151U || number > 200U) return 0;
    return &kSymbols[number - 151U];
}

int dm1_v1_m151_m200_has_verified_owner_pc34(unsigned int number)
{
    (void)number;
    return 0;
}

int dm1_v1_m151_m200_has_synthetic_route_pc34(void)
{
    return 0;
}

const char *dm1_v1_m151_m200_source_audit_evidence_pc34(void)
{
    return "ReDMCSB REDMCSB_LABEL_PARAMETER_FULL_AUDIT.tsv contains no M151-M200 "
           "entries, and the reference source has no M151-M200 label. No generated macro, "
           "graphics, UI, input, timing, memory, or platform route.";
}
