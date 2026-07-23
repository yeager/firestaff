#include "dm1_v1_p0051_p0100_parameter_owner_audit.h"

int main(void)
{
    const DM1V1P0051P0100Audit *text =
        dm1_v1_p0051_p0100_parameter_owner_audit(57, 768);
    const DM1V1P0051P0100Audit *sound =
        dm1_v1_p0051_p0100_parameter_owner_audit(84, 60);
    const DM1V1P0051P0100Audit *viewport =
        dm1_v1_p0051_p0100_parameter_owner_audit(100, 97);

    return text != 0 && text->standaloneForbidden && sound != 0 &&
                   sound->standaloneForbidden && viewport != 0 &&
                   viewport->standaloneForbidden &&
                   dm1_v1_p0051_p0100_parameter_owner_audit(57, 52) == 0 &&
                   dm1_v1_p0051_p0100_parameter_owner_audit(101, 97) == 0
               ? 0
               : 1;
}
