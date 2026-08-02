/* DM2 save orchestrator — top-level save pipeline.
 * Source: sksvgame.cpp:2086-2287 (DM2_GAME_SAVE_MENU).
 *
 * Sequences all save phases in the exact order of the original:
 * 1. Raw header + sgwords + data blocks + record arrays + map data
 * 2. SUPPRESS init
 * 3. SUPPRESS: savegame buffer (0x3c bytes, table1d631a mask)
 * 4. SUPPRESS: v1e0104 (1 byte × 8, all-ones mask)
 * 5. SUPPRESS: globalb (1 byte × 0x40, all-ones mask)
 * 6. SUPPRESS: globalw (2 bytes × 0x40, all-ones mask)
 * 7. SUPPRESS: heroes (263 bytes × hero_count, table1d6356 mask)
 * 8. SUPPRESS: save state (6 bytes, table1d645d mask)
 * 9. SUPPRESS: timers (timer_size × num_timers, vsgame mask)
 * 10. WRITE_RECORD_CHECKCODE per hero item slot (30 × hero_count)
 * 11. WRITE_RECORD_CHECKCODE for wpc link
 * 12. STORE_EXTRA_DUNGEON_DATA
 * 13. WRITE_POSSESSION_INDICES
 * 14. SUPPRESS flush */

#include "dm2_v1_save_orchestrator_pc34_compat.h"
#include <string.h>

/* All-ones mask for globalb/globalw/v1e0104 SUPPRESS sections.
 * Source: dm2data.cpp:1154 — v1d6316 initialized to {0xFF, 0xFF, 0xFF, 0xFF}. */
static const uint8_t s_full_mask[4] = {0xFF, 0xFF, 0xFF, 0xFF};

int dm2_v1_save_orchestrate(
    const DM2_SaveOrchestratorCallbacks *cb,
    uint8_t *out_buf, size_t out_cap,
    DM2_SaveOrchestratorResult *result)
{
    if (!cb || !out_buf || !result) return -1;
    memset(result, 0, sizeof(*result));

    const uint8_t *record_sizes = dm2_v1_save_record_sizes();
    int rc;

    /* Phase 1: Raw data writes.
     * Source: sksvgame.cpp:2181-2202. */

    /* 1a. Save header (0x2a bytes). */
    {
        uint8_t header[DM2_SAVE_HEADER_SIZE];
        if (cb->get_header(cb->ctx, header) != 0) { result->error = 1; return -1; }
        if (cb->write_raw(cb->ctx, header, DM2_SAVE_HEADER_SIZE) != 0) {
            result->error = 2; return -1;
        }
    }

    /* 1b. Savegame words (0x2c bytes). */
    {
        uint8_t sgwords[DM2_SAVE_SGWORDS_SIZE];
        if (cb->get_sgwords(cb->ctx, sgwords) != 0) { result->error = 3; return -1; }
        if (cb->write_raw(cb->ctx, sgwords, DM2_SAVE_SGWORDS_SIZE) != 0) {
            result->error = 4; return -1;
        }
    }

    /* 1c. Raw data blocks (v1e03c8, v1e03d8, dm2_v1e038c, v1e03d0).
     * Sizes come from sgwords fields. */
    for (int blk = 0; blk < 4; blk++) {
        size_t size = 0;
        const uint8_t *data = cb->get_raw_block(cb->ctx, blk, &size);
        if (size > 0 && data) {
            if (cb->write_raw(cb->ctx, data, size) != 0) {
                result->error = 5; return -1;
            }
        }
    }

    /* 1d. Record type arrays (0-15).
     * Source: sksvgame.cpp:2194-2200. */
    for (int type = 0; type < DM2_SAVE_RECORD_TYPES; type++) {
        size_t count = 0;
        const uint8_t *data = cb->get_record_array(cb->ctx, type, &count);
        size_t total = count * record_sizes[type];
        if (total > 0 && data) {
            if (cb->write_raw(cb->ctx, data, total) != 0) {
                result->error = 6; return -1;
            }
        }
    }

    /* 1e. Map data (v1e03e0). */
    {
        size_t size = 0;
        const uint8_t *data = cb->get_map_data(cb->ctx, &size);
        if (size > 0 && data) {
            if (cb->write_raw(cb->ctx, data, size) != 0) {
                result->error = 7; return -1;
            }
        }
    }

    /* Phase 2: SUPPRESS-encoded sections.
     * Source: sksvgame.cpp:2232-2246. */

    DM2_SuppressWriter writer;
    dm2_suppress_writer_init(&writer);
    size_t out_pos = 0;

    /* Helper: write SUPPRESS data into out_buf. */
    #define ORCH_SUPPRESS(data_ptr, mask_ptr, item_size, item_count) do { \
        for (int _i = 0; _i < (int)(item_count); _i++) { \
            size_t _w; \
            rc = dm2_suppress_writer_write(&writer, \
                (data_ptr) + (_i * (item_size)), \
                (mask_ptr), (item_size), \
                out_buf + out_pos, out_cap - out_pos, &_w); \
            if (rc != 0) { result->error = 10; return -1; } \
            out_pos += _w; \
        } \
    } while (0)

    /* 2a. Savegame buffer (0x3c bytes, table1d631a mask). */
    {
        DM2_SaveGameBuffer sgbuf;
        memset(&sgbuf, 0, sizeof(sgbuf));
        if (cb->fill_savegame_buffer(cb->ctx, &sgbuf) != 0) {
            result->error = 8; return -1;
        }
        const uint8_t *mask = dm2_v1_save_mask_savegame_buffer();
        ORCH_SUPPRESS((const uint8_t *)&sgbuf, mask, 0x3c, 1);
    }

    /* 2b. v1e0104 (1 byte × 8, all-ones mask).
     * Source: sksvgame.cpp:2235. */
    {
        const uint8_t *data = cb->get_v1e0104(cb->ctx);
        if (data) ORCH_SUPPRESS(data, s_full_mask, 1, 8);
    }

    /* 2c. globalb (1 byte × 0x40, all-ones mask).
     * Source: sksvgame.cpp:2237. */
    {
        const uint8_t *data = cb->get_globalb(cb->ctx);
        if (data) ORCH_SUPPRESS(data, s_full_mask, 1, 0x40);
    }

    /* 2d. globalw (2 bytes × 0x40, all-ones mask).
     * Source: sksvgame.cpp:2239. */
    {
        const uint8_t *data = cb->get_globalw(cb->ctx);
        if (data) ORCH_SUPPRESS(data, s_full_mask, 2, 0x40);
    }

    /* 2e. Heroes (263 bytes × hero_count, table1d6356 mask).
     * Source: sksvgame.cpp:2241. */
    {
        int hero_count = cb->get_hero_count(cb->ctx);
        const uint8_t *mask = dm2_v1_save_mask_hero();
        for (int h = 0; h < hero_count; h++) {
            const uint8_t *data = cb->get_hero_data(cb->ctx, h);
            if (!data) { result->error = 9; return -1; }
            ORCH_SUPPRESS(data, mask, 263, 1);
        }
    }

    /* 2f. Save state (6 bytes, table1d645d mask).
     * Source: sksvgame.cpp:2243. */
    {
        const uint8_t *data = cb->get_save_state(cb->ctx);
        if (data) {
            const uint8_t *mask = dm2_v1_save_mask_save_state();
            ORCH_SUPPRESS(data, mask, 6, 1);
        }
    }

    /* 2g. Timers (timer_size × num_timers, vsgame mask).
     * Source: sksvgame.cpp:2245. */
    {
        int timer_count = 0;
        const uint8_t *data = cb->get_timer_array(cb->ctx, &timer_count);
        if (data && timer_count > 0) {
            size_t entry_size = cb->get_timer_entry_size(cb->ctx);
            const uint8_t *mask = dm2_v1_save_mask_timer();
            if (mask) ORCH_SUPPRESS(data, mask, entry_size, timer_count);
        }
    }

    #undef ORCH_SUPPRESS

    /* Phase 3: WRITE_RECORD_CHECKCODE for hero inventory.
     * Source: sksvgame.cpp:2254-2274. */
    {
        int creature_indices[256], container_indices[256];
        DM2_WriteRecordSession wrs;
        dm2_v1_write_record_session_init(&wrs,
            out_buf + out_pos, out_cap - out_pos,
            creature_indices, 256, container_indices, 256,
            NULL, 0);

        /* Copy the writer state from the SUPPRESS section. */
        wrs.writer = writer;

        int hero_count = cb->get_hero_count(cb->ctx);
        DM2_WriteRecordCallbacks wcb = cb->write_record_cb;

        for (int h = 0; h < hero_count; h++) {
            for (int slot = 0; slot < DM2_SAVE_HERO_ITEM_SLOTS; slot++) {
                uint16_t link = cb->get_hero_item_link(cb->ctx, h, slot);
                rc = dm2_v1_write_record_checkcode(&wrs, &wcb, link, 0, 0);
                if (rc != 0) { result->error = 11; return -1; }
            }
        }

        /* wpc link. */
        {
            uint16_t wpc = cb->get_wpc_link(cb->ctx);
            rc = dm2_v1_write_record_checkcode(&wrs, &wcb, wpc, 0, 0);
            if (rc != 0) { result->error = 12; return -1; }
        }

        out_pos += wrs.out_written;
        writer = wrs.writer;
        result->creatures_written = wrs.creature_count;
        result->containers_written = wrs.container_count;
    }

    /* Phase 4: STORE_EXTRA_DUNGEON_DATA.
     * Source: sksvgame.cpp:2277-2278. */
    {
        DM2_WriteRecordSession wrs;
        int creature_indices[256], container_indices[256];
        dm2_v1_write_record_session_init(&wrs,
            out_buf + out_pos, out_cap - out_pos,
            creature_indices, 256, container_indices, 256,
            NULL, 0);
        wrs.writer = writer;

        DM2_WriteRecordCallbacks wcb = cb->write_record_cb;
        DM2_StoreExtraDungeonCallbacks dcb = cb->dungeon_cb;

        rc = dm2_v1_store_extra_dungeon_data(&wrs, &wcb, &dcb, -1);
        if (rc != 0) { result->error = 13; return -1; }

        out_pos += wrs.out_written;
        writer = wrs.writer;
    }

    /* Phase 5: WRITE_POSSESSION_INDICES.
     * Source: sksvgame.cpp:2279-2280. */
    {
        DM2_WriteRecordSession wrs;
        int ci[1], co[1];
        dm2_v1_write_record_session_init(&wrs,
            out_buf + out_pos, out_cap - out_pos,
            ci, 0, co, 0, NULL, 0);
        wrs.writer = writer;

        DM2_WritePossessionCallbacks pcb = cb->possession_cb;

        /* Count is from sgwords warr_00[0xf] (type 0xF record count). */
        int poss_count = cb->get_sgwords_field(cb->ctx, 0x0f);
        /* Possession links buffer is managed by caller via possession_cb. */

        /* The actual call needs the possession link array — but the
         * orchestrator doesn't own it. The possession callback resolves
         * indices directly. For the orchestrator, we pass through. */
        rc = dm2_v1_write_possession_indices(&wrs, &pcb, NULL, poss_count);
        if (rc != 0) { result->error = 14; return -1; }

        out_pos += wrs.out_written;
        writer = wrs.writer;
    }

    /* Phase 6: SUPPRESS flush.
     * Source: sksvgame.cpp:2281-2282. */
    {
        size_t flush_w;
        rc = dm2_suppress_writer_flush(&writer,
            out_buf + out_pos, out_cap - out_pos, &flush_w);
        if (rc != 0) { result->error = 15; return -1; }
        out_pos += flush_w;
    }

    result->suppress_bits_written = out_pos;
    return 0;
}
