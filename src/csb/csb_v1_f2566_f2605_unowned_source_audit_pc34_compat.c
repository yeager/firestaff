#include "csb_v1_f2566_f2605_unowned_source_audit_pc34_compat.h"

#define NONE(number) { number##u, "no numbered F" #number " body in ReDMCSB callable inventory", "fail_closed: no source owner", 0, 1, 1, 1 }

static const CSB_V1_F2566F2605SourceAuditPc34 k_audit[] = {
    NONE(2566), NONE(2567), NONE(2568), NONE(2569), NONE(2570), NONE(2571), NONE(2572), NONE(2573), NONE(2574), NONE(2575),
    NONE(2576), NONE(2577), NONE(2578), NONE(2579), NONE(2580), NONE(2581), NONE(2582), NONE(2583), NONE(2584), NONE(2585),
    NONE(2586), NONE(2587), NONE(2588), NONE(2589), NONE(2590), NONE(2591), NONE(2592), NONE(2593), NONE(2594), NONE(2595),
    NONE(2596), NONE(2597), NONE(2598), NONE(2599), NONE(2600), NONE(2601), NONE(2602), NONE(2603), NONE(2604), NONE(2605)
};

#undef NONE

const CSB_V1_F2566F2605SourceAuditPc34 *
csb_v1_f2566_f2605_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const CSB_V1_F2566F2605SourceAuditPc34 *
csb_v1_f2566_f2605_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;

    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *csb_v1_f2566_f2605_source_audit_evidence_pc34(void)
{
    return "The ReDMCSB callable inventory has no numbered F2566-F2605 bodies, "
           "and no CSB PC34 owner is present. Every route fails closed without "
           "authenticated PC34 material. This audit does not render or synthesize "
           "UI, graphics, timing, input, audio, save, memory, or file behavior.";
}
