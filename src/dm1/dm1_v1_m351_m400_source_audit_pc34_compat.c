#include "dm1_v1_m351_m400_source_audit_pc34_compat.h"

#define ABSENT(number) {number, "M" #number "_absent", "ReDMCSB label inventory: no assigned module symbol"}

static const Dm1V1M351M400SourceAuditPc34 kSymbols[] = {
    ABSENT(351), ABSENT(352), ABSENT(353), ABSENT(354), ABSENT(355),
    ABSENT(356), ABSENT(357), ABSENT(358), ABSENT(359), ABSENT(360),
    ABSENT(361), ABSENT(362), ABSENT(363), ABSENT(364), ABSENT(365),
    ABSENT(366), ABSENT(367), ABSENT(368), ABSENT(369), ABSENT(370),
    ABSENT(371), ABSENT(372), ABSENT(373), ABSENT(374), ABSENT(375),
    ABSENT(376), ABSENT(377), ABSENT(378), ABSENT(379), ABSENT(380),
    ABSENT(381), ABSENT(382), ABSENT(383), ABSENT(384), ABSENT(385),
    ABSENT(386), ABSENT(387), ABSENT(388), ABSENT(389), ABSENT(390),
    ABSENT(391), ABSENT(392), ABSENT(393), ABSENT(394), ABSENT(395),
    ABSENT(396), ABSENT(397), ABSENT(398), ABSENT(399), ABSENT(400),
};

const Dm1V1M351M400SourceAuditPc34 *
dm1_v1_m351_m400_source_audit_pc34(unsigned int number)
{
    if (number < 351U || number > 400U) return 0;
    return &kSymbols[number - 351U];
}

int dm1_v1_m351_m400_has_verified_owner_pc34(unsigned int number)
{
    (void)number;
    return 0;
}

int dm1_v1_m351_m400_has_synthetic_route_pc34(void)
{
    return 0;
}

const char *dm1_v1_m351_m400_source_audit_evidence_pc34(void)
{
    return "ReDMCSB REDMCSB_LABEL_PARAMETER_FULL_AUDIT.tsv contains no M351-M400 "
           "entries, and the reference source has no M351-M400 label. No generated macro, "
           "graphics, UI, input, timing, memory, or platform route.";
}
