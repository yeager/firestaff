#include "theron_v1_track19_inventory.h"

#include <string.h>

int theron_v1_track19_inventory(const char *md5,
                                size_t bytes,
                                Theron_V1Track19InventoryReceipt *out) {
    const char *variant = NULL;

    if (out) {
        memset(out, 0, sizeof(*out));
    }
    if (!out || !md5 || bytes == 0u || bytes % 2352u != 0u) {
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

    out->valid = 1;
    out->sector_aligned = 1;
    out->container_format_unproven = 1;
    out->startup_usable = 0;
    out->level_usable = 0;
    out->bitmap_usable = 0;
    out->bytes = bytes;
    out->variant = variant;
    return 1;
}
