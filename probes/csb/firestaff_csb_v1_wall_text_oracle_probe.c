/*
 * firestaff_csb_v1_wall_text_oracle_probe.c
 *
 * CSB V1 wall text / oracle-style extraction regression probe.
 *
 * Scope:
 *   - one synthetic hash-verified dungeon slice
 *   - one level
 *   - one wall square
 *   - one textstring object
 *   - one encoded inscription payload ("ORACLE")
 *   - extraction from the loader-owned raw buffer, not the sidecar file copy
 *
 * Why this exists:
 *   ReDMCSB DUNGEON.C F0168 decodes inscription text from dungeon text
 *   words, and DUNGEON.C F0171/F0172 feed the same selected text thing
 *   into the wall-ornament path. BUG0_76 notes that only one wall text is
 *   retained for draw purposes, so this probe keeps the scope to a single
 *   square/object chain and verifies that the extracted wall text matches
 *   the source-locked inscription codes.
 *
 * This is intentionally not an end-to-end CSB runtime parity claim.
 */

#include "asset_find_by_hash.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#include <direct.h>
#include <process.h>
#define MKDIR(path) _mkdir(path)
#define RMDIR(path) _rmdir(path)
#define UNLINK(path) remove(path)
#define GETPID() _getpid()
#else
#include <sys/stat.h>
#include <unistd.h>
#define MKDIR(path) mkdir((path), 0775)
#define RMDIR(path) rmdir(path)
#define UNLINK(path) remove(path)
#define GETPID() getpid()
#endif

enum {
    WALL_TEXT_ORACLE_SLICE_SIZE = 22,
    WALL_TEXT_ORACLE_TEXT_BASE = 16,
    WALL_TEXT_ORACLE_THING_BASE = 12,
    WALL_TEXT_ORACLE_TEXT_WORDS = 3
};

static const unsigned char kWallTextOracleSlice[WALL_TEXT_ORACLE_SLICE_SIZE] = {
    0x01, 0x00, 0x10, 0x00, 0x01, 0x01, 0x0a, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xfe, 0xff, 0x01, 0x00, 0x20, 0x3a, 0x64, 0x09,
    0x00, 0x7c
};

static const char *kWallTextOracleSliceMd5 = "d26def63e7587be6ec52c643f2bd29df";

static int g_failures = 0;

#define CHECK(cond, msg) do { \
    if (cond) { \
        printf("PASS: %s\n", msg); \
    } else { \
        printf("FAIL: %s\n", msg); \
        g_failures++; \
    } \
} while (0)

static int ensure_dir(const char *path) {
    if (!path) {
        return 0;
    }
    if (MKDIR(path) == 0) {
        return 1;
    }
#ifdef _WIN32
    return 0;
#else
    return errno == EEXIST;
#endif
}

static int write_slice_file(const char *path) {
    FILE *fp = fopen(path, "wb");
    if (!fp) {
        return 0;
    }
    if (fwrite(kWallTextOracleSlice, 1, sizeof(kWallTextOracleSlice), fp) !=
        sizeof(kWallTextOracleSlice)) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static int verify_slice_hash(const char *search_dir, char *resolved_path, int resolved_cap) {
    return asset_find_by_md5(search_dir, kWallTextOracleSliceMd5,
                             resolved_path, resolved_cap, 1);
}

/*
 * ReDMCSB DUNGEON.C F0168:2255-2334 decodes 3 five-bit codes per 16-bit
 * word and stops on code 31. For inscriptions, codes 0..27 are preserved
 * as glyph codes, 28 is the separator byte, and 31 is the terminator.
 */
static size_t decode_inscription_codes(const unsigned char *words,
                                       size_t word_count,
                                       unsigned char *out,
                                       size_t out_cap)
{
    size_t out_len = 0;
    size_t i;

    if (!words || !out || out_cap == 0) {
        return 0;
    }

    for (i = 0; i < word_count; ++i) {
        unsigned short raw = (unsigned short)(words[i * 2U]) |
                             (unsigned short)((unsigned short)words[i * 2U + 1U] << 8U);
        unsigned short codes[3];
        int code_idx;

        codes[0] = (unsigned short)((raw >> 10U) & 0x1FU);
        codes[1] = (unsigned short)((raw >> 5U) & 0x1FU);
        codes[2] = (unsigned short)(raw & 0x1FU);

        for (code_idx = 0; code_idx < 3; ++code_idx) {
            unsigned short code = codes[code_idx];
            if (code == 31U) {
                return out_len;
            }
            if (out_len + 1U >= out_cap) {
                return out_len;
            }
            if (code == 28U) {
                out[out_len++] = 0x80U;
            } else {
                out[out_len++] = (unsigned char)code;
            }
        }
    }

    return out_len;
}

static void render_oracle_label(const unsigned char *codes,
                                size_t len,
                                char *out,
                                size_t out_cap)
{
    size_t i;
    size_t pos = 0;

    if (!out || out_cap == 0) {
        return;
    }
    out[0] = '\0';

    for (i = 0; i < len && pos + 1U < out_cap; ++i) {
        unsigned char code = codes[i];
        char ch = '?';
        if (code < 26U) {
            ch = (char)('A' + code);
        } else if (code == 26U) {
            ch = ' ';
        } else if (code == 27U) {
            ch = '.';
        } else if (code == 0x80U) {
            ch = '/';
        }
        out[pos++] = ch;
    }
    out[pos] = '\0';
}

static void poison_sidecar_text_words(unsigned char *file_buf, size_t file_size)
{
    size_t i;

    if (!file_buf ||
        WALL_TEXT_ORACLE_TEXT_BASE +
            (WALL_TEXT_ORACLE_TEXT_WORDS * 2U) > file_size) {
        return;
    }

    for (i = 0; i < WALL_TEXT_ORACLE_TEXT_WORDS * 2U; ++i) {
        file_buf[WALL_TEXT_ORACLE_TEXT_BASE + i] ^= 0x5AU;
    }
}

static int extract_wall_text_from_slice(const unsigned char *file_buf,
                                        size_t file_size,
                                        const CSB_V1_DungeonData *dungeon,
                                        char *decoded_label,
                                        size_t decoded_label_cap)
{
    int first_thing;
    unsigned short next_word;
    unsigned short meta_word;
    unsigned short visible;
    unsigned short text_word_offset;
    const unsigned char *text_words;
    unsigned char glyph_codes[16];
    size_t glyph_len;

    if (!file_buf || !dungeon || !decoded_label || decoded_label_cap == 0) {
        return -1;
    }

    first_thing = csb_v1_dungeon_get_first_thing(dungeon, 0, 0, 0);
    CHECK(first_thing == 0, "wall square points to the single textstring object");
    if (first_thing != 0) {
        return -1;
    }

    if (WALL_TEXT_ORACLE_THING_BASE + 4U > file_size) {
        return -1;
    }

    next_word = (unsigned short)file_buf[WALL_TEXT_ORACLE_THING_BASE] |
                (unsigned short)((unsigned short)file_buf[WALL_TEXT_ORACLE_THING_BASE + 1U] << 8U);
    meta_word = (unsigned short)file_buf[WALL_TEXT_ORACLE_THING_BASE + 2U] |
                (unsigned short)((unsigned short)file_buf[WALL_TEXT_ORACLE_THING_BASE + 3U] << 8U);

    visible = (unsigned short)(meta_word & 0x1U);
    text_word_offset = (unsigned short)(meta_word >> 3U);

    CHECK(next_word == 0xFFFEU, "single wall text object terminates the thing chain");
    CHECK(visible == 1U, "wall text object is visible");
    CHECK(text_word_offset == 0U, "wall text object points at the first dungeon-text word");
    if (next_word != 0xFFFEU || visible != 1U || text_word_offset != 0U) {
        return -1;
    }

    text_words = file_buf + WALL_TEXT_ORACLE_TEXT_BASE + ((size_t)text_word_offset * 2U);
    if ((size_t)(text_words - file_buf) + (WALL_TEXT_ORACLE_TEXT_WORDS * 2U) > file_size) {
        return -1;
    }

    glyph_len = decode_inscription_codes(text_words,
                                         WALL_TEXT_ORACLE_TEXT_WORDS,
                                         glyph_codes,
                                         sizeof(glyph_codes));
    CHECK(glyph_len == 6U, "inscription decoder returns six glyph codes before terminator");
    if (glyph_len != 6U) {
        return -1;
    }

    CHECK(glyph_codes[0] == 14U, "glyph 0 is O");
    CHECK(glyph_codes[1] == 17U, "glyph 1 is R");
    CHECK(glyph_codes[2] == 0U, "glyph 2 is A");
    CHECK(glyph_codes[3] == 2U, "glyph 3 is C");
    CHECK(glyph_codes[4] == 11U, "glyph 4 is L");
    CHECK(glyph_codes[5] == 4U, "glyph 5 is E");

    render_oracle_label(glyph_codes, glyph_len, decoded_label, decoded_label_cap);
    return 0;
}

static int write_and_verify_fixture(char *resolved_path, size_t resolved_cap,
                                    char *fixture_path, size_t fixture_path_cap,
                                    char *fixture_dir, size_t fixture_dir_cap) {
    char path[256];

    if (!resolved_path || resolved_cap == 0 || !fixture_path ||
        fixture_path_cap == 0 || !fixture_dir || fixture_dir_cap == 0) {
        return 0;
    }

    if (snprintf(fixture_dir, fixture_dir_cap,
                 "csb_wall_text_oracle_probe_tmp_%ld",
                 (long)GETPID()) <= 0) {
        return 0;
    }

    if (!ensure_dir(fixture_dir)) {
        return 0;
    }

    if (snprintf(path, sizeof(path), "%s/wall_text_oracle_slice.bin",
                 fixture_dir) <= 0) {
        return 0;
    }

    if (!write_slice_file(path)) {
        return 0;
    }

    if (!verify_slice_hash(fixture_dir, resolved_path, (int)resolved_cap)) {
        return 0;
    }

    if (snprintf(fixture_path, fixture_path_cap, "%s", path) <= 0) {
        return 0;
    }

    return 1;
}

int main(void) {
    CSB_V1_DungeonData dungeon;
    CSB_V1_DungeonData file_loaded_dungeon;
    unsigned char *file_buf = NULL;
    char resolved_path[ASSET_PATH_MAX];
    char fixture_path[ASSET_PATH_MAX];
    char fixture_dir[128];
    char decoded_label[32];
    char file_loaded_label[32];
    FILE *fp = NULL;
    long file_size = 0;
    int load_ret;
    int file_load_ret;
    int tile_ret;
    int ok;
    int file_ok;
    CSB_V1_DecodedSquare decoded_square;

    printf("=== CSB V1 Wall Text / Oracle Probe ===\n");

    if (!write_and_verify_fixture(resolved_path, sizeof(resolved_path),
                                  fixture_path, sizeof(fixture_path),
                                  fixture_dir, sizeof(fixture_dir))) {
        printf("FAIL: could not write and hash-verify the synthetic slice\n");
        return 1;
    }
    CHECK(strcmp(resolved_path, fixture_path) == 0,
          "hash discovery returns the generated CSB dungeon slice path");

    fp = fopen(resolved_path, "rb");
    if (!fp) {
        printf("FAIL: could not reopen hash-verified slice: %s\n", resolved_path);
        UNLINK(resolved_path);
        RMDIR(fixture_dir);
        return 1;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        UNLINK(resolved_path);
        RMDIR(fixture_dir);
        return 1;
    }

    file_size = ftell(fp);
    if (file_size <= 0) {
        fclose(fp);
        UNLINK(resolved_path);
        RMDIR(fixture_dir);
        return 1;
    }

    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        UNLINK(resolved_path);
        RMDIR(fixture_dir);
        return 1;
    }

    file_buf = (unsigned char *)malloc((size_t)file_size);
    if (!file_buf) {
        fclose(fp);
        UNLINK(resolved_path);
        RMDIR(fixture_dir);
        return 1;
    }

    if (fread(file_buf, 1, (size_t)file_size, fp) != (size_t)file_size) {
        free(file_buf);
        fclose(fp);
        UNLINK(resolved_path);
        RMDIR(fixture_dir);
        return 1;
    }
    fclose(fp);

    memset(&dungeon, 0xCC, sizeof(dungeon));
    load_ret = csb_v1_dungeon_load(&dungeon, file_buf, (int)file_size);
    CHECK(load_ret == 0, "CSB dungeon loader accepts the hash-verified slice");
    CHECK(dungeon.level_count == 1, "slice contains exactly one level");
    CHECK(dungeon.level_widths[0] == 1, "slice level width is one square");
    CHECK(dungeon.level_heights[0] == 1, "slice level height is one square");
    CHECK(dungeon.raw_data != NULL, "loader owns a raw copy of the hash-verified slice");
    CHECK(dungeon.raw_size == (int)file_size, "loader raw copy preserves slice byte count");
    CHECK(dungeon.raw_data != file_buf,
          "loader raw copy is independent from the reopened sidecar buffer");
    CHECK(dungeon.raw_data != NULL &&
          dungeon.raw_size == (int)file_size &&
          memcmp(dungeon.raw_data, file_buf, (size_t)file_size) == 0,
          "loader raw copy is byte-identical to the hash-verified slice");
    CHECK(csb_v1_dungeon_get_square_type(&dungeon, 0, 0, 0) == 0,
          "single square is a wall square");
    CHECK(csb_v1_dungeon_get_first_thing(&dungeon, 0, 0, 0) == 0,
          "single wall square exposes the first thing index");
    memset(&decoded_square, 0xCC, sizeof(decoded_square));
    tile_ret = csb_v1_dungeon_decode_tile(&dungeon, 0, 0, 0, &decoded_square);
    CHECK(tile_ret == 0, "decoded tile is available for the wall square");
    CHECK(tile_ret == 0 && decoded_square.type == 0,
          "decoded tile keeps the wall square type");
    CHECK(tile_ret == 0 && decoded_square.first_thing == 0,
          "decoded tile keeps the wall text thing index");

    memset(&file_loaded_dungeon, 0xCC, sizeof(file_loaded_dungeon));
    file_load_ret = csb_v1_dungeon_load_from_file(&file_loaded_dungeon, resolved_path);
    CHECK(file_load_ret == 0,
          "CSB file loader accepts the scanner-resolved dungeon slice");
    CHECK(file_load_ret == 0 && file_loaded_dungeon.raw_data != NULL,
          "file loader owns a raw copy of the scanner-resolved slice");
    CHECK(file_load_ret == 0 && file_loaded_dungeon.raw_size == (int)file_size,
          "file loader raw copy preserves slice byte count");
    CHECK(file_load_ret == 0 &&
          csb_v1_dungeon_get_square_type(&file_loaded_dungeon, 0, 0, 0) == 0,
          "file-loaded slice keeps the wall square type");
    CHECK(file_load_ret == 0 &&
          csb_v1_dungeon_get_first_thing(&file_loaded_dungeon, 0, 0, 0) == 0,
          "file-loaded slice keeps the wall text thing index");
    file_loaded_label[0] = '\0';
    file_ok = -1;
    if (file_load_ret == 0 && file_loaded_dungeon.raw_data &&
        file_loaded_dungeon.raw_size > 0) {
        file_ok = extract_wall_text_from_slice(file_loaded_dungeon.raw_data,
                                               (size_t)file_loaded_dungeon.raw_size,
                                               &file_loaded_dungeon,
                                               file_loaded_label,
                                               sizeof(file_loaded_label));
    }
    CHECK(file_ok == 0 && strcmp(file_loaded_label, "ORACLE") == 0,
          "file-loaded hash path decodes the ORACLE wall text");
    csb_v1_dungeon_free(&file_loaded_dungeon);

    poison_sidecar_text_words(file_buf, (size_t)file_size);
    CHECK(dungeon.raw_data != NULL &&
          memcmp(dungeon.raw_data, file_buf, (size_t)file_size) != 0,
          "sidecar text mutation cannot affect the loader-owned raw buffer");

    decoded_label[0] = '\0';
    ok = 0;
    if (load_ret == 0 && dungeon.raw_data && dungeon.raw_size > 0) {
        ok = extract_wall_text_from_slice(dungeon.raw_data, (size_t)dungeon.raw_size,
                                          &dungeon, decoded_label,
                                          sizeof(decoded_label));
    }
    CHECK(ok == 0, "one wall/object path decodes successfully");
    CHECK(ok == 0 && strcmp(decoded_label, "ORACLE") == 0,
          "decoded wall text is ORACLE");

    if (ok == 0) {
        printf("Decoded wall text: %s\n", decoded_label);
    }

    csb_v1_dungeon_free(&dungeon);
    free(file_buf);
    UNLINK(resolved_path);
    RMDIR(fixture_dir);

    printf("Result: %s\n", g_failures == 0 ? "PASS" : "FAIL");
    return g_failures == 0 ? 0 : 1;
}
