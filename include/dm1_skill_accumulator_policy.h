#ifndef DM1_SKILL_ACCUMULATOR_POLICY_H
#define DM1_SKILL_ACCUMULATOR_POLICY_H

#include <stdint.h>

/* ReDMCSB CHAMPION.C F0303:730-738 and DEFS.H:608-621.
 * Policy describes source arithmetic, not a rendering/preservation option. */
enum DM1SkillAccumulatorPolicy {
    DM1_SKILL_ACCUMULATOR_SIGNED_EARLY = 0,
    DM1_SKILL_ACCUMULATOR_UNSIGNED_LATE = 1
};

static inline uint32_t dm1_skill_temporary_bits(uint16_t word,
    enum DM1SkillAccumulatorPolicy policy)
{
    if (policy == DM1_SKILL_ACCUMULATOR_UNSIGNED_LATE || word < 0x8000u)
        return word;
    return UINT32_C(0xffff0000) | word;
}

/* Each source assignment wraps before hidden/base averaging. Avoid host
 * signed overflow and implementation-defined right shift of negatives. */
static inline int dm1_skill_accumulator_level(uint32_t experience,
    uint16_t temporary, uint32_t baseExperience, uint16_t baseTemporary,
    int hidden, int ignoreTemporary, enum DM1SkillAccumulatorPolicy policy)
{
    uint32_t sum = experience;
    int level = 1;
    if (!ignoreTemporary) sum += dm1_skill_temporary_bits(temporary, policy);
    if (hidden) {
        sum += baseExperience;
        if (!ignoreTemporary) sum += dm1_skill_temporary_bits(baseTemporary, policy);
        sum = (sum >> 1) |
            (policy == DM1_SKILL_ACCUMULATOR_SIGNED_EARLY
                ? (sum & UINT32_C(0x80000000)) : 0u);
    }
    if (policy == DM1_SKILL_ACCUMULATOR_SIGNED_EARLY &&
        (sum & UINT32_C(0x80000000))) return 1;
    while (sum >= 500u) { sum >>= 1; ++level; }
    return level;
}

/* F0304:887/893: preserve the 32-bit stored XP pattern, never saturate. */
static inline int32_t dm1_skill_add_permanent_bits(int32_t experience, uint16_t award)
{
    uint32_t sum = (uint32_t)experience + (uint32_t)award;
    return sum <= INT32_MAX ? (int32_t)sum
        : (int32_t)((int64_t)sum - INT64_C(4294967296));
}

#endif
