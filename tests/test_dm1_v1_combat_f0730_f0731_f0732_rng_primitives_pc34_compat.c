/*
 * test_dm1_v1_combat_f0730_f0731_f0732_rng_primitives_pc34_compat.c
 *
 * Source-locked to ReDMCSB COMBAT.C F0730_COMBAT_RngInit +
 * F0731_COMBAT_RngNextRaw + F0732_COMBAT_RngRandom.
 *
 * These are the combat-RNG primitives.  F0731 is a linear
 * congruential generator (LCG): seed = seed * 1103515245 + 12345.
 * F0732 advances the LCG and returns (raw >> 16) & 0x7FFF mod n.
 *
 * Pins the F0730/F0731/F0732 contract:
 *  T1  F0730 NULL rng returns 0
 *  T2  F0730 init sets seed
 *  T3  F0730 init returns 1
 *  T4  F0731 NULL rng returns 0
 *  T5  F0731 first call after init with seed=1: returns
 *      1 * 1103515245 + 12345 = 1103527590
 *  T6  F0731 second call: returns
 *      1103527590 * 1103515245 + 12345 mod 2^32
 *  T7  F0731 is deterministic: same seed, same sequence
 *  T8  F0732 NULL rng returns 0
 *  T9  F0732 modulus <= 0 returns 0 (does not advance state)
 *  T10 F0732 first call after init with seed=1, mod 16:
 *      raw = 1103527590
 *      shifted = (1103527590 >> 16) & 0x7FFF = 16841
 *      result = 16841 % 16 = 9
 *  T11 F0732 advances state (raw seed changes after call)
 *  T12 F0732 with mod 1 always returns 0
 *  T13 F0732 with mod 256 returns 0..255
 *  T14 F0732 with large seed (0xDEADBEEF) still works
 *  T15 Multiple F0732 calls produce a range of values
 *  T16 F0732 with modulus 0 does NOT advance state (Invariant 16)
 *
 * Source-locked to ReDMCSB COMBAT.C F0730..F0732.
 */

#include "memory_combat_pc34_compat.h"

#include <stdio.h>
#include <stdint.h>

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, msg); \
        return 1; \
    } \
} while (0)

int main(void) {
    struct RngState_Compat rng;
    uint32_t raw1, raw2, raw3;

    /* T1: F0730 NULL rng returns 0. */
    CHECK(F0730_COMBAT_RngInit_Compat(NULL, 42) == 0,
          "T1: F0730 NULL rng returns 0");

    /* T2-T3: F0730 init. */
    rng.seed = 999;
    CHECK(F0730_COMBAT_RngInit_Compat(&rng, 42) == 1, "T3: F0730 returns 1");
    CHECK(rng.seed == 42, "T2: F0730 sets seed to input");

    /* T4: F0731 NULL rng returns 0. */
    CHECK(F0731_COMBAT_RngNextRaw_Compat(NULL) == 0,
          "T4: F0731 NULL rng returns 0");

    /* T5: F0731 with seed=1 -> 1*1103515245+12345 = 1103527590. */
    rng.seed = 1u;
    raw1 = F0731_COMBAT_RngNextRaw_Compat(&rng);
    CHECK(raw1 == 1103527590u, "T5: seed=1 -> 1103527590");

    /* T6: Second call advances state. */
    raw2 = F0731_COMBAT_RngNextRaw_Compat(&rng);
    {
        uint32_t expected = 1103527590u * 1103515245u + 12345u;
        CHECK(raw2 == expected,
              "T6: second call follows LCG formula");
    }

    /* T7: Deterministic. */
    {
        struct RngState_Compat a, b;
        F0730_COMBAT_RngInit_Compat(&a, 0xCAFEBABEu);
        F0730_COMBAT_RngInit_Compat(&b, 0xCAFEBABEu);
        int k;
        for (k = 0; k < 5; ++k) {
            uint32_t ra = F0731_COMBAT_RngNextRaw_Compat(&a);
            uint32_t rb = F0731_COMBAT_RngNextRaw_Compat(&b);
            CHECK(ra == rb, "T7: deterministic for same seed");
        }
    }

    /* T8: F0732 NULL rng returns 0. */
    CHECK(F0732_COMBAT_RngRandom_Compat(NULL, 16) == 0,
          "T8: F0732 NULL rng returns 0");

    /* T9: F0732 modulus <= 0 returns 0 and does not advance state. */
    rng.seed = 12345u;
    CHECK(F0732_COMBAT_RngRandom_Compat(&rng, 0) == 0, "T9a: F0732 mod 0 returns 0");
    CHECK(rng.seed == 12345u, "T9b: F0732 mod 0 does not advance state");
    CHECK(F0732_COMBAT_RngRandom_Compat(&rng, -5) == 0, "T9c: F0732 mod -5 returns 0");
    CHECK(rng.seed == 12345u, "T9d: F0732 mod -5 does not advance state");

    /* T10: F0732 with seed=1, mod 16. */
    F0730_COMBAT_RngInit_Compat(&rng, 1u);
    {
        int r = F0732_COMBAT_RngRandom_Compat(&rng, 16);
        /* raw = 1 * 1103515245 + 12345 = 1103527590 = 0x41C67EA6
         * shifted = (0x41C67EA6 >> 16) & 0x7FFF = 0x41C6 & 0x7FFF = 16838
         * 16838 % 16 = 6 */
        CHECK(r == 6, "T10: seed=1 mod 16 == 6 (shifted 16838 % 16)");
    }

    /* T11: F0732 advances state. */
    F0730_COMBAT_RngInit_Compat(&rng, 1u);
    uint32_t seedBefore = rng.seed;
    F0732_COMBAT_RngRandom_Compat(&rng, 16);
    CHECK(rng.seed != seedBefore, "T11: F0732 advances state");
    /* After F0732, the seed should equal the LCG-advanced value. */
    CHECK(rng.seed == 1103527590u, "T11: seed advanced to 1103527590");

    /* T12: F0732 mod 1 always returns 0. */
    F0730_COMBAT_RngInit_Compat(&rng, 42u);
    {
        int k;
        for (k = 0; k < 10; ++k) {
            CHECK(F0732_COMBAT_RngRandom_Compat(&rng, 1) == 0,
                  "T12: F0732 mod 1 always returns 0");
        }
    }

    /* T13: F0732 mod 256 returns 0..255. */
    F0730_COMBAT_RngInit_Compat(&rng, 42u);
    {
        int k;
        for (k = 0; k < 100; ++k) {
            int r = F0732_COMBAT_RngRandom_Compat(&rng, 256);
            CHECK(r >= 0 && r < 256, "T13: F0732 mod 256 in [0..255]");
        }
    }

    /* T14: F0732 with large seed still works. */
    F0730_COMBAT_RngInit_Compat(&rng, 0xDEADBEEFu);
    {
        int k;
        for (k = 0; k < 100; ++k) {
            int r = F0732_COMBAT_RngRandom_Compat(&rng, 100);
            CHECK(r >= 0 && r < 100, "T14: F0732 large seed mod 100");
        }
    }

    /* T15: Multiple calls produce a range of values. */
    F0730_COMBAT_RngInit_Compat(&rng, 0xCAFEBABEu);
    {
        int seen[8] = {0};
        int k;
        for (k = 0; k < 1000; ++k) {
            int r = F0732_COMBAT_RngRandom_Compat(&rng, 8);
            if (r >= 0 && r < 8) seen[r] = 1;
        }
        int distinct = 0;
        for (k = 0; k < 8; ++k) distinct += seen[k];
        CHECK(distinct >= 4, "T15: 1000 calls produce multiple distinct values");
    }

    /* T16: F0732 with modulus 0 does NOT advance state. */
    F0730_COMBAT_RngInit_Compat(&rng, 0xFEEDFACEu);
    uint32_t before = rng.seed;
    int r = F0732_COMBAT_RngRandom_Compat(&rng, 0);
    CHECK(r == 0, "T16a: F0732 mod 0 returns 0");
    CHECK(rng.seed == before, "T16b: state unchanged on mod 0");

    /* Use raw to silence "unused" warning. */
    raw3 = F0731_COMBAT_RngNextRaw_Compat(&rng);
    (void)raw3;

    printf("PASS: COMBAT F0730/F0731/F0732 RNG primitive invariants (16 scenarios)\n");
    return 0;
}
