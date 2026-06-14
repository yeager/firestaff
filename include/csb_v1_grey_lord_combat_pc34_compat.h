/*
 * csb_v1_grey_lord_combat_pc34_compat.h
 *
 * CSB V1 Grey Lord Combat Behavior (Combat GAP 2).
 * Source-locked per ReDMCSB DEFS.H:1679 (Grey Lord =
 * C5_ATTACK_MAGIC source, same category as Lord Chaos),
 * Attack.cpp:2423 (monster type assignment), and the
 * widened IsLordChaosHere() that includes Grey Lord
 * proximity checks.
 *
 * The Grey Lord (0x1a) is the only new creature type in
 * CSB vs DM1 PC 3.4.  It uses the existing C5_ATTACK_MAGIC
 * projectile infrastructure (Lightning Bolt + psionic
 * payload) but with its own attack-byte sequences and a
 * Lord-Chaos-equivalent proximity check (BUG0_69 fix).
 *
 * v1 (2026-06-14): a single toggle flag
 * (csb_v1_grey_lord_aware_get/set) defaults to 1, which
 * makes IsLordChaosHere() also accept Grey Lord cells.
 * For DM1 PC 3.4 the flag is 0 (no Grey Lord at all).
 */
#ifndef REDMCSB_CSB_V1_GREY_LORD_COMBAT_PC34_COMPAT_H
#define REDMCSB_CSB_V1_GREY_LORD_COMBAT_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Returns 1 when IsLordChaosHere() should also accept Grey
 * Lord cells.  Default 1 (CSB default).  Set to 0 for
 * DM1 PC 3.4. */
int  csb_v1_grey_lord_aware_get(void);
void csb_v1_grey_lord_aware_set(int enabled);

/* IsLordChaosHere-equivalent for CSB.  Returns 1 when
 * there is a Lord-Chaos OR a Grey-Lord cell in the supplied
 * cellMask.  Grey Lord awareness is toggled via
 * csb_v1_grey_lord_aware_set(0). */
int csb_v1_is_lord_chaos_or_grey_lord_here(int cellMask,
                                           int hasLordChaosCell,
                                           int hasGreyLordCell);

#ifdef __cplusplus
}
#endif

#endif /* REDMCSB_CSB_V1_GREY_LORD_COMBAT_PC34_COMPAT_H */
