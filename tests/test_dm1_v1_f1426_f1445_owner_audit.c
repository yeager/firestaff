#include "dm1_v1_f1426_f1445_owner_audit.h"

int main(void)
{
    const DM1V1F1426F1445Audit *first =
        dm1_v1_f1426_f1445_owner_audit(1426);
    const DM1V1F1426F1445Audit *last =
        dm1_v1_f1426_f1445_owner_audit(1445);

    return first != 0 && first->failClosed && last != 0 && last->failClosed &&
                   dm1_v1_f1426_f1445_owner_audit(1446) == 0
               ? 0
               : 1;
}
