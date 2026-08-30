#include "asset_find_by_hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const char *archive = getenv("FIRESTAFF_CSB_UTILITY_7Z");
    const char *member = "Chaos Strikes Back Utility.stx";
    char virtual_path[ASSET_PATH_MAX * 2];
    uint8_t *bytes = NULL;
    size_t size = 0U;

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
    puts("M12 native CSB 7z virtual read: PASS");
    return 0;
}
