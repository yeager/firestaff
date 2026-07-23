#include "dm1_v1_f1406_f1425_unmapped_source_audit_pc34_compat.h"

static const DM1_V1_F1406F1425SourceAuditPc34 k_audit[] = {
    { 1406u, "no F1406 symbol in audited ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1407u, "no F1407 symbol in audited ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1408u, "no F1408 symbol in audited ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1409u, "no F1409 symbol in audited ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1410u, "no F1410 symbol in audited ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1411u, "no F1411 symbol in audited ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1412u, "no F1412 symbol in audited ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1413u, "no F1413 symbol in audited ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1414u, "no F1414 symbol in audited ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1415u, "no F1415 symbol in audited ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1416u, "no F1416 symbol in audited ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1417u, "no F1417 symbol in audited ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1418u, "no F1418 symbol in audited ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1419u, "no F1419 symbol in audited ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1420u, "no F1420 symbol in audited ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1421u, "no F1421 symbol in audited ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1422u, "no F1422 symbol in audited ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1423u, "no F1423 symbol in audited ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1424u, "no F1424 symbol in audited ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1425u, "no F1425 symbol in audited ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 }
};

const DM1_V1_F1406F1425SourceAuditPc34 *
dm1_v1_f1406_f1425_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_F1406F1425SourceAuditPc34 *
dm1_v1_f1406_f1425_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_f1406_f1425_source_audit_evidence_pc34(void)
{
    return "The audited ReDMCSB corpus contains no F1406-F1425 symbols or "
           "source bodies. No authentic PC34 owner or material exists for this "
           "interval, so every row remains fail closed. The audit does not render "
           "or synthesize UI or timing paths.";
}
