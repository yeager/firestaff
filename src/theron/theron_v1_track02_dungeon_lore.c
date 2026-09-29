/*
 * theron_v1_track02_dungeon_lore.c — source-compatible lore accessors.
 *
 * Do not maintain a paraphrased copy here. The retail US Track 02 narrative,
 * including its original 0x01/0x02/0x03 presentation controls, is owned by
 * theron_v1_track02_dungeon_text.c and must be returned byte-for-byte.
 */

#include "theron_v1_track02_dungeon_lore.h"
#include "theron_v1_track02_dungeon_text.h"

const char *theron_v1_track02_us_dungeon_lore(unsigned int dungeon_index) {
    return theron_v1_track02_us_dungeon_story(dungeon_index);
}

const char *theron_v1_track02_us_file_exists_warning(void) {
    return "THAT FILE ALREADY EXISTS!";
}

const char *theron_v1_track02_us_replace_label(void) {
    return "REPLACE";
}

const char *theron_v1_track02_us_no_label(void) {
    return "NO";
}
