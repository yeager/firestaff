#ifndef DM2_V1_SAVE_LOAD_EXTRA_DUNGEON_DATA_PC34_COMPAT_H
#define DM2_V1_SAVE_LOAD_EXTRA_DUNGEON_DATA_PC34_COMPAT_H

/* DM2 READ_SKSAVE_DUNGEON — per-map tile walker + record chain deserializer.
 * Source: sksvgame.cpp:1108-1400. Inverse of STORE_EXTRA_DUNGEON_DATA.
 *
 * Walks every map, every tile, reads SUPPRESS-encoded tile headers,
 * then calls READ_RECORD_CHECKCODE for the tile's record chain.
 * Types 0-3 records are skipped (bulk-deserialized elsewhere). */

#include "dm2_v1_save_read_record_checkcode_pc34_compat.h"
#include "dm2_v1_save_store_extra_dungeon_data_pc34_compat.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Callbacks for dungeon load — mirrors DM2_StoreExtraDungeonCallbacks
 * but reads instead of writes. */
typedef struct {
    int (*get_map_count)(void *ctx);
    void (*get_map_dimensions)(void *ctx, int *width, int *height);
    void (*change_current_map)(void *ctx, int map_index);
    uint8_t (*get_tile)(void *ctx, int x, int y);

    /* Store the tile after DM2_SUPPRESS_READER has restored its masked bits.
     * SKProject passes the live t_tile address to the reader; omitting this
     * callback would consume the stream into a temporary byte and silently
     * lose authentic tile state. Required when a tile mask is nonzero. */
    int (*set_tile)(void *ctx, int x, int y, uint8_t tile);

    /* Return the source-owned tile record-link head.  READ_SKSAVE_DUNGEON
     * does not replace resident DB0..DB3 chains: it restores their masked
     * record bytes in place, and invokes READ_RECORD_CHECKCODE only when
     * this is OBJECT_END_MARKER. */
    uint16_t (*get_tile_record_link)(void *ctx, int x, int y);

    /* Restore the masked bytes of a resident tile chain.  The callback must
     * traverse the exact existing chain and consume its SUPPRESS fields;
     * returning nonzero rejects the whole load transaction.  It is never
     * legal to allocate a replacement chain for a non-empty tile. */
    int (*restore_existing_tile_record_chain)(void *ctx,
                                               DM2_ReadRecordSession *session,
                                               uint16_t root_link,
                                               int x, int y);

    /* Set tile record link head only for an originally empty tile whose
     * chain was just decoded by READ_RECORD_CHECKCODE. */
    void (*set_tile_record_link)(void *ctx, int x, int y, uint16_t link);

    /* Get teleporter detail for forward-ref skipping.
     * Returns nonzero if detail was filled. */
    int (*get_teleporter_detail)(void *ctx, DM2_TeleporterDetail *out,
                                 int x, int y);

    void *ctx;
} DM2_LoadExtraDungeonCallbacks;

typedef struct {
    int valid;
    int maps_loaded;
    int tiles_loaded;
    int record_chains_loaded;
    int teleporter_forward_refs_skipped;
    int error;
} DM2_V1_LoadExtraDungeonReceipt;

/* Load dungeon data from a SUPPRESS stream.
 * restore_map: map index to switch back to after loading.
 * Returns 0 on success, nonzero on error. */
int dm2_v1_load_extra_dungeon_data(
    DM2_ReadRecordSession *session,
    const DM2_ReadRecordCallbacks *rec_cb,
    const DM2_LoadExtraDungeonCallbacks *dung_cb,
    int restore_map,
    DM2_V1_LoadExtraDungeonReceipt *receipt);

#ifdef __cplusplus
}
#endif

#endif /* DM2_V1_SAVE_LOAD_EXTRA_DUNGEON_DATA_PC34_COMPAT_H */
