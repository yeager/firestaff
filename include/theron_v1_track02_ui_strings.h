#ifndef THERON_V1_TRACK02_UI_STRINGS_H
#define THERON_V1_TRACK02_UI_STRINGS_H

#include <stddef.h>

/* US Track 02 UI/HUD string tables extracted from
 * hash-verified BIN (MD5: f23601102138f87c33025877767ebf76).
 * Evidence: parity-evidence/pass215_theron_track02_binary_analysis.md */

#define THERON_TRACK02_FLASK_STATE_COUNT     4u
#define THERON_TRACK02_DIRECTION_COUNT       4u
#define THERON_TRACK02_ITEM_STATUS_COUNT     3u

const char *theron_v1_track02_us_flask_state(unsigned int index);
const char *theron_v1_track02_us_direction_name(unsigned int index);
const char *theron_v1_track02_us_party_facing(void);
const char *theron_v1_track02_us_weighs(void);
const char *theron_v1_track02_us_kg_suffix(void);
const char *theron_v1_track02_us_burnt_out(void);
const char *theron_v1_track02_us_consumable(void);
const char *theron_v1_track02_us_item_status(unsigned int index);
const char *theron_v1_track02_us_status_separator(void);
const char *theron_v1_track02_us_status_and(void);
const char *theron_v1_track02_us_wake_up(void);
const char *theron_v1_track02_us_game_frozen(void);
const char *theron_v1_track02_us_resurrected(void);

#define THERON_TRACK02_SAVE_DELETE_STRING_COUNT 5u
const char *theron_v1_track02_us_save_delete_string(unsigned int index);

#endif
