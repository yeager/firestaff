#include "dm1_v1_f1366_f1385_owner_audit.h"

int main(void)
{
    const DM1V1F1366F1385Audit *logo =
        dm1_v1_f1366_f1385_owner_audit(1368);
    const DM1V1F1366F1385Audit *vblank =
        dm1_v1_f1366_f1385_owner_audit(1372);

    return logo != 0 && logo->failClosed && vblank != 0 && vblank->failClosed &&
                   dm1_v1_f1366_f1385_owner_audit(1386) == 0
               ? 0
               : 1;
}
