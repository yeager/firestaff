/*
 * memory_champion_stamina_adjusted_pc34_compat.h
 *
 * F0306_CHAMPION_GetStaminaAdjustedValue (CHAMPION.C:1078-1103).
 * Source-locked per ReDMCSB CHAMPION.C F0306:1078-1103 + the
 * BUGX_XX comment.  When a champion's stamina drops below half
 * of its maximum, the Strength and Maximum Load values are
 * reduced.  The exact value depends on the compiler's operand
 * evaluation order:
 *
 *   - Megamax C (DM 1.x/CSB 2.x Atari ST), High C
 *     (DM 2.0/CSB 3.1 FM-Towns), THINK C 4.0 (CSB 3.x Amiga,
 *     X68000; DM 3.0 X68000): first-operand order.
 *   - Aztec C 3.6a (DM 2.x Amiga), MPW IIGS C (DM 2.x
 *     Apple IIGS), Turbo C 2.0 (DM 2.0/CSB 3.1 PC-98),
 *     THINK C 5.0 (DM 3.6 Amiga): second-operand order.
 *   - Turbo C++ 1.01 (DM 3.4 PC — what Firestaff ships):
 *     second-operand order.  A champion with maxStamina=100
 *     carrying 40 KG (>= half) drops to 38 KG when current
 *     stamina drops to 45 (instead of 29 in first-operand
 *     order).
 *
 * v1 (2026-06-14): implements the PC 3.4 (Turbo C++ 1.01)
 * behaviour — second operand first.  Configurable via
 * csb_v1_stamina_compiler_order_get/set for tests.
 */
#ifndef REDMCSB_MEMORY_CHAMPION_STAMINA_ADJUSTED_PC34_COMPAT_H
#define REDMCSB_MEMORY_CHAMPION_STAMINA_ADJUSTED_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Configuration: which operand does the compiler evaluate
 * first?  0 = second first (Turbo C++ 1.01, PC 3.4, default);
 * 1 = first first (Megamax C, High C, THINK C 4.0).  Default
 * 0 matches Firestaff's DM 3.4 PC 3.4 target. */
int csb_v1_stamina_compiler_order_get(void);
void csb_v1_stamina_compiler_order_set(int firstOperandFirst);

/* F0306_CHAMPION_GetStaminaAdjustedValue.  Returns the
 * stamina-adjusted value (strength or maximum load) per the
 * source-locked formula:
 *
 *   if (curStam >= halfMaxStam) return value;
 *   tmp = (value * curStam) / halfMaxStam;
 *   if (curStam <= halfMaxStam) {
 *     return (value >>= 1) + tmp;
 *     // BUGX_XX: the second operand (tmp) is computed before
 *     // the first (value >>= 1) under Turbo C++ 1.01.
 *   }
 *
 * The order-toggle is exposed via
 * csb_v1_stamina_compiler_order_get/set so tests can verify
 * both orderings match the source. */
int F0306_CHAMPION_GetStaminaAdjustedValue_Compat(
    int currentStamina,
    int halfMaxStamina,
    int value);

#ifdef __cplusplus
}
#endif

#endif /* REDMCSB_MEMORY_CHAMPION_STAMINA_ADJUSTED_PC34_COMPAT_H */
