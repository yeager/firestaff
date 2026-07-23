#include "dm1_v1_f1726_f1745_owner_audit.h"

#include <stddef.h>

/* ReDMCSB numbered callable inventory has no F1726-F1745 symbols. */
static const DM1V1F1726F1745Audit kAudit[] = {
    {1726, 1}, {1727, 1}, {1728, 1}, {1729, 1}, {1730, 1},
    {1731, 1}, {1732, 1}, {1733, 1}, {1734, 1}, {1735, 1},
    {1736, 1}, {1737, 1}, {1738, 1}, {1739, 1}, {1740, 1},
    {1741, 1}, {1742, 1}, {1743, 1}, {1744, 1}, {1745, 1}
};

const DM1V1F1726F1745Audit *
dm1_v1_f1726_f1745_owner_audit(uint16_t routine)
{
    size_t index;

    for (index = 0; index < sizeof(kAudit) / sizeof(kAudit[0]); ++index) {
        if (kAudit[index].routine == routine) {
            return &kAudit[index];
        }
    }
    return NULL;
}
