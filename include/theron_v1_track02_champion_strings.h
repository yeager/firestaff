#ifndef THERON_V1_TRACK02_CHAMPION_STRINGS_H
#define THERON_V1_TRACK02_CHAMPION_STRINGS_H

#include <stddef.h>

/* US Track 02 champion/combat/spell string tables extracted from
 * hash-verified BIN (MD5: f23601102138f87c33025877767ebf76).
 * Evidence: parity-evidence/pass215_theron_track02_binary_analysis.md */

#define THERON_TRACK02_CLASS_COUNT        4u
#define THERON_TRACK02_STAT_COUNT         6u
#define THERON_TRACK02_RESOURCE_COUNT     3u
#define THERON_TRACK02_SKILL_LEVEL_COUNT 16u
#define THERON_TRACK02_ACTION_COUNT      30u

const char *theron_v1_track02_us_class_name(unsigned int index);
const char *theron_v1_track02_us_stat_name(unsigned int index);
const char *theron_v1_track02_us_resource_name(unsigned int index);
const char *theron_v1_track02_us_skill_level_name(unsigned int index);
const char *theron_v1_track02_us_action_name(unsigned int index);

const char *theron_v1_track02_us_cant_reach(void);
const char *theron_v1_track02_us_need_ammo(void);
const char *theron_v1_track02_us_heads(void);
const char *theron_v1_track02_us_tails(void);

#endif
