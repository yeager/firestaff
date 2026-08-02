#ifndef DM2_V1_SAVE_STORE_EXTRA_DUNGEON_DATA_PC34_COMPAT_H
#define DM2_V1_SAVE_STORE_EXTRA_DUNGEON_DATA_PC34_COMPAT_H

#include "dm2_v1_save_write_record_checkcode_pc34_compat.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DM2 STORE_EXTRA_DUNGEON_DATA — per-map tile walker + record chain serializer.
 * Source: sksvgame.cpp:1958-2041.
 *
 * Iterates all maps, all tiles (width x height), applies tile-type SUPPRESS
 * masks, then calls WRITE_RECORD_CHECKCODE for each tile's record chain.
 * Uses callbacks for dungeon state access. */

/* Teleporter detail (c_5bytes in skproject). */
typedef struct {
    uint8_t bytes[5];
} DM2_TeleporterDetail;

/* Callbacks for dungeon map/tile access. */
typedef struct {
    /* Switch current map context. */
    void (*change_current_map)(void *ctx, int map_index);

    /* Get tile value at (x, y) in current map.
     * Tiles are walked column-major: x outer, y inner. */
    uint8_t (*get_tile)(void *ctx, int x, int y);

    /* Get the record link word for tile at (x, y).
     * Returns the first record link, or DM2_RECORD_LINK_NONE if bit 4 is clear. */
    uint16_t (*get_record_link)(void *ctx, int x, int y);

    /* Get teleporter detail at (x, y).
     * Returns 0 if no teleporter, nonzero on success (fills detail). */
    int (*get_teleporter_detail)(void *ctx, DM2_TeleporterDetail *out,
                                  int x, int y);

    /* Get total number of maps. */
    int (*get_map_count)(void *ctx);

    /* Get current map dimensions. Must be called after change_current_map. */
    void (*get_map_dimensions)(void *ctx, int *width, int *height);

    /* Initialize SUPPRESS writer state (called once at start via 2066_0b44).
     * Returns 0 on success, nonzero on failure. */
    int (*init_suppress)(void *ctx);

    void *ctx;
} DM2_StoreExtraDungeonCallbacks;

/* Walk all maps and tiles, writing tile masks and record chains.
 * restore_map: the map index to restore after completion.
 *
 * Returns 0 on success, nonzero on failure. */
int dm2_v1_store_extra_dungeon_data(
    DM2_WriteRecordSession *session,
    const DM2_WriteRecordCallbacks *rec_cb,
    const DM2_StoreExtraDungeonCallbacks *dung_cb,
    int restore_map);

#ifdef __cplusplus
}
#endif

#endif /* DM2_V1_SAVE_STORE_EXTRA_DUNGEON_DATA_PC34_COMPAT_H */
