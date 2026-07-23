#include "dm1_v1_f1866_f1905_hint_owner_audit.h"

int main(void)
{
    const DM1V1F1866F1905Audit *vblank =
        dm1_v1_f1866_f1905_hint_owner_audit(1870);
    const DM1V1F1866F1905Audit *graphics =
        dm1_v1_f1866_f1905_hint_owner_audit(1872);
    const DM1V1F1866F1905Audit *absent =
        dm1_v1_f1866_f1905_hint_owner_audit(1905);

    return vblank != 0 &&
                   vblank->admission == DM1_V1_F1866_F1905_PLATFORM_FAIL_CLOSED &&
                   graphics != 0 &&
                   graphics->admission == DM1_V1_F1866_F1905_UNPROVEN_FAIL_CLOSED &&
                   absent != 0 &&
                   absent->admission == DM1_V1_F1866_F1905_ABSENT_FAIL_CLOSED &&
                   dm1_v1_f1866_f1905_hint_owner_audit(1906) == 0
               ? 0
               : 1;
}
