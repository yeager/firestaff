#ifndef THERON_V1_TRACK02_SAVE_STRINGS_H
#define THERON_V1_TRACK02_SAVE_STRINGS_H

#include <stddef.h>

#define THERON_TRACK02_SAVE_FILE_COUNT 3u

const char *theron_v1_track02_us_which_file_play(void);
const char *theron_v1_track02_us_which_file_load(void);
const char *theron_v1_track02_us_file_exists(void);
const char *theron_v1_track02_us_save_file_name(unsigned int index);
const char *theron_v1_track02_us_yes(void);
const char *theron_v1_track02_us_no(void);
const char *theron_v1_track02_us_replace(void);

#endif /* THERON_V1_TRACK02_SAVE_STRINGS_H */
