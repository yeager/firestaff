/* C161 source-media branch regression: ReDMCSB REVIVE.C F0282 compiles
 * original PC, Atari ST, and Amiga/FM Towns vital/statistic variants.  This
 * constructs runtime state only; it does not manufacture game media. */
#include "csb_v1_runtime_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(condition, message) do { \
    if (!(condition)) { fprintf(stderr, "FAIL: %s\n", message); ++failures; } \
} while (0)

static void expected_increments(uint32_t seed, unsigned int out[CSB_V1_STAT_COUNT])
{
    int i;
    memset(out, 0, sizeof(unsigned int) * CSB_V1_STAT_COUNT);
    for (i = 0; i < 12; ++i) {
        uint16_t raw;
        seed = seed * UINT32_C(0xbb40e62d) + UINT32_C(11);
        raw = (uint16_t)(seed >> 8);
        ++out[raw % CSB_V1_STAT_COUNT];
    }
}

static void prepare_profile(CSB_V1_RuntimeProfile *profile,
                            CSB_V1_VariantId variant, uint32_t seed)
{
    CSB_V1_Champion *champion;
    int stat;
    memset(profile, 0, sizeof(*profile));
    profile->variant_id = variant;
    profile->party_state_valid = 1;
    profile->party_state.ChampionCount = 1;
    profile->csbwin_random_seed_valid = 1;
    profile->csbwin_random_seed = seed;
    champion = &profile->party_state.Champions[0];
    champion->CurrentHealth = champion->MaximumHealth = 100;
    champion->CurrentStamina = champion->MaximumStamina = 100;
    champion->CurrentMana = champion->MaximumMana = 100;
    for (stat = 0; stat < CSB_V1_STAT_COUNT; ++stat) {
        champion->Statistics[stat][CSB_V1_STAT_MIN] = 30;
        champion->Statistics[stat][CSB_V1_STAT_CUR] = 80;
        champion->Statistics[stat][CSB_V1_STAT_MAX] = 80;
    }
    memset(champion->Skills, 1, sizeof(champion->Skills));
    memset(champion->SkillExperience, 1, sizeof(champion->SkillExperience));
    memset(champion->SkillTemporaryExperience, 1,
           sizeof(champion->SkillTemporaryExperience));
}

static void check_variant(CSB_V1_VariantId variant, int expected_vitals,
                          int expected_non_luck_base, int expected_luck_base,
                          const char *name)
{
    CSB_V1_RuntimeProfile profile;
    unsigned int increments[CSB_V1_STAT_COUNT];
    const uint32_t seed = UINT32_C(0x13579bdf);
    int stat;

    prepare_profile(&profile, variant, seed);
    expected_increments(seed, increments);
    CHECK(csb_v1_runtime_reincarnate_pending_mirror_candidate_source_compat(
              &profile, 0), name);
    CHECK(profile.party_state.Champions[0].CurrentHealth == expected_vitals &&
              profile.party_state.Champions[0].MaximumHealth == expected_vitals &&
              profile.party_state.Champions[0].CurrentStamina == expected_vitals &&
              profile.party_state.Champions[0].MaximumStamina == expected_vitals &&
              profile.party_state.Champions[0].CurrentMana == expected_vitals &&
              profile.party_state.Champions[0].MaximumMana == expected_vitals,
          "C161 applies the source-package vital divisor");
    for (stat = 0; stat < CSB_V1_STAT_COUNT; ++stat) {
        int base = stat == CSB_V1_STAT_LUCK ? expected_luck_base : expected_non_luck_base;
        CHECK(profile.party_state.Champions[0].Statistics[stat][CSB_V1_STAT_CUR]
                  == (uint16_t)(base + increments[stat]) &&
              profile.party_state.Champions[0].Statistics[stat][CSB_V1_STAT_MAX]
                  == (uint16_t)(base + increments[stat]),
              "C161 retains CHANGE7_24's Luck exception but boosts every F0027-selected stat");
    }
    CHECK(profile.party_state.Champions[0].Skills[0] == 0 &&
              profile.party_state.Champions[0].SkillExperience[0] == 0 &&
              profile.party_state.Champions[0].SkillTemporaryExperience[0] == 0,
          "C161 clears all source-owned skill rows before boosts");
}

int main(void)
{
    check_variant(CSB_V1_VARIANT_REFERENCE_I34_EN, 100, 80, 80,
                  "PC I34 keeps vitals/statistics before boosts");
    check_variant(CSB_V1_VARIANT_ST20_EN, 50, 70, 80,
                  "Atari ST halves vitals and reduces non-Luck stats");
    check_variant(CSB_V1_VARIANT_AMIGA31_EN, 25, 70, 80,
                  "Amiga quarters vitals and reduces non-Luck stats");
    check_variant(CSB_V1_VARIANT_FMTOWNS_JA, 25, 70, 80,
                  "FM Towns quarters vitals and reduces non-Luck stats");
    if (failures) return 1;
    printf("PASS: CSB C161 source-media branch regression\n");
    return 0;
}
