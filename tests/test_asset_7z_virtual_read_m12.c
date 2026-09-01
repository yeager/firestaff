#include "asset_find_by_hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const char *archive = getenv("FIRESTAFF_CSB_UTILITY_7Z");
    const char *member = "Chaos Strikes Back Utility.stx";
    const char *nonmatching_md5[] = {
        "00000000000000000000000000000000", NULL
    };
    char virtual_path[ASSET_PATH_MAX * 2];
    char paths[1][ASSET_PATH_MAX];
    uint8_t *bytes = NULL;
    size_t size = 0U;
    int matched[1] = {0};

    if (!archive || !archive[0]) {
        puts("SKIP: FIRESTAFF_CSB_UTILITY_7Z is not configured");
        return 77;
    }
    if (snprintf(virtual_path, sizeof(virtual_path), "%s::%s", archive,
                 member) < 0 ||
        !asset_read_path_alloc(virtual_path, &bytes, &size) ||
        !bytes || size != 411568U || bytes[0] != 'R' || bytes[1] != 'S') {
        free(bytes);
        fputs("FAIL: M12 cannot read the native CSB 7z member in memory\n",
              stderr);
        return 1;
    }
    free(bytes);
    if (snprintf(virtual_path, sizeof(virtual_path), "%s::%s::START.PRG",
                 archive, member) < 0 ||
        !asset_read_virtual_path_alloc(virtual_path, &bytes, &size) ||
        !bytes || size < 2U || bytes[0] != 0x60U) {
        free(bytes);
        fputs("FAIL: M12 cannot read nested CSB 7z/STX data in memory\n",
              stderr);
        return 1;
    }
    free(bytes);
    /* A supported native archive with no hash from the current game must not
     * be reported as requiring a host 7z program.  This happens during a
     * broad multi-game scan, where CSB's Utility Disk is inspected beside
     * other games' asset lists. */
    asset_scan_clear_missing_extractor_diagnostics();
    memset(paths, 0, sizeof(paths));
    if (asset_find_all_by_md5_list(archive, nonmatching_md5, paths, matched,
                                   1, 0) != 0 || matched[0] ||
        asset_scan_missing_extractor_count() != 0) {
        fputs("FAIL: supported native CSB 7z was marked as external\n", stderr);
        return 1;
    }
    puts("M12 native CSB 7z virtual read: PASS");
    return 0;
}
