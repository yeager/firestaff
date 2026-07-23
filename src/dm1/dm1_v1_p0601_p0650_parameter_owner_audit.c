#include "dm1_v1_p0601_p0650_parameter_owner_audit.h"

#define ROW(number, owner, anchor) { number##u, owner##u, anchor, 1 }

static const DM1_V1_P0601P0650ParameterOwnerAudit k_audit[] = {
    ROW(601, 285, "CHAMPION.C:F0285"),
    ROW(602, 286, "CHAMPION.C:F0286"), ROW(603, 286, "CHAMPION.C:F0286"),
    ROW(604, 286, "CHAMPION.C:F0286"), ROW(605, 287, "CHAMDRAW.C:F0287"),
    ROW(606, 288, "CHAMDRAW.C:F0288"), ROW(607, 288, "CHAMDRAW.C:F0288"),
    ROW(608, 288, "CHAMDRAW.C:F0288"), ROW(609, 289, "CHAMDRAW.C:F0289"),
    ROW(609, 289, "CHAMDRAW.C:F0289"), ROW(610, 289, "CHAMDRAW.C:F0289"),
    ROW(611, 289, "CHAMDRAW.C:F0289"), ROW(612, 290, "CHAMDRAW.C:F0290"),
    ROW(613, 291, "CHAMDRAW.C:F0291"), ROW(614, 291, "CHAMDRAW.C:F0291"),
    ROW(615, 292, "CHAMDRAW.C:F0292"), ROW(616, 294, "CHAMPION.C:F0294"),
    ROW(617, 294, "CHAMPION.C:F0294"), ROW(618, 294, "CHAMPION.C:F0294"),
    ROW(619, 295, "CHAMDRAW.C:F0295"), ROW(620, 295, "CHAMDRAW.C:F0295"),
    ROW(621, 2, "MAIN.C:F0002"), ROW(621, 297, "CHAMPION.C:F0297"),
    ROW(622, 2, "MAIN.C:F0002"), ROW(622, 297, "CHAMPION.C:F0297"),
    ROW(623, 299, "CHAMPION.C:F0299"), ROW(624, 299, "CHAMPION.C:F0299"),
    ROW(624, 7020, "CEDT004.C:F7020"), ROW(625, 299, "CHAMPION.C:F0299"),
    ROW(625, 7020, "CEDT004.C:F7020"), ROW(626, 299, "CHAMPION.C:F0299"),
    ROW(627, 299, "CHAMPION.C:F0299"), ROW(628, 300, "CHAMPION.C:F0300"),
    ROW(629, 300, "CHAMPION.C:F0300"), ROW(630, 301, "CHAMPION.C:F0301"),
    ROW(631, 301, "CHAMPION.C:F0301"), ROW(632, 301, "CHAMPION.C:F0301"),
    ROW(633, 302, "CHAMPION.C:F0302"), ROW(634, 303, "CHAMPION.C:F0303"),
    ROW(635, 303, "CHAMPION.C:F0303"), ROW(636, 304, "CHAMPION.C:F0304"),
    ROW(637, 304, "CHAMPION.C:F0304"), ROW(638, 304, "CHAMPION.C:F0304"),
    ROW(639, 305, "CHAMPION.C:F0305"), ROW(640, 306, "CHAMPION.C:F0306"),
    ROW(641, 306, "CHAMPION.C:F0306"), ROW(642, 307, "CHAMPION.C:F0307"),
    ROW(643, 307, "CHAMPION.C:F0307"), ROW(644, 307, "CHAMPION.C:F0307"),
    ROW(645, 308, "CHAMPION.C:F0308"), ROW(646, 308, "CHAMPION.C:F0308"),
    ROW(647, 309, "CHAMPION.C:F0309"), ROW(648, 310, "CHAMPION.C:F0310"),
    ROW(649, 311, "CHAMPION.C:F0311"), ROW(650, 312, "CHAMPION.C:F0312")
};

#undef ROW

const DM1_V1_P0601P0650ParameterOwnerAudit *
dm1_v1_p0601_p0650_parameter_owner_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_P0601P0650ParameterOwnerAudit *
dm1_v1_p0601_p0650_parameter_owner_find_pc34(uint16_t parameter_number,
                                               size_t occurrence)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].parameter_number == parameter_number &&
            occurrence-- == 0u) {
            return &k_audit[index];
        }
    }
    return 0;
}

const char *dm1_v1_p0601_p0650_parameter_owner_evidence_pc34(void)
{
    return "ReDMCSB CHAMPION.C:F0285-F0312, CHAMDRAW.C:F0287-F0295, "
           "MAIN.C:F0002, and CEDT004.C:F7020 own P0601-P0650. "
           "They are routine parameters, not independent Firestaff state; "
           "a standalone port is forbidden.";
}
