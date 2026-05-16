
#include "firestaff_data_validator.h"
#include "nexus_v1_iso_reader.h"
#ifdef _WIN32
#include <io.h>
#define strcasecmp _stricmp
#else
#include <dirent.h>
#endif
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

static FS_ValidateResult check_file(const char *dir, const char *subdir,
    const char *filename, int min_size, int *actual_size)
{
    char path[512];
    struct stat st;

    snprintf(path, sizeof(path), "%s/%s/%s", dir, subdir, filename);
    if (stat(path, &st) != 0) {
        /* Try uppercase */
        char upper[64];
        int i;
        strncpy(upper, filename, sizeof(upper) - 1);
        for (i = 0; upper[i]; i++) {
            if (upper[i] >= 'a' && upper[i] <= 'z') upper[i] -= 32;
        }
        snprintf(path, sizeof(path), "%s/%s/%s", dir, subdir, upper);
        if (stat(path, &st) != 0) {
            if (actual_size) *actual_size = 0;
            return FS_VALIDATE_MISSING;
        }
    }

    if (actual_size) *actual_size = (int)st.st_size;

    if (st.st_size < min_size)
        return FS_VALIDATE_WRONG_SIZE;

    return FS_VALIDATE_OK;
}

int fs_validate_data_dir(const char *data_dir, FS_ValidationReport *report) {
    int total_ok = 0;
    if (!data_dir || !report) return 0;
    memset(report, 0, sizeof(*report));

    /* DM1 */
    report->dm1[0].file = "GRAPHICS.DAT";
    report->dm1[0].expected_min_size = 100000;
    report->dm1[0].result = check_file(data_dir, "dm1", "GRAPHICS.DAT",
        100000, &report->dm1[0].actual_size);

    report->dm1[1].file = "DUNGEON.DAT";
    report->dm1[1].expected_min_size = 10000;
    report->dm1[1].result = check_file(data_dir, "dm1", "DUNGEON.DAT",
        10000, &report->dm1[1].actual_size);

    report->dm1_ready = (report->dm1[0].result == FS_VALIDATE_OK &&
                         report->dm1[1].result == FS_VALIDATE_OK);
    if (report->dm1_ready) total_ok++;

    /* CSB */
    report->csb[0].file = "GRAPHICS.DAT";
    report->csb[0].expected_min_size = 100000;
    report->csb[0].result = check_file(data_dir, "csb", "GRAPHICS.DAT",
        100000, &report->csb[0].actual_size);

    report->csb[1].file = "DUNGEON.DAT";
    report->csb[1].expected_min_size = 10000;
    report->csb[1].result = check_file(data_dir, "csb", "DUNGEON.DAT",
        10000, &report->csb[1].actual_size);

    report->csb_ready = (report->csb[0].result == FS_VALIDATE_OK &&
                         report->csb[1].result == FS_VALIDATE_OK);
    if (report->csb_ready) total_ok++;

    /* DM2 */
    report->dm2[0].file = "GRAPHICS.DAT";
    report->dm2[0].expected_min_size = 50000;
    report->dm2[0].result = check_file(data_dir, "dm2", "GRAPHICS.DAT",
        50000, &report->dm2[0].actual_size);

    report->dm2[1].file = "DUNGEON.DAT";
    report->dm2[1].expected_min_size = 10000;
    report->dm2[1].result = check_file(data_dir, "dm2", "DUNGEON.DAT",
        10000, &report->dm2[1].actual_size);

    report->dm2_ready = (report->dm2[0].result == FS_VALIDATE_OK &&
                         report->dm2[1].result == FS_VALIDATE_OK);
    if (report->dm2_ready) total_ok++;

    /* Nexus: check for extracted files OR ISO image */
    {
        int nexus_size = 0;
        FS_ValidateResult r = check_file(data_dir, "nexus", "DM.BIN", 100000, &nexus_size);
        if (r == FS_VALIDATE_OK) {
            report->nexus_ready = 1;
        } else {
            /* Try ISO: look for .cue file in nexus/ */
            char nexus_dir[512];
            snprintf(nexus_dir, sizeof(nexus_dir), "%s/nexus", data_dir);
#ifndef _WIN32
            DIR *d = opendir(nexus_dir);
            if (d) {
                struct dirent *ent;
                while ((ent = readdir(d)) != NULL) {
                    int len = (int)strlen(ent->d_name);
                    if (len > 4 && strcasecmp(ent->d_name + len - 4, ".cue") == 0) {
                        char cue_path[512];
                        snprintf(cue_path, sizeof(cue_path), "%s/%s", nexus_dir, ent->d_name);
                        Nexus_ISOReader iso;
                        if (nexus_iso_open_cue(&iso, cue_path) > 0) {
                            report->nexus_ready = nexus_iso_is_nexus(&iso);
                            nexus_iso_close(&iso);
                        }
                        break;
                    }
                }
                closedir(d);
            }
#endif
        }
        if (report->nexus_ready) total_ok++;
    }

    return total_ok;
}

void fs_validate_print_report(const FS_ValidationReport *report) {
    const char *status[] = {"OK", "MISSING", "WRONG SIZE", "CORRUPT"};
    if (!report) return;

    printf("=== Firestaff Data Validation ===\n\n");

    printf("Dungeon Master:        %s\n", report->dm1_ready ? "READY" : "NOT FOUND");
    printf("  GRAPHICS.DAT:        %s", status[report->dm1[0].result]);
    if (report->dm1[0].actual_size > 0) printf(" (%d bytes)", report->dm1[0].actual_size);
    printf("\n");
    printf("  DUNGEON.DAT:         %s", status[report->dm1[1].result]);
    if (report->dm1[1].actual_size > 0) printf(" (%d bytes)", report->dm1[1].actual_size);
    printf("\n\n");

    printf("Chaos Strikes Back:    %s\n", report->csb_ready ? "READY" : "NOT FOUND");
    printf("  GRAPHICS.DAT:        %s\n", status[report->csb[0].result]);
    printf("  DUNGEON.DAT:         %s\n\n", status[report->csb[1].result]);

    printf("Dungeon Master II:     %s\n", report->dm2_ready ? "READY" : "NOT FOUND");
    printf("  GRAPHICS.DAT:        %s\n", status[report->dm2[0].result]);
    printf("  DUNGEON.DAT:         %s\n\n", status[report->dm2[1].result]);

    printf("DM Nexus:              %s\n\n", report->nexus_ready ? "READY" : "NOT FOUND");

    int total = report->dm1_ready + report->csb_ready + report->dm2_ready + report->nexus_ready;
    printf("Games ready: %d/4\n", total);
    if (total == 0) {
        printf("\nNo game data found. See docs/DATA_SETUP.md for instructions.\n");
        printf("Place game files in ~/.firestaff/data/ or use --data DIR\n");
    }
}
