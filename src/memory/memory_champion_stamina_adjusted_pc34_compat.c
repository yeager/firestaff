/*
 * memory_champion_stamina_adjusted_pc34_compat.c
 *
 * Source-locked per ReDMCSB CHAMPION.C F0306:1078-1103 + the
 * BUGX_XX comment.  The C expression is
 *
 *   (P0641_i_Value >>= 1) + (P0641_i_Value * curStam / halfMax)
 *
 * C guarantees the side effect of >>= is sequenced before
 * the read of P0641_i_Value in the second operand.  But
 * different compilers schedule the two expressions
 * differently in machine code:
 *
 *   - First-operand compilers (Megamax C, High C, THINK C
 *     4.0):  A is computed first.  A reads (and writes)
 *     P0641_i_Value/2, then B reads the new value.  Result
 *     = (P/2) + (P/2 * curStam / halfMax).
 *
 *   - Second-operand compilers (Aztec C 3.6a, MPW IIGS C,
 *     Turbo C 2.0, THINK C 5.0, Turbo C++ 1.01 = PC 3.4):
 *     A and B are scheduled such that B reads the pre-
 *     decrement P0641_i_Value.  A's >>= still executes, but
 *     B uses the original value.  Result =
 *     (P/2) + (P * curStam / halfMax).
 *
 * v1 default = PC 3.4 (Turbo C++ 1.01) = second-operand-
 * first.  csb_v1_stamina_compiler_order_set(1) flips to
 * first-operand-first for Atari ST / FM-Towns tests.
 */
#include "memory_champion_stamina_adjusted_pc34_compat.h"

static int g_csb_v1_stamina_first_operand_first = 0;

int csb_v1_stamina_compiler_order_get(void) {
    return g_csb_v1_stamina_first_operand_first;
}

void csb_v1_stamina_compiler_order_set(int firstOperandFirst) {
    g_csb_v1_stamina_first_operand_first = firstOperandFirst ? 1 : 0;
}

int F0306_CHAMPION_GetStaminaAdjustedValue_Compat(
    int currentStamina,
    int halfMaxStamina,
    int value)
{
    int halfValue;
    int scaledStam;
    if (currentStamina >= halfMaxStamina) {
        return value;
    }
    if (halfMaxStamina <= 0) {
        return value;
    }
    halfValue = value >> 1;
    if (g_csb_v1_stamina_first_operand_first) {
        /* First-operand order: B uses the halved value. */
        scaledStam = (int)(((long)halfValue * (long)currentStamina)
                          / (long)halfMaxStamina);
    } else {
        /* Second-operand order (PC 3.4 Turbo C++ 1.01): B uses
         * the pre-decrement value. */
        scaledStam = (int)(((long)value * (long)currentStamina)
                          / (long)halfMaxStamina);
    }
    return halfValue + scaledStam;
}
