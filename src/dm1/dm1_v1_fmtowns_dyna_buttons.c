#include "dm1_v1_fmtowns_dyna_buttons.h"

/* Source-locked English DYNA_BUTTONS label pool. Every string is a
 * byte-verified extraction from the hash-verified HMA-240 English
 * EDM.EXP DYNA_BUTTONS symbol at vaddr 0x24194. Do not modify
 * without matching EDM.EXP evidence. */
static const char *const k_dm1_fmtowns_dyna_button_labels_pc34[
        DM1_V1_FMTOWNS_DYNA_BUTTONS_COUNT] = {
    /*  0 */ "N",           /* placeholder / empty-slot glyph      */
    /*  1 */ "BLOCK",
    /*  2 */ "CHOP",
    /*  3 */ "X",           /* placeholder / disabled-slot glyph   */
    /*  4 */ "BLOW HORN",
    /*  5 */ "FLIP",
    /*  6 */ "PUNCH",
    /*  7 */ "KICK",
    /*  8 */ "WAR CRY",
    /*  9 */ "STAB",
    /* 10 */ "CLIMB DOWN",
    /* 11 */ "FREEZE LIFE",
    /* 12 */ "HIT",
    /* 13 */ "SWING",
    /* 14 */ "STAB",        /* duplicate of index 9 in source pool */
    /* 15 */ "THRUST",
    /* 16 */ "JAB",
    /* 17 */ "PARRY",
    /* 18 */ "HACK",
    /* 19 */ "BERZERK",
    /* 20 */ "FIREBALL",
    /* 21 */ "DISPELL",
    /* 22 */ "CONFUSE",
    /* 23 */ "LIGHTNING",
    /* 24 */ "DISRUPT",
    /* 25 */ "MELEE",
    /* 26 */ "X",
    /* 27 */ "INVOKE",
    /* 28 */ "SLASH",
    /* 29 */ "CLEAVE",
    /* 30 */ "BASH",
    /* 31 */ "STUN",
    /* 32 */ "SHOOT",
    /* 33 */ "SPELLSHIELD",
    /* 34 */ "FIRESHIELD",
    /* 35 */ "FLUXCAGE",
    /* 36 */ "HEAL",
    /* 37 */ "CALM",
    /* 38 */ "LIGHT",
    /* 39 */ "WINDOW",
    /* 40 */ "SPIT",
    /* 41 */ "BRANDISH",
    /* 42 */ "THROW",
    /* 43 */ "FUSE",
};

const char *dm1_v1_fmtowns_dyna_button_label_pc34(unsigned int index) {
    if (index == DM1_V1_FMTOWNS_DYNA_LABEL_SENTINEL) {
        return "";
    }
    if (index >= DM1_V1_FMTOWNS_DYNA_BUTTONS_COUNT) {
        return NULL;
    }
    return k_dm1_fmtowns_dyna_button_labels_pc34[index];
}

size_t dm1_v1_fmtowns_dyna_buttons_source_span_bytes_pc34(void) {
    /* Sum of string lengths + one NUL each. Byte-verified from
     * EDM.EXP's DYNA_BUTTONS pool (vaddr 0x24194); the 11 empty
     * padding entries between index 43 and symbol SPELLS at 0x242c0
     * account for the remaining 11 bytes in the executable. */
    size_t total = 0;
    unsigned int i;
    for (i = 0; i < DM1_V1_FMTOWNS_DYNA_BUTTONS_COUNT; ++i) {
        const char *s = k_dm1_fmtowns_dyna_button_labels_pc34[i];
        while (*s) { ++total; ++s; }
        ++total;                              /* NUL separator */
    }
    return total;
}
