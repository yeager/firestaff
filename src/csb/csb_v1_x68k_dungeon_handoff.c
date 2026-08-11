#include "csb_v1_x68k_dungeon_handoff.h"

#include "csb_v1_x68k_hdm.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

int csb_v1_x68k_hdm_load_dungeon(CSB_V1_DungeonData *out_dungeon,
                                 const uint8_t *hdm, size_t hdm_size,
                                 CSB_V1_X68kHdmReceipt *out_receipt) {
    uint8_t *compressed = NULL;
    size_t compressed_size = 0u;
    int loaded;

    if (!out_dungeon || !hdm) return CSB_V1_X68K_DUNGEON_HANDOFF_ERR_ARGUMENT;
    memset(out_dungeon, 0, sizeof(*out_dungeon));
    if (!csb_v1_x68k_hdm_extract_root_file(hdm, hdm_size, "DUNGEON.DAT", NULL,
                                            0u, &compressed_size, out_receipt) ||
        !compressed_size || compressed_size > (size_t)INT_MAX ||
        !(compressed = (uint8_t *)malloc(compressed_size)) ||
        !csb_v1_x68k_hdm_extract_root_file(hdm, hdm_size, "DUNGEON.DAT", compressed,
                                            compressed_size, &compressed_size, NULL)) {
        free(compressed);
        return CSB_V1_X68K_DUNGEON_HANDOFF_ERR_MEDIA;
    }
    loaded = csb_v1_dungeon_load_source_bytes(out_dungeon, compressed,
                                               (int)compressed_size);
    free(compressed);
    if (loaded != 0) {
        csb_v1_dungeon_free(out_dungeon);
        return CSB_V1_X68K_DUNGEON_HANDOFF_ERR_LOAD;
    }
    return CSB_V1_X68K_DUNGEON_HANDOFF_OK;
}
