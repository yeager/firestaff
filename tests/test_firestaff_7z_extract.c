#include "firestaff_7z_extract.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const char *path = getenv("FIRESTAFF_CSB_UTILITY_7Z");
    FILE *archive;
    uint8_t *bytes = NULL;
    size_t size = 0u;
    char name[64];
    if (!path || !path[0]) {
        puts("SKIP: FIRESTAFF_CSB_UTILITY_7Z is not configured");
        return 77;
    }
    archive = fopen(path, "rb");
    if (!archive) {
        puts("SKIP: original one-member CSB utility 7z is not staged");
        return 77;
    }
    fclose(archive);
    if (!firestaff_7z_extract_single_lzma2_file(path, &bytes, &size,
                                                 name, sizeof(name)) ||
        !bytes || size != 411568u || bytes[0] != 'R' || bytes[1] != 'S' ||
        strcmp(name, "Chaos Strikes Back Utility.stx") != 0) {
        free(bytes);
        fputs("FAIL: native CSB LZMA2 7z extraction\n", stderr);
        return 1;
    }
    free(bytes);
    puts("native CSB LZMA2 7z extraction: PASS");
    return 0;
}
