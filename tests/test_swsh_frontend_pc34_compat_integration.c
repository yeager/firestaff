#include <stdio.h>
#include <string.h>

#include "swsh_frontend_pc34_compat.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static const char* swsh_kind_name(SWSH_CompatSourceEventKind kind) {
    switch (kind) {
    case SWSH_COMPAT_SOURCE_EVENT_LOAD_LOGO_BITMAP: return "LOAD_LOGO_BITMAP";
    case SWSH_COMPAT_SOURCE_EVENT_START_SOUND: return "START_SOUND";
    case SWSH_COMPAT_SOURCE_EVENT_SET_PALETTE_COLOR: return "SET_PALETTE_COLOR";
    case SWSH_COMPAT_SOURCE_EVENT_WAIT_VBLANKS: return "WAIT_VBLANKS";
    case SWSH_COMPAT_SOURCE_EVENT_RUN_START_PROGRAM: return "RUN_START_PROGRAM";
    }
    return "UNKNOWN";
}

static int test_swsh_logo_expand_same_stride(void) {
    unsigned char src[6] = {0x04, 0x00, 0x01, 0x00, 0x31};
    unsigned char dst[2] = {0x00, 0x00};
    SWSH_Compat_ExpandLogoToBitmap(src, dst);
    return dst[0] == 0x11 && dst[1] == 0x11;
}

static int test_swsh_logo_expand_raw_literal(void) {
    unsigned char src[8] = {0x04, 0x00, 0x01, 0x00, 0x91, 0x03, 0x23, 0x45};
    unsigned char dst[2] = {0x00, 0x00};
    SWSH_Compat_ExpandLogoToBitmap(src, dst);
    return dst[0] == 0x23 && dst[1] == 0x45;
}

static int test_swsh_logo_expand_previous_line(void) {
    unsigned char src[11] = {
        0x06, 0x00, 0x02, 0x00,
        0x52,
        0xb3, 0x04,
        0x00, 0x00, 0x00, 0x00
    };
    unsigned char dst[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    SWSH_Compat_ExpandLogoToBitmap(src, dst);
    return dst[0] == 0x22 && dst[1] == 0x22 && dst[2] == 0x22 &&
           dst[3] == 0x22 && dst[4] == 0x22 && dst[5] == 0x23;
}

static int test_swsh_embedded_mz_payload_detection(void) {
    unsigned char rawImg[1600];
    unsigned char mzImg[2048];
    unsigned int pos = 0u;
    unsigned int row;
    const unsigned char* rawPayload;
    const unsigned char* mzPayload;
    memset(rawImg, 0, sizeof(rawImg));
    rawImg[pos++] = 0x40u; rawImg[pos++] = 0x01u; rawImg[pos++] = 0xc8u; rawImg[pos++] = 0x00u;
    for (row = 0u; row < 51u; ++row) { rawImg[pos++] = 0xc0u; rawImg[pos++] = 0x01u; rawImg[pos++] = 0x3fu; }
    for (row = 0u; row < 119u; ++row) {
        rawImg[pos++] = 0x80u; rawImg[pos++] = 0x17u;
        rawImg[pos++] = 0x8fu; rawImg[pos++] = 0x51u;
        rawImg[pos++] = 0x80u; rawImg[pos++] = 0xa2u;
        rawImg[pos++] = 0x0fu;
        rawImg[pos++] = 0x80u; rawImg[pos++] = 0x31u;
    }
    for (row = 0u; row < 30u; ++row) { rawImg[pos++] = 0xc0u; rawImg[pos++] = 0x01u; rawImg[pos++] = 0x3fu; }
    memset(mzImg, 0, sizeof(mzImg));
    mzImg[0] = 'M'; mzImg[1] = 'Z';
    memcpy(mzImg + 64u, rawImg, pos);
    rawPayload = SWSH_Compat_FindLogoImagePayload(rawImg, pos);
    mzPayload = SWSH_Compat_FindLogoImagePayload(mzImg, 64u + pos);
    return rawPayload == rawImg &&
           mzPayload == mzImg + 64u &&
           SWSH_Compat_FindLogoImagePayload(mzImg, 5u) == NULL;
}

static int test_swsh_palette_word_conversion(void) {
    unsigned char rgb[3] = {0, 0, 0};
    SWSH_Compat_ConvertPcSwooshRgbWordToRgb8(0x0777u, rgb);
    if (rgb[0] != 255u || rgb[1] != 255u || rgb[2] != 255u) return 0;
    SWSH_Compat_ConvertPcSwooshRgbWordToRgb8(0x0555u, rgb);
    if (rgb[0] != 190u || rgb[1] != 190u || rgb[2] != 190u) return 0;
    SWSH_Compat_ConvertPcSwooshRgbWordToRgb8(0x0222u, rgb);
    if (rgb[0] != 125u || rgb[1] != 125u || rgb[2] != 125u) return 0;
    SWSH_Compat_ConvertPcSwooshRgbWordToRgb8(0x0770u, rgb);
    return rgb[0] == 255u && rgb[1] == 255u && rgb[2] == 0u;
}

static int test_swsh_source_animation_schedule(void) {
    unsigned int i;
    unsigned int colorSetCount = 0u;
    unsigned int waitCommandCount = 0u;
    unsigned int waitVblankCount = 0u;
    int ok = 1;
    SWSH_CompatSourceTiming timing = SWSH_Compat_GetSourceTimingEvidence();
    SWSH_CompatSourceAnimationStep first;
    SWSH_CompatSourceAnimationStep sound;
    SWSH_CompatSourceAnimationStep color1;
    SWSH_CompatSourceAnimationStep finalYellow;
    SWSH_CompatSourceAnimationStep runStart;

    memset(&first, 0, sizeof(first));
    memset(&sound, 0, sizeof(sound));
    memset(&color1, 0, sizeof(color1));
    memset(&finalYellow, 0, sizeof(finalYellow));
    memset(&runStart, 0, sizeof(runStart));

    printf("probe=firestaff_swsh_source_animation_schedule\n");
    printf("sourceAnimationEvidence=%s\n", SWSH_Compat_GetSourceAnimationEvidence());
    printf("sourceAnimationStepCount=%u\n", SWSH_Compat_GetSourceAnimationStepCount());
    if (SWSH_Compat_GetSourceAnimationStepCount() != 29u) ok = 0;
    for (i = 1u; i <= SWSH_Compat_GetSourceAnimationStepCount(); ++i) {
        SWSH_CompatSourceAnimationStep step;
        if (!SWSH_Compat_GetSourceAnimationStep(i, &step)) { ok = 0; continue; }
        printf("sourceAnimationStep[%u]=kind:%s color:%u:%03x vblank:%u line:%u evidence:%s\n",
               i,
               swsh_kind_name(step.kind),
               step.colorIndex,
               step.colorValue,
               step.vblankCount,
               step.sourceLine,
               step.sourceLineEvidence ? step.sourceLineEvidence : "");
        if (i == 1u) first = step;
        if (i == 2u) sound = step;
        if (i == 3u) color1 = step;
        if (i == 28u) finalYellow = step;
        if (i == 29u) runStart = step;
        if (step.kind == SWSH_COMPAT_SOURCE_EVENT_SET_PALETTE_COLOR) colorSetCount++;
        if (step.kind == SWSH_COMPAT_SOURCE_EVENT_WAIT_VBLANKS) { waitCommandCount++; waitVblankCount += step.vblankCount; }
    }
    if (first.kind != SWSH_COMPAT_SOURCE_EVENT_LOAD_LOGO_BITMAP) ok = 0;
    if (sound.kind != SWSH_COMPAT_SOURCE_EVENT_START_SOUND) ok = 0;
    if (color1.kind != SWSH_COMPAT_SOURCE_EVENT_SET_PALETTE_COLOR || color1.colorIndex != 1u || color1.colorValue != 0x0777u || color1.sourceLine != 282u) ok = 0;
    if (finalYellow.kind != SWSH_COMPAT_SOURCE_EVENT_SET_PALETTE_COLOR || finalYellow.colorIndex != 14u || finalYellow.colorValue != 0x0770u || finalYellow.sourceLine != 307u) ok = 0;
    if (runStart.kind != SWSH_COMPAT_SOURCE_EVENT_RUN_START_PROGRAM) ok = 0;
    printf("paletteCounts=color:%u/%u wait:%u/%u vblank:%u/%u\n",
           colorSetCount, timing.paletteColorSetCount,
           waitCommandCount, timing.paletteWaitCommandCount,
           waitVblankCount, timing.paletteWaitVblankCount);
    printf("soundCounts=write:%u wait:%u vblank:%u\n",
           timing.soundRegisterWriteCount,
           timing.soundWaitCommandCount,
           timing.soundWaitVblankCount);
    if (colorSetCount != timing.paletteColorSetCount || waitCommandCount != timing.paletteWaitCommandCount || waitVblankCount != timing.paletteWaitVblankCount) ok = 0;
    if (timing.paletteCommandCount != 26u || timing.paletteWaitVblankCount != 30u || timing.soundRegisterWriteCount != 17u || timing.soundWaitCommandCount != 10u || timing.soundWaitVblankCount != 20u) ok = 0;
    if (SWSH_Compat_GetRuntimeDelayMsForVblankCount(1u) != SWSH_COMPAT_RUNTIME_VBLANK_MS) ok = 0;
    if (SWSH_Compat_GetRuntimeDelayMsForVblankCount(3u) != 3u * SWSH_COMPAT_RUNTIME_VBLANK_MS) ok = 0;
    if (SWSH_Compat_GetRuntimeInitialLogoHoldMs() != 20u * SWSH_COMPAT_RUNTIME_VBLANK_MS) ok = 0;
    if (SWSH_Compat_GetRuntimeFinalHoldMs() != 120u * SWSH_COMPAT_RUNTIME_VBLANK_MS) ok = 0;
    printf("runtimeDelaysMs=oneVblank:%u initialLogo:%u finalHold:%u\n",
           SWSH_Compat_GetRuntimeDelayMsForVblankCount(1u),
           SWSH_Compat_GetRuntimeInitialLogoHoldMs(),
           SWSH_Compat_GetRuntimeFinalHoldMs());
    printf("swshSourceAnimationScheduleInvariantOk=%d\n", ok);
    return ok;
}

int main(void) {
    if (!test_swsh_logo_expand_same_stride()) {
        fprintf(stderr, "test_swsh_logo_expand_same_stride failed\n");
        return 1;
    }
    if (!test_swsh_logo_expand_raw_literal()) {
        fprintf(stderr, "test_swsh_logo_expand_raw_literal failed\n");
        return 1;
    }
    if (!test_swsh_logo_expand_previous_line()) {
        fprintf(stderr, "test_swsh_logo_expand_previous_line failed\n");
        return 1;
    }
    if (!test_swsh_embedded_mz_payload_detection()) {
        fprintf(stderr, "test_swsh_embedded_mz_payload_detection failed\n");
        return 1;
    }
    if (!test_swsh_palette_word_conversion()) {
        fprintf(stderr, "test_swsh_palette_word_conversion failed\n");
        return 1;
    }
    if (!test_swsh_source_animation_schedule()) {
        fprintf(stderr, "test_swsh_source_animation_schedule failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
