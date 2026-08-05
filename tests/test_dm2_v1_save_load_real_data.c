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
#include "dm2_v1_save_record_masks_pc34_compat.h"
#include "dm2_v1_save_suppress_masks_pc34_compat.h"

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

static uint16_t read_u16_le(const uint8_t *bytes)
{
    return (uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8);
}

/* SKProject: SKWINSPX/src/v5/sksvgame.cpp::DM2_GAME_LOAD lines
 * 1482-1517.  This is deliberately a read-only corpus receipt: it proves
 * the actual shared SUPPRESS sequence through the source-owned fixed
 * sections, but it does not claim that the following record-link stream is
 * restored or playable.  In particular, s_savegamebuffer is 0x3c bytes and
 * c_tim is 0x0c bytes; older Firestaff diagnostics used unowned 56/10-byte
 * convenience views and are not used here. */
static int raw_save_fixed_suppress_sections_decode(const uint8_t *payload,
                                                   size_t payload_size)
{
    DM2_V1_OriginalRawDungeonReceipt dungeon;
    DM2_SuppressReader reader;
    uint8_t savegame_buffer[60];
    uint8_t source_full_mask[2] = { 0xffu, 0xffu };
    uint8_t v1e0104[8];
    uint8_t globalb[64];
    uint8_t globalw[128];
    uint8_t hero[263];
    uint8_t save_state[6];
    uint8_t timer[12];
    const uint8_t *hero_mask;
    const uint8_t *save_state_mask;
    const uint8_t *timer_mask;
    size_t vsgame_size = 0u;
    uint16_t champion_count;
    uint16_t timer_count;

    if (!payload ||
        !dm2_v1_original_raw_sksave_dungeon_receipt(payload, payload_size,
                                                     &dungeon) ||
        !dungeon.valid || dungeon.suppress_state_offset >= payload_size) {
        return 0;
    }
    hero_mask = dm2_v1_save_mask_hero();
    save_state_mask = dm2_v1_save_mask_save_state();
    timer_mask = dm2_v1_save_vsgame_raw(&vsgame_size);
    if (!hero_mask || !save_state_mask || !timer_mask || vsgame_size < 12u) {
        return 0;
    }
    dm2_suppress_reader_init(&reader, payload + dungeon.suppress_state_offset,
                             payload_size - dungeon.suppress_state_offset);
    if (dm2_suppress_reader_read(&reader,
                                 dm2_v1_save_mask_savegame_buffer(),
                                 sizeof(savegame_buffer), savegame_buffer,
                                 0u) != 0) {
        return 0;
    }
    /* s_savegamebuffer::w_08 and ::w_14. */
    champion_count = read_u16_le(savegame_buffer + 8u);
    timer_count = read_u16_le(savegame_buffer + 20u);
    if (champion_count > 4u || timer_count > 4096u) return 0;
    if (dm2_suppress_reader_read(&reader, source_full_mask, 1u, v1e0104,
                                 0u) != 0 ||
        dm2_suppress_reader_read(&reader, source_full_mask, 1u, globalb,
                                 0u) != 0 ||
        dm2_suppress_reader_read(&reader, source_full_mask, 2u, globalw,
                                 0u) != 0) {
        return 0;
    }
    for (uint16_t i = 0u; i < champion_count; ++i) {
        if (dm2_suppress_reader_read(&reader, hero_mask, sizeof(hero), hero,
                                     0u) != 0) {
            return 0;
        }
    }
    if (dm2_suppress_reader_read(&reader, save_state_mask, sizeof(save_state),
                                 save_state, 0u) != 0) {
        return 0;
    }
    for (uint16_t i = 0u; i < timer_count; ++i) {
        if (dm2_suppress_reader_read(&reader, timer_mask, sizeof(timer),
                                     timer, 0u) != 0) {
            return 0;
        }
    }
    /* The next source operation is READ_SKSAVE_DUNGEON on this same reader.
     * Retain it as an unowned boundary rather than interpreting its bits. */
    return reader.position != 0u;
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
    CHECK(raw_save_fixed_suppress_sections_decode(bytes + 42u,
                                                  byte_count - 42u),
          "real SKSave follows SKProject's 60-byte state and 12-byte timer SUPPRESS order");
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
