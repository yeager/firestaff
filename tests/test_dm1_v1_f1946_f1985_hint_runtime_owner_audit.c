#include "dm1_v1_f1946_f1985_hint_runtime_owner_audit.h"

int main(void)
{
    const DM1V1F1946F1985Audit *bitmap =
        dm1_v1_f1946_f1985_hint_runtime_owner_audit(1947);
    const DM1V1F1946F1985Audit *cpsx =
        dm1_v1_f1946_f1985_hint_runtime_owner_audit(1952);
    const DM1V1F1946F1985Audit *lowercase =
        dm1_v1_f1946_f1985_hint_runtime_owner_audit(1984);

    return bitmap != 0 &&
                   bitmap->admission == DM1_V1_F1946_F1985_UNPROVEN_FAIL_CLOSED &&
                   cpsx != 0 &&
                   cpsx->admission == DM1_V1_F1946_F1985_PLATFORM_FAIL_CLOSED &&
                   lowercase != 0 &&
                   lowercase->admission ==
                       DM1_V1_F1946_F1985_EXISTING_SOURCE_OWNER &&
                   dm1_v1_f1946_f1985_hint_runtime_owner_audit(1986) == 0
               ? 0
               : 1;
}
