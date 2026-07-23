#include "dm1_v1_p0151_p0200_parameter_owner_audit.h"

int main(void)
{
    const DM1V1P0151P0200Audit *square =
        dm1_v1_p0151_p0200_parameter_owner_audit(153, 118);
    const DM1V1P0151P0200Audit *shrink =
        dm1_v1_p0151_p0200_parameter_owner_audit(186, 1004);
    const DM1V1P0151P0200Audit *blit =
        dm1_v1_p0151_p0200_parameter_owner_audit(200, 7330);

    return square != 0 && square->standaloneForbidden && shrink != 0 &&
                   shrink->standaloneForbidden && blit != 0 &&
                   blit->standaloneForbidden &&
                   dm1_v1_p0151_p0200_parameter_owner_audit(199, 1013) == 0 &&
                   dm1_v1_p0151_p0200_parameter_owner_audit(201, 132) == 0
               ? 0
               : 1;
}
