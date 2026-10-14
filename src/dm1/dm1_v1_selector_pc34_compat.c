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
