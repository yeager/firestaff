#include "dm1_v1_f1466_f1485_owner_audit.h"

#include <stddef.h>

/* ReDMCSB numbered callable inventory has no F1466-F1485 symbols. */
static const DM1V1F1466F1485Audit kAudit[] = {
    {1466, 1}, {1467, 1}, {1468, 1}, {1469, 1}, {1470, 1},
    {1471, 1}, {1472, 1}, {1473, 1}, {1474, 1}, {1475, 1},
    {1476, 1}, {1477, 1}, {1478, 1}, {1479, 1}, {1480, 1},
    {1481, 1}, {1482, 1}, {1483, 1}, {1484, 1}, {1485, 1}
};

const DM1V1F1466F1485Audit *
dm1_v1_f1466_f1485_owner_audit(uint16_t routine)
{
    size_t index;

    for (index = 0; index < sizeof(kAudit) / sizeof(kAudit[0]); ++index) {
        if (kAudit[index].routine == routine) {
            return &kAudit[index];
        }
    }
    return NULL;
}
