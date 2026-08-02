#ifndef THERON_V1_TRACK02_CLASS_SKILL_PARAMS_H
#define THERON_V1_TRACK02_CLASS_SKILL_PARAMS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Source: US Track 02 BIN (MD5 f23601102138f87c33025877767ebf76).
 * UD 0x0899C0, bank 68 PCE addr $79C0.
 * 110 bytes immediately before class name strings at UD 0x089A32.
 *
 * Tail 16 bytes (offset 94): skill XP cost table, 4 classes x 4 skills.
 * Each class has cost=0 for its primary skill (diagonal pattern).
 * Preceding 94 bytes: undecoded stat/skill parameter block. */

#define THERON_TRACK02_CLASS_SKILL_RAW_SIZE       110u
#define THERON_TRACK02_CLASS_COUNT                   4u
#define THERON_TRACK02_SKILL_CATEGORY_COUNT          4u
#define THERON_TRACK02_SKILL_XP_COST_TABLE_SIZE     16u

/* Skill XP cost: lower value = easier to gain.
 * Layout: [class][skill_category], row-major.
 * FIGHTER={0,3,2,2}, NINJA={5,0,2,0}, PRIEST={1,1,0,0}, WIZARD={1,0,0,0}. */
const uint8_t *theron_v1_track02_skill_xp_cost_table(void);
uint8_t theron_v1_track02_skill_xp_cost(unsigned int class_idx,
                                         unsigned int skill_cat);

const uint8_t *theron_v1_track02_class_skill_raw(void);
size_t theron_v1_track02_class_skill_raw_size(void);

/* Class name string offset table (4 bytes at UD 0x089A2E).
 * Offsets from UD 0x089A2E to null-terminated class name strings. */
const uint8_t *theron_v1_track02_class_name_offsets(void);

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_TRACK02_CLASS_SKILL_PARAMS_H */
