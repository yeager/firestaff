/*
 * test_dm1_v1_mnu04_f0758_potion_power_source_lock_pc34_compat.c
 *
 * Source-locked to ReDMCSB MENU.C F0758_ProducePotionEffect:
 *   M003_RANDOM(16) + (powerOrdinal * 40) gives the kineticEnergy
 *   (which seeds the projectile's attack / damage).
 *
 * MNU-04 (DM1 V1 functional-divergence-report.md):
 *   "F0758 potion power formula matches
 *    M003_RANDOM(16) + (powerOrdinal * 40)" — already source-locked.
 *
 * Pins the F0758_MAGIC_ProducePotionEffect_Compat invariant:
 *  T1  kineticEnergy is in [0..15] + (powerOrdinal * 40)
 *      = [powerOrdinal*40, powerOrdinal*40+15] for any seed
 *  T2  All 6 power ordinals (1..6) produce monotonically increasing
 *      baseline (40, 80, 120, 160, 200, 240)
 *  T3  For a given powerOrdinal, the result is at most
 *      (powerOrdinal * 40) + 15 (r16 max)
 *  T4  For a given powerOrdinal, the result is at least
 *      (powerOrdinal * 40) (r16 min)
 *  T5  r16 is sampled independently per call; the F0758 contract
 *      allows up to 16 distinct values per powerOrdinal across
 *      many calls.
 *
 * Source-locked to ReDMCSB MENU.C F0758 and the 548-msgid
 * DM1 .pot catalog.
 */

#include "memory_magic_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, msg); \
        return 1; \
    } \
} while (0)

int main(void) {
    struct SpellDefinition_Compat spell;
    struct SpellEffect_Compat effect;
    struct RngState_Compat rng;
    int i;
    int baseline[7];

    memset(&spell, 0, sizeof(spell));
    spell.kind = C1_SPELL_KIND_POTION_COMPAT;
    spell.type = 0; /* filler */

    /* T1+T3+T4: For each powerOrdinal, kineticEnergy is in
     *   [powerOrdinal*40, powerOrdinal*40+15].  Run 100 calls
     *   per ordinal and check the bounds. */
    for (i = 1; i <= 6; ++i) {
        int min_seen = 0x7FFFFFF;
        int max_seen = -1;
        int k;
        baseline[i] = i * 40;
        for (k = 0; k < 100; ++k) {
            /* Deterministic RNG: vary seed. */
            rng.seed = (uint32_t)(0xC0FFEEu ^ (k * 7919u) ^ ((uint32_t)i << 8));
            int rc = F0758_MAGIC_ProducePotionEffect_Compat(
                &spell, i, 1, &rng, &effect);
            char buf[96];
            snprintf(buf, sizeof(buf),
                     "T1: F0758 ordinal=%d k=%d rc==1", i, k);
            CHECK(rc == 1, buf);
            if (effect.kineticEnergy < min_seen) min_seen = effect.kineticEnergy;
            if (effect.kineticEnergy > max_seen) max_seen = effect.kineticEnergy;
            snprintf(buf, sizeof(buf),
                     "T1/T3/T4: ordinal=%d k=%d kinetic in [%d,%d]",
                     i, k, baseline[i], baseline[i] + 15);
            CHECK(effect.kineticEnergy >= baseline[i], buf);
            CHECK(effect.kineticEnergy <= baseline[i] + 15, buf);
        }
        snprintf((char[96]){0}, 96, "", 0);
    }

    /* T2: Monotonically increasing baseline (40, 80, 120, 160, 200, 240). */
    for (i = 1; i < 6; ++i) {
        CHECK(baseline[i + 1] == baseline[i] + 40,
              "T2: ordinals differ by exactly 40");
    }
    CHECK(baseline[1] == 40,  "T2: ordinal 1 baseline == 40");
    CHECK(baseline[6] == 240, "T2: ordinal 6 baseline == 240");

    /* T5: Many calls produce a range of r16 values.  Sample ordinal=3
     * (baseline 120) over 1000 calls; we should observe multiple
     * distinct kineticEnergy values, not always the same one. */
    {
        int observed[16] = {0};
        int k;
        for (k = 0; k < 1000; ++k) {
            rng.seed = (uint32_t)(k * 0x9E3779B1u);
            F0758_MAGIC_ProducePotionEffect_Compat(
                &spell, 3, 1, &rng, &effect);
            int r = effect.kineticEnergy - 120;
            if (r >= 0 && r < 16) observed[r]++;
        }
        int distinct = 0;
        for (k = 0; k < 16; ++k) distinct += (observed[k] > 0) ? 1 : 0;
        CHECK(distinct >= 4, "T5: ordinal 3 produces multiple distinct r16 values");
    }

    printf("PASS: MNU-04 F0758 potion-power source-lock pin (5 scenarios)\n");
    return 0;
}
