#include "theron_v1_track02_spell_descriptors.h"

/* Source: US Track 02 BIN (MD5 f23601102138f87c33025877767ebf76).
 * Table 1 (cost): 41 bytes from UD 0x09EFAF, immediately after spell names.
 * Table 2 (secondary): 41 bytes from UD 0x09EFD8. */

static const uint8_t g_cost[THERON_TRACK02_ACTION_SPELL_DESCRIPTOR_COUNT] = {
    /*  0 N            */  0,
    /*  1 BLOCK        */  6,
    /*  2 CHOP         */  8,
    /*  3 X            */  0,
    /*  4 BLOW HORN    */  6,
    /*  5 FLIP         */  3,
    /*  6 PUNCH        */  1,
    /*  7 KICK         */  5,
    /*  8 WAR CRY      */  3,
    /*  9 STAB         */  5,
    /* 10 CLIMB DOWN   */ 35,
    /* 11 FREEZE LIFE  */ 20,
    /* 12 HIT          */  4,
    /* 13 SWING        */  6,
    /* 14 STAB         */ 10,
    /* 15 THRUST       */ 16,
    /* 16 JAB          */  2,
    /* 17 PARRY        */ 18,
    /* 18 HACK         */  8,
    /* 19 BERZERK      */ 30,
    /* 20 FIREBALL     */ 42,
    /* 21 DISPELL      */ 31,
    /* 22 CONFUSE      */ 10,
    /* 23 LIGHTNING    */ 38,
    /* 24 DISRUPT      */  9,
    /* 25 MELEE        */ 20,
    /* 26 X            */ 10,
    /* 27 INVOKE       */ 16,
    /* 28 SLASH        */  4,
    /* 29 CLEAVE       */ 12,
    /* 30 BASH         */ 20,
    /* 31 STUN         */  7,
    /* 32 SHOOT        */ 14,
    /* 33 SPELLSHIELD  */ 30,
    /* 34 FIRESHIELD   */ 35,
    /* 35 HEAL         */  2,
    /* 36 CALM         */ 19,
    /* 37 LIGHT        */  9,
    /* 38 SPIT         */ 10,
    /* 39 BRANDISH     */ 15,
    /* 40 THROW        */ 22,
};

static const uint8_t g_secondary[THERON_TRACK02_ACTION_SPELL_DESCRIPTOR_COUNT] = {
    /*  0 N            */ 10,
    /*  1 BLOCK        */  0,
    /*  2 CHOP         */  2,
    /*  3 X            */  0,
    /*  4 BLOW HORN    */ 15,
    /*  5 FLIP         */ 48,
    /*  6 PUNCH        */  0,
    /*  7 KICK         */  0,
    /*  8 WAR CRY      */  0,
    /*  9 STAB         */ 32,
    /* 10 CLIMB DOWN   */ 48,
    /* 11 FREEZE LIFE  */  0,
    /* 12 HIT          */ 48,
    /* 13 SWING        */  0,
    /* 14 STAB         */  0,
    /* 15 THRUST       */ 20,
    /* 16 JAB          */ 16,
    /* 17 PARRY        */ 60,
    /* 18 HACK         */ 66,
    /* 19 BERZERK      */  8,
    /* 20 FIREBALL     */  8,
    /* 21 DISPELL      */ 25,
    /* 22 CONFUSE      */ 96,
    /* 23 LIGHTNING    */  0,
    /* 24 DISRUPT      */  0,
    /* 25 MELEE        */  0,
    /* 26 X            */  0,
    /* 27 INVOKE       */ 55,
    /* 28 SLASH        */ 60,
    /* 29 CLEAVE       */  0,
    /* 30 BASH         */  0,
    /* 31 STUN         */ 16,
    /* 32 SHOOT        */ 48,
    /* 33 SPELLSHIELD  */ 50,
    /* 34 FIRESHIELD   */ 16,
    /* 35 HEAL         */  0,
    /* 36 CALM         */  0,
    /* 37 LIGHT        */  0,
    /* 38 SPIT         */  0,
    /* 39 BRANDISH     */  0,
    /* 40 THROW        */  0,
};

uint8_t theron_v1_track02_action_spell_cost(unsigned int index) {
    if (index >= THERON_TRACK02_ACTION_SPELL_DESCRIPTOR_COUNT) return 0;
    return g_cost[index];
}

uint8_t theron_v1_track02_action_spell_secondary(unsigned int index) {
    if (index >= THERON_TRACK02_ACTION_SPELL_DESCRIPTOR_COUNT) return 0;
    return g_secondary[index];
}
