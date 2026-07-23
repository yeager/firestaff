#include "dm1_v1_f1686_f1705_usio_owner_audit.h"

int main(void)
{
    const DM1V1F1686F1705Audit *keyboard =
        dm1_v1_f1686_f1705_usio_owner_audit(1690);
    const DM1V1F1686F1705Audit *interrupt =
        dm1_v1_f1686_f1705_usio_owner_audit(1687);
    const DM1V1F1686F1705Audit *absent =
        dm1_v1_f1686_f1705_usio_owner_audit(1705);

    return keyboard != 0 &&
                   keyboard->admission ==
                       DM1_V1_F1686_F1705_EXISTING_SOURCE_OWNER &&
                   interrupt != 0 &&
                   interrupt->admission ==
                       DM1_V1_F1686_F1705_PLATFORM_FAIL_CLOSED &&
                   absent != 0 &&
                   absent->admission == DM1_V1_F1686_F1705_ABSENT_FAIL_CLOSED &&
                   dm1_v1_f1686_f1705_usio_owner_audit(1706) == 0
               ? 0
               : 1;
}
