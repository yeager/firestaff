#include "dm1_v1_l0001_l0050_local_owner_audit.h"

int main(void)
{
    const DM1V1L0001L0050Audit *object =
        dm1_v1_l0001_l0050_local_owner_audit(1, 31);
    const DM1V1L0001L0050Audit *text =
        dm1_v1_l0001_l0050_local_owner_audit(30, 46);
    const DM1V1L0001L0050Audit *sound =
        dm1_v1_l0001_l0050_local_owner_audit(44, 64);

    return object != 0 && object->standaloneForbidden && text != 0 &&
                   text->standaloneForbidden && sound != 0 &&
                   sound->standaloneForbidden &&
                   dm1_v1_l0001_l0050_local_owner_audit(1, 32) == 0 &&
                   dm1_v1_l0001_l0050_local_owner_audit(51, 66) == 0
               ? 0
               : 1;
}
