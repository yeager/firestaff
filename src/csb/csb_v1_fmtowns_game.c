#include "csb_v1_fmtowns_game.h"

#include <stdio.h>
#include <string.h>

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
    CSB_V1_FMTOWNS_UTILE_MENU_VIRTUAL_OFFSET = 0x11578u,
    CSB_V1_FMTOWNS_UTILJ_MENU_VIRTUAL_OFFSET = 0x11628u,
    CSB_V1_FMTOWNS_UTILE_MENU_BYTES = 76u,
    CSB_V1_FMTOWNS_UTILJ_MENU_BYTES = 68u,
    CSB_V1_FMTOWNS_UTILE_MENU_FNV1A = 0xfd9986bfu,
    CSB_V1_FMTOWNS_UTILJ_MENU_FNV1A = 0xdceefc60u,
    /* The retail F31E/F31J programs carry identical 10*32*32 selector
     * tables. These offsets are from the raw verified executable image. */
    CSB_V1_FMTOWNS_CHTWE_MUSIC_TABLE_OFFSET = 271144u,
    CSB_V1_FMTOWNS_CHTWJ_MUSIC_TABLE_OFFSET = 271624u,
    CSB_V1_FMTOWNS_GAME_MUSIC_TABLE_FNV1A = 0x3faffb70u
};

/* ReDMCSB CEDT018.C:829-838 clears the F31 screen, blacks its curtain and
 * loads C09_ICON before F7268 restores the normal curtain. CEDT027.C:45-62
 * owns C09_ICON itself. Keep the original Towns six-bit component values;
 * M11 is responsible for its RGB6-to-host presentation boundary. */
static const uint8_t k_csb_v1_fmtowns_utility_icon_palette_rgb6[
    CSB_V1_FMTOWNS_UTILITY_ICON_PALETTE_COLOR_COUNT][3] = {
    { 0x00u, 0x00u, 0x00u }, { 0x1bu, 0x1bu, 0x1bu },
    { 0x24u, 0x24u, 0x24u }, { 0x1bu, 0x09u, 0x00u },
    { 0x00u, 0x36u, 0x36u }, { 0x24u, 0x12u, 0x00u },
    { 0x00u, 0x24u, 0x00u }, { 0x00u, 0x36u, 0x00u },
    { 0x3fu, 0x00u, 0x00u }, { 0x3fu, 0x2du, 0x00u },
    { 0x36u, 0x24u, 0x1bu }, { 0x3fu, 0x3fu, 0x00u },
    { 0x12u, 0x12u, 0x12u }, { 0x2du, 0x2du, 0x2du },
    { 0x00u, 0x00u, 0x3fu }, { 0x3fu, 0x3fu, 0x3fu }
};

int csb_v1_fmtowns_utility_icon_palette_rgb6(
    uint8_t out_rgb6[CSB_V1_FMTOWNS_UTILITY_ICON_PALETTE_COLOR_COUNT][3])
{
    if (!out_rgb6) return 0;
    memcpy(out_rgb6, k_csb_v1_fmtowns_utility_icon_palette_rgb6,
           sizeof(k_csb_v1_fmtowns_utility_icon_palette_rgb6));
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
    const char *path, uint32_t expected_file_size,
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

    if (!path || !receipt ||
        !csb_v1_fmtowns_game_read_span(path, 0u, header, sizeof(header)) ||
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
    unsigned char music_table[CSB_V1_FMTOWNS_GAME_MUSIC_TABLE_BYTES];
    CSB_V1_VariantId expected_variant;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!profile || !profile->assets_verified || !profile->graphics_verified ||
        !profile->dungeon_verified || !profile->asset_root[0]) return 0;

    if (language == CSB_FMTOWNS_SWITCH_ENGLISH) {
        name = "CHTWE.EXP";
        expected_size = CSB_V1_FMTOWNS_CHTWE_SIZE;
        expected_hash = CSB_V1_FMTOWNS_CHTWE_FNV1A;
        music_table_offset = CSB_V1_FMTOWNS_CHTWE_MUSIC_TABLE_OFFSET;
        mini_name = "CDATA/MINI.DAT";
        mini_expected_size = CSB_V1_FMTOWNS_CDATA_MINI_SIZE;
        mini_expected_hash = CSB_V1_FMTOWNS_CDATA_MINI_FNV1A;
        expected_variant = CSB_V1_VARIANT_FMTOWNS_EN;
    } else if (language == CSB_FMTOWNS_SWITCH_JAPANESE) {
        name = "CHTWJ.EXP";
        expected_size = CSB_V1_FMTOWNS_CHTWJ_SIZE;
        expected_hash = CSB_V1_FMTOWNS_CHTWJ_FNV1A;
        music_table_offset = CSB_V1_FMTOWNS_CHTWJ_MUSIC_TABLE_OFFSET;
        mini_name = "CJDATA/MINI.DAT";
        mini_expected_size = CSB_V1_FMTOWNS_CJDATA_MINI_SIZE;
        mini_expected_hash = CSB_V1_FMTOWNS_CJDATA_MINI_FNV1A;
        expected_variant = CSB_V1_VARIANT_FMTOWNS_JA;
    } else {
        return 0;
    }
    if (profile->variant_id != expected_variant ||
        snprintf(out_receipt->executable_path,
                 sizeof(out_receipt->executable_path), "%s/%s",
                 profile->asset_root, name) < 0 ||
        strlen(out_receipt->executable_path) >=
            sizeof(out_receipt->executable_path)) return 0;

    actual_hash = csb_v1_fmtowns_game_file_fnv1a(
        out_receipt->executable_path, &actual_size);
    if (actual_size != expected_size || actual_hash != expected_hash) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    /* ReDMCSB MUSIC.C G4099 (line 6) is indexed in F0743 at lines 632-646.
     * Bind that exact 10*32*32 payload from the already authenticated F31
     * executable, rather than recreating a coordinate-to-music table. */
    if (!csb_v1_fmtowns_game_read_span(out_receipt->executable_path,
                                       music_table_offset, music_table,
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
    if (snprintf(out_receipt->startup_mini_path,
                 sizeof(out_receipt->startup_mini_path), "%s/%s",
                 profile->asset_root, mini_name) >= 0 &&
        strlen(out_receipt->startup_mini_path) <
            sizeof(out_receipt->startup_mini_path)) {
        mini_actual_hash = csb_v1_fmtowns_game_file_fnv1a(
            out_receipt->startup_mini_path, &mini_actual_size);
        out_receipt->startup_mini_size = mini_actual_size;
        out_receipt->startup_mini_fnv1a = mini_actual_hash;
        out_receipt->startup_mini_verified =
            mini_actual_size == mini_expected_size &&
            mini_actual_hash == mini_expected_hash;
    }
    out_receipt->music_table_verified = 1;
    out_receipt->music_table_source_offset = music_table_offset;
    out_receipt->music_table_size = sizeof(music_table);
    out_receipt->music_table_fnv1a = CSB_V1_FMTOWNS_GAME_MUSIC_TABLE_FNV1A;
    out_receipt->source_evidence =
        "ReDMCSB COMPILE.H EXEID 60/61 lines 367-375; "
        "STARTUP1.C F0435 line 163; CEDTDATA.C G2297 lines 380-387; "
        "ENTRANCE.C F0807 line 85; "
        "MUSIC.C G4099 line 6/F0743 lines 632-646";
    out_receipt->valid = 1;
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
        expected_variant = CSB_V1_VARIANT_FMTOWNS_EN;
    } else if (language == CSB_FMTOWNS_SWITCH_JAPANESE) {
        name = "UTILJ.EXP";
        expected_size = CSB_V1_FMTOWNS_UTILJ_SIZE;
        expected_hash = CSB_V1_FMTOWNS_UTILJ_FNV1A;
        expected_variant = CSB_V1_VARIANT_FMTOWNS_JA;
    } else return 0;
    if (profile->variant_id != expected_variant ||
        snprintf(out_receipt->executable_path,
                 sizeof(out_receipt->executable_path), "%s/%s",
                 profile->asset_root, name) < 0 ||
        strlen(out_receipt->executable_path) >=
            sizeof(out_receipt->executable_path)) return 0;
    actual_hash = csb_v1_fmtowns_game_file_fnv1a(
        out_receipt->executable_path, &actual_size);
    if (actual_size != expected_size || actual_hash != expected_hash) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    /* COMPILE.H EXEID 63/64 lines 379-385 identifies these P3 executables
     * as separate C06_CEDT programs. Bind their native entry envelopes before
     * any future TBIOS/CEDT decoder consumes a menu or save command. */
    if (!csb_v1_fmtowns_utility_p3_header_open(out_receipt->executable_path,
                                                actual_size, out_receipt)) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    out_receipt->valid = 1;
    out_receipt->executable_verified = 1;
    out_receipt->language_matches_profile = 1;
    out_receipt->utility_program_is_c06_cedt = 1;
    out_receipt->language = language;
    out_receipt->variant_id = expected_variant;
    out_receipt->executable_size = actual_size;
    out_receipt->executable_fnv1a = actual_hash;
    snprintf(out_receipt->executable_name, sizeof(out_receipt->executable_name),
             "%s", name);
    out_receipt->source_evidence =
        "ReDMCSB SWITCH.C F2279; AUTOEXEC.BAT exits 2/5; "
        "COMPILE.H EXEID 63/64 lines 379-385 C06_CEDT";
    return 1;
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
        !csb_v1_fmtowns_game_read_span(
            handoff.executable_path,
            handoff.p3_load_image_offset + virtual_offset,
            out_receipt->source_bytes, byte_count) ||
        csb_v1_fmtowns_game_bytes_fnv1a(out_receipt->source_bytes,
                                         byte_count) != expected_hash) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    memcpy(out_receipt->label_offsets, offsets,
           sizeof(out_receipt->label_offsets));
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
    if (!csb_v1_fmtowns_game_read_span(receipt->executable_path,
                                       receipt->music_table_source_offset +
                                           table_index,
                                       &track, sizeof(track))) return 0;
    *out_track = track;
    return 1;
}
