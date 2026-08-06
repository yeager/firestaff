#include "csb_v1_fmtowns_game.h"

#include <stdio.h>
#include <string.h>

enum {
    CSB_V1_FMTOWNS_CHTWE_SIZE = 283936u,
    CSB_V1_FMTOWNS_CHTWJ_SIZE = 284416u,
    CSB_V1_FMTOWNS_CHTWE_FNV1A = 0x3da136f6u,
    CSB_V1_FMTOWNS_CHTWJ_FNV1A = 0xf937db45u,
    /* The retail F31E/F31J programs carry identical 10*32*32 selector
     * tables. These offsets are from the raw verified executable image. */
    CSB_V1_FMTOWNS_CHTWE_MUSIC_TABLE_OFFSET = 271144u,
    CSB_V1_FMTOWNS_CHTWJ_MUSIC_TABLE_OFFSET = 271624u,
    CSB_V1_FMTOWNS_GAME_MUSIC_TABLE_FNV1A = 0x3faffb70u
};

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
        expected_variant = CSB_V1_VARIANT_FMTOWNS_EN;
    } else if (language == CSB_FMTOWNS_SWITCH_JAPANESE) {
        name = "CHTWJ.EXP";
        expected_size = CSB_V1_FMTOWNS_CHTWJ_SIZE;
        expected_hash = CSB_V1_FMTOWNS_CHTWJ_FNV1A;
        music_table_offset = CSB_V1_FMTOWNS_CHTWJ_MUSIC_TABLE_OFFSET;
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
    out_receipt->music_table_verified = 1;
    out_receipt->music_table_source_offset = music_table_offset;
    out_receipt->music_table_size = sizeof(music_table);
    out_receipt->music_table_fnv1a = CSB_V1_FMTOWNS_GAME_MUSIC_TABLE_FNV1A;
    out_receipt->source_evidence =
        "ReDMCSB COMPILE.H EXEID 60/61 lines 367-375; "
        "STARTUP1.C F0435 line 163; ENTRANCE.C F0807 line 85; "
        "MUSIC.C G4099 line 6/F0743 lines 632-646";
    out_receipt->valid = 1;
    return 1;
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
