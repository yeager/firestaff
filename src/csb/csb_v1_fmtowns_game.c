#include "csb_v1_fmtowns_game.h"

#include "redmcsb_f7061_save_header_pc34_compat.h"
#include "redmcsb_f7062_save_header_pc34_compat.h"
#include "redmcsb_f7055_saveutil_pc34_compat.h"
#include "fs_portable_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <dirent.h>
#endif

enum {
    CSB_V1_FMTOWNS_CHTWE_SIZE = 283936u,
    CSB_V1_FMTOWNS_CHTWJ_SIZE = 284416u,
    CSB_V1_FMTOWNS_CHTWE_FNV1A = 0x3da136f6u,
    CSB_V1_FMTOWNS_CHTWJ_FNV1A = 0xf937db45u,
    CSB_V1_FMTOWNS_UTILE_SIZE = 152387u,
    CSB_V1_FMTOWNS_UTILJ_SIZE = 152499u,
    CSB_V1_FMTOWNS_UTILE_FNV1A = 0xff240e0cu,
    CSB_V1_FMTOWNS_UTILJ_FNV1A = 0xbb3b47c2u,
    CSB_V1_FMTOWNS_CDATA_MINI_SIZE = 42776u,
    CSB_V1_FMTOWNS_CJDATA_MINI_SIZE = 43208u,
    CSB_V1_FMTOWNS_CDATA_MINI_FNV1A = 0x494999c9u,
    CSB_V1_FMTOWNS_CJDATA_MINI_FNV1A = 0x284799d1u,
    CSB_V1_FMTOWNS_SAVE_HEADER_BYTES = 512u,
    CSB_V1_FMTOWNS_CSB_HEADER_KEY_WORD_INDEX = 29u,
    CSB_V1_FMTOWNS_SAVE_HEADER_USELESS_OFFSET = 0x12cu,
    CSB_V1_FMTOWNS_SAVE_HEADER_FORMAT_OFFSET = 0x12du,
    CSB_V1_FMTOWNS_SAVE_HEADER_PLATFORM_OFFSET = 0x178u,
    CSB_V1_FMTOWNS_SAVE_HEADER_DUNGEON_ID_OFFSET = 0x17au,
    CSB_V1_FMTOWNS_SAVE_HEADER_KEYS_OFFSET = 0x138u,
    CSB_V1_FMTOWNS_SAVE_HEADER_CHECKSUMS_OFFSET = 0x158u,
    CSB_V1_FMTOWNS_SAVE_HEADER_FORMAT_C5 = 5u,
    CSB_V1_FMTOWNS_SAVE_HEADER_DUNGEON_CSB_PRISON = 12u,
    CSB_V1_FMTOWNS_SAVE_HEADER_DUNGEON_CSB_GAME = 13u,
    CSB_V1_FMTOWNS_GLOBAL_DATA_BYTES = 128u,
    CSB_V1_FMTOWNS_ACTIVE_GROUP_BYTES = 16u,
    CSB_V1_FMTOWNS_CHAMPION_PARTY_BYTES = 1404u,
    CSB_V1_FMTOWNS_EVENT_BYTES = 10u,
    CSB_V1_FMTOWNS_TIMELINE_ENTRY_BYTES = 2u,
    CSB_V1_FMTOWNS_GLOBAL_PARTY_CHAMPION_COUNT_OFFSET = 10u,
    CSB_V1_FMTOWNS_GLOBAL_PARTY_MAP_X_OFFSET = 12u,
    CSB_V1_FMTOWNS_GLOBAL_PARTY_MAP_Y_OFFSET = 14u,
    CSB_V1_FMTOWNS_GLOBAL_PARTY_DIRECTION_OFFSET = 16u,
    CSB_V1_FMTOWNS_GLOBAL_PARTY_MAP_INDEX_OFFSET = 18u,
    CSB_V1_FMTOWNS_GLOBAL_EVENT_COUNT_OFFSET = 24u,
    CSB_V1_FMTOWNS_GLOBAL_FIRST_UNUSED_EVENT_INDEX_OFFSET = 26u,
    CSB_V1_FMTOWNS_GLOBAL_EVENT_MAXIMUM_COUNT_OFFSET = 28u,
    CSB_V1_FMTOWNS_GLOBAL_CURRENT_ACTIVE_GROUP_COUNT_OFFSET = 30u,
    CSB_V1_FMTOWNS_GLOBAL_ACTIVE_GROUP_CAPACITY_OFFSET = 46u,
    CSB_V1_FMTOWNS_EXTERNAL_PORTRAIT_BYTES = 464u,
    CSB_V1_FMTOWNS_EXTERNAL_PORTRAIT_COUNT = 4u,
    CSB_V1_FMTOWNS_DUNGEON_HEADER_BYTES = 44u,
    CSB_V1_FMTOWNS_DUNGEON_MAP_BYTES = 16u,
    CSB_V1_FMTOWNS_DUNGEON_MAP_WIDTH_OFFSET = 8u,
    CSB_V1_FMTOWNS_DUNGEON_RAW_MAP_BYTES_OFFSET = 2u,
    CSB_V1_FMTOWNS_DUNGEON_MAP_COUNT_OFFSET = 4u,
    CSB_V1_FMTOWNS_DUNGEON_TEXT_WORD_COUNT_OFFSET = 6u,
    CSB_V1_FMTOWNS_DUNGEON_SQUARE_FIRST_THING_COUNT_OFFSET = 10u,
    CSB_V1_FMTOWNS_DUNGEON_THING_COUNTS_OFFSET = 12u,
    CSB_V1_FMTOWNS_DUNGEON_TRAILER_BYTES = 2u,
    CSB_V1_FMTOWNS_UTILE_MENU_VIRTUAL_OFFSET = 0x11578u,
    CSB_V1_FMTOWNS_UTILJ_MENU_VIRTUAL_OFFSET = 0x11628u,
    CSB_V1_FMTOWNS_UTILE_MENU_BYTES = 76u,
    CSB_V1_FMTOWNS_UTILJ_MENU_BYTES = 68u,
    CSB_V1_FMTOWNS_UTILE_MENU_FNV1A = 0xfd9986bfu,
    CSB_V1_FMTOWNS_UTILJ_MENU_FNV1A = 0xdceefc60u,
    CSB_V1_FMTOWNS_UTILE_ICON_PALETTE_OFFSET = 0x17db0u,
    CSB_V1_FMTOWNS_UTILJ_ICON_PALETTE_OFFSET = 0x17e18u,
    /* ReDMCSB CEDT019.C:18-35 / CEDTFNT.C:43-94.  These raw offsets were
     * located once in the retail F31E/F31J images and remain separately
     * hash-bound below; they must never be replaced by a compiled font. */
    CSB_V1_FMTOWNS_UTILE_INTERFACE_FONT_OFFSET = 0x150d8u,
    CSB_V1_FMTOWNS_UTILJ_INTERFACE_FONT_OFFSET = 0x15140u,
    CSB_V1_FMTOWNS_UTILITY_INTERFACE_FONT_FNV1A = 0x8c36f65bu,
    CSB_V1_FMTOWNS_UTILE_MIRROR_BITMAP_OFFSET = 0x14e78u,
    CSB_V1_FMTOWNS_UTILJ_MIRROR_BITMAP_OFFSET = 0x14ee0u,
    CSB_V1_FMTOWNS_UTILITY_MIRROR_BITMAP_FNV1A = 0xf8a19ba4u,
    CSB_V1_FMTOWNS_UTILE_FILE_PICKER_ARROWS_OFFSET = 0x14f70u,
    CSB_V1_FMTOWNS_UTILJ_FILE_PICKER_ARROWS_OFFSET = 0x14fd8u,
    CSB_V1_FMTOWNS_UTILITY_FILE_PICKER_ARROWS_FNV1A = 0xe2226054u,
    /* CEDT001.C F7000's source-owned filename operation.  These are raw
     * offsets in the verified UTILE/UTILJ P3 images, confirmed by the
     * #CHAMP_NAME# and 2:\\#CHAMP_NAME#.CMP string references. */
    CSB_V1_FMTOWNS_UTILE_SAVE_CMP_MAPPING_OFFSET = 0x11ffcu,
    CSB_V1_FMTOWNS_UTILJ_SAVE_CMP_MAPPING_OFFSET = 0x12064u,
    CSB_V1_FMTOWNS_UTILITY_SAVE_CMP_MAPPING_BYTES = 20u,
    /* The retail F31E/F31J programs carry identical 10*32*32 selector
     * tables. These offsets are from the raw verified executable image. */
    CSB_V1_FMTOWNS_CHTWE_MUSIC_TABLE_OFFSET = 271144u,
    CSB_V1_FMTOWNS_CHTWJ_MUSIC_TABLE_OFFSET = 271624u,
    CSB_V1_FMTOWNS_GAME_MUSIC_TABLE_FNV1A = 0x3faffb70u
};

/* ReDMCSB DRAWVIEW.C:383-407 / VIDEODRV.C:765-782,
 * MEDIA670_F31E_F31J. C28_ENTRANCE_CSB is sixteen indexed native six-bit
 * RGB entries plus the COLOR_DEF terminator in each retail Game program. */
static const uint8_t k_fmtowns_c28_entrance_palette[17u * 4u] = {
    0x00,0x00,0x00,0x00, 0x01,0x1b,0x1b,0x1b,
    0x02,0x23,0x23,0x23, 0x03,0x23,0x13,0x03,
    0x04,0x33,0x2b,0x23, 0x05,0x13,0x0b,0x0b,
    0x06,0x03,0x03,0x23, 0x07,0x03,0x03,0x2b,
    0x08,0x23,0x1b,0x13, 0x09,0x3f,0x03,0x03,
    0x0a,0x2b,0x23,0x1b, 0x0b,0x1b,0x13,0x0b,
    0x0c,0x13,0x13,0x13, 0x0d,0x2b,0x2b,0x2b,
    0x0e,0x1b,0x0b,0x03, 0x0f,0x3f,0x3f,0x3f,
    0xff,0x00,0x00,0x00
};

/* First row of the contiguous G8151_LIGHT0..G8156_LIGHT5 corpus. The whole
 * executable is already edition-hash verified; this exact source signature
 * locates the tables and the structural checks below admit all six rows. */
static const uint8_t k_fmtowns_c00_light0_palette[17u * 4u] = {
    0x10,0x00,0x00,0x00, 0x11,0x1b,0x1b,0x1b,
    0x12,0x24,0x24,0x24, 0x13,0x1b,0x09,0x00,
    0x14,0x00,0x36,0x36, 0x15,0x24,0x12,0x00,
    0x16,0x00,0x24,0x00, 0x17,0x00,0x36,0x00,
    0x18,0x3f,0x00,0x00, 0x19,0x3f,0x2d,0x00,
    0x1a,0x36,0x24,0x1b, 0x1b,0x3f,0x3f,0x00,
    0x1c,0x12,0x12,0x12, 0x1d,0x2d,0x2d,0x2d,
    0x1e,0x00,0x00,0x3f, 0x1f,0x3f,0x3f,0x3f,
    0xff,0x00,0x00,0x00
};

static int csb_v1_fmtowns_game_read_span(const char *path, uint32_t offset,
                                         unsigned char *bytes, size_t size);
static int csb_v1_fmtowns_utility_read_span(
    const CSB_V1_FmtownsUtilityHandoffReceipt *receipt, uint32_t offset,
    unsigned char *bytes, size_t size);
static int csb_v1_fmtowns_game_read_startup_span(
    const CSB_V1_FmtownsGameHandoffReceipt *receipt, uint32_t offset,
    unsigned char *bytes, size_t size);
static int csb_v1_fmtowns_game_read_executable_span(
    const CSB_V1_FmtownsGameHandoffReceipt *receipt, uint32_t offset,
    unsigned char *bytes, size_t size);
static uint32_t csb_v1_fmtowns_game_bytes_fnv1a(const unsigned char *bytes,
                                                  size_t size);
static uint32_t csb_v1_fmtowns_game_file_fnv1a(const char *path,
                                               uint32_t *out_size);
static uint16_t csb_v1_fmtowns_game_read_le16(const unsigned char *bytes);
static uint32_t csb_v1_fmtowns_game_read_le32(const unsigned char *bytes);
static int csb_v1_fmtowns_game_original_backup_path(
    const char *path, char *out, size_t out_size);

static uint16_t csb_v1_fmtowns_game_next_source_random_word(
    uint32_t *state)
{
    if (!state) return 0u;
    /* ReDMCSB BASE.C F0027 advances the persisted G0349 stream before
     * returning the high sixteen bits. CEDTINC8.C F7052 consumes sixteen
     * words for save-part keys, then CEDTINC6.C F7062 consumes 127 more for
     * the header's random first half. */
    *state = *state * UINT32_C(0xbb40e62d) + UINT32_C(11);
    return (uint16_t)(*state >> 8);
}

static int csb_v1_fmtowns_game_copy_file(const char *source_path,
                                         const char *destination_path)
{
    unsigned char buffer[65536];
    FILE *source = NULL;
    FILE *destination = NULL;
    size_t count;
    int ok = 0;

    if (!source_path || !destination_path ||
        !(source = fopen(source_path, "rb")) ||
        !(destination = fopen(destination_path, "wb"))) {
        if (source) fclose(source);
        if (destination) fclose(destination);
        return 0;
    }
    while ((count = fread(buffer, 1u, sizeof(buffer), source)) != 0u) {
        if (fwrite(buffer, 1u, count, destination) != count) goto done;
    }
    ok = !ferror(source) && fflush(destination) == 0;
done:
    if (fclose(destination) != 0) ok = 0;
    if (fclose(source) != 0) ok = 0;
    if (!ok) remove(destination_path);
    return ok;
}

static void csb_v1_fmtowns_game_write_le16(unsigned char *bytes,
                                           uint16_t value)
{
    bytes[0] = (unsigned char)value;
    bytes[1] = (unsigned char)(value >> 8);
}

static void csb_v1_fmtowns_game_write_le32(unsigned char *bytes,
                                           uint32_t value)
{
    csb_v1_fmtowns_game_write_le16(bytes, (uint16_t)value);
    csb_v1_fmtowns_game_write_le16(bytes + 2u, (uint16_t)(value >> 16));
}

int csb_v1_fmtowns_game_encode_dungeon_tail(
    const CSB_V1_DungeonData *dungeon, uint8_t *out_bytes, size_t out_size)
{
    CSB_V1_DungeonData check;
    uint16_t checksum = 0u;
    size_t index;

    if (!dungeon || !dungeon->raw_data || !out_bytes || dungeon->raw_size < 2 ||
        out_size != (size_t)dungeon->raw_size + 2u || dungeon->square_bytes != 1)
        return 0;
    memcpy(out_bytes, dungeon->raw_data, (size_t)dungeon->raw_size);
    for (index = 0u; index < (size_t)dungeon->raw_size; ++index)
        checksum = (uint16_t)(checksum + out_bytes[index]);
    csb_v1_fmtowns_game_write_le16(
        out_bytes + dungeon->raw_size, checksum);

    /* Re-admit the exact bytes through the source loader. This checks the
     * header, map descriptors, database spans and one-byte square layout
     * before the caller can replace the native slot. */
    memset(&check, 0, sizeof(check));
    if (csb_v1_dungeon_load_source_bytes(
            &check, out_bytes, dungeon->raw_size) != 0) return 0;
    csb_v1_dungeon_free(&check);
    return 1;
}

static int csb_v1_fmtowns_game_write_dungeon_tail(
    unsigned char *file_bytes, uint32_t file_size,
    const CSB_V1_FmtownsUserSaveReceipt *receipt,
    const CSB_V1_DungeonData *dungeon)
{
    if (!file_bytes || !receipt || !dungeon ||
        receipt->dungeon_tail_size < 2u ||
        receipt->dungeon_tail_offset > file_size ||
        file_size - receipt->dungeon_tail_offset < 2u ||
        receipt->dungeon_tail_size >
            file_size - receipt->dungeon_tail_offset - 2u)
        return 0;
    return csb_v1_fmtowns_game_encode_dungeon_tail(
        dungeon, file_bytes + receipt->dungeon_tail_offset,
        (size_t)receipt->dungeon_tail_size + 2u);
}

/* F0435 keeps its save handle open through the party, portrait and dungeon
 * reads (ReDMCSB LOADSAVE.C:2721-2829). A host receipt must have the same
 * ownership rule: offsets are valid only for the exact image it admitted. */
static int csb_v1_fmtowns_game_receipt_source_matches(
    const CSB_V1_FmtownsGameHandoffReceipt *receipt)
{
    uint32_t current_size = 0u;

    if (!receipt) return 0;
    if (receipt->startup_mini_bytes && receipt->startup_mini_bytes_size != 0u)
        return receipt->startup_mini_bytes_size == receipt->startup_mini_size &&
            csb_v1_fmtowns_game_bytes_fnv1a(
                receipt->startup_mini_bytes,
                receipt->startup_mini_bytes_size) ==
                receipt->startup_mini_fnv1a;
    return receipt->startup_mini_path[0] &&
        receipt->startup_mini_size != 0u &&
        receipt->startup_mini_fnv1a != 0u &&
        csb_v1_fmtowns_game_file_fnv1a(receipt->startup_mini_path,
                                        &current_size) ==
            receipt->startup_mini_fnv1a &&
        current_size == receipt->startup_mini_size;
}

/* All reads from the packed CD's MINI.DAT must stay inside the member that
 * was admitted by the boot profile.  Loose development trees retain the
 * historical file-backed path. */
static int csb_v1_fmtowns_game_read_startup_span(
    const CSB_V1_FmtownsGameHandoffReceipt *receipt, uint32_t offset,
    unsigned char *bytes, size_t size)
{
    if (!receipt || !bytes || size == 0u ||
        offset > receipt->startup_mini_size ||
        size > receipt->startup_mini_size - offset) return 0;
    if (receipt->startup_mini_bytes &&
        receipt->startup_mini_bytes_size == receipt->startup_mini_size) {
        memcpy(bytes, receipt->startup_mini_bytes + offset, size);
        return 1;
    }
    return csb_v1_fmtowns_game_read_span(receipt->startup_mini_path, offset,
                                          bytes, size);
}

static int csb_v1_fmtowns_game_read_executable_span(
    const CSB_V1_FmtownsGameHandoffReceipt *receipt, uint32_t offset,
    unsigned char *bytes, size_t size)
{
    if (!receipt || !bytes || size == 0u ||
        offset > receipt->executable_size ||
        size > receipt->executable_size - offset) return 0;
    if (receipt->executable_bytes &&
        receipt->executable_bytes_size == receipt->executable_size) {
        memcpy(bytes, receipt->executable_bytes + offset, size);
        return 1;
    }
    return csb_v1_fmtowns_game_read_span(receipt->executable_path, offset,
                                          bytes, size);
}

/* MENU.C G0487:48-84 / DEFS.H SPELL:1755-1757. Both verified retail
 * executables contain the same 29 little-endian eight-byte records at
 * these raw-file offsets. Only their bytes populate the retained table. */
static int csb_v1_fmtowns_game_bind_spell_table(
    CSB_V1_FmtownsGameHandoffReceipt *receipt,
    CSB_V1_FmtownsSwitchLanguage language)
{
    uint8_t bytes[CSB_V1_FMTOWNS_GAME_SPELL_COUNT * 8u];
    uint32_t offset;
    unsigned int index;
    if (language == CSB_FMTOWNS_SWITCH_ENGLISH) offset = 0x2a07cu;
    else if (language == CSB_FMTOWNS_SWITCH_JAPANESE) offset = 0x2a254u;
    else return 0;
    if (!csb_v1_fmtowns_game_read_executable_span(receipt, offset, bytes, sizeof(bytes)) ||
        csb_v1_fmtowns_game_bytes_fnv1a(bytes, sizeof(bytes)) != 0x9fd916c2u) return 0;
    for (index = 0; index < CSB_V1_FMTOWNS_GAME_SPELL_COUNT; ++index) {
        const uint8_t *record = bytes + index * 8u;
        CSB_V1_FmtownsGameSpell *spell = &receipt->spells[index];
        spell->symbols = csb_v1_fmtowns_game_read_le32(record);
        spell->base_required_skill_level = record[4];
        spell->skill_index = record[5];
        spell->attributes = csb_v1_fmtowns_game_read_le16(record + 6u);
    }
    receipt->spell_table_source_offset = offset;
    receipt->spell_table_fnv1a = 0x9fd916c2u;
    receipt->spell_table_verified = 1;
    return 1;
}

static int csb_v1_fmtowns_game_bind_entrance_palette(
    CSB_V1_FmtownsGameHandoffReceipt *receipt)
{
    uint8_t *image;
    uint32_t offset;
    uint32_t color;
    if (!receipt || receipt->executable_size <
            sizeof(k_fmtowns_c28_entrance_palette)) return 0;
    image = (uint8_t *)malloc(receipt->executable_size);
    if (!image || !csb_v1_fmtowns_game_read_executable_span(
            receipt, 0u, image, receipt->executable_size)) {
        free(image);
        return 0;
    }
    for (offset = 0u; offset <= receipt->executable_size -
             sizeof(k_fmtowns_c28_entrance_palette); ++offset) {
        if (memcmp(image + offset, k_fmtowns_c28_entrance_palette,
                   sizeof(k_fmtowns_c28_entrance_palette)) != 0) continue;
        for (color = 0u; color < 16u; ++color) {
            receipt->entrance_palette_rgb6[color][0] =
                image[offset + color * 4u + 1u];
            receipt->entrance_palette_rgb6[color][1] =
                image[offset + color * 4u + 2u];
            receipt->entrance_palette_rgb6[color][2] =
                image[offset + color * 4u + 3u];
        }
        receipt->entrance_palette_source_offset = offset;
        receipt->entrance_palette_verified = 1;
        free(image);
        return 1;
    }
    free(image);
    return 0;
}

static int csb_v1_fmtowns_game_bind_dungeon_palettes(
    CSB_V1_FmtownsGameHandoffReceipt *receipt)
{
    enum { PALETTE_BYTES = 17 * 4, CORPUS_BYTES = 6 * PALETTE_BYTES };
    uint8_t *image;
    uint32_t offset;
    uint32_t level;
    uint32_t color;
    if (!receipt || receipt->executable_size < CORPUS_BYTES) return 0;
    image = (uint8_t *)malloc(receipt->executable_size);
    if (!image || !csb_v1_fmtowns_game_read_executable_span(
            receipt, 0u, image, receipt->executable_size)) {
        free(image);
        return 0;
    }
    for (offset = 0u; offset <= receipt->executable_size - CORPUS_BYTES;
         ++offset) {
        if (memcmp(image + offset, k_fmtowns_c00_light0_palette,
                   sizeof(k_fmtowns_c00_light0_palette)) != 0) continue;
        for (level = 0u; level < 6u; ++level) {
            const uint8_t *row = image + offset + level * PALETTE_BYTES;
            for (color = 0u; color < 16u; ++color) {
                /* The original F31 LIGHT3 row contains its documented
                 * duplicate 0x18 index at slot 7; preserve it exactly. */
                uint8_t expected = (uint8_t)(0x10u + color);
                if (level == 3u && color == 7u) expected = 0x18u;
                if (row[color * 4u] != expected ||
                    row[color * 4u + 1u] > 0x3fu ||
                    row[color * 4u + 2u] > 0x3fu ||
                    row[color * 4u + 3u] > 0x3fu) break;
            }
            if (color != 16u || row[64] != 0xffu || row[65] != 0u ||
                row[66] != 0u || row[67] != 0u) break;
        }
        if (level != 6u) continue;
        for (level = 0u; level < 6u; ++level) {
            const uint8_t *row = image + offset + level * PALETTE_BYTES;
            for (color = 0u; color < 16u; ++color) {
                receipt->dungeon_palette_rgb6[level][color][0] =
                    row[color * 4u + 1u];
                receipt->dungeon_palette_rgb6[level][color][1] =
                    row[color * 4u + 2u];
                receipt->dungeon_palette_rgb6[level][color][2] =
                    row[color * 4u + 3u];
            }
        }
        receipt->dungeon_palettes_source_offset = offset;
        receipt->dungeon_palettes_verified = 1;
        free(image);
        return 1;
    }
    free(image);
    return 0;
}

static int csb_v1_fmtowns_game_resolve_active_group_owners(
    CSB_V1_FmtownsStartupState *state)
{
    uint16_t slot;

    if (!state || !state->dungeon.raw_data || state->party_map_index < 0 ||
        state->party_map_index >= state->dungeon.level_count) {
        return 0;
    }
    for (slot = 0u; slot < state->active_group_capacity; ++slot) {
        const int16_t group_index = (int16_t)csb_v1_fmtowns_game_read_le16(
            state->active_groups[slot]);
        int level;
        int matches = 0;

        if (group_index < 0) continue;
        for (level = 0; level < state->dungeon.level_count; ++level) {
            int x;
            for (x = 0; x < state->dungeon.level_widths[level]; ++x) {
                int y;
                for (y = 0; y < state->dungeon.level_heights[level]; ++y) {
                    int thing = csb_v1_dungeon_get_first_thing(
                        &state->dungeon, level, x, y);
                    int guard;
                    for (guard = 0; guard < 128 && thing >= 0 &&
                         thing != THING_ENDOFLIST; ++guard) {
                        const uint8_t *record;
                        int type = -1;
                        int size = 0;

                        record = csb_v1_dungeon_get_thing_record(
                            &state->dungeon, (uint16_t)thing, &type, NULL,
                            &size);
                        if (!record || size < 2) return 0;
                        if (type == CSB_V1_THING_TYPE_GROUP &&
                            ((uint16_t)thing & 0x03ffu) ==
                                (uint16_t)group_index) {
                            if (++matches != 1 || level != state->party_map_index) {
                                return 0;
                            }
                            state->active_group_owners[slot].valid = 1;
                            state->active_group_owners[slot].group_thing_index =
                                (uint16_t)group_index;
                            state->active_group_owners[slot].group_thing =
                                (uint16_t)thing;
                            state->active_group_owners[slot].map_index = level;
                            state->active_group_owners[slot].map_x = x;
                            state->active_group_owners[slot].map_y = y;
                            ++state->active_group_resolved_count;
                        }
                        thing = (int)csb_v1_fmtowns_game_read_le16(record);
                    }
                    if (guard == 128 && thing != THING_ENDOFLIST) return 0;
                }
            }
        }
        if (matches != 1) {
            return 0;
        }
    }
    return state->active_group_resolved_count == state->active_group_count;
}

static void csb_v1_fmtowns_game_copy_text(char *dst, size_t dst_size,
                                           const unsigned char *src,
                                           size_t src_size)
{
    size_t count = 0u;
    if (!dst || dst_size == 0u || !src) return;
    while (count + 1u < dst_size && count < src_size && src[count] != '\0') {
        dst[count] = (char)src[count];
        ++count;
    }
    dst[count] = '\0';
}

static const uint8_t k_csb_v1_fmtowns_thing_data_bytes[16] = {
    4u, 6u, 4u, 8u, 16u, 4u, 4u, 4u,
    4u, 8u, 4u, 0u, 0u, 0u, 8u, 4u
};

static int csb_v1_fmtowns_game_read_sum_span(
    const CSB_V1_FmtownsGameHandoffReceipt *receipt,
    uint32_t file_size, uint32_t *in_out_offset,
    uint32_t byte_count, uint16_t *in_out_checksum)
{
    unsigned char *bytes;
    uint32_t index;

    if (!receipt || !in_out_offset || !in_out_checksum ||
        *in_out_offset > file_size || byte_count > file_size - *in_out_offset)
        return 0;
    if (byte_count == 0u) return 1;
    bytes = (unsigned char *)malloc(byte_count);
    if (!bytes || !csb_v1_fmtowns_game_read_startup_span(
                      receipt, *in_out_offset, bytes, byte_count)) {
        free(bytes);
        return 0;
    }
    for (index = 0u; index < byte_count; ++index)
        *in_out_checksum = (uint16_t)(*in_out_checksum + bytes[index]);
    free(bytes);
    *in_out_offset += byte_count;
    return 1;
}

static int csb_v1_fmtowns_game_startup_mini_dungeon_tail_open(
    CSB_V1_FmtownsGameHandoffReceipt *receipt)
{
    unsigned char header[CSB_V1_FMTOWNS_DUNGEON_HEADER_BYTES];
    unsigned char maps[255u * CSB_V1_FMTOWNS_DUNGEON_MAP_BYTES];
    uint32_t offset;
    uint32_t map_bytes;
    uint16_t checksum = 0u;
    uint16_t column_count = 0u;
    uint16_t raw_map_byte_count;
    uint16_t text_data_word_count;
    uint16_t square_first_thing_count;
    uint16_t saved_checksum;
    uint32_t map_index;
    uint32_t type;
    uint8_t map_count;

    if (!receipt || receipt->startup_mini_size <
            CSB_V1_FMTOWNS_EXTERNAL_PORTRAIT_BYTES *
                CSB_V1_FMTOWNS_EXTERNAL_PORTRAIT_COUNT ||
        receipt->startup_mini_verified_save_body_offset >
            receipt->startup_mini_size - CSB_V1_FMTOWNS_EXTERNAL_PORTRAIT_BYTES *
                        CSB_V1_FMTOWNS_EXTERNAL_PORTRAIT_COUNT) return 0;
    offset = receipt->startup_mini_verified_save_body_offset +
             CSB_V1_FMTOWNS_EXTERNAL_PORTRAIT_BYTES *
                 CSB_V1_FMTOWNS_EXTERNAL_PORTRAIT_COUNT;
    if (!csb_v1_fmtowns_game_read_startup_span(receipt, offset, header,
                                               sizeof(header)))
        return 0;
    for (map_index = 0u; map_index < sizeof(header); ++map_index)
        checksum = (uint16_t)(checksum + header[map_index]);
    raw_map_byte_count = csb_v1_fmtowns_game_read_le16(
        header + CSB_V1_FMTOWNS_DUNGEON_RAW_MAP_BYTES_OFFSET);
    map_count = header[CSB_V1_FMTOWNS_DUNGEON_MAP_COUNT_OFFSET];
    text_data_word_count = csb_v1_fmtowns_game_read_le16(
        header + CSB_V1_FMTOWNS_DUNGEON_TEXT_WORD_COUNT_OFFSET);
    square_first_thing_count = csb_v1_fmtowns_game_read_le16(
        header + CSB_V1_FMTOWNS_DUNGEON_SQUARE_FIRST_THING_COUNT_OFFSET);
    map_bytes = (uint32_t)map_count * CSB_V1_FMTOWNS_DUNGEON_MAP_BYTES;
    if (map_bytes > sizeof(maps) || offset > receipt->startup_mini_size -
            sizeof(header) || map_bytes > receipt->startup_mini_size - offset -
            sizeof(header) ||
        !csb_v1_fmtowns_game_read_startup_span(
            receipt, offset + sizeof(header), maps, map_bytes)) return 0;
    for (map_index = 0u; map_index < map_bytes; ++map_index)
        checksum = (uint16_t)(checksum + maps[map_index]);
    for (map_index = 0u; map_index < map_count; ++map_index) {
        uint16_t descriptor = csb_v1_fmtowns_game_read_le16(
            maps + map_index * CSB_V1_FMTOWNS_DUNGEON_MAP_BYTES +
            CSB_V1_FMTOWNS_DUNGEON_MAP_WIDTH_OFFSET);
        column_count = (uint16_t)(column_count + ((descriptor >> 6) & 0x1fu) +
                                  1u);
    }
    if (receipt->startup_mini_party_map_index < 0 ||
        receipt->startup_mini_party_map_index >= map_count ||
        receipt->startup_mini_party_map_x < 0 ||
        receipt->startup_mini_party_map_y < 0 ||
        receipt->startup_mini_party_direction < 0 ||
        receipt->startup_mini_party_direction > 3) return 0;
    {
        const unsigned char *party_map = maps +
            (uint32_t)receipt->startup_mini_party_map_index *
                CSB_V1_FMTOWNS_DUNGEON_MAP_BYTES;
        const uint16_t descriptor = csb_v1_fmtowns_game_read_le16(
            party_map + CSB_V1_FMTOWNS_DUNGEON_MAP_WIDTH_OFFSET);
        const int width = ((descriptor >> 6) & 0x1fu) + 1;
        const int height = ((descriptor >> 11) & 0x1fu) + 1;
        if (receipt->startup_mini_party_map_x >= width ||
            receipt->startup_mini_party_map_y >= height) return 0;
    }
    offset += sizeof(header) + map_bytes;
    if (!csb_v1_fmtowns_game_read_sum_span(
            receipt, receipt->startup_mini_size, &offset,
            (uint32_t)column_count * 2u, &checksum) ||
        !csb_v1_fmtowns_game_read_sum_span(
            receipt, receipt->startup_mini_size, &offset,
            (uint32_t)square_first_thing_count * 2u, &checksum) ||
        !csb_v1_fmtowns_game_read_sum_span(
            receipt, receipt->startup_mini_size, &offset,
            (uint32_t)text_data_word_count * 2u, &checksum)) return 0;
    for (type = 0u; type < 16u; ++type) {
        uint16_t count = csb_v1_fmtowns_game_read_le16(
            header + CSB_V1_FMTOWNS_DUNGEON_THING_COUNTS_OFFSET + type * 2u);
        if (!csb_v1_fmtowns_game_read_sum_span(
                receipt, receipt->startup_mini_size, &offset,
                (uint32_t)count * k_csb_v1_fmtowns_thing_data_bytes[type],
                &checksum)) return 0;
    }
    if (!csb_v1_fmtowns_game_read_sum_span(
            receipt, receipt->startup_mini_size, &offset, raw_map_byte_count,
            &checksum) ||
        offset > receipt->startup_mini_size -
            CSB_V1_FMTOWNS_DUNGEON_TRAILER_BYTES ||
        !csb_v1_fmtowns_game_read_startup_span(receipt, offset, header,
                                       CSB_V1_FMTOWNS_DUNGEON_TRAILER_BYTES))
        return 0;
    saved_checksum = csb_v1_fmtowns_game_read_le16(header);
    if (saved_checksum != checksum ||
        offset + CSB_V1_FMTOWNS_DUNGEON_TRAILER_BYTES !=
            receipt->startup_mini_size) return 0;
    receipt->startup_mini_dungeon_map_count = map_count;
    receipt->startup_mini_dungeon_column_count = column_count;
    receipt->startup_mini_dungeon_tail_checksum = checksum;
    receipt->startup_mini_first_map_offset_x = maps[6u];
    receipt->startup_mini_first_map_offset_y = maps[7u];
    receipt->startup_mini_dungeon_tail_offset =
        receipt->startup_mini_verified_save_body_offset +
        CSB_V1_FMTOWNS_EXTERNAL_PORTRAIT_BYTES *
            CSB_V1_FMTOWNS_EXTERNAL_PORTRAIT_COUNT;
    receipt->startup_mini_dungeon_tail_size = offset -
        receipt->startup_mini_dungeon_tail_offset;
    receipt->startup_mini_dungeon_tail_verified = 1;
    return 1;
}

int csb_v1_fmtowns_game_copy_verified_dungeon_tail(
    const CSB_V1_FmtownsGameHandoffReceipt *receipt,
    uint8_t *out_bytes, size_t out_size)
{
    if (!receipt || !receipt->valid || !receipt->startup_mini_verified ||
        !receipt->startup_mini_dungeon_tail_verified || !out_bytes ||
        receipt->startup_mini_dungeon_tail_size == 0u ||
        out_size != receipt->startup_mini_dungeon_tail_size ||
        !csb_v1_fmtowns_game_receipt_source_matches(receipt)) return 0;
    return csb_v1_fmtowns_game_read_startup_span(
        receipt, receipt->startup_mini_dungeon_tail_offset, out_bytes, out_size);
}

int csb_v1_fmtowns_game_load_startup_party(
    const CSB_V1_FmtownsGameHandoffReceipt *receipt,
    CSB_V1_PartyState *out_party)
{
    enum {
        champion_bytes = 319u,
        champion_count = 4u,
        party_offset = CSB_V1_FMTOWNS_SAVE_HEADER_BYTES +
            CSB_V1_FMTOWNS_GLOBAL_DATA_BYTES,
        name_offset = 0u,
        name_bytes = 8u,
        title_offset = 8u,
        /* ReDMCSB DEFS.H CHAMPION: Name[8], Title[20], Direction, Cell.
         * These are the F31's 319-byte records; do not reuse the PC34
         * 16-byte title layout here. */
        title_bytes = 20u,
        direction_offset = 28u,
        cell_offset = 29u,
        action_offset = 32u,
        incantation_offset = 34u,
        facing_offset = 40u,
        poison_event_count_offset = 42u,
        enable_action_event_offset = 44u,
        hide_damage_event_offset = 46u,
        attributes_offset = 48u,
        wounds_offset = 50u,
        health_offset = 52u,
        food_offset = 66u,
        statistics_offset = 70u,
        skill_offset = 92u,
        slots_offset = 212u,
        load_offset = 272u,
        shield_offset = 274u
    };
    unsigned char bytes[CSB_V1_FMTOWNS_CHAMPION_PARTY_BYTES];
    unsigned char header[CSB_V1_FMTOWNS_SAVE_HEADER_BYTES];
    uint32_t champion_part_offset;
    uint32_t champion_index;
    uint16_t key;
    uint16_t checksum;
    int stat;
    static const int k_source_stat_to_firestaff[CSB_V1_STAT_COUNT] =
        { 6, 0, 1, 2, 3, 4, 5 };

    if (!receipt || !out_party || !receipt->valid ||
        !receipt->startup_mini_verified ||
        !receipt->startup_mini_header_verified ||
        !receipt->startup_mini_save_parts_verified ||
        receipt->startup_mini_party_champion_count == 0u ||
        receipt->startup_mini_party_champion_count > CSB_V1_MAX_CHAMPIONS ||
        receipt->startup_mini_active_group_capacity == 0u ||
        receipt->startup_mini_active_group_capacity >
            CSB_V1_FMTOWNS_USER_SAVE_ACTIVE_GROUP_CAPACITY ||
        !csb_v1_fmtowns_game_receipt_source_matches(receipt) ||
        !csb_v1_fmtowns_game_read_startup_span(receipt, 0u, header,
                                                sizeof(header)) ||
        !redmcsb_f7061_is_read_save_header_successful_pc34(
            header, sizeof(header), CSB_V1_FMTOWNS_CSB_HEADER_KEY_WORD_INDEX)) {
        return 0;
    }
    champion_part_offset = CSB_V1_FMTOWNS_SAVE_HEADER_BYTES +
        CSB_V1_FMTOWNS_GLOBAL_DATA_BYTES +
        (uint32_t)receipt->startup_mini_active_group_capacity *
            CSB_V1_FMTOWNS_ACTIVE_GROUP_BYTES;
    if (!csb_v1_fmtowns_game_read_startup_span(
            receipt, champion_part_offset, bytes, sizeof(bytes))) return 0;
    key = csb_v1_fmtowns_game_read_le16(
        header + CSB_V1_FMTOWNS_SAVE_HEADER_KEYS_OFFSET + 4u);
    checksum = csb_v1_fmtowns_game_read_le16(
        header + CSB_V1_FMTOWNS_SAVE_HEADER_CHECKSUMS_OFFSET + 4u);
    if (!redmcsb_f7057_read_save_part_with_checksum_pc34(
            bytes, sizeof(bytes), key, checksum)) return 0;

    memset(out_party, 0, sizeof(*out_party));
    out_party->ChampionCount = receipt->startup_mini_party_champion_count;
    out_party->PartyDirection = receipt->startup_mini_party_direction & 3;
    out_party->PartyMapX = receipt->startup_mini_party_map_x;
    out_party->PartyMapY = receipt->startup_mini_party_map_y;
    out_party->LeaderIndex = -1;
    out_party->MagicCasterIndex = -1;
    for (champion_index = 0u; champion_index < champion_count;
         ++champion_index) {
        const unsigned char *source = bytes + champion_index * champion_bytes;
        CSB_V1_Champion *champion = &out_party->Champions[champion_index];

        csb_v1_champion_init(champion);
        csb_v1_fmtowns_game_copy_text(champion->Name, sizeof(champion->Name),
                                       source + name_offset, name_bytes);
        csb_v1_fmtowns_game_copy_text(champion->Title, sizeof(champion->Title),
                                       source + title_offset, title_bytes);
        champion->Cell = source[cell_offset] & 3u;
        champion->Direction = source[direction_offset] & 3u;
        champion->ActionIndex = source[action_offset];
        memcpy(champion->Incantation, source + incantation_offset,
               sizeof(champion->Incantation));
        champion->CsbWinFacing3 = source[facing_offset] & 3u;
        champion->PoisonEventCount = source[poison_event_count_offset];
        champion->EnableActionEventIndex = (int16_t)csb_v1_fmtowns_game_read_le16(
            source + enable_action_event_offset);
        champion->HideDamageReceivedEventIndex = (int16_t)csb_v1_fmtowns_game_read_le16(
            source + hide_damage_event_offset);
        champion->Attributes = csb_v1_fmtowns_game_read_le16(source + attributes_offset);
        champion->Wounds = csb_v1_fmtowns_game_read_le16(source + wounds_offset);
        champion->CurrentHealth = (int16_t)csb_v1_fmtowns_game_read_le16(source + health_offset);
        champion->MaximumHealth = (int16_t)csb_v1_fmtowns_game_read_le16(source + health_offset + 2u);
        champion->CurrentStamina = (int16_t)csb_v1_fmtowns_game_read_le16(source + health_offset + 4u);
        champion->MaximumStamina = (int16_t)csb_v1_fmtowns_game_read_le16(source + health_offset + 6u);
        champion->CurrentMana = (int16_t)csb_v1_fmtowns_game_read_le16(source + health_offset + 8u);
        champion->MaximumMana = (int16_t)csb_v1_fmtowns_game_read_le16(source + health_offset + 10u);
        champion->Food = (int16_t)csb_v1_fmtowns_game_read_le16(source + food_offset);
        champion->Water = (int16_t)csb_v1_fmtowns_game_read_le16(source + food_offset + 2u);
        for (stat = 0; stat < CSB_V1_STAT_COUNT; ++stat) {
            const int target = k_source_stat_to_firestaff[stat];
            champion->Statistics[target][CSB_V1_STAT_MAX] = source[statistics_offset + (uint32_t)stat * 3u];
            champion->Statistics[target][CSB_V1_STAT_CUR] = source[statistics_offset + (uint32_t)stat * 3u + 1u];
            champion->Statistics[target][CSB_V1_STAT_MIN] = source[statistics_offset + (uint32_t)stat * 3u + 2u];
        }
        for (stat = 0; stat < CSB_V1_FULL_SKILL_COUNT; ++stat) {
            const uint32_t offset = skill_offset + (uint32_t)stat * 6u;
            const uint16_t level = csb_v1_fmtowns_game_read_le16(source + offset);
            champion->SkillExperience[stat] =
                (uint32_t)csb_v1_fmtowns_game_read_le16(source + offset + 2u) |
                ((uint32_t)csb_v1_fmtowns_game_read_le16(source + offset + 4u) << 16);
            if (stat < CSB_V1_SKILL_COUNT)
                champion->Skills[stat] = (uint8_t)(level > 255u ? 255u : level);
        }
        champion->SkillExperienceValid = 1u;
        for (stat = 0; stat < CSB_V1_SLOT_COUNT; ++stat)
            champion->Slots[stat] = csb_v1_fmtowns_game_read_le16(
                source + slots_offset + (uint32_t)stat * 2u);
        champion->Load = csb_v1_fmtowns_game_read_le16(source + load_offset);
        champion->ShieldStrength = csb_v1_fmtowns_game_read_le16(source + shield_offset);
    }
    return 1;
}

int csb_v1_fmtowns_game_load_startup_portraits(
    const CSB_V1_FmtownsGameHandoffReceipt *receipt,
    CSB_V1_FmtownsStartupPortraitReceipt *out_receipt)
{
    uint32_t source_offset;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!receipt || !receipt->valid || !receipt->startup_mini_verified ||
        !receipt->startup_mini_header_verified ||
        !receipt->startup_mini_save_parts_verified ||
        !receipt->startup_mini_dungeon_tail_verified ||
        !csb_v1_fmtowns_game_receipt_source_matches(receipt) ||
        receipt->startup_mini_verified_save_body_offset > UINT32_MAX -
            sizeof(out_receipt->source_bytes)) return 0;
    source_offset = receipt->startup_mini_verified_save_body_offset;
    /* ReDMCSB CEDT019.C F2124 reads the four external portraits between
     * F0435's five save parts and F7063's dungeon tail. Keep that original
     * planar form: decoding or a fallback portrait belongs to no host model. */
    if (source_offset + sizeof(out_receipt->source_bytes) !=
            receipt->startup_mini_dungeon_tail_offset ||
        !csb_v1_fmtowns_game_read_startup_span(
            receipt, source_offset,
            (unsigned char *)out_receipt->source_bytes,
            sizeof(out_receipt->source_bytes))) return 0;
    out_receipt->valid = 1;
    out_receipt->language = receipt->language;
    out_receipt->variant_id = receipt->variant_id;
    out_receipt->source_file_offset = source_offset;
    out_receipt->source_size = sizeof(out_receipt->source_bytes);
    out_receipt->source_fnv1a = csb_v1_fmtowns_game_bytes_fnv1a(
        (const unsigned char *)out_receipt->source_bytes,
        sizeof(out_receipt->source_bytes));
    out_receipt->source_evidence =
        "ReDMCSB LOADSAVE.C F0435; CEDT019.C F2124 lines 85-109; "
        "CEDT006.C F7033/F7040 lines 372-395/512-620";
    return 1;
}

int csb_v1_fmtowns_game_load_startup_dungeon(
    const CSB_V1_FmtownsGameHandoffReceipt *receipt,
    CSB_V1_DungeonData *out_dungeon)
{
    uint8_t *bytes;
    int result;

    uint32_t source_size = 0u;
    if (!receipt || !out_dungeon || !receipt->valid ||
        !receipt->startup_mini_verified) return 0;
    if (receipt->startup_mini_dungeon_tail_verified &&
        receipt->startup_mini_dungeon_tail_size != 0u) {
        source_size = receipt->startup_mini_dungeon_tail_size;
    } else if (receipt->startup_dungeon_bytes &&
               receipt->startup_dungeon_bytes_size != 0u) {
        source_size = (uint32_t)receipt->startup_dungeon_bytes_size;
    } else if (receipt->startup_dungeon_path[0] &&
               csb_v1_fmtowns_game_file_fnv1a(receipt->startup_dungeon_path,
                                              &source_size) != 0u) {
        /* Loose development trees use the original file-backed DUNGEON.DAT. */
    } else {
        return 0;
    }
    if (source_size > 0x7fffffffu) return 0;
    bytes = (uint8_t *)malloc(source_size);
    if (!bytes) return 0;
    if (receipt->startup_mini_dungeon_tail_verified &&
        receipt->startup_mini_dungeon_tail_size != 0u) {
        if (!csb_v1_fmtowns_game_copy_verified_dungeon_tail(
                receipt, bytes, source_size)) {
            free(bytes);
            return 0;
        }
    } else if (receipt->startup_dungeon_bytes) {
        memcpy(bytes, receipt->startup_dungeon_bytes, source_size);
    } else if (!csb_v1_fmtowns_game_read_span(
                   receipt->startup_dungeon_path, 0u, bytes, source_size)) {
        free(bytes);
        return 0;
    }
    memset(out_dungeon, 0, sizeof(*out_dungeon));
    result = csb_v1_dungeon_load_source_bytes(
        out_dungeon, bytes, (int)source_size);
    free(bytes);
    return result == 0;
}

void csb_v1_fmtowns_game_startup_state_free(
    CSB_V1_FmtownsStartupState *state)
{
    if (!state) return;
    csb_v1_dungeon_free(&state->dungeon);
    memset(state, 0, sizeof(*state));
}

int csb_v1_fmtowns_game_load_startup_state(
    const CSB_V1_FmtownsGameHandoffReceipt *receipt,
    CSB_V1_FmtownsStartupState *out_state)
{
    unsigned char header[CSB_V1_FMTOWNS_SAVE_HEADER_BYTES];
    unsigned char global_data[CSB_V1_FMTOWNS_GLOBAL_DATA_BYTES];
    unsigned char *events = NULL;
    unsigned char *timeline = NULL;
    uint32_t active_offset;
    uint32_t party_offset;
    uint32_t event_offset;
    uint32_t timeline_offset;
    uint32_t index;
    uint16_t keys[5];
    uint16_t checksums[5];

    if (!receipt || !out_state || !receipt->valid ||
        !receipt->startup_mini_verified ||
        !receipt->startup_mini_header_verified ||
        !receipt->startup_mini_save_parts_verified ||
        receipt->startup_mini_active_group_capacity == 0u ||
        receipt->startup_mini_active_group_capacity >
            CSB_V1_FMTOWNS_USER_SAVE_ACTIVE_GROUP_CAPACITY ||
        receipt->startup_mini_event_maximum_count == 0u ||
        receipt->startup_mini_event_maximum_count > DM1_EVENT_MAX_COUNT ||
        receipt->startup_mini_event_count >
            receipt->startup_mini_event_maximum_count ||
        receipt->startup_mini_first_unused_event_index >
            receipt->startup_mini_event_maximum_count ||
        receipt->startup_mini_current_active_group_count >
            receipt->startup_mini_active_group_capacity ||
        receipt->startup_mini_size == 0u ||
        receipt->startup_mini_fnv1a == 0u ||
        !csb_v1_fmtowns_game_receipt_source_matches(receipt) ||
        !csb_v1_fmtowns_game_read_startup_span(receipt, 0u, header,
                                                sizeof(header)) ||
        !redmcsb_f7061_is_read_save_header_successful_pc34(
            header, sizeof(header), CSB_V1_FMTOWNS_CSB_HEADER_KEY_WORD_INDEX) ||
        !csb_v1_fmtowns_game_read_startup_span(
            receipt, CSB_V1_FMTOWNS_SAVE_HEADER_BYTES, global_data,
            sizeof(global_data))) {
        return 0;
    }
    memset(out_state, 0, sizeof(*out_state));
    for (index = 0u; index < 5u; ++index) {
        keys[index] = csb_v1_fmtowns_game_read_le16(
            header + CSB_V1_FMTOWNS_SAVE_HEADER_KEYS_OFFSET + index * 2u);
        checksums[index] = csb_v1_fmtowns_game_read_le16(
            header + CSB_V1_FMTOWNS_SAVE_HEADER_CHECKSUMS_OFFSET + index * 2u);
    }
    if (!redmcsb_f7057_read_save_part_with_checksum_pc34(
            global_data, sizeof(global_data), keys[0], checksums[0])) return 0;
    active_offset = CSB_V1_FMTOWNS_SAVE_HEADER_BYTES +
        CSB_V1_FMTOWNS_GLOBAL_DATA_BYTES;
    party_offset = active_offset +
        (uint32_t)receipt->startup_mini_active_group_capacity *
            CSB_V1_FMTOWNS_ACTIVE_GROUP_BYTES;
    event_offset = party_offset + CSB_V1_FMTOWNS_CHAMPION_PARTY_BYTES;
    timeline_offset = event_offset +
        (uint32_t)receipt->startup_mini_event_maximum_count *
            CSB_V1_FMTOWNS_EVENT_BYTES;
    if (timeline_offset > receipt->startup_mini_size ||
        (uint32_t)receipt->startup_mini_event_maximum_count *
                CSB_V1_FMTOWNS_TIMELINE_ENTRY_BYTES >
            receipt->startup_mini_size - timeline_offset ||
        !(events = (unsigned char *)malloc(
            (uint32_t)receipt->startup_mini_event_maximum_count *
                CSB_V1_FMTOWNS_EVENT_BYTES)) ||
        !(timeline = (unsigned char *)malloc(
            (uint32_t)receipt->startup_mini_event_maximum_count *
                CSB_V1_FMTOWNS_TIMELINE_ENTRY_BYTES)) ||
        !csb_v1_fmtowns_game_read_startup_span(
            receipt, active_offset,
            (unsigned char *)out_state->active_groups,
            (uint32_t)receipt->startup_mini_active_group_capacity *
                CSB_V1_FMTOWNS_ACTIVE_GROUP_BYTES) ||
        !redmcsb_f7057_read_save_part_with_checksum_pc34(
            (unsigned char *)out_state->active_groups,
            (uint32_t)receipt->startup_mini_active_group_capacity *
                CSB_V1_FMTOWNS_ACTIVE_GROUP_BYTES, keys[1], checksums[1]) ||
        !csb_v1_fmtowns_game_read_startup_span(
            receipt, event_offset, events,
            (uint32_t)receipt->startup_mini_event_maximum_count *
                CSB_V1_FMTOWNS_EVENT_BYTES) ||
        !redmcsb_f7057_read_save_part_with_checksum_pc34(
            events, (uint32_t)receipt->startup_mini_event_maximum_count *
                CSB_V1_FMTOWNS_EVENT_BYTES, keys[3], checksums[3]) ||
        !csb_v1_fmtowns_game_read_startup_span(
            receipt, timeline_offset, timeline,
            (uint32_t)receipt->startup_mini_event_maximum_count *
                CSB_V1_FMTOWNS_TIMELINE_ENTRY_BYTES) ||
        !redmcsb_f7057_read_save_part_with_checksum_pc34(
            timeline, (uint32_t)receipt->startup_mini_event_maximum_count *
                CSB_V1_FMTOWNS_TIMELINE_ENTRY_BYTES, keys[4], checksums[4])) {
        free(events);
        free(timeline);
        return 0;
    }
    if (!csb_v1_fmtowns_game_load_startup_party(receipt, &out_state->party)) {
        free(events);
        free(timeline);
        csb_v1_fmtowns_game_startup_state_free(out_state);
        return 0;
    }
    if (!csb_v1_fmtowns_game_load_startup_dungeon(receipt, &out_state->dungeon)) {
        free(events);
        free(timeline);
        csb_v1_fmtowns_game_startup_state_free(out_state);
        return 0;
    }
    out_state->game_time = csb_v1_fmtowns_game_read_le32(global_data);
    out_state->party_map_index = (int16_t)csb_v1_fmtowns_game_read_le16(
        global_data + CSB_V1_FMTOWNS_GLOBAL_PARTY_MAP_INDEX_OFFSET);
    out_state->active_group_capacity = receipt->startup_mini_active_group_capacity;
    out_state->active_group_count = receipt->startup_mini_current_active_group_count;
    out_state->timeline_queue.gameTick = out_state->game_time;
    out_state->timeline_queue.eventCount = receipt->startup_mini_event_count;
    out_state->timeline_queue.firstUnusedIndex =
        receipt->startup_mini_first_unused_event_index;
    out_state->timeline_queue.maxEvents = receipt->startup_mini_event_maximum_count;
    for (index = 0u; index < receipt->startup_mini_event_maximum_count; ++index) {
        const unsigned char *event = events + index * CSB_V1_FMTOWNS_EVENT_BYTES;
        out_state->timeline_queue.events[index].map_time =
            csb_v1_fmtowns_game_read_le32(event);
        out_state->timeline_queue.events[index].type = event[4u];
        out_state->timeline_queue.events[index].priority = event[5u];
        out_state->timeline_queue.events[index].b_mapX = event[6u];
        out_state->timeline_queue.events[index].b_mapY = event[7u];
        out_state->timeline_queue.events[index].c_cell = event[8u];
        out_state->timeline_queue.events[index].c_effect = event[9u];
        out_state->timeline_queue.timeline[index] =
            csb_v1_fmtowns_game_read_le16(timeline + index * 2u);
        if (index < receipt->startup_mini_event_count &&
            out_state->timeline_queue.timeline[index] >=
                receipt->startup_mini_event_maximum_count) {
            free(events);
            free(timeline);
            csb_v1_fmtowns_game_startup_state_free(out_state);
            return 0;
        }
    }
    free(events);
    free(timeline);
    if (!csb_v1_fmtowns_game_resolve_active_group_owners(out_state)) {
        csb_v1_fmtowns_game_startup_state_free(out_state);
        return 0;
    }
    out_state->valid = 1;
    return 1;
}

static void csb_v1_fmtowns_game_patch_party_part(
    unsigned char *bytes, const CSB_V1_PartyState *party)
{
    enum {
        /* ReDMCSB DEFS.H CHAMPION: Name[8], Title[20], Direction, Cell.
         * F0433 must patch the same F31 record layout that F0435 reads. */
        champion_bytes = 319u, name_bytes = 8u, title_bytes = 20u,
        direction_offset = 28u, cell_offset = 29u, action_offset = 32u,
        incantation_offset = 34u, facing_offset = 40u,
        poison_event_count_offset = 42u, enable_action_event_offset = 44u,
        hide_damage_event_offset = 46u, attributes_offset = 48u,
        wounds_offset = 50u, health_offset = 52u, food_offset = 66u,
        statistics_offset = 70u, skill_offset = 92u, slots_offset = 212u,
        load_offset = 272u, shield_offset = 274u
    };
    static const int source_stat_to_firestaff[CSB_V1_STAT_COUNT] =
        { 6, 0, 1, 2, 3, 4, 5 };
    int champion_index;

    if (!bytes || !party) return;
    for (champion_index = 0; champion_index < CSB_V1_MAX_CHAMPIONS;
         ++champion_index) {
        unsigned char *dst = bytes + (size_t)champion_index * champion_bytes;
        const CSB_V1_Champion *champion = &party->Champions[champion_index];
        int stat;
        if (champion_index >= party->ChampionCount) continue;
        memset(dst, 0, name_bytes + title_bytes);
        memcpy(dst, champion->Name,
               strlen(champion->Name) < name_bytes ? strlen(champion->Name) : name_bytes);
        memcpy(dst + name_bytes, champion->Title,
               strlen(champion->Title) < title_bytes ? strlen(champion->Title) : title_bytes);
        memcpy(dst + cell_offset, &champion->Cell, 1u);
        memcpy(dst + direction_offset, &champion->Direction, 1u);
        dst[action_offset] = champion->ActionIndex;
        memcpy(dst + incantation_offset, champion->Incantation,
               sizeof(champion->Incantation));
        dst[facing_offset] = champion->CsbWinFacing3 & 3u;
        dst[poison_event_count_offset] = champion->PoisonEventCount;
        csb_v1_fmtowns_game_write_le16(dst + enable_action_event_offset,
                                        (uint16_t)champion->EnableActionEventIndex);
        csb_v1_fmtowns_game_write_le16(dst + hide_damage_event_offset,
                                        (uint16_t)champion->HideDamageReceivedEventIndex);
        csb_v1_fmtowns_game_write_le16(dst + attributes_offset, champion->Attributes);
        csb_v1_fmtowns_game_write_le16(dst + wounds_offset, champion->Wounds);
        csb_v1_fmtowns_game_write_le16(dst + health_offset,
                                        (uint16_t)champion->CurrentHealth);
        csb_v1_fmtowns_game_write_le16(dst + health_offset + 2u,
                                        (uint16_t)champion->MaximumHealth);
        csb_v1_fmtowns_game_write_le16(dst + health_offset + 4u,
                                        (uint16_t)champion->CurrentStamina);
        csb_v1_fmtowns_game_write_le16(dst + health_offset + 6u,
                                        (uint16_t)champion->MaximumStamina);
        csb_v1_fmtowns_game_write_le16(dst + health_offset + 8u,
                                        (uint16_t)champion->CurrentMana);
        csb_v1_fmtowns_game_write_le16(dst + health_offset + 10u,
                                        (uint16_t)champion->MaximumMana);
        csb_v1_fmtowns_game_write_le16(dst + food_offset, (uint16_t)champion->Food);
        csb_v1_fmtowns_game_write_le16(dst + food_offset + 2u,
                                        (uint16_t)champion->Water);
        for (stat = 0; stat < CSB_V1_STAT_COUNT; ++stat) {
            int source = source_stat_to_firestaff[stat];
            dst[statistics_offset + stat * 3] =
                champion->Statistics[source][CSB_V1_STAT_MAX];
            dst[statistics_offset + stat * 3 + 1] =
                champion->Statistics[source][CSB_V1_STAT_CUR];
            dst[statistics_offset + stat * 3 + 2] =
                champion->Statistics[source][CSB_V1_STAT_MIN];
        }
        for (stat = 0; stat < CSB_V1_FULL_SKILL_COUNT; ++stat) {
            unsigned char *skill = dst + skill_offset + stat * 6;
            uint32_t experience = champion->SkillExperience[stat];
            /* Firestaff stores 16 playable skill levels; the native CSBWin
             * record has four additional experience-only entries.  Keep
             * those entries zero-level instead of indexing past Skills. */
            csb_v1_fmtowns_game_write_le16(
                skill, stat < CSB_V1_SKILL_COUNT ? champion->Skills[stat] : 0u);
            csb_v1_fmtowns_game_write_le16(skill + 2u,
                                           (uint16_t)experience);
            csb_v1_fmtowns_game_write_le16(skill + 4u,
                                           (uint16_t)(experience >> 16));
        }
        for (stat = 0; stat < CSB_V1_SLOT_COUNT; ++stat)
            csb_v1_fmtowns_game_write_le16(
                dst + slots_offset + stat * 2u, champion->Slots[stat]);
        csb_v1_fmtowns_game_write_le16(dst + load_offset, champion->Load);
        csb_v1_fmtowns_game_write_le16(dst + shield_offset,
                                       champion->ShieldStrength);
    }
}

static int csb_v1_fmtowns_game_write_user_save_internal(
    CSB_V1_BootProfile *profile,
    const CSB_V1_FmtownsGameHandoffReceipt *game_receipt,
    const CSB_V1_FmtownsStartupPortraitReceipt *portraits,
    const char *save_path, int rotate_backup)
{
    CSB_V1_FmtownsUserSaveReceipt receipt;
    CSB_V1_RuntimeProfile *runtime;
    unsigned char *file_bytes = NULL;
    unsigned char header[CSB_V1_FMTOWNS_SAVE_HEADER_BYTES];
    unsigned char *parts[5] = { NULL, NULL, NULL, NULL, NULL };
    uint32_t part_sizes[5];
    uint32_t offsets[5];
    uint16_t keys[5];
    uint16_t checksums[5];
    uint16_t random_words[REDMCSB_F7062_RANDOM_WORDS];
    uint32_t random_state;
    unsigned char encoded_header[CSB_V1_FMTOWNS_SAVE_HEADER_BYTES];
    uint32_t file_size = 0u;
    uint32_t offset;
    uint32_t index;
    char temp_path[1024];
    char backup_path[1024];
    FILE *file = NULL;
    int ok = 0;

    if (!profile || !game_receipt || !save_path || !save_path[0] ||
        !profile->runtime.party_state_valid ||
        !profile->runtime.csbwin_random_seed_valid ||
        !csb_v1_fmtowns_game_user_save_open(
            profile, game_receipt, save_path, &receipt) ||
        receipt.source_size == 0u || receipt.source_size > 0x7fffffffu) {
        return 0;
    }
    runtime = &profile->runtime;
    /* F0433 writes bounded native arrays.  Reject an incomplete or widened
     * host runtime rather than truncating it into a superficially valid F31
     * slot. */
    if (runtime->party_state.ChampionCount < 0 ||
        runtime->party_state.ChampionCount > CSB_V1_MAX_CHAMPIONS ||
        runtime->party_x < 0 || runtime->party_x > 255 ||
        runtime->party_y < 0 || runtime->party_y > 255 ||
        runtime->current_level < 0 || runtime->current_level > 255 ||
        runtime->party_dir < 0 || runtime->party_dir > 3 ||
        runtime->active_group_state_count > receipt.active_group_capacity ||
        runtime->active_group_state_count >
            CSB_V1_FMTOWNS_USER_SAVE_ACTIVE_GROUP_CAPACITY ||
        runtime->timeline_queue.maxEvents < 0 ||
        runtime->timeline_queue.maxEvents > receipt.event_maximum_count ||
        runtime->timeline_queue.eventCount < 0 ||
        runtime->timeline_queue.eventCount > runtime->timeline_queue.maxEvents ||
        runtime->timeline_queue.firstUnusedIndex < 0 ||
        runtime->timeline_queue.firstUnusedIndex >
            runtime->timeline_queue.maxEvents) {
        return 0;
    }
    file_size = receipt.source_size;
    file_bytes = (unsigned char *)malloc(file_size);
    if (!file_bytes || !(file = fopen(save_path, "rb")) ||
        fread(file_bytes, 1u, file_size, file) != file_size) goto done;
    fclose(file); file = NULL;
    /* F7052 writes C06's encoded portrait array after the five save parts.
     * Its bytes are planar F31 data, not a Firestaff portrait model. */
    if (portraits) {
        if (!portraits->valid || portraits->language != game_receipt->language ||
            portraits->variant_id != game_receipt->variant_id ||
            portraits->source_size != sizeof(portraits->source_bytes) ||
            receipt.portraits_offset > file_size ||
            sizeof(portraits->source_bytes) >
                file_size - receipt.portraits_offset) goto done;
        memcpy(file_bytes + receipt.portraits_offset, portraits->source_bytes,
               sizeof(portraits->source_bytes));
    }
    /* F0433/F0802 writes a save-specific dungeon tail, not necessarily the
     * complete CD DUNGEON.DAT image. Some authentic C03/C04 slots carry a
     * 6540-byte tail while the mounted CD dungeon is 33114 bytes. Never
     * truncate the full source dungeon into that slot or invent a widened
     * replacement. When the admitted slot and live source layout coincide,
     * rebuild the native tail/checksum; otherwise retain the already
     * authenticated tail byte-for-byte and patch only the owned parts. */
    if (!runtime->dungeon_handle) goto done;
    if ((uint32_t)runtime->dungeon_handle->raw_size ==
            receipt.dungeon_tail_size &&
        !csb_v1_fmtowns_game_write_dungeon_tail(
            file_bytes, file_size, &receipt, runtime->dungeon_handle)) {
        goto done;
    }
    memcpy(header, file_bytes, sizeof(header));
    if (!redmcsb_f7061_is_read_save_header_successful_pc34(
            header, sizeof(header), CSB_V1_FMTOWNS_CSB_HEADER_KEY_WORD_INDEX)) {
        goto done;
    }
    for (index = 0u; index < 5u; ++index) {
        keys[index] = csb_v1_fmtowns_game_read_le16(
            header + CSB_V1_FMTOWNS_SAVE_HEADER_KEYS_OFFSET + index * 2u);
        checksums[index] = csb_v1_fmtowns_game_read_le16(
            header + CSB_V1_FMTOWNS_SAVE_HEADER_CHECKSUMS_OFFSET + index * 2u);
    }
    part_sizes[0] = CSB_V1_FMTOWNS_GLOBAL_DATA_BYTES;
    part_sizes[1] = (uint32_t)receipt.active_group_capacity *
        CSB_V1_FMTOWNS_ACTIVE_GROUP_BYTES;
    part_sizes[2] = CSB_V1_FMTOWNS_CHAMPION_PARTY_BYTES;
    part_sizes[3] = (uint32_t)receipt.event_maximum_count *
        CSB_V1_FMTOWNS_EVENT_BYTES;
    part_sizes[4] = (uint32_t)receipt.event_maximum_count *
        CSB_V1_FMTOWNS_TIMELINE_ENTRY_BYTES;
    offsets[0] = CSB_V1_FMTOWNS_SAVE_HEADER_BYTES;
    for (index = 1u; index < 5u; ++index)
        offsets[index] = offsets[index - 1u] + part_sizes[index - 1u];
    offset = offsets[4] + part_sizes[4];
    if (offset > file_size || receipt.portraits_offset < offset) goto done;
    for (index = 0u; index < 5u; ++index) {
        parts[index] = (unsigned char *)malloc(part_sizes[index]);
        if (!parts[index] || offsets[index] > file_size ||
            part_sizes[index] > file_size - offsets[index]) goto done;
        memcpy(parts[index], file_bytes + offsets[index], part_sizes[index]);
        if (!redmcsb_f7057_read_save_part_with_checksum_pc34(
                parts[index], part_sizes[index], keys[index], checksums[index]))
            goto done;
    }
    random_state = runtime->csbwin_random_seed;
    /* F7052 reads the old keyed parts first, then refreshes all sixteen
     * header keys. The first five keys drive F7058's rewritten parts. */
    for (index = 0u; index < 16u; ++index) {
        uint16_t key = csb_v1_fmtowns_game_next_source_random_word(
            &random_state);
        csb_v1_fmtowns_game_write_le16(
            header + CSB_V1_FMTOWNS_SAVE_HEADER_KEYS_OFFSET + index * 2u,
            key);
        if (index < 5u) keys[index] = key;
    }

    csb_v1_fmtowns_game_write_le32(parts[0], runtime->game_time);
    csb_v1_fmtowns_game_write_le16(
        parts[0] + CSB_V1_FMTOWNS_GLOBAL_PARTY_CHAMPION_COUNT_OFFSET,
        (uint16_t)runtime->party_state.ChampionCount);
    csb_v1_fmtowns_game_write_le16(
        parts[0] + CSB_V1_FMTOWNS_GLOBAL_PARTY_MAP_X_OFFSET,
        (uint16_t)runtime->party_x);
    csb_v1_fmtowns_game_write_le16(
        parts[0] + CSB_V1_FMTOWNS_GLOBAL_PARTY_MAP_Y_OFFSET,
        (uint16_t)runtime->party_y);
    csb_v1_fmtowns_game_write_le16(
        parts[0] + CSB_V1_FMTOWNS_GLOBAL_PARTY_DIRECTION_OFFSET,
        (uint16_t)(runtime->party_dir & 3));
    csb_v1_fmtowns_game_write_le16(
        parts[0] + CSB_V1_FMTOWNS_GLOBAL_PARTY_MAP_INDEX_OFFSET,
        (uint16_t)runtime->current_level);
    csb_v1_fmtowns_game_write_le16(
        parts[0] + CSB_V1_FMTOWNS_GLOBAL_EVENT_COUNT_OFFSET,
        (uint16_t)runtime->timeline_queue.eventCount);
    csb_v1_fmtowns_game_write_le16(
        parts[0] + CSB_V1_FMTOWNS_GLOBAL_FIRST_UNUSED_EVENT_INDEX_OFFSET,
        (uint16_t)runtime->timeline_queue.firstUnusedIndex);
    csb_v1_fmtowns_game_write_le16(
        parts[0] + CSB_V1_FMTOWNS_GLOBAL_CURRENT_ACTIVE_GROUP_COUNT_OFFSET,
        runtime->active_group_state_count);
    csb_v1_fmtowns_game_patch_party_part(parts[2], &runtime->party_state);
    /* The runtime table is sparse after a group dies.  F31 indexes ACTIVE_GROUP
     * records by table slot, so preserve holes and do not compact the live
     * states into the first N source records. */
    for (index = 0u; index < part_sizes[1] /
         CSB_V1_FMTOWNS_ACTIVE_GROUP_BYTES; ++index) {
        const CSB_V1_RuntimeActiveGroupState *state =
            &runtime->active_group_state[index];
        unsigned char *dst = parts[1] + index * CSB_V1_FMTOWNS_ACTIVE_GROUP_BYTES;
        if (!state->valid) continue;
        csb_v1_fmtowns_game_write_le16(dst + 2u, state->directions);
        dst[3u] = state->cells;
        dst[4u] = (unsigned char)state->last_move_time;
        dst[5u] = state->delay_fleeing_from_target;
        dst[6u] = (unsigned char)state->target_map_x;
        dst[7u] = (unsigned char)state->target_map_y;
        dst[8u] = (unsigned char)state->prior_map_x;
        dst[9u] = (unsigned char)state->prior_map_y;
        dst[10u] = (unsigned char)state->home_map_x;
        dst[11u] = (unsigned char)state->home_map_y;
        memcpy(dst + 12u, state->aspect, sizeof(state->aspect));
    }
    for (index = 0u; index < (uint32_t)runtime->timeline_queue.maxEvents &&
         index < receipt.event_maximum_count; ++index) {
        const struct DM1_Event_V1 *event =
            &runtime->timeline_queue.events[index];
        unsigned char *dst = parts[3] + index * CSB_V1_FMTOWNS_EVENT_BYTES;
        csb_v1_fmtowns_game_write_le32(dst, event->map_time);
        dst[4u] = event->type; dst[5u] = event->priority;
        dst[6u] = event->b_mapX; dst[7u] = event->b_mapY;
        dst[8u] = event->c_cell; dst[9u] = event->c_effect;
        csb_v1_fmtowns_game_write_le16(
            parts[4] + index * CSB_V1_FMTOWNS_TIMELINE_ENTRY_BYTES,
            runtime->timeline_queue.timeline[index]);
    }
    for (index = 0u; index < 5u; ++index) {
        if (!redmcsb_f7058_write_save_part_with_checksum_pc34(
                parts[index], part_sizes[index], keys[index], &checksums[index]))
            goto done;
        /* F7058 deliberately restores its caller buffer to plaintext after
         * reporting the checksum. F0433 writes the obfuscated bytes, so run
         * the source transform once more before copying the part to disk. */
        (void)redmcsb_f7055_saveutil_get_checksum_and_obfuscate_pc34(
            parts[index], part_sizes[index], keys[index]);
        memcpy(file_bytes + offsets[index], parts[index], part_sizes[index]);
        csb_v1_fmtowns_game_write_le16(
            header + CSB_V1_FMTOWNS_SAVE_HEADER_CHECKSUMS_OFFSET + index * 2u,
            checksums[index]);
    }
    for (index = 0u; index < REDMCSB_F7062_RANDOM_WORDS; ++index)
        random_words[index] = csb_v1_fmtowns_game_next_source_random_word(
            &random_state);
    if (!redmcsb_f7062_prepare_obfuscated_save_header_pc34(
            header, sizeof(header), CSB_V1_FMTOWNS_CSB_HEADER_KEY_WORD_INDEX,
            random_words, REDMCSB_F7062_RANDOM_WORDS,
            encoded_header, sizeof(encoded_header))) goto done;
    memcpy(file_bytes, encoded_header, sizeof(encoded_header));
    if (snprintf(temp_path, sizeof(temp_path), "%s.firestaff-tmp", save_path) < 0 ||
        strlen(temp_path) >= sizeof(temp_path) ||
        !(file = fopen(temp_path, "wb")) ||
        fwrite(file_bytes, 1u, file_size, file) != file_size ||
        fclose(file) != 0) {
        if (file) fclose(file);
        file = NULL;
        remove(temp_path);
        goto done;
    }
    file = NULL;
    if (rotate_backup) {
        if (!csb_v1_fmtowns_game_original_backup_path(
                save_path, backup_path, sizeof(backup_path)) ||
            !csb_v1_fmtowns_game_copy_file(save_path, backup_path)) {
            remove(temp_path);
            goto done;
        }
    }
    if (rename(temp_path, save_path) != 0) {
        remove(temp_path);
        goto done;
    }
    /* Both F7052's part keys and F7062's header filler have now consumed the
     * shared source RNG. Commit the G0349 successor only after the native
     * replacement becomes visible, so a host I/O failure cannot leave a
     * durable save paired with a different runtime receipt. */
    runtime->csbwin_random_seed = random_state;
    ok = 1;
done:
    if (file) fclose(file);
    free(file_bytes);
    for (index = 0u; index < 5u; ++index) free(parts[index]);
    return ok;
}

int csb_v1_fmtowns_game_write_user_save(
    CSB_V1_BootProfile *profile,
    const CSB_V1_FmtownsGameHandoffReceipt *game_receipt,
    const char *save_path)
{
    return csb_v1_fmtowns_game_write_user_save_internal(
        profile, game_receipt, NULL, save_path, 1);
}

static int csb_v1_fmtowns_game_create_user_save_from_startup_internal(
    CSB_V1_BootProfile *profile,
    const CSB_V1_FmtownsGameHandoffReceipt *game_receipt,
    const CSB_V1_FmtownsStartupPortraitReceipt *portraits,
    const char *save_path)
{
    unsigned char *startup_bytes = NULL;
    char backup_path[1024];
    char staging_path[1024];
    FILE *existing = NULL;
    uint32_t random_before;
    int ok = 0;

    /* CEDTINC8.C F7052 opens M746 (CSBGAME.DAT), creates its first native
     * record, and only then replaces the selected medium.  Keep the verified
     * MINI.DAT bootstrap private in a sibling stage: it is never published at
     * the canonical destination. */
    if (!profile || !game_receipt || !save_path || !save_path[0] ||
        !profile->runtime.csbwin_random_seed_valid ||
        !csb_v1_fmtowns_game_receipt_source_matches(game_receipt) ||
        !csb_v1_fmtowns_game_original_backup_path(
            save_path, backup_path, sizeof(backup_path)) ||
        snprintf(staging_path, sizeof(staging_path), "%s.firestaff-bootstrap",
                 save_path) < 0 || strlen(staging_path) >= sizeof(staging_path)) {
        return 0;
    }
    existing = fopen(save_path, "rb");
    if (existing) {
        fclose(existing);
        return 0;
    }
    existing = fopen(staging_path, "rb");
    if (existing) {
        fclose(existing);
        return 0;
    }
    startup_bytes = (unsigned char *)malloc(game_receipt->startup_mini_size);
    if (!startup_bytes ||
        !csb_v1_fmtowns_game_read_startup_span(
            game_receipt, 0u, startup_bytes, game_receipt->startup_mini_size) ||
        !(existing = fopen(staging_path, "wb"))) {
        remove(staging_path);
        free(startup_bytes);
        return 0;
    }
    if (fwrite(startup_bytes, 1u, game_receipt->startup_mini_size, existing) !=
            game_receipt->startup_mini_size) {
        fclose(existing);
        existing = NULL;
        remove(staging_path);
        free(startup_bytes);
        return 0;
    }
    if (fclose(existing) != 0) {
        existing = NULL;
        remove(staging_path);
        free(startup_bytes);
        return 0;
    }
    existing = NULL;
    free(startup_bytes);
    random_before = profile->runtime.csbwin_random_seed;
    if (!csb_v1_fmtowns_game_write_user_save_internal(
            profile, game_receipt, portraits, staging_path, 0)) goto done;
    /* The target was absent on entry and is checked again before publication.
     * Do not replace a slot another process has created while F7052 ran. */
    existing = fopen(save_path, "rb");
    if (existing) {
        fclose(existing);
        existing = NULL;
        goto done;
    }
    if (rename(staging_path, save_path) != 0) goto done;
    ok = 1;
done:
    if (existing) fclose(existing);
    if (!ok) {
        profile->runtime.csbwin_random_seed = random_before;
        remove(staging_path);
    }
    return ok;
}

int csb_v1_fmtowns_game_create_user_save_from_startup(
    CSB_V1_BootProfile *profile,
    const CSB_V1_FmtownsGameHandoffReceipt *game_receipt,
    const char *save_path)
{
    return csb_v1_fmtowns_game_create_user_save_from_startup_internal(
        profile, game_receipt, NULL, save_path);
}

int csb_v1_fmtowns_game_create_utility_user_save_from_startup(
    CSB_V1_BootProfile *profile,
    const CSB_V1_FmtownsGameHandoffReceipt *game_receipt,
    const CSB_V1_FmtownsStartupPortraitReceipt *portraits,
    const char *save_path)
{
    if (!portraits || !portraits->valid ||
        portraits->source_size != sizeof(portraits->source_bytes)) return 0;
    return csb_v1_fmtowns_game_create_user_save_from_startup_internal(
        profile, game_receipt, portraits, save_path);
}

int csb_v1_fmtowns_game_write_utility_user_save(
    CSB_V1_BootProfile *profile,
    const CSB_V1_FmtownsGameHandoffReceipt *game_receipt,
    const CSB_V1_FmtownsStartupPortraitReceipt *portraits,
    const char *save_path)
{
    if (!portraits || !portraits->valid ||
        portraits->source_size != sizeof(portraits->source_bytes)) return 0;
    return csb_v1_fmtowns_game_write_user_save_internal(
        profile, game_receipt, portraits, save_path, 1);
}

int csb_v1_fmtowns_game_apply_startup_state(
    CSB_V1_FmtownsStartupState *state, CSB_V1_RuntimeProfile *runtime)
{
    CSB_V1_DungeonData *new_dungeon;
    CSB_V1_DungeonData *old_dungeon;
    uint16_t slot;

    if (!state || !runtime || !state->valid || !state->dungeon.raw_data ||
        state->party_map_index < 0 ||
        state->party_map_index >= state->dungeon.level_count ||
        state->active_group_resolved_count != state->active_group_count ||
        state->timeline_queue.eventCount < 0 ||
        state->timeline_queue.maxEvents <= 0 ||
        state->timeline_queue.maxEvents > DM1_EVENT_MAX_COUNT ||
        !(new_dungeon = (CSB_V1_DungeonData *)malloc(sizeof(*new_dungeon))) ) {
        return 0;
    }
    if (csb_v1_runtime_set_party_state(runtime, &state->party) != 0) {
        free(new_dungeon);
        return 0;
    }
    /* Every saved active record must have its uniquely resolved C04 owner
     * before this point. GROUP.C F0183/F0184 uses the saved table index to
     * select that group; it is never legal to replace a missing owner. */
    for (slot = 0u; slot < state->active_group_capacity; ++slot) {
        if ((int16_t)csb_v1_fmtowns_game_read_le16(
                state->active_groups[slot]) >= 0 &&
            !state->active_group_owners[slot].valid) {
            free(new_dungeon);
            return 0;
        }
    }
    *new_dungeon = state->dungeon;
    memset(&state->dungeon, 0, sizeof(state->dungeon));
    old_dungeon = runtime->dungeon_handle;
    runtime->dungeon_handle = new_dungeon;
    runtime->level_count = new_dungeon->level_count;
    runtime->current_level = state->party_map_index;
    runtime->party_x = state->party.PartyMapX;
    runtime->party_y = state->party.PartyMapY;
    runtime->party_dir = state->party.PartyDirection & 3;
    runtime->game_time = state->game_time;
    runtime->timeline_queue = state->timeline_queue;
    runtime->timeline_queue.gameTick = state->game_time;
    memset(runtime->active_group_state, 0, sizeof(runtime->active_group_state));
    runtime->active_group_state_count = 0u;
    for (slot = 0u; slot < state->active_group_capacity; ++slot) {
        const uint8_t *raw = state->active_groups[slot];
        const int16_t group_index = (int16_t)csb_v1_fmtowns_game_read_le16(raw);
        CSB_V1_RuntimeActiveGroupState *target;
        uint32_t last_move_time;

        if (group_index < 0) continue;
        target = &runtime->active_group_state[slot];
        target->valid = 1;
        target->group_thing = state->active_group_owners[slot].group_thing;
        target->map_index = state->active_group_owners[slot].map_index;
        target->map_x = state->active_group_owners[slot].map_x;
        target->map_y = state->active_group_owners[slot].map_y;
        target->directions = raw[2u];
        target->cells = raw[3u];
        /* ACTIVE_GROUP stores only G0313's low byte. Reconstruct the most
         * recent source tick, never a future host time. */
        last_move_time = (state->game_time & ~0xffu) | raw[4u];
        if (last_move_time > state->game_time) last_move_time -= 0x100u;
        target->last_move_time = last_move_time;
        target->delay_fleeing_from_target = raw[5u];
        target->target_map_x = raw[6u];
        target->target_map_y = raw[7u];
        target->prior_map_x = raw[8u];
        target->prior_map_y = raw[9u];
        target->home_map_x = raw[10u];
        target->home_map_y = raw[11u];
        memcpy(target->aspect, raw + 12u, sizeof(target->aspect));
        ++runtime->active_group_state_count;
    }
    csb_v1_dungeon_set_current(new_dungeon);
    csb_v1_dungeon_set_current_level(runtime->current_level);
    if (old_dungeon) {
        csb_v1_dungeon_free(old_dungeon);
        free(old_dungeon);
    }
    return 1;
}

static int csb_v1_fmtowns_game_startup_mini_save_parts_open(
    CSB_V1_FmtownsGameHandoffReceipt *receipt,
    const unsigned char *header)
{
    uint16_t keys[5];
    uint16_t checksums[5];
    uint32_t part_sizes[5];
    unsigned char global_data[CSB_V1_FMTOWNS_GLOBAL_DATA_BYTES];
    unsigned char *part_data = NULL;
    uint32_t offset = CSB_V1_FMTOWNS_SAVE_HEADER_BYTES;
    uint32_t index;
    uint16_t event_maximum_count;
    uint16_t active_group_capacity;

    if (!header || !receipt ||
        !csb_v1_fmtowns_game_read_startup_span(receipt, offset, global_data,
                                       sizeof(global_data))) return 0;
    for (index = 0u; index < 5u; ++index) {
        keys[index] = csb_v1_fmtowns_game_read_le16(
            header + CSB_V1_FMTOWNS_SAVE_HEADER_KEYS_OFFSET + index * 2u);
        checksums[index] = csb_v1_fmtowns_game_read_le16(
            header + CSB_V1_FMTOWNS_SAVE_HEADER_CHECKSUMS_OFFSET + index * 2u);
    }
    if (!redmcsb_f7057_read_save_part_with_checksum_pc34(
            global_data, sizeof(global_data), keys[0], checksums[0])) return 0;
    event_maximum_count = csb_v1_fmtowns_game_read_le16(
        global_data + CSB_V1_FMTOWNS_GLOBAL_EVENT_MAXIMUM_COUNT_OFFSET);
    active_group_capacity = csb_v1_fmtowns_game_read_le16(
        global_data + CSB_V1_FMTOWNS_GLOBAL_ACTIVE_GROUP_CAPACITY_OFFSET);
    if (event_maximum_count == 0u || active_group_capacity == 0u) return 0;
    part_sizes[0] = CSB_V1_FMTOWNS_GLOBAL_DATA_BYTES;
    part_sizes[1] = active_group_capacity * CSB_V1_FMTOWNS_ACTIVE_GROUP_BYTES;
    part_sizes[2] = CSB_V1_FMTOWNS_CHAMPION_PARTY_BYTES;
    part_sizes[3] = event_maximum_count * CSB_V1_FMTOWNS_EVENT_BYTES;
    part_sizes[4] = event_maximum_count * CSB_V1_FMTOWNS_TIMELINE_ENTRY_BYTES;
    offset += part_sizes[0];
    for (index = 1u; index < 5u; ++index) {
        if (offset > receipt->startup_mini_size ||
            part_sizes[index] > receipt->startup_mini_size - offset ||
            (part_sizes[index] & 1u) != 0u ||
            !(part_data = (unsigned char *)malloc(part_sizes[index])) ||
            !csb_v1_fmtowns_game_read_startup_span(receipt, offset, part_data,
                                            part_sizes[index]) ||
            !redmcsb_f7057_read_save_part_with_checksum_pc34(
                part_data, part_sizes[index], keys[index], checksums[index])) {
            free(part_data);
            return 0;
        }
        free(part_data);
        part_data = NULL;
        offset += part_sizes[index];
    }
    /* ReDMCSB DEFS.H GLOBAL_DATA lines 538-568 fixes these F31 offsets.
     * Retain the source heap metadata even though Resume stays closed until
     * its event records and the eight live ACTIVE_GROUP owners transfer in
     * the same transaction as the dungeon tail. */
    receipt->startup_mini_party_champion_count = csb_v1_fmtowns_game_read_le16(
        global_data + CSB_V1_FMTOWNS_GLOBAL_PARTY_CHAMPION_COUNT_OFFSET);
    receipt->startup_mini_event_count = csb_v1_fmtowns_game_read_le16(
        global_data + CSB_V1_FMTOWNS_GLOBAL_EVENT_COUNT_OFFSET);
    receipt->startup_mini_first_unused_event_index = csb_v1_fmtowns_game_read_le16(
        global_data + CSB_V1_FMTOWNS_GLOBAL_FIRST_UNUSED_EVENT_INDEX_OFFSET);
    receipt->startup_mini_current_active_group_count = csb_v1_fmtowns_game_read_le16(
        global_data + CSB_V1_FMTOWNS_GLOBAL_CURRENT_ACTIVE_GROUP_COUNT_OFFSET);
    receipt->startup_mini_game_time = csb_v1_fmtowns_game_read_le32(global_data);
    receipt->startup_mini_party_map_x = (int16_t)csb_v1_fmtowns_game_read_le16(
        global_data + CSB_V1_FMTOWNS_GLOBAL_PARTY_MAP_X_OFFSET);
    receipt->startup_mini_party_map_y = (int16_t)csb_v1_fmtowns_game_read_le16(
        global_data + CSB_V1_FMTOWNS_GLOBAL_PARTY_MAP_Y_OFFSET);
    receipt->startup_mini_party_direction = (int16_t)csb_v1_fmtowns_game_read_le16(
        global_data + CSB_V1_FMTOWNS_GLOBAL_PARTY_DIRECTION_OFFSET);
    receipt->startup_mini_party_map_index = (int16_t)csb_v1_fmtowns_game_read_le16(
        global_data + CSB_V1_FMTOWNS_GLOBAL_PARTY_MAP_INDEX_OFFSET);
    receipt->startup_mini_event_maximum_count = event_maximum_count;
    receipt->startup_mini_active_group_capacity = active_group_capacity;
    receipt->startup_mini_verified_save_body_offset = offset;
    receipt->startup_mini_save_parts_verified = 1;
    return csb_v1_fmtowns_game_startup_mini_dungeon_tail_open(receipt);
}

static int csb_v1_fmtowns_game_startup_mini_header_open(
    uint16_t expected_platform, CSB_V1_FmtownsGameHandoffReceipt *receipt)
{
    unsigned char header[CSB_V1_FMTOWNS_SAVE_HEADER_BYTES];
    uint16_t key;

    if (!receipt ||
        !csb_v1_fmtowns_game_read_startup_span(receipt, 0u, header,
                                               sizeof(header))) {
        return 0;
    }
    key = csb_v1_fmtowns_game_read_le16(
        header + CSB_V1_FMTOWNS_CSB_HEADER_KEY_WORD_INDEX * 2u);
    /* ReDMCSB CEDTINCD.C F7051 lines 211-255 selects the CSB word-29
     * header route for F31E/F31J. DEFS.H defines C5 as the family including
     * FM Towns CSB. F7061 validates and deobfuscates the second 256 bytes. */
    if (!redmcsb_f7061_is_read_save_header_successful_pc34(
            header, sizeof(header),
            CSB_V1_FMTOWNS_CSB_HEADER_KEY_WORD_INDEX) ||
        header[CSB_V1_FMTOWNS_SAVE_HEADER_USELESS_OFFSET] != 1u ||
        header[CSB_V1_FMTOWNS_SAVE_HEADER_FORMAT_OFFSET] !=
            CSB_V1_FMTOWNS_SAVE_HEADER_FORMAT_C5 ||
        csb_v1_fmtowns_game_read_le16(
            header + CSB_V1_FMTOWNS_SAVE_HEADER_PLATFORM_OFFSET) !=
            expected_platform ||
        csb_v1_fmtowns_game_read_le16(
            header + CSB_V1_FMTOWNS_SAVE_HEADER_DUNGEON_ID_OFFSET) !=
            CSB_V1_FMTOWNS_SAVE_HEADER_DUNGEON_CSB_GAME) {
        return 0;
    }
    receipt->startup_mini_header_key = key;
    receipt->startup_mini_header_format_id =
        header[CSB_V1_FMTOWNS_SAVE_HEADER_FORMAT_OFFSET];
    receipt->startup_mini_header_platform = csb_v1_fmtowns_game_read_le16(
        header + CSB_V1_FMTOWNS_SAVE_HEADER_PLATFORM_OFFSET);
    receipt->startup_mini_header_dungeon_id = csb_v1_fmtowns_game_read_le16(
        header + CSB_V1_FMTOWNS_SAVE_HEADER_DUNGEON_ID_OFFSET);
    receipt->startup_mini_header_verified = 1;
    return csb_v1_fmtowns_game_startup_mini_save_parts_open(receipt, header);
}

int csb_v1_fmtowns_game_user_save_open(
    const CSB_V1_BootProfile *profile,
    const CSB_V1_FmtownsGameHandoffReceipt *game_receipt,
    const char *save_path,
    CSB_V1_FmtownsUserSaveReceipt *out_receipt)
{
    unsigned char header[CSB_V1_FMTOWNS_SAVE_HEADER_BYTES];
    unsigned char global_data[CSB_V1_FMTOWNS_GLOBAL_DATA_BYTES];
    unsigned char *part = NULL;
    uint16_t keys[5];
    uint16_t checksums[5];
    uint32_t part_sizes[5];
    uint32_t offset = CSB_V1_FMTOWNS_SAVE_HEADER_BYTES;
    uint32_t index;
    uint16_t expected_platform;
    int header_ok;
    uint32_t actual_size;
    CSB_V1_FmtownsGameHandoffReceipt tail_receipt;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!profile || !game_receipt || !game_receipt->valid || !save_path ||
        !save_path[0] || !profile->dungeon_verified ||
        !profile->dungeon_path[0]) return 0;
    expected_platform = game_receipt->language == CSB_FMTOWNS_SWITCH_ENGLISH
        ? 7u : game_receipt->language == CSB_FMTOWNS_SWITCH_JAPANESE ? 8u : 0u;
    if (expected_platform == 0u) return 0;
    out_receipt->source_fnv1a = csb_v1_fmtowns_game_file_fnv1a(save_path, &actual_size);
    if (!out_receipt->source_fnv1a || actual_size < CSB_V1_FMTOWNS_SAVE_HEADER_BYTES ||
        !csb_v1_fmtowns_game_read_span(save_path, 0u, header, sizeof(header))) return 0;
    header_ok = redmcsb_f7061_is_read_save_header_successful_pc34(
            header, sizeof(header), CSB_V1_FMTOWNS_CSB_HEADER_KEY_WORD_INDEX) ||
        0;
    if (!header_ok ||
        header[CSB_V1_FMTOWNS_SAVE_HEADER_USELESS_OFFSET] != 1u ||
        header[CSB_V1_FMTOWNS_SAVE_HEADER_FORMAT_OFFSET] !=
            CSB_V1_FMTOWNS_SAVE_HEADER_FORMAT_C5 ||
        csb_v1_fmtowns_game_read_le16(
            header + CSB_V1_FMTOWNS_SAVE_HEADER_PLATFORM_OFFSET) !=
            expected_platform ||
        (csb_v1_fmtowns_game_read_le16(
             header + CSB_V1_FMTOWNS_SAVE_HEADER_DUNGEON_ID_OFFSET) !=
             CSB_V1_FMTOWNS_SAVE_HEADER_DUNGEON_CSB_PRISON &&
        csb_v1_fmtowns_game_read_le16(
             header + CSB_V1_FMTOWNS_SAVE_HEADER_DUNGEON_ID_OFFSET) !=
             CSB_V1_FMTOWNS_SAVE_HEADER_DUNGEON_CSB_GAME) ||
        !csb_v1_fmtowns_game_read_span(save_path, offset, global_data,
                                        sizeof(global_data))) return 0;
    for (index = 0u; index < 5u; ++index) {
        keys[index] = csb_v1_fmtowns_game_read_le16(
            header + CSB_V1_FMTOWNS_SAVE_HEADER_KEYS_OFFSET + index * 2u);
        checksums[index] = csb_v1_fmtowns_game_read_le16(
            header + CSB_V1_FMTOWNS_SAVE_HEADER_CHECKSUMS_OFFSET + index * 2u);
    }
    if (!redmcsb_f7057_read_save_part_with_checksum_pc34(
        global_data, sizeof(global_data), keys[0], checksums[0])) return 0;
    part_sizes[0] = sizeof(global_data);
    part_sizes[1] = (uint32_t)csb_v1_fmtowns_game_read_le16(
        global_data + CSB_V1_FMTOWNS_GLOBAL_ACTIVE_GROUP_CAPACITY_OFFSET) *
        CSB_V1_FMTOWNS_ACTIVE_GROUP_BYTES;
    part_sizes[2] = CSB_V1_FMTOWNS_CHAMPION_PARTY_BYTES;
    part_sizes[3] = (uint32_t)csb_v1_fmtowns_game_read_le16(
        global_data + CSB_V1_FMTOWNS_GLOBAL_EVENT_MAXIMUM_COUNT_OFFSET) *
        CSB_V1_FMTOWNS_EVENT_BYTES;
    part_sizes[4] = (uint32_t)csb_v1_fmtowns_game_read_le16(
        global_data + CSB_V1_FMTOWNS_GLOBAL_EVENT_MAXIMUM_COUNT_OFFSET) *
        CSB_V1_FMTOWNS_TIMELINE_ENTRY_BYTES;
    if (part_sizes[1] == 0u || part_sizes[3] == 0u ||
        part_sizes[1] > CSB_V1_FMTOWNS_USER_SAVE_ACTIVE_GROUP_CAPACITY *
                            CSB_V1_FMTOWNS_ACTIVE_GROUP_BYTES ||
        part_sizes[3] / CSB_V1_FMTOWNS_EVENT_BYTES > DM1_EVENT_MAX_COUNT) return 0;
    offset += part_sizes[0];
    for (index = 1u; index < 5u; ++index) {
        if (offset > actual_size || part_sizes[index] > actual_size - offset ||
            !(part = (unsigned char *)malloc(part_sizes[index])) ||
            !csb_v1_fmtowns_game_read_span(save_path, offset, part,
                                            part_sizes[index]) ||
            !redmcsb_f7057_read_save_part_with_checksum_pc34(
                part, part_sizes[index], keys[index], checksums[index])) {
            free(part);
            return 0;
        }
        free(part);
        part = NULL;
        offset += part_sizes[index];
    }
    if (offset > actual_size ||
        CSB_V1_FMTOWNS_EXTERNAL_PORTRAIT_BYTES *
            CSB_V1_FMTOWNS_EXTERNAL_PORTRAIT_COUNT > actual_size - offset) return 0;
    /* F0435/F7063 consumes the native dungeon tail after the four portrait
     * rasters. A valid header and five checksummed parts are not enough to
     * admit a slot: the tail must be complete and the saved pose must belong
     * to one of its original maps. C12/C13 layouts are not interchangeable. */
    memset(&tail_receipt, 0, sizeof(tail_receipt));
    tail_receipt.valid = 1;
    tail_receipt.startup_mini_verified = 1;
    tail_receipt.startup_mini_size = actual_size;
    tail_receipt.startup_mini_fnv1a = out_receipt->source_fnv1a;
    tail_receipt.startup_mini_verified_save_body_offset = offset;
    tail_receipt.startup_mini_party_map_x = (int16_t)csb_v1_fmtowns_game_read_le16(
        global_data + CSB_V1_FMTOWNS_GLOBAL_PARTY_MAP_X_OFFSET);
    tail_receipt.startup_mini_party_map_y = (int16_t)csb_v1_fmtowns_game_read_le16(
        global_data + CSB_V1_FMTOWNS_GLOBAL_PARTY_MAP_Y_OFFSET);
    tail_receipt.startup_mini_party_direction = (int16_t)csb_v1_fmtowns_game_read_le16(
        global_data + CSB_V1_FMTOWNS_GLOBAL_PARTY_DIRECTION_OFFSET);
    tail_receipt.startup_mini_party_map_index = (int16_t)csb_v1_fmtowns_game_read_le16(
        global_data + CSB_V1_FMTOWNS_GLOBAL_PARTY_MAP_INDEX_OFFSET);
    snprintf(tail_receipt.startup_mini_path,
             sizeof(tail_receipt.startup_mini_path), "%s", save_path);
    if (!csb_v1_fmtowns_game_startup_mini_dungeon_tail_open(&tail_receipt))
        return 0;
    out_receipt->valid = 1;
    out_receipt->language = game_receipt->language;
    out_receipt->variant_id = game_receipt->variant_id;
    out_receipt->source_size = actual_size;
    out_receipt->header_key = csb_v1_fmtowns_game_read_le16(
        header + CSB_V1_FMTOWNS_CSB_HEADER_KEY_WORD_INDEX * 2u);
    out_receipt->platform = expected_platform;
    out_receipt->dungeon_id = csb_v1_fmtowns_game_read_le16(
        header + CSB_V1_FMTOWNS_SAVE_HEADER_DUNGEON_ID_OFFSET);
    out_receipt->party_champion_count = csb_v1_fmtowns_game_read_le16(
        global_data + CSB_V1_FMTOWNS_GLOBAL_PARTY_CHAMPION_COUNT_OFFSET);
    out_receipt->event_count = csb_v1_fmtowns_game_read_le16(
        global_data + CSB_V1_FMTOWNS_GLOBAL_EVENT_COUNT_OFFSET);
    out_receipt->first_unused_event_index = csb_v1_fmtowns_game_read_le16(
        global_data + CSB_V1_FMTOWNS_GLOBAL_FIRST_UNUSED_EVENT_INDEX_OFFSET);
    out_receipt->current_active_group_count = csb_v1_fmtowns_game_read_le16(
        global_data + CSB_V1_FMTOWNS_GLOBAL_CURRENT_ACTIVE_GROUP_COUNT_OFFSET);
    out_receipt->event_maximum_count = (uint16_t)(part_sizes[3] /
        CSB_V1_FMTOWNS_EVENT_BYTES);
    out_receipt->active_group_capacity = (uint16_t)(part_sizes[1] /
        CSB_V1_FMTOWNS_ACTIVE_GROUP_BYTES);
    out_receipt->game_time = csb_v1_fmtowns_game_read_le32(global_data);
    out_receipt->party_map_x = (int16_t)csb_v1_fmtowns_game_read_le16(
        global_data + CSB_V1_FMTOWNS_GLOBAL_PARTY_MAP_X_OFFSET);
    out_receipt->party_map_y = (int16_t)csb_v1_fmtowns_game_read_le16(
        global_data + CSB_V1_FMTOWNS_GLOBAL_PARTY_MAP_Y_OFFSET);
    out_receipt->party_direction = (int16_t)csb_v1_fmtowns_game_read_le16(
        global_data + CSB_V1_FMTOWNS_GLOBAL_PARTY_DIRECTION_OFFSET);
    out_receipt->party_map_index = (int16_t)csb_v1_fmtowns_game_read_le16(
        global_data + CSB_V1_FMTOWNS_GLOBAL_PARTY_MAP_INDEX_OFFSET);
    out_receipt->portraits_offset = offset;
    out_receipt->dungeon_tail_offset =
        tail_receipt.startup_mini_dungeon_tail_offset;
    out_receipt->dungeon_tail_size =
        tail_receipt.startup_mini_dungeon_tail_size;
    out_receipt->dungeon_tail_checksum =
        tail_receipt.startup_mini_dungeon_tail_checksum;
    snprintf(out_receipt->source_path, sizeof(out_receipt->source_path), "%s", save_path);
    snprintf(out_receipt->dungeon_path, sizeof(out_receipt->dungeon_path), "%s",
             profile->dungeon_path);
    out_receipt->source_evidence =
        "ReDMCSB LOADSAVE.C F0435 lines 2700-2890; CEDTINC6.C F7057/F7061; "
        "F0434 continues on the same save-file handle with the F7063 dungeon tail.";
    return 1;
}

static int csb_v1_fmtowns_game_original_backup_path(const char *path,
                                                     char *out,
                                                     size_t out_size)
{
    const char *name;
    static const char original_name[] = "CSBGAME.DAT";
    size_t index;

    if (!path || !out || out_size == 0u) return 0;
    name = strrchr(path, '/');
    name = name ? name + 1 : path;
    for (index = 0u; name[index] != '\0' && original_name[index] != '\0'; ++index) {
        char actual = name[index];
        if (actual >= 'a' && actual <= 'z') actual = (char)(actual - ('a' - 'A'));
        if (actual != original_name[index]) return 0;
    }
    if (name[index] != '\0' || original_name[index] != '\0') return 0;
    {
        size_t prefix_length = strlen(path) - 4u;
        int written = snprintf(out, out_size, "%.*s.BAK", (int)prefix_length, path);
        return written >= 0 && (size_t)written < out_size;
    }
}

int csb_v1_fmtowns_game_user_save_open_or_restore_backup(
    const CSB_V1_BootProfile *profile,
    const CSB_V1_FmtownsGameHandoffReceipt *game_receipt,
    const char *save_path,
    CSB_V1_FmtownsUserSaveReceipt *out_receipt)
{
    char backup_path[1024];
    CSB_V1_FmtownsUserSaveReceipt backup_receipt;

    if (csb_v1_fmtowns_game_user_save_open(profile, game_receipt, save_path,
                                            out_receipt)) return 1;
    if (!out_receipt ||
        !csb_v1_fmtowns_game_original_backup_path(save_path, backup_path,
                                                   sizeof(backup_path))) return 0;
    memset(&backup_receipt, 0, sizeof(backup_receipt));
    if (!csb_v1_fmtowns_game_user_save_open(profile, game_receipt, backup_path,
                                            &backup_receipt)) return 0;
    /* ReDMCSB LOADSAVE.C F0435:2906-2907: do not bind a valid backup as an
     * alternate runtime source.  It first becomes the selected canonical
     * slot, and only then may F0435 mutate the live game. */
#if defined(_WIN32)
    /* MoveFileEx is the Win32 replacement form of POSIX rename().  Do not
     * remove the selected path first: a failed replacement must leave the
     * validated .BAK intact and cannot turn a load failure into data loss. */
    if (!MoveFileExA(backup_path, save_path, MOVEFILE_REPLACE_EXISTING)) return 0;
#else
    /* POSIX rename atomically replaces a regular destination.  This has the
     * source-visible end state of F0435:2906-2907 while keeping either the
     * old selected slot or the validated backup if the filesystem refuses
     * the transition. */
    if (rename(backup_path, save_path) != 0) return 0;
#endif
    if (!csb_v1_fmtowns_game_user_save_open(profile, game_receipt, save_path,
                                            out_receipt)) return 0;
    out_receipt->recovered_from_backup = 1;
    return 1;
}

int csb_v1_fmtowns_game_load_user_save_state(
    const CSB_V1_FmtownsUserSaveReceipt *receipt,
    CSB_V1_FmtownsStartupState *out_state)
{
    CSB_V1_FmtownsGameHandoffReceipt compat;

    if (!receipt || !out_state || !receipt->valid || !receipt->source_path[0] ||
        receipt->source_size == 0u || receipt->portraits_offset == 0u ||
        receipt->dungeon_tail_size == 0u ||
        receipt->dungeon_tail_offset != receipt->portraits_offset +
            CSB_V1_FMTOWNS_EXTERNAL_PORTRAIT_BYTES *
                CSB_V1_FMTOWNS_EXTERNAL_PORTRAIT_COUNT) return 0;
    memset(&compat, 0, sizeof(compat));
    compat.valid = 1;
    compat.language = receipt->language;
    compat.variant_id = receipt->variant_id;
    compat.startup_mini_verified = 1;
    compat.startup_mini_size = receipt->source_size;
    compat.startup_mini_fnv1a = receipt->source_fnv1a;
    snprintf(compat.startup_mini_path, sizeof(compat.startup_mini_path), "%s",
             receipt->source_path);
    compat.startup_mini_header_verified = 1;
    compat.startup_mini_header_key = receipt->header_key;
    compat.startup_mini_header_format_id = CSB_V1_FMTOWNS_SAVE_HEADER_FORMAT_C5;
    compat.startup_mini_header_platform = receipt->platform;
    compat.startup_mini_header_dungeon_id = receipt->dungeon_id;
    compat.startup_mini_save_parts_verified = 1;
    compat.startup_mini_party_champion_count = receipt->party_champion_count;
    compat.startup_mini_event_count = receipt->event_count;
    compat.startup_mini_first_unused_event_index = receipt->first_unused_event_index;
    compat.startup_mini_current_active_group_count = receipt->current_active_group_count;
    compat.startup_mini_event_maximum_count = receipt->event_maximum_count;
    compat.startup_mini_active_group_capacity = receipt->active_group_capacity;
    compat.startup_mini_game_time = receipt->game_time;
    compat.startup_mini_party_map_x = receipt->party_map_x;
    compat.startup_mini_party_map_y = receipt->party_map_y;
    compat.startup_mini_party_direction = receipt->party_direction;
    compat.startup_mini_party_map_index = receipt->party_map_index;
    compat.startup_mini_verified_save_body_offset = receipt->portraits_offset;
    compat.startup_mini_dungeon_tail_verified = 1;
    compat.startup_mini_dungeon_tail_offset = receipt->dungeon_tail_offset;
    compat.startup_mini_dungeon_tail_size = receipt->dungeon_tail_size;
    compat.startup_mini_dungeon_tail_checksum = receipt->dungeon_tail_checksum;
    /* F0435 keeps the accepted save file open while F0434 consumes its
     * appended dungeon. Reuse the same checksum-checked transfer, not a
     * MINI.DAT or host snapshot substitution. */
    return csb_v1_fmtowns_game_load_startup_state(&compat, out_state);
}

int csb_v1_fmtowns_game_load_user_save_portraits(
    const CSB_V1_FmtownsUserSaveReceipt *receipt,
    CSB_V1_FmtownsStartupPortraitReceipt *out_receipt)
{
    uint32_t actual_size;
    uint32_t actual_fnv1a;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!receipt || !receipt->valid || !receipt->source_path[0] ||
        receipt->source_size == 0u || receipt->source_fnv1a == 0u ||
        receipt->portraits_offset > receipt->source_size ||
        sizeof(out_receipt->source_bytes) >
            receipt->source_size - receipt->portraits_offset) return 0;
    /* F0435 and F2124 consume one selected file handle. Recompute the
     * receipt identity here so a swapped path cannot combine validated party
     * parts with portraits from another user file. */
    actual_fnv1a = csb_v1_fmtowns_game_file_fnv1a(receipt->source_path,
                                                   &actual_size);
    if (actual_fnv1a != receipt->source_fnv1a ||
        actual_size != receipt->source_size ||
        !csb_v1_fmtowns_game_read_span(
            receipt->source_path, receipt->portraits_offset,
            (unsigned char *)out_receipt->source_bytes,
            sizeof(out_receipt->source_bytes))) return 0;
    out_receipt->valid = 1;
    out_receipt->language = receipt->language;
    out_receipt->variant_id = receipt->variant_id;
    out_receipt->source_file_offset = receipt->portraits_offset;
    out_receipt->source_size = sizeof(out_receipt->source_bytes);
    out_receipt->source_fnv1a = csb_v1_fmtowns_game_bytes_fnv1a(
        (const unsigned char *)out_receipt->source_bytes,
        sizeof(out_receipt->source_bytes));
    out_receipt->source_evidence =
        "ReDMCSB CEDTINCD.C F7051; CEDT019.C F2124 lines 85-109";
    return 1;
}

static int csb_v1_fmtowns_utility_icon_palette_open(
    const CSB_V1_FmtownsUtilityHandoffReceipt *source_receipt,
    uint32_t file_offset,
    CSB_V1_FmtownsUtilityHandoffReceipt *receipt)
{
    uint8_t source[CSB_V1_FMTOWNS_UTILITY_ICON_PALETTE_RECORD_BYTES];
    uint32_t index;

    if (!source_receipt || !receipt ||
        !csb_v1_fmtowns_utility_read_span(source_receipt, file_offset, source,
                                          sizeof(source))) return 0;
    for (index = 0u;
         index < CSB_V1_FMTOWNS_UTILITY_ICON_PALETTE_COLOR_COUNT;
         ++index) {
        const uint8_t *entry = source + index * 4u;
        if (entry[0] != index || entry[1] > 0x3fu || entry[2] > 0x3fu ||
            entry[3] > 0x3fu) return 0;
        receipt->icon_palette_rgb6[index][0] = entry[1];
        receipt->icon_palette_rgb6[index][1] = entry[2];
        receipt->icon_palette_rgb6[index][2] = entry[3];
    }
    if (source[64] != 0xffu || source[65] != 0u || source[66] != 0u ||
        source[67] != 0u) return 0;
    receipt->icon_palette_file_offset = file_offset;
    receipt->icon_palette_verified = 1;
    return 1;
}

static int csb_v1_fmtowns_utility_static_art_open(
    const CSB_V1_FmtownsUtilityHandoffReceipt *source_receipt,
    uint32_t mirror_offset, uint32_t arrows_offset,
    CSB_V1_FmtownsUtilityHandoffReceipt *receipt)
{
    if (!source_receipt || !receipt ||
        !csb_v1_fmtowns_utility_read_span(
            source_receipt, mirror_offset, receipt->mirror_bitmap,
            sizeof(receipt->mirror_bitmap)) ||
        csb_v1_fmtowns_game_bytes_fnv1a(receipt->mirror_bitmap,
                                        sizeof(receipt->mirror_bitmap)) !=
            CSB_V1_FMTOWNS_UTILITY_MIRROR_BITMAP_FNV1A ||
        !csb_v1_fmtowns_utility_read_span(
            source_receipt, arrows_offset, receipt->file_picker_arrows,
            sizeof(receipt->file_picker_arrows)) ||
        csb_v1_fmtowns_game_bytes_fnv1a(receipt->file_picker_arrows,
                                        sizeof(receipt->file_picker_arrows)) !=
            CSB_V1_FMTOWNS_UTILITY_FILE_PICKER_ARROWS_FNV1A) return 0;
    receipt->mirror_bitmap_file_offset = mirror_offset;
    receipt->mirror_bitmap_fnv1a = CSB_V1_FMTOWNS_UTILITY_MIRROR_BITMAP_FNV1A;
    receipt->file_picker_arrows_file_offset = arrows_offset;
    receipt->file_picker_arrows_fnv1a =
        CSB_V1_FMTOWNS_UTILITY_FILE_PICKER_ARROWS_FNV1A;
    receipt->static_art_verified = 1;
    return 1;
}

int csb_v1_fmtowns_utility_icon_palette_rgb6(
    const CSB_V1_FmtownsUtilityMenuReceipt *receipt,
    uint8_t out_rgb6[CSB_V1_FMTOWNS_UTILITY_ICON_PALETTE_COLOR_COUNT][3])
{
    if (!receipt || !receipt->valid || !receipt->icon_palette_verified ||
        !out_rgb6) return 0;
    memcpy(out_rgb6, receipt->icon_palette_rgb6, sizeof(receipt->icon_palette_rgb6));
    return 1;
}

static uint32_t csb_v1_fmtowns_game_bytes_fnv1a(const unsigned char *bytes,
                                                  size_t size)
{
    uint32_t hash = 2166136261u;
    size_t index;

    if (!bytes) return 0u;
    for (index = 0u; index < size; ++index) {
        hash ^= bytes[index];
        hash *= 16777619u;
    }
    return hash;
}

static uint16_t csb_v1_fmtowns_game_read_le16(const unsigned char *bytes)
{
    return (uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8);
}

static uint32_t csb_v1_fmtowns_game_read_le32(const unsigned char *bytes)
{
    return (uint32_t)bytes[0] | ((uint32_t)bytes[1] << 8) |
           ((uint32_t)bytes[2] << 16) | ((uint32_t)bytes[3] << 24);
}

static int csb_v1_fmtowns_game_read_span(const char *path, uint32_t offset,
                                         unsigned char *bytes, size_t size)
{
    FILE *file;

    if (!path || !path[0] || !bytes || size == 0u) return 0;
    file = fopen(path, "rb");
    if (!file) return 0;
    if (fseek(file, (long)offset, SEEK_SET) != 0 ||
        fread(bytes, 1u, size, file) != size || ferror(file)) {
        fclose(file);
        return 0;
    }
    fclose(file);
    return 1;
}

static int csb_v1_fmtowns_utility_read_span(
    const CSB_V1_FmtownsUtilityHandoffReceipt *receipt, uint32_t offset,
    unsigned char *bytes, size_t size)
{
    if (!receipt || !bytes || size == 0u ||
        offset > receipt->executable_size ||
        size > receipt->executable_size - offset) return 0;
    if (receipt->executable_bytes &&
        receipt->executable_bytes_size == receipt->executable_size) {
        memcpy(bytes, receipt->executable_bytes + offset, size);
        return 1;
    }
    return csb_v1_fmtowns_game_read_span(receipt->executable_path, offset,
                                         bytes, size);
}

static uint32_t csb_v1_fmtowns_game_file_fnv1a(const char *path,
                                                 uint32_t *out_size)
{
    FILE *file;
    unsigned char buffer[4096];
    size_t count;
    uint32_t hash = 2166136261u;
    uint32_t size = 0u;

    if (out_size) *out_size = 0u;
    if (!path || !path[0]) return 0u;
    file = fopen(path, "rb");
    if (!file) return 0u;
    while ((count = fread(buffer, 1u, sizeof(buffer), file)) != 0u) {
        size_t index;
        if (size > UINT32_MAX - (uint32_t)count) {
            fclose(file);
            return 0u;
        }
        for (index = 0u; index < count; ++index) {
            hash ^= buffer[index];
            hash *= 16777619u;
        }
        size += (uint32_t)count;
    }
    if (ferror(file)) {
        fclose(file);
        return 0u;
    }
    fclose(file);
    if (out_size) *out_size = size;
    return hash;
}

static int csb_v1_fmtowns_utility_p3_header_open(
    const CSB_V1_FmtownsUtilityHandoffReceipt *source_receipt,
    uint32_t expected_file_size,
    CSB_V1_FmtownsUtilityHandoffReceipt *receipt)
{
    unsigned char header[0x78];
    uint32_t header_size;
    uint32_t declared_file_size;
    uint32_t runtime_offset;
    uint32_t runtime_size;
    uint32_t load_offset;
    uint32_t load_size;
    uint32_t memory_size;
    uint32_t initial_eip;

    if (!source_receipt || !receipt ||
        !csb_v1_fmtowns_utility_read_span(source_receipt, 0u, header,
                                          sizeof(header)) ||
        header[0] != 'P' || header[1] != '3' ||
        csb_v1_fmtowns_game_read_le16(header + 2u) != 1u) return 0;
    header_size = csb_v1_fmtowns_game_read_le16(header + 4u);
    declared_file_size = csb_v1_fmtowns_game_read_le32(header + 6u);
    runtime_offset = csb_v1_fmtowns_game_read_le32(header + 0x0cu);
    runtime_size = csb_v1_fmtowns_game_read_le32(header + 0x10u);
    load_offset = csb_v1_fmtowns_game_read_le32(header + 0x26u);
    load_size = csb_v1_fmtowns_game_read_le32(header + 0x2au);
    initial_eip = csb_v1_fmtowns_game_read_le32(header + 0x68u);
    memory_size = csb_v1_fmtowns_game_read_le32(header + 0x74u);
    if (header_size < 0x80u || header_size > expected_file_size ||
        declared_file_size != expected_file_size ||
        runtime_offset < header_size || runtime_offset > expected_file_size ||
        runtime_size > expected_file_size - runtime_offset ||
        load_offset < header_size || load_offset > expected_file_size ||
        load_size > expected_file_size - load_offset ||
        memory_size < load_size || initial_eip >= memory_size) return 0;
    receipt->p3_header_verified = 1;
    receipt->p3_header_size = header_size;
    receipt->p3_load_image_offset = load_offset;
    receipt->p3_load_image_size = load_size;
    receipt->p3_initial_eip = initial_eip;
    return 1;
}

int csb_v1_fmtowns_game_handoff_open(
    const CSB_V1_BootProfile *profile,
    CSB_V1_FmtownsSwitchLanguage language,
    CSB_V1_FmtownsGameHandoffReceipt *out_receipt)
{
    const char *name;
    uint32_t expected_size;
    uint32_t expected_hash;
    uint32_t actual_size;
    uint32_t actual_hash;
    uint32_t music_table_offset;
    const char *mini_name;
    uint32_t mini_expected_size;
    uint32_t mini_expected_hash;
    uint32_t mini_actual_size;
    uint32_t mini_actual_hash;
    uint16_t mini_expected_platform;
    unsigned char music_table[CSB_V1_FMTOWNS_GAME_MUSIC_TABLE_BYTES];
    CSB_V1_VariantId expected_variant;
    int packed_media;
    int path_length;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!profile || !profile->assets_verified || !profile->graphics_verified ||
        !profile->dungeon_verified) return 0;
    packed_media = profile->fmtowns_executable_bytes != NULL &&
        profile->fmtowns_mini_bytes != NULL &&
        profile->fmtowns_executable_size != 0u &&
        profile->fmtowns_mini_size != 0u;
    if (!profile->asset_root[0] && !packed_media) {
        return 0;
    }

    if (language == CSB_FMTOWNS_SWITCH_ENGLISH) {
        name = "CHTWE.EXP";
        expected_size = CSB_V1_FMTOWNS_CHTWE_SIZE;
        expected_hash = CSB_V1_FMTOWNS_CHTWE_FNV1A;
        music_table_offset = CSB_V1_FMTOWNS_CHTWE_MUSIC_TABLE_OFFSET;
        mini_name = "CDATA/MINI.DAT";
        mini_expected_size = CSB_V1_FMTOWNS_CDATA_MINI_SIZE;
        mini_expected_hash = CSB_V1_FMTOWNS_CDATA_MINI_FNV1A;
        mini_expected_platform = 7u;
        expected_variant = CSB_V1_VARIANT_FMTOWNS_EN;
    } else if (language == CSB_FMTOWNS_SWITCH_JAPANESE) {
        name = "CHTWJ.EXP";
        expected_size = CSB_V1_FMTOWNS_CHTWJ_SIZE;
        expected_hash = CSB_V1_FMTOWNS_CHTWJ_FNV1A;
        music_table_offset = CSB_V1_FMTOWNS_CHTWJ_MUSIC_TABLE_OFFSET;
        mini_name = "CJDATA/MINI.DAT";
        mini_expected_size = CSB_V1_FMTOWNS_CJDATA_MINI_SIZE;
        mini_expected_hash = CSB_V1_FMTOWNS_CJDATA_MINI_FNV1A;
        mini_expected_platform = 8u;
        expected_variant = CSB_V1_VARIANT_FMTOWNS_JA;
    } else {
        return 0;
    }
    if (profile->variant_id != expected_variant) {
        return 0;
    }
    if (packed_media) {
        path_length = snprintf(out_receipt->executable_path,
                               sizeof(out_receipt->executable_path), "%s::%s",
                               profile->asset_root, name);
        if (path_length < 0 || (size_t)path_length >=
                sizeof(out_receipt->executable_path)) return 0;
        actual_size = (uint32_t)profile->fmtowns_executable_size;
        actual_hash = csb_v1_fmtowns_game_bytes_fnv1a(
            profile->fmtowns_executable_bytes,
            profile->fmtowns_executable_size);
    } else {
        if (snprintf(out_receipt->executable_path,
                     sizeof(out_receipt->executable_path), "%s/%s",
                     profile->asset_root, name) < 0 ||
            strlen(out_receipt->executable_path) >=
                sizeof(out_receipt->executable_path)) return 0;
        actual_hash = csb_v1_fmtowns_game_file_fnv1a(
            out_receipt->executable_path, &actual_size);
    }
    if (actual_size != expected_size || actual_hash != expected_hash) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    /* ReDMCSB MUSIC.C G4099 (line 6) is indexed in F0743 at lines 632-646.
     * Bind that exact 10*32*32 payload from the already authenticated F31
     * executable, rather than recreating a coordinate-to-music table. */
    out_receipt->executable_bytes = packed_media
        ? profile->fmtowns_executable_bytes : NULL;
    out_receipt->executable_bytes_size = packed_media
        ? profile->fmtowns_executable_size : 0u;
    out_receipt->executable_size = actual_size;
    if (!csb_v1_fmtowns_game_bind_entrance_palette(out_receipt) ||
        !csb_v1_fmtowns_game_bind_dungeon_palettes(out_receipt) ||
        !csb_v1_fmtowns_game_bind_spell_table(out_receipt, language)) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    if (!csb_v1_fmtowns_game_read_executable_span(
            out_receipt, music_table_offset, music_table,
            sizeof(music_table)) ||
        csb_v1_fmtowns_game_bytes_fnv1a(music_table,
                                        sizeof(music_table)) !=
            CSB_V1_FMTOWNS_GAME_MUSIC_TABLE_FNV1A) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    out_receipt->language = language;
    out_receipt->variant_id = profile->variant_id;
    out_receipt->executable_size = actual_size;
    out_receipt->executable_fnv1a = actual_hash;
    out_receipt->executable_verified = 1;
    out_receipt->language_matches_profile = 1;
    out_receipt->game_program_is_c03_game = 1;
    snprintf(out_receipt->executable_name, sizeof(out_receipt->executable_name),
             "%s", name);
    snprintf(out_receipt->graphics_md5, sizeof(out_receipt->graphics_md5),
             "%s", profile->graphics_md5);
    snprintf(out_receipt->dungeon_md5, sizeof(out_receipt->dungeon_md5),
             "%s", profile->dungeon_md5);
    /* ReDMCSB CEDTDATA.C G2297 lines 380-387 selects CDATA/CJDATA MINI.DAT
     * for F31E/F31J. F0435 then reads its native 512-byte save header. The
     * shipped seed is recorded by exact bytes here, never decoded as the
     * unrelated big-endian Atari/Amiga GAMEBLOCK layout. */
    /* The packed path is provenance for the already-owned member bytes.
     * mini_name already includes CDATA/CJDATA, so adding the directory a
     * second time produced a nonexistent `archive::CDATA/CDATA/MINI.DAT`
     * identity even though the real bytes had passed their hash gate. */
    if (snprintf(out_receipt->startup_mini_path,
                 sizeof(out_receipt->startup_mini_path),
                 packed_media ? "%s::%s" : "%s/%s",
                 profile->asset_root, mini_name) >= 0 &&
        strlen(out_receipt->startup_mini_path) <
            sizeof(out_receipt->startup_mini_path)) {
        if (packed_media) {
            mini_actual_size = (uint32_t)profile->fmtowns_mini_size;
            mini_actual_hash = csb_v1_fmtowns_game_bytes_fnv1a(
                profile->fmtowns_mini_bytes, profile->fmtowns_mini_size);
        } else {
            mini_actual_hash = csb_v1_fmtowns_game_file_fnv1a(
                out_receipt->startup_mini_path, &mini_actual_size);
        }
        out_receipt->startup_mini_bytes = packed_media
            ? profile->fmtowns_mini_bytes : NULL;
        out_receipt->startup_mini_bytes_size = packed_media
            ? profile->fmtowns_mini_size : 0u;
        out_receipt->startup_mini_size = mini_actual_size;
        out_receipt->startup_mini_fnv1a = mini_actual_hash;
        out_receipt->startup_mini_verified =
            mini_actual_size == mini_expected_size &&
            mini_actual_hash == mini_expected_hash;
        if (!out_receipt->startup_mini_verified ||
            !csb_v1_fmtowns_game_startup_mini_header_open(
                mini_expected_platform, out_receipt)) {
            memset(out_receipt, 0, sizeof(*out_receipt));
            return 0;
        }
    }
    out_receipt->music_table_verified = 1;
    out_receipt->music_table_source_offset = music_table_offset;
    out_receipt->music_table_size = sizeof(music_table);
    out_receipt->music_table_fnv1a = CSB_V1_FMTOWNS_GAME_MUSIC_TABLE_FNV1A;
    out_receipt->source_evidence =
        "ReDMCSB COMPILE.H EXEID 60/61 lines 367-375; "
        "STARTUP1.C F0435 line 163; CEDTDATA.C G2297 lines 380-387/F7051 "
        "lines 211-255; CEDTINC6.C F7061/F7057/F7059; CEDTINCA.C F7063; "
        "CEDTINCT.C F7054; "
        "DEFS.H C5/F7/F8/C13; "
        "ENTRANCE.C F0806 lines 409-443/C28_ENTRANCE_CSB; "
        "DRAWVIEW.C lines 97-208 G8151-G8156 and lines 383-444 G8174/G8176; "
        "ENTRANCE.C F0807 line 85; "
        "MUSIC.C G4099 line 6/F0743 lines 632-646";
    out_receipt->valid = 1;
    return 1;
}

int csb_v1_fmtowns_game_user_save_handoff_open(
    const CSB_V1_BootProfile *profile,
    CSB_V1_FmtownsSwitchLanguage language,
    const char *save_path,
    CSB_V1_FmtownsGameHandoffReceipt *out_receipt)
{
    CSB_V1_FmtownsGameHandoffReceipt retail;
    CSB_V1_FmtownsUserSaveReceipt user_save;

    if (!out_receipt || !save_path || !save_path[0]) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));

    /* ReDMCSB STARTUP1.C F0435 reaches the C5 save reader only after the
     * language-owned C03 program is selected.  Keep that retail-program
     * admission separate from the variable bytes of a user-created slot. */
    memset(&retail, 0, sizeof(retail));
    if (!csb_v1_fmtowns_game_handoff_open(profile, language, &retail))
        return 0;
    /* A packed CD has no writable host pathname for MINI.DAT.  Its verified
     * F0435 bootstrap receipt already owns the exact C5 byte view, so a
     * resume request that names that same archive locator must retain the
     * receipt rather than trying to reopen `archive.zip::CDATA/MINI.DAT` as
     * a loose user slot.  Real CSBGAME.DAT paths still take the validation
     * and backup-recovery route below. */
    if (retail.startup_mini_bytes &&
        retail.startup_mini_bytes_size != 0u &&
        strcmp(save_path, retail.startup_mini_path) == 0) {
        *out_receipt = retail;
        return 1;
    }
    memset(&user_save, 0, sizeof(user_save));
    /* Keep the legacy F0435 handoff on the same canonical-slot recovery
     * route as M11.  Utility/file-picker callers must not bypass a validated
     * CSBGAME.BAK merely because they consume the older receipt shape. */
    if (!csb_v1_fmtowns_game_user_save_open_or_restore_backup(
            profile, &retail, save_path, &user_save)) return 0;
    /* Preserve this legacy receipt API for callers which then use the
     * existing F0435 state loader.  The source path and all decoded bounds
     * now come from the real user-save validator, so C12 Prison is not
     * accidentally constrained by MINI.DAT's C13 bootstrap identity. */
    retail.startup_mini_size = user_save.source_size;
    retail.startup_mini_fnv1a = user_save.source_fnv1a;
    snprintf(retail.startup_mini_path, sizeof(retail.startup_mini_path), "%s",
             user_save.source_path);
    retail.startup_mini_verified = 1;
    retail.startup_mini_header_verified = 1;
    retail.startup_mini_header_key = user_save.header_key;
    retail.startup_mini_header_format_id = CSB_V1_FMTOWNS_SAVE_HEADER_FORMAT_C5;
    retail.startup_mini_header_platform = user_save.platform;
    retail.startup_mini_header_dungeon_id = user_save.dungeon_id;
    retail.startup_mini_save_parts_verified = 1;
    retail.startup_mini_party_champion_count = user_save.party_champion_count;
    retail.startup_mini_event_count = user_save.event_count;
    retail.startup_mini_first_unused_event_index = user_save.first_unused_event_index;
    retail.startup_mini_current_active_group_count = user_save.current_active_group_count;
    retail.startup_mini_event_maximum_count = user_save.event_maximum_count;
    retail.startup_mini_active_group_capacity = user_save.active_group_capacity;
    retail.startup_mini_game_time = user_save.game_time;
    retail.startup_mini_party_map_x = user_save.party_map_x;
    retail.startup_mini_party_map_y = user_save.party_map_y;
    retail.startup_mini_party_direction = user_save.party_direction;
    retail.startup_mini_party_map_index = user_save.party_map_index;
    retail.startup_mini_verified_save_body_offset = user_save.portraits_offset;
    retail.startup_mini_dungeon_tail_verified = 1;
    retail.startup_mini_dungeon_tail_offset = user_save.dungeon_tail_offset;
    retail.startup_mini_dungeon_tail_size = user_save.dungeon_tail_size;
    retail.startup_mini_dungeon_tail_checksum = user_save.dungeon_tail_checksum;
    retail.source_evidence =
        "ReDMCSB STARTUP1.C F0435 line 163; LOADSAVE.C F0435; "
        "CEDTINCD.C F7051/F7057; CEDTINCA.C F7063";
    retail.valid = 1;
    *out_receipt = retail;
    return 1;
}

int csb_v1_fmtowns_utility_handoff_open(
    const CSB_V1_BootProfile *profile,
    CSB_V1_FmtownsSwitchLanguage language,
    CSB_V1_FmtownsUtilityHandoffReceipt *out_receipt)
{
    const char *name;
    uint32_t expected_size;
    uint32_t expected_hash;
    uint32_t icon_palette_offset;
    uint32_t mirror_bitmap_offset;
    uint32_t file_picker_arrows_offset;
    uint32_t actual_size;
    uint32_t actual_hash;
    CSB_V1_VariantId expected_variant;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!profile || !profile->assets_verified || !profile->graphics_verified ||
        !profile->dungeon_verified || !profile->asset_root[0]) return 0;
    if (language == CSB_FMTOWNS_SWITCH_ENGLISH) {
        name = "UTILE.EXP";
        expected_size = CSB_V1_FMTOWNS_UTILE_SIZE;
        expected_hash = CSB_V1_FMTOWNS_UTILE_FNV1A;
        icon_palette_offset = CSB_V1_FMTOWNS_UTILE_ICON_PALETTE_OFFSET;
        mirror_bitmap_offset = CSB_V1_FMTOWNS_UTILE_MIRROR_BITMAP_OFFSET;
        file_picker_arrows_offset =
            CSB_V1_FMTOWNS_UTILE_FILE_PICKER_ARROWS_OFFSET;
        expected_variant = CSB_V1_VARIANT_FMTOWNS_EN;
    } else if (language == CSB_FMTOWNS_SWITCH_JAPANESE) {
        name = "UTILJ.EXP";
        expected_size = CSB_V1_FMTOWNS_UTILJ_SIZE;
        expected_hash = CSB_V1_FMTOWNS_UTILJ_FNV1A;
        icon_palette_offset = CSB_V1_FMTOWNS_UTILJ_ICON_PALETTE_OFFSET;
        mirror_bitmap_offset = CSB_V1_FMTOWNS_UTILJ_MIRROR_BITMAP_OFFSET;
        file_picker_arrows_offset =
            CSB_V1_FMTOWNS_UTILJ_FILE_PICKER_ARROWS_OFFSET;
        expected_variant = CSB_V1_VARIANT_FMTOWNS_JA;
    } else return 0;
    if (profile->variant_id != expected_variant) return 0;
    if (profile->fmtowns_utility_bytes && profile->fmtowns_utility_size) {
        out_receipt->executable_bytes = profile->fmtowns_utility_bytes;
        out_receipt->executable_bytes_size = profile->fmtowns_utility_size;
        actual_size = (uint32_t)profile->fmtowns_utility_size;
        actual_hash = csb_v1_fmtowns_game_bytes_fnv1a(
            profile->fmtowns_utility_bytes, profile->fmtowns_utility_size);
    } else {
        if (snprintf(out_receipt->executable_path,
                     sizeof(out_receipt->executable_path), "%s/%s",
                     profile->asset_root, name) < 0 ||
            strlen(out_receipt->executable_path) >=
                sizeof(out_receipt->executable_path)) return 0;
        actual_hash = csb_v1_fmtowns_game_file_fnv1a(
            out_receipt->executable_path, &actual_size);
    }
    if (actual_size != expected_size || actual_hash != expected_hash) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    /* COMPILE.H EXEID 63/64 lines 379-385 identifies these P3 executables
     * as separate C06_CEDT programs. Bind their native entry envelopes before
     * any future TBIOS/CEDT decoder consumes a menu or save command. */
    out_receipt->executable_size = actual_size;
    out_receipt->executable_fnv1a = actual_hash;
    if (!csb_v1_fmtowns_utility_p3_header_open(out_receipt, actual_size,
                                               out_receipt)) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    /* ReDMCSB CEDT027.C:45-62 declares C09_ICON.  The exact indexed RGB6
     * sequence is present in each verified F31 C06 image, at a different
     * raw offset because the English and Japanese P3 layouts differ. */
    if (!csb_v1_fmtowns_utility_icon_palette_open(
            out_receipt, icon_palette_offset, out_receipt)) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    /* ReDMCSB CEDT018.C F31 G2267/G2268 and CEDT006.C F7040/F7004 consume
     * these two native bitmaps. Their retail image locations differ by
     * language, so bind them before any host rendering path can see them. */
    if (!csb_v1_fmtowns_utility_static_art_open(
            out_receipt, mirror_bitmap_offset, file_picker_arrows_offset,
            out_receipt)) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    out_receipt->valid = 1;
    out_receipt->executable_verified = 1;
    out_receipt->language_matches_profile = 1;
    out_receipt->utility_program_is_c06_cedt = 1;
    out_receipt->language = language;
    out_receipt->variant_id = expected_variant;
    snprintf(out_receipt->executable_name, sizeof(out_receipt->executable_name),
             "%s", name);
    out_receipt->source_evidence =
        "ReDMCSB SWITCH.C F2279; AUTOEXEC.BAT exits 2/5; "
        "COMPILE.H EXEID 63/64 lines 379-385 C06_CEDT";
    return 1;
}

int csb_v1_fmtowns_utility_save_mapping_open(
    const CSB_V1_BootProfile *profile,
    CSB_V1_FmtownsSwitchLanguage language,
    CSB_V1_FmtownsUtilitySaveMappingReceipt *out_receipt)
{
    CSB_V1_FmtownsUtilityHandoffReceipt handoff;
    uint32_t source_offset;
    unsigned char source[CSB_V1_FMTOWNS_UTILITY_SAVE_CMP_MAPPING_BYTES];
    static const unsigned char expected[] =
        "2:\\#CHAMP_NAME#.CMP";

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (sizeof(expected) != CSB_V1_FMTOWNS_UTILITY_SAVE_CMP_MAPPING_BYTES ||
        !csb_v1_fmtowns_utility_handoff_open(profile, language, &handoff))
        return 0;
    source_offset = language == CSB_FMTOWNS_SWITCH_ENGLISH ?
        CSB_V1_FMTOWNS_UTILE_SAVE_CMP_MAPPING_OFFSET :
        CSB_V1_FMTOWNS_UTILJ_SAVE_CMP_MAPPING_OFFSET;
    if (!csb_v1_fmtowns_utility_read_span(
            &handoff, source_offset, source, sizeof(source)) ||
        memcmp(source, expected, sizeof(expected)) != 0) return 0;
    memcpy(out_receipt->template_bytes, source, sizeof(source));
    out_receipt->valid = 1;
    out_receipt->language = language;
    out_receipt->variant_id = handoff.variant_id;
    out_receipt->source_file_offset = source_offset;
    out_receipt->source_size = sizeof(source);
    out_receipt->source_fnv1a = csb_v1_fmtowns_game_bytes_fnv1a(
        source, sizeof(source));
    out_receipt->source_evidence =
        "ReDMCSB CEDT001.C F7000; CEDTDATA.C M747_FILE_ID_SAVE_CMP; "
        "verified UTILE/UTILJ 2:\\#CHAMP_NAME#.CMP mapping";
    return out_receipt->source_fnv1a != 0u;
}

int csb_v1_fmtowns_utility_menu_open(
    const CSB_V1_BootProfile *profile,
    CSB_V1_FmtownsSwitchLanguage language,
    CSB_V1_FmtownsUtilityMenuReceipt *out_receipt)
{
    CSB_V1_FmtownsUtilityHandoffReceipt handoff;
    uint32_t virtual_offset;
    uint32_t byte_count;
    uint32_t expected_hash;
    static const uint16_t k_english_offsets[
        CSB_V1_FMTOWNS_UTILITY_MENU_ACTION_COUNT] = {0u, 16u, 32u, 52u, 60u, 68u};
    static const uint16_t k_japanese_offsets[
        CSB_V1_FMTOWNS_UTILITY_MENU_ACTION_COUNT] = {0u, 12u, 28u, 44u, 52u, 60u};
    const uint16_t *offsets;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!csb_v1_fmtowns_utility_handoff_open(profile, language, &handoff))
        return 0;
    if (language == CSB_FMTOWNS_SWITCH_ENGLISH) {
        virtual_offset = CSB_V1_FMTOWNS_UTILE_MENU_VIRTUAL_OFFSET;
        byte_count = CSB_V1_FMTOWNS_UTILE_MENU_BYTES;
        expected_hash = CSB_V1_FMTOWNS_UTILE_MENU_FNV1A;
        offsets = k_english_offsets;
    } else if (language == CSB_FMTOWNS_SWITCH_JAPANESE) {
        virtual_offset = CSB_V1_FMTOWNS_UTILJ_MENU_VIRTUAL_OFFSET;
        byte_count = CSB_V1_FMTOWNS_UTILJ_MENU_BYTES;
        expected_hash = CSB_V1_FMTOWNS_UTILJ_MENU_FNV1A;
        offsets = k_japanese_offsets;
    } else return 0;
    if (byte_count > sizeof(out_receipt->source_bytes) ||
        virtual_offset > UINT32_MAX - handoff.p3_load_image_offset ||
        !csb_v1_fmtowns_utility_read_span(
            &handoff,
            handoff.p3_load_image_offset + virtual_offset,
            out_receipt->source_bytes, byte_count) ||
        csb_v1_fmtowns_game_bytes_fnv1a(out_receipt->source_bytes,
                                         byte_count) != expected_hash) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    memcpy(out_receipt->label_offsets, offsets,
           sizeof(out_receipt->label_offsets));
    out_receipt->icon_palette_verified = handoff.icon_palette_verified;
    out_receipt->icon_palette_file_offset = handoff.icon_palette_file_offset;
    memcpy(out_receipt->icon_palette_rgb6, handoff.icon_palette_rgb6,
           sizeof(out_receipt->icon_palette_rgb6));
    out_receipt->valid = 1;
    out_receipt->language = language;
    out_receipt->variant_id = handoff.variant_id;
    out_receipt->source_virtual_offset = virtual_offset;
    out_receipt->source_file_offset = handoff.p3_load_image_offset + virtual_offset;
    out_receipt->source_size = byte_count;
    out_receipt->source_fnv1a = expected_hash;
    out_receipt->source_evidence =
        "UTILE/UTILJ Phar Lap P3 disassembly: C06 menu label pool; "
        "ReDMCSB COMPILE.H EXEID 63/64 C06_CEDT";
    return 1;
}

int csb_v1_fmtowns_utility_game_source_open(
    const CSB_V1_BootProfile *profile,
    CSB_V1_FmtownsSwitchLanguage language,
    CSB_V1_FmtownsUtilityGameSourceReceipt *out_receipt)
{
    enum {
        C06_GAME_SOURCE_TITLE_VIRTUAL_OFFSET = 0x118a0u,
        C06_GAME_SOURCE_CHOICES_VIRTUAL_OFFSET = 0x1194cu,
        C06_GAME_SAVE_PROMPT_VIRTUAL_OFFSET = 0x11bb8u,
        C06_DIALOG_OK_VIRTUAL_OFFSET = 0x118feu,
        C06_GAME_SOURCE_TITLE_FNV1A = 0x7a56b380u,
        C06_GAME_SOURCE_CHOICES_FNV1A = 0x289297b9u,
        C06_GAME_SAVE_PROMPT_FNV1A = 0xe89d74ecu,
        C06_DIALOG_OK_FNV1A = 0xf60df1fdu,
        C06_GAME_SOURCE_TITLE_F31J_FNV1A = 0xff18ba3fu,
        C06_GAME_SOURCE_CHOICES_F31J_FNV1A = 0xfefe61beu,
        C06_GAME_SAVE_PROMPT_F31J_FNV1A = 0x4ba0cfecu,
        C06_DIALOG_OK_F31J_FNV1A = 0x973d52a6u
    };
    CSB_V1_FmtownsUtilityHandoffReceipt handoff;
    uint32_t hash;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if ((language != CSB_FMTOWNS_SWITCH_ENGLISH &&
         language != CSB_FMTOWNS_SWITCH_JAPANESE) ||
        !csb_v1_fmtowns_utility_handoff_open(profile, language, &handoff) ||
        !csb_v1_fmtowns_utility_read_span(
            &handoff, handoff.p3_load_image_offset +
            C06_GAME_SOURCE_TITLE_VIRTUAL_OFFSET, out_receipt->title,
            sizeof(out_receipt->title)) ||
        !csb_v1_fmtowns_utility_read_span(
            &handoff, handoff.p3_load_image_offset +
            C06_GAME_SOURCE_CHOICES_VIRTUAL_OFFSET, out_receipt->choices,
            sizeof(out_receipt->choices)) ||
        !csb_v1_fmtowns_utility_read_span(
            &handoff, handoff.p3_load_image_offset +
            C06_GAME_SAVE_PROMPT_VIRTUAL_OFFSET, out_receipt->save_prompt,
            sizeof(out_receipt->save_prompt)) ||
        !csb_v1_fmtowns_utility_read_span(
            &handoff, handoff.p3_load_image_offset +
            C06_DIALOG_OK_VIRTUAL_OFFSET, out_receipt->ok,
            sizeof(out_receipt->ok))) return 0;
    hash = csb_v1_fmtowns_game_bytes_fnv1a(out_receipt->title,
                                            sizeof(out_receipt->title));
    if (hash != (language == CSB_FMTOWNS_SWITCH_ENGLISH
                     ? C06_GAME_SOURCE_TITLE_FNV1A
                     : C06_GAME_SOURCE_TITLE_F31J_FNV1A) ||
        csb_v1_fmtowns_game_bytes_fnv1a(out_receipt->choices,
                                        sizeof(out_receipt->choices)) !=
            (language == CSB_FMTOWNS_SWITCH_ENGLISH
                 ? C06_GAME_SOURCE_CHOICES_FNV1A
                 : C06_GAME_SOURCE_CHOICES_F31J_FNV1A) ||
        csb_v1_fmtowns_game_bytes_fnv1a(out_receipt->save_prompt,
                                        sizeof(out_receipt->save_prompt)) !=
            (language == CSB_FMTOWNS_SWITCH_ENGLISH
                 ? C06_GAME_SAVE_PROMPT_FNV1A
                 : C06_GAME_SAVE_PROMPT_F31J_FNV1A) ||
        csb_v1_fmtowns_game_bytes_fnv1a(out_receipt->ok,
                                        sizeof(out_receipt->ok)) !=
            (language == CSB_FMTOWNS_SWITCH_ENGLISH
                 ? C06_DIALOG_OK_FNV1A
                 : C06_DIALOG_OK_F31J_FNV1A)) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    out_receipt->valid = 1;
    out_receipt->language = language;
    out_receipt->variant_id = handoff.variant_id;
    out_receipt->title_file_offset = handoff.p3_load_image_offset +
        C06_GAME_SOURCE_TITLE_VIRTUAL_OFFSET;
    out_receipt->choices_file_offset = handoff.p3_load_image_offset +
        C06_GAME_SOURCE_CHOICES_VIRTUAL_OFFSET;
    out_receipt->save_prompt_file_offset = handoff.p3_load_image_offset +
        C06_GAME_SAVE_PROMPT_VIRTUAL_OFFSET;
    out_receipt->ok_file_offset = handoff.p3_load_image_offset +
        C06_DIALOG_OK_VIRTUAL_OFFSET;
    out_receipt->source_fnv1a = hash ^
        (language == CSB_FMTOWNS_SWITCH_ENGLISH
             ? C06_GAME_SOURCE_CHOICES_FNV1A
             : C06_GAME_SOURCE_CHOICES_F31J_FNV1A) ^
        (language == CSB_FMTOWNS_SWITCH_ENGLISH
             ? C06_GAME_SAVE_PROMPT_FNV1A
             : C06_GAME_SAVE_PROMPT_F31J_FNV1A) ^
        (language == CSB_FMTOWNS_SWITCH_ENGLISH
             ? C06_DIALOG_OK_FNV1A
             : C06_DIALOG_OK_F31J_FNV1A);
    out_receipt->source_evidence =
        language == CSB_FMTOWNS_SWITCH_ENGLISH
            ? "UTILE.EXP P3 C06 strings at 0x118a0/0x1194c/0x11bb8; "
              "ReDMCSB CEDTDATA.C G7085/G7065 and diskneeded message table"
            : "UTILJ.EXP P3 Shift-JIS C06 strings at 0x118a0/0x1194c/0x11bb8; "
              "TBIOS glyph consumer remains required for rendering";
    return 1;
}

int csb_v1_fmtowns_utility_font_open(
    const CSB_V1_BootProfile *profile,
    CSB_V1_FmtownsSwitchLanguage language,
    CSB_V1_FmtownsUtilityFontReceipt *out_receipt)
{
    CSB_V1_FmtownsUtilityHandoffReceipt handoff;
    uint32_t source_offset;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!csb_v1_fmtowns_utility_handoff_open(profile, language, &handoff))
        return 0;
    if (language == CSB_FMTOWNS_SWITCH_ENGLISH) {
        source_offset = CSB_V1_FMTOWNS_UTILE_INTERFACE_FONT_OFFSET;
    } else if (language == CSB_FMTOWNS_SWITCH_JAPANESE) {
        source_offset = CSB_V1_FMTOWNS_UTILJ_INTERFACE_FONT_OFFSET;
    } else return 0;
    /* CEDT019.C:18 declares G1103; CEDTFNT.C:43-94 expands exactly these
     * 420 source bytes into native C06 text colors.  Bind the recovered
     * object to its own retail program rather than shipping its bytes. */
    if (!csb_v1_fmtowns_utility_read_span(
            &handoff, source_offset,
            out_receipt->source_bytes,
            sizeof(out_receipt->source_bytes)) ||
        csb_v1_fmtowns_game_bytes_fnv1a(out_receipt->source_bytes,
                                        sizeof(out_receipt->source_bytes)) !=
            CSB_V1_FMTOWNS_UTILITY_INTERFACE_FONT_FNV1A) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    out_receipt->valid = 1;
    out_receipt->language = language;
    out_receipt->variant_id = handoff.variant_id;
    out_receipt->source_file_offset = source_offset;
    out_receipt->source_size = sizeof(out_receipt->source_bytes);
    out_receipt->source_fnv1a = CSB_V1_FMTOWNS_UTILITY_INTERFACE_FONT_FNV1A;
    out_receipt->source_evidence =
        "ReDMCSB CEDT019.C G1103 lines 18-35; CEDTFNT.C F7337 lines 43-94";
    return 1;
}

static int csb_v1_fmtowns_game_cmp_filename_is_valid(const char *filename)
{
    size_t length;
    if (!filename) return 0;
    length = strlen(filename);
    if (length < 5u || length >= CSB_V1_FMTOWNS_UTILITY_PORTRAIT_FILENAME_CAPACITY)
        return 0;
    return filename[length - 4u] == '.' &&
        (filename[length - 3u] == 'C' || filename[length - 3u] == 'c') &&
        (filename[length - 2u] == 'M' || filename[length - 2u] == 'm') &&
        (filename[length - 1u] == 'P' || filename[length - 1u] == 'p');
}

static int csb_v1_fmtowns_game_portrait_catalog_entry_compare(
    const void *left, const void *right)
{
    const CSB_V1_FmtownsUtilityPortraitCatalogEntry *a = left;
    const CSB_V1_FmtownsUtilityPortraitCatalogEntry *b = right;
    return strcmp(a->filename, b->filename);
}

static int csb_v1_fmtowns_game_portrait_catalog_add(
    CSB_V1_FmtownsUtilityPortraitCatalog *catalog, const char *filename)
{
    CSB_V1_FmtownsUtilityPortraitCatalogEntry *entry;
    uint8_t bytes[CSB_FMTOWNS_PORTRAIT_FILE_SIZE];
    int written;

    if (!catalog || !filename) return 0;
    /* Directory entries such as . and .. are not candidates, not scan
     * failures. Only a .CMP record is subject to native PORTRAIT admission. */
    if (!csb_v1_fmtowns_game_cmp_filename_is_valid(filename)) return 1;
    if (catalog->entry_count >= CSB_V1_FMTOWNS_UTILITY_PORTRAIT_CATALOG_CAPACITY) {
        ++catalog->rejected_entry_count;
        return 1;
    }
    entry = &catalog->entries[catalog->entry_count];
    written = snprintf(entry->source_path, sizeof(entry->source_path), "%s/%s",
                       catalog->source_directory, filename);
    if (written < 0 || (size_t)written >= sizeof(entry->source_path) ||
        !csb_v1_fmtowns_game_read_span(entry->source_path, 0u, bytes,
                                        sizeof(bytes)) ||
        !csb_v1_fmtowns_portrait_decode(bytes, sizeof(bytes),
                                        (uint8_t[CSB_FMTOWNS_PORTRAIT_PIXEL_COUNT]){0},
                                        CSB_FMTOWNS_PORTRAIT_PIXEL_COUNT,
                                        &entry->portrait)) {
        ++catalog->rejected_entry_count;
        memset(entry, 0, sizeof(*entry));
        return 1;
    }
    memcpy(entry->filename, filename, strlen(filename) + 1u);
    entry->source_fnv1a = csb_v1_fmtowns_game_bytes_fnv1a(bytes, sizeof(bytes));
    ++catalog->entry_count;
    return 1;
}

static int csb_v1_fmtowns_game_portrait_catalog_add_memory(
    CSB_V1_FmtownsUtilityPortraitCatalog *catalog, const char *filename,
    const uint8_t *bytes, size_t byte_count, const char *source_path)
{
    CSB_V1_FmtownsUtilityPortraitCatalogEntry *entry;
    uint8_t pixels[CSB_FMTOWNS_PORTRAIT_PIXEL_COUNT];

    if (!catalog || !filename || !bytes ||
        byte_count != CSB_FMTOWNS_PORTRAIT_FILE_SIZE ||
        !csb_v1_fmtowns_game_cmp_filename_is_valid(filename)) return 0;
    if (catalog->entry_count >= CSB_V1_FMTOWNS_UTILITY_PORTRAIT_CATALOG_CAPACITY) {
        ++catalog->rejected_entry_count;
        return 1;
    }
    entry = &catalog->entries[catalog->entry_count];
    if (!csb_v1_fmtowns_portrait_decode(
            bytes, byte_count, pixels, sizeof(pixels), &entry->portrait)) {
        ++catalog->rejected_entry_count;
        memset(entry, 0, sizeof(*entry));
        return 1;
    }
    snprintf(entry->filename, sizeof(entry->filename), "%s", filename);
    if (source_path) {
        size_t source_path_length = strlen(source_path);
        /* A truncated member reference is not a usable provenance receipt.
         * Reject it rather than letting the compiler- or platform-dependent
         * snprintf truncation turn an authentic packed source into a path
         * that cannot be resolved again. */
        if (source_path_length >= sizeof(entry->source_path)) {
            ++catalog->rejected_entry_count;
            memset(entry, 0, sizeof(*entry));
            return 1;
        }
        memcpy(entry->source_path, source_path, source_path_length + 1u);
    }
    entry->source_bytes = bytes;
    entry->source_bytes_size = byte_count;
    entry->source_fnv1a = csb_v1_fmtowns_game_bytes_fnv1a(bytes, byte_count);
    ++catalog->entry_count;
    return 1;
}

int csb_v1_fmtowns_utility_portrait_catalog_open(
    const CSB_V1_BootProfile *profile,
    CSB_V1_FmtownsSwitchLanguage language,
    CSB_V1_FmtownsUtilityPortraitCatalog *out_catalog)
{
    int written;

    if (!out_catalog) return 0;
    memset(out_catalog, 0, sizeof(*out_catalog));
    /* Admission ties the catalogue to the same language-owned C06 program
     * selected by SWITCHTW before scanning any user-visible source files. */
    if (!profile || !profile->asset_root[0] ||
        (language != CSB_FMTOWNS_SWITCH_ENGLISH &&
         language != CSB_FMTOWNS_SWITCH_JAPANESE)) return 0;
    {
        CSB_V1_FmtownsUtilityHandoffReceipt handoff;
        memset(&handoff, 0, sizeof(handoff));
        if (!csb_v1_fmtowns_utility_handoff_open(profile, language, &handoff))
            return 0;
    }
    written = snprintf(out_catalog->source_directory,
                       sizeof(out_catalog->source_directory), "%s/PORTRAIT",
                       profile->asset_root);
    if (written < 0 || (size_t)written >= sizeof(out_catalog->source_directory))
        return 0;
    if (profile->fmtowns_portrait_count > 0u) {
        uint16_t index;
        for (index = 0u; index < profile->fmtowns_portrait_count; ++index) {
            char source_path[sizeof(out_catalog->source_directory) + 64u];
            written = snprintf(source_path, sizeof(source_path),
                               "%s::PORTRAIT/%s", profile->asset_root,
                               profile->fmtowns_portrait_names[index]);
            if (written < 0 || (size_t)written >= sizeof(source_path) ||
                !csb_v1_fmtowns_game_portrait_catalog_add_memory(
                    out_catalog, profile->fmtowns_portrait_names[index],
                    profile->fmtowns_portrait_bytes[index],
                    profile->fmtowns_portrait_sizes[index], source_path))
                return 0;
        }
        qsort(out_catalog->entries, out_catalog->entry_count,
              sizeof(out_catalog->entries[0]),
              csb_v1_fmtowns_game_portrait_catalog_entry_compare);
        out_catalog->valid = 1;
        out_catalog->language = language;
        out_catalog->source_evidence =
            "ReDMCSB CEDT008.C FILE_PICKER / CEDT001.C F7002_ReadCMP; "
            "PORTRAIT.C F7251; packed ISO member view";
        return 1;
    }
#if defined(_WIN32)
    {
        int written;
        WIN32_FIND_DATAA data;
        char pattern[sizeof(out_catalog->source_directory) + 8u];
        HANDLE handle;
        written = snprintf(pattern, sizeof(pattern), "%s\\*.CMP",
                           out_catalog->source_directory);
        if (written < 0 || (size_t)written >= sizeof(pattern)) return 0;
        handle = FindFirstFileA(pattern, &data);
        if (handle == INVALID_HANDLE_VALUE) return 0;
        do {
            if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0u &&
                !csb_v1_fmtowns_game_portrait_catalog_add(out_catalog,
                                                           data.cFileName)) {
                FindClose(handle);
                return 0;
            }
        } while (FindNextFileA(handle, &data) != 0);
        FindClose(handle);
    }
#else
    {
        DIR *directory = opendir(out_catalog->source_directory);
        struct dirent *entry;
        if (!directory) return 0;
        while ((entry = readdir(directory)) != NULL) {
            if (!csb_v1_fmtowns_game_portrait_catalog_add(out_catalog,
                                                           entry->d_name)) {
                closedir(directory);
                return 0;
            }
        }
        closedir(directory);
    }
#endif
    qsort(out_catalog->entries, out_catalog->entry_count,
          sizeof(out_catalog->entries[0]),
          csb_v1_fmtowns_game_portrait_catalog_entry_compare);
    out_catalog->valid = 1;
    out_catalog->language = language;
    out_catalog->source_evidence =
        "ReDMCSB CEDT008.C FILE_PICKER / CEDT001.C F7002_ReadCMP; "
        "PORTRAIT.C F7251";
    return 1;
}

static uint32_t csb_v1_fmtowns_utility_portrait_catalog_hash(
    const CSB_V1_FmtownsUtilityPortraitCatalog *catalog)
{
    uint32_t hash = 2166136261u;
    uint16_t index;

    if (!catalog || !catalog->valid || catalog->entry_count == 0u) return 0u;
    for (index = 0u; index < catalog->entry_count; ++index) {
        const CSB_V1_FmtownsUtilityPortraitCatalogEntry *entry =
            &catalog->entries[index];
        size_t i;
        for (i = 0u; i < sizeof(entry->filename) && entry->filename[i]; ++i) {
            hash ^= (uint8_t)entry->filename[i];
            hash *= 16777619u;
        }
        hash ^= entry->source_fnv1a;
        hash *= 16777619u;
        hash ^= entry->portrait.pixel_fnv1a;
        hash *= 16777619u;
    }
    return hash ? hash : 1u;
}

int csb_v1_fmtowns_utility_portrait_selector_open(
    const CSB_V1_FmtownsUtilityPortraitCatalog *catalog,
    uint16_t initial_index,
    CSB_V1_FmtownsUtilityPortraitSelector *out_selector)
{
    uint32_t catalog_hash;

    if (!out_selector) return 0;
    memset(out_selector, 0, sizeof(*out_selector));
    catalog_hash = csb_v1_fmtowns_utility_portrait_catalog_hash(catalog);
    if (!catalog_hash || initial_index >= catalog->entry_count ||
        !catalog->entries[initial_index].portrait.valid) return 0;
    out_selector->valid = 1;
    out_selector->selected_index = initial_index;
    out_selector->entry_count = catalog->entry_count;
    out_selector->catalog_fnv1a = catalog_hash;
    out_selector->catalog = catalog;
    out_selector->source_evidence =
        "ReDMCSB CEDT008.C F7083/F7084; CEDT001.C F7002_ReadCMP";
    return 1;
}

int csb_v1_fmtowns_utility_portrait_selector_move(
    CSB_V1_FmtownsUtilityPortraitSelector *selector,
    int direction)
{
    int next;

    if (!selector || !selector->valid || !selector->catalog ||
        selector->entry_count != selector->catalog->entry_count ||
        selector->catalog_fnv1a !=
            csb_v1_fmtowns_utility_portrait_catalog_hash(selector->catalog) ||
        (direction != -1 && direction != 1)) return 0;
    next = (int)selector->selected_index + direction;
    if (next < 0 || next >= (int)selector->entry_count ||
        !selector->catalog->entries[next].portrait.valid) return 0;
    selector->selected_index = (uint16_t)next;
    return 1;
}

int csb_v1_fmtowns_utility_portrait_selector_load(
    const CSB_V1_FmtownsUtilityPortraitSelector *selector,
    CSB_V1_PartyState *party, uint16_t selected_champion,
    CSB_V1_FmtownsStartupPortraitReceipt *portraits)
{
    if (!selector || !selector->valid || !selector->catalog ||
        selector->entry_count != selector->catalog->entry_count ||
        selector->catalog_fnv1a !=
            csb_v1_fmtowns_utility_portrait_catalog_hash(selector->catalog))
        return 0;
    return csb_v1_fmtowns_utility_load_portrait(
        selector->catalog, selector->selected_index, party,
        selected_champion, portraits);
}

int csb_v1_fmtowns_utility_load_portrait(
    const CSB_V1_FmtownsUtilityPortraitCatalog *catalog,
    uint16_t catalog_index, CSB_V1_PartyState *party,
    uint16_t selected_champion,
    CSB_V1_FmtownsStartupPortraitReceipt *portraits)
{
    const CSB_V1_FmtownsUtilityPortraitCatalogEntry *entry;
    unsigned char source[CSB_FMTOWNS_PORTRAIT_FILE_SIZE];
    uint32_t source_size = 0u;
    uint32_t source_fnv1a;
    CSB_V1_Champion *champion;

    if (!catalog || !catalog->valid || catalog_index >= catalog->entry_count ||
        !party || selected_champion >= (uint16_t)party->ChampionCount ||
        !portraits || !portraits->valid) return 0;
    entry = &catalog->entries[catalog_index];
    if (!entry->portrait.valid ||
        (!entry->source_bytes && !entry->source_path[0])) return 0;
    if (entry->source_bytes) {
        if (entry->source_bytes_size != sizeof(source)) return 0;
        memcpy(source, entry->source_bytes, sizeof(source));
    } else if (!csb_v1_fmtowns_game_read_span(
                   entry->source_path, 0u, source, sizeof(source))) {
        return 0;
    }
    if (!csb_v1_fmtowns_portrait_probe(source, sizeof(source))) return 0;
    /* CEDT008's selector returns a file name, not a stale directory row.
     * Recheck the exact admitted file before copying its payload so a host
     * replacement between scan and selection cannot enter the party. */
    if (entry->source_bytes) {
        source_size = (uint32_t)entry->source_bytes_size;
        source_fnv1a = csb_v1_fmtowns_game_bytes_fnv1a(
            entry->source_bytes, entry->source_bytes_size);
    } else {
        source_fnv1a = csb_v1_fmtowns_game_file_fnv1a(
            entry->source_path, &source_size);
    }
    if (source_size != CSB_FMTOWNS_PORTRAIT_FILE_SIZE ||
        source_fnv1a == 0u || source_fnv1a != entry->source_fnv1a) return 0;

    champion = &party->Champions[selected_champion];
    memset(champion->Name, 0, sizeof(champion->Name));
    memcpy(champion->Name, entry->portrait.name,
           sizeof(champion->Name) - 1u < sizeof(entry->portrait.name) ?
               sizeof(champion->Name) - 1u : sizeof(entry->portrait.name));
    memset(champion->Title, 0, sizeof(champion->Title));
    memcpy(champion->Title, entry->portrait.title,
           sizeof(champion->Title) - 1u < sizeof(entry->portrait.title) ?
               sizeof(champion->Title) - 1u : sizeof(entry->portrait.title));
    memcpy(portraits->source_bytes[selected_champion],
           source + CSB_FMTOWNS_PORTRAIT_HEADER_SIZE,
           CSB_FMTOWNS_PORTRAIT_DATA_SIZE);
    portraits->source_evidence =
        "ReDMCSB CEDT001.C F7002_ReadCMP; CEDT019.C F2124";
    return 1;
}

static int csb_v1_fmtowns_utility_name_matches(
    const char *left, const char *right)
{
    if (!left || !right || !left[0] || !right[0]) return 0;
    return strncmp(left, right, CSB_V1_MAX_NAME_LEN) == 0;
}

int csb_v1_fmtowns_utility_save_portraits(
    const CSB_V1_FmtownsUtilityPortraitCatalog *catalog,
    const CSB_V1_PartyState *party,
    const CSB_V1_FmtownsStartupPortraitReceipt *portraits)
{
    unsigned int champion_index;
    int entry_indices[CSB_V1_FMTOWNS_STARTUP_PORTRAIT_COUNT];

    if (!catalog || !catalog->valid || !party || !portraits ||
        !portraits->valid || party->ChampionCount < 1 ||
        party->ChampionCount > (int)CSB_V1_FMTOWNS_STARTUP_PORTRAIT_COUNT)
        return 0;

    /* Legacy test-only helper: resolve existing media records before touching
     * disk.  The live F7001 route uses F7000's selected dynamic destination
     * and must never call this batch rewrite. */
    for (champion_index = 0u;
         champion_index < (unsigned int)party->ChampionCount;
         ++champion_index) {
        unsigned int entry_index;
        entry_indices[champion_index] = -1;
        for (entry_index = 0u; entry_index < catalog->entry_count;
             ++entry_index) {
            const CSB_V1_FmtownsUtilityPortraitCatalogEntry *entry =
                &catalog->entries[entry_index];
            if (entry->portrait.valid &&
                csb_v1_fmtowns_utility_name_matches(
                    party->Champions[champion_index].Name,
                    entry->portrait.name)) {
                entry_indices[champion_index] = (int)entry_index;
                break;
            }
        }
        if (entry_indices[champion_index] < 0) return 0;
    }
    /* A packed CD member is an authenticated read source, not a writable
     * save destination.  Reject the complete operation before touching any
     * loose file so a mixed catalog can never produce a partial save. */
    for (champion_index = 0u;
         champion_index < (unsigned int)party->ChampionCount;
         ++champion_index) {
        if (catalog->entries[entry_indices[champion_index]].source_bytes)
            return 0;
    }

    for (champion_index = 0u;
         champion_index < (unsigned int)party->ChampionCount;
         ++champion_index) {
        const CSB_V1_FmtownsUtilityPortraitCatalogEntry *entry =
            &catalog->entries[entry_indices[champion_index]];
        unsigned char source[CSB_FMTOWNS_PORTRAIT_FILE_SIZE];
        char temporary_path[sizeof(entry->source_path) + 32u];
        FILE *file;

        if (!csb_v1_fmtowns_game_read_span(entry->source_path, 0u, source,
                                           sizeof(source)) ||
            !csb_v1_fmtowns_portrait_probe(source, sizeof(source)) ||
            snprintf(temporary_path, sizeof(temporary_path), "%s.firestaff-tmp",
                     entry->source_path) < 0 ||
            strlen(temporary_path) >= sizeof(temporary_path))
            return 0;
        /* CEDT006.C dispatches SAVE CHAMPIONS by first copying the live
         * champion name (8 bytes) and title (20 bytes) into C06's CMP
         * buffer, then F7001 writes that complete record.  Keeping the
         * original header here discarded legitimate title changes while
         * reporting a successful C06 save.  The remaining header words are
        * source identity/format fields and stay byte-for-byte intact. */
        memset(source + 16u, 0, CSB_FMTOWNS_PORTRAIT_NAME_LEN);
        strncpy((char *)source + 16u, party->Champions[champion_index].Name,
                CSB_FMTOWNS_PORTRAIT_NAME_LEN);
        memset(source + 24u, 0, CSB_FMTOWNS_PORTRAIT_TITLE_LEN);
        strncpy((char *)source + 24u, party->Champions[champion_index].Title,
                CSB_FMTOWNS_PORTRAIT_TITLE_LEN);
        memcpy(source + CSB_FMTOWNS_PORTRAIT_HEADER_SIZE,
               portraits->source_bytes[champion_index],
               CSB_FMTOWNS_PORTRAIT_DATA_SIZE);
        file = fopen(temporary_path, "wb");
        if (!file) {
            remove(temporary_path);
            return 0;
        }
        if (fwrite(source, 1u, sizeof(source), file) != sizeof(source) ||
            fflush(file) != 0) {
            fclose(file);
            remove(temporary_path);
            return 0;
        }
        if (fclose(file) != 0) {
            remove(temporary_path);
            return 0;
        }
        if (rename(temporary_path, entry->source_path) != 0) {
            remove(temporary_path);
            return 0;
        }
    }
    return 1;
}

static int csb_v1_fmtowns_utility_portrait_medium_path(
    char *out, size_t out_size)
{
    char root[FSP_PATH_MAX];

    if (!out || out_size == 0u) return 0;
#if defined(_WIN32)
    {
        DWORD length = GetModuleFileNameA(NULL, root, (DWORD)sizeof(root));
        if (length == 0u || length >= sizeof(root) ||
            !FSP_ParentDir(root, sizeof(root), root)) return 0;
        return FSP_JoinPath(out, out_size, root, "portraits");
    }
#else
    /* The default originals location ends in ~/.firestaff/data.  Its parent
     * is deliberately the user-selected Firestaff root, not a game-media
     * directory, because F7000's drive 2: is a writable portrait disk. */
    if (!FSP_GetDefaultOriginalsDir(root, sizeof(root)) ||
        !FSP_ParentDir(root, sizeof(root), root)) return 0;
    return FSP_JoinPath(out, out_size, root, "portraits");
#endif
}

int csb_v1_fmtowns_utility_portrait_medium_catalog_open(
    const CSB_V1_FmtownsUtilitySaveMappingReceipt *mapping,
    CSB_V1_FmtownsSwitchLanguage language,
    CSB_V1_FmtownsUtilityPortraitCatalog *out_catalog)
{
    if (!out_catalog) return 0;
    memset(out_catalog, 0, sizeof(*out_catalog));
    if (!mapping || !mapping->valid ||
        strcmp(mapping->template_bytes, "2:\\#CHAMP_NAME#.CMP") != 0 ||
        (language != CSB_FMTOWNS_SWITCH_ENGLISH &&
         language != CSB_FMTOWNS_SWITCH_JAPANESE) ||
        !csb_v1_fmtowns_utility_portrait_medium_path(
            out_catalog->source_directory,
            sizeof(out_catalog->source_directory)) ||
        !FSP_CreateDirectoryRecursive(out_catalog->source_directory)) return 0;
#if defined(_WIN32)
    {
        int written;
        WIN32_FIND_DATAA data;
        char pattern[sizeof(out_catalog->source_directory) + 8u];
        HANDLE handle;
        written = snprintf(pattern, sizeof(pattern), "%s\\*.CMP",
                           out_catalog->source_directory);
        if (written < 0 || (size_t)written >= sizeof(pattern)) return 0;
        handle = FindFirstFileA(pattern, &data);
        if (handle != INVALID_HANDLE_VALUE) {
            do {
                if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0u &&
                    !csb_v1_fmtowns_game_portrait_catalog_add(out_catalog,
                                                               data.cFileName)) {
                    FindClose(handle);
                    return 0;
                }
            } while (FindNextFileA(handle, &data) != 0);
            FindClose(handle);
        }
    }
#else
    {
        DIR *directory = opendir(out_catalog->source_directory);
        struct dirent *entry;
        if (!directory) return 0;
        while ((entry = readdir(directory)) != NULL) {
            if (!csb_v1_fmtowns_game_portrait_catalog_add(out_catalog,
                                                           entry->d_name)) {
                closedir(directory);
                return 0;
            }
        }
        closedir(directory);
    }
#endif
    qsort(out_catalog->entries, out_catalog->entry_count,
          sizeof(out_catalog->entries[0]),
          csb_v1_fmtowns_game_portrait_catalog_entry_compare);
    out_catalog->valid = 1;
    out_catalog->language = language;
    out_catalog->source_evidence =
        "ReDMCSB CEDT008.C F7083/F7084 NEW DISK; CEDT001.C F7000; "
        "verified M747 2:\\#CHAMP_NAME#.CMP medium";
    return 1;
}

int csb_v1_fmtowns_utility_save_selected_portrait(
    const CSB_V1_FmtownsUtilitySaveMappingReceipt *mapping,
    const CSB_V1_FmtownsUtilityPortraitCatalog *catalog,
    const CSB_V1_PartyState *party,
    const CSB_V1_FmtownsStartupPortraitReceipt *portraits,
    uint16_t selected_champion, char *out_path, size_t out_path_size)
{
    const CSB_V1_Champion *champion;
    unsigned char record[CSB_FMTOWNS_PORTRAIT_FILE_SIZE];
    char directory[FSP_PATH_MAX];
    char filename[CSB_V1_MAX_NAME_LEN + 5u];
    char destination[FSP_PATH_MAX];
    char temporary[FSP_PATH_MAX];
    size_t name_length = 0u;
    size_t index;
    const CSB_V1_FmtownsUtilityPortraitCatalogEntry *template_entry = NULL;
    FILE *file;

    if (out_path && out_path_size != 0u) out_path[0] = '\0';
    if (!mapping || !mapping->valid ||
        strcmp(mapping->template_bytes, "2:\\#CHAMP_NAME#.CMP") != 0 ||
        !catalog || !catalog->valid || !party || !portraits || !portraits->valid ||
        selected_champion >= (uint16_t)party->ChampionCount ||
        selected_champion >= CSB_V1_FMTOWNS_STARTUP_PORTRAIT_COUNT ||
        !csb_v1_fmtowns_utility_portrait_medium_path(
            directory, sizeof(directory)) ||
        !FSP_CreateDirectoryRecursive(directory)) return 0;

    champion = &party->Champions[selected_champion];
    /* G2246_CMPData starts from a real CMP selected by C06.  Preserve that
     * source record's opaque 44-byte identity header instead of inventing
     * platform/dungeon words for a new host file. */
    for (index = 0u; index < catalog->entry_count; ++index) {
        const CSB_V1_FmtownsUtilityPortraitCatalogEntry *entry =
            &catalog->entries[index];
        if (entry->portrait.valid &&
            strncmp(entry->portrait.name, champion->Name,
                    CSB_V1_MAX_NAME_LEN) == 0) {
            template_entry = entry;
            break;
        }
    }
    if (!template_entry) return 0;
    /* F7000 admits only A-Z from the first eight CMP name bytes and falls
     * back to PORTRAIT when none survive.  Do not use a host filename or
     * preserve punctuation that the original file operation discards. */
    for (index = 0u; index < CSB_V1_MAX_NAME_LEN &&
         champion->Name[index] != '\0'; ++index) {
        unsigned char ch = (unsigned char)champion->Name[index];
        if (ch >= 'A' && ch <= 'Z') filename[name_length++] = (char)ch;
    }
    if (name_length == 0u) {
        memcpy(filename, "PORTRAIT", 8u);
        name_length = 8u;
    }
    memcpy(filename + name_length, ".CMP", 5u);
    if (!FSP_JoinPath(destination, sizeof(destination), directory, filename) ||
        snprintf(temporary, sizeof(temporary), "%s.firestaff-tmp",
                 destination) < 0 || strlen(temporary) >= sizeof(temporary))
        return 0;

    if (template_entry->source_bytes) {
        if (template_entry->source_bytes_size != sizeof(record)) return 0;
        memcpy(record, template_entry->source_bytes, sizeof(record));
    } else if (!csb_v1_fmtowns_game_read_span(template_entry->source_path, 0u,
                                               record, sizeof(record)) ||
               !csb_v1_fmtowns_portrait_probe(record, sizeof(record))) {
        return 0;
    }
    /* F7000 writes 508 bytes after replacing its live name, title and planar
     * portrait payload.  The copied source header stays byte-exact. */
    memcpy(record + 16u, champion->Name, CSB_FMTOWNS_PORTRAIT_NAME_LEN);
    memcpy(record + 24u, champion->Title, CSB_FMTOWNS_PORTRAIT_TITLE_LEN);
    memcpy(record + CSB_FMTOWNS_PORTRAIT_HEADER_SIZE,
           portraits->source_bytes[selected_champion],
           CSB_FMTOWNS_PORTRAIT_DATA_SIZE);
    if (!csb_v1_fmtowns_portrait_probe(record, sizeof(record))) return 0;

    file = fopen(temporary, "wb");
    if (!file) return 0;
    if (fwrite(record, 1u, sizeof(record), file) != sizeof(record) ||
        fflush(file) != 0) {
        fclose(file);
        remove(temporary);
        return 0;
    }
    if (fclose(file) != 0) {
        remove(temporary);
        return 0;
    }
    if (rename(temporary, destination) != 0) {
        remove(temporary);
        return 0;
    }
    if (out_path && out_path_size != 0u) {
        if (snprintf(out_path, out_path_size, "%s", destination) < 0 ||
            strlen(destination) >= out_path_size) return 0;
    }
    return 1;
}

int csb_v1_fmtowns_utility_menu_action_at(
    const CSB_V1_FmtownsUtilityMenuReceipt *receipt,
    int16_t source_x, int16_t source_y,
    CSB_V1_FmtownsUtilityMenuHitBox *out_hit_box)
{
    static const CSB_V1_FmtownsUtilityMenuHitBox k_english_hits[
        CSB_V1_FMTOWNS_UTILITY_MENU_ACTION_COUNT] = {
        { CSB_V1_FMTOWNS_UTILITY_ACTION_LOAD_CHAMPIONS, 2, 92, 186, 194 },
        { CSB_V1_FMTOWNS_UTILITY_ACTION_SAVE_CHAMPIONS, 102, 192, 186, 194 },
        { CSB_V1_FMTOWNS_UTILITY_ACTION_MAKE_NEW_ADVENTURE, 202, 316, 186, 194 },
        { CSB_V1_FMTOWNS_UTILITY_ACTION_REVERT, 156, 196, 159, 167 },
        { CSB_V1_FMTOWNS_UTILITY_ACTION_UNDO, 225, 253, 159, 167 },
        { CSB_V1_FMTOWNS_UTILITY_ACTION_QUIT, 288, 316, 5, 13 }
    };
    static const CSB_V1_FmtownsUtilityMenuHitBox k_japanese_hits[
        CSB_V1_FMTOWNS_UTILITY_MENU_ACTION_COUNT] = {
        { CSB_V1_FMTOWNS_UTILITY_ACTION_LOAD_CHAMPIONS, 2, 92, 179, 196 },
        { CSB_V1_FMTOWNS_UTILITY_ACTION_SAVE_CHAMPIONS, 98, 197, 179, 196 },
        { CSB_V1_FMTOWNS_UTILITY_ACTION_MAKE_NEW_ADVENTURE, 203, 317, 179, 196 },
        { CSB_V1_FMTOWNS_UTILITY_ACTION_REVERT, 156, 196, 154, 171 },
        { CSB_V1_FMTOWNS_UTILITY_ACTION_UNDO, 213, 253, 154, 171 },
        { CSB_V1_FMTOWNS_UTILITY_ACTION_QUIT, 266, 317, 6, 23 }
    };
    const CSB_V1_FmtownsUtilityMenuHitBox *hits;
    uint32_t index;

    if (out_hit_box) memset(out_hit_box, 0, sizeof(*out_hit_box));
    if (!receipt || !out_hit_box || !receipt->valid) return 0;
    if (receipt->language == CSB_FMTOWNS_SWITCH_ENGLISH) {
        hits = k_english_hits;
    } else if (receipt->language == CSB_FMTOWNS_SWITCH_JAPANESE) {
        hits = k_japanese_hits;
    } else return 0;

    /* ReDMCSB CEDTDATA.C G2272_MouseInputs keeps both box boundaries. */
    for (index = 0u; index < CSB_V1_FMTOWNS_UTILITY_MENU_ACTION_COUNT;
         ++index) {
        if (source_x >= hits[index].left && source_x <= hits[index].right &&
            source_y >= hits[index].top && source_y <= hits[index].bottom) {
            *out_hit_box = hits[index];
            return 1;
        }
    }
    return 0;
}

int csb_v1_fmtowns_game_music_track_at(
    const CSB_V1_FmtownsGameHandoffReceipt *receipt,
    uint32_t map_index,
    uint32_t map_x,
    uint32_t map_y,
    uint8_t *out_track)
{
    unsigned char track;
    uint32_t table_index;

    if (out_track) *out_track = 0u;
    if (!receipt || !out_track || !receipt->valid ||
        !receipt->executable_verified || !receipt->music_table_verified ||
        receipt->music_table_size != CSB_V1_FMTOWNS_GAME_MUSIC_TABLE_BYTES ||
        receipt->music_table_fnv1a != CSB_V1_FMTOWNS_GAME_MUSIC_TABLE_FNV1A ||
        map_index >= CSB_V1_FMTOWNS_GAME_MUSIC_MAP_COUNT ||
        map_x >= CSB_V1_FMTOWNS_GAME_MUSIC_MAP_WIDTH ||
        map_y >= CSB_V1_FMTOWNS_GAME_MUSIC_MAP_HEIGHT) return 0;
    table_index = map_index * CSB_V1_FMTOWNS_GAME_MUSIC_MAP_WIDTH *
                  CSB_V1_FMTOWNS_GAME_MUSIC_MAP_HEIGHT +
                  map_y * CSB_V1_FMTOWNS_GAME_MUSIC_MAP_WIDTH + map_x;
    if (!csb_v1_fmtowns_game_read_executable_span(
            receipt, receipt->music_table_source_offset + table_index,
            &track, sizeof(track))) return 0;
    *out_track = track;
    return 1;
}

int csb_v1_fmtowns_game_entrance_music_track(
    const CSB_V1_FmtownsGameHandoffReceipt *receipt,
    uint8_t *out_track)
{
    if (out_track) *out_track = 0u;
    if (!receipt || !out_track || !receipt->valid ||
        !receipt->executable_verified || !receipt->language_matches_profile ||
        !receipt->game_program_is_c03_game ||
        (receipt->variant_id != CSB_V1_VARIANT_FMTOWNS_EN &&
         receipt->variant_id != CSB_V1_VARIANT_FMTOWNS_JA)) {
        return 0;
    }
    /* ReDMCSB MUSIC.C:385, F31E/F31J G2038_auc_MusicIndexToMusicTrack =
     * { 2, 0, 0, 17 }; ENTRANCE.C:733/836 passes C0_MUSIC_ENTRANCE (0) to
     * F0741. F0719 subtracts two only for G4084's compacted timing table:
     * cdr_mtplay itself starts physical CD-DA track 02. */
    *out_track = CSB_V1_FMTOWNS_GAME_ENTRANCE_CDDA_TRACK;
    return 1;
}
