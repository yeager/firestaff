#include "dm1_v1_p0001_p0050_parameter_owner_audit.h"

int main(void)
{
    const DM1V1P0001P0050Audit *screenBox =
        dm1_v1_p0001_p0050_parameter_owner_audit(1, 6);
    const DM1V1P0001P0050Audit *copyBytes =
        dm1_v1_p0001_p0050_parameter_owner_audit(7, 7317);
    const DM1V1P0001P0050Audit *text =
        dm1_v1_p0001_p0050_parameter_owner_audit(50, 40);

    return screenBox != 0 && screenBox->standaloneForbidden &&
                   copyBytes != 0 && copyBytes->standaloneForbidden &&
                   text != 0 && text->standaloneForbidden &&
                   dm1_v1_p0001_p0050_parameter_owner_audit(1, 7) == 0 &&
                   dm1_v1_p0001_p0050_parameter_owner_audit(51, 40) == 0
               ? 0
               : 1;
}
