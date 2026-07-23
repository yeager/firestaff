#include "dm1_v1_f1246_f1265_owner_audit.h"

#include <stddef.h>

/*
 * ReDMCSB ANIM.C/ANIMSND.C/CNFG.C F1246-F1265. F1253 is only a reference
 * in the existing creature renderer, not an authenticated PC34 sound owner.
 * Master-disk, memory and prerequisite routes have no portable PC34 contract.
 */
static const DM1V1F1246F1265Audit kAudit[] = {
    {1246, 1}, {1247, 1}, {1248, 1}, {1249, 1}, {1250, 1},
    {1251, 1}, {1252, 1}, {1253, 1}, {1254, 1}, {1255, 1},
    {1256, 1}, {1257, 1}, {1258, 1}, {1259, 1}, {1260, 1},
    {1261, 1}, {1262, 1}, {1263, 1}, {1264, 1}, {1265, 1}
};

const DM1V1F1246F1265Audit *
dm1_v1_f1246_f1265_owner_audit(uint16_t routine)
{
    size_t index;

    for (index = 0; index < sizeof(kAudit) / sizeof(kAudit[0]); ++index) {
        if (kAudit[index].routine == routine) {
            return &kAudit[index];
        }
    }
    return NULL;
}
