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
#include "dm2_v1_save_timers_pc34_compat.h"
#include <string.h>

/* All-ones mask for globalb/globalw/v1e0104 SUPPRESS sections.
 * Source: dm2data.cpp:1154 — v1d6316 initialized to {0xFF, 0xFF, 0xFF, 0xFF}. */
static const uint8_t s_full_mask[4] = {0xFF, 0xFF, 0xFF, 0xFF};

/* c_savegame.cpp::DM2_READ_SKSAVE_DUNGEON and DM2_GAME_SAVE_MENU allocate
 * ddat.savegamep3 with 0xc8 bytes: exactly 100 source ObjectIDs.  The list
 * is populated in WRITE_RECORD_CHECKCODE and consumed only by
 * WRITE_POSSESSION_INDICES; it is not the DB15 pool count. */
enum { DM2_V1_SAVE_POSSESSION_LINK_CAPACITY = 100 };

/* DM2_GAME_SAVE_MENU compacts the live c_tim array before serialising it.
 * c_timer.h fixes every entry at 12 bytes; WRITE_RECORD_CHECKCODE needs just
 * the source type and valueB fields for 0x19/0x3c/0x3d ownership. */
static int dm2_v1_save_timer_links_from_raw(
    const uint8_t *raw, int count, size_t entry_size,
    DM2_WriteRecordTimer out[DM2_V1_SAVE_TIMER_MAX])
{
    if (count < 0 || ((count > 0) && !raw) ||
        entry_size != DM2_V1_SAVE_TIMER_RECORD_SIZE ||
        (unsigned)count > DM2_V1_SAVE_TIMER_MAX) return -1;

    for (int i = 0; i < count; ++i) {
        const uint8_t *entry = raw + (size_t)i * entry_size;
        out[i].type = entry[4]; /* c_timer.h:66 ttype */
        out[i].record_link = (uint16_t)entry[8] |
            ((uint16_t)entry[9] << 8); /* c_timer.h:88 getB */
    }
    return 0;
}

typedef struct {
    const DM2_WriteRecordCallbacks *source;
    uint16_t *links;
    size_t capacity;
    size_t count;
    int overflow;
} DM2_V1_SavePossessionCollector;

static int dm2_v1_save_collect_get_record(
    void *ctx, uint16_t link, DM2_WriteRecordData *out)
{
    DM2_V1_SavePossessionCollector *collector =
        (DM2_V1_SavePossessionCollector *)ctx;
    return !collector || !collector->source || !collector->source->get_record
        ? -1 : collector->source->get_record(collector->source->ctx, link, out);
}

static uint16_t dm2_v1_save_collect_get_next(void *ctx, uint16_t link)
{
    DM2_V1_SavePossessionCollector *collector =
        (DM2_V1_SavePossessionCollector *)ctx;
    return !collector || !collector->source || !collector->source->get_next_link
        ? DM2_RECORD_LINK_NONE
        : collector->source->get_next_link(collector->source->ctx, link);
}

static int dm2_v1_save_collect_query_ai(void *ctx, uint16_t link)
{
    DM2_V1_SavePossessionCollector *collector =
        (DM2_V1_SavePossessionCollector *)ctx;
    return !collector || !collector->source ||
        !collector->source->query_creature_ai_spec_flags ? 0
        : collector->source->query_creature_ai_spec_flags(collector->source->ctx,
                                                            link);
}

static int dm2_v1_save_collect_is_container_map(void *ctx, uint16_t link)
{
    DM2_V1_SavePossessionCollector *collector =
        (DM2_V1_SavePossessionCollector *)ctx;
    return !collector || !collector->source || !collector->source->is_container_map
        ? 0 : collector->source->is_container_map(collector->source->ctx, link);
}

static int dm2_v1_save_collect_is_moneybox(void *ctx, uint16_t link)
{
    DM2_V1_SavePossessionCollector *collector =
        (DM2_V1_SavePossessionCollector *)ctx;
    return !collector || !collector->source ||
        !collector->source->is_container_moneybox ? 0
        : collector->source->is_container_moneybox(collector->source->ctx, link);
}

static void dm2_v1_save_collect_possession(void *ctx, uint16_t link)
{
    DM2_V1_SavePossessionCollector *collector =
        (DM2_V1_SavePossessionCollector *)ctx;

    if (!collector || !collector->source) return;
    if (collector->count >= collector->capacity) {
        collector->overflow = 1;
    } else {
        collector->links[collector->count++] = link;
    }
    if (collector->source->add_possession_index) {
        collector->source->add_possession_index(collector->source->ctx, link);
    }
}

static void dm2_v1_save_collect_callbacks(
    DM2_WriteRecordCallbacks *out,
    DM2_V1_SavePossessionCollector *collector,
    const DM2_WriteRecordCallbacks *source)
{
    memset(out, 0, sizeof(*out));
    collector->source = source;
    out->get_record = dm2_v1_save_collect_get_record;
    out->get_next_link = dm2_v1_save_collect_get_next;
    out->query_creature_ai_spec_flags = dm2_v1_save_collect_query_ai;
    out->is_container_map = dm2_v1_save_collect_is_container_map;
    out->is_container_moneybox = dm2_v1_save_collect_is_moneybox;
    out->add_possession_index = dm2_v1_save_collect_possession;
    out->ctx = collector;
}

/* DM2_GAME_SAVE_MENU has no source-owned optional section.  A missing
 * callback is therefore an incomplete save graph, not an empty block.  Keep
 * the transaction fail-closed before the first header byte is emitted.
 * SKProject: SKWINSPX/src/v5/sksvgame.cpp:2181-2202, 2233-2282. */
static int dm2_v1_save_orchestrator_callbacks_valid(
    const DM2_SaveOrchestratorCallbacks *cb)
{
    return cb && cb->write_raw && cb->get_header && cb->get_sgwords &&
           cb->get_raw_block && cb->get_record_array && cb->get_map_data &&
           cb->fill_savegame_buffer && cb->get_globalb &&
           cb->get_v1e0104 && cb->get_globalw && cb->get_hero_data &&
           cb->get_hero_count && cb->get_save_state && cb->get_timer_array &&
           cb->get_timer_entry_size && cb->get_hero_item_link &&
           cb->get_wpc_link &&
           cb->write_record_cb.get_record &&
           cb->write_record_cb.get_next_link &&
           cb->write_record_cb.query_creature_ai_spec_flags &&
           cb->write_record_cb.is_container_map &&
           cb->write_record_cb.is_container_moneybox &&
           cb->write_record_cb.add_possession_index &&
           cb->dungeon_cb.change_current_map && cb->dungeon_cb.get_tile &&
           cb->dungeon_cb.get_record_link &&
           cb->dungeon_cb.get_teleporter_detail &&
           cb->dungeon_cb.get_map_count &&
           cb->dungeon_cb.get_map_dimensions &&
           cb->dungeon_cb.get_current_map &&
           cb->possession_cb.resolve_possession_index;
}

int dm2_v1_save_orchestrate(
    const DM2_SaveOrchestratorCallbacks *cb,
    uint8_t *out_buf, size_t out_cap,
    DM2_SaveOrchestratorResult *result)
{
    DM2_V1_SavePossessionCollector possession_collector;
    DM2_WriteRecordCallbacks record_callbacks;
    uint16_t possession_links[DM2_V1_SAVE_POSSESSION_LINK_CAPACITY];
    DM2_WriteRecordTimer timer_links[DM2_V1_SAVE_TIMER_MAX];
    const uint8_t *timer_data;
    int timer_count = 0;
    size_t timer_entry_size;
    if (!cb || !out_buf || !result) return -1;
    memset(result, 0, sizeof(*result));

    if (out_cap == 0u || !dm2_v1_save_orchestrator_callbacks_valid(cb)) {
        result->error = 100; /* source graph/output owner unavailable */
        return -1;
    }
    memset(&possession_collector, 0, sizeof(possession_collector));
    possession_collector.links = possession_links;
    possession_collector.capacity = sizeof(possession_links) /
        sizeof(possession_links[0]);
    dm2_v1_save_collect_callbacks(&record_callbacks, &possession_collector,
                                  &cb->write_record_cb);

    timer_data = cb->get_timer_array(cb->ctx, &timer_count);
    timer_entry_size = cb->get_timer_entry_size(cb->ctx);
    if (dm2_v1_save_timer_links_from_raw(timer_data, timer_count,
            timer_entry_size, timer_links) != 0) {
        result->error = 10;
        return -1;
    }

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
        if (size > 0 && !data) {
            result->error = 5; return -1;
        }
        if (size > 0) {
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
        if (total > 0 && !data) {
            result->error = 6; return -1;
        }
        if (total > 0) {
            if (cb->write_raw(cb->ctx, data, total) != 0) {
                result->error = 6; return -1;
            }
        }
    }

    /* 1e. Map data (v1e03e0). */
    {
        size_t size = 0;
        const uint8_t *data = cb->get_map_data(cb->ctx, &size);
        if (size > 0 && !data) {
            result->error = 7; return -1;
        }
        if (size > 0) {
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
        if (!data) { result->error = 10; return -1; }
        ORCH_SUPPRESS(data, s_full_mask, 1, 8);
    }

    /* 2c. globalb (1 byte × 0x40, all-ones mask).
     * Source: sksvgame.cpp:2237. */
    {
        const uint8_t *data = cb->get_globalb(cb->ctx);
        if (!data) { result->error = 10; return -1; }
        ORCH_SUPPRESS(data, s_full_mask, 1, 0x40);
    }

    /* 2d. globalw (2 bytes × 0x40, all-ones mask).
     * Source: sksvgame.cpp:2239. */
    {
        const uint8_t *data = cb->get_globalw(cb->ctx);
        if (!data) { result->error = 10; return -1; }
        ORCH_SUPPRESS(data, s_full_mask, 2, 0x40);
    }

    /* 2e. Heroes (263 bytes × hero_count, table1d6356 mask).
     * Source: sksvgame.cpp:2241. */
    {
        int hero_count = cb->get_hero_count(cb->ctx);
        if (hero_count < 0 || hero_count > DM2_SAVE_MAX_HEROES) {
            result->error = 9; return -1;
        }
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
        if (!data) { result->error = 10; return -1; }
        {
            const uint8_t *mask = dm2_v1_save_mask_save_state();
            ORCH_SUPPRESS(data, mask, 6, 1);
        }
    }

    /* 2g. Timers (timer_size × num_timers, vsgame mask).
     * Source: sksvgame.cpp:2245. */
    {
        if (timer_data && timer_count > 0) {
            const uint8_t *mask = dm2_v1_save_mask_timer();
            if (mask) ORCH_SUPPRESS(timer_data, mask, timer_entry_size,
                                    timer_count);
        }
    }

    #undef ORCH_SUPPRESS

    /* Phases 3–4 share one source session: index arrays, timer view and the
     * SUPPRESS bit state persist from hero roots through DM2_2066_0b44 and
     * STORE_EXTRA_DUNGEON_DATA. */
    {
        int creature_indices[256], container_indices[256];
        DM2_WriteRecordSession wrs;
        dm2_v1_write_record_session_init(&wrs,
            out_buf + out_pos, out_cap - out_pos,
            creature_indices, 256, container_indices, 256,
            timer_links, timer_count);

        /* Copy the writer state from the SUPPRESS section. */
        wrs.writer = writer;

        int hero_count = cb->get_hero_count(cb->ctx);
        DM2_WriteRecordCallbacks wcb = record_callbacks;

        for (int h = 0; h < hero_count; h++) {
            for (int slot = 0; slot < DM2_SAVE_HERO_ITEM_SLOTS; slot++) {
                uint16_t link = cb->get_hero_item_link(cb->ctx, h, slot);
                rc = dm2_v1_write_record_checkcode(&wrs, &wcb, link, 0, 0);
                if (rc != 0 || possession_collector.overflow) {
                    result->error = 11; return -1;
                }
            }
        }

        /* wpc link. */
        {
            uint16_t wpc = cb->get_wpc_link(cb->ctx);
            rc = dm2_v1_write_record_checkcode(&wrs, &wcb, wpc, 0, 0);
            if (rc != 0 || possession_collector.overflow) {
                result->error = 12; return -1;
            }
        }

        /* Phase 4: DM2_2066_0b44 then STORE_EXTRA_DUNGEON_DATA.
         * Source: c_savegame.cpp:1942-2041, 2277-2278. */
        DM2_StoreExtraDungeonCallbacks dcb = cb->dungeon_cb;

        rc = dm2_v1_store_extra_dungeon_data(&wrs, &wcb, &dcb,
                                              dcb.get_current_map(dcb.ctx));
        if (rc != 0 || possession_collector.overflow) {
            result->error = 13; return -1;
        }

        out_pos += wrs.out_written;
        writer = wrs.writer;
        result->creatures_written = wrs.creature_count;
        result->containers_written = wrs.container_count;
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

        /* Source writes the ordered savegamep3 list accumulated by the two
         * preceding WRITE_RECORD_CHECKCODE passes. `warr_00[0xf]` is merely
         * the DB15 pool capacity and is unrelated to this list. */
        rc = dm2_v1_write_possession_indices(&wrs, &pcb, possession_links,
            (int)possession_collector.count);
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
