#include "csb_v1_f2486_f2525_unowned_source_audit_pc34_compat.h"

#define NONE(number) { number##u, "no numbered F" #number " body in ReDMCSB callable inventory", "fail_closed: no source owner", 0, 1, 1, 1 }

static const CSB_V1_F2486F2525SourceAuditPc34 k_audit[] = {
    NONE(2486), NONE(2487), NONE(2488), NONE(2489), NONE(2490), NONE(2491), NONE(2492), NONE(2493), NONE(2494), NONE(2495),
    NONE(2496), NONE(2497), NONE(2498), NONE(2499), NONE(2500), NONE(2501), NONE(2502), NONE(2503), NONE(2504), NONE(2505),
    NONE(2506), NONE(2507), NONE(2508), NONE(2509), NONE(2510), NONE(2511), NONE(2512), NONE(2513), NONE(2514), NONE(2515),
    NONE(2516), NONE(2517), NONE(2518), NONE(2519), NONE(2520), NONE(2521), NONE(2522), NONE(2523), NONE(2524), NONE(2525)
};

#undef NONE

const CSB_V1_F2486F2525SourceAuditPc34 *
csb_v1_f2486_f2525_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const CSB_V1_F2486F2525SourceAuditPc34 *
csb_v1_f2486_f2525_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;

    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *csb_v1_f2486_f2525_source_audit_evidence_pc34(void)
{
    return "The ReDMCSB callable inventory has no numbered F2486-F2525 bodies, "
           "and no CSB PC34 owner is present. Every route fails closed without "
           "authenticated PC34 material. This audit does not render or synthesize "
           "UI, graphics, timing, input, audio, save, memory, or file behavior.";
}
