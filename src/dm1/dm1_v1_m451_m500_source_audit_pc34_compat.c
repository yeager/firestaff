#include "dm1_v1_m451_m500_source_audit_pc34_compat.h"

#define ABSENT(number) {number, "M" #number "_absent", "ReDMCSB label inventory", DM1_V1_M451_M500_ABSENT_PC34}

static const Dm1V1M451M500SourceAuditPc34 kSymbols[] = {
    ABSENT(451), ABSENT(452), ABSENT(453), ABSENT(454), ABSENT(455),
    ABSENT(456), ABSENT(457), ABSENT(458), ABSENT(459), ABSENT(460),
    ABSENT(461), ABSENT(462), ABSENT(463), ABSENT(464), ABSENT(465),
    ABSENT(466), ABSENT(467), ABSENT(468), ABSENT(469), ABSENT(470),
    ABSENT(471), ABSENT(472), ABSENT(473), ABSENT(474), ABSENT(475),
    ABSENT(476), ABSENT(477), ABSENT(478), ABSENT(479), ABSENT(480),
    ABSENT(481), ABSENT(482), ABSENT(483), ABSENT(484), ABSENT(485),
    ABSENT(486), ABSENT(487), ABSENT(488), ABSENT(489), ABSENT(490),
    ABSENT(491), ABSENT(492), ABSENT(493), ABSENT(494), ABSENT(495),
    ABSENT(496), ABSENT(497), ABSENT(498), ABSENT(499),
    {500, "M500_RGB_BLACK", "DEFS.H:2098; TITLE.C", DM1_V1_M451_M500_UNVERIFIED_NO_ROUTE_PC34},
};

const Dm1V1M451M500SourceAuditPc34 *
dm1_v1_m451_m500_source_audit_pc34(unsigned int number)
{
    if (number < 451U || number > 500U) return 0;
    return &kSymbols[number - 451U];
}

int dm1_v1_m451_m500_has_verified_owner_pc34(unsigned int number)
{
    (void)number;
    return 0;
}

int dm1_v1_m451_m500_has_synthetic_route_pc34(void)
{
    return 0;
}

const char *dm1_v1_m451_m500_source_audit_evidence_pc34(void)
{
    return "ReDMCSB REDMCSB_LABEL_PARAMETER_FULL_AUDIT.tsv M500; DEFS.H:2098; "
           "TITLE.C. M451-M499 are absent. M500 remains a non-standalone macro with no "
           "verified PC34 owner. No generated macro, palette, graphics, UI, input, timing, "
           "memory, or platform route.";
}
