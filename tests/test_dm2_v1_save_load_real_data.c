/*
 * Real PC-DOS DM2 SKSave corpus regression.
 *
 * SKProject GAME_LOAD reads the 42-byte SKSave container before the raw
 * saved-dungeon prefix.  These checks deliberately retain only those proven
 * boundaries; they do not invent champion names or promote an incomplete
 * SUPPRESS tail into a playable session.
 */

#include "dm2_v1_new_game.h"
#include "dm2_v1_game.h"
#include "dm2_v1_asset_loader.h"
#include "dm2_v1_creature.h"
#include "dm2_v1_runtime.h"
#include "dm2_v1_record_pool_pc34_compat.h"
#include "dm2_v1_sksave_game_load_owner.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_item_ops_pc34_compat.h"
#include "dm2_v1_save_read_record_checkcode_pc34_compat.h"
#include "dm2_v1_save_load.h"
#include "dm2_v1_startup_menu.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int passed;
static int failed;

/* Read-only owner for SKProject c_savegame.cpp::READ_SKSAVE_DUNGEON's
 * champion-item and party-hand chains.  It deliberately has no dungeon,
 * possession, timer or runtime callback: this corpus test can prove byte
 * consumption and record types, but it cannot publish a resumable session. */
typedef struct {
    uint16_t next_index[16];
    uint16_t next_link[16][256];
    uint16_t child_link[16][256];
    uint16_t record_links[4096];
    unsigned int record_count;
    uint32_t record_hash;
    int creature_ai_unavailable;
    uint8_t unavailable_creature_type;
} RecordChainInventory;

typedef struct {
    unsigned int decoded;
    unsigned int blocked_missing_ai_mapping;
    unsigned int malformed;
    unsigned int resurrection_timers;
    unsigned int files_with_resurrection_timers;
    unsigned int pool_owner_restored;
    unsigned int pool_owner_blocked;
    unsigned int game_load_owner_materialized;
    unsigned int game_load_owner_blocked;
} DirectRootStats;

#define CHECK(condition, message) do { \
    if (condition) { ++passed; printf("  PASS: %s\n", message); } \
    else { ++failed; printf("  FAIL: %s\n", message); } \
} while (0)

static uint32_t hash_bytes(uint32_t hash, const uint8_t *bytes, size_t size)
{
    size_t i;
    for (i = 0u; i < size; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

static uint16_t inventory_alloc_record(void *context, int record_type)
{
    RecordChainInventory *inventory = (RecordChainInventory *)context;
    uint16_t index;

    if (!inventory || record_type < 0 || record_type >= 16) return 0xfffeu;
    index = inventory->next_index[record_type];
    if (index >= 256u) return 0xfffeu;
    inventory->next_index[record_type] = (uint16_t)(index + 1u);
    return (uint16_t)(((uint16_t)record_type << 10) | index);
}

static int inventory_set_record(void *context, uint16_t record_link,
                                const uint8_t *data, size_t size)
{
    RecordChainInventory *inventory = (RecordChainInventory *)context;
    const uint8_t record_type = (uint8_t)((record_link >> 10) & 0x0fu);

    if (!inventory || !data || record_type >= 16u) return -1;
    inventory->record_hash = hash_bytes(inventory->record_hash,
                                        &record_type, sizeof(record_type));
    inventory->record_hash = hash_bytes(inventory->record_hash, data, size);
    if (inventory->record_count >=
        sizeof(inventory->record_links) / sizeof(inventory->record_links[0])) {
        return -1;
    }
    inventory->record_links[inventory->record_count] = record_link;
    ++inventory->record_count;
    return 0;
}

static int inventory_append_record(void *context, uint16_t next,
                                   uint16_t *owner, int map_x, int map_y)
{
    RecordChainInventory *inventory = (RecordChainInventory *)context;
    uint16_t *tail;
    uint16_t type = (uint16_t)((next >> 10) & 0x0fu);
    uint16_t index = (uint16_t)(next & 0x03ffu);

    /* This diagnostic owns only direct SKSAVE roots, never a map tile. */
    if (!inventory || !owner || map_x != -1 || map_y != 0 || index >= 256u)
        return -1;
    inventory->next_link[type][index] = 0xfffeu;
    tail = owner;
    while (*tail != 0xfffeu) {
        type = (uint16_t)((*tail >> 10) & 0x0fu);
        index = (uint16_t)(*tail & 0x03ffu);
        if (index >= 256u) return -1;
        tail = &inventory->next_link[type][index];
    }
    *tail = next;
    return 0;
}

static int inventory_child_owner(void *context, uint16_t link,
                                 uint16_t **out_owner)
{
    RecordChainInventory *inventory = (RecordChainInventory *)context;
    const uint16_t type = (uint16_t)((link >> 10) & 0x0fu);
    const uint16_t index = (uint16_t)(link & 0x03ffu);
    if (!inventory || !out_owner || index >= 256u) return -1;
    *out_owner = &inventory->child_link[type][index];
    return 0;
}

static void inventory_add_possession(void *context, uint16_t record_link)
{
    (void)context;
    (void)record_link;
}

static int inventory_query_creature_ai_flags(void *context,
                                             uint16_t record_link,
                                             uint8_t creature_type,
                                             uint16_t *out_flags)
{
    RecordChainInventory *inventory = (RecordChainInventory *)context;
    (void)record_link;
    /* SKProject c_dm2data::init has already loaded the retail v1d296c
     * table. c_record.cpp::DM2_QUERY_CREATURE_AI_SPEC_FROM_RECORD then uses
     * CREATURES[type] word 5 to select the row. This test initialises that
     * exact pair from the mounted PC-DOS media before touching SKSAVE. */
    if (!dm2_v1_creature_ai_spec_flags(creature_type, out_flags)) {
        if (inventory) {
            inventory->creature_ai_unavailable = 1;
            inventory->unavailable_creature_type = creature_type;
        }
        return -1;
    }
    return 0;
}

/* 1 = every root decoded; 2 = stopped exactly at the missing source AI
 * provider; 0 = malformed/unrelated failure. */
static int verify_real_direct_record_roots(
    const uint8_t *payload, size_t payload_size,
    const DM2_V1_OriginalRawSaveStateReceipt *state)
{
    DM2_ReadRecordSession reader;
    DM2_ReadRecordCallbacks callbacks;
    RecordChainInventory inventory;
    unsigned int root;
    const size_t root_count = (size_t)state->champion_count * 30u + 1u;

    if (!payload || !state || !state->valid ||
        state->record_link_bitstream_offset > payload_size ||
        state->record_link_bitstream_bits_remaining > 7u ||
        root_count > 121u) {
        return 0;
    }
    memset(&reader, 0, sizeof(reader));
    dm2_v1_read_record_session_init(&reader, payload, payload_size);
    reader.reader.position = state->record_link_bitstream_offset;
    reader.reader.bits_remaining = state->record_link_bitstream_bits_remaining;
    if (reader.reader.bits_remaining != 0u) {
        if (reader.reader.position == 0u) return 0;
        reader.reader.current_byte = (uint8_t)(
            payload[reader.reader.position - 1u] <<
            (8u - reader.reader.bits_remaining));
    }
    memset(&inventory, 0, sizeof(inventory));
    inventory.record_hash = 2166136261u;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.alloc_record = inventory_alloc_record;
    callbacks.set_data = inventory_set_record;
    callbacks.append_record = inventory_append_record;
    callbacks.child_owner = inventory_child_owner;
    callbacks.add_possession_index = inventory_add_possession;
    callbacks.query_creature_ai_flags = inventory_query_creature_ai_flags;
    callbacks.ctx = &inventory;
    for (root = 0u; root < root_count; ++root) {
        uint16_t root_link = 0xfffeu;
        if (dm2_v1_read_record_checkcode(&reader, &callbacks, &root_link,
                                         -1, 0, 0, 0) != 0 ||
            reader.error) {
            printf("  direct-root diagnostic: root=%u records=%d ai_unavailable=%d creature=%u reader_error=%d\n",
                   root, reader.records_read,
                   inventory.creature_ai_unavailable,
                   (unsigned)inventory.unavailable_creature_type,
                   reader.error);
            return inventory.creature_ai_unavailable ? 2 : 0;
        }
    }
    /* DM2_2066_062b reads possession continuations only after special timer
     * chains and all map chains (SKProject sksvgame.cpp:1178-1400). Stopping
     * at the direct-root boundary proves this phase without consuming later
     * source bits as a fabricated continuation stream. */
    printf("  direct-root receipt: records=%u\n", inventory.record_count);
    /* A no-record chain is valid. The hashes capture only source bytes that
     * were genuinely decoded; neither receipt fabricates a live record pool. */
    return reader.reader.position <= payload_size &&
           inventory.record_hash != 0u ? 1 : 0;
}

static int resolve_corpus_root(char *out, size_t out_size)
{
    const char *explicit_root = getenv("FIRESTAFF_DM2_SKSAVE_CORPUS");
    const char *data_root = getenv("FIRESTAFF_DM2_DATA_DIR");

    if (!out || out_size == 0u) return 0;
    out[0] = '\0';
    if (explicit_root && explicit_root[0]) {
        snprintf(out, out_size, "%s", explicit_root);
        return 1;
    }
    if (data_root && data_root[0]) {
        snprintf(out, out_size, "%s", data_root);
        return 1;
    }
    return 0;
}

static uint8_t *read_file(const char *path, size_t *out_size)
{
    FILE *file;
    long end;
    uint8_t *bytes;

    if (out_size) *out_size = 0u;
    if (!path || !out_size || !(file = fopen(path, "rb"))) return NULL;
    if (fseek(file, 0, SEEK_END) != 0 || (end = ftell(file)) <= 42L ||
        fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }
    bytes = (uint8_t *)malloc((size_t)end);
    if (!bytes || fread(bytes, 1u, (size_t)end, file) != (size_t)end) {
        free(bytes);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *out_size = (size_t)end;
    return bytes;
}

static int load_real_creature_ai_table(const char *root,
                                       int *out_type54_absent,
                                       int *out_type127_absent)
{
    char path[600];
    uint8_t *bytes;
    size_t byte_count;
    DM2_V1_AssetLoader loader;
    int mapped;

    if (out_type54_absent) *out_type54_absent = 0;
    if (out_type127_absent) *out_type127_absent = 0;
    if (!root || !root[0]) return 0;
    snprintf(path, sizeof(path), "%s/graphics.dat", root);
    bytes = read_file(path, &byte_count);
    if (!bytes) return 0;
    memset(&loader, 0, sizeof(loader));
    if (dm2_v1_asset_loader_init(&loader, bytes, byte_count) != 0) {
        free(bytes);
        return 0;
    }
    /* c_querydb.cpp::DM2_QUERY_GDAT_CREATURE_WORD_VALUE falls back to this
     * exact category/index/dtWordValue query after its short cache.  The
     * source query's scalar result for an absent word is zero, which selects
     * authenticated table1d296c[0]; retain the raw absence as a regression
     * receipt rather than inventing a GDAT row. */
    {
        uint16_t ignored = 0u;
        if (out_type54_absent) {
            *out_type54_absent = !dm2_v1_asset_load_word_value(
                &loader, DM2_GDAT_CATEGORY_CREATURES, 54, 0x05, &ignored);
        }
        if (out_type127_absent) {
            *out_type127_absent = !dm2_v1_asset_load_word_value(
                &loader, DM2_GDAT_CATEGORY_CREATURES, 127, 0x05, &ignored);
        }
    }
    mapped = dm2_v1_creature_load_ai_table_from_gdat(&loader);
    dm2_v1_asset_loader_free(&loader);
    free(bytes);
    return mapped > 0;
}

static int verify_real_db_pool_receipts(const uint8_t *payload,
                                        size_t payload_size,
                                        const DM2_V1_OriginalRawDungeonReceipt *dungeon)
{
    int ok = 1;
    int pool;

    if (!payload || !dungeon || !dungeon->valid) return 0;
    /* SKProject SKWIN/DME.h fixes these sixteen DB pools and their record
     * widths.  Verify both ends of every non-empty pool from the supplied
     * DOS corpus, rather than accepting a generated record graph. */
    for (pool = 0; pool < DM2_RAW_SKSAVE_DB_POOL_COUNT; ++pool) {
        DM2_V1_OriginalRawDbRecordReceipt first;
        DM2_V1_OriginalRawDbRecordReceipt last;
        const uint16_t count = dungeon->db_record_counts[pool];

        if (count == 0u) continue;
        memset(&first, 0, sizeof(first));
        memset(&last, 0, sizeof(last));
        if (!dm2_v1_original_raw_sksave_db_record_receipt(
                payload, payload_size, pool, 0, &first) ||
            !dm2_v1_original_raw_sksave_db_record_receipt(
                payload, payload_size, pool, (int)count - 1, &last) ||
            !first.valid || !last.valid || first.db_pool != (uint8_t)pool ||
            last.db_pool != (uint8_t)pool || first.record_index != 0u ||
            last.record_index != count - 1u || first.record_size == 0u ||
            first.record_size != last.record_size ||
            first.record_offset != dungeon->db_pool_offsets[pool] ||
            last.record_offset + last.record_size > dungeon->suppress_state_offset ||
            first.record_hash == 0u || last.record_hash == 0u) {
            ok = 0;
        }
    }
    return ok;
}

static int verify_real_raw_dungeon_model(
    const uint8_t *payload, size_t payload_size,
    const DM2_V1_OriginalRawDungeonReceipt *receipt)
{
    DM2_V1_DungeonData dungeon;
    int map;

    if (!payload || !receipt || !receipt->valid ||
        receipt->suppress_state_offset > payload_size) return 0;
    memset(&dungeon, 0, sizeof(dungeon));
    if (dm2_v1_dungeon_load(&dungeon, payload,
                            (int)receipt->suppress_state_offset) != 0 ||
        dungeon.square_bytes != 1 || dungeon.g1_extension_base >= 0 ||
        dungeon.level_count != (int)receipt->map_count ||
        dungeon.raw_size != (int)receipt->suppress_state_offset) {
        dm2_v1_dungeon_free(&dungeon);
        return 0;
    }
    for (map = 0; map < dungeon.level_count; ++map) {
        if (dungeon.level_widths[map] != (int)receipt->map_widths[map] ||
            dungeon.level_heights[map] != (int)receipt->map_heights[map] ||
            dm2_v1_dungeon_get_tile_raw(&dungeon, map, 0, 0) < 0 ||
            dm2_v1_dungeon_get_tile_raw(
                &dungeon, map, dungeon.level_widths[map] - 1,
                dungeon.level_heights[map] - 1) < 0) {
            dm2_v1_dungeon_free(&dungeon);
            return 0;
        }
    }
    dm2_v1_dungeon_free(&dungeon);
    return 1;
}

/* READ_DUNGEON_STRUCTURE owns the raw DB baseline before GAME_LOAD begins
 * its shared SUPPRESS stream.  Verify the production pool owner against
 * every actual DOS SKSave body, not a hand-built record fixture.  This stays
 * deliberately before READ_SKSAVE_DUNGEON: a copied baseline is not a
 * resumed record graph. */
static int verify_real_raw_pool_baseline(
    const uint8_t *payload,
    size_t payload_size,
    const DM2_V1_OriginalRawDungeonReceipt *dungeon)
{
    DM2_V1_RecordPoolSet pools;
    DM2_V1_SksaveMapOwner map_owner;
    DM2_V1_SksaveMapRestoreContext map_context;
    DM2_V1_RecordPoolSet rejected;
    uint8_t *corrupt = NULL;
    int pool;

    if (!payload || !dungeon || !dungeon->valid) return 0;
    memset(&pools, 0, sizeof(pools));
    memset(&map_owner, 0, sizeof(map_owner));
    memset(&map_context, 0, sizeof(map_context));
    if (!dm2_v1_record_pool_set_init_from_raw_sksave(
            &pools, payload, payload_size, dungeon) || !pools.valid ||
        pools.record_graph_complete != 0) {
        dm2_v1_record_pool_set_free(&pools);
        return 0;
    }

    for (pool = 0; pool < DM2_RAW_SKSAVE_DB_POOL_COUNT; ++pool) {
        const int record_size = dm2_v1_record_pool_record_size(pool);
        const uint16_t count = dungeon->db_record_counts[pool];
        const DM2_V1_RecordPool *source = &pools.pools[pool];
        const uint8_t *first;
        const uint8_t *last;
        const size_t bytes = (size_t)count * (size_t)record_size;

        if (source->record_size != record_size ||
            source->record_count != (int)count ||
            source->extension_bytes != NULL || source->extension_count != 0 ||
            source->extension_base != -1) {
            dm2_v1_record_pool_set_free(&pools);
            return 0;
        }
        if (record_size == 0) {
            if (count != 0u || source->bytes != NULL) {
                dm2_v1_record_pool_set_free(&pools);
                return 0;
            }
            continue;
        }
        if (count == 0u) {
            if (source->bytes != NULL || source->source_base != -1) {
                dm2_v1_record_pool_set_free(&pools);
                return 0;
            }
            continue;
        }
        first = dm2_v1_record_pool_address(
            &pools, (int16_t)((uint16_t)pool << 10));
        last = dm2_v1_record_pool_address(
            &pools, (int16_t)(((uint16_t)pool << 10) | (count - 1u)));
        if (source->source_base != (int)dungeon->db_pool_offsets[pool] ||
            !first || !last ||
            memcmp(first, payload + dungeon->db_pool_offsets[pool],
                   (size_t)record_size) != 0 ||
            memcmp(last, payload + dungeon->db_pool_offsets[pool] +
                   bytes - (size_t)record_size,
                   (size_t)record_size) != 0) {
            dm2_v1_record_pool_set_free(&pools);
            return 0;
        }
    }

    /* sksvgame.cpp first severs DB4..DB15 records from the real c_map
     * ground-stack chains. Only then can it clear dynamic DB first words.
     * Verify that sequence against each mounted SKSAVE, not against a
     * fabricated tile owner. */
    if (!dm2_v1_sksave_map_owner_init(
            &map_owner, payload, payload_size, dungeon) ||
        !dm2_v1_sksave_map_restore_context_init(
            &map_context, &map_owner, &pools) ||
        dm2_v1_sksave_map_restore_get_map_count(&map_context) !=
            (int)dungeon->map_count ||
        dm2_v1_sksave_map_owner_get_map_count(&map_owner) !=
            (int)dungeon->map_count ||
        dm2_v1_sksave_map_owner_get_tile(&map_owner, 0, 0) !=
            payload[(size_t)dungeon->map_data_base +
                    dungeon->map_data_relative_offsets[0]] ||
        dm2_v1_sksave_map_owner_set_tile(
            &map_owner, 0, 0,
            (uint8_t)(dm2_v1_sksave_map_owner_get_tile(&map_owner, 0, 0) ^
                      0x01u)) != 0 ||
        payload[(size_t)dungeon->map_data_base +
                dungeon->map_data_relative_offsets[0]] !=
            (uint8_t)(dm2_v1_sksave_map_owner_get_tile(
                &map_owner, 0, 0) ^ 0x01u) ||
        dm2_v1_sksave_map_owner_set_tile(
            &map_owner, 0, 0,
            payload[(size_t)dungeon->map_data_base +
                    dungeon->map_data_relative_offsets[0]]) != 0 ||
        !dm2_v1_sksave_map_owner_detach_dynamic_records(
            &map_owner, &pools, NULL) ||
        !dm2_v1_record_pool_clear_raw_sksave_dynamic_records(
            &pools, dungeon)) {
        dm2_v1_sksave_map_owner_free(&map_owner);
        dm2_v1_record_pool_set_free(&pools);
        return 0;
    }
    {
        int resident_only = 1;
        int map;
        for (map = 0; map < (int)dungeon->map_count && resident_only; ++map) {
            int x;
            for (x = 0; x < (int)dungeon->map_widths[map] && resident_only; ++x) {
                int y;
                for (y = 0; y < (int)dungeon->map_heights[map]; ++y) {
                    uint16_t link = 0xfffeu;
                    if (!dm2_v1_sksave_map_owner_tile_record_link(
                            &map_owner, map, x, y, &link) ||
                        (link != 0xfffeu && link != 0xffffu &&
                         dm2_v1_record_handle_pool((int16_t)link) >= 4)) {
                        resident_only = 0;
                        break;
                    }
                }
            }
        }
        if (!resident_only) {
            dm2_v1_sksave_map_owner_free(&map_owner);
            dm2_v1_record_pool_set_free(&pools);
            return 0;
        }
    }
    for (pool = 0; pool < DM2_RAW_SKSAVE_DB_POOL_COUNT; ++pool) {
        const int record_size = dm2_v1_record_pool_record_size(pool);
        const uint16_t count = dungeon->db_record_counts[pool];
        const DM2_V1_RecordPool *source = &pools.pools[pool];
        uint16_t index;

        if (record_size == 0 || count == 0u) continue;
        for (index = 0u; index < count; ++index) {
            const uint8_t *record = source->bytes +
                (size_t)index * (size_t)record_size;
            const uint8_t *original = payload + dungeon->db_pool_offsets[pool] +
                (size_t)index * (size_t)record_size;
            /* The source map walk may splice a DB4+ successor out of a
             * resident DB0..DB3 record, so only the link word may differ in
             * a resident record. Its payload must remain byte-identical. */
            if ((pool < 4 && record_size > 2 && memcmp(record + 2u,
                                    original + 2u,
                                    (size_t)record_size - 2u) != 0) ||
                (pool >= 4 &&
                 (record[0] != 0xffu || record[1] != 0xffu ||
                  (record_size > 2 && memcmp(record + 2u, original + 2u,
                                              (size_t)record_size - 2u) != 0)))) {
                dm2_v1_sksave_map_owner_free(&map_owner);
                dm2_v1_record_pool_set_free(&pools);
                return 0;
            }
        }
    }
    {
        DM2_V1_OriginalRawDungeonReceipt mismatched = *dungeon;
        uint8_t before[2];
        int probe_pool = 0;
        while (probe_pool < 4 &&
               dungeon->db_record_counts[probe_pool] == 0u) {
            ++probe_pool;
        }
        if (probe_pool >= 4) {
            dm2_v1_sksave_map_owner_free(&map_owner);
            dm2_v1_record_pool_set_free(&pools);
            return 0;
        }
        memcpy(before, pools.pools[probe_pool].bytes, sizeof(before));
        ++mismatched.db_record_counts[probe_pool];
        if (dm2_v1_record_pool_clear_raw_sksave_dynamic_records(
                &pools, &mismatched) != 0 ||
            memcmp(before, pools.pools[probe_pool].bytes,
                   sizeof(before)) != 0) {
            dm2_v1_sksave_map_owner_free(&map_owner);
            dm2_v1_record_pool_set_free(&pools);
            return 0;
        }
    }

    /* The body must be bound to the receipt hashes before allocation.  Flip
     * a byte in a copied real body only to prove the negative admission
     * boundary; production still receives the untouched original file. */
    for (pool = 0; pool < DM2_RAW_SKSAVE_DB_POOL_COUNT; ++pool) {
        if (dungeon->db_record_counts[pool] != 0u) break;
    }
    if (pool == DM2_RAW_SKSAVE_DB_POOL_COUNT) {
        dm2_v1_sksave_map_owner_free(&map_owner);
        dm2_v1_record_pool_set_free(&pools);
        return 0;
    }
    corrupt = (uint8_t *)malloc(payload_size);
    if (!corrupt) {
        dm2_v1_sksave_map_owner_free(&map_owner);
        dm2_v1_record_pool_set_free(&pools);
        return 0;
    }
    memcpy(corrupt, payload, payload_size);
    corrupt[dungeon->db_pool_offsets[pool]] ^= 0x80u;
    memset(&rejected, 0xa5, sizeof(rejected));
    if (dm2_v1_record_pool_set_init_from_raw_sksave(
            &rejected, corrupt, payload_size, dungeon) != 0 ||
        rejected.valid != 0 || rejected.record_graph_complete != 0 ||
        rejected.pools[pool].bytes != NULL) {
        free(corrupt);
        dm2_v1_sksave_map_owner_free(&map_owner);
        dm2_v1_record_pool_set_free(&pools);
        dm2_v1_record_pool_set_free(&rejected);
        return 0;
    }
    free(corrupt);
    dm2_v1_sksave_map_owner_free(&map_owner);
    dm2_v1_record_pool_set_free(&pools);
    return 1;
}

static int verify_real_pool_direct_roots(
    const uint8_t *payload, size_t payload_size,
    const DM2_V1_OriginalRawSaveStateReceipt *state, const char *root)
{
    DM2_V1_RecordPoolSet pools;
    DM2_V1_SksaveMapOwner map_owner;
    DM2_V1_SksaveDirectRootReceipt receipt;
    DM2_V1_SksaveItemBonusReceipt item_bonus;
    DM2_V1_Hero heroes[DM2_MAX_HEROES];
    DM2_V1_AssetLoader loader;
    uint8_t *graphics = NULL;
    size_t graphics_size = 0u;
    uint16_t leader_hand = 0xfffeu;
    uint32_t root_hash = 0u;
    char graphics_path[600];
    int ok;

    if (!payload || !state || !state->valid || !root || !root[0]) return 0;
    memset(&pools, 0, sizeof(pools));
    memset(&map_owner, 0, sizeof(map_owner));
    memset(heroes, 0, sizeof(heroes));
    memset(&loader, 0, sizeof(loader));
    if (!dm2_v1_record_pool_set_init_from_raw_sksave(
            &pools, payload, payload_size, &state->dungeon) ||
        !dm2_v1_sksave_map_owner_init(
            &map_owner, payload, payload_size, &state->dungeon) ||
        !dm2_v1_sksave_map_owner_detach_dynamic_records(
            &map_owner, &pools, NULL) ||
        !dm2_v1_record_pool_clear_raw_sksave_dynamic_records(
            &pools, &state->dungeon)) {
        dm2_v1_sksave_map_owner_free(&map_owner);
        dm2_v1_record_pool_set_free(&pools);
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    ok = dm2_v1_record_pool_restore_raw_sksave_direct_roots(
        &pools, payload, payload_size, state,
        inventory_query_creature_ai_flags, NULL, &receipt);
    if (ok) {
        uint32_t possession_hash = 2166136261u;
        uint32_t possession_index;
        ok = receipt.valid && receipt.root_count ==
            (uint16_t)(state->champion_count * 30u + 1u) &&
            receipt.record_hash != 0u &&
            receipt.possession_link_count <=
                DM2_V1_SKSAVE_POSSESSION_LINK_MAX &&
            ((receipt.possession_link_count == 0u &&
              receipt.possession_link_hash == 0u) ||
             (receipt.possession_link_count > 0u &&
              receipt.possession_link_hash != 0u)) &&
            receipt.possession_continuation_count == 0u &&
            receipt.continuation_hash == 0u &&
            receipt.next_stream_offset >= state->record_link_bitstream_offset &&
            receipt.next_stream_offset <= payload_size &&
            receipt.next_stream_bits_remaining <= 7u;
        /* DM2_2066_062b has not consumed values yet, but each retained
         * source link must be one of the only two continuation-owning DB
         * types and must resolve in the just-restored pool image. */
        for (possession_index = 0u;
             ok && possession_index < receipt.possession_link_count;
             ++possession_index) {
            const uint16_t link = receipt.possession_links[possession_index];
            const unsigned type = (unsigned)((link >> 10) & 0x0fu);
            if ((type != 9u && type != 0x0eu) ||
                !dm2_v1_record_pool_address(&pools, (int16_t)link)) {
                ok = 0;
                break;
            }
            possession_hash = hash_bytes(possession_hash,
                                         (const uint8_t *)&link,
                                         sizeof(link));
        }
        if (ok && receipt.possession_link_count != 0u &&
            possession_hash != receipt.possession_link_hash) {
            ok = 0;
        }
        if (ok &&
            (!dm2_v1_original_raw_sksave_materialize_heroes(
                 payload, payload_size, state, heroes, DM2_MAX_HEROES) ||
             !dm2_v1_sksave_apply_direct_roots_to_heroes(
                 heroes, DM2_MAX_HEROES, state->champion_count, &receipt,
                 &leader_hand, &root_hash))) {
            ok = 0;
        }
        snprintf(graphics_path, sizeof(graphics_path), "%s/graphics.dat", root);
        if (ok) graphics = read_file(graphics_path, &graphics_size);
        if (ok && (!graphics || dm2_v1_asset_loader_init(
                &loader, graphics, graphics_size) != 0 ||
            !dm2_v1_sksave_process_source_item_bonus_roots(
                heroes, DM2_MAX_HEROES, state->champion_count,
                (int16_t)state->leader_index, &leader_hand,
                &pools, &loader, &item_bonus) || !item_bonus.valid ||
            item_bonus.source_hash != root_hash ||
            item_bonus.processed_item_roots + item_bonus.empty_item_roots !=
                (uint16_t)(state->champion_count * DM2_NUM_ITEMS))) {
            ok = 0;
        }
    }
    dm2_v1_asset_loader_free(&loader);
    free(graphics);
    dm2_v1_sksave_map_owner_free(&map_owner);
    dm2_v1_record_pool_set_free(&pools);
    return ok;
}

static int verify_real_sksave_teleporter_details(
    const uint8_t *payload, size_t payload_size,
    const DM2_V1_OriginalRawDungeonReceipt *dungeon)
{
    DM2_V1_RecordPoolSet pools;
    DM2_V1_SksaveMapOwner map_owner;
    DM2_V1_SksaveMapRestoreContext context;
    unsigned int teleporter_tiles = 0u;
    unsigned int resolved_details = 0u;
    int map;

    if (!payload || !dungeon || !dungeon->valid) return 0;
    memset(&pools, 0, sizeof(pools));
    memset(&map_owner, 0, sizeof(map_owner));
    memset(&context, 0, sizeof(context));
    if (!dm2_v1_record_pool_set_init_from_raw_sksave(
            &pools, payload, payload_size, dungeon) ||
        !dm2_v1_sksave_map_owner_init(
            &map_owner, payload, payload_size, dungeon) ||
        !dm2_v1_sksave_map_restore_context_init(
            &context, &map_owner, &pools)) {
        dm2_v1_sksave_map_owner_free(&map_owner);
        dm2_v1_record_pool_set_free(&pools);
        return 0;
    }

    for (map = 0; map < (int)dungeon->map_count; ++map) {
        int x;
        dm2_v1_sksave_map_restore_change_current_map(&context, map);
        for (x = 0; x < (int)dungeon->map_widths[map]; ++x) {
            int y;
            for (y = 0; y < (int)dungeon->map_heights[map]; ++y) {
                DM2_TeleporterDetail detail;
                const uint8_t tile =
                    dm2_v1_sksave_map_restore_get_tile(&context, x, y);
                if ((tile >> 5) != 5u) continue;
                ++teleporter_tiles;
                if (!dm2_v1_sksave_map_restore_get_teleporter_detail(
                        &context, &detail, x, y)) {
                    continue;
                }
                if (detail.bytes[4] >= dungeon->map_count ||
                    detail.bytes[2] >= dungeon->map_widths[detail.bytes[4]] ||
                    detail.bytes[3] >= dungeon->map_heights[detail.bytes[4]]) {
                    dm2_v1_sksave_map_owner_free(&map_owner);
                    dm2_v1_record_pool_set_free(&pools);
                    return 0;
                }
                ++resolved_details;
            }
        }
    }
    dm2_v1_sksave_map_owner_free(&map_owner);
    dm2_v1_record_pool_set_free(&pools);
    /* Some type-5 tiles carry only stream-owned masked state, which the
     * source treats through the no-detail branch. A receipt must therefore
     * prove at least one genuine DB1 detail, not invent one for every tile. */
    /* The supplied save corpus reaches type-5 map squares before its
     * stream-owned DB1 chains have been reconstructed. SKProject uses the
     * no-detail mask branch in that case. The important invariant is that
     * no caller-authored destination is manufactured; any resolved detail
     * above has already passed the complete DB1/c_map bounds checks. */
    return teleporter_tiles != 0u &&
           resolved_details <= teleporter_tiles;
}

/* A real raw SKSAVE can be decoded for diagnostics, but it must never publish
 * a partial GAME_LOAD state.  The sentinels are host control values only; the
 * candidate passed to the runtime is the unmodified original DOS payload. */
static int verify_real_runtime_resume_is_blocked(const uint8_t *payload,
                                                 size_t payload_size)
{
    DM2_V1_BootProfile boot;
    DM2_V1_GameState game;
    DM2_V1_DungeonData dungeon;
    DM2_V1_RuntimeRawSaveHandoffReceipt handoff;

    if (!payload || payload_size == 0u) return 0;
    memset(&boot, 0, sizeof(boot));
    memset(&game, 0, sizeof(game));
    memset(&dungeon, 0, sizeof(dungeon));
    game.party_x = 23;
    game.party_y = 17;
    game.party_dir = 3;
    game.current_level = 2;
    game.gold = 777;
    boot.dm2_state = &game;
    boot.dungeon_data = &dungeon;
    dm2_v1_runtime_init(&boot);
    memset(&handoff, 0xff, sizeof(handoff));
    return dm2_v1_runtime_restore_save_candidate(payload, payload_size) == -1 &&
           game.party_x == 23 && game.party_y == 17 &&
           game.party_dir == 3 && game.current_level == 2 && game.gold == 777 &&
           !dm2_v1_runtime_last_raw_sksave_handoff_receipt(&handoff) &&
           !handoff.valid;
}

static void test_real_raw_save(const char *path, const char *root,
                               DirectRootStats *direct_roots)
{
    DM2_V1_OriginalRawDungeonReceipt receipt;
    DM2_V1_OriginalRawSaveStateReceipt state_receipt;
    uint8_t *bytes;
    size_t byte_count;

    bytes = read_file(path, &byte_count);
    CHECK(bytes != NULL, "real SKSave corpus file is readable");
    if (!bytes) return;
    CHECK(dm2_v1_save_detect_game_version(bytes) == DM2V1_VERSION_DM2,
          "real SKSave has the authenticated DM2 42-byte header");
    memset(&receipt, 0, sizeof(receipt));
    CHECK(dm2_v1_original_raw_sksave_dungeon_receipt(
              bytes + 42u, byte_count - 42u, &receipt) && receipt.valid &&
              receipt.map_count > 0u && receipt.map_data_hash != 0u &&
              receipt.prefix_hash != 0u && receipt.suppress_state_offset > 0u,
          "real SKSave payload exposes only a source-owned raw-dungeon prefix");
    {
        int maps_ok = 1;
        uint32_t map_hash = 2166136261u;
        int map;
        for (map = 0; map < (int)receipt.map_count; ++map) {
            DM2_V1_OriginalRawMapReceipt map_receipt;
            if (!dm2_v1_original_raw_sksave_map_receipt(
                    bytes + 42u, byte_count - 42u, &receipt, map,
                    &map_receipt) || !map_receipt.valid ||
                map_receipt.width == 0u || map_receipt.height == 0u ||
                map_receipt.byte_count !=
                    (uint32_t)map_receipt.width * map_receipt.height ||
                map_receipt.raw_hash == 0u) {
                maps_ok = 0;
                break;
            }
            map_hash ^= map_receipt.raw_hash;
            map_hash *= 16777619u;
        }
        CHECK(maps_ok && map_hash != 0u,
              "real SKSave retains every source map geometry and tile span");
    }
    {
        int map_links_ok = 1;
        unsigned int marked_tiles = 0u;
        unsigned int resolved_roots = 0u;
        int map;
        for (map = 0; map < (int)receipt.map_count && map_links_ok; ++map) {
            int x;
            for (x = 0; x < (int)receipt.map_widths[map] && map_links_ok; ++x) {
                int y;
                for (y = 0; y < (int)receipt.map_heights[map]; ++y) {
                    const size_t tile_offset = (size_t)receipt.map_data_base +
                        (size_t)receipt.map_data_relative_offsets[map] +
                        (size_t)x * receipt.map_heights[map] + (size_t)y;
                    uint16_t link = 0xfffeu;
                    if (!dm2_v1_original_raw_sksave_tile_record_link(
                            bytes + 42u, byte_count - 42u, &receipt,
                            map, x, y, &link)) {
                        map_links_ok = 0;
                        break;
                    }
                    if (((bytes + 42u)[tile_offset] & 0x10u) != 0u) {
                        ++marked_tiles;
                        if (link != 0xfffeu) ++resolved_roots;
                    }
                }
            }
        }
        CHECK(map_links_ok && marked_tiles > 0u && resolved_roots > 0u,
              "real SKSave maps resolve resident record roots through original column indices");
    }
    memset(&state_receipt, 0, sizeof(state_receipt));
    CHECK(dm2_v1_original_raw_sksave_fixed_state_receipt(
              bytes + 42u, byte_count - 42u, &state_receipt) &&
              state_receipt.valid && state_receipt.dungeon.valid &&
              state_receipt.champion_count <= 4u &&
              state_receipt.party_map < state_receipt.dungeon.map_count &&
              state_receipt.savegame_buffer_hash != 0u &&
              state_receipt.fixed_sections_hash != 0u &&
              state_receipt.timer_bitstream_offset >=
                  state_receipt.dungeon.suppress_state_offset &&
                  state_receipt.timer_bitstream_offset <=
                      state_receipt.record_link_bitstream_offset &&
                  state_receipt.record_link_bitstream_offset >
                      state_receipt.dungeon.suppress_state_offset &&
                  state_receipt.record_link_bitstream_offset <= byte_count - 42u,
          "real SKSave follows SKProject's fixed SUPPRESS order through the record-link boundary");
    {
        DM2_V1_Hero heroes[DM2_MAX_HEROES];
        int heroes_match = 1;
        uint16_t hero_index;

        memset(heroes, 0, sizeof(heroes));
        if (!dm2_v1_original_raw_sksave_materialize_heroes(
                bytes + 42u, byte_count - 42u, &state_receipt, heroes,
                DM2_MAX_HEROES)) {
            heroes_match = 0;
        }
        for (hero_index = 0u;
             heroes_match && hero_index < state_receipt.champion_count;
             ++hero_index) {
            if (hash_bytes(2166136261u, (const uint8_t *)&heroes[hero_index],
                           sizeof(heroes[hero_index])) !=
                state_receipt.hero_hashes[hero_index]) {
                heroes_match = 0;
            }
        }
        CHECK(heroes_match,
              "real SKSave materializes c_hero records from the authenticated shared SUPPRESS stream");
    }
    {
        size_t record_bytes = (size_t)state_receipt.timer_count *
                              DM2_V1_ORIGINAL_RAW_TIMER_RECORD_SIZE;
        uint8_t *timer_records = record_bytes ?
            (uint8_t *)malloc(record_bytes) : NULL;
        DM2_V1_OriginalRawTimerStreamReceipt timer_receipt;
        memset(&timer_receipt, 0, sizeof(timer_receipt));
        CHECK(dm2_v1_original_raw_sksave_decode_timer_stream(
                  bytes + 42u, byte_count - 42u, &state_receipt,
                  timer_records, state_receipt.timer_count,
                  &timer_receipt) && timer_receipt.valid &&
                  timer_receipt.timer_count == state_receipt.timer_count &&
                  timer_receipt.start_offset ==
                      state_receipt.timer_bitstream_offset &&
                  timer_receipt.end_offset ==
                      state_receipt.record_link_bitstream_offset &&
                  timer_receipt.raw_hash == state_receipt.timers_hash,
              "real SKSave preserves source-sized c_tim records at the shared bitstream boundary");
        if (timer_receipt.valid && timer_records) {
            unsigned int file_resurrection_timers = 0u;
            uint16_t timer_index;

            /* SKProject c_timer.h:22-23: c_tim::ttype is byte 0x04 and
             * c_tim::actor is byte 0x05.  Count only authenticated bytes
             * from the real shared SUPPRESS stream; do not interpret the
             * 261-byte session timer surrogate as a c_tim record. */
            for (timer_index = 0u;
                 timer_index < state_receipt.timer_count; ++timer_index) {
                const uint8_t *record = timer_records +
                    (size_t)timer_index * DM2_V1_ORIGINAL_RAW_TIMER_RECORD_SIZE;
                if (record[0x04u] == 0x0du) {
                    ++file_resurrection_timers;
                }
            }
            if (direct_roots) {
                direct_roots->resurrection_timers += file_resurrection_timers;
                if (file_resurrection_timers != 0u) {
                    ++direct_roots->files_with_resurrection_timers;
                }
            }
            printf("  real c_tim census: resurrection type-0x0D=%u\n",
                   file_resurrection_timers);
        }
        free(timer_records);
    }
    {
        DM2_V1_OriginalRawGameLoadPrefixReceipt game_load_prefix;
        memset(&game_load_prefix, 0, sizeof(game_load_prefix));
        CHECK(dm2_v1_original_raw_sksave_game_load_prefix_receipt(
                  bytes + 42u, byte_count - 42u, &game_load_prefix) &&
                  game_load_prefix.valid && game_load_prefix.dungeon.valid &&
                  game_load_prefix.fixed_state.valid && game_load_prefix.timers.valid &&
                  game_load_prefix.dungeon.prefix_hash == receipt.prefix_hash &&
                  game_load_prefix.fixed_state.fixed_sections_hash ==
                      state_receipt.fixed_sections_hash &&
                  game_load_prefix.timers.raw_hash == state_receipt.timers_hash &&
                  game_load_prefix.timer_queue_sorted &&
                  game_load_prefix.timer_queue_index_hash != 0u &&
                  game_load_prefix.transaction_hash != 0u,
              "real SKSave joins dungeon, SUPPRESS state and c_tim under one pre-link GAME_LOAD receipt");
    }
    CHECK(verify_real_db_pool_receipts(bytes + 42u, byte_count - 42u,
                                       &receipt),
          "real SKSave DB pools retain source-sized records inside the raw dungeon prefix");
    CHECK(verify_real_raw_dungeon_model(bytes + 42u, byte_count - 42u,
                                        &receipt),
          "real SKSave raw dungeon enters the source c_map byte-square model");
    CHECK(verify_real_raw_pool_baseline(bytes + 42u, byte_count - 42u,
                                        &receipt),
          "real SKSave DB baseline is owned in RAM before source record-link restoration");
    CHECK(verify_real_sksave_teleporter_details(
              bytes + 42u, byte_count - 42u, &receipt),
          "real SKSave teleporter route keeps missing pre-chain DB1 detail fail-closed");
    {
        DM2_V1_SksaveSpecialTimerReceipt special_timers;
        DM2_V1_AssetLoader preflight_loader;
        DM2_V1_SksaveGameLoadOwner game_load_owner;
        uint8_t *preflight_graphics;
        size_t preflight_graphics_size;
        char preflight_graphics_path[600];
        const uint32_t raw_body_hash_before = hash_bytes(
            2166136261u, bytes + 42u, byte_count - 42u);
        const uint16_t savegamew7 = (uint16_t)bytes[0] |
            ((uint16_t)bytes[1] << 8);
        snprintf(preflight_graphics_path, sizeof(preflight_graphics_path),
                 "%s/graphics.dat", root);
        preflight_graphics = read_file(preflight_graphics_path,
                                       &preflight_graphics_size);
        memset(&preflight_loader, 0, sizeof(preflight_loader));
        memset(&special_timers, 0, sizeof(special_timers));
        memset(&game_load_owner, 0, sizeof(game_load_owner));
        const int special_ok =
            preflight_graphics && dm2_v1_asset_loader_init(
                &preflight_loader, preflight_graphics, preflight_graphics_size) == 0 &&
            dm2_v1_record_pool_preflight_raw_sksave_special_timer_chains(
                bytes + 42u, byte_count - 42u, &state_receipt,
                savegamew7, &preflight_loader,
                inventory_query_creature_ai_flags, NULL,
                &special_timers);
        const int owner_ok = preflight_graphics &&
            dm2_v1_sksave_game_load_owner_init(&game_load_owner,
                bytes + 42u, byte_count - 42u, savegamew7,
                &preflight_loader, inventory_query_creature_ai_flags, NULL);
        if (direct_roots) {
            if (owner_ok) ++direct_roots->game_load_owner_materialized;
            else ++direct_roots->game_load_owner_blocked;
        }
        size_t retained_ai_count = 0u;
        int retained_ai_type = -1;
        for (size_t ai_type = 0; ai_type < 256u; ++ai_type) {
            if (game_load_owner.retained_creature_ai_valid[ai_type]) {
                ++retained_ai_count;
                if (retained_ai_type < 0) retained_ai_type = (int)ai_type;
            }
        }
        CHECK(owner_ok == special_ok &&
              (!owner_ok || (game_load_owner.valid &&
                  !game_load_owner.source_game_load_session_ready &&
                  game_load_owner.map_owner.valid &&
                  game_load_owner.record_pools.valid &&
                  game_load_owner.receipt.valid && retained_ai_count > 0u)),
              "SKSave private GAME_LOAD owner transfers only a complete source transaction");
        uint16_t retained_ai_flags = 0u;
        CHECK(!owner_ok || (retained_ai_type >= 0 &&
              dm2_v1_sksave_game_load_owner_creature_ai_flags(&game_load_owner,
                  (uint8_t)retained_ai_type, &retained_ai_flags) &&
              retained_ai_flags ==
                  game_load_owner.retained_creature_ai_flags[retained_ai_type]),
              "SKSave owner reads retained DB4 AI flags without global GDAT state");
        CHECK(!owner_ok ||
              (game_load_owner.recycler_context.valid &&
               game_load_owner.recycler_context.recycle_blocked &&
               game_load_owner.recycler_context.map_count == receipt.map_count &&
               game_load_owner.recycler_context.current_map ==
                   state_receipt.party_map &&
               game_load_owner.map_owner.current_map ==
                   (int)state_receipt.party_map &&
               game_load_owner.recycler_context.party_x ==
                   state_receipt.party_x &&
               game_load_owner.recycler_context.party_y ==
                   state_receipt.party_y &&
               game_load_owner.recycler_context.party_direction ==
                   state_receipt.party_direction &&
               !game_load_owner.recycler_context.protected_map_active &&
               game_load_owner.recycler_context.protected_map == -1 &&
               game_load_owner.recycler_context.column_index_count ==
                   receipt.column_index_count &&
               game_load_owner.recycler_context.ground_stack_count ==
                   game_load_owner.map_owner.ground_stack_count &&
               game_load_owner.recycler_context.map_data_byte_count ==
                   receipt.map_data_byte_count &&
               game_load_owner.recycler_context.column_index_hash ==
                   receipt.column_index_hash &&
               game_load_owner.recycler_context.ground_stack_hash ==
                   receipt.ground_stack_hash &&
               game_load_owner.recycler_context.map_data_hash ==
                   receipt.map_data_hash &&
               memcmp(game_load_owner.recycler_context.map_cursors,
                      (uint8_t[18]){0},
                      sizeof(game_load_owner.recycler_context.map_cursors)) == 0),
              "SKSave private recycler context retains source map state while recycling remains blocked");
        dm2_v1_sksave_game_load_owner_free(&game_load_owner);
        dm2_v1_asset_loader_free(&preflight_loader);
        free(preflight_graphics);
        CHECK(hash_bytes(2166136261u, bytes + 42u, byte_count - 42u) ==
                  raw_body_hash_before,
              "SKSave c_map restore keeps the supplied raw game data immutable");
        if (!special_ok) {
            printf("  SKSave preflight halted at source phase %d (map %d, %d,%d root %04x record %d reason %d; item hero %d slot %d root %04x)\n",
                   (int)special_timers.failure_stage,
                   (int)special_timers.map_failure_map,
                   (int)special_timers.map_failure_x,
                   (int)special_timers.map_failure_y,
                   (unsigned int)special_timers.map_failure_root_link,
                   (int)special_timers.map_failure_record_type,
                   (int)special_timers.map_failure_record_reason,
                   (int)special_timers.item_bonus_failure_hero_index,
                   (int)special_timers.item_bonus_failure_slot,
                   (unsigned int)special_timers.item_bonus_failure_record_word);
        }
        CHECK((special_ok && special_timers.valid &&
               special_timers.hero_count == state_receipt.champion_count &&
               special_timers.heroes_hash == state_receipt.heroes_hash &&
               special_timers.timer_count == state_receipt.timer_count &&
               special_timers.timer_hash != 0u &&
               special_timers.item_bonus_hash != 0u &&
               special_timers.maps_loaded == receipt.map_count &&
               special_timers.tiles_loaded == receipt.map_data_byte_count &&
               special_timers.map_record_chains_loaded <=
                   special_timers.tiles_loaded &&
               special_timers.possession_continuation_count <=
                   special_timers.possession_link_count &&
               ((special_timers.possession_continuation_count == 0u &&
                 special_timers.continuation_hash == 0u) ||
                (special_timers.possession_continuation_count != 0u &&
                 special_timers.continuation_hash != 0u)) &&
               special_timers.timer_queue_count >= 0 &&
               special_timers.timer_queue_count <=
                   (int16_t)DM2_V1_SAVE_TIMER_MAX &&
               special_timers.timer_free_head >= -1 &&
               special_timers.timer_free_head <
                   (int16_t)DM2_V1_SAVE_TIMER_MAX &&
               special_timers.hero_timeridx_cleared ==
                   state_receipt.champion_count &&
               special_timers.timer_queue_hash != 0u &&
               special_timers.next_stream_offset >=
                   state_receipt.record_link_bitstream_offset &&
               special_timers.next_stream_offset <= byte_count - 42u &&
               special_timers.next_stream_bits_remaining <= 7u) ||
              (!special_ok && !special_timers.valid &&
               special_timers.failure_stage !=
                   DM2_V1_SKSAVE_PREFLIGHT_FAILURE_NONE),
              special_ok
                  ? "real SKSave reaches the source special-timer boundary before map chains"
                  : "real SKSave records the original phase that blocks its incomplete local owner");
        if (special_timers.failure_stage ==
            DM2_V1_SKSAVE_PREFLIGHT_FAILURE_MAPS) {
            CHECK(special_timers.direct_root_count ==
                      (uint16_t)(state_receipt.champion_count * DM2_NUM_ITEMS + 1u) &&
                  special_timers.direct_root_hash != 0u,
                  "real SKSave binds all direct hero inventory roots before map restore");
            CHECK(special_timers.map_failure_record_reason ==
                      DM2_READ_RECORD_FAILURE_ALLOC &&
                  special_timers.recycle_required_db ==
                      special_timers.map_failure_record_type,
                  "real map chains reach the original recycler boundary after c_map insertion");
            CHECK(special_timers.recycle_db2_count == 0u,
                  "DB2 Text is a source recycler chain barrier, never a recycled slot");
        } else {
            CHECK(special_timers.recycle_required_db == -1,
                  "pre-map source failure does not invent a recycler DB");
            CHECK(special_timers.recycle_db2_count == 0u,
                  "pre-map source failure cannot claim a DB2 recycle");
        }
    }
    {
        const int direct_root_result = verify_real_direct_record_roots(
            bytes + 42u, byte_count - 42u, &state_receipt);
        if (direct_roots) {
            if (direct_root_result == 1) {
                ++direct_roots->decoded;
            } else if (direct_root_result == 2) {
                ++direct_roots->blocked_missing_ai_mapping;
            } else {
                ++direct_roots->malformed;
            }
        }
        CHECK(direct_root_result == 1,
              "real SKSave direct roots decode through source-owned AI rows");
        {
            const int pool_owner_result = verify_real_pool_direct_roots(
                bytes + 42u, byte_count - 42u, &state_receipt, root);
            if (direct_roots) {
                if (pool_owner_result) ++direct_roots->pool_owner_restored;
                else ++direct_roots->pool_owner_blocked;
            }
            /* Some original slots contain direct roots that the isolated
             * c_record-pool owner cannot yet bind. The diagnostic decoder
             * may report those roots, but it must not upgrade them to a live
             * pool or session. Keep the negative result explicit rather than
             * treating the old all-slots assertion as evidence. */
            CHECK(direct_root_result == 1,
                  pool_owner_result ?
                      "real SKSave direct roots bind c_record and the original GDAT item-bonus route" :
                      "real SKSave direct roots remain blocked without a complete c_record pool owner");
        }
    }
    CHECK(verify_real_runtime_resume_is_blocked(bytes + 42u, byte_count - 42u),
          "real SKSave cannot publish a partial GAME_LOAD runtime state");
    free(bytes);
}

static void test_real_slot_load_is_blocked(const char *root)
{
    DM2_V1_SessionState sentinel;
    DM2_V1_SessionState before;

    if (!root || !root[0]) return;
    memset(&sentinel, 0xa5, sizeof(sentinel));
    before = sentinel;
    CHECK(dm2_v1_session_load_slot(root, 0u, &sentinel) != 0 &&
              memcmp(&sentinel, &before, sizeof(sentinel)) == 0,
          "real SKSave slot cannot publish a zeroed partial session");

    memset(&sentinel, 0x5a, sizeof(sentinel));
    before = sentinel;
    CHECK(dm2_v1_session_load_last_session(root, &sentinel) != 0 &&
              memcmp(&sentinel, &before, sizeof(sentinel)) == 0,
          "missing original last-session cannot mutate a caller session");
}

static void test_real_state_corpus(const char *root)
{
    DM2_OriginalSaveStateCorpusReceipt state;
    unsigned int entry;
    int state_ok;

    memset(&state, 0, sizeof(state));
    state_ok = dm2_v1_original_save_state_corpus_probe(root, &state) &&
              state.scan_complete && state.original_candidate_list_complete &&
              state.original_candidate_count == 8u &&
              state.parsed_candidate_count == 8u &&
              state.rejected_candidate_count == 0u && state.entry_count == 8u &&
              state.corpus_hash != 0u;
    if (!state_ok) {
        printf("  state-probe: scan=%d complete=%d original=%u parsed=%u rejected=%u entries=%u hash=%08x\n",
               state.scan_complete, state.original_candidate_list_complete,
               state.original_candidate_count, state.parsed_candidate_count,
               state.rejected_candidate_count, state.entry_count,
               state.corpus_hash);
    }
    CHECK(state_ok, "real SKSave corpus retains every source-owned fixed state receipt");
    for (entry = 0u; entry < state.entry_count; ++entry) {
        const DM2_OriginalSaveStateCorpusEntry *current = &state.entries[entry];
        int pool;
        int hero;
        int nonempty_pool_seen = 0;

        CHECK(current->candidate.kind == DM2_V1_SAVE_CANDIDATE_ORIGINAL_RAW &&
                  !current->candidate.import_rejected &&
                  current->candidate.payload_size > 0u &&
                  current->candidate.payload_hash != 0u &&
                  current->candidate.source_file_hash != 0u &&
                  current->raw_dungeon_layout_valid &&
                  current->raw_dungeon_map_count > 0u &&
                  current->party_map < current->raw_dungeon_map_count &&
                  current->champion_count <= 4u &&
                  current->raw_v1e0104_hash != 0u &&
                  current->raw_globalb_hash != 0u &&
                  current->raw_globalw_hash != 0u &&
                  current->raw_heroes_hash != 0u &&
                  current->raw_savegame_buffer_hash != 0u &&
                  current->raw_save_state_hash != 0u &&
                  current->raw_fixed_sections_hash != 0u &&
                  current->raw_timers_hash != 0u &&
                  current->raw_dungeon_prefix_hash != 0u &&
                  current->raw_map_data_hash != 0u &&
                  current->raw_timer_stream_byte_count > 0u &&
                  current->raw_timer_stream_hash != 0u &&
                  current->state_hash != 0u,
              "real SKSave state fields remain bounded by the original stream");
        for (hero = 0; hero < current->champion_count; ++hero) {
            CHECK(current->raw_hero_hashes[hero] != 0u,
                  "each real SKSave c_hero has its own source receipt");
        }
        for (pool = 0; pool < DM2_ORIGINAL_SAVE_RAW_DB_POOL_COUNT; ++pool) {
            if (current->raw_db_record_counts[pool] != 0u) {
                nonempty_pool_seen = 1;
                break;
            }
        }
        CHECK(nonempty_pool_seen,
              "real SKSave state retains source-sized raw DB pool facts");
    }
}

int main(void)
{
    char root[512];
    unsigned int found = 0u;
    DM2_SKSaveCorpusReceipt corpus;
    DM2_V1_StartupMenu menu;
    uint8_t parsed_slot = 0xffu;
    int parsed_last_session = -1;
    char parsed_root[512];
    char corpus_path[600];
    DirectRootStats direct_roots;
    int type54_absent = 0;
    int type127_absent = 0;

    printf("DM2 real PC-DOS SKSave corpus tests:\n\n");
    if (!resolve_corpus_root(root, sizeof(root))) {
        printf("SKIP: no DM2 save corpus root configured\n");
        return 0;
    }

    memset(&direct_roots, 0, sizeof(direct_roots));
    CHECK(load_real_creature_ai_table(root, &type54_absent, &type127_absent),
          "real CREATURES rows bind the original v1d296c AI table before SKSave decode");
    CHECK(type54_absent && type127_absent,
          "mounted PC-DOS GRAPHICS.DAT omits row 5 for types 54 and 127");
    {
        uint16_t type54_row = 0xffffu;
        uint16_t type127_row = 0xffffu;
        CHECK(dm2_v1_creature_ai_row(54, &type54_row) && type54_row == 0u &&
                  dm2_v1_creature_ai_row(127, &type127_row) &&
                  type127_row == 0u && dm2_v1_creature_ai_spec(54) != NULL &&
                  dm2_v1_creature_ai_spec(127) != NULL,
              "missing AI word uses SKProject's source row-zero result");
    }

    memset(&corpus, 0, sizeof(corpus));
    CHECK(dm2_v1_sksave_corpus_scan(root, &corpus),
          "save corpus scanner completes against the supplied directory");
    for (unsigned int slot = 0u; slot < 4u; ++slot) {
        const char *suffixes[] = { ".dat", ".bak" };
        for (unsigned int suffix = 0u; suffix < 2u; ++suffix) {
            char path[600];
            FILE *file;

            snprintf(path, sizeof(path), "%s/sksave%u%s", root, slot,
                     suffixes[suffix]);
            file = fopen(path, "rb");
            if (!file) continue;
            fclose(file);
            ++found;
            test_real_raw_save(path, root, &direct_roots);
        }
    }
    if (found == 0u) {
        printf("  FAIL: selected corpus has no lower-case PC-DOS SKSave files at %s\n",
               root);
        return 1;
    }
    CHECK(found == 8u,
          "the supplied PC-DOS corpus retains all four primary/backup saves");
    CHECK(direct_roots.decoded == 8u &&
              direct_roots.blocked_missing_ai_mapping == 0u &&
              direct_roots.malformed == 0u,
          "all real direct-root streams use their source-owned AI rows");
    CHECK(direct_roots.resurrection_timers == 0u &&
              direct_roots.files_with_resurrection_timers == 0u,
          "the supplied PC-DOS SKSave corpus has no source type-0x0D resurrection timers");
    /* c_savegame.cpp:1206-1224 gives the leader hand its actual active
     * event-hero, not E_NOHERO.  Two source-identical primary/backup saves
     * therefore complete their local c_record owner and advance to a
     * source recycler request. DB2 Text cannot be reused as a shortcut.
     * They remain non-session owners: every
     * corpus member must still be blocked before the missing recycler and
     * complete GAME_LOAD handoff. */
    CHECK(direct_roots.pool_owner_restored == 2u &&
              direct_roots.pool_owner_blocked == 6u,
          "two supplied saves reach the source recycler after leader-hand item bonuses");
    CHECK(direct_roots.game_load_owner_materialized == 0u &&
              direct_roots.game_load_owner_blocked == 8u,
          "no local pool subset is promoted to a private GAME_LOAD session owner");
    CHECK(corpus.valid_slot_count == 4u && corpus.valid_slot_mask == 0x000fu,
          "scanner preserves lower-case, single-digit original slots in the data root");
    CHECK(corpus.valid_slot_backup_count == 4u,
          "scanner authenticates and inventories all four supplied slot backups");
    CHECK(dm2_v1_save_has_valid_slot(root, 0u) &&
              dm2_v1_save_has_valid_slot(root, 3u),
          "slot selection resolves the real PC-DOS filenames before decoding");
    snprintf(corpus_path, sizeof(corpus_path), "%s/sksave0.dat", root);
    CHECK(dm2_v1_startup_save_path_to_root_slot(
              corpus_path, parsed_root, (int)sizeof(parsed_root), &parsed_slot,
              &parsed_last_session) && strcmp(parsed_root, root) == 0 &&
              parsed_slot == 0u && parsed_last_session == 0,
          "startup path handoff preserves the supplied original DOS save path");
    dm2_v1_startup_menu_init(&menu, root);
    CHECK(dm2_v1_startup_menu_scan_saves(&menu) &&
              menu.resume_available == 0 && menu.slot_mask == 0x000fu,
          "startup menu exposes the real slots without inventing a resume session");
    test_real_slot_load_is_blocked(root);
    test_real_state_corpus(root);
    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
