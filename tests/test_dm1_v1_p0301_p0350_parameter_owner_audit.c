#include "dm1_v1_p0301_p0350_parameter_owner_audit.h"

int main(void)
{
    const DM1V1P0301P0350Audit *ornament =
        dm1_v1_p0301_p0350_parameter_owner_audit(309);
    const DM1V1P0301P0350Audit *target =
        dm1_v1_p0301_p0350_parameter_owner_audit(327);
    const DM1V1P0301P0350Audit *generate =
        dm1_v1_p0301_p0350_parameter_owner_audit(350);

    return ornament != 0 && ornament->ownerRoutine == 171 &&
                   target != 0 && target->ownerRoutine == 177 &&
                   generate != 0 && generate->ownerRoutine == 185 &&
                   dm1_v1_p0301_p0350_parameter_owner_audit(300) == 0 &&
                   dm1_v1_p0301_p0350_parameter_owner_audit(351) == 0
               ? 0
               : 1;
}
