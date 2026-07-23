#include "dm1_v1_f1306_f1325_fio_owner_audit.h"

#include <stddef.h>

/* ReDMCSB FIO1MAIN.C/FIO1.C. Only caller-owned F1321/F1323 contracts exist. */
static const DM1V1F1306F1325Audit kAudit[] = {
    {1306, DM1_V1_F1306_F1325_FAIL_CLOSED},
    {1307, DM1_V1_F1306_F1325_FAIL_CLOSED},
    {1308, DM1_V1_F1306_F1325_FAIL_CLOSED},
    {1309, DM1_V1_F1306_F1325_FAIL_CLOSED},
    {1310, DM1_V1_F1306_F1325_FAIL_CLOSED},
    {1311, DM1_V1_F1306_F1325_FAIL_CLOSED},
    {1312, DM1_V1_F1306_F1325_FAIL_CLOSED},
    {1313, DM1_V1_F1306_F1325_FAIL_CLOSED},
    {1314, DM1_V1_F1306_F1325_FAIL_CLOSED},
    {1315, DM1_V1_F1306_F1325_FAIL_CLOSED},
    {1316, DM1_V1_F1306_F1325_FAIL_CLOSED},
    {1317, DM1_V1_F1306_F1325_FAIL_CLOSED},
    {1318, DM1_V1_F1306_F1325_FAIL_CLOSED},
    {1319, DM1_V1_F1306_F1325_FAIL_CLOSED},
    {1320, DM1_V1_F1306_F1325_FAIL_CLOSED},
    {1321, DM1_V1_F1306_F1325_EXISTING_SOURCE_OWNER},
    {1322, DM1_V1_F1306_F1325_FAIL_CLOSED},
    {1323, DM1_V1_F1306_F1325_EXISTING_SOURCE_OWNER},
    {1324, DM1_V1_F1306_F1325_FAIL_CLOSED},
    {1325, DM1_V1_F1306_F1325_FAIL_CLOSED}
};

const DM1V1F1306F1325Audit *
dm1_v1_f1306_f1325_fio_owner_audit(uint16_t routine)
{
    size_t index;

    for (index = 0; index < sizeof(kAudit) / sizeof(kAudit[0]); ++index) {
        if (kAudit[index].routine == routine) {
            return &kAudit[index];
        }
    }
    return NULL;
}
