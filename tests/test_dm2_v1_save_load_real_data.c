/*
 * Real PC-DOS DM2 SKSave corpus regression.
 *
 * SKProject GAME_LOAD reads the 42-byte SKSave container before the raw
 * saved-dungeon prefix.  These checks deliberately retain only those proven
 * boundaries; they do not invent champion names or promote an incomplete
 * SUPPRESS tail into a playable session.
 */

#include "dm2_v1_new_game.h"
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
} RecordChainInventory;

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
    callbacks.ctx = &inventory;
    for (root = 0u; root < root_count; ++root) {
        uint16_t root_link = 0xfffeu;
        if (dm2_v1_read_record_checkcode(&reader, &callbacks, &root_link,
                                         -1, 0, 0, 0) != 0 ||
            reader.error) {
            return 0;
        }
    }
    /* A no-record chain is valid. The hash captures its source consumption
     * only when a record body is genuinely present. */
    return reader.reader.position <= payload_size &&
           inventory.record_hash != 0u;
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

static void test_real_raw_save(const char *path)
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
    CHECK(verify_real_direct_record_roots(bytes + 42u, byte_count - 42u,
                                          &state_receipt),
          "real SKSave reads every source champion-item and party root without a fixture graph");
    free(bytes);
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

    printf("DM2 real PC-DOS SKSave corpus tests:\n\n");
    if (!resolve_corpus_root(root, sizeof(root))) {
        printf("SKIP: no DM2 save corpus root configured\n");
        return 0;
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
            test_real_raw_save(path);
        }
    }
    if (found == 0u) {
        printf("SKIP: no lower-case PC-DOS SKSave corpus at %s\n", root);
        return failed == 0 ? 0 : 1;
    }
    CHECK(found == 8u,
          "the supplied PC-DOS corpus retains all four primary/backup saves");
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
    test_real_state_corpus(root);
    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
