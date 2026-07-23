#include "dm1_v1_f1246_f1265_owner_audit.h"

int main(void)
{
    const DM1V1F1246F1265Audit *fade =
        dm1_v1_f1246_f1265_owner_audit(1253);
    const DM1V1F1246F1265Audit *media =
        dm1_v1_f1246_f1265_owner_audit(1255);

    return fade != 0 && fade->failClosed && media != 0 && media->failClosed &&
                   dm1_v1_f1246_f1265_owner_audit(1266) == 0
               ? 0
               : 1;
}
