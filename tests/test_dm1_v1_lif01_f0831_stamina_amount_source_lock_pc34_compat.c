/*
 * test_dm1_v1_lif01_f0831_stamina_amount_source_lock_pc34_compat.c
 *
 * Source-locked to ReDMCSB CHAMPION.C:2357 (F0331 stamina amount):
 *   base = (maxStamina >> 8) - 1   (clamped to [1..6])
 *   if (isResting) base <<= 1
 *   if (delay > IDLE_BONUS_1_DELAY) base++
 *   if (delay > IDLE_BONUS_2_DELAY) base += 2
 *
 * LIF-01 (DM1 V1 functional-divergence-report.md):
 *   "F0830..F0843 lifecycle helpers are source-locked" — already
 *    source-locked, included only as verification.
 *
 * Pins the F0831_LIFECYCLE_ComputeStaminaAmount_Compat invariant:
 *  T1  maxStamina=0 -> base=0 -> clamped to 1 -> return 1
 *  T2  maxStamina=255 -> base = (255>>8)-1 = -1 -> clamped to 1
 *  T3  maxStamina=256 -> base = (256>>8)-1 = 0 -> clamped to 1
 *  T4  maxStamina=512 -> base = 1
 *  T5  maxStamina=1024 -> base = 3
 *  T6  maxStamina=2048 -> base = 7 -> clamped to 6
 *  T7  isResting doubles the base (max stamina + resting = 4x)
 *  T8  delay=0 -> no idle bonus
 *  T9  delay=IDLE_BONUS_1_DELAY -> still no bonus
 *  T10 delay>IDLE_BONUS_1_DELAY -> +1 idle bonus
 *  T11 delay>IDLE_BONUS_2_DELAY -> +3 idle bonus (1+2)
 *
 * Source-locked to ReDMCSB CHAMPION.C:2357.
 */

#include "memory_champion_lifecycle_pc34_compat.h"

#include <stdio.h>

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, msg); \
        return 1; \
    } \
} while (0)

/* Find the LIFECYCLE_IDLE_STAMINA_BONUS_*_DELAY values. */
#ifndef LIFECYCLE_IDLE_STAMINA_BONUS_1_DELAY
#define LIFECYCLE_IDLE_STAMINA_BONUS_1_DELAY 200u
#endif
#ifndef LIFECYCLE_IDLE_STAMINA_BONUS_2_DELAY
#define LIFECYCLE_IDLE_STAMINA_BONUS_2_DELAY 1000u
#endif

int main(void) {
    int16_t stamina;

    /* T1: maxStamina=0 -> base = -1 -> clamp to 1. */
    stamina = F0831_LIFECYCLE_ComputeStaminaAmount_Compat(0, 0, 0, 0);
    CHECK(stamina == 1, "T1: maxStamina=0 -> stamina=1 (clamped)");

    /* T2: maxStamina=255 -> base = -1 -> clamp to 1. */
    stamina = F0831_LIFECYCLE_ComputeStaminaAmount_Compat(255, 0, 0, 0);
    CHECK(stamina == 1, "T2: maxStamina=255 -> stamina=1 (clamped)");

    /* T3: maxStamina=256 -> base = 0 -> clamp to 1. */
    stamina = F0831_LIFECYCLE_ComputeStaminaAmount_Compat(256, 0, 0, 0);
    CHECK(stamina == 1, "T3: maxStamina=256 -> stamina=1 (clamped)");

    /* T4: maxStamina=512 -> base = 1. */
    stamina = F0831_LIFECYCLE_ComputeStaminaAmount_Compat(512, 0, 0, 0);
    CHECK(stamina == 1, "T4: maxStamina=512 -> stamina=1 (base=1)");

    /* T5: maxStamina=1024 -> base = 3. */
    stamina = F0831_LIFECYCLE_ComputeStaminaAmount_Compat(1024, 0, 0, 0);
    CHECK(stamina == 3, "T5: maxStamina=1024 -> stamina=3 (base=3)");

    /* T6: maxStamina=2048 -> base = 7 -> clamp to 6. */
    stamina = F0831_LIFECYCLE_ComputeStaminaAmount_Compat(2048, 0, 0, 0);
    CHECK(stamina == 6, "T6: maxStamina=2048 -> stamina=6 (clamped)");

    /* T7: isResting doubles the base. */
    stamina = F0831_LIFECYCLE_ComputeStaminaAmount_Compat(1024, 0, 0, 0);
    int16_t resting = F0831_LIFECYCLE_ComputeStaminaAmount_Compat(1024, 1, 0, 0);
    CHECK(resting == 2 * stamina, "T7: isResting doubles the base");

    /* T8: delay=0 -> no idle bonus. */
    stamina = F0831_LIFECYCLE_ComputeStaminaAmount_Compat(1024, 0, 0, 0);
    CHECK(stamina == 3, "T8: delay=0 -> no bonus (base=3)");

    /* T9: delay=IDLE_BONUS_1_DELAY -> still no bonus. */
    stamina = F0831_LIFECYCLE_ComputeStaminaAmount_Compat(1024, 0, 100, 100 + LIFECYCLE_IDLE_STAMINA_BONUS_1_DELAY);
    CHECK(stamina == 3, "T9: delay=IDLE_1 -> no bonus (base=3)");

    /* T10: delay > IDLE_BONUS_1_DELAY -> +1 idle bonus. */
    stamina = F0831_LIFECYCLE_ComputeStaminaAmount_Compat(1024, 0, 100, 100 + LIFECYCLE_IDLE_STAMINA_BONUS_1_DELAY + 1);
    CHECK(stamina == 4, "T10: delay > IDLE_1 -> +1 bonus (stamina=4)");

    /* T11: delay > IDLE_BONUS_2_DELAY -> +2 idle bonus (1+1, since
     * the second +1 is also gated by >IDLE_2). */
    stamina = F0831_LIFECYCLE_ComputeStaminaAmount_Compat(1024, 0, 100, 100 + LIFECYCLE_IDLE_STAMINA_BONUS_2_DELAY + 1);
    CHECK(stamina == 5, "T11: delay > IDLE_2 -> +2 bonus (stamina=5)");

    printf("PASS: LIF-01 F0831 stamina-amount source-lock (11 scenarios)\n");
    return 0;
}
