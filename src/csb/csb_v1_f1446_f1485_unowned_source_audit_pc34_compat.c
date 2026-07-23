#include "csb_v1_f1446_f1485_unowned_source_audit_pc34_compat.h"

#define NONE(number) { number##u, "no numbered F" #number " body in ReDMCSB callable inventory", "fail_closed: no source owner", 0, 1, 1, 1 }

static const CSB_V1_F1446F1485SourceAuditPc34 k_audit[] = {
    NONE(1446), NONE(1447), NONE(1448), NONE(1449), NONE(1450),
    NONE(1451), NONE(1452), NONE(1453), NONE(1454), NONE(1455),
    NONE(1456), NONE(1457), NONE(1458), NONE(1459), NONE(1460),
    NONE(1461), NONE(1462), NONE(1463), NONE(1464), NONE(1465),
    NONE(1466), NONE(1467), NONE(1468), NONE(1469), NONE(1470),
    NONE(1471), NONE(1472), NONE(1473), NONE(1474), NONE(1475),
    NONE(1476), NONE(1477), NONE(1478), NONE(1479), NONE(1480),
    NONE(1481), NONE(1482), NONE(1483), NONE(1484), NONE(1485)
};

#undef NONE

const CSB_V1_F1446F1485SourceAuditPc34 *
csb_v1_f1446_f1485_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const CSB_V1_F1446F1485SourceAuditPc34 *
csb_v1_f1446_f1485_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;

    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *csb_v1_f1446_f1485_source_audit_evidence_pc34(void)
{
    return "The ReDMCSB callable inventory has no numbered F1446-F1485 bodies. "
           "No CSB implementation owner is inferred from local/label numbering. "
           "All routes fail closed without authenticated PC34 material; this audit "
           "does not render or synthesize UI, graphics, timing, input, or files.";
}
