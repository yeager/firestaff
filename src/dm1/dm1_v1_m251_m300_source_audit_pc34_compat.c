#include "dm1_v1_m251_m300_source_audit_pc34_compat.h"

#define ABSENT(number) {number, "M" #number "_absent", "ReDMCSB label inventory: no assigned module symbol"}

static const Dm1V1M251M300SourceAuditPc34 kSymbols[] = {
    ABSENT(251), ABSENT(252), ABSENT(253), ABSENT(254), ABSENT(255),
    ABSENT(256), ABSENT(257), ABSENT(258), ABSENT(259), ABSENT(260),
    ABSENT(261), ABSENT(262), ABSENT(263), ABSENT(264), ABSENT(265),
    ABSENT(266), ABSENT(267), ABSENT(268), ABSENT(269), ABSENT(270),
    ABSENT(271), ABSENT(272), ABSENT(273), ABSENT(274), ABSENT(275),
    ABSENT(276), ABSENT(277), ABSENT(278), ABSENT(279), ABSENT(280),
    ABSENT(281), ABSENT(282), ABSENT(283), ABSENT(284), ABSENT(285),
    ABSENT(286), ABSENT(287), ABSENT(288), ABSENT(289), ABSENT(290),
    ABSENT(291), ABSENT(292), ABSENT(293), ABSENT(294), ABSENT(295),
    ABSENT(296), ABSENT(297), ABSENT(298), ABSENT(299), ABSENT(300),
};

const Dm1V1M251M300SourceAuditPc34 *
dm1_v1_m251_m300_source_audit_pc34(unsigned int number)
{
    if (number < 251U || number > 300U) return 0;
    return &kSymbols[number - 251U];
}

int dm1_v1_m251_m300_has_verified_owner_pc34(unsigned int number)
{
    (void)number;
    return 0;
}

int dm1_v1_m251_m300_has_synthetic_route_pc34(void)
{
    return 0;
}

const char *dm1_v1_m251_m300_source_audit_evidence_pc34(void)
{
    return "ReDMCSB REDMCSB_LABEL_PARAMETER_FULL_AUDIT.tsv contains no M251-M300 "
           "entries, and the reference source has no M251-M300 label. No generated macro, "
           "graphics, UI, input, timing, memory, or platform route.";
}
