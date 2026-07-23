#include "dm1_v1_p0651_p0700_parameter_owner_audit.h"

#define ROW(number, owner, anchor) { number##u, owner##u, anchor, 1 }

static const DM1_V1_P0651P0700ParameterOwnerAudit k_audit[] = {
    ROW(651, 312, "CHAMPION.C:F0312"),
    ROW(652, 313, "CHAMPION.C:F0313"), ROW(653, 313, "CHAMPION.C:F0313"),
    ROW(654, 315, "CHAMPION.C:F0315"), ROW(655, 315, "CHAMPION.C:F0315"),
    ROW(656, 316, "CHAMPION.C:F0316"),
    ROW(657, 317, "CHAMPION.C:F0317"), ROW(658, 317, "CHAMPION.C:F0317"),
    ROW(659, 317, "CHAMPION.C:F0317"), ROW(660, 318, "CHAMPION.C:F0318"),
    ROW(661, 319, "CHAMPION.C:F0319"),
    ROW(662, 321, "CHAMPION.C:F0321"), ROW(663, 321, "CHAMPION.C:F0321"),
    ROW(664, 321, "CHAMPION.C:F0321"), ROW(665, 321, "CHAMPION.C:F0321"),
    ROW(666, 322, "CHAMPION.C:F0322"), ROW(667, 322, "CHAMPION.C:F0322"),
    ROW(668, 323, "CHAMPION.C:F0323"),
    ROW(669, 324, "CHAMPION.C:F0324"), ROW(670, 324, "CHAMPION.C:F0324"),
    ROW(671, 324, "CHAMPION.C:F0324"),
    ROW(672, 325, "CHAMPION.C:F0325"), ROW(673, 325, "CHAMPION.C:F0325"),
    ROW(674, 326, "CHAMPION.C:F0326"), ROW(675, 326, "CHAMPION.C:F0326"),
    ROW(676, 326, "CHAMPION.C:F0326"), ROW(677, 326, "CHAMPION.C:F0326"),
    ROW(678, 326, "CHAMPION.C:F0326"),
    ROW(679, 327, "CHAMPION.C:F0327"), ROW(680, 327, "CHAMPION.C:F0327"),
    ROW(681, 327, "CHAMPION.C:F0327"), ROW(682, 327, "CHAMPION.C:F0327"),
    ROW(683, 328, "CHAMPION.C:F0328"), ROW(684, 328, "CHAMPION.C:F0328"),
    ROW(685, 328, "CHAMPION.C:F0328"), ROW(686, 329, "CHAMPION.C:F0329"),
    ROW(687, 330, "CHAMPION.C:F0330"), ROW(688, 330, "CHAMPION.C:F0330"),
    ROW(689, 332, "PANEL.C:F0332"), ROW(690, 332, "PANEL.C:F0332"),
    ROW(691, 332, "PANEL.C:F0332"),
    ROW(692, 333, "CHEST.C:F0333"), ROW(693, 333, "CHEST.C:F0333"),
    ROW(694, 333, "CHEST.C:F0333"), ROW(695, 335, "PANEL.C:F0335"),
    ROW(696, 336, "PANEL.C:F0336"), ROW(697, 336, "PANEL.C:F0336"),
    ROW(698, 336, "PANEL.C:F0336"), ROW(699, 336, "PANEL.C:F0336"),
    ROW(700, 336, "PANEL.C:F0336")
};

#undef ROW

const DM1_V1_P0651P0700ParameterOwnerAudit *
dm1_v1_p0651_p0700_parameter_owner_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_P0651P0700ParameterOwnerAudit *
dm1_v1_p0651_p0700_parameter_owner_find_pc34(uint16_t parameter_number,
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

const char *dm1_v1_p0651_p0700_parameter_owner_evidence_pc34(void)
{
    return "ReDMCSB CHAMPION.C:F0312-F0330, PANEL.C:F0332/F0335-F0336, "
           "and CHEST.C:F0333 own P0651-P0700. They are routine "
           "parameters, not independent Firestaff state; a standalone "
           "port is forbidden.";
}
