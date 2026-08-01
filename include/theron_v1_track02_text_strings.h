#ifndef THERON_V1_TRACK02_TEXT_STRINGS_H
#define THERON_V1_TRACK02_TEXT_STRINGS_H

#include <stddef.h>

/* US Track 02 text strings extracted from hash-verified BIN
 * (MD5: f23601102138f87c33025877767ebf76).
 * Evidence: parity-evidence/pass215_theron_track02_binary_analysis.md */

#define THERON_TRACK02_LEVEL_COUNT        15u
#define THERON_TRACK02_QUEST_MESSAGE_COUNT  7u
#define THERON_TRACK02_SAVE_SLOT_COUNT      3u
#define THERON_TRACK02_STATUS_STRING_COUNT  5u

const char *theron_v1_track02_us_level_name(unsigned int level_index);
const char *theron_v1_track02_us_quest_message(unsigned int quest_index);
const char *theron_v1_track02_us_save_slot_label(unsigned int slot_index);
const char *theron_v1_track02_us_play_prompt(void);
const char *theron_v1_track02_us_load_prompt(void);
const char *theron_v1_track02_us_yes_label(void);
const char *theron_v1_track02_us_no_label(void);
const char *theron_v1_track02_us_status_string(unsigned int index);

#endif
