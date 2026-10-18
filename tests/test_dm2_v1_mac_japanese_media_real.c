/* Authentic Japanese Macintosh CD data-media reader gate. */
#include "dm2_v1_mac_media.h"
#include "asset_find_by_hash.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    const char *zip = getenv("FIRESTAFF_DM2_MAC_JA_ZIP");
    DM2_V1_MacMedia media;
    if (!zip || !zip[0]) {
        puts("SKIP: DM2 Japanese Mac ZIP environment is not set");
        return 77;
    }
    if (dm2_v1_mac_media_read_zip(zip, &media) != 0) {
        fprintf(stderr, "authentic Japanese Mac ZIP could not be read: %s\n", zip);
        return 1;
    }
    if (media.graphics_size != 2025699u || media.dungeon_size != 37957u ||
        !media.graphics || !media.dungeon) {
        fprintf(stderr, "unexpected Japanese Mac data sizes: graphics=%zu dungeon=%zu\n",
                media.graphics_size, media.dungeon_size);
        dm2_v1_mac_media_free(&media);
        return 1;
    }
    if (!asset_memory_matches_md5(media.graphics, media.graphics_size,
                                  "283d5456c4f676609489e200219605bb")) {
        fprintf(stderr, "Japanese Mac GRAPHICS.DAT hash does not match the catalog\n");
        dm2_v1_mac_media_free(&media);
        return 1;
    }
    /* The Japanese retail image carries the four movie resources below, but
     * no Mac application, music map, or sound resources. Keep this exact
     * distinction from the English application-backed startup route visible. */
    if (media.movie_present_mask != 0x17u ||
        media.movie_resource_present_mask != 0x17u ||
        media.movie_moov_present_mask != 0x17u ||
        media.sound_resource_fork_present_mask != 0u ||
        media.music_map_size != 0u || media.application_data_size != 0u ||
        media.application_resource_size != 0u || media.application_data ||
        media.application_resource) {
        fprintf(stderr, "Japanese Mac HFS resources differ from the known media receipt\n");
        dm2_v1_mac_media_free(&media);
        return 1;
    }
    dm2_v1_mac_media_free(&media);
    puts("PASS: authentic Japanese DM2 Macintosh HFS data read in RAM");
    return 0;
}
