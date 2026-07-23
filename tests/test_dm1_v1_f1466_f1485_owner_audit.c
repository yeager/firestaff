#include "dm1_v1_f1466_f1485_owner_audit.h"

int main(void)
{
    const DM1V1F1466F1485Audit *first =
        dm1_v1_f1466_f1485_owner_audit(1466);
    const DM1V1F1466F1485Audit *last =
        dm1_v1_f1466_f1485_owner_audit(1485);

    return first != 0 && first->failClosed && last != 0 && last->failClosed &&
                   dm1_v1_f1466_f1485_owner_audit(1486) == 0
               ? 0
               : 1;
}
