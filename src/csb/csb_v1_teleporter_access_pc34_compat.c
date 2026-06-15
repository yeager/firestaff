/*
 * csb_v1_teleporter_access_pc34_compat.c
 *
 * Source-locked per ReDMCSB DUNGEON.C:1085 + GROUP.C:2090-2150
 * + BUG0_69 fix.  CSB widens teleporter access; v1
 * defaults to DM1 baseline.
 */
#include "csb_v1_teleporter_access_pc34_compat.h"

static int g_csb_v1_teleporter_access_enabled = 0;

int csb_v1_teleporter_access_get(void) {
    return g_csb_v1_teleporter_access_enabled;
}

void csb_v1_teleporter_access_set(int enabled) {
    g_csb_v1_teleporter_access_enabled = enabled ? 1 : 0;
}

int csb_v1_can_creature_use_teleporter(int creatureType) {
    /* DM1 PC 3.4: only Lord-tier creatures (Lord Chaos,
     * Lord Order) can use teleporters.  Some other
     * creatures are explicitly excluded (F0823 dispatch
     * path). */
    if (creatureType == 22 /* Lord Chaos */ ||
        creatureType == 24 /* Lord Order */) {
        return 1;
    }
    /* CSB PC 3.4 widens the list: Grey Lord (26) and
     * Materializer (27) can also use teleporters. */
    if (g_csb_v1_teleporter_access_enabled) {
        if (creatureType == 26 /* Grey Lord */ ||
            creatureType == 27 /* Materializer */) {
            return 1;
        }
    }
    return 0;
}
