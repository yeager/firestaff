#include "csb_v1_f2326_f2365_unowned_source_audit_pc34_compat.h"

#define NONE(number) { number##u, "no numbered F" #number " body in ReDMCSB callable inventory", "fail_closed: no source owner", 0, 1, 1, 1 }

static const CSB_V1_F2326F2365SourceAuditPc34 k_audit[] = {
    NONE(2326), NONE(2327), NONE(2328), NONE(2329), NONE(2330), NONE(2331), NONE(2332), NONE(2333), NONE(2334), NONE(2335),
    NONE(2336), NONE(2337), NONE(2338), NONE(2339), NONE(2340), NONE(2341), NONE(2342), NONE(2343), NONE(2344), NONE(2345),
    NONE(2346), NONE(2347), NONE(2348), NONE(2349), NONE(2350), NONE(2351), NONE(2352), NONE(2353), NONE(2354), NONE(2355),
    NONE(2356), NONE(2357), NONE(2358), NONE(2359), NONE(2360), NONE(2361), NONE(2362), NONE(2363), NONE(2364), NONE(2365)
};

#undef NONE

const CSB_V1_F2326F2365SourceAuditPc34 *
csb_v1_f2326_f2365_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const CSB_V1_F2326F2365SourceAuditPc34 *
csb_v1_f2326_f2365_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;

    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *csb_v1_f2326_f2365_source_audit_evidence_pc34(void)
{
    return "The ReDMCSB callable inventory has no numbered F2326-F2365 bodies, "
           "and no CSB PC34 owner is present. Every route fails closed without "
           "authenticated PC34 material. This audit does not render or synthesize "
           "UI, graphics, timing, input, audio, save, memory, or file behavior.";
}
