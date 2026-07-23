#include "dm1_v1_f0902_draw_ftl_logo_presentation_plan_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

static void check(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

int main(void)
{
    const char *realMediaPath = getenv("FIRESTAFF_DM1_PC34_SWOOSH");
    uint8_t frame[DM1_V1_F0902_FTL_LOGO_PACKED_STRIDE_PC34 *
                  DM1_V1_F0902_FTL_LOGO_HEIGHT_PC34];
    uint16_t palette[DM1_V1_F0902_FTL_LOGO_PALETTE_COLORS_PC34];
    DM1_V1_F0902_RealMediaInput_PC34 input;
    DM1_V1_F0902_RealMediaReceipt_PC34 receipt;
    DM1_V1_F0902_DrawFTLLogoPresentationPlan_PC34 plan;
    uint8_t *mediaBytes = NULL;
    size_t mediaBytesCount = 0U;
    FILE *media = NULL;

    memset(frame, 0, sizeof(frame));
    memset(palette, 0, sizeof(palette));
    memset(&input, 0, sizeof(input));
    input.decodedPackedFrame = frame;
    input.decodedPackedFrameCapacity = sizeof(frame);
    input.sourcePalette = palette;
    input.sourcePaletteCapacity = sizeof(palette) / sizeof(palette[0]);

    check(!dm1_v1_f0902_draw_ftl_logo_real_media_pc34(&input, &receipt),
          "missing real source media fails closed");

    if (realMediaPath && realMediaPath[0] != '\0') {
        long fileSize;
        media = fopen(realMediaPath, "rb");
        check(media != NULL, "configured real PC34 SWOOSH opens");
        if (media) {
            (void)fseek(media, 0L, SEEK_END);
            fileSize = ftell(media);
            (void)fseek(media, 0L, SEEK_SET);
            check(fileSize > 0L, "configured real PC34 SWOOSH is nonempty");
            if (fileSize > 0L) {
                mediaBytes = (uint8_t *)malloc((size_t)fileSize);
                check(mediaBytes != NULL, "real PC34 SWOOSH buffer allocates");
                if (mediaBytes) {
                    mediaBytesCount = (size_t)fileSize;
                    check(fread(mediaBytes, 1U, mediaBytesCount, media) == mediaBytesCount,
                          "configured real PC34 SWOOSH reads");
                }
            }
            fclose(media);
        }
        if (mediaBytes) {
            input.sourceMedia = mediaBytes;
            input.sourceMediaBytes = mediaBytesCount;
            input.sourceMediaKind = DM1_V1_F0902_SOURCE_MEDIA_PC34_SWOOSH_PC34;
            input.sourceMediaHashVerified = 1;
            check(dm1_v1_f0902_draw_ftl_logo_real_media_pc34(&input, &receipt),
                  "authenticated real PC34 SWOOSH decodes a FTL plan");
            plan = receipt.plan;
            check(receipt.valid &&
                      (receipt.sourcePayloadStoredInMedia == 0 ||
                       receipt.sourcePayloadOffset > 0U) &&
                      receipt.paletteCommandCount == 26U &&
                      receipt.paletteWaitVblanks == 30U &&
                      receipt.initialHoldVblanks == 20U && receipt.finalHoldVblanks == 120U &&
                      plan.kind == DM1_V1_F0902_DRAW_FTL_LOGO_PLAN_BLIT_PACKED_FRAME_PC34 &&
                      plan.packedFrame == frame && plan.packedFrameBytes == sizeof(frame) &&
                      plan.palette == palette && plan.paletteColorCount == 16U,
                  "real media receipt preserves F0902 frame palette and timing");
            input.sourceMediaHashVerified = 0;
            check(!dm1_v1_f0902_draw_ftl_logo_real_media_pc34(&input, &receipt),
                  "unverified source media fails closed");
        }
    }
    free(mediaBytes);

    return failures ? 1 : 0;
}
