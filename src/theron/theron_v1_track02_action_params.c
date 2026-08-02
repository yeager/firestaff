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
