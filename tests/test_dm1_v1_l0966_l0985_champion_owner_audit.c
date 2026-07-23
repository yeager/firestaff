#include "dm1_v1_l0966_l0985_champion_owner_audit.h"

int main(void)
{
    const DM1V1L0966L0985Audit *damage =
        dm1_v1_l0966_l0985_champion_owner_audit(967);
    const DM1V1L0966L0985Audit *unpoison =
        dm1_v1_l0966_l0985_champion_owner_audit(982);

    return damage != 0 && damage->routine == 320 &&
                   damage->owner == DM1_V1_L0966_L0985_OWNER_EXISTING_CHAMPION &&
                   unpoison != 0 && unpoison->routine == 323 &&
                   unpoison->owner == DM1_V1_L0966_L0985_OWNER_EXISTING_CHAMPION &&
                   dm1_v1_l0966_l0985_champion_owner_audit(986) == 0
               ? 0
               : 1;
}
