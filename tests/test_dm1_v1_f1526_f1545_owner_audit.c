#include "dm1_v1_f1526_f1545_owner_audit.h"

int main(void)
{
    const DM1V1F1526F1545Audit *screen =
        dm1_v1_f1526_f1545_owner_audit(1528);
    const DM1V1F1526F1545Audit *aes =
        dm1_v1_f1526_f1545_owner_audit(1534);

    return screen != 0 && screen->failClosed && aes != 0 && aes->failClosed &&
                   dm1_v1_f1526_f1545_owner_audit(1546) == 0
               ? 0
               : 1;
}
