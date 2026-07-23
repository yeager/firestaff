#include "dm1_v1_f1106_f1125_media_owner_audit.h"

int main(void)
{
    const DM1V1F1106F1125Audit *trackdisk =
        dm1_v1_f1106_f1125_media_owner_audit(1106);
    const DM1V1F1106F1125Audit *palette =
        dm1_v1_f1106_f1125_media_owner_audit(1122);

    return trackdisk != 0 &&
                   trackdisk->admission == DM1_V1_F1106_F1125_FAIL_CLOSED &&
                   palette != 0 &&
                   palette->admission == DM1_V1_F1106_F1125_EXISTING_SOURCE_OWNER &&
                   dm1_v1_f1106_f1125_media_owner_audit(1126) == 0
               ? 0
               : 1;
}
