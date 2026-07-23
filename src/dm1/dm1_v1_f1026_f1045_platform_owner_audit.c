#include "dm1_v1_f1026_f1045_platform_owner_audit.h"

#include <stddef.h>

/*
 * ReDMCSB audit: F1026-F1045.  Existing source owners remain authoritative;
 * platform, missing, and unverified routes deliberately receive no host ABI.
 */
static const DM1V1F1026F1045Audit kAudit[] = {
    {1026, DM1_V1_F1026_F1045_PLATFORM_FAIL_CLOSED},
    {1027, DM1_V1_F1026_F1045_PLATFORM_FAIL_CLOSED},
    {1028, DM1_V1_F1026_F1045_UNPROVEN_FAIL_CLOSED},
    {1029, DM1_V1_F1026_F1045_UNPROVEN_FAIL_CLOSED},
    {1030, DM1_V1_F1026_F1045_EXISTING_SOURCE_OWNER},
    {1031, DM1_V1_F1026_F1045_EXISTING_SOURCE_OWNER},
    {1032, DM1_V1_F1026_F1045_PLATFORM_FAIL_CLOSED},
    {1033, DM1_V1_F1026_F1045_EXISTING_SOURCE_OWNER},
    {1034, DM1_V1_F1026_F1045_PLATFORM_FAIL_CLOSED},
    {1035, DM1_V1_F1026_F1045_PLATFORM_FAIL_CLOSED},
    {1036, DM1_V1_F1026_F1045_PLATFORM_FAIL_CLOSED},
    {1037, DM1_V1_F1026_F1045_PLATFORM_FAIL_CLOSED},
    {1038, DM1_V1_F1026_F1045_PLATFORM_FAIL_CLOSED},
    {1039, DM1_V1_F1026_F1045_ABSENT_FAIL_CLOSED},
    {1040, DM1_V1_F1026_F1045_UNPROVEN_FAIL_CLOSED},
    {1041, DM1_V1_F1026_F1045_PLATFORM_FAIL_CLOSED},
    {1042, DM1_V1_F1026_F1045_PLATFORM_FAIL_CLOSED},
    {1043, DM1_V1_F1026_F1045_PLATFORM_FAIL_CLOSED},
    {1044, DM1_V1_F1026_F1045_ABSENT_FAIL_CLOSED},
    {1045, DM1_V1_F1026_F1045_ABSENT_FAIL_CLOSED}
};

const DM1V1F1026F1045Audit *
dm1_v1_f1026_f1045_platform_owner_audit(uint16_t routine)
{
    size_t index;

    for (index = 0; index < sizeof(kAudit) / sizeof(kAudit[0]); ++index) {
        if (kAudit[index].routine == routine) {
            return &kAudit[index];
        }
    }
    return NULL;
}
