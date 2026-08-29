/* Runtime-family admission from the supplied CSB Amiga preservation media.
 *
 * This is deliberately separate from the PC3.4 IMG3 startup decoder: the
 * Amiga retail file is a big-endian DMCSB2 container whose source records
 * are decoded by the native IMG1 consumer.  ZIP and ADF members are held in
 * RAM for their complete lifetime; no game member is materialized on disk. */

#include "csb_v1_amiga_graphics_dat.h"
#include "firestaff_amiga_adf.h"
#include "firestaff_zip_extract.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    unsigned char *bytes;
    size_t size;
} GraphicsCapture;

static int capture_graphics_dat(const char *name, const uint8_t *bytes,
                                size_t size, void *user_data)
{
    GraphicsCapture *capture = (GraphicsCapture *)user_data;
    if (!capture || !name || !bytes || strcmp(name, "Graphics.DAT") != 0)
        return 0;
    capture->bytes = (unsigned char *)malloc(size);
    if (!capture->bytes) return -1;
    memcpy(capture->bytes, bytes, size);
    capture->size = size;
    return 1;
}

static int decode_range(const unsigned char *graphics, size_t size,
                        unsigned int first, unsigned int last,
                        const char *family)
{
    unsigned char pixels[640u * 400u];
    unsigned int index;
    for (index = first; index <= last; ++index) {
        uint16_t width = 0u, height = 0u;
        if (!csb_v1_amiga_graphics_decode_item(graphics, size,
                                                (uint16_t)index, pixels,
                                                sizeof(pixels), &width,
                                                &height) || width == 0u ||
            height == 0u || width > 640u || height > 400u) {
            fprintf(stderr, "FAIL: Amiga %s C%03u did not decode\n",
                    family, index);
            return 0;
        }
    }
    return 1;
}

int main(void)
{
    const char *archive = getenv("FIRESTAFF_CSB_AMIGA_ADF_ARCHIVE");
    unsigned char *adf = NULL;
    size_t adf_size = 0u;
    GraphicsCapture graphics;
    CSB_V1_AmigaGraphicsReceipt receipt;
    int visited;

    if (!archive || !archive[0]) {
        puts("SKIP: CSB Amiga archive is not configured");
        return 0;
    }
    memset(&graphics, 0, sizeof(graphics));
    if (firestaff_zip_extract_by_suffix(archive, "Chaos Strikes Back (FTL) A.adf",
                                        &adf, &adf_size) != 0 || !adf) {
        fputs("FAIL: supplied CSB Amiga archive lacks disk A\n", stderr);
        return 1;
    }
    visited = firestaff_amiga_adf_visit_ofs_files(adf, adf_size,
                                                   capture_graphics_dat,
                                                   &graphics);
    free(adf);
    if (visited < 0 || !graphics.bytes ||
        csb_v1_amiga_graphics_receipt(graphics.bytes, graphics.size,
                                      &receipt) != 0 || !receipt.is_amiga) {
        free(graphics.bytes);
        fputs("FAIL: supplied CSB Amiga GRAPHICS.DAT was not admitted\n", stderr);
        return 1;
    }

    if (!decode_range(graphics.bytes, graphics.size, 17u, 17u, "inventory") ||
        !decode_range(graphics.bytes, graphics.size, 40u, 41u, "panel") ||
        !decode_range(graphics.bytes, graphics.size, 49u, 77u, "pit/field") ||
        !decode_range(graphics.bytes, graphics.size, 108u, 245u, "stairs") ||
        !decode_range(graphics.bytes, graphics.size, 259u, 384u, "wall") ||
        !decode_range(graphics.bytes, graphics.size, 385u, 438u, "floor") ||
        !decode_range(graphics.bytes, graphics.size, 439u, 453u, "door")) {
        free(graphics.bytes);
        return 1;
    }
    free(graphics.bytes);
    puts("PASS: CSB Amiga runtime graphics decode from ZIP/ADF in memory");
    return 0;
}
