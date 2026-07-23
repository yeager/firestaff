#include "dm1_v1_f1786_f1825_animation_owner_audit.h"

int main(void)
{
    const DM1V1F1786F1825Audit *sound =
        dm1_v1_f1786_f1825_animation_owner_audit(1788);
    const DM1V1F1786F1825Audit *animation =
        dm1_v1_f1786_f1825_animation_owner_audit(1799);
    const DM1V1F1786F1825Audit *absent =
        dm1_v1_f1786_f1825_animation_owner_audit(1803);

    return sound != 0 &&
                   sound->admission == DM1_V1_F1786_F1825_REFERENCE_FAIL_CLOSED &&
                   animation != 0 &&
                   animation->admission ==
                       DM1_V1_F1786_F1825_REFERENCE_FAIL_CLOSED &&
                   absent != 0 &&
                   absent->admission == DM1_V1_F1786_F1825_ABSENT_FAIL_CLOSED &&
                   dm1_v1_f1786_f1825_animation_owner_audit(1826) == 0
               ? 0
               : 1;
}
