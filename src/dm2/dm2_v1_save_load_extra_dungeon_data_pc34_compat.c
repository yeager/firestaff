/* DM2 READ_SKSAVE_DUNGEON — per-map tile walker + record chain deserializer.
 * Source: sksvgame.cpp:1108-1400. Inverse of STORE_EXTRA_DUNGEON_DATA. */

#include "dm2_v1_save_load_extra_dungeon_data_pc34_compat.h"
#include <string.h>

static int tile_type(uint8_t tile) { return (tile >> 5) & 0x7; }

/* Compute the SUPPRESS mask byte for reading a tile based on its type.
 * Source: sksvgame.cpp:1996-2023 (same as writer). */
static uint8_t tile_suppress_mask(int type)
{
    switch (type) {
        case 0: case 1: case 3: case 7: return 0;
        case 2: return 0x08;
        case 4: return 0x07;
        case 6: return 0x04;
        default: return 0;
    }
}

int dm2_v1_load_extra_dungeon_data(
    DM2_ReadRecordSession *session,
    const DM2_ReadRecordCallbacks *rec_cb,
    const DM2_LoadExtraDungeonCallbacks *dung_cb,
    int restore_map,
    DM2_V1_LoadExtraDungeonReceipt *receipt)
{
    DM2_V1_LoadExtraDungeonReceipt local;
    memset(&local, 0, sizeof(local));
    if (receipt) memset(receipt, 0, sizeof(*receipt));

    if (!session || !rec_cb || !dung_cb) {
        if (receipt) receipt->error = 1;
        return -1;
    }

    int map_count = dung_cb->get_map_count(dung_cb->ctx);

    for (int map_idx = 0; map_idx < map_count; map_idx++) {
        dung_cb->change_current_map(dung_cb->ctx, map_idx);

        int map_w = 0, map_h = 0;
        dung_cb->get_map_dimensions(dung_cb->ctx, &map_w, &map_h);

        for (int x = 0; x < map_w; x++) {
            for (int y = 0; y < map_h; y++) {
                uint8_t tile = dung_cb->get_tile(dung_cb->ctx, x, y);
                int type = tile_type(tile);
                int skip_record_chain = 0;

                /* Type 5 (teleporter): check for forward-ref skip.
                 * sksvgame.cpp:2010-2017: if dest_map < current_map,
                 * skip the record chain (it was already read). */
                uint8_t mask_byte;
                if (type == 5) {
                    DM2_TeleporterDetail detail;
                    if (dung_cb->get_teleporter_detail == NULL ||
                        dung_cb->get_teleporter_detail(dung_cb->ctx, &detail, x, y) == 0) {
                        mask_byte = 0x08;
                    } else {
                        mask_byte = 0;
                        if ((unsigned)map_idx > (unsigned)detail.bytes[4]) {
                            skip_record_chain = 1;
                            local.teleporter_forward_refs_skipped++;
                        }
                    }
                } else {
                    mask_byte = tile_suppress_mask(type);
                }

                /* Read tile SUPPRESS data if mask is nonzero. */
                if (mask_byte != 0) {
                    uint8_t tile_data = dung_cb->get_tile(
                        dung_cb->ctx, x, y);
                    /* SKProject sksvgame.cpp:1277 passes the live tile
                     * address, not a zeroed scratch byte. Keep the
                     * unmasked tile bits and publish the decoded byte before
                     * its record chain is consumed. */
                    if (!dung_cb->set_tile) {
                        local.error = 1;
                        goto done;
                    }
                    if (dm2_suppress_reader_read_preserve(
                            &session->reader, &mask_byte, 1, &tile_data)) {
                        local.error = 1;
                        goto done;
                    }
                    if (dung_cb->set_tile(dung_cb->ctx, x, y, tile_data) != 0) {
                        local.error = 1;
                        goto done;
                    }
                }

                local.tiles_loaded++;

                /* SKProject sksvgame.cpp:1320-1399 distinguishes resident
                 * DB0..DB3 chains from an empty tile.  Only an empty tile
                 * calls READ_RECORD_CHECKCODE.  The former implementation
                 * decoded a new chain for every tile, consumed the wrong
                 * bits from a real SKSAVE stream, and discarded the source
                 * record owner. */
                if (!skip_record_chain) {
                    uint16_t root_link;
                    if (!dung_cb->get_tile_record_link) {
                        local.error = 1;
                        goto done;
                    }
                    root_link = dung_cb->get_tile_record_link(
                        dung_cb->ctx, x, y);
                    if (root_link == 0xfffeu) {
                        int rc = dm2_v1_read_record_checkcode(
                            session, rec_cb, &root_link, x, y, 1, 1);
                        if (rc != 0 || session->error ||
                            (root_link != 0xfffeu &&
                             !dung_cb->set_tile_record_link)) {
                            local.error = 1;
                            goto done;
                        }
                        if (root_link != 0xfffeu) {
                            dung_cb->set_tile_record_link(
                                dung_cb->ctx, x, y, root_link);
                        }
                    } else if (!dung_cb->restore_existing_tile_record_chain ||
                               dung_cb->restore_existing_tile_record_chain(
                                   dung_cb->ctx, session, root_link, x, y) != 0 ||
                               session->error) {
                        /* A partial loader may not reinterpret resident
                         * records as dynamic ones.  Fail closed until the
                         * GAME_LOAD transaction supplies this owner. */
                        local.error = 1;
                        goto done;
                    }
                    local.record_chains_loaded++;
                }
            }
        }
        local.maps_loaded++;
    }

    local.valid = 1;

done:
    dung_cb->change_current_map(dung_cb->ctx, restore_map);
    if (receipt) *receipt = local;
    return local.error ? 1 : 0;
}
