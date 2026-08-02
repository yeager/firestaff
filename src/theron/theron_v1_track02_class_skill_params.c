#include "theron_v1_track02_class_skill_params.h"

/* Source: US Track 02 BIN (MD5 f23601102138f87c33025877767ebf76).
 * 110 bytes at UD 0x0899C0 (bank 68, PCE $79C0).
 * Identical copy at UD 0x1C99C0 (bank 228, same PCE address). */

static const uint8_t g_raw[THERON_TRACK02_CLASS_SKILL_RAW_SIZE] = {
    /* Offset  0: undecoded stat/skill parameters (94 bytes) */
    0x28, 0x6F, 0x5A, 0x73,  /*  0: per-class values (40, 111, 90, 115) */
    0x09, 0x42, 0x26, 0x42,  /*  4: (9, 66, 38, 66)  */
    0x26, 0x09, 0x0A, 0x18,  /*  8: (38, 9, 10, 24)  */
    0x26, 0x34, 0x42, 0x50,  /* 12: (38, 52, 66, 80) ascending */
    0x50, 0x42, 0x09, 0x09,  /* 16: (80, 66, 9, 9)   */
    0x09, 0x09, 0x03, 0x03,  /* 20: (9, 9, 3, 3)     */
    0x03, 0x19, 0x19, 0x19,  /* 24: (3, 25, 25, 25)  */
    0x08, 0x08, 0x08, 0x08,  /* 28: constant 8        */
    0x08, 0x08,              /* 32: constant 8        */
    /* Offset 34: 10 groups x 6 values (possibly 6 stats x 10 levels) */
    0x14, 0x02, 0x0A, 0x16, 0x22, 0x14,  /* group 0: varied base values */
    0x1D, 0x1D, 0x1C, 0x1D, 0x1C, 0x1D,  /* group 1: ~29 */
    0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E,  /* group 2: constant 14 */
    0x0F, 0x1D, 0x56, 0x56, 0x56, 0x47,  /* group 3: 15/29/86/86/86/71 */
    0x16, 0x16, 0x16, 0x16, 0x16, 0x16,  /* group 4: constant 22 */
    0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,  /* group 5: constant 12 */
    0x0B, 0x08, 0x0C, 0x0C, 0x0C, 0x0B,  /* group 6: ~12 */
    0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A,  /* group 7: constant 10 */
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08,  /* group 8: constant 8 */
    0x08, 0x09, 0x09, 0x09, 0x09, 0x08,  /* group 9: ~9 */
    /* Offset 94: skill XP cost table (4 classes x 4 skill categories) */
    0x00, 0x03, 0x02, 0x02,  /* FIGHTER: Fighter=0 Ninja=3 Priest=2 Wizard=2 */
    0x05, 0x00, 0x02, 0x00,  /* NINJA:   Fighter=5 Ninja=0 Priest=2 Wizard=0 */
    0x01, 0x01, 0x00, 0x00,  /* PRIEST:  Fighter=1 Ninja=1 Priest=0 Wizard=0 */
    0x01, 0x00, 0x00, 0x00,  /* WIZARD:  Fighter=1 Ninja=0 Priest=0 Wizard=0 */
};

/* Skill XP cost sub-table starts at offset 94 */
static const uint8_t *g_skill_xp_cost = &g_raw[94];

/* Class name string offsets at UD 0x089A2E (4 bytes) */
static const uint8_t g_class_name_offsets[THERON_TRACK02_CLASS_COUNT] = {
    0x04, 0x0C, 0x12, 0x19,
};

const uint8_t *theron_v1_track02_skill_xp_cost_table(void) {
    return g_skill_xp_cost;
}

uint8_t theron_v1_track02_skill_xp_cost(unsigned int class_idx,
                                         unsigned int skill_cat) {
    if (class_idx >= THERON_TRACK02_CLASS_COUNT) return 0xFF;
    if (skill_cat >= THERON_TRACK02_SKILL_CATEGORY_COUNT) return 0xFF;
    return g_skill_xp_cost[class_idx * THERON_TRACK02_SKILL_CATEGORY_COUNT + skill_cat];
}

const uint8_t *theron_v1_track02_class_skill_raw(void) {
    return g_raw;
}

size_t theron_v1_track02_class_skill_raw_size(void) {
    return THERON_TRACK02_CLASS_SKILL_RAW_SIZE;
}

const uint8_t *theron_v1_track02_class_name_offsets(void) {
    return g_class_name_offsets;
}
