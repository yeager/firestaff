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
    if (hash != 0x00c6756au) {
        fprintf(stderr, "FAIL: I34E selector view checksum %08x\n", hash);
        return 1;
    }
    {
        unsigned char entrance[320u * 200u];
        unsigned char left[105u * 161u];
        unsigned char right[127u * 161u];
        unsigned char frame[320u * 200u];
        unsigned int i;
        for (i = 0u; i < sizeof(entrance); ++i) entrance[i] = 1u;
        for (i = 0u; i < sizeof(left); ++i) left[i] = (unsigned char)(i % 16u);
        for (i = 0u; i < sizeof(right); ++i) right[i] = (unsigned char)(i % 16u);
        if (!DM1_V1_Selector_ComposeFramePc34(
                entrance, 320u, 200u, left, 105u, 161u, right, 127u, 161u,
                pixels, 224u, 136u, 0u, frame, sizeof(frame)) ||
            frame[33u * 320u] != left[0] ||
            frame[33u * 320u + 105u] != right[0] ||
            !DM1_V1_Selector_ComposeFramePc34(
                entrance, 320u, 200u, left, 105u, 161u, right, 127u, 161u,
                pixels, 224u, 136u, 1u, frame, sizeof(frame)) ||
            frame[33u * 320u] != left[4] ||
            frame[33u * 320u + 109u] != right[0]) {
            fprintf(stderr, "FAIL: source F8360 entrance composition\n");
            free(pixels);
            return 1;
        }
    }
    free(pixels);
    puts("PASS: authenticated I34E selector view matches the DOS IMG3 decode");
    return 0;
}
