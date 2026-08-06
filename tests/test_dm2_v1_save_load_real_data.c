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
    unsigned int record_count;
    uint32_t record_hash;
    int creature_ai_unavailable;
    uint8_t unavailable_creature_type;
} RecordChainInventory;

typedef struct {
    unsigned int decoded;
    unsigned int blocked_missing_ai_mapping;
    unsigned int malformed;
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
    /* A no-record chain is valid. The hash captures its source consumption
     * only when a record body is genuinely present. */
    return reader.reader.position <= payload_size &&
           inventory.record_hash != 0u ? 1 : 0;
}

static int resolve_corpus_root(char *out, size_t out_size)
{
    const char *explicit_root = getenv("FIRESTAFF_DM2_SKSAVE_CORPUS");
    const char *data_root = getenv("FIRESTAFF_DM2_DATA_DIR");
    const char *home = getenv("HOME");

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
    if (!home || !home[0]) return 0;
    snprintf(out, out_size, "%s/.firestaff/data/dm2", home);
    return 1;
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

static void test_real_raw_save(const char *path, DirectRootStats *direct_roots)
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
    memset(&state_receipt, 0, sizeof(state_receipt));
    CHECK(dm2_v1_original_raw_sksave_fixed_state_receipt(
              bytes + 42u, byte_count - 42u, &state_receipt) &&
              state_receipt.valid && state_receipt.dungeon.valid &&
              state_receipt.champion_count <= 4u &&
              state_receipt.party_map < state_receipt.dungeon.map_count &&
              state_receipt.fixed_sections_hash != 0u &&
              state_receipt.timer_bitstream_offset >=
                  state_receipt.dungeon.suppress_state_offset &&
              state_receipt.timer_bitstream_offset <=
                  state_receipt.record_link_bitstream_offset &&
              state_receipt.record_link_bitstream_offset >
                  state_receipt.dungeon.suppress_state_offset &&
              state_receipt.record_link_bitstream_offset <= byte_count - 42u,
          "real SKSave follows SKProject's fixed SUPPRESS order through the record-link boundary");
    CHECK(verify_real_db_pool_receipts(bytes + 42u, byte_count - 42u,
                                       &receipt),
          "real SKSave DB pools retain source-sized records inside the raw dungeon prefix");
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
              "real SKSave direct roots decode through the source AI lookup");
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
                  current->raw_save_state_hash != 0u &&
                  current->raw_fixed_sections_hash != 0u &&
                  current->raw_timers_hash != 0u &&
                  current->raw_dungeon_prefix_hash != 0u &&
                  current->raw_map_data_hash != 0u &&
                  current->raw_timer_stream_byte_count > 0u &&
                  current->raw_timer_stream_hash != 0u &&
                  current->state_hash != 0u,
              "real SKSave state fields remain bounded by the original stream");
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
          "mounted PC-DOS GRAPHICS.DAT omits row 5 for types 54 and 127; source uses scalar-zero AI row");

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
            test_real_raw_save(path, &direct_roots);
        }
    }
    if (found == 0u) {
        printf("SKIP: no lower-case PC-DOS SKSave corpus at %s\n", root);
        return failed == 0 ? 0 : 1;
    }
    CHECK(found == 8u,
          "the supplied PC-DOS corpus retains all four primary/backup saves");
    CHECK(direct_roots.decoded == 8u &&
              direct_roots.blocked_missing_ai_mapping == 0u &&
              direct_roots.malformed == 0u,
          "all eight real direct-root streams decode through the source-owned AI lookup");
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
