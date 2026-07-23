#include "dm1_v1_f1146_f1165_io_owner_audit.h"

#include <stddef.h>

/*
 * ReDMCSB COPYPROE.C, AMIGINIT.C and USIO*.C F1146-F1165. Existing shared
 * platform boundaries retain their own source evidence, but none supplies an
 * authenticated PC34 runtime input for DM1; no independent route is admitted.
 */
static const DM1V1F1146F1165Audit kAudit[] = {
    {1146, 1}, {1147, 1}, {1148, 1}, {1149, 1}, {1150, 1},
    {1151, 1}, {1152, 1}, {1153, 1}, {1154, 1}, {1155, 1},
    {1156, 1}, {1157, 1}, {1158, 1}, {1159, 1}, {1160, 1},
    {1161, 1}, {1162, 1}, {1163, 1}, {1164, 1}, {1165, 1}
};

const DM1V1F1146F1165Audit *
dm1_v1_f1146_f1165_io_owner_audit(uint16_t routine)
{
    size_t index;

    for (index = 0; index < sizeof(kAudit) / sizeof(kAudit[0]); ++index) {
        if (kAudit[index].routine == routine) {
            return &kAudit[index];
        }
    }
    return NULL;
}
