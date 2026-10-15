/* Production-linked directory cancellation with authenticated original media. */
#define _DARWIN_C_SOURCE 1
#define _DEFAULT_SOURCE 1
#define _XOPEN_SOURCE 700
#define _POSIX_C_SOURCE 200809L
#include "asset_status_m12.h"
#include "dm2_v1_boot.h"
#include "fs_portable_compat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct CancelProbe {
    unsigned entry_callbacks;
    int reached;
} CancelProbe;

static int progress(const M12_AssetScanProgress *p, void *opaque) {
    CancelProbe *probe = opaque;
    if (strcmp(p->currentGameId, "dm2") == 0 &&
        strcmp(p->currentTask, "enumerating ZIP candidates") == 0 &&
        strstr(p->currentPath, "/Cancel-entry-") != NULL) {
        if (++probe->entry_callbacks == 3u) {
            probe->reached = 1;
            return 0;
        }
    }
    return 1;
}

static int marker_path(char *out, size_t capacity, const char *directory,
                        unsigned index) {
    char name[40];
    int n = snprintf(name, sizeof(name), "Cancel-entry-%03u.txt", index);
    return n >= 0 && (size_t)n < sizeof(name) &&
        FSP_JoinPath(out, capacity, directory, name);
}

int main(int argc, char **argv) {
    char before[33], after[33], root[4096] = {0};
    char directory[4096] = {0}, path[4096];
    char original[4096];
    const char *scratch = getenv("FIRESTAFF_TEST_SCRATCH");
    M12_AssetStatus *status = calloc(1u, sizeof(*status));
    DM2_V1_BootProfile *profile = calloc(1u, sizeof(*profile));
    int success = 0;
    if (argc != 2 || access(argv[1], R_OK) != 0) {
        free(profile);
        free(status);
        return 77;
    }
    if (!status || !profile || !realpath(argv[1], original) ||
        !m12_file_md5_hex(original, before)) goto done;
    dm2_v1_boot_profile_init(profile);
    if (dm2_v1_boot_scan_assets(profile, original) != 0 ||
        !profile->assets_verified || profile->platform != DM2_PLATFORM_PC_EN ||
        strcmp(profile->graphics_md5, "25247ede4dabb6a71e5dabdfbcd5907d") != 0 ||
        !dm2_v1_boot_asset_hash_pair_supported(profile->graphics_md5,
                                               profile->dungeon_md5)) goto done;
    dm2_v1_boot_cleanup(profile);
    if (!scratch || !*scratch) scratch = ".codex-scratch";
    if (mkdir(scratch, 0700) != 0 && access(scratch, F_OK) != 0) goto done;
    if (!FSP_JoinPath(root, sizeof(root), scratch,
                      "dm2-directory-cancel.XXXXXX") || !mkdtemp(root)) goto done;
    if (!FSP_JoinPath(directory, sizeof(directory), root, "dm2") ||
        mkdir(directory, 0700) != 0) goto cleanup;
    /* Non-game markers force multiple directory chunks; these are transport
     * geometry, never substitute graphics or dungeon data. */
    for (unsigned i = 0u; i < 128u; ++i) {
        if (!marker_path(path, sizeof(path), directory, i)) goto cleanup;
        FILE *marker = fopen(path, "wbx");
        if (!marker || fclose(marker) != 0) goto cleanup;
    }
    if (!FSP_JoinPath(path, sizeof(path), directory, "Original.ZIP") ||
        symlink(original, path) != 0) goto cleanup;
    for (int scoped = 0; scoped < 2; ++scoped) {
        CancelProbe probe = {0u, 0};
        M12_AssetStatusScanOptions options = {0};
        options.honorRequestedDataDir = 1;
        options.progressFn = progress;
        options.progressUserData = &probe;
        memset(status, 0, sizeof(*status));
        int rc = 0;
        if (scoped)
            M12_AssetStatus_ScanGameWithOptions(status, root, "dm2", &options);
        else
            rc = M12_AssetStatus_ScanWithOptions(status, root, &options);
        if (!probe.reached || probe.entry_callbacks != 3u || rc != 0 ||
            !status->scanProgress.cancelled || status->scanProgress.complete ||
            status->dm2Available) goto cleanup;
        for (size_t g = 0u; g < M12_ASSET_GAME_COUNT; ++g)
            for (size_t v = 0u; v < M12_ASSET_MAX_VERSIONS_PER_GAME; ++v)
                if (status->versions[g][v].matched) goto cleanup;
    }
    success = m12_file_md5_hex(original, after) && strcmp(before, after) == 0;
cleanup:
    if (FSP_JoinPath(path, sizeof(path), directory, "Original.ZIP")) unlink(path);
    for (unsigned i = 0u; i < 128u; ++i)
        if (marker_path(path, sizeof(path), directory, i)) unlink(path);
    rmdir(directory);
    rmdir(root);
done:
    if (profile) dm2_v1_boot_cleanup(profile);
    free(profile);
    free(status);
    puts(success ? "PASS: scoped/unscoped mid-directory cancellation rolls back publication; original unchanged"
                 : "FAIL: DM2 mid-directory cancellation boundary");
    return success ? 0 : 1;
}
