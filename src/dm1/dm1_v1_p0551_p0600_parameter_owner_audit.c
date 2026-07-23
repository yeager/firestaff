#include "dm1_v1_p0551_p0600_parameter_owner_audit.h"

#define ROW(number, owner, anchor) { number##u, owner##u, anchor, 1 }

static const DM1_V1_P0551P0600ParameterOwnerAudit k_audit[] = {
    ROW(551, 265, "MOVESENS.C:F0265"),
    ROW(552, 266, "MOVESENS.C:F0266"), ROW(553, 266, "MOVESENS.C:F0266"),
    ROW(554, 266, "MOVESENS.C:F0266"), ROW(555, 266, "MOVESENS.C:F0266"),
    ROW(556, 266, "MOVESENS.C:F0266"),
    ROW(557, 267, "MOVESENS.C:F0267"), ROW(558, 267, "MOVESENS.C:F0267"),
    ROW(559, 267, "MOVESENS.C:F0267"), ROW(560, 267, "MOVESENS.C:F0267"),
    ROW(561, 267, "MOVESENS.C:F0267"),
    ROW(562, 268, "MOVESENS.C:F0268"), ROW(563, 268, "MOVESENS.C:F0268"),
    ROW(564, 268, "MOVESENS.C:F0268"), ROW(565, 268, "MOVESENS.C:F0268"),
    ROW(566, 268, "MOVESENS.C:F0268"), ROW(567, 268, "MOVESENS.C:F0268"),
    ROW(568, 269, "MOVESENS.C:F0269"), ROW(569, 269, "MOVESENS.C:F0269"),
    ROW(570, 269, "MOVESENS.C:F0269"),
    ROW(571, 270, "MOVESENS.C:F0270"), ROW(572, 270, "MOVESENS.C:F0270"),
    ROW(573, 270, "MOVESENS.C:F0270"), ROW(574, 270, "MOVESENS.C:F0270"),
    ROW(575, 272, "MOVESENS.C:F0272"), ROW(576, 272, "MOVESENS.C:F0272"),
    ROW(577, 272, "MOVESENS.C:F0272"), ROW(578, 272, "MOVESENS.C:F0272"),
    ROW(579, 272, "MOVESENS.C:F0272"),
    ROW(580, 273, "MOVESENS.C:F0273"), ROW(581, 273, "MOVESENS.C:F0273"),
    ROW(582, 273, "MOVESENS.C:F0273"), ROW(583, 273, "MOVESENS.C:F0273"),
    ROW(584, 274, "MOVESENS.C:F0274"),
    ROW(585, 275, "MOVESENS.C:F0275"), ROW(586, 275, "MOVESENS.C:F0275"),
    ROW(587, 275, "MOVESENS.C:F0275"),
    ROW(588, 276, "MOVESENS.C:F0276"), ROW(589, 276, "MOVESENS.C:F0276"),
    ROW(590, 276, "MOVESENS.C:F0276"), ROW(591, 276, "MOVESENS.C:F0276"),
    ROW(592, 276, "MOVESENS.C:F0276"),
    ROW(593, 277, "COPYPRO6.C:F0277"), ROW(593, 277, "COPYPRO6.C:F0277"),
    ROW(594, 279, "REVIVE.C:F0279"), ROW(595, 279, "REVIVE.C:F0279"),
    ROW(596, 280, "REVIVE.C:F0280"), ROW(597, 281, "REVIVE.C:F0281"),
    ROW(598, 282, "REVIVE.C:F0282"), ROW(599, 283, "REVIVE.C:F0283"),
    ROW(600, 284, "CHAMPION.C:F0284")
};

#undef ROW

const DM1_V1_P0551P0600ParameterOwnerAudit *
dm1_v1_p0551_p0600_parameter_owner_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_P0551P0600ParameterOwnerAudit *
dm1_v1_p0551_p0600_parameter_owner_find_pc34(uint16_t parameter_number,
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

const char *dm1_v1_p0551_p0600_parameter_owner_evidence_pc34(void)
{
    return "ReDMCSB MOVESENS.C:F0265-F0276, COPYPRO6.C:F0277, "
           "REVIVE.C:F0279-F0283, and CHAMPION.C:F0284 own P0551-P0600. "
           "They are routine parameters, not independent Firestaff state; "
           "a standalone port is forbidden.";
}
