/*
 * ReDMCSB SWSH.C F0902_DrawFTLLogo presentation-plan adapter.
 *
 * The adapter deliberately does not decode, synthesize, or render the FTL
 * logo. It publishes a plan only for the original expanded 320x200 packed
 * 4bpp frame and its original 16-color palette.
 */
#ifndef DM1_V1_F0902_DRAW_FTL_LOGO_PRESENTATION_PLAN_PC34_COMPAT_H
#define DM1_V1_F0902_DRAW_FTL_LOGO_PRESENTATION_PLAN_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_F0902_FTL_LOGO_WIDTH_PC34 320u
#define DM1_V1_F0902_FTL_LOGO_HEIGHT_PC34 200u
#define DM1_V1_F0902_FTL_LOGO_PACKED_STRIDE_PC34 160u
#define DM1_V1_F0902_FTL_LOGO_PALETTE_COLORS_PC34 16u

typedef struct DM1_V1_F0902_DrawFTLLogoOriginalInput_PC34 {
    const uint8_t *packedFrame;
    size_t packedFrameBytes;
    unsigned int frameWidth;
    unsigned int frameHeight;
    unsigned int frameStrideBytes;
    const uint16_t *palette;
    size_t paletteColorCount;
} DM1_V1_F0902_DrawFTLLogoOriginalInput_PC34;

typedef enum DM1_V1_F0902_DrawFTLLogoPlanKind_PC34 {
    DM1_V1_F0902_DRAW_FTL_LOGO_PLAN_NONE_PC34 = 0,
    DM1_V1_F0902_DRAW_FTL_LOGO_PLAN_BLIT_PACKED_FRAME_PC34 = 1
} DM1_V1_F0902_DrawFTLLogoPlanKind_PC34;

typedef struct DM1_V1_F0902_DrawFTLLogoPresentationPlan_PC34 {
    DM1_V1_F0902_DrawFTLLogoPlanKind_PC34 kind;
    const uint8_t *packedFrame;
    size_t packedFrameBytes;
    unsigned int srcWidth;
    unsigned int srcHeight;
    unsigned int srcStrideBytes;
    unsigned int dstX;
    unsigned int dstY;
    const uint16_t *palette;
    size_t paletteColorCount;
} DM1_V1_F0902_DrawFTLLogoPresentationPlan_PC34;

/*
 * The PC-34 release stores the compressed SWSHGDAT logo in the original
 * SWOOSH program.  SWSHIIGS uses the same source-sized output contract even
 * though its loader is different.  A caller may only select a format it has
 * already authenticated through the game-data scanner.
 */
typedef enum DM1_V1_F0902_SourceMediaKind_PC34 {
    DM1_V1_F0902_SOURCE_MEDIA_NONE_PC34 = 0,
    DM1_V1_F0902_SOURCE_MEDIA_PC34_SWOOSH_PC34 = 1,
    DM1_V1_F0902_SOURCE_MEDIA_IIGS_SWSHIIGS_PC34 = 2
} DM1_V1_F0902_SourceMediaKind_PC34;

typedef struct DM1_V1_F0902_RealMediaInput_PC34 {
    const uint8_t *sourceMedia;
    size_t sourceMediaBytes;
    DM1_V1_F0902_SourceMediaKind_PC34 sourceMediaKind;
    int sourceMediaHashVerified;
    uint8_t *decodedPackedFrame;
    size_t decodedPackedFrameCapacity;
    uint16_t *sourcePalette;
    size_t sourcePaletteCapacity;
} DM1_V1_F0902_RealMediaInput_PC34;

typedef struct DM1_V1_F0902_RealMediaReceipt_PC34 {
    int valid;
    DM1_V1_F0902_SourceMediaKind_PC34 sourceMediaKind;
    uint32_t sourceMediaFnv1a;
    int sourcePayloadStoredInMedia;
    size_t sourcePayloadOffset;
    unsigned int paletteCommandCount;
    unsigned int paletteWaitVblanks;
    unsigned int initialHoldVblanks;
    unsigned int finalHoldVblanks;
    DM1_V1_F0902_DrawFTLLogoPresentationPlan_PC34 plan;
} DM1_V1_F0902_RealMediaReceipt_PC34;

/*
 * Builds the full-screen FTL logo plan. Returns 1 only when both original
 * inputs have SWSH.C's 320x200 packed-4bpp / 16-color shape. On failure,
 * `outPlan` is cleared and no substitute presentation is authorized.
 */
int dm1_v1_f0902_draw_ftl_logo_presentation_plan_pc34(
    const DM1_V1_F0902_DrawFTLLogoOriginalInput_PC34 *input,
    DM1_V1_F0902_DrawFTLLogoPresentationPlan_PC34 *outPlan);

/*
 * Decodes only an authenticated original SWSH/SWSHIIGS media payload.  It
 * derives the palette schedule and timing from SWSH.C, does not fabricate a
 * frame or logo, and leaves the receipt empty on any provenance or decode
 * failure.  `decodedPackedFrame` and `sourcePalette` remain caller-owned.
 */
int dm1_v1_f0902_draw_ftl_logo_real_media_pc34(
    const DM1_V1_F0902_RealMediaInput_PC34 *input,
    DM1_V1_F0902_RealMediaReceipt_PC34 *outReceipt);

const char *dm1_v1_f0902_draw_ftl_logo_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_F0902_DRAW_FTL_LOGO_PRESENTATION_PLAN_PC34_COMPAT_H */
