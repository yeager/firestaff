#include "dm1_v1_p0401_p0450_parameter_owner_audit.h"

int main(void)
{
    const DM1V1P0401P0450Audit *movement =
        dm1_v1_p0401_p0450_parameter_owner_audit(401);
    const DM1V1P0401P0450Audit *event =
        dm1_v1_p0401_p0450_parameter_owner_audit(427);
    const DM1V1P0401P0450Audit *projectile =
        dm1_v1_p0401_p0450_parameter_owner_audit(440);
    const DM1V1P0401P0450Audit *deletion =
        dm1_v1_p0401_p0450_parameter_owner_audit(450);

    return movement != 0 && movement->ownerRoutine == 202 &&
                   event != 0 && event->ownerRoutine == 209 &&
                   projectile != 0 && projectile->ownerRoutine == 212 &&
                   deletion != 0 && deletion->ownerRoutine == 215 &&
                   dm1_v1_p0401_p0450_parameter_owner_audit(400) == 0 &&
                   dm1_v1_p0401_p0450_parameter_owner_audit(451) == 0
               ? 0
               : 1;
}
