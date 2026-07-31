#include "theron_v1_track19_inventory.h"

#include <string.h>

int theron_v1_track19_inventory(const char *md5,
                                size_t bytes,
                                Theron_V1Track19InventoryReceipt *out) {
    const char *variant = NULL;

    if (out) {
        memset(out, 0, sizeof(*out));
    }
    size_t sector_bytes;

    if (!out || !md5 || bytes == 0u) {
        return 0;
    }

    if (strcmp(md5, "51b40a17b92a30339957ba564aa0015c") == 0) {
        variant = "us";
    } else if (strcmp(md5, "f9f069a5e489b91207f3156059b756f1") == 0) {
        variant = "jp";
    }
    if (!variant) {
        return 0;
    }

    /* Track 19 is authored as MODE1/2048 in the supplied CUE/ISO corpus.
     * Raw MODE1/2352 is also retained as an accepted transport form, but
     * neither format alone proves the game-record grammar. */
    if (bytes % 2048u == 0u &&
        ((strcmp(md5, "51b40a17b92a30339957ba564aa0015c") == 0 &&
          bytes == 5984256u) ||
         (strcmp(md5, "f9f069a5e489b91207f3156059b756f1") == 0 &&
          bytes == 6291456u))) {
        sector_bytes = 2048u;
    } else if (bytes % 2352u == 0u) {
        sector_bytes = 2352u;
    } else {
        return 0;
    }

    out->valid = 1;
    out->sector_aligned = 1;
    out->container_format_unproven = sector_bytes == 2352u;
    out->startup_usable = 0;
    out->level_usable = 0;
    out->bitmap_usable = 0;
    out->mode1_2048 = sector_bytes == 2048u;
    out->mode1_2352 = sector_bytes == 2352u;
    out->sector_count = bytes / sector_bytes;
    out->bytes = bytes;
    out->source_format = sector_bytes == 2048u ? "MODE1/2048-ISO" :
        "MODE1/2352-RAW";
    out->variant = variant;
    return 1;
}
