#ifndef DM1_V1_FMTOWNS_CD_AUDIO_H
#define DM1_V1_FMTOWNS_CD_AUDIO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* FM Towns DM1 CD audio track mapping.
 *
 * The FM Towns DM1 disc has 19 CDDA audio tracks (02-20) that replace
 * the PC 3.4 SONG.DAT file. The original game uses TBIOS CD-ROM
 * system calls to play these tracks for in-game music.
 *
 * Track assignments (from DMWeb FM Towns edition page):
 *   02: Title screen / main theme
 *   03: Hall of Champions
 *   04: Unused
 *   05: Entrance dungeon and map 6 (Gate of the Southern Keep)
 *   06: Maps 1-2 (Hall of Champions area)
 *   07: Unused
 *   08: Maps 3-4 (Creature Caverns / The Prison)
 *   09: Map 5 (Top of the Keep)
 *   10: Map 7 (Dungeon Entrance)
 *   11: Maps 8-9 (The Corridors)
 *   12: Map 10 (The Tomb)
 *   13: Game Over
 *   14: Map 11 (The Dark Side)
 *   15: Map 12 (Hall of the Dragon Lord)
 *   16: Map 13 (Corbum's Maze)
 *   17: Map 14-15 (end game areas)
 *   18: Game Won / credits
 *   19: Ambient / exploration
 *   20: Unused (20 seconds of silence)
 */

#define DM1_FMTOWNS_CD_FIRST_AUDIO_TRACK  2
#define DM1_FMTOWNS_CD_LAST_AUDIO_TRACK   20
#define DM1_FMTOWNS_CD_AUDIO_TRACK_COUNT  19

typedef enum {
    DM1_FMTOWNS_TRACK_TITLE          = 2,
    DM1_FMTOWNS_TRACK_HALL           = 3,
    DM1_FMTOWNS_TRACK_UNUSED_04      = 4,
    DM1_FMTOWNS_TRACK_ENTRANCE_MAP6  = 5,
    DM1_FMTOWNS_TRACK_MAPS_1_2       = 6,
    DM1_FMTOWNS_TRACK_UNUSED_07      = 7,
    DM1_FMTOWNS_TRACK_MAPS_3_4       = 8,
    DM1_FMTOWNS_TRACK_MAP_5          = 9,
    DM1_FMTOWNS_TRACK_MAP_7          = 10,
    DM1_FMTOWNS_TRACK_MAPS_8_9       = 11,
    DM1_FMTOWNS_TRACK_MAP_10         = 12,
    DM1_FMTOWNS_TRACK_GAME_OVER      = 13,
    DM1_FMTOWNS_TRACK_MAP_11         = 14,
    DM1_FMTOWNS_TRACK_MAP_12         = 15,
    DM1_FMTOWNS_TRACK_MAP_13         = 16,
    DM1_FMTOWNS_TRACK_MAPS_14_15     = 17,
    DM1_FMTOWNS_TRACK_GAME_WON       = 18,
    DM1_FMTOWNS_TRACK_AMBIENT        = 19,
    DM1_FMTOWNS_TRACK_SILENCE        = 20
} DM1_V1_FmtownsCdTrack;

typedef struct {
    int      track_number;
    int      is_unused;
    const char *description;
} DM1_V1_FmtownsCdTrackInfo;

/* Get the CD audio track number for a given dungeon map index (0-based).
 * Returns the track number (2-20), or 0 if no track is assigned. */
int dm1_v1_fmtowns_cd_track_for_map(int map_index);

/* Get the CD audio track for a game event.
 * event: 0=title, 1=hall, 2=game_over, 3=game_won
 * Returns the track number, or 0 if invalid. */
int dm1_v1_fmtowns_cd_track_for_event(int event);

/* Get track info for a given track number.
 * Returns NULL if track_number is out of range. */
const DM1_V1_FmtownsCdTrackInfo *dm1_v1_fmtowns_cd_track_info(int track_number);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_FMTOWNS_CD_AUDIO_H */
