#include "dm1_v1_f1586_f1605_tos_aes_owner_audit.h"

int main(void)
{
    const DM1V1F1586F1605Audit *aes =
        dm1_v1_f1586_f1605_tos_aes_owner_audit(1592);
    const DM1V1F1586F1605Audit *tos =
        dm1_v1_f1586_f1605_tos_aes_owner_audit(1603);
    const DM1V1F1586F1605Audit *absent =
        dm1_v1_f1586_f1605_tos_aes_owner_audit(1605);

    return aes != 0 &&
                   aes->admission == DM1_V1_F1586_F1605_PLATFORM_FAIL_CLOSED &&
                   tos != 0 &&
                   tos->admission == DM1_V1_F1586_F1605_PLATFORM_FAIL_CLOSED &&
                   absent != 0 &&
                   absent->admission == DM1_V1_F1586_F1605_ABSENT_FAIL_CLOSED &&
                   dm1_v1_f1586_f1605_tos_aes_owner_audit(1606) == 0
               ? 0
               : 1;
}
