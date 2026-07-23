#include "dm1_v1_f1066_f1085_amiga_owner_audit.h"

int main(void)
{
    const DM1V1F1066F1085Audit *memory =
        dm1_v1_f1066_f1085_amiga_owner_audit(1066);
    const DM1V1F1066F1085Audit *init =
        dm1_v1_f1066_f1085_amiga_owner_audit(1067);
    const DM1V1F1066F1085Audit *vectors =
        dm1_v1_f1066_f1085_amiga_owner_audit(1085);

    return memory != 0 &&
                   memory->admission == DM1_V1_F1066_F1085_EXISTING_SOURCE_OWNER &&
                   init != 0 &&
                   init->admission == DM1_V1_F1066_F1085_PLATFORM_FAIL_CLOSED &&
                   vectors != 0 &&
                   vectors->admission == DM1_V1_F1066_F1085_PLATFORM_FAIL_CLOSED &&
                   dm1_v1_f1066_f1085_amiga_owner_audit(1086) == 0
               ? 0
               : 1;
}
