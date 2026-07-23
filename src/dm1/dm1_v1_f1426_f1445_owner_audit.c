#include "dm1_v1_f1426_f1445_owner_audit.h"

#include <stddef.h>

/* ReDMCSB numbered callable inventory has no F1426-F1445 symbols. */
static const DM1V1F1426F1445Audit kAudit[] = {
    {1426, 1}, {1427, 1}, {1428, 1}, {1429, 1}, {1430, 1},
    {1431, 1}, {1432, 1}, {1433, 1}, {1434, 1}, {1435, 1},
    {1436, 1}, {1437, 1}, {1438, 1}, {1439, 1}, {1440, 1},
    {1441, 1}, {1442, 1}, {1443, 1}, {1444, 1}, {1445, 1}
};

const DM1V1F1426F1445Audit *
dm1_v1_f1426_f1445_owner_audit(uint16_t routine)
{
    size_t index;

    for (index = 0; index < sizeof(kAudit) / sizeof(kAudit[0]); ++index) {
        if (kAudit[index].routine == routine) {
            return &kAudit[index];
        }
    }
    return NULL;
}
