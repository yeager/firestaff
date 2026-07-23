#include "dm1_v1_f1146_f1165_io_owner_audit.h"

int main(void)
{
    const DM1V1F1146F1165Audit *cpsx =
        dm1_v1_f1146_f1165_io_owner_audit(1148);
    const DM1V1F1146F1165Audit *usio =
        dm1_v1_f1146_f1165_io_owner_audit(1165);

    return cpsx != 0 && cpsx->failClosed && usio != 0 && usio->failClosed &&
                   dm1_v1_f1146_f1165_io_owner_audit(1166) == 0
               ? 0
               : 1;
}
