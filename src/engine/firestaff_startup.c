
#include "asset_status_m12.h"
#include "firestaff_startup.h"
#include "fs_portable_compat.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>

static const char *fs_get_default_data_dir(void) {
    static char buf[FSP_PATH_MAX];
    if (FSP_ResolveDataDir(buf, sizeof(buf), NULL)) {
        return buf;
    }
    return "data";
}

static void fs_mkdir_p(const char *path) {
#ifdef _WIN32
    char cmd[600];
    snprintf(cmd, sizeof(cmd), "mkdir \"%s\" 2>nul", path);
    system(cmd);
#else
    char cmd[600];
    snprintf(cmd, sizeof(cmd), "mkdir -p '%s' 2>/dev/null", path);
    system(cmd);
#endif
}

/* Create data directories if they don't exist */
void fs_startup_ensure_data_dirs(const char *base_dir) {
    const char *subdirs[] = {"dm1", "csb", "dm2", "dm1-multilingual", "nexus", "theron", NULL};

    int i;
    char path[512];

    if (!base_dir) base_dir = fs_get_default_data_dir();

    fs_mkdir_p(base_dir);
    for (i = 0; subdirs[i]; i++) {
        snprintf(path, sizeof(path), "%s/%s", base_dir, subdirs[i]);
        fs_mkdir_p(path);
    }

    /* Write README if missing */
    snprintf(path, sizeof(path), "%s/README.txt", base_dir);
    {
        FILE *f = fopen(path, "r");
        if (!f) {
            f = fopen(path, "w");
            if (f) {
                fprintf(f, "Firestaff Game Data\n");
                fprintf(f, "Place original game files in subdirectories:\n");
                fprintf(f, "  dm1/    - Dungeon Master (GRAPHICS.DAT + DUNGEON.DAT)\n");
                fprintf(f, "  csb/    - Chaos Strikes Back\n");
                fprintf(f, "  dm2/    - Dungeon Master II\n");
                fprintf(f, "  nexus/  - DM Nexus (extracted Saturn ISO)\n");
                fprintf(f, "  theron/ - Theron's Quest (PC Engine HuCard)\n");

                fprintf(f, "Run: firestaff --validate\n");
                fclose(f);
            }
        } else {
            fclose(f);
        }
    }
}

void fs_startup_check_games(const char *data_dir, FS_GameAvailability *avail) {
    M12_AssetStatus status;
    if (!avail) return;
    memset(avail, 0, sizeof(*avail));

    if (!data_dir) data_dir = fs_get_default_data_dir();
    avail->data_dir = data_dir;

    fs_startup_ensure_data_dirs(data_dir);

    M12_AssetStatus_Scan(&status, data_dir);
    avail->dm1_available = M12_AssetStatus_GameAvailable(&status, "dm1");
    avail->csb_available = M12_AssetStatus_GameAvailable(&status, "csb");
    avail->dm2_available = M12_AssetStatus_GameAvailable(&status, "dm2");
    avail->nexus_available = M12_AssetStatus_GameAvailable(&status, "nexus");
    avail->theron_available = M12_AssetStatus_GameAvailable(&status, "theron");
    avail->data_dir = M12_AssetStatus_GetDataDir(&status);

    printf("Game data: DM1=%s CSB=%s DM2=%s Nexus=%s Theron=%s\n",
        avail->dm1_available ? "YES" : "no",
        avail->csb_available ? "YES" : "no",
        avail->dm2_available ? "YES" : "no",
        avail->nexus_available ? "YES" : "no",
        avail->theron_available ? "YES" : "no");
}
