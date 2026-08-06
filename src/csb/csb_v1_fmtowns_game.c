#include "csb_v1_fmtowns_game.h"

#include <stdio.h>
#include <string.h>

enum {
    CSB_V1_FMTOWNS_CHTWE_SIZE = 283936u,
    CSB_V1_FMTOWNS_CHTWJ_SIZE = 284416u,
    CSB_V1_FMTOWNS_CHTWE_FNV1A = 0x3da136f6u,
    CSB_V1_FMTOWNS_CHTWJ_FNV1A = 0xf937db45u
};

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
    CSB_V1_VariantId expected_variant;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!profile || !profile->assets_verified || !profile->graphics_verified ||
        !profile->dungeon_verified || !profile->asset_root[0]) return 0;

    if (language == CSB_FMTOWNS_SWITCH_ENGLISH) {
        name = "CHTWE.EXP";
        expected_size = CSB_V1_FMTOWNS_CHTWE_SIZE;
        expected_hash = CSB_V1_FMTOWNS_CHTWE_FNV1A;
        expected_variant = CSB_V1_VARIANT_FMTOWNS_EN;
    } else if (language == CSB_FMTOWNS_SWITCH_JAPANESE) {
        name = "CHTWJ.EXP";
        expected_size = CSB_V1_FMTOWNS_CHTWJ_SIZE;
        expected_hash = CSB_V1_FMTOWNS_CHTWJ_FNV1A;
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
    out_receipt->source_evidence =
        "ReDMCSB COMPILE.H EXEID 60/61 lines 367-375; "
        "STARTUP1.C F0435 line 163; ENTRANCE.C F0807 line 85";
    out_receipt->valid = 1;
    return 1;
}
