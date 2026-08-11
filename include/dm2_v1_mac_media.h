#ifndef DM2_V1_MAC_MEDIA_H
#define DM2_V1_MAC_MEDIA_H

#include <stddef.h>
#include <stdint.h>

#define DM2_V1_MAC_MOVIE_COUNT 5
#define DM2_V1_MAC_SOUND_RESOURCE_COUNT 3

typedef enum {
    DM2_V1_MAC_SOUND_MUSIC = 0,
    DM2_V1_MAC_SOUND_GENERAL,
    DM2_V1_MAC_SOUND_WEAPON
} DM2_V1_MacSoundResourceId;

typedef enum {
    DM2_V1_MAC_MOVIE_TITLE = 0,
    DM2_V1_MAC_MOVIE_STORY,
    DM2_V1_MAC_MOVIE_SWOOSH,
    DM2_V1_MAC_MOVIE_CREDITS,
    DM2_V1_MAC_MOVIE_ENDING
} DM2_V1_MacMovieId;

/* Read the original Macintosh CD image contained in a ZIP without creating
 * a mounted or extracted game-data directory.  The returned forks are owned
 * by the caller and must be freed with dm2_v1_mac_media_free(). */
typedef struct {
    uint8_t *graphics;
    size_t graphics_size;
    uint8_t *dungeon;
    size_t dungeon_size;
    uint8_t *music_map;
    size_t music_map_size;
    uint8_t *movie[DM2_V1_MAC_MOVIE_COUNT];
    size_t movie_size[DM2_V1_MAC_MOVIE_COUNT];
    uint32_t movie_present_mask;
    /* Classic HFS resource forks are retained separately.  For MooV files
     * this is the authentic Resource Manager fork containing the moov
     * resource; it is not flattened or converted on disk. */
    uint8_t *movie_resource[DM2_V1_MAC_MOVIE_COUNT];
    size_t movie_resource_size[DM2_V1_MAC_MOVIE_COUNT];
    uint32_t movie_resource_present_mask;
    uint8_t *movie_moov[DM2_V1_MAC_MOVIE_COUNT];
    size_t movie_moov_size[DM2_V1_MAC_MOVIE_COUNT];
    uint32_t movie_moov_present_mask;
    uint8_t *sound_resource_fork[DM2_V1_MAC_SOUND_RESOURCE_COUNT];
    size_t sound_resource_fork_size[DM2_V1_MAC_SOUND_RESOURCE_COUNT];
    uint32_t sound_resource_fork_present_mask;
    int demo;
} DM2_V1_MacMedia;

int dm2_v1_mac_media_read_zip(const char *zip_path, DM2_V1_MacMedia *out);
void dm2_v1_mac_media_free(DM2_V1_MacMedia *media);

#endif
