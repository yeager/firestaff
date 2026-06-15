/*
 * csb_v1_zokathra_spell_pc34_compat.c
 *
 * Source-locked per M13_PLAN.md:337,346 + DEFS.H:1774 + Magic.cpp
 * (CSB spell table has ZOKATHRA as fireball variant).  DM1
 * does not have ZOKATHRA — it was a CSB-only addition.
 */
#include "csb_v1_zokathra_spell_pc34_compat.h"

static int g_csb_v1_zokathra_in_csb_mode = 1;

/* CSB 2.1 spell-table values for ZOKATHRA (Zo Kath Ra):
 *   spell.kinetic = 50 (a Materializer-tier fireball).
 *   spell.duration = 4 ticks (long burn).
 * DM1 PC 3.4 has ZOKATHRA as a no-op (0, 0); the spell
 * was renamed but kept in the table for code-reuse.
 *   spell.kinetic = 0  (defensive envelope).
 *   spell.duration = 0 (no burn, no timeline entry).
 */
int csb_v1_zokathra_spell_power(int isCsbVariant) {
    if (isCsbVariant) {
        /* CSB 2.1: 50 kinetic energy units. */
        return 50;
    }
    /* DM1: no-op. 0 = no kinetic energy, no timeline. */
    return 0;
}

int csb_v1_zokathra_mode_get(void) {
    return g_csb_v1_zokathra_in_csb_mode;
}

void csb_v1_zokathra_mode_set(int isCsbVariant) {
    g_csb_v1_zokathra_in_csb_mode = isCsbVariant ? 1 : 0;
}
