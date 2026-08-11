#include "dm2_v1_mac_media.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    const char *zip = getenv("FIRESTAFF_DM2_MAC_EN_ZIP");
    DM2_V1_MacMedia media;
    if (!zip || !zip[0]) {
        puts("SKIP: FIRESTAFF_DM2_MAC_EN_ZIP is not set");
        return 0;
    }
    if (dm2_v1_mac_media_read_zip(zip, &media) != 0) {
        fprintf(stderr, "authentic Mac ZIP could not be read: %s\n", zip);
        return 1;
    }
    if (media.graphics_size != 8157169u || media.dungeon_size != 39411u ||
        media.music_map_size != 176u) {
        fprintf(stderr, "unexpected Mac fork sizes: graphics=%zu dungeon=%zu md=%zu\n",
                media.graphics_size, media.dungeon_size, media.music_map_size);
        dm2_v1_mac_media_free(&media);
        return 1;
    }
    dm2_v1_mac_media_free(&media);
    puts("PASS: authentic DM2 Macintosh retail HFS forks read in RAM");
    return 0;
}
