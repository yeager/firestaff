#ifndef NEXUS_V1_DGN_TEXTURE_DECODE_H
#define NEXUS_V1_DGN_TEXTURE_DECODE_H

#include <stdint.h>
#include "nexus_v1_dungeon.h"

typedef enum {
    NEXUS_V1_DGN_TEXTURE_DECODE_OK = 0,
    NEXUS_V1_DGN_TEXTURE_DECODE_BLOCKED_INPUT,
    NEXUS_V1_DGN_TEXTURE_DECODE_BLOCKED_DESCRIPTOR,
    NEXUS_V1_DGN_TEXTURE_DECODE_BLOCKED_BOUNDS,
    NEXUS_V1_DGN_TEXTURE_DECODE_OUTPUT_TOO_SMALL
} Nexus_V1_DgnTextureDecodeStatus;

typedef struct {
    int image_id;
    uint16_t encoding;
    uint16_t width;
    uint16_t height;
    uint32_t image_offset;
    uint32_t image_bytes;
    uint32_t palette_offset;
    int indexed4;
    int palette_entries;
    /* Always 0 here: source identity belongs to the hash-verified LEV corpus
     * admission route, not to this byte-format decoder. */
    int source_verified;
    int decoded;
    int palette_decoded;
    int direct_color_555;
} Nexus_V1_DgnTextureDecodeReceipt;

/* DMWeb Translation Kit / DMNDataFileDecoder.vbs Structure2 decoder.
 * Output is indexed nibbles expanded to one byte per pixel for encoding 08,
 * or big-endian Saturn 5-5-5 words for encoding 28.  DMWeb's Structure2
 * palette-ID reuse rule is resolved from the preceding descriptor list when
 * a descriptor has Palette offset 0. The receipt's decoded bit is format
 * evidence only; source_verified remains clear until a caller binds the same
 * bytes to a canonical LEV identity. */
int nexus_v1_dgn_texture_decode(const uint8_t *dgn, int dgn_size,
                                int image_id, uint8_t *pixels,
                                int pixel_capacity, uint16_t *palette,
                                int palette_capacity,
                                Nexus_V1_DgnTextureDecodeReceipt *receipt);

const char *nexus_v1_dgn_texture_decode_status_name(
    Nexus_V1_DgnTextureDecodeStatus status);

#endif
