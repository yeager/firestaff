#include "dm1_v1_selector_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>

int main(void) {
    const char* path = getenv("FIRESTAFF_DM1_I34E_SELECTOR");
    unsigned char* pixels = NULL;
    unsigned int count = 0u;
    unsigned int i;
    unsigned int hash = 2166136261u;
    if (!path || !path[0]) {
        puts("SKIP: FIRESTAFF_DM1_I34E_SELECTOR is not configured");
        return 0;
    }
    if (!DM1_V1_Selector_ExtractDungeonViewPc34(path, &pixels, &count) ||
        count != 224u * 136u) {
        fprintf(stderr, "FAIL: could not extract authenticated I34E selector view\n");
        free(pixels);
        return 1;
    }
    for (i = 0u; i < count; ++i) hash = (hash ^ pixels[i]) * 16777619u;
    free(pixels);
    if (hash != 0x00c6756au) {
        fprintf(stderr, "FAIL: I34E selector view checksum %08x\n", hash);
        return 1;
    }
    puts("PASS: authenticated I34E selector view matches the DOS IMG3 decode");
    return 0;
}
