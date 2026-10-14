#include "dm1_v1_selector_pc34_compat.h"

#include "asset_find_by_hash.h"
#include "image_backend_pc34_compat.h"
#include "lzexe_unpack_v091.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define DM1_I34E_SELECTOR_SIZE 15474u
#define DM1_I34E_SELECTOR_MD5 "165ae5438ce2686ae62f2fbb61f3e831"
#define DM1_I34E_VIEW_SOURCE_BYTES 7673u
#define DM1_I34E_VIEW_WIDTH 224u
#define DM1_I34E_VIEW_HEIGHT 136u

/* ReDMCSB SELECTOR.C:271-274 declares G8250_DungeonView as a 7673-byte
 * 224x136 IMG3 source image. I34E SELECTOR has that array in its LZEXE
 * load module. Authenticate the installed executable before locating it. */
int DM1_V1_Selector_ExtractDungeonViewPc34(const char* selectorPath,
                                           unsigned char** outPixels,
                                           unsigned int* outPixelCount) {
    uint8_t* executable = NULL;
    size_t executableBytes = 0u;
    char md5[33];

    if (outPixels) *outPixels = NULL;
    if (outPixelCount) *outPixelCount = 0u;
    if (!selectorPath || !selectorPath[0] || !outPixels || !outPixelCount ||
        !asset_file_md5_hex(selectorPath, md5) ||
        strcmp(md5, DM1_I34E_SELECTOR_MD5) != 0 ||
        !asset_read_path_alloc(selectorPath, &executable, &executableBytes) ||
        executableBytes != DM1_I34E_SELECTOR_SIZE) {
        free(executable);
        return 0;
    }
    {
        const int result = DM1_V1_Selector_DecodeDungeonViewPc34(
            executable, (unsigned int)executableBytes,
            outPixels, outPixelCount);
        free(executable);
        return result;
    }
}

int DM1_V1_Selector_DecodeDungeonViewPc34(const unsigned char* executable,
                                          unsigned int executableBytes,
                                          unsigned char** outPixels,
                                          unsigned int* outPixelCount) {
    unsigned char* unpacked = NULL;
    unsigned int unpackedBytes = 0u;
    unsigned char* packed = NULL;
    unsigned char* pixels = NULL;
    unsigned int i;
    unsigned int imageOffset = 0u;
    unsigned int imageMatches = 0u;
    const unsigned int pixelCount = DM1_I34E_VIEW_WIDTH * DM1_I34E_VIEW_HEIGHT;
    const unsigned int packedBytes = pixelCount / 2u;
    if (outPixels) *outPixels = NULL;
    if (outPixelCount) *outPixelCount = 0u;
    if (!executable || executableBytes != DM1_I34E_SELECTOR_SIZE ||
        !outPixels || !outPixelCount ||
        !V1_LZEXE_UnpackLoadModule091(executable, executableBytes,
                                      &unpacked, &unpackedBytes)) {
        free(unpacked);
        return 0;
    }
    for (i = 0u; i + DM1_I34E_VIEW_SOURCE_BYTES <= unpackedBytes; ++i) {
        if (unpacked[i] == 0xe0u && unpacked[i + 1u] == 0x00u &&
            unpacked[i + 2u] == 0x88u && unpacked[i + 3u] == 0x00u) {
            imageOffset = i;
            imageMatches++;
        }
    }
    if (imageMatches != 1u) goto done;
    packed = (unsigned char*)calloc(packedBytes, 1u);
    pixels = (unsigned char*)malloc(pixelCount);
    if (!packed || !pixels) goto done;
    IMG3_Compat_ExpandFromSource(unpacked + imageOffset, packed);
    for (i = 0u; i < pixelCount; ++i) {
        const unsigned char byte = packed[i >> 1];
        pixels[i] = (unsigned char)((i & 1u) ? (byte & 0x0fu) : (byte >> 4));
    }
    *outPixels = pixels;
    *outPixelCount = pixelCount;
    pixels = NULL;
done:
    free(pixels);
    free(packed);
    free(unpacked);
    return *outPixels != NULL;
}

static void dm1_i34e_copy_rect(const unsigned char *source,
                               unsigned int source_width,
                               unsigned int source_height,
                               unsigned int source_x,
                               unsigned int source_y,
                               unsigned char *destination,
                               unsigned int destination_width,
                               unsigned int destination_height,
                               unsigned int destination_x,
                               unsigned int destination_y,
                               unsigned int width,
                               unsigned int height) {
    unsigned int row;
    if (!source || !destination || source_x >= source_width ||
        source_y >= source_height || destination_x >= destination_width ||
        destination_y >= destination_height) return;
    if (width > source_width - source_x) width = source_width - source_x;
    if (height > source_height - source_y) height = source_height - source_y;
    if (width > destination_width - destination_x)
        width = destination_width - destination_x;
    if (height > destination_height - destination_y)
        height = destination_height - destination_y;
    for (row = 0u; row < height; ++row) {
        memcpy(destination + (size_t)(destination_y + row) * destination_width +
                   destination_x,
               source + (size_t)(source_y + row) * source_width + source_x,
               width);
    }
}

int DM1_V1_Selector_ComposeFramePc34(
    const unsigned char* entrance, unsigned int entranceWidth,
    unsigned int entranceHeight,
    const unsigned char* leftDoor, unsigned int leftDoorWidth,
    unsigned int leftDoorHeight,
    const unsigned char* rightDoor, unsigned int rightDoorWidth,
    unsigned int rightDoorHeight,
    const unsigned char* selectorView, unsigned int selectorViewWidth,
    unsigned int selectorViewHeight, unsigned int doorStep,
    unsigned char* outFrame, unsigned int outFrameBytes) {
    unsigned char doors[233u * 161u];
    unsigned char view[233u * 161u];
    const unsigned int displacement = doorStep * 4u;
    unsigned int x, width;
    if (!entrance || entranceWidth != 320u || entranceHeight != 200u ||
        !leftDoor || leftDoorWidth == 0u || leftDoorHeight < 161u ||
        !rightDoor || rightDoorWidth == 0u || rightDoorHeight < 161u ||
        !selectorView || selectorViewWidth != 224u ||
        selectorViewHeight != 136u || !outFrame ||
        outFrameBytes < 320u * 200u || doorStep > 64u) return 0;

    memcpy(outFrame, entrance, 320u * 200u);
    /* SELECTOR.C:944-951 creates G8231 from the authentic entrance crop,
     * then places G8250 into its upper-left 224x136 aperture. */
    dm1_i34e_copy_rect(entrance, 320u, 200u, 0u, 33u, doors, 233u, 161u,
                       0u, 0u, 233u, 161u);
    dm1_i34e_copy_rect(selectorView, 224u, 136u, 0u, 0u, doors, 233u, 161u,
                       0u, 0u, 224u, 136u);
    memcpy(view, doors, sizeof(view));

    /* ReDMCSB SELECTOR.C F8360. Keep its independent source/destination
     * clipping and draw order: left shutter, right shutter, then the shared
     * door composite while the right panel passes the center. */
    if (leftDoorWidth > displacement) {
        width = leftDoorWidth - displacement;
        dm1_i34e_copy_rect(leftDoor, leftDoorWidth, leftDoorHeight,
                           displacement, 0u, view, 233u, 161u, 0u, 0u,
                           width, 161u);
    }
    if (rightDoorWidth > displacement) {
        width = rightDoorWidth - displacement;
        dm1_i34e_copy_rect(rightDoor, rightDoorWidth, rightDoorHeight,
                           0u, 0u, view, 233u, 161u,
                           displacement + 105u, 0u, width, 161u);
    }
    if (doorStep > 0u && rightDoorWidth >= displacement) {
        x = displacement < 105u ? 105u - displacement : 0u;
        width = displacement + 105u - x;
        if (width > 233u - x) width = 233u - x;
        dm1_i34e_copy_rect(doors, 233u, 161u, 0u, 0u,
                           view, 233u, 161u, x, 0u, width, 161u);
    }
    dm1_i34e_copy_rect(view, 233u, 161u, 0u, 0u,
                       outFrame, 320u, 200u, 0u, 33u, 233u, 161u);
    return 1;
}
