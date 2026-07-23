#include "dm1_v1_p0101_p0150_parameter_owner_audit.h"

int main(void)
{
    const DM1V1P0101P0150Audit *copy =
        dm1_v1_p0101_p0150_parameter_owner_audit(101, 791);
    const DM1V1P0101P0150Audit *door =
        dm1_v1_p0101_p0150_parameter_owner_audit(124, 111);
    const DM1V1P0101P0150Audit *square =
        dm1_v1_p0101_p0150_parameter_owner_audit(150, 117);

    return copy != 0 && copy->standaloneForbidden && door != 0 &&
                   door->standaloneForbidden && square != 0 &&
                   square->standaloneForbidden &&
                   dm1_v1_p0101_p0150_parameter_owner_audit(101, 100) == 0 &&
                   dm1_v1_p0101_p0150_parameter_owner_audit(151, 117) == 0
               ? 0
               : 1;
}
