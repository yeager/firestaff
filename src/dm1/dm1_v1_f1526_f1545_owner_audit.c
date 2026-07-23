#include "dm1_v1_f1526_f1545_owner_audit.h"

#include <stddef.h>

/*
 * ReDMCSB UTSTWKS/IO/UTSTAES F1526-F1545. Existing numbered references are
 * not behavioral receipts; the AES trap ABI has no PC34 contract.
 */
static const DM1V1F1526F1545Audit kAudit[] = {
    {1526, 1}, {1527, 1}, {1528, 1}, {1529, 1}, {1530, 1},
    {1531, 1}, {1532, 1}, {1533, 1}, {1534, 1}, {1535, 1},
    {1536, 1}, {1537, 1}, {1538, 1}, {1539, 1}, {1540, 1},
    {1541, 1}, {1542, 1}, {1543, 1}, {1544, 1}, {1545, 1}
};

const DM1V1F1526F1545Audit *
dm1_v1_f1526_f1545_owner_audit(uint16_t routine)
{
    size_t index;

    for (index = 0; index < sizeof(kAudit) / sizeof(kAudit[0]); ++index) {
        if (kAudit[index].routine == routine) {
            return &kAudit[index];
        }
    }
    return NULL;
}
