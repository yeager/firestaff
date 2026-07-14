#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1004_video_blit_shrink_with_palette_changes_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    static const uint8_t identity_source[] = {
        UINT8_C(0x12), UINT8_C(0x34), UINT8_C(0x56),
        UINT8_C(0x78), UINT8_C(0x9a), UINT8_C(0xbc)
    };
    static const uint8_t shrink_source[] = {
        UINT8_C(0x01), UINT8_C(0x23), UINT8_C(0x45),
        UINT8_C(0x67), UINT8_C(0x89), UINT8_C(0xab)
    };
    static const uint8_t palette_changes[16] = {
        UINT8_C(0x0f), UINT8_C(0x0e), UINT8_C(0x0d), UINT8_C(0x0c),
        UINT8_C(0x0b), UINT8_C(0x0a), UINT8_C(0x09), UINT8_C(0x08),
        UINT8_C(0x07), UINT8_C(0x06), UINT8_C(0x05), UINT8_C(0x04),
        UINT8_C(0x03), UINT8_C(0x02), UINT8_C(0x01), UINT8_C(0x00)
    };
    uint8_t destination[6];

    memset(destination, 0, sizeof(destination));
    F1004_VIDEO_BlitShrinkWithPaletteChanges_PC34(
        identity_source, destination, 6u, 2u, 6, 2, NULL);
    assert(memcmp(destination, identity_source, sizeof(destination)) == 0);

    memset(destination, 0, sizeof(destination));
    F1004_VIDEO_BlitShrinkWithPaletteChanges_PC34(
        shrink_source, destination, 6u, 2u, 2, 2, palette_changes);
    assert(destination[0] == UINT8_C(0xeb));
    assert(destination[1] == UINT8_C(0x85));

    assert(strstr(
               redmcsb_f1004_video_blit_shrink_with_palette_changes_source_evidence(),
               "BLTSHRNK.C:1556-1595") != NULL);
    assert(strstr(
               redmcsb_f1004_video_blit_shrink_with_palette_changes_source_evidence(),
               "not divided by 10") != NULL);

    puts("ok: ReDMCSB F1004 packed 4bpp shrink and literal palette remap");
    return 0;
}
