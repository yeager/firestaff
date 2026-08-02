#ifndef THERON_V1_TRACK02_HUD_STRINGS_H
#define THERON_V1_TRACK02_HUD_STRINGS_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Source: US Track 02 BIN, UD 0x089B6B region.
 * HUD display strings: flask fill levels, compass directions,
 * stat names, weight format, spell feedback messages. */

/* Flask fill levels (4 entries) */
#define THERON_TRACK02_FLASK_LEVEL_COUNT  4u
const char *theron_v1_track02_us_flask_level(unsigned int index);

/* Compass direction names (4 entries: N/E/S/W) */
#define THERON_TRACK02_DIRECTION_COUNT  4u
const char *theron_v1_track02_us_direction_name(unsigned int index);

/* Attribute stat names from UD 0x089B0F (6 entries) */
#define THERON_TRACK02_STAT_NAME_COUNT  6u
const char *theron_v1_track02_us_stat_name(unsigned int index);

/* Vital stat names from UD 0x08FD84 (3 entries) */
#define THERON_TRACK02_VITAL_NAME_COUNT  3u
const char *theron_v1_track02_us_vital_name(unsigned int index);

/* Spell feedback messages from UD 0x089A6E (6 entries) */
#define THERON_TRACK02_SPELL_MSG_COUNT  6u
const char *theron_v1_track02_us_spell_message(unsigned int index);

/* Weight label: "WEIGHS" */
const char *theron_v1_track02_us_weight_label(void);

/* Weight unit: " KG." */
const char *theron_v1_track02_us_weight_unit(void);

/* Burnt out label */
const char *theron_v1_track02_us_burnt_out_label(void);

/* Party facing label */
const char *theron_v1_track02_us_party_facing_label(void);

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_TRACK02_HUD_STRINGS_H */
