#include "dm1_v1_f1066_f1085_amiga_owner_audit.h"

#include <stddef.h>

/*
 * ReDMCSB AMIGINIT.C F1066-F1085.  F1066's bounded calculation is already
 * source-owned. All remaining routines depend on Amiga Exec/Intuition state
 * and deliberately have no PC34 runtime, UI, graphics, or timing admission.
 */
static const DM1V1F1066F1085Audit kAudit[] = {
    {1066, DM1_V1_F1066_F1085_EXISTING_SOURCE_OWNER},
    {1067, DM1_V1_F1066_F1085_PLATFORM_FAIL_CLOSED},
    {1068, DM1_V1_F1066_F1085_PLATFORM_FAIL_CLOSED},
    {1069, DM1_V1_F1066_F1085_PLATFORM_FAIL_CLOSED},
    {1070, DM1_V1_F1066_F1085_PLATFORM_FAIL_CLOSED},
    {1071, DM1_V1_F1066_F1085_PLATFORM_FAIL_CLOSED},
    {1072, DM1_V1_F1066_F1085_PLATFORM_FAIL_CLOSED},
    {1073, DM1_V1_F1066_F1085_PLATFORM_FAIL_CLOSED},
    {1074, DM1_V1_F1066_F1085_PLATFORM_FAIL_CLOSED},
    {1075, DM1_V1_F1066_F1085_PLATFORM_FAIL_CLOSED},
    {1076, DM1_V1_F1066_F1085_PLATFORM_FAIL_CLOSED},
    {1077, DM1_V1_F1066_F1085_PLATFORM_FAIL_CLOSED},
    {1078, DM1_V1_F1066_F1085_PLATFORM_FAIL_CLOSED},
    {1079, DM1_V1_F1066_F1085_PLATFORM_FAIL_CLOSED},
    {1080, DM1_V1_F1066_F1085_PLATFORM_FAIL_CLOSED},
    {1081, DM1_V1_F1066_F1085_PLATFORM_FAIL_CLOSED},
    {1082, DM1_V1_F1066_F1085_PLATFORM_FAIL_CLOSED},
    {1083, DM1_V1_F1066_F1085_PLATFORM_FAIL_CLOSED},
    {1084, DM1_V1_F1066_F1085_PLATFORM_FAIL_CLOSED},
    {1085, DM1_V1_F1066_F1085_PLATFORM_FAIL_CLOSED}
};

const DM1V1F1066F1085Audit *
dm1_v1_f1066_f1085_amiga_owner_audit(uint16_t routine)
{
    size_t index;

    for (index = 0; index < sizeof(kAudit) / sizeof(kAudit[0]); ++index) {
        if (kAudit[index].routine == routine) {
            return &kAudit[index];
        }
    }
    return NULL;
}
