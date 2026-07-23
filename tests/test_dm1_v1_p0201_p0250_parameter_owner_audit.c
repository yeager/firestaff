#include "dm1_v1_p0201_p0250_parameter_owner_audit.h"

int main(void)
{
    const DM1V1P0201P0250Audit *blit =
        dm1_v1_p0201_p0250_parameter_owner_audit(201);
    const DM1V1P0201P0250Audit *projectile =
        dm1_v1_p0201_p0250_parameter_owner_audit(238);
    const DM1V1P0201P0250Audit *group =
        dm1_v1_p0201_p0250_parameter_owner_audit(250);

    return blit != 0 && blit->ownerRoutine == 0 &&
                   blit->admission == DM1_V1_P0201_P0250_FAIL_CLOSED &&
                   projectile != 0 && projectile->ownerRoutine == 142 &&
                   projectile->admission ==
                       DM1_V1_P0201_P0250_EXISTING_PC34_OWNER &&
                   group != 0 && group->ownerRoutine == 148 &&
                   group->admission == DM1_V1_P0201_P0250_EXISTING_PC34_OWNER &&
                   dm1_v1_p0201_p0250_parameter_owner_audit(251) == 0
               ? 0
               : 1;
}
