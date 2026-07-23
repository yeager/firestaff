#include "dm1_v1_f2066_f2104_editor_owner_audit.h"

int main(void)
{
    const DM1V1F2066F2104Audit *floppy =
        dm1_v1_f2066_f2104_editor_owner_audit(2079);
    const DM1V1F2066F2104Audit *memory =
        dm1_v1_f2066_f2104_editor_owner_audit(2095);
    const DM1V1F2066F2104Audit *portrait =
        dm1_v1_f2066_f2104_editor_owner_audit(2104);

    return floppy != 0 &&
                   floppy->admission == DM1_V1_F2066_F2104_PLATFORM_FAIL_CLOSED &&
                   memory != 0 &&
                   memory->admission == DM1_V1_F2066_F2104_UNPROVEN_FAIL_CLOSED &&
                   portrait != 0 &&
                   portrait->admission == DM1_V1_F2066_F2104_EXISTING_SOURCE_OWNER &&
                   dm1_v1_f2066_f2104_editor_owner_audit(2105) == 0
               ? 0
               : 1;
}
