#include "dm1_v1_f1306_f1325_fio_owner_audit.h"

int main(void)
{
    const DM1V1F1306F1325Audit *substitute =
        dm1_v1_f1306_f1325_fio_owner_audit(1321);
    const DM1V1F1306F1325Audit *disk =
        dm1_v1_f1306_f1325_fio_owner_audit(1325);

    return substitute != 0 &&
                   substitute->admission == DM1_V1_F1306_F1325_EXISTING_SOURCE_OWNER &&
                   disk != 0 && disk->admission == DM1_V1_F1306_F1325_FAIL_CLOSED &&
                   dm1_v1_f1306_f1325_fio_owner_audit(1326) == 0
               ? 0
               : 1;
}
