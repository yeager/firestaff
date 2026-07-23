#include "csb_v1_f2406_f2445_unowned_source_audit_pc34_compat.h"

#define NONE(number) { number##u, "no numbered F" #number " body in ReDMCSB callable inventory", "fail_closed: no source owner", 0, 1, 1, 1 }

static const CSB_V1_F2406F2445SourceAuditPc34 k_audit[] = {
    NONE(2406), NONE(2407), NONE(2408), NONE(2409), NONE(2410), NONE(2411), NONE(2412), NONE(2413), NONE(2414), NONE(2415),
    NONE(2416), NONE(2417), NONE(2418), NONE(2419), NONE(2420), NONE(2421), NONE(2422), NONE(2423), NONE(2424), NONE(2425),
    NONE(2426), NONE(2427), NONE(2428), NONE(2429), NONE(2430), NONE(2431), NONE(2432), NONE(2433), NONE(2434), NONE(2435),
    NONE(2436), NONE(2437), NONE(2438), NONE(2439), NONE(2440), NONE(2441), NONE(2442), NONE(2443), NONE(2444), NONE(2445)
};

#undef NONE

const CSB_V1_F2406F2445SourceAuditPc34 *
csb_v1_f2406_f2445_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const CSB_V1_F2406F2445SourceAuditPc34 *
csb_v1_f2406_f2445_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;

    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *csb_v1_f2406_f2445_source_audit_evidence_pc34(void)
{
    return "The ReDMCSB callable inventory has no numbered F2406-F2445 bodies, "
           "and no CSB PC34 owner is present. Every route fails closed without "
           "authenticated PC34 material. This audit does not render or synthesize "
           "UI, graphics, timing, input, audio, save, memory, or file behavior.";
}
