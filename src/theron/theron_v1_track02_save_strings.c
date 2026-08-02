#include "theron_v1_track02_save_strings.h"
#include <stddef.h>

/* Source: US Track 02 BIN (MD5 f23601102138f87c33025877767ebf76).
 * Save file management strings at UD 0x27519B-0x275239.
 * Used by the save/load menu system. */

const char *theron_v1_track02_us_which_file_play(void) {
    return "   WHICH FILE DO YOU PLAY?";
}

const char *theron_v1_track02_us_which_file_load(void) {
    return "   WHICH FILE DO YOU LOAD?";
}

const char *theron_v1_track02_us_file_exists(void) {
    return " THAT FILE ALREADY EXISTS!";
}

static const char *const g_file_names[THERON_TRACK02_SAVE_FILE_COUNT] = {
    "FILE_1",  /* UD 0x2751EF */
    "FILE_2",  /* UD 0x2751F7 */
    "FILE_3",  /* UD 0x2751FF */
};

const char *theron_v1_track02_us_save_file_name(unsigned int index) {
    if (index >= THERON_TRACK02_SAVE_FILE_COUNT) return NULL;
    return g_file_names[index];
}

const char *theron_v1_track02_us_yes(void) { return "YES"; }
const char *theron_v1_track02_us_no(void) { return "NO "; }
const char *theron_v1_track02_us_replace(void) { return "REPLACE"; }
