#include "dm1_v1_cedt019_portrait_save_pc34_compat.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

static void fill_chunky(uint8_t chunky[DM1_V1_CEDT019_PORTRAIT_CHUNKY_BYTES_PC34])
{
    size_t index;

    for (index = 0; index < DM1_V1_CEDT019_PORTRAIT_CHUNKY_BYTES_PC34;
         ++index) {
        chunky[index] = (uint8_t)(index & 0x0fu);
    }
}

static void fill_slots(
    DM1_V1_CEDT019_PortraitSlotPc34 slots[DM1_V1_CEDT019_PORTRAIT_COUNT_PC34],
    const uint8_t planar[DM1_V1_CEDT019_PORTRAIT_COUNT_PC34]
                         [DM1_V1_CEDT019_PORTRAIT_PLANAR_BYTES_PC34],
    uint8_t planarOut[DM1_V1_CEDT019_PORTRAIT_COUNT_PC34]
                     [DM1_V1_CEDT019_PORTRAIT_PLANAR_BYTES_PC34],
    const uint8_t chunky[DM1_V1_CEDT019_PORTRAIT_COUNT_PC34]
                         [DM1_V1_CEDT019_PORTRAIT_CHUNKY_BYTES_PC34],
    uint8_t chunkyOut[DM1_V1_CEDT019_PORTRAIT_COUNT_PC34]
                     [DM1_V1_CEDT019_PORTRAIT_CHUNKY_BYTES_PC34])
{
    size_t index;

    memset(slots, 0, sizeof(DM1_V1_CEDT019_PortraitSlotPc34) *
                         DM1_V1_CEDT019_PORTRAIT_COUNT_PC34);
    for (index = 0; index < DM1_V1_CEDT019_PORTRAIT_COUNT_PC34; ++index) {
        slots[index].present = 1;
        slots[index].portraitBytesProven = 1;
        slots[index].planarSource = planar[index];
        slots[index].planarSourceSize =
            DM1_V1_CEDT019_PORTRAIT_PLANAR_BYTES_PC34;
        slots[index].planarDestination = planarOut[index];
        slots[index].planarDestinationSize =
            DM1_V1_CEDT019_PORTRAIT_PLANAR_BYTES_PC34;
        slots[index].chunkySource = chunky[index];
        slots[index].chunkySourceSize =
            DM1_V1_CEDT019_PORTRAIT_CHUNKY_BYTES_PC34;
        slots[index].chunkyDestination = chunkyOut[index];
        slots[index].chunkyDestinationSize =
            DM1_V1_CEDT019_PORTRAIT_CHUNKY_BYTES_PC34;
    }
}

static void prepare_portrait_buffers(
    uint8_t planar[DM1_V1_CEDT019_PORTRAIT_COUNT_PC34]
                  [DM1_V1_CEDT019_PORTRAIT_PLANAR_BYTES_PC34],
    uint8_t chunky[DM1_V1_CEDT019_PORTRAIT_COUNT_PC34]
                  [DM1_V1_CEDT019_PORTRAIT_CHUNKY_BYTES_PC34])
{
    (void)planar;
    uint8_t onePortrait[DM1_V1_CEDT019_PORTRAIT_CHUNKY_BYTES_PC34];
    size_t index;

    fill_chunky(onePortrait);
    for (index = 0; index < DM1_V1_CEDT019_PORTRAIT_COUNT_PC34; ++index) {
        memcpy(chunky[index], onePortrait, sizeof(onePortrait));
        assert(DM1_V1_PortraitPanel_ConvertChunkyBufferToPlanarPc34Compat(
            chunky[index], DM1_V1_CEDT019_PORTRAIT_CHUNKY_BYTES_PC34,
            planar[index], DM1_V1_CEDT019_PORTRAIT_PLANAR_BYTES_PC34));
    }
}

static void test_load_and_post_save_decode_all_four_portraits(void)
{
    uint8_t planar[DM1_V1_CEDT019_PORTRAIT_COUNT_PC34]
                  [DM1_V1_CEDT019_PORTRAIT_PLANAR_BYTES_PC34];
    uint8_t planarOut[DM1_V1_CEDT019_PORTRAIT_COUNT_PC34]
                     [DM1_V1_CEDT019_PORTRAIT_PLANAR_BYTES_PC34];
    uint8_t chunky[DM1_V1_CEDT019_PORTRAIT_COUNT_PC34]
                  [DM1_V1_CEDT019_PORTRAIT_CHUNKY_BYTES_PC34];
    uint8_t chunkyOut[DM1_V1_CEDT019_PORTRAIT_COUNT_PC34]
                     [DM1_V1_CEDT019_PORTRAIT_CHUNKY_BYTES_PC34];
    DM1_V1_CEDT019_PortraitSlotPc34 slots[DM1_V1_CEDT019_PORTRAIT_COUNT_PC34];
    DM1_V1_CEDT019_PortraitBatchReceiptPc34 receipt;
    (void)receipt;

    memset(planarOut, 0, sizeof(planarOut));
    memset(chunkyOut, 0, sizeof(chunkyOut));
    prepare_portrait_buffers(planar, chunky);
    fill_slots(slots, planar, planarOut, chunky, chunkyOut);

    assert(F2122_DecodeAllPortraitsWhileLoading(
        slots, DM1_V1_CEDT019_PORTRAIT_COUNT_PC34, &receipt) == 1);
    assert(receipt.valid == 1);
    assert(receipt.convertedPortraitCount == 4);
    assert(receipt.rejectedPortraitCount == 0);
    assert(receipt.sourceLineStart == 41);
    assert(memcmp(chunkyOut[0], chunky[0],
                  DM1_V1_CEDT019_PORTRAIT_CHUNKY_BYTES_PC34) == 0);

    memset(chunkyOut, 0, sizeof(chunkyOut));
    assert(F2124_DecodeAllPortraitsAfterSaving(
        slots, DM1_V1_CEDT019_PORTRAIT_COUNT_PC34, &receipt) == 1);
    assert(receipt.convertedPortraitCount == 4);
    assert(receipt.sourceLineStart == 85);
    assert(memcmp(chunkyOut[3], chunky[3],
                  DM1_V1_CEDT019_PORTRAIT_CHUNKY_BYTES_PC34) == 0);
}

static void test_save_encode_all_four_portraits(void)
{
    uint8_t planar[DM1_V1_CEDT019_PORTRAIT_COUNT_PC34]
                  [DM1_V1_CEDT019_PORTRAIT_PLANAR_BYTES_PC34];
    uint8_t planarOut[DM1_V1_CEDT019_PORTRAIT_COUNT_PC34]
                     [DM1_V1_CEDT019_PORTRAIT_PLANAR_BYTES_PC34];
    uint8_t chunky[DM1_V1_CEDT019_PORTRAIT_COUNT_PC34]
                  [DM1_V1_CEDT019_PORTRAIT_CHUNKY_BYTES_PC34];
    uint8_t chunkyOut[DM1_V1_CEDT019_PORTRAIT_COUNT_PC34]
                     [DM1_V1_CEDT019_PORTRAIT_CHUNKY_BYTES_PC34];
    DM1_V1_CEDT019_PortraitSlotPc34 slots[DM1_V1_CEDT019_PORTRAIT_COUNT_PC34];
    DM1_V1_CEDT019_PortraitBatchReceiptPc34 receipt;
    (void)receipt;

    memset(planarOut, 0, sizeof(planarOut));
    memset(chunkyOut, 0, sizeof(chunkyOut));
    prepare_portrait_buffers(planar, chunky);
    fill_slots(slots, planar, planarOut, chunky, chunkyOut);

    assert(F2123_EncodeAllPortraitsBeforeSaving(
        slots, DM1_V1_CEDT019_PORTRAIT_COUNT_PC34, &receipt) == 1);
    assert(receipt.valid == 1);
    assert(receipt.convertedPortraitCount == 4);
    assert(receipt.rejectedPortraitCount == 0);
    assert(receipt.sourceLineStart == 58);
    assert(memcmp(planarOut[2], planar[2],
                  DM1_V1_CEDT019_PORTRAIT_PLANAR_BYTES_PC34) == 0);
}

static void test_missing_proof_or_wrong_count_fails_closed(void)
{
    uint8_t planar[DM1_V1_CEDT019_PORTRAIT_COUNT_PC34]
                  [DM1_V1_CEDT019_PORTRAIT_PLANAR_BYTES_PC34];
    uint8_t planarOut[DM1_V1_CEDT019_PORTRAIT_COUNT_PC34]
                     [DM1_V1_CEDT019_PORTRAIT_PLANAR_BYTES_PC34];
    uint8_t chunky[DM1_V1_CEDT019_PORTRAIT_COUNT_PC34]
                  [DM1_V1_CEDT019_PORTRAIT_CHUNKY_BYTES_PC34];
    uint8_t chunkyOut[DM1_V1_CEDT019_PORTRAIT_COUNT_PC34]
                     [DM1_V1_CEDT019_PORTRAIT_CHUNKY_BYTES_PC34];
    DM1_V1_CEDT019_PortraitSlotPc34 slots[DM1_V1_CEDT019_PORTRAIT_COUNT_PC34];
    DM1_V1_CEDT019_PortraitBatchReceiptPc34 receipt;
    (void)receipt;

    memset(planarOut, 0, sizeof(planarOut));
    memset(chunkyOut, 0, sizeof(chunkyOut));
    prepare_portrait_buffers(planar, chunky);
    fill_slots(slots, planar, planarOut, chunky, chunkyOut);

    slots[1].portraitBytesProven = 0;
    assert(F2122_DecodeAllPortraitsWhileLoading(
        slots, DM1_V1_CEDT019_PORTRAIT_COUNT_PC34, &receipt) == 0);
    assert(receipt.valid == 1);
    assert(receipt.convertedPortraitCount == 3);
    assert(receipt.rejectedPortraitCount == 1);

    slots[1].portraitBytesProven = 1;
    assert(F2123_EncodeAllPortraitsBeforeSaving(slots, 3, &receipt) == 0);
    assert(receipt.valid == 0);
    assert(receipt.requiredPortraitCount == 4);
}

static void test_source_evidence_names_boundaries(void)
{
    const char *evidence = F2122_F2123_F2124_CEDT019_SourceEvidencePc34();
    (void)evidence;

    assert(strstr(evidence, "CEDT019.C:41") != 0);
    assert(strstr(evidence, "CEDT019.C:58") != 0);
    assert(strstr(evidence, "CEDT019.C:85") != 0);
    assert(strstr(evidence, "caller-owned") != 0);
    assert(strstr(evidence, "does not synthesize") != 0);
    assert(strstr(evidence, "portrait bytes") != 0);
}

int main(void)
{
    test_load_and_post_save_decode_all_four_portraits();
    test_save_encode_all_four_portraits();
    test_missing_proof_or_wrong_count_fails_closed();
    test_source_evidence_names_boundaries();
    return 0;
}
