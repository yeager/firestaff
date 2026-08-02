#include "dm1_v1_fmtowns_cd_audio.h"
#include <stddef.h>

static const DM1_V1_FmtownsCdTrackInfo track_table[19] = {
    {  2, 0, "Title screen"             },
    {  3, 0, "Hall of Champions"         },
    {  4, 1, "Unused"                    },
    {  5, 0, "Entrance and map 6"        },
    {  6, 0, "Maps 1-2"                  },
    {  7, 1, "Unused"                    },
    {  8, 0, "Maps 3-4"                  },
    {  9, 0, "Map 5"                     },
    { 10, 0, "Map 7"                     },
    { 11, 0, "Maps 8-9"                  },
    { 12, 0, "Map 10"                    },
    { 13, 0, "Game Over"                 },
    { 14, 0, "Map 11"                    },
    { 15, 0, "Map 12"                    },
    { 16, 0, "Map 13"                    },
    { 17, 0, "Maps 14-15"               },
    { 18, 0, "Game Won"                  },
    { 19, 0, "Ambient"                   },
    { 20, 1, "Silence"                   },
};

/* Map index (0-based) to CD audio track number.
 * The entrance micro-dungeon is map index 255 in the original;
 * use map_index -1 for it. */
static const int map_to_track[16] = {
    /* map  0 */ 6,     /* Hall of Champions area */
    /* map  1 */ 6,     /* Hall of Champions area */
    /* map  2 */ 8,     /* Creature Caverns */
    /* map  3 */ 8,     /* The Prison */
    /* map  4 */ 9,     /* Top of the Keep */
    /* map  5 */ 5,     /* Gate of the Southern Keep */
    /* map  6 */ 10,    /* Dungeon Entrance */
    /* map  7 */ 11,    /* The Corridors */
    /* map  8 */ 11,    /* The Corridors */
    /* map  9 */ 12,    /* The Tomb */
    /* map 10 */ 14,    /* The Dark Side */
    /* map 11 */ 15,    /* Hall of the Dragon Lord */
    /* map 12 */ 16,    /* Corbum's Maze */
    /* map 13 */ 17,    /* End game area */
    /* map 14 */ 17,    /* End game area */
    /* map 15 */ 0,     /* No track */
};

int dm1_v1_fmtowns_cd_track_for_map(int map_index) {
    if (map_index < 0 || map_index > 15) return 0;
    return map_to_track[map_index];
}

int dm1_v1_fmtowns_cd_track_for_event(int event) {
    switch (event) {
    case 0: return DM1_FMTOWNS_TRACK_TITLE;
    case 1: return DM1_FMTOWNS_TRACK_HALL;
    case 2: return DM1_FMTOWNS_TRACK_GAME_OVER;
    case 3: return DM1_FMTOWNS_TRACK_GAME_WON;
    default: return 0;
    }
}

const DM1_V1_FmtownsCdTrackInfo *dm1_v1_fmtowns_cd_track_info(int track_number) {
    if (track_number < DM1_FMTOWNS_CD_FIRST_AUDIO_TRACK ||
        track_number > DM1_FMTOWNS_CD_LAST_AUDIO_TRACK)
        return NULL;
    return &track_table[track_number - DM1_FMTOWNS_CD_FIRST_AUDIO_TRACK];
}
