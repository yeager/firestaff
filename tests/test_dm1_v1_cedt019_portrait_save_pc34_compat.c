#include "dm1_v1_cedt019_portrait_save_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_constants(void)
{
    int count = DM1_V1_CEDT019_PORTRAIT_COUNT_PC34;
    int planar = DM1_V1_CEDT019_PORTRAIT_PLANAR_BYTES_PC34;
    int chunky = DM1_V1_CEDT019_PORTRAIT_CHUNKY_BYTES_PC34;
    (void)count;
    (void)planar;
    (void)chunky;
    assert(count == 4);
    assert(planar == DM1_PORTRAIT_PLANAR_BYTES);
    assert(chunky == DM1_PORTRAIT_CHUNKY_BYTES);
}

static void test_decode_empty(void)
{
    DM1_V1_CEDT019_PortraitSlotPc34 portraits[4];
    DM1_V1_CEDT019_PortraitBatchReceiptPc34 out;
    int rc;

    memset(portraits, 0, sizeof(portraits));
    memset(&out, 0, sizeof(out));
    rc = F2122_DecodeAllPortraitsWhileLoading(portraits, 4, &out);
    (void)rc;
    assert(out.valid == 1);
    assert(out.convertedPortraitCount == 0);
}

static void test_encode_empty(void)
{
    DM1_V1_CEDT019_PortraitSlotPc34 portraits[4];
    DM1_V1_CEDT019_PortraitBatchReceiptPc34 out;
    int rc;

    memset(portraits, 0, sizeof(portraits));
    memset(&out, 0, sizeof(out));
    rc = F2123_EncodeAllPortraitsBeforeSaving(portraits, 4, &out);
    (void)rc;
    assert(out.valid == 1);
    assert(out.convertedPortraitCount == 0);
}

static void test_source_evidence(void)
{
    const char *ev = F2122_F2123_F2124_CEDT019_SourceEvidencePc34();
    (void)ev;
    assert(ev != NULL);
    assert(ev[0] != '\0');
}

int main(void)
{
    test_constants();
    test_decode_empty();
    test_encode_empty();
    test_source_evidence();
    puts("ok: DM1 CEDT019 portrait save (Q-DM1-08) 4 tests passed");
    return 0;
}
