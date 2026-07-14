#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f0720_shrinkblt_sub1_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    const uint8_t source[] = { 0xa1, 0xb2, 0xc3, 0xd4 };
    uint8_t destination[] = { 0xee, 0xee, 0xee, 0xee };

    /* The PC 3.4 loop samples source pixels 1, 3, 4, and 6 here. */
    redmcsb_f0720_shrinkblt_sub1_pc34_compat(
        source, destination, 1, 2, 96, 4);
    assert(destination[0] == 0xee);
    assert(destination[1] == 0x12);
    assert(destination[2] == 0xcd);
    assert(destination[3] == 0xee);

    /* F0720 writes a complete packed byte even when its supplied width is odd. */
    memset(destination, 0xee, sizeof(destination));
    redmcsb_f0720_shrinkblt_sub1_pc34_compat(
        source, destination, 0, 0, 64, 1);
    assert(destination[0] == 0xa1);
    assert(destination[1] == 0xee);

    assert(strstr(redmcsb_f0720_shrinkblt_sub1_source_evidence_pc34(),
                  "BLTSHRNK.C F0720_ShrinkBLT_Sub1") != NULL);

    puts("ok: ReDMCSB F0720 PC 3.4 packed shrink inner loop");
    return 0;
}
