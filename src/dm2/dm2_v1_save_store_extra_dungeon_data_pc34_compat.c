/* DM2 STORE_EXTRA_DUNGEON_DATA — per-map tile walker + record chain serializer.
 * Source: sksvgame.cpp:1958-2041. */

#include "dm2_v1_save_store_extra_dungeon_data_pc34_compat.h"

/* Tile type is bits 7:5 of the tile byte. */
static int tile_type(uint8_t tile) { return (tile >> 5) & 0x7; }

/* Tile has records if bit 4 is set. */
static int tile_has_records(uint8_t tile) { return (tile & 0x10) != 0; }

/* Compute the SUPPRESS mask byte for a tile based on its type.
 * Source: sksvgame.cpp:1996-2023 switch. */
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

int dm2_v1_store_extra_dungeon_data(
    DM2_WriteRecordSession *session,
    const DM2_WriteRecordCallbacks *rec_cb,
    const DM2_StoreExtraDungeonCallbacks *dung_cb,
    int restore_map)
{
    if (!session || !rec_cb || !dung_cb || !dung_cb->change_current_map ||
        !dung_cb->get_tile || !dung_cb->get_record_link ||
        !dung_cb->get_map_count || !dung_cb->get_map_dimensions ||
        !dung_cb->get_current_map || session->timer_count < 0 ||
        (session->timer_count > 0 && !session->timers)) return -1;

    /* c_savegame.cpp::DM2_2066_0b44 precedes STORE_EXTRA_DUNGEON_DATA.
     * Special type-0x3c/0x3d timers write their valueB roots through the
     * same record session as hero and map chains. */
    for (int i = 0; i < session->timer_count; ++i) {
        const DM2_WriteRecordTimer *timer = &session->timers[i];
        if (timer->type == 0x3cu || timer->type == 0x3du) {
            int rc = dm2_v1_write_record_checkcode(session, rec_cb,
                timer->record_link, 0, 0);
            if (rc != 0) return rc;
        }
    }

    if (restore_map < 0)
        restore_map = dung_cb->get_current_map(dung_cb->ctx);
    if (restore_map < 0) return -1;

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

                /* Type 5 (teleporter): check destination map for backward ref. */
                uint8_t mask_byte;
                if (type == 5) {
                    DM2_TeleporterDetail detail;
                    if (dung_cb->get_teleporter_detail == NULL ||
                        dung_cb->get_teleporter_detail(dung_cb->ctx, &detail, x, y) == 0) {
                        mask_byte = 0x08;
                    } else {
                        mask_byte = 0;
                        if ((unsigned)map_idx > (unsigned)detail.bytes[4])
                            skip_record_chain = 1;
                    }
                } else {
                    mask_byte = tile_suppress_mask(type);
                }

                /* Write tile SUPPRESS mask if nonzero. */
                if (mask_byte != 0) {
                    size_t w;
                    if (session->out_written >= session->out_cap) return 1;
                    if (dm2_suppress_writer_write(&session->writer,
                            &tile, &mask_byte, 1,
                            session->out_buf + session->out_written,
                            session->out_cap - session->out_written, &w) != 0)
                        return 1;
                    session->out_written += w;
                }

                /* Get record link for this tile. */
                uint16_t rec_link;
                if (!tile_has_records(tile))
                    rec_link = DM2_RECORD_LINK_NONE;
                else
                    rec_link = dung_cb->get_record_link(dung_cb->ctx, x, y);

                /* Write the record chain unless skipped (teleporter backward ref). */
                if (!skip_record_chain) {
                    int rc = dm2_v1_write_record_checkcode(
                        session, rec_cb, rec_link, 1, 1);
                    if (rc != 0) return rc;
                }
            }
        }
    }

    dung_cb->change_current_map(dung_cb->ctx, restore_map);
    return 0;
}
