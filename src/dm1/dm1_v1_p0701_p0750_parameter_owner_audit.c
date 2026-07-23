#include "dm1_v1_p0701_p0750_parameter_owner_audit.h"

#define ROW(number, owner, anchor) { number##u, owner##u, anchor, 1 }

static const DM1_V1_P0701P0750ParameterOwnerAudit k_audit[] = {
    ROW(701, 336, "PANEL.C:F0336"), ROW(702, 339, "PANEL.C:F0339"),
    ROW(703, 340, "PANEL.C:F0340"), ROW(704, 340, "PANEL.C:F0340"),
    ROW(705, 341, "PANEL.C:F0341"),
    ROW(706, 342, "PANEL.C:F0342"), ROW(707, 342, "PANEL.C:F0342"),
    ROW(708, 343, "PANEL.C:F0343"), ROW(709, 343, "PANEL.C:F0343"),
    ROW(710, 343, "PANEL.C:F0343"), ROW(711, 343, "PANEL.C:F0343"),
    ROW(712, 344, "PANEL.C:F0344"), ROW(713, 344, "PANEL.C:F0344"),
    ROW(714, 344, "PANEL.C:F0344"),
    ROW(715, 348, "PANEL.C:F0348"), ROW(716, 348, "PANEL.C:F0348"),
    ROW(717, 348, "PANEL.C:F0348"), ROW(718, 354, "PANEL.C:F0354"),
    ROW(719, 355, "PANEL.C:F0355"),
    ROW(720, 356, "COPYPRO7.C:F0356"),
    ROW(720, 356, "COPYPRO7.C:F0356"),
    ROW(721, 358, "COMMAND.C:F0358"), ROW(722, 358, "COMMAND.C:F0358"),
    ROW(723, 358, "COMMAND.C:F0358"), ROW(724, 358, "COMMAND.C:F0358"),
    ROW(725, 359, "COMMAND.C:F0359"), ROW(726, 359, "COMMAND.C:F0359"),
    ROW(727, 359, "COMMAND.C:F0359"), ROW(728, 361, "COMMAND.C:F0361"),
    ROW(729, 362, "CLIKMENU.C:F0362"), ROW(730, 362, "CLIKMENU.C:F0362"),
    ROW(731, 362, "CLIKMENU.C:F0362"), ROW(732, 362, "CLIKMENU.C:F0362"),
    ROW(733, 364, "CLIKMENU.C:F0364"), ROW(734, 365, "CLIKMENU.C:F0365"),
    ROW(735, 366, "CLIKMENU.C:F0366"),
    ROW(736, 367, "CLIKCHAM.C:F0367"), ROW(737, 367, "CLIKCHAM.C:F0367"),
    ROW(738, 367, "CLIKCHAM.C:F0367"), ROW(739, 368, "DEFS.H:F0368"),
    ROW(740, 369, "CLIKMENU.C:F0369"),
    ROW(741, 370, "CLIKMENU.C:F0370"), ROW(742, 370, "CLIKMENU.C:F0370"),
    ROW(743, 371, "CLIKMENU.C:F0371"), ROW(744, 371, "CLIKMENU.C:F0371"),
    ROW(745, 373, "CLIKVIEW.C:F0373"), ROW(746, 374, "CLIKVIEW.C:F0374"),
    ROW(747, 375, "CLIKVIEW.C:F0375"), ROW(748, 375, "CLIKVIEW.C:F0375"),
    ROW(749, 376, "CLIKVIEW.C:F0376"), ROW(749, 798, "COORD.C:F0798"),
    ROW(750, 376, "CLIKVIEW.C:F0376;COORD.C:F0798")
};

#undef ROW

const DM1_V1_P0701P0750ParameterOwnerAudit *
dm1_v1_p0701_p0750_parameter_owner_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_P0701P0750ParameterOwnerAudit *
dm1_v1_p0701_p0750_parameter_owner_find_pc34(uint16_t parameter_number,
                                               size_t occurrence)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].parameter_number == parameter_number &&
            occurrence-- == 0u) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_p0701_p0750_parameter_owner_evidence_pc34(void)
{
    return "ReDMCSB PANEL.C:F0336-F0355, COPYPRO7.C:F0356, COMMAND.C:F0358-"
           "F0361, CLIKMENU.C:F0362/F0364-F0371, CLIKCHAM.C:F0367, and "
           "CLIKVIEW.C:F0373-F0376/COORD.C:F0798 own P0701-P0750. They are "
           "routine parameters, not independent Firestaff state; a standalone "
           "port is forbidden.";
}
