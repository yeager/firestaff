/* DM2 load orchestrator — top-level load pipeline.
 * Source: sksvgame.cpp:1415-1530 (DM2_GAME_LOAD).
 * Inverse of dm2_v1_save_orchestrate. */

#include "dm2_v1_load_orchestrator_pc34_compat.h"
#include <string.h>

static const uint8_t s_full_mask[4] = {0xFF, 0xFF, 0xFF, 0xFF};

int dm2_v1_load_orchestrate(
    const DM2_LoadOrchestratorCallbacks *cb,
    const uint8_t *in_buf, size_t in_size,
    DM2_LoadOrchestratorResult *result)
{
    DM2_LoadOrchestratorResult local;
    memset(&local, 0, sizeof(local));
    if (result) memset(result, 0, sizeof(*result));

    if (!cb || !in_buf || !result) {
        if (result) result->error = 1;
        return -1;
    }

    const uint8_t *record_sizes = dm2_v1_save_record_sizes();

    /* Phase 1: Raw header (0x2a bytes).
     * Source: sksvgame.cpp:1476. */
    {
        if (in_size < DM2_SAVE_HEADER_SIZE) { result->error = 2; return -1; }
        if (cb->set_header && cb->set_header(cb->ctx, in_buf) != 0) {
            result->error = 2; return -1;
        }
    }

    /* Phase 2: Read dungeon structure (sgwords + raw blocks + records + maps).
     * Source: sksvgame.cpp:1480 DM2_READ_DUNGEON_STRUCTURE.
     * The raw reading mirrors save orchestrator phases 1b-1e. */
    size_t raw_pos = DM2_SAVE_HEADER_SIZE;

    /* 2a. Savegame words (0x2c bytes). */
    {
        if (raw_pos + DM2_SAVE_SGWORDS_SIZE > in_size) {
            result->error = 3; return -1;
        }
        if (cb->set_sgwords &&
            cb->set_sgwords(cb->ctx, in_buf + raw_pos) != 0) {
            result->error = 3; return -1;
        }
        raw_pos += DM2_SAVE_SGWORDS_SIZE;
    }

    /* 2b. Raw data blocks. */
    /* Sizes come from sgwords fields — we need get_sgwords_field to be set. */
    /* For the load orchestrator, the raw blocks are read in order but their
     * sizes are embedded in the sgwords. The save orchestrator gets them
     * from callbacks. Here we let the caller tell us sizes via set_raw_block. */

    /* 2c. Record type arrays (0-15).
     * Source: sksvgame.cpp:2194-2200 (write), 1170-1182 (read). */
    for (int type = 0; type < DM2_SAVE_RECORD_TYPES; type++) {
        uint16_t count = cb->get_sgwords_field
            ? cb->get_sgwords_field(cb->ctx, type + 6)
            : 0;
        size_t total = (size_t)count * record_sizes[type];
        if (total > 0) {
            if (raw_pos + total > in_size) {
                result->error = 6; return -1;
            }
            if (cb->set_record_array &&
                cb->set_record_array(cb->ctx, type,
                                     in_buf + raw_pos, count) != 0) {
                result->error = 6; return -1;
            }
            raw_pos += total;
        }
    }

    /* Phase 3: SUPPRESS-encoded sections.
     * Source: sksvgame.cpp:1482-1517. */
    DM2_SuppressReader reader;
    dm2_suppress_reader_init(&reader, in_buf + raw_pos, in_size - raw_pos);

    /* Helper macro for SUPPRESS reads. */
    #define ORCH_SUPPRESS_READ(out_ptr, mask_ptr, item_size, item_count) do { \
        for (int _i = 0; _i < (int)(item_count); _i++) { \
            if (dm2_suppress_reader_read(&reader, (mask_ptr), (item_size), \
                    (out_ptr) + (_i * (item_size)), 0x00) != 0) { \
                result->error = 10; return -1; \
            } \
        } \
    } while (0)

    /* 3a. Savegame buffer (0x3c bytes, table1d631a mask).
     * Source: sksvgame.cpp:1483. */
    {
        DM2_SaveGameBuffer sgbuf;
        memset(&sgbuf, 0, sizeof(sgbuf));
        const uint8_t *mask = dm2_v1_save_mask_savegame_buffer();
        ORCH_SUPPRESS_READ((uint8_t *)&sgbuf, mask, 0x3c, 1);

        local.hero_count = sgbuf.heros_in_party;
        local.num_timers = sgbuf.num_timers;
        local.current_map = sgbuf.current_map;

        if (cb->receive_savegame_buffer &&
            cb->receive_savegame_buffer(cb->ctx, &sgbuf) != 0) {
            result->error = 8; return -1;
        }
    }

    /* 3b. v1e0104 (1 byte x 8, all-ones mask).
     * Source: sksvgame.cpp:1512. */
    {
        uint8_t v1e0104[8];
        memset(v1e0104, 0, sizeof(v1e0104));
        ORCH_SUPPRESS_READ(v1e0104, s_full_mask, 1, 8);
        if (cb->set_v1e0104 &&
            cb->set_v1e0104(cb->ctx, v1e0104) != 0) {
            result->error = 11; return -1;
        }
    }

    /* 3c. globalb (1 byte x 0x40, all-ones mask).
     * Source: sksvgame.cpp:1513. */
    {
        uint8_t globalb[0x40];
        memset(globalb, 0, sizeof(globalb));
        ORCH_SUPPRESS_READ(globalb, s_full_mask, 1, 0x40);
        if (cb->set_globalb &&
            cb->set_globalb(cb->ctx, globalb) != 0) {
            result->error = 12; return -1;
        }
    }

    /* 3d. globalw (2 bytes x 0x40, all-ones mask).
     * Source: sksvgame.cpp:1514. */
    {
        uint8_t globalw[0x80];
        memset(globalw, 0, sizeof(globalw));
        ORCH_SUPPRESS_READ(globalw, s_full_mask, 2, 0x40);
        if (cb->set_globalw &&
            cb->set_globalw(cb->ctx, globalw) != 0) {
            result->error = 13; return -1;
        }
    }

    /* 3e. Heroes (263 bytes x hero_count, table1d6356 mask).
     * Source: sksvgame.cpp:1515. */
    {
        const uint8_t *mask = dm2_v1_save_mask_hero();
        for (int h = 0; h < (int)local.hero_count; h++) {
            uint8_t hero[263];
            memset(hero, 0, sizeof(hero));
            ORCH_SUPPRESS_READ(hero, mask, 263, 1);
            if (cb->set_hero_data &&
                cb->set_hero_data(cb->ctx, h, hero) != 0) {
                result->error = 9; return -1;
            }
        }
    }

    /* 3f. Save state (6 bytes, table1d645d mask).
     * Source: sksvgame.cpp:1516. */
    {
        uint8_t save_state[6];
        memset(save_state, 0, sizeof(save_state));
        const uint8_t *mask = dm2_v1_save_mask_save_state();
        ORCH_SUPPRESS_READ(save_state, mask, 6, 1);
        if (cb->set_save_state &&
            cb->set_save_state(cb->ctx, save_state) != 0) {
            result->error = 14; return -1;
        }
    }

    /* 3g. Timers (entry_size x num_timers, vsgame mask).
     * Source: sksvgame.cpp:1517. */
    {
        size_t entry_size = cb->get_timer_entry_size
            ? cb->get_timer_entry_size(cb->ctx) : 12;
        const uint8_t *mask = dm2_v1_save_mask_timer();
        for (int t = 0; t < (int)local.num_timers; t++) {
            uint8_t timer[16];
            memset(timer, 0, sizeof(timer));
            if (entry_size > sizeof(timer)) entry_size = sizeof(timer);
            if (mask) {
                ORCH_SUPPRESS_READ(timer, mask, entry_size, 1);
            }
            if (cb->set_timer_entry &&
                cb->set_timer_entry(cb->ctx, t, timer, entry_size) != 0) {
                result->error = 15; return -1;
            }
        }
    }

    #undef ORCH_SUPPRESS_READ

    /* Phase 4: READ_SKSAVE_DUNGEON — hero inventory + tile record chains.
     * Source: sksvgame.cpp:1526.
     * The READ_RECORD_CHECKCODE session continues on the same SUPPRESS
     * stream. Hero inventory: 30 slots x hero_count, then wpc link.
     * Then LOAD_EXTRA_DUNGEON_DATA for tile chains. */
    {
        DM2_ReadRecordSession rrs;
        dm2_v1_read_record_session_init(&rrs,
            reader.data + reader.position,
            reader.size - reader.position);
        rrs.reader = reader;

        /* 4a. Hero inventory record chains.
         * Source: sksvgame.cpp:1186-1200. */
        for (int h = 0; h < (int)local.hero_count; h++) {
            for (int slot = 0; slot < DM2_SAVE_HERO_ITEM_SLOTS; slot++) {
                int rc = dm2_v1_read_record_checkcode(
                    &rrs, &cb->read_record_cb, NULL, -1, 0, 0, 0);
                if (rc != 0) { result->error = 16; return -1; }
            }
        }

        /* 4b. wpc link.
         * Source: sksvgame.cpp:1199. */
        {
            int rc = dm2_v1_read_record_checkcode(
                &rrs, &cb->read_record_cb, NULL, -1, 0, 0, 0);
            if (rc != 0) { result->error = 17; return -1; }
        }

        local.hero_items_loaded = 1;

        /* 4c. LOAD_EXTRA_DUNGEON_DATA.
         * Source: sksvgame.cpp:1229-1331 (tile walk + per-tile record chains). */
        {
            DM2_V1_LoadExtraDungeonReceipt dung_receipt;
            int rc = dm2_v1_load_extra_dungeon_data(
                &rrs, &cb->read_record_cb, &cb->dungeon_cb,
                (int)local.current_map, &dung_receipt);
            if (rc != 0) { result->error = 18; return -1; }
            local.dungeon_loaded = 1;
        }
    }

    local.valid = 1;
    *result = local;
    return 0;
}
