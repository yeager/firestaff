#include "dm1_v1_f1726_f1745_owner_audit.h"

int main(void)
{
    const DM1V1F1726F1745Audit *first =
        dm1_v1_f1726_f1745_owner_audit(1726);
    const DM1V1F1726F1745Audit *last =
        dm1_v1_f1726_f1745_owner_audit(1745);

    return first != 0 && first->failClosed && last != 0 && last->failClosed &&
                   dm1_v1_f1726_f1745_owner_audit(1746) == 0
               ? 0
               : 1;
}
