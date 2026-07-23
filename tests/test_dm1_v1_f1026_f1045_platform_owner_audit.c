#include "dm1_v1_f1026_f1045_platform_owner_audit.h"

int main(void)
{
    const DM1V1F1026F1045Audit *source =
        dm1_v1_f1026_f1045_platform_owner_audit(1031);
    const DM1V1F1026F1045Audit *platform =
        dm1_v1_f1026_f1045_platform_owner_audit(1026);
    const DM1V1F1026F1045Audit *unknown =
        dm1_v1_f1026_f1045_platform_owner_audit(1045);

    return source != 0 &&
                   source->admission == DM1_V1_F1026_F1045_EXISTING_SOURCE_OWNER &&
                   platform != 0 &&
                   platform->admission == DM1_V1_F1026_F1045_PLATFORM_FAIL_CLOSED &&
                   unknown != 0 &&
                   unknown->admission == DM1_V1_F1026_F1045_ABSENT_FAIL_CLOSED &&
                   dm1_v1_f1026_f1045_platform_owner_audit(1046) == 0
               ? 0
               : 1;
}
