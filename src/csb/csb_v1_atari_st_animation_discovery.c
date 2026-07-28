#include "csb_v1_atari_st_animation_discovery.h"
#include "fs_portable_compat.h"

#include <stdio.h>
#include <string.h>

static const char *const g_csb_atari_animation_hashes[] = {
    "4174d6de5384323072b185640ed31723", /* ANIMATE.SCR, Atari ST CSB 2.0 */
    "9f8feb269c959c9fe722ac08f99d9c35", /* ANIMATE.DAT, Atari ST CSB 2.0 */
    NULL
};

static int csb_v1_atari_st_animation_source_identity(
    const char *path, char out[ASSET_PATH_MAX], int *is_virtual)
{
    const char *virtual_separator;
    const char *slash;
    size_t length;

    if (!path || !path[0] || !out || !is_virtual) return 0;
    virtual_separator = strstr(path, "::");
    if (virtual_separator) {
        length = (size_t)(virtual_separator - path);
        if (length == 0u || length >= ASSET_PATH_MAX) return 0;
        memcpy(out, path, length);
        out[length] = '\0';
        *is_virtual = 1;
        return 1;
    }
    slash = strrchr(path, '/');
#ifdef _WIN32
    {
        const char *backslash = strrchr(path, '\\');
        if (!slash || (backslash && backslash > slash)) slash = backslash;
    }
#endif
    if (!slash || slash == path) return 0;
    length = (size_t)(slash - path);
    if (length >= ASSET_PATH_MAX) return 0;
    memcpy(out, path, length);
    out[length] = '\0';
    *is_virtual = 0;
    return 1;
}

int csb_v1_atari_st_animation_discover(
    const char *search_root, CSB_V1_AtariStAnimationDiscoveryReceipt *out)
{
    char paths[2][ASSET_PATH_MAX];
    int matched[2];
    int script_virtual;
    int data_virtual;
    char script_identity[ASSET_PATH_MAX];
    char data_identity[ASSET_PATH_MAX];

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!search_root || !search_root[0]) return 0;
    memset(paths, 0, sizeof(paths));
    memset(matched, 0, sizeof(matched));
    if (asset_find_all_by_md5_list(search_root, g_csb_atari_animation_hashes,
            paths, matched, 2, 8) != 2 || !matched[0] || !matched[1] ||
        !csb_v1_atari_st_animation_source_identity(paths[0], script_identity,
            &script_virtual) ||
        !csb_v1_atari_st_animation_source_identity(paths[1], data_identity,
            &data_virtual) || script_virtual != data_virtual ||
        strcmp(script_identity, data_identity) != 0) return 0;
    snprintf(out->script_path, sizeof(out->script_path), "%s", paths[0]);
    snprintf(out->data_path, sizeof(out->data_path), "%s", paths[1]);
    snprintf(out->source_identity, sizeof(out->source_identity), "%s",
             script_identity);
    out->source_is_virtual = script_virtual;
    out->valid = 1;
    return 1;
}

int csb_v1_atari_st_animation_materialize(
    const CSB_V1_AtariStAnimationDiscoveryReceipt *receipt,
    const char *cache_root, char script_path[ASSET_PATH_MAX],
    char data_path[ASSET_PATH_MAX])
{
    char cache_dir[ASSET_PATH_MAX];
    char cached_script[ASSET_PATH_MAX];
    char cached_data[ASSET_PATH_MAX];

    if (!receipt || !receipt->valid || !script_path || !data_path) return 0;
    script_path[0] = '\0';
    data_path[0] = '\0';
    if (!receipt->source_is_virtual) {
        if (!asset_file_matches_md5(receipt->script_path,
                g_csb_atari_animation_hashes[0]) ||
            !asset_file_matches_md5(receipt->data_path,
                g_csb_atari_animation_hashes[1])) return 0;
        snprintf(script_path, ASSET_PATH_MAX, "%s", receipt->script_path);
        snprintf(data_path, ASSET_PATH_MAX, "%s", receipt->data_path);
        return 1;
    }
    if (!cache_root || !cache_root[0] ||
        !FSP_JoinPath(cache_dir, sizeof(cache_dir), cache_root,
            "csb-atari-animation") ||
        !FSP_CreateDirectoryRecursive(cache_dir) ||
        !FSP_JoinPath(cached_script, sizeof(cached_script), cache_dir,
            "ANIMATE.SCR") ||
        !FSP_JoinPath(cached_data, sizeof(cached_data), cache_dir,
            "ANIMATE.DAT") ||
        !asset_extract_virtual_path(receipt->script_path, cached_script) ||
        !asset_extract_virtual_path(receipt->data_path, cached_data) ||
        !asset_file_matches_md5(cached_script, g_csb_atari_animation_hashes[0]) ||
        !asset_file_matches_md5(cached_data, g_csb_atari_animation_hashes[1])) {
        return 0;
    }
    snprintf(script_path, ASSET_PATH_MAX, "%s", cached_script);
    snprintf(data_path, ASSET_PATH_MAX, "%s", cached_data);
    return 1;
}
