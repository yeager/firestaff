#include "dm1_v1_p0351_p0400_parameter_owner_audit.h"

int main(void)
{
    const DM1V1P0351P0400Audit *possessions =
        dm1_v1_p0351_p0400_parameter_owner_audit(355);
    const DM1V1P0351P0400Audit *damage =
        dm1_v1_p0351_p0400_parameter_owner_audit(369);
    const DM1V1P0351P0400Audit *movement =
        dm1_v1_p0351_p0400_parameter_owner_audit(400);

    return possessions != 0 && possessions->ownerRoutine == 186 &&
                   damage != 0 && damage->ownerRoutine == 190 &&
                   movement != 0 && movement->ownerRoutine == 202 &&
                   dm1_v1_p0351_p0400_parameter_owner_audit(350) == 0 &&
                   dm1_v1_p0351_p0400_parameter_owner_audit(401) == 0
               ? 0
               : 1;
}
