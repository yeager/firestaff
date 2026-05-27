#include "csb_v1_boot.h"

#include "asset_find_by_hash.h"

#include <stdio.h>
#include <string.h>

/* ReDMCSB source-lock for this boot/profile boundary:
 * ENTRANCE.C F0806 lines 409-441 builds the entrance micro-dungeon and
 * selects C28_ENTRANCE_CSB for CSB media.
 * ENTRANCE.C F0806 lines 857-883 waits on the entrance state machine and
 * switches G0298_B_NewGame to C001_MODE_LOAD_DUNGEON.
 * LOADSAVE.C F0435 lines 1940-1944 loads the initial party location from
 * DUNGEON.DAT and sets G0309_i_PartyMapIndex to map 0 for new games.
 */

static const char *const g_csb_boot_graphics_hashes[] = {
    "61fbfd56887c94adc26888a9491c6611",
    "ebf6a57af3f27782e358c0490bfd2f2e",
    "291e1bc6803e3dc4b974c60117ca5d68",
    "cefaddfdf5651df2c91f61b5611a8362",
    NULL
};

static const CSB_V1_VariantId g_csb_boot_graphics_variants[] = {
    CSB_V1_VARIANT_PC34_EN,
    CSB_V1_VARIANT_ST21_EN,
    CSB_V1_VARIANT_AMIGA35_EN,
    CSB_V1_VARIANT_AMIGA35_MULTI
};

static const char *const g_csb_boot_dungeon_hashes[] = {
    "6695d2acebce49f95db1d8f3a5c733de",
    NULL
};

static void csb_v1_boot_copy(char *dst, size_t dst_size, const char *src)
{
    if (!dst || dst_size == 0U) return;
    if (!src) src = "";
    snprintf(dst, dst_size, "%s", src);
}

static CSB_V1_AssetGfxArchiveType csb_v1_boot_graphics_kind(const char *path)
{
    const char *name;
    if (!path) return CSB_V1_ASSET_GFX_ARCHIVE_NONE;
    name = strrchr(path, '/');
#if defined(_WIN32)
    {
        const char *slash = strrchr(path, '\\');
        if (slash && (!name || slash > name)) name = slash;
    }
#endif
    name = name ? name + 1 : path;
    if (strcmp(name, "CSB.DAT") == 0 || strcmp(name, "csb.dat") == 0) {
        return CSB_V1_ASSET_GFX_ARCHIVE_CSB;
    }
    if (strcmp(name, "CSBGRAPH.DAT") == 0 || strcmp(name, "csbgraph.dat") == 0) {
        return CSB_V1_ASSET_GFX_ARCHIVE_CSBGRAF;
    }
    return CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS;
}

void csb_v1_boot_profile_init(CSB_V1_BootProfile *profile)
{
    if (!profile) return;
    memset(profile, 0, sizeof(*profile));
    csb_v1_boot_copy(profile->game_id, sizeof(profile->game_id), CSB_V1_BOOT_GAME_ID);
    profile->state = CSB_V1_BOOT_STATE_PROFILE_READY;
    profile->variant_id = CSB_V1_VARIANT_UNKNOWN;
    csb_v1_boot_copy(profile->version_id, sizeof(profile->version_id), "unknown");
    csb_v1_boot_copy(profile->variant_label, sizeof(profile->variant_label), "Unknown");
    profile->tick_ms = CSB_V1_TICK_MS_NOMINAL;
    profile->entrance_map_index = 255U;
    profile->start_map_index = 0U;
    profile->default_party_x = CSB_V1_START_PARTY_X;
    profile->default_party_y = CSB_V1_START_PARTY_Y;
    profile->default_party_dir = CSB_V1_START_PARTY_DIR;
    csb_v1_runtime_init(&profile->runtime, NULL);
}

void csb_v1_boot_set_save_root(CSB_V1_BootProfile *profile, const char *save_dir)
{
    if (!profile) return;
    if (save_dir && save_dir[0] != '\0') {
        csb_v1_boot_copy(profile->save_root, sizeof(profile->save_root), save_dir);
    } else if (profile->asset_root[0] != '\0') {
        snprintf(profile->save_root, sizeof(profile->save_root),
                 "%s/../%s", profile->asset_root, CSB_V1_BOOT_SAVE_SUBDIR);
    } else {
        csb_v1_boot_copy(profile->save_root, sizeof(profile->save_root),
                         csb_v1_runtime_save_dir());
    }
}

int csb_v1_boot_scan_assets(CSB_V1_BootProfile *profile, const char *data_dir)
{
    char graphics_path[ASSET_PATH_MAX];
    char dungeon_path[ASSET_PATH_MAX];
    int graphics_match = -1;
    int dungeon_match = -1;
    const CSB_V1_VariantInfo *variant;
    const char *root;

    if (!profile) return -1;
    root = (data_dir && data_dir[0] != '\0') ? data_dir : ".";
    csb_v1_boot_copy(profile->asset_root, sizeof(profile->asset_root), root);

    profile->graphics_verified =
        asset_find_by_md5_list(root, g_csb_boot_graphics_hashes,
                               graphics_path, sizeof(graphics_path),
                               &graphics_match, 4);
    profile->dungeon_verified =
        asset_find_by_md5_list(root, g_csb_boot_dungeon_hashes,
                               dungeon_path, sizeof(dungeon_path),
                               &dungeon_match, 4);
    if (profile->graphics_verified) {
        csb_v1_boot_copy(profile->graphics_path, sizeof(profile->graphics_path),
                         graphics_path);
        csb_v1_boot_copy(profile->graphics_md5, sizeof(profile->graphics_md5),
                         g_csb_boot_graphics_hashes[graphics_match]);
        profile->graphics_kind = csb_v1_boot_graphics_kind(graphics_path);
        profile->variant_id = g_csb_boot_graphics_variants[graphics_match];
    }
    if (profile->dungeon_verified) {
        csb_v1_boot_copy(profile->dungeon_path, sizeof(profile->dungeon_path),
                         dungeon_path);
        csb_v1_boot_copy(profile->dungeon_md5, sizeof(profile->dungeon_md5),
                         g_csb_boot_dungeon_hashes[dungeon_match]);
    }

    profile->assets_verified = profile->graphics_verified && profile->dungeon_verified;
    if (profile->variant_id == CSB_V1_VARIANT_UNKNOWN && profile->dungeon_verified) {
        profile->variant_id = CSB_V1_VARIANT_UNKNOWN;
    }
    variant = csb_v1_runtime_get_variant_info(profile->variant_id);
    csb_v1_boot_copy(profile->variant_label, sizeof(profile->variant_label),
                     variant->name);
    csb_v1_boot_copy(profile->media_ref, sizeof(profile->media_ref),
                     variant->media_ref);
    csb_v1_boot_copy(profile->version_id, sizeof(profile->version_id),
                     profile->graphics_md5[0] ? profile->graphics_md5 : "unknown");
    if (profile->save_root[0] == '\0') {
        csb_v1_boot_set_save_root(profile, NULL);
    }
    if (profile->assets_verified) {
        profile->state = CSB_V1_BOOT_STATE_ASSETS_READY;
        return 0;
    }
    profile->state = CSB_V1_BOOT_STATE_PROFILE_READY;
    return -1;
}

int csb_v1_boot_probe_available(const char *data_dir)
{
    CSB_V1_BootProfile profile;
    csb_v1_boot_profile_init(&profile);
    return csb_v1_boot_scan_assets(&profile, data_dir) == 0 ? 1 : 0;
}

int csb_v1_boot_enter_game(CSB_V1_BootProfile *profile)
{
    if (!profile || !profile->assets_verified) return -1;
    csb_v1_runtime_init(&profile->runtime, profile->asset_root);
    profile->runtime.variant_id = profile->variant_id;
    profile->runtime.difficulty = CSB_V1_DIFFICULTY_HARD;
    profile->runtime.dungeon_path = profile->dungeon_path;
    profile->runtime.graphics_path = profile->graphics_path;
    profile->runtime.dungeon_asset.path = profile->dungeon_path;
    profile->runtime.graphics_asset.path = profile->graphics_path;
    profile->runtime.graphics_asset.kind = profile->graphics_kind;
    profile->runtime.state = CSB_STATE_TITLE;
    profile->runtime.chaos_magic.magic_initialized = 1;
    profile->runtime.chaos_magic.spell_grid_version = 0U;
    profile->runtime.chaos_magic.chaos_level = 0U;
    profile->state = CSB_V1_BOOT_STATE_RUNTIME_READY;
    return 0;
}

void csb_v1_boot_cleanup(CSB_V1_BootProfile *profile)
{
    if (!profile) return;
    profile->state = CSB_V1_BOOT_STATE_PROFILE_READY;
    memset(&profile->runtime, 0, sizeof(profile->runtime));
}

size_t csb_v1_boot_diagnostic_report(const CSB_V1_BootProfile *profile,
                                     char *buf,
                                     size_t buf_size)
{
    int n;
    if (!profile || !buf || buf_size == 0U) return 0U;
    n = snprintf(buf, buf_size,
                 "=== CSB V1 Boot Profile ===\n"
                 "state=%d verified=%s variant=%s media=%s\n"
                 "asset_root=%s\n"
                 "graphics=%s md5=%s\n"
                 "dungeon=%s md5=%s\n"
                 "save_root=%s tick_ms=%u entrance_map=%u start_map=%u\n",
                 (int)profile->state,
                 profile->assets_verified ? "YES" : "NO",
                 profile->variant_label,
                 profile->media_ref,
                 profile->asset_root[0] ? profile->asset_root : "(unset)",
                 profile->graphics_path[0] ? profile->graphics_path : "(missing)",
                 profile->graphics_md5[0] ? profile->graphics_md5 : "(missing)",
                 profile->dungeon_path[0] ? profile->dungeon_path : "(missing)",
                 profile->dungeon_md5[0] ? profile->dungeon_md5 : "(missing)",
                 profile->save_root[0] ? profile->save_root : "(unset)",
                 (unsigned)profile->tick_ms,
                 (unsigned)profile->entrance_map_index,
                 (unsigned)profile->start_map_index);
    if (n < 0) return 0U;
    return (size_t)n < buf_size ? (size_t)n : buf_size - 1U;
}

void csb_v1_boot_print_summary(const CSB_V1_BootProfile *profile)
{
    if (!profile) {
        printf("CSB: no boot profile\n");
        return;
    }
    printf("CSB: %s assets=%s dungeon=%s graphics=%s\n",
           profile->variant_label,
           profile->assets_verified ? "READY" : "missing",
           profile->dungeon_verified ? "ok" : "missing",
           profile->graphics_verified ? "ok" : "missing");
}

const char *csb_v1_boot_source_evidence(void)
{
    return
        "ReDMCSB ENTRANCE.C F0806 lines 409-441: CSB entrance setup and C28_ENTRANCE_CSB palette\n"
        "ReDMCSB ENTRANCE.C F0806 lines 857-883: entrance waits then switches G0298_B_NewGame\n"
        "ReDMCSB LOADSAVE.C F0435 lines 1940-1944: new-game party location and map 0\n"
        "ReDMCSB BASE.C lines 36-39: G0298_B_NewGame boot/load mode storage\n";
}
