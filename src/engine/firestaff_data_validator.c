
#include "firestaff_data_validator.h"
#include "asset_status_m12.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

static int validator_file_size(const char *path) {
    struct stat st;
    if (!path || path[0] == '\0' || strstr(path, "::") != NULL ||
        stat(path, &st) != 0 || st.st_size < 0) {
        return 0;
    }
    return (int)st.st_size;
}

static void validator_fill_required_entry(const M12_AssetStatus *status,
                                          const char *gameId,
                                          size_t index,
                                          FS_ValidateEntry *entry,
                                          const char *fallbackLabel) {
    const M12_AssetRequiredFileStatus *required;
    if (!entry) {
        return;
    }
    memset(entry, 0, sizeof(*entry));
    entry->file = fallbackLabel;
    entry->result = FS_VALIDATE_MISSING;

    required = M12_AssetStatus_GetRequiredFile(status, gameId, index);
    if (!required) {
        return;
    }
    entry->file = required->label ? required->label : fallbackLabel;
    if (required->matched) {
        entry->result = FS_VALIDATE_OK;
        entry->actual_size = validator_file_size(required->matchedPath);
    }
}

int fs_validate_data_dir(const char *data_dir, FS_ValidationReport *report) {
    int total_ok = 0;
    M12_AssetStatus status;
    if (!data_dir || !report) return 0;
    memset(report, 0, sizeof(*report));

    M12_AssetStatus_Scan(&status, data_dir);

    validator_fill_required_entry(&status, "dm1", 0U, &report->dm1[0], "GRAPHICS.DAT");
    validator_fill_required_entry(&status, "dm1", 1U, &report->dm1[1], "DUNGEON.DAT");
    report->dm1_ready = status.dm1Available ? 1 : 0;
    if (report->dm1_ready) total_ok++;

    validator_fill_required_entry(&status, "csb", 0U, &report->csb[0], "GRAPHICS.DAT");
    validator_fill_required_entry(&status, "csb", 1U, &report->csb[1], "DUNGEON.DAT");
    report->csb_ready = status.csbAvailable ? 1 : 0;
    if (report->csb_ready) total_ok++;

    validator_fill_required_entry(&status, "dm2", 0U, &report->dm2[0], "GRAPHICS.DAT");
    validator_fill_required_entry(&status, "dm2", 1U, &report->dm2[1], "DUNGEON.DAT");
    report->dm2_ready = status.dm2Available ? 1 : 0;
    if (report->dm2_ready) total_ok++;

    report->nexus_ready = status.nexusAvailable ? 1 : 0;
    if (report->nexus_ready) total_ok++;

    report->theron_ready = status.theronAvailable ? 1 : 0;
    if (report->theron_ready) total_ok++;

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

    printf("Theron's Quest:        %s\n\n",
           report->theron_ready ? "READY" : "NOT FOUND");

    int total = report->dm1_ready + report->csb_ready + report->dm2_ready
              + report->nexus_ready + report->theron_ready;
    printf("Games ready: %d/5\n", total);
    if (total == 0) {
        printf("\nNo game data found. See docs/DATA_SETUP.md for instructions.\n");
        printf("Place game files in ~/.firestaff/data/ or use --data DIR\n");
    }
}
