#include "dm1_v1_p0451_p0500_parameter_owner_audit.h"

int main(void)
{
    const DM1V1P0451P0500Audit *impact =
        dm1_v1_p0451_p0500_parameter_owner_audit(453);
    const DM1V1P0451P0500Audit *visibility =
        dm1_v1_p0451_p0500_parameter_owner_audit(478);
    const DM1V1P0451P0500Audit *melee =
        dm1_v1_p0451_p0500_parameter_owner_audit(495);
    const DM1V1P0451P0500Audit *mapY =
        dm1_v1_p0451_p0500_parameter_owner_audit(500);

    return impact != 0 && impact->ownerRoutine == 217 &&
                   visibility != 0 && visibility->ownerRoutine == 227 &&
                   melee != 0 && melee->ownerRoutine == 231 &&
                   mapY != 0 && mapY->ownerRoutine == 231 &&
                   dm1_v1_p0451_p0500_parameter_owner_audit(450) == 0 &&
                   dm1_v1_p0451_p0500_parameter_owner_audit(501) == 0
               ? 0
               : 1;
}
