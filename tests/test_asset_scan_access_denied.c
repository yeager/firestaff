#include "asset_find_by_hash.h"
#include "menu_startup_m12.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main(void) {
    char root[] = "asset-scan-access-denied.XXXXXX";
    char denied[ASSET_PATH_MAX];
    char home[ASSET_PATH_MAX];
    char configPath[ASSET_PATH_MAX];
    char cachePath[ASSET_PATH_MAX];
    char cacheDir[ASSET_PATH_MAX];
    char firestaffDir[ASSET_PATH_MAX];
    char matchedPaths[1][ASSET_PATH_MAX];
    int matched[1] = {0};
    const char *hashes[] = {"00000000000000000000000000000000", NULL};
    int count;
    M12_StartupMenuState menu;
    M12_StartupMenuInitOptions menuOptions;

    if (geteuid() == 0) {
        fprintf(stderr, "SKIP: root bypasses directory permission bits\n");
        return 77;
    }
    if (!mkdtemp(root)) {
        perror("mkdtemp");
        return 1;
    }
    if (snprintf(denied, sizeof(denied), "%s/unreadable", root) >=
        (int)sizeof(denied) || mkdir(denied, 0700) != 0) {
        perror("mkdir unreadable");
        rmdir(root);
        return 1;
    }
    if (snprintf(home, sizeof(home), "%s/home", root) >= (int)sizeof(home) ||
        mkdir(home, 0700) != 0 ||
        snprintf(configPath, sizeof(configPath), "%s/config.toml", home) >=
            (int)sizeof(configPath) ||
        snprintf(firestaffDir, sizeof(firestaffDir), "%s/.firestaff", home) >=
            (int)sizeof(firestaffDir) ||
        snprintf(cacheDir, sizeof(cacheDir), "%s/cache", firestaffDir) >=
            (int)sizeof(cacheDir) ||
        snprintf(cachePath, sizeof(cachePath), "%s/asset_scan_cache.dat",
                 cacheDir) >= (int)sizeof(cachePath)) {
        fprintf(stderr, "FAIL: could not prepare isolated scanner home\n");
        rmdir(home);
        rmdir(denied);
        rmdir(root);
        return 1;
    }
    if (setenv("HOME", home, 1) != 0 ||
        setenv("FIRESTAFF_CONFIG_PATH", configPath, 1) != 0) {
        perror("setenv isolated test paths");
        rmdir(home);
        rmdir(denied);
        rmdir(root);
        return 1;
    }
    if (chmod(denied, 0000) != 0) {
        perror("chmod unreadable");
        chmod(denied, 0700);
        rmdir(denied);
        rmdir(root);
        return 1;
    }

    asset_scan_clear_access_denied_directories();
    (void)asset_find_all_by_md5_list(root, hashes, matchedPaths, matched, 1, 4);
    count = asset_scan_access_denied_directory_count();
    if (count != 1 || !asset_scan_access_denied_directory_path(0) ||
        strcmp(asset_scan_access_denied_directory_path(0), denied) != 0) {
        fprintf(stderr, "FAIL: denied directory was not recorded (count=%d)\n", count);
        chmod(denied, 0700);
        rmdir(denied);
        rmdir(root);
        return 1;
    }

    asset_scan_clear_access_denied_directories();
    if (asset_scan_access_denied_directory_count() != 0) {
        fprintf(stderr, "FAIL: clear did not reset denied directory diagnostics\n");
        chmod(denied, 0700);
        rmdir(denied);
        rmdir(root);
        return 1;
    }

    memset(&menuOptions, 0, sizeof(menuOptions));
    menuOptions.skipAssetScan = 1;
    menuOptions.skipScreenshotGalleryScan = 1;
    M12_StartupMenu_InitWithOptions(&menu, NULL, NULL, &menuOptions);
    menu.languageExplicit = 1;
    menu.settings.languageIndex = 0;
    if (!M12_StartupMenu_SetDataDirectory(&menu, root)) {
        fprintf(stderr, "FAIL: launcher rejected the accessible selected root\n");
        M12_StartupMenu_Destroy(&menu);
        chmod(denied, 0700);
        rmdir(denied);
        rmdir(root);
        return 1;
    }
    if (menu.view != M12_MENU_VIEW_MESSAGE || !menu.messageLine1 ||
        strcmp(menu.messageLine1, "1 FOLDER REQUIRES ACCESS") != 0 ||
        !menu.messageLine2 || !strstr(menu.messageLine2, denied) ||
        !menu.messageLine3 ||
        strcmp(menu.messageLine3,
               "ALLOW ACCESS IN SYSTEM SETTINGS, THEN RESCAN") != 0) {
        fprintf(stderr, "FAIL: launcher did not explain the denied folder\n");
        M12_StartupMenu_Destroy(&menu);
        chmod(denied, 0700);
        rmdir(denied);
        rmdir(root);
        return 1;
    }
    M12_StartupMenu_Destroy(&menu);

    /* Startup against a selected collection must preserve the permission
     * receipt instead of replacing it with a generic no-data result. */
    asset_scan_clear_access_denied_directories();
    memset(&menuOptions, 0, sizeof(menuOptions));
    menuOptions.skipScreenshotGalleryScan = 1;
    M12_StartupMenu_InitWithOptions(&menu, root, NULL, &menuOptions);
    if (menu.view != M12_MENU_VIEW_MESSAGE || !menu.messageLine2 ||
        !strstr(menu.messageLine2, denied)) {
        fprintf(stderr,
                "FAIL: startup scan did not preserve the denied-folder prompt\n");
        M12_StartupMenu_Destroy(&menu);
        chmod(denied, 0700);
        rmdir(denied);
        rmdir(root);
        return 1;
    }
    M12_StartupMenu_Destroy(&menu);

    if (unlink(cachePath) != 0 && errno != ENOENT) {
        perror("remove scanner cache");
        return 1;
    }
    if ((rmdir(cacheDir) != 0 && errno != ENOENT) ||
        (rmdir(firestaffDir) != 0 && errno != ENOENT) ||
        (unlink(configPath) != 0 && errno != ENOENT) || rmdir(home) != 0 ||
        chmod(denied, 0700) != 0 || rmdir(denied) != 0 || rmdir(root) != 0) {
        perror("cleanup permission fixture");
        return 1;
    }

    puts("PASS: denied directories are recorded and diagnostics can be cleared");
    return 0;
}
