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

static void test_real_raw_save(const char *path)
{
    DM2_V1_OriginalRawDungeonReceipt receipt;
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
    free(bytes);
}

int main(void)
{
    char root[512];
    unsigned int found = 0u;
    DM2_SKSaveCorpusReceipt corpus;

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
    CHECK(corpus.recursive_candidate_count >= found,
          "scanner records the lower-case original corpus as recursive candidates");
    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
