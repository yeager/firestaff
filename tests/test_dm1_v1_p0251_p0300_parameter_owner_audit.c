#include "dm1_v1_p0251_p0300_parameter_owner_audit.h"

int main(void)
{
    const DM1V1P0251P0300Audit *movement =
        dm1_v1_p0251_p0300_parameter_owner_audit(253);
    const DM1V1P0251P0300Audit *thingList =
        dm1_v1_p0251_p0300_parameter_owner_audit(287);
    const DM1V1P0251P0300Audit *text =
        dm1_v1_p0251_p0300_parameter_owner_audit(300);

    return movement != 0 && movement->ownerRoutine == 150 &&
                   thingList != 0 && thingList->ownerRoutine == 163 &&
                   text != 0 && text->ownerRoutine == 168 &&
                   dm1_v1_p0251_p0300_parameter_owner_audit(250) == 0 &&
                   dm1_v1_p0251_p0300_parameter_owner_audit(301) == 0
               ? 0
               : 1;
}
