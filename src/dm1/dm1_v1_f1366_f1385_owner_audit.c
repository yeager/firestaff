#include "dm1_v1_f1366_f1385_owner_audit.h"

#include <stddef.h>

/*
 * ReDMCSB SWSH.C/VBLANK.C F1366-F1385. No authenticated PC34 title bitmap,
 * palette, sound, or VBlank ownership exists. The F1372 token is reference
 * evidence only and must not admit synthetic scheduling or display output.
 */
static const DM1V1F1366F1385Audit kAudit[] = {
    {1366, 1}, {1367, 1}, {1368, 1}, {1369, 1}, {1370, 1},
    {1371, 1}, {1372, 1}, {1373, 1}, {1374, 1}, {1375, 1},
    {1376, 1}, {1377, 1}, {1378, 1}, {1379, 1}, {1380, 1},
    {1381, 1}, {1382, 1}, {1383, 1}, {1384, 1}, {1385, 1}
};

const DM1V1F1366F1385Audit *
dm1_v1_f1366_f1385_owner_audit(uint16_t routine)
{
    size_t index;

    for (index = 0; index < sizeof(kAudit) / sizeof(kAudit[0]); ++index) {
        if (kAudit[index].routine == routine) {
            return &kAudit[index];
        }
    }
    return NULL;
}
