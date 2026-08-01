#include "theron_v1_track02_text_strings.h"

/* Source: US Track 02 BIN, hash-verified MD5 f23601102138f87c33025877767ebf76.
 * Evidence: parity-evidence/pass215_theron_track02_binary_analysis.md */

/* UD 0x27423B: 15 fixed-width 9-byte level names (8 chars + null) */
static const char *const g_level_names[THERON_TRACK02_LEVEL_COUNT] = {
    "LEVEL  1", "LEVEL  2", "LEVEL  3", "LEVEL  4", "LEVEL  5",
    "LEVEL  6", "LEVEL  7", "LEVEL  8", "LEVEL  9", "LEVEL 10",
    "LEVEL 11", "LEVEL 12", "LEVEL 13", "LEVEL 14", "LEVEL 15",
};

/* UD 0x27713D: quest item retrieval messages (cleaned of control bytes) */
static const char *const g_quest_messages[THERON_TRACK02_QUEST_MESSAGE_COUNT] = {
    "THERON has retrieved the Shield Defiant.",
    "THERON has retrieved the Taza Boots.",
    "THERON has retrieved the Taza Poleyn.",
    "THERON has retrieved the Soulcage.",
    "THERON has retrieved the Taza Armour.",
    "THERON has retrieved the Tazahelm.",
    "THERON has retrieved the Retaliator.",
};

/* UD 0x27711B: 3 save slot labels (6-byte fixed-width with 0x02/0x01 framing) */
static const char *const g_save_slot_labels[THERON_TRACK02_SAVE_SLOT_COUNT] = {
    "FILE_1", "FILE_2", "FILE_3",
};

/* UD 0x1C65DF: status condition strings */
static const char *const g_status_strings[THERON_TRACK02_STATUS_STRING_COUNT] = {
    "POISONED", "BROKEN", "CURSED", ", ", " AND ",
};

const char *theron_v1_track02_us_level_name(unsigned int level_index) {
    if (level_index >= THERON_TRACK02_LEVEL_COUNT) return NULL;
    return g_level_names[level_index];
}

const char *theron_v1_track02_us_quest_message(unsigned int quest_index) {
    if (quest_index >= THERON_TRACK02_QUEST_MESSAGE_COUNT) return NULL;
    return g_quest_messages[quest_index];
}

const char *theron_v1_track02_us_save_slot_label(unsigned int slot_index) {
    if (slot_index >= THERON_TRACK02_SAVE_SLOT_COUNT) return NULL;
    return g_save_slot_labels[slot_index];
}

/* UD 0x2770E9 */
const char *theron_v1_track02_us_play_prompt(void) {
    return "WHICH FILE DO YOU PLAY?";
}

/* UD 0x277102 */
const char *theron_v1_track02_us_load_prompt(void) {
    return "WHICH FILE DO YOU LOAD?";
}

/* UD 0x277133 */
const char *theron_v1_track02_us_yes_label(void) { return "YES"; }
const char *theron_v1_track02_us_no_label(void)  { return "NO";  }

const char *theron_v1_track02_us_status_string(unsigned int index) {
    if (index >= THERON_TRACK02_STATUS_STRING_COUNT) return NULL;
    return g_status_strings[index];
}
