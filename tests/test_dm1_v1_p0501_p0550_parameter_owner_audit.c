#include "dm1_v1_p0501_p0550_parameter_owner_audit.h"

int main(void)
{
    const DM1V1P0501P0550Audit *melee =
        dm1_v1_p0501_p0550_parameter_owner_audit(501);
    const DM1V1P0501P0550Audit *timeline =
        dm1_v1_p0501_p0550_parameter_owner_audit(522);
    const DM1V1P0501P0550Audit *teleporter =
        dm1_v1_p0501_p0550_parameter_owner_audit(544);
    const DM1V1P0501P0550Audit *movement =
        dm1_v1_p0501_p0550_parameter_owner_audit(550);

    return melee != 0 && melee->ownerRoutine == 231 &&
                   timeline != 0 && timeline->ownerRoutine == 247 &&
                   teleporter != 0 && teleporter->ownerRoutine == 263 &&
                   movement != 0 && movement->ownerRoutine == 265 &&
                   dm1_v1_p0501_p0550_parameter_owner_audit(500) == 0 &&
                   dm1_v1_p0501_p0550_parameter_owner_audit(533) == 0 &&
                   dm1_v1_p0501_p0550_parameter_owner_audit(551) == 0
               ? 0
               : 1;
}
