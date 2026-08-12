/* Authentic Japanese Macintosh CD data-media reader gate. */
#include "dm2_v1_mac_media.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    const char *zip = getenv("FIRESTAFF_DM2_MAC_JA_ZIP");
    DM2_V1_MacMedia media;
    if (!zip || !zip[0]) {
        puts("SKIP: DM2 Japanese Mac ZIP environment is not set");
        return 0;
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
    dm2_v1_mac_media_free(&media);
    puts("PASS: authentic Japanese DM2 Macintosh HFS data read in RAM");
    return 0;
}
