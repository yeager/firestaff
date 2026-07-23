#include "dm1_v1_m301_m350_source_audit_pc34_compat.h"

#define ABSENT(number) {number, "M" #number "_absent", "ReDMCSB label inventory: no assigned module symbol"}

static const Dm1V1M301M350SourceAuditPc34 kSymbols[] = {
    ABSENT(301), ABSENT(302), ABSENT(303), ABSENT(304), ABSENT(305),
    ABSENT(306), ABSENT(307), ABSENT(308), ABSENT(309), ABSENT(310),
    ABSENT(311), ABSENT(312), ABSENT(313), ABSENT(314), ABSENT(315),
    ABSENT(316), ABSENT(317), ABSENT(318), ABSENT(319), ABSENT(320),
    ABSENT(321), ABSENT(322), ABSENT(323), ABSENT(324), ABSENT(325),
    ABSENT(326), ABSENT(327), ABSENT(328), ABSENT(329), ABSENT(330),
    ABSENT(331), ABSENT(332), ABSENT(333), ABSENT(334), ABSENT(335),
    ABSENT(336), ABSENT(337), ABSENT(338), ABSENT(339), ABSENT(340),
    ABSENT(341), ABSENT(342), ABSENT(343), ABSENT(344), ABSENT(345),
    ABSENT(346), ABSENT(347), ABSENT(348), ABSENT(349), ABSENT(350),
};

const Dm1V1M301M350SourceAuditPc34 *
dm1_v1_m301_m350_source_audit_pc34(unsigned int number)
{
    if (number < 301U || number > 350U) return 0;
    return &kSymbols[number - 301U];
}

int dm1_v1_m301_m350_has_verified_owner_pc34(unsigned int number)
{
    (void)number;
    return 0;
}

int dm1_v1_m301_m350_has_synthetic_route_pc34(void)
{
    return 0;
}

const char *dm1_v1_m301_m350_source_audit_evidence_pc34(void)
{
    return "ReDMCSB REDMCSB_LABEL_PARAMETER_FULL_AUDIT.tsv contains no M301-M350 "
           "entries, and the reference source has no M301-M350 label. No generated macro, "
           "graphics, UI, input, timing, memory, or platform route.";
}
