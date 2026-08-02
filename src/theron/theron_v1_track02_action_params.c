#include "theron_v1_track02_action_params.h"

/* Source: US Track 02 BIN (MD5 f23601102138f87c33025877767ebf76).
 * Raw bytes from UD 0x09EFB0, immediately after the action/spell name table. */

static const uint8_t g_raw[THERON_TRACK02_ACTION_PARAM_RAW_SIZE] = {
    /* Spell  0 FIREBALL    */ 0x06, 0x08,
    /* Spell  1 DISPELL     */ 0x00, 0x06,
    /* Spell  2 CONFUSE     */ 0x03, 0x01,
    /* Spell  3 LIGHTNING   */ 0x05, 0x03,
    /* Spell  4 DISRUPT     */ 0x05, 0x23,
    /* Spell  5 MELEE       */ 0x14, 0x04,
    /* Spell  6 X           */ 0x06, 0x0A,
    /* Spell  7 INVOKE      */ 0x10, 0x02,
    /* Spell  8 SLASH       */ 0x12, 0x08,
    /* Spell  9 CLEAVE      */ 0x1E, 0x2A,
    /* Spell 10 BASH        */ 0x1F, 0x0A,
    /* Spell 11 STUN        */ 0x26, 0x09,
    /* Spell 12 SHOOT       */ 0x14, 0x0A,
    /* Spell 13 SPELLSHIELD */ 0x10, 0x04,
    /* Spell 14 FIRESHIELD  */ 0x0C, 0x14,
    /* Spell 15 HEAL        */ 0x07, 0x0E,
    /* Spell 16 CALM        */ 0x1E, 0x23,
    /* Spell 17 LIGHT       */ 0x02, 0x13,
    /* Spell 18 SPIT        */ 0x09, 0x0A,
    /* Spell 19 BRANDISH    */ 0x0F, 0x16,
    /* Spell 20 THROW       */ 0x0A, 0x00,
    /* Extended block (record boundaries unconfirmed) */
    0x02, 0x00, 0x0F, 0x30, 0x00, 0x00,
    0x00, 0x20, 0x30, 0x00, 0x30, 0x00, 0x00, 0x14,
    0x10, 0x3C, 0x42, 0x08, 0x08, 0x19, 0x60, 0x00,
    0x00, 0x00, 0x00, 0x37, 0x3C, 0x00, 0x00, 0x10,
    0x30, 0x32, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const Theron_Track02SpellParam g_spell_params[THERON_TRACK02_SPELL_PARAM_COUNT] = {
    { 0x06, 0x08 }, /* FIREBALL    */
    { 0x00, 0x06 }, /* DISPELL     */
    { 0x03, 0x01 }, /* CONFUSE     */
    { 0x05, 0x03 }, /* LIGHTNING   */
    { 0x05, 0x23 }, /* DISRUPT     */
    { 0x14, 0x04 }, /* MELEE       */
    { 0x06, 0x0A }, /* X           */
    { 0x10, 0x02 }, /* INVOKE      */
    { 0x12, 0x08 }, /* SLASH       */
    { 0x1E, 0x2A }, /* CLEAVE      */
    { 0x1F, 0x0A }, /* BASH        */
    { 0x26, 0x09 }, /* STUN        */
    { 0x14, 0x0A }, /* SHOOT       */
    { 0x10, 0x04 }, /* SPELLSHIELD */
    { 0x0C, 0x14 }, /* FIRESHIELD  */
    { 0x07, 0x0E }, /* HEAL        */
    { 0x1E, 0x23 }, /* CALM        */
    { 0x02, 0x13 }, /* LIGHT       */
    { 0x09, 0x0A }, /* SPIT        */
    { 0x0F, 0x16 }, /* BRANDISH    */
    { 0x0A, 0x00 }, /* THROW       */
};

const Theron_Track02SpellParam *theron_v1_track02_spell_param(unsigned int spell_index) {
    if (spell_index >= THERON_TRACK02_SPELL_PARAM_COUNT) return NULL;
    return &g_spell_params[spell_index];
}

size_t theron_v1_track02_spell_param_count(void) {
    return THERON_TRACK02_SPELL_PARAM_COUNT;
}

const uint8_t *theron_v1_track02_action_param_raw(void) {
    return g_raw;
}

/* Combat action parameters from UD 0x09EE5B (40 bytes before string pointer table). */
static const Theron_Track02CombatActionParam g_combat_params[THERON_TRACK02_COMBAT_ACTION_PARAM_COUNT] = {
    { 0x00, 0x04 }, /*  0 N            */
    { 0x10, 0x11 }, /*  1 BLOCK        */
    { 0x0E, 0x11 }, /*  2 CHOP         */
    { 0x10, 0x10 }, /*  3 X            */
    { 0x09, 0x04 }, /*  4 BLOW HORN    */
    { 0x05, 0x04 }, /*  5 FLIP         */
    { 0x05, 0x04 }, /*  6 PUNCH        */
    { 0x03, 0x04 }, /*  7 KICK         */
    { 0x04, 0x04 }, /*  8 WAR CRY      */
    { 0x06, 0x06 }, /*  9 STAB         */
    { 0x06, 0x00 }, /* 10 CLIMB DOWN   */
    { 0x09, 0x0B }, /* 11 FREEZE LIFE  */
    { 0x0F, 0x0F }, /* 12 HIT          */
    { 0x10, 0x10 }, /* 13 SWING        */
    { 0x12, 0x11 }, /* 14 STAB         */
    { 0x11, 0x13 }, /* 15 THRUST       */
    { 0x0E, 0x00 }, /* 16 JAB          */
    { 0x0E, 0x08 }, /* 17 PARRY        */
    { 0x09, 0x09 }, /* 18 HACK         */
    { 0x0A, 0x03 }, /* 19 BERZERK      */
};

const Theron_Track02CombatActionParam *theron_v1_track02_combat_action_param(unsigned int index) {
    if (index >= THERON_TRACK02_COMBAT_ACTION_PARAM_COUNT) return NULL;
    return &g_combat_params[index];
}

size_t theron_v1_track02_combat_action_param_count(void) {
    return THERON_TRACK02_COMBAT_ACTION_PARAM_COUNT;
}

/* PCE string pointer table from UD 0x09EE6F (44 x uint16 LE). */
static const uint16_t g_action_ptrs[THERON_TRACK02_ACTION_PTR_TABLE_COUNT] = {
    0x6EA3, 0x6EA5, 0x6EAB, 0x6EB0, 0x6EB2, 0x6EBC, 0x6EC1, 0x6EC7,
    0x6ECC, 0x6ED4, 0x6ED9, 0x6EE4, 0x6EF0, 0x6EF4, 0x6EFA, 0x6EFF,
    0x6F06, 0x6F0A, 0x6F10, 0x6F15, 0x6F1D, 0x6F26, 0x6F2E, 0x6F36,
    0x6F40, 0x6F48, 0x6F4E, 0x6F50, 0x6F57, 0x6F5D, 0x6F64, 0x6F69,
    0x6F6E, 0x6F74, 0x6F80, 0x6F8B, 0x6F8B, 0x6F90, 0x6F95, 0x6F9B,
    0x6F9B, 0x6FA0, 0x6FA9, 0x6FAF,
};

const uint16_t *theron_v1_track02_action_ptr_table(void) {
    return g_action_ptrs;
}
