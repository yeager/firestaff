#include "dm1_v1_cedt019_portrait_save_pc34_compat.h"

#include <limits.h>
#include <string.h>

typedef int (*DM1_V1_CEDT019_PortraitStepPc34)(
    DM1_V1_CEDT019_PortraitSlotPc34 *slot);

static void init_receipt(DM1_V1_CEDT019_PortraitBatchReceiptPc34 *out,
                         int sourceLineStart,
                         int sourceLineEnd)
{
    if (!out) return;
    memset(out, 0, sizeof(*out));
    out->requiredPortraitCount = DM1_V1_CEDT019_PORTRAIT_COUNT_PC34;
    out->requiredPlanarBytes = DM1_V1_CEDT019_PORTRAIT_PLANAR_BYTES_PC34;
    out->requiredChunkyBytes = DM1_V1_CEDT019_PORTRAIT_CHUNKY_BYTES_PC34;
    out->sourceLineStart = sourceLineStart;
    out->sourceLineEnd = sourceLineEnd;
}

static int size_fits_u16(size_t value)
{
    return value <= (size_t)UINT16_MAX;
}

static int decode_planar_to_chunky(DM1_V1_CEDT019_PortraitSlotPc34 *slot)
{
    if (!slot || !slot->present || !slot->portraitBytesProven ||
        !slot->planarSource || !slot->chunkyDestination ||
        slot->planarSourceSize < DM1_V1_CEDT019_PORTRAIT_PLANAR_BYTES_PC34 ||
        slot->chunkyDestinationSize <
            DM1_V1_CEDT019_PORTRAIT_CHUNKY_BYTES_PC34 ||
        !size_fits_u16(slot->planarSourceSize) ||
        !size_fits_u16(slot->chunkyDestinationSize)) {
        return 0;
    }
    return DM1_V1_PortraitPanel_ConvertPlanarBufferToChunkyPc34Compat(
        slot->planarSource, (uint16_t)slot->planarSourceSize,
        slot->chunkyDestination, (uint16_t)slot->chunkyDestinationSize) ? 1 : 0;
}

static int encode_chunky_to_planar(DM1_V1_CEDT019_PortraitSlotPc34 *slot)
{
    if (!slot || !slot->present || !slot->portraitBytesProven ||
        !slot->chunkySource || !slot->planarDestination ||
        slot->chunkySourceSize < DM1_V1_CEDT019_PORTRAIT_CHUNKY_BYTES_PC34 ||
        slot->planarDestinationSize <
            DM1_V1_CEDT019_PORTRAIT_PLANAR_BYTES_PC34 ||
        !size_fits_u16(slot->chunkySourceSize) ||
        !size_fits_u16(slot->planarDestinationSize)) {
        return 0;
    }
    return DM1_V1_PortraitPanel_ConvertChunkyBufferToPlanarPc34Compat(
        slot->chunkySource, (uint16_t)slot->chunkySourceSize,
        slot->planarDestination, (uint16_t)slot->planarDestinationSize) ? 1 : 0;
}

static int run_all_portraits(DM1_V1_CEDT019_PortraitSlotPc34 *portraits,
                             size_t portraitCount,
                             DM1_V1_CEDT019_PortraitStepPc34 step,
                             DM1_V1_CEDT019_PortraitBatchReceiptPc34 *out,
                             int sourceLineStart,
                             int sourceLineEnd)
{
    size_t index;

    init_receipt(out, sourceLineStart, sourceLineEnd);
    if (!out || !portraits || !step ||
        portraitCount != DM1_V1_CEDT019_PORTRAIT_COUNT_PC34) {
        return 0;
    }

    out->valid = 1;
    for (index = 0; index < portraitCount; ++index) {
        if (step(&portraits[index])) {
            ++out->convertedPortraitCount;
        } else {
            ++out->rejectedPortraitCount;
        }
    }
    return out->rejectedPortraitCount == 0;
}

int F2122_DecodeAllPortraitsWhileLoading(
    DM1_V1_CEDT019_PortraitSlotPc34 *portraits,
    size_t portraitCount,
    DM1_V1_CEDT019_PortraitBatchReceiptPc34 *out)
{
    return run_all_portraits(portraits, portraitCount, decode_planar_to_chunky,
                             out, 41, 55);
}

int F2123_EncodeAllPortraitsBeforeSaving(
    DM1_V1_CEDT019_PortraitSlotPc34 *portraits,
    size_t portraitCount,
    DM1_V1_CEDT019_PortraitBatchReceiptPc34 *out)
{
    return run_all_portraits(portraits, portraitCount, encode_chunky_to_planar,
                             out, 58, 82);
}

int F2124_DecodeAllPortraitsAfterSaving(
    DM1_V1_CEDT019_PortraitSlotPc34 *portraits,
    size_t portraitCount,
    DM1_V1_CEDT019_PortraitBatchReceiptPc34 *out)
{
    return run_all_portraits(portraits, portraitCount, decode_planar_to_chunky,
                             out, 85, 121);
}

const char *F2122_F2123_F2124_CEDT019_SourceEvidencePc34(void)
{
    return "CEDT019.C:41 F2122_DecodeAllPortraitsWhileLoading, "
           "CEDT019.C:58 F2123_EncodeAllPortraitsBeforeSaving, and "
           "CEDT019.C:85 F2124_DecodeAllPortraitsAfterSaving are modeled as "
           "four-portrait save/load conversion gates over caller-owned "
           "portrait spans. PC34 requires proven portrait bytes and exact "
           "32x29 portrait buffers, and does not synthesize champion data, "
           "portrait bytes, screen pixels, file IO, or input events.";
}
