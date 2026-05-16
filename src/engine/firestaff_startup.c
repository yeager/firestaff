
#include "firestaff_data_validator.h"
#include "firestaff_known_hashes.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>

/* Platform-specific data directory */
static const char *fs_get_default_data_dir(void) {
#ifdef _WIN32
    static char buf[512];
    const char *appdata = getenv("APPDATA");
    if (appdata) {
        snprintf(buf, sizeof(buf), "%s\\Firestaff\\data", appdata);
        return buf;
    }
    return "C:\\Firestaff\\data";
#else
    static char buf[512];
    const char *home = getenv("HOME");
    if (home) {
        snprintf(buf, sizeof(buf), "%s/.firestaff/data", home);
        return buf;
    }
    return "/tmp/firestaff/data";
#endif
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
    const char *subdirs[] = {"dm1", "csb", "dm2", "dm1-multilingual", "nexus", NULL};
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
                fprintf(f, "Run: firestaff --validate\n");
                fclose(f);
            }
        } else {
            fclose(f);
        }
    }
}

/* Validate and return which games are available */
typedef struct {
    int dm1_available;
    int csb_available;
    int dm2_available;
    int nexus_available;
    const char *data_dir;
} FS_GameAvailability;

void fs_startup_check_games(const char *data_dir, FS_GameAvailability *avail) {
    FS_ValidationReport report;
    if (!avail) return;
    memset(avail, 0, sizeof(*avail));

    if (!data_dir) data_dir = fs_get_default_data_dir();
    avail->data_dir = data_dir;

    fs_startup_ensure_data_dirs(data_dir);

    if (fs_validate_data_dir(data_dir, &report) >= 0) {
        avail->dm1_available = report.dm1_ready;
        avail->csb_available = report.csb_ready;
        avail->dm2_available = report.dm2_ready;
        avail->nexus_available = report.nexus_ready;
    }

    printf("Game data: DM1=%s CSB=%s DM2=%s Nexus=%s\n",
        avail->dm1_available ? "YES" : "no",
        avail->csb_available ? "YES" : "no",
        avail->dm2_available ? "YES" : "no",
        avail->nexus_available ? "YES" : "no");
}
