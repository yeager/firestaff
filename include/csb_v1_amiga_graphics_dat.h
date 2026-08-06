#ifndef CSB_V1_AMIGA_GRAPHICS_DAT_H
#define CSB_V1_AMIGA_GRAPHICS_DAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* CSB Amiga GRAPHICS.DAT classifier and admission gate.
 *
 * The Amiga CSB releases use the DMCSB2 container format (0x8001 marker)
 * in big-endian byte order. The header word reads 0x80 0x01 (BE), followed
 * by a BE uint16 item count. Graphic items use IMG1 nibble RLE encoding
 * with BE width/height headers.
 *
 * Known variants and their item counts:
 *   61fbfd56887c94adc26888a9491c6611  CSB Amiga 3.1/3.3 Multilanguage  (749 items)
 *   291e1bc6803e3dc4b974c60117ca5d68  CSB Amiga 3.5 English
 *   cefaddfdf5651df2c91f61b5611a8362  CSB Amiga 3.5 Multilanguage
 *   21197b1d4994fd835c403d5a33dcac2b  CSB Amiga X.X/3.1 English
 *
 * Source references:
 *   ReDMCSB IMAGE1.C: IMG1 nibble RLE decoding
 *   dmweb.free.fr Data Files: CSB Amiga v3.1 ML = 749-item BE DMCSB2 */

#define CSB_AMIGA_GRAPHICS_CONTAINER_WORD  0x8001u
#define CSB_AMIGA_GRAPHICS_MIN_SIZE        300000u
#define CSB_AMIGA_GRAPHICS_MAX_SIZE        500000u

typedef enum {
    CSB_AMIGA_LANG_UNKNOWN = 0,
    CSB_AMIGA_LANG_EN      = 1,
    CSB_AMIGA_LANG_MULTI   = 2
} CSB_V1_AmigaLang;

typedef enum {
    CSB_AMIGA_VER_UNKNOWN = 0,
    CSB_AMIGA_VER_XX      = 1,
    CSB_AMIGA_VER_3_1     = 2,
    CSB_AMIGA_VER_3_3     = 3,
    CSB_AMIGA_VER_3_5     = 4
} CSB_V1_AmigaVersion;

typedef struct {
    int                  is_amiga;
    uint16_t             item_count;
    uint32_t             file_size;
    CSB_V1_AmigaLang     lang;
    CSB_V1_AmigaVersion  version;
    uint8_t              md5[16];
} CSB_V1_AmigaGraphicsReceipt;

typedef struct {
    uint16_t compressedByteCount;
    uint16_t decompressedByteCount;
    uint32_t dataOffset;
} CSB_V1_AmigaGraphicsItem;

int csb_v1_amiga_graphics_probe(const uint8_t *data, size_t size);

int csb_v1_amiga_graphics_receipt(const uint8_t *data, size_t size,
                                  CSB_V1_AmigaGraphicsReceipt *out);
int csb_v1_amiga_graphics_item(const uint8_t *data, size_t size,
                               uint16_t itemIndex,
                               CSB_V1_AmigaGraphicsItem *out);

#ifdef __cplusplus
}
#endif

#endif /* CSB_V1_AMIGA_GRAPHICS_DAT_H */
