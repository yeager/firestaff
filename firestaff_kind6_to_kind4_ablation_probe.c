#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "boot_plan_script_pc34_compat.h"
#include "memory_graphics_dat_startup_pc34_compat.h"
#include "screen_bitmap_present_pc34_compat.h"
#include "dialog_frontend_pc34_compat.h"
#include "image_expand_pc34_compat.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static unsigned short read_u16_le(const unsigned char* p) {
    return (unsigned short)(p[0] | ((unsigned short)p[1] << 8));
}

static void write_u16_le(unsigned char* p, unsigned short v) {
    p[0] = (unsigned char)(v & 0xFFU);
    p[1] = (unsigned char)((v >> 8) & 0xFFU);
}

static unsigned char read_nibble_at(const unsigned char* src, unsigned int nibbleIndex) {
    unsigned char packed = src[nibbleIndex >> 1];
    return (nibbleIndex & 1U) ? (packed & 0x0F) : (packed >> 4);
}

static void write_nibble_at(unsigned char* dst, unsigned int nibbleIndex, unsigned char value) {
    unsigned char* packed = &dst[nibbleIndex >> 1];
    value &= 0x0FU;
    if (nibbleIndex & 1U) {
        *packed = (unsigned char)((*packed & 0xF0U) | value);
    } else {
        *packed = (unsigned char)((*packed & 0x0FU) | (value << 4));
    }
}

static unsigned int img3_read_pixel_count_from_nibbles(const unsigned char* src, unsigned int* nibbleIndex) {
    unsigned int value = read_nibble_at(src, (*nibbleIndex)++);
    if (value == 15U) {
        value = (read_nibble_at(src, (*nibbleIndex)++) << 4) | read_nibble_at(src, (*nibbleIndex)++);
        if (value == 255U) {
            value = (read_nibble_at(src, (*nibbleIndex)++) << 12)
                  | (read_nibble_at(src, (*nibbleIndex)++) << 8)
                  | (read_nibble_at(src, (*nibbleIndex)++) << 4)
                  | read_nibble_at(src, (*nibbleIndex)++);
        } else {
            value += 17U;
        }
    } else {
        value += 2U;
    }
    return value;
}

static int mutate_kind6_to_kind4(unsigned char* rawGraphic, unsigned short width, unsigned short height, unsigned long* outChangedCommands) {
    unsigned int nibbleIndex = 8U;
    unsigned long totalPixels = (unsigned long)width * (unsigned long)height;
    unsigned long producedPixels = 0;
    unsigned int i;
    unsigned long changed = 0;

    if (rawGraphic == 0) return 0;
    for (i = 0; i < 6U; ++i) {
        (void)read_nibble_at(rawGraphic, nibbleIndex++);
    }
    while (producedPixels < totalPixels) {
        unsigned int commandNibbleIndex = nibbleIndex;
        unsigned char command = read_nibble_at(rawGraphic, nibbleIndex++);
        unsigned char kind = command & 0x07U;
        unsigned int runLength;
        if (kind == 7U) {
            (void)read_nibble_at(rawGraphic, nibbleIndex++);
        }
        if (command & 0x08U) {
            runLength = img3_read_pixel_count_from_nibbles(rawGraphic, &nibbleIndex);
        } else {
            runLength = 1U;
        }
        if (kind == 6U) {
            write_nibble_at(rawGraphic, commandNibbleIndex, (unsigned char)((command & 0x08U) | 0x04U));
            changed++;
        }
        producedPixels += (unsigned long)runLength;
        if (producedPixels > totalPixels) return 0;
    }
    if (outChangedCommands) *outChangedCommands = changed;
    return 1;
}

static unsigned char get_nibble(const unsigned char* bitmap, unsigned short x, unsigned short y) {
    unsigned short width = read_u16_le(bitmap - 4);
    unsigned short bytesPerRow = (unsigned short)(((width + 1) & 0xFFFE) >> 1);
    const unsigned char* row = bitmap + ((unsigned long)y * (unsigned long)bytesPerRow);
    unsigned char packed = row[x >> 1];
    return (x & 1) ? (packed & 0x0F) : (packed >> 4);
}

static void patch_signed_delta_same_size(
    const unsigned char* a,
    const unsigned char* b,
    unsigned short x1,
    unsigned short y1,
    unsigned short x2,
    unsigned short y2,
    unsigned long* outPositiveDeltaSum,
    unsigned long* outNegativeDeltaSum,
    long* outSignedDeltaSum) {
    unsigned short x, y;
    unsigned long positive = 0;
    unsigned long negative = 0;
    long signedSum = 0;
    for (y = y1; y < y2; ++y) {
        for (x = x1; x < x2; ++x) {
            int av = (int)get_nibble(a, x, y);
            int bv = (int)get_nibble(b, x, y);
            int delta = av - bv;
            if (delta > 0) positive += (unsigned long)delta;
            else if (delta < 0) negative += (unsigned long)(-delta);
            signedSum += (long)delta;
        }
    }
    if (outPositiveDeltaSum) *outPositiveDeltaSum = positive;
    if (outNegativeDeltaSum) *outNegativeDeltaSum = negative;
    if (outSignedDeltaSum) *outSignedDeltaSum = signedSum;
}

int main(int argc, char** argv) {
    const char* graphicsDatPath;
    const char* scriptName;
    unsigned int frameIndex;
    struct MemoryGraphicsDatState_Compat fileState;
    struct MemoryGraphicsDatStartupResult_Compat startup;
    struct ScreenBitmapPresentResult_Compat presentBase;
    struct ScreenBitmapPresentResult_Compat presentMut;
    unsigned short dialogGraphicIndex = 1;
    unsigned short viewportGraphicIndex;
    unsigned char* viewportGraphicBuffer;
    unsigned char* viewportStorage;
    unsigned char* mutatedViewportStorage;
    unsigned char* screenStorage;
    unsigned char* mutatedScreenStorage;
    unsigned char* backdropStorage;
    unsigned char* viewportBitmap;
    unsigned char* mutatedViewportBitmap;
    unsigned char* screenBitmap;
    unsigned char* mutatedScreenBitmap;
    unsigned char* backdropBitmap;
    unsigned char* mutatedRaw;
    unsigned long changedCommands = 0;
    unsigned long basePos = 0, baseNeg = 0, mutPos = 0, mutNeg = 0;
    long baseSigned = 0, mutSigned = 0;

    if (argc != 4) {
        fprintf(stderr, "usage: %s GRAPHICS.DAT scriptName frameIndex\n", argv[0]);
        return 1;
    }
    graphicsDatPath = argv[1];
    scriptName = argv[2];
    frameIndex = (unsigned int)strtoul(argv[3], 0, 10);
    {
        const struct BootPlanScript_Compat* script = F9010_RUNTIME_GetBootPlanScript_Compat(scriptName);
        if ((script == 0) || (frameIndex == 0) || (frameIndex > script->stepCount)) {
            fprintf(stderr, "bad script/frame\n");
            return 1;
        }
        viewportGraphicIndex = (unsigned short)script->steps[frameIndex - 1].viewportGraphicIndex;
    }

    memset(&fileState, 0, sizeof(fileState));
    memset(&startup, 0, sizeof(startup));
    memset(&presentBase, 0, sizeof(presentBase));
    memset(&presentMut, 0, sizeof(presentMut));

    viewportGraphicBuffer = (unsigned char*)calloc(4096U, 1);
    viewportStorage = (unsigned char*)calloc(65536U, 1);
    mutatedViewportStorage = (unsigned char*)calloc(65536U, 1);
    screenStorage = (unsigned char*)calloc(65536U, 1);
    mutatedScreenStorage = (unsigned char*)calloc(65536U, 1);
    backdropStorage = (unsigned char*)calloc(65536U, 1);
    mutatedRaw = (unsigned char*)calloc(4096U, 1);
    if (!viewportGraphicBuffer || !viewportStorage || !mutatedViewportStorage || !screenStorage || !mutatedScreenStorage || !backdropStorage || !mutatedRaw) {
        fprintf(stderr, "alloc failed\n");
        return 1;
    }

    viewportBitmap = viewportStorage + 4;
    mutatedViewportBitmap = mutatedViewportStorage + 4;
    screenBitmap = screenStorage + 4;
    mutatedScreenBitmap = mutatedScreenStorage + 4;
    backdropBitmap = backdropStorage + 4;

    if (!F0479_MEMORY_StartupGraphicsChain_Compat(
            graphicsDatPath,
            &fileState,
            dialogGraphicIndex,
            viewportGraphicIndex,
            viewportGraphicBuffer,
            viewportBitmap,
            &startup)) {
        fprintf(stderr, "startup failed\n");
        return 1;
    }

    memcpy(mutatedRaw, viewportGraphicBuffer, 4096U);
    if (!mutate_kind6_to_kind4(mutatedRaw,
            startup.viewportSelection.widthHeight.Width,
            startup.viewportSelection.widthHeight.Height,
            &changedCommands)) {
        fprintf(stderr, "mutation failed\n");
        return 1;
    }

    memcpy(mutatedViewportStorage, viewportStorage, 65536U);
    memset(mutatedViewportBitmap, 0, 65532U);
    write_u16_le(mutatedViewportStorage + 0, startup.viewportSelection.widthHeight.Width);
    write_u16_le(mutatedViewportStorage + 2, startup.viewportSelection.widthHeight.Height);
    F0689_IMG_ExpandGraphicToBitmap_Compat(mutatedRaw, mutatedViewportBitmap);

    F0427_DIALOG_DrawBackdrop_Compat(
        startup.specials.dialogBoxGraphic,
        backdropBitmap,
        &startup.runtimeState.widthHeight[dialogGraphicIndex]);
    F0427_DIALOG_DrawBackdrop_Compat(
        startup.specials.dialogBoxGraphic,
        screenBitmap,
        &startup.runtimeState.widthHeight[dialogGraphicIndex]);
    F0427_DIALOG_DrawBackdrop_Compat(
        startup.specials.dialogBoxGraphic,
        mutatedScreenBitmap,
        &startup.runtimeState.widthHeight[dialogGraphicIndex]);

    if (!F9005_SCREEN_OverlayBitmapOnScreen_Compat(viewportBitmap, screenBitmap, 0, &presentBase)) {
        fprintf(stderr, "base overlay failed\n");
        return 1;
    }
    if (!F9005_SCREEN_OverlayBitmapOnScreen_Compat(mutatedViewportBitmap, mutatedScreenBitmap, 0, &presentMut)) {
        fprintf(stderr, "mut overlay failed\n");
        return 1;
    }

    patch_signed_delta_same_size(screenBitmap, backdropBitmap, 153, 90, 167, 109, &basePos, &baseNeg, &baseSigned);
    patch_signed_delta_same_size(mutatedScreenBitmap, backdropBitmap, 153, 90, 167, 109, &mutPos, &mutNeg, &mutSigned);

    printf("frame=%u graphic=%u changedKind6Commands=%lu baseSigned=%ld basePos=%lu baseNeg=%lu mutatedSigned=%ld mutatedPos=%lu mutatedNeg=%lu deltaFromBase=%ld\n",
        frameIndex,
        (unsigned int)viewportGraphicIndex,
        changedCommands,
        baseSigned,
        basePos,
        baseNeg,
        mutSigned,
        mutPos,
        mutNeg,
        (long)(mutSigned - baseSigned));

    F0479_MEMORY_FreeStartupGraphicsChain_Compat(&startup);
    free(viewportGraphicBuffer);
    free(viewportStorage);
    free(mutatedViewportStorage);
    free(screenStorage);
    free(mutatedScreenStorage);
    free(backdropStorage);
    free(mutatedRaw);
    return 0;
}
