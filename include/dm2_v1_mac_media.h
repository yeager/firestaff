#ifndef DM2_V1_MAC_MEDIA_H
#define DM2_V1_MAC_MEDIA_H

#include <stddef.h>
#include <stdint.h>

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
    int demo;
} DM2_V1_MacMedia;

int dm2_v1_mac_media_read_zip(const char *zip_path, DM2_V1_MacMedia *out);
void dm2_v1_mac_media_free(DM2_V1_MacMedia *media);

#endif
