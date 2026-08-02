#ifndef THERON_V1_TRACK02_DUNGEON_TEXT_H
#define THERON_V1_TRACK02_DUNGEON_TEXT_H

#include <stddef.h>

#define THERON_TRACK02_DUNGEON_COUNT 7u

#define THERON_DUNGEON_AKUTUBA  0u
#define THERON_DUNGEON_DRATOR   1u
#define THERON_DUNGEON_FORMIC   2u
#define THERON_DUNGEON_SARMON   3u
#define THERON_DUNGEON_SHADO    4u
#define THERON_DUNGEON_THIEF    5u
#define THERON_DUNGEON_DEMON    6u

const char *theron_v1_track02_us_dungeon_name(unsigned int index);
const char *theron_v1_track02_us_dungeon_story(unsigned int index);
const char *theron_v1_track02_us_treasure_name(unsigned int index);
const char *theron_v1_track02_us_retrieval_message(unsigned int index);
const char *theron_v1_track02_us_game_speed_label(void);

#endif /* THERON_V1_TRACK02_DUNGEON_TEXT_H */
