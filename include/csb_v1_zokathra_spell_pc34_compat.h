/*
 * csb_v1_zokathra_spell_pc34_compat.h
 *
 * CSB V1 ZOKATHRA Spell Power Variant (Mechanics GAP 3).
 * Source-locked per M13_PLAN.md:337,346, DEFS.H:1774
 * (C7_SPELL_TYPE_OTHER_ZOKATHRA = 7), MENU.C:2014-2023, and
 * Magic.cpp in the decompilation.
 *
 * ZOKATHRA (Zo Kath Ra) is CSB's fireball variant with
 * different kinetic energy vs Ful Ir (the DM1 standard
 * Fireball).  In CSB the spell has a longer burn duration
 * and lower per-tick damage.  The variant-aware power
 * lookup keys on (spell, gameVariant) so DM1 vs CSB yield
 * different values, while Ful Ir remains identical across
 * variants.
 *
 * v1 (2026-06-14): the helper reads from a static
 * variant-aware table.  Default = CSB 2.1 (the source-
 * locked CSB version for the 8-level dungeon).  A flag
 * toggles between CSB and DM1.
 */
#ifndef REDMCSB_CSB_V1_ZOKATHRA_SPELL_PC34_COMPAT_H
#define REDMCSB_CSB_V1_ZOKATHRA_SPELL_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Spell power for ZOKATHRA per variant.  Returns kinetic
 * energy units (1..100) used by the spell power table in
 * memory_magic_pc34_compat.c.  CSB's value is HIGHER than
 * DM1's because ZOKATHRA is more powerful in CSB
 * (Materializer-tier fireball). */
int  csb_v1_zokathra_spell_power(int isCsbVariant);

/* Toggle CSB mode for the helper.  Default = 1 (CSB 2.1). */
void csb_v1_zokathra_mode_set(int isCsbVariant);
int  csb_v1_zokathra_mode_get(void);

#ifdef __cplusplus
}
#endif

#endif /* REDMCSB_CSB_V1_ZOKATHRA_SPELL_PC34_COMPAT_H */
