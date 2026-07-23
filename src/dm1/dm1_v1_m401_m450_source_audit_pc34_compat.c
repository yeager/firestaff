#include "dm1_v1_m401_m450_source_audit_pc34_compat.h"

#define ABSENT(number) {number, "M" #number "_absent", "ReDMCSB label inventory: no assigned module symbol"}

static const Dm1V1M401M450SourceAuditPc34 kSymbols[] = {
    ABSENT(401), ABSENT(402), ABSENT(403), ABSENT(404), ABSENT(405),
    ABSENT(406), ABSENT(407), ABSENT(408), ABSENT(409), ABSENT(410),
    ABSENT(411), ABSENT(412), ABSENT(413), ABSENT(414), ABSENT(415),
    ABSENT(416), ABSENT(417), ABSENT(418), ABSENT(419), ABSENT(420),
    ABSENT(421), ABSENT(422), ABSENT(423), ABSENT(424), ABSENT(425),
    ABSENT(426), ABSENT(427), ABSENT(428), ABSENT(429), ABSENT(430),
    ABSENT(431), ABSENT(432), ABSENT(433), ABSENT(434), ABSENT(435),
    ABSENT(436), ABSENT(437), ABSENT(438), ABSENT(439), ABSENT(440),
    ABSENT(441), ABSENT(442), ABSENT(443), ABSENT(444), ABSENT(445),
    ABSENT(446), ABSENT(447), ABSENT(448), ABSENT(449), ABSENT(450),
};

const Dm1V1M401M450SourceAuditPc34 *
dm1_v1_m401_m450_source_audit_pc34(unsigned int number)
{
    if (number < 401U || number > 450U) return 0;
    return &kSymbols[number - 401U];
}

int dm1_v1_m401_m450_has_verified_owner_pc34(unsigned int number)
{
    (void)number;
    return 0;
}

int dm1_v1_m401_m450_has_synthetic_route_pc34(void)
{
    return 0;
}

const char *dm1_v1_m401_m450_source_audit_evidence_pc34(void)
{
    return "ReDMCSB REDMCSB_LABEL_PARAMETER_FULL_AUDIT.tsv contains no M401-M450 "
           "entries, and the reference source has no M401-M450 label. No generated macro, "
           "graphics, UI, input, timing, memory, or platform route.";
}
