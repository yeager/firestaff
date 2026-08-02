
#include "nexus_v1_creature_names.h"
#include <stddef.h>

static const char* g_mns_names[NEXUS_CREATURE_NAME_COUNT] = {
    "ANTMAN.MNS",    /*  0 */
    "MUMMY.MNS",     /*  1 */
    "GHOST.MNS",     /*  2 */
    "VEXIRK.MNS",    /*  3 */
    "GIGGLER.MNS",   /*  4 */
    "GOLEM.MNS",     /*  5 */
    "H_HOUND.MNS",   /*  6 */
    "D_GOLD.MNS",    /*  7 */
    "D_SILVER.MNS",  /*  8 */
    "D_RED.MNS",     /*  9 */
    "LAS_MON.MNS",   /* 10 */
    "OITU.MNS",      /* 11 */
    "RAT.MNS",       /* 12 */
    "ROCKPILE.MNS",  /* 13 */
    "SCREAMER.MNS",  /* 14 */
    "SCORPION.MNS",  /* 15 */
    "SN_FLOOR.MNS",  /* 16 — material file, not a creature */
    "SN_WALL.MNS",   /* 17 — material file, not a creature */
    "S_SWORD.MNS",   /* 18 */
    "S_SHIELD.MNS",  /* 19 */
    "WORM.MNS",      /* 20 */
    "GRN_DRA.MNS",   /* 21 */
    "RED_DRA.MNS",   /* 22 */
    "DRA_ZOM.MNS",   /* 23 */
    "MINI_DRA.MNS",  /* 24 */
    "BORKETH.MNS",   /* 25 */
    "CHAOS.MNS",     /* 26 */
    "LORD_RIB.MNS",  /* 27 */
    "BIGWORM.MNS",   /* 28 */
    "OBAKE.MNS",     /* 29 */
};

const char* nexus_v1_creature_mns_name(int cret_index) {
    if (cret_index < 0 || cret_index >= NEXUS_CREATURE_NAME_COUNT)
        return NULL;
    return g_mns_names[cret_index];
}
