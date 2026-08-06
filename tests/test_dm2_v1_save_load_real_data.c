/*
 * Real PC-DOS DM2 SKSave corpus regression.
 *
 * SKProject GAME_LOAD reads the 42-byte SKSave container before the raw
 * saved-dungeon prefix.  These checks deliberately retain only those proven
 * boundaries; they do not invent champion names or promote an incomplete
 * SUPPRESS tail into a playable session.
 */

#include "dm2_v1_new_game.h"
#include "dm2_v1_save_load.h"
#include "dm2_v1_startup_menu.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(condition, message) do { \
    if (condition) { ++passed; printf("  PASS: %s\n", message); } \
    else { ++failed; printf("  FAIL: %s\n", message); } \
} while (0)

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
    free(bytes);
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
    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
