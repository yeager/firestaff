#include "redmcsb_amiga_graphic_expand.h"

#include <stdlib.h>
#include <string.h>

static uint16_t read_be16(const uint8_t *bytes) {
    return (uint16_t)(((uint16_t)bytes[0] << 8) | bytes[1]);
}

int redmcsb_amiga_graphic_expand(const uint8_t *stream, size_t stream_size,
                                 uint8_t *indexed_pixels,
                                 size_t pixel_capacity,
                                 uint16_t *out_width,
                                 uint16_t *out_height) {
    size_t source = 0u;
    size_t pixel = 0u;
    size_t visible_total;
    size_t padded_total;
    size_t padded_width;
    uint8_t *padded_pixels = NULL;
    uint16_t width;
    uint16_t height;

    if (out_width) *out_width = 0u;
    if (out_height) *out_height = 0u;
    if (!stream || !indexed_pixels) return 0;
    /* ReDMCSB EXPAND.C F0466 skips optional FF81 prefix words before the
     * big-endian bitmap width and height. */
    while (source + 2u <= stream_size &&
           stream[source] == 0xffu && stream[source + 1u] == 0x81u) {
        source += 2u;
    }
    if (source + 4u > stream_size) return 0;
    width = read_be16(stream + source);
    height = read_be16(stream + source + 2u);
    source += 4u;
    if (width == 0u || height == 0u ||
        (size_t)height > SIZE_MAX / (size_t)width ||
        (visible_total = (size_t)width * (size_t)height) > pixel_capacity) {
        return 0;
    }
#if SIZE_MAX <= UINT16_MAX
    if ((size_t)width > SIZE_MAX - 15u) return 0;
#endif
    /* F0466 stores each output row in 16-pixel units: EXPAND.C rounds the
     * width with (width + 15) & ~15 before expanding. Compact only after the
     * complete padded bitmap has been produced. */
    padded_width = ((size_t)width + 15u) & ~(size_t)15u;
    if ((size_t)height > SIZE_MAX / padded_width ||
        (padded_total = padded_width * (size_t)height) > UINT16_MAX * 4096u) {
        return 0;
    }
    padded_pixels = (uint8_t *)calloc(padded_total, 1u);
    if (!padded_pixels) return 0;
    /* F0466 rounds only the destination row stride. The command cursor uses
     * the unrounded width; row transitions advance by the padded stride. */
    while (pixel < visible_total) {
        uint8_t command;
        uint8_t color;
        size_t count;
        if (source >= stream_size) goto fail;
        command = stream[source++];
        color = (uint8_t)(command & 0x0fu);
        /* EXPAND.C F0466's A?/E? commands advance the destination without
         * changing it. On a fresh bitmap the untouched pixels stay zero.
         * Their length uses the low nibble, optionally extended by bit 6;
         * it is not the normal one/two-byte run length. */
        if ((command & 0x80u) != 0u && (command & 0x10u) == 0u &&
            (command & 0x20u) != 0u) {
            size_t length_code = command & 0x0fu;
            if (source >= stream_size) goto fail;
            if ((command & 0x40u) != 0u) length_code |= 0x10u;
            if (length_code < 0x1du) {
                count = length_code + 1u;
            } else if (length_code == 0x1du) {
                count = (size_t)stream[source++] + 1u;
            } else if (length_code == 0x1eu) {
                count = 256u + (size_t)stream[source++] + 1u;
            } else {
                size_t high;
                size_t low;
                if (source + 2u > stream_size) goto fail;
                high = stream[source++];
                low = stream[source++];
                count = (high << 8u) + low + 1u;
            }
            if (count > visible_total - pixel) goto fail;
            pixel += count;
            continue;
        }
        if ((command & 0x80u) == 0u) {
            count = (size_t)(command >> 4u) + 1u;
        } else {
            if (source >= stream_size) goto fail;
            count = (size_t)stream[source++] + 1u;
            if ((command & 0x40u) != 0u) {
                if (source >= stream_size) goto fail;
                count = (count - 1u) * 256u + (size_t)stream[source++] + 1u;
            }
        }
        if (count == 0u || count > visible_total - pixel) goto fail;
        if ((command & 0x80u) == 0u || (command & 0x10u) == 0u) {
            while (count-- != 0u) {
                size_t destination = (pixel / (size_t)width) * padded_width +
                                    pixel % (size_t)width;
                padded_pixels[destination] = color;
                ++pixel;
            }
        } else if ((command & 0x20u) == 0u) {
            if ((count & 1u) != 0u) {
                size_t destination = (pixel / (size_t)width) * padded_width +
                                     pixel % (size_t)width;
                padded_pixels[destination] = color;
                ++pixel;
                --count;
            }
            while (count != 0u) {
                uint8_t packed;
                if (source >= stream_size) goto fail;
                packed = stream[source++];
                size_t destination = (pixel / (size_t)width) * padded_width +
                                     pixel % (size_t)width;
                padded_pixels[destination] = (uint8_t)(packed >> 4u);
                ++pixel;
                destination = (pixel / (size_t)width) * padded_width +
                              pixel % (size_t)width;
                padded_pixels[destination] = (uint8_t)(packed & 0x0fu);
                ++pixel;
                count -= 2u;
            }
        } else {
            if (pixel < (size_t)width) goto fail;
            while (count-- != 0u) {
                size_t destination = (pixel / (size_t)width) * padded_width +
                                     pixel % (size_t)width;
                padded_pixels[destination] =
                    padded_pixels[destination - padded_width];
                ++pixel;
            }
            /* ReDMCSB EXPAND.C F0466 does not stop after F10F7E copies the
             * previous-line run. Both copy paths (10DE2 and 10E0A) then call
             * F10EF2 once with d2 still holding the command's low nibble,
             * followed by F10E3E. That emits one additional color pixel. */
            if (pixel >= visible_total) goto fail;
            padded_pixels[(pixel / (size_t)width) * padded_width +
                          pixel % (size_t)width] = color;
            ++pixel;
        }
    }
    for (size_t row = 0u; row < height; ++row) {
        memcpy(indexed_pixels + row * (size_t)width,
               padded_pixels + row * padded_width, (size_t)width);
    }
    free(padded_pixels);
    if (out_width) *out_width = width;
    if (out_height) *out_height = height;
    return 1;

fail:
    free(padded_pixels);
    return 0;
}

int dm1_v1_amiga36_graphic_expand(const uint8_t *stream, size_t stream_size,
                                  uint8_t *indexed_pixels,
                                  size_t pixel_capacity,
                                  uint16_t *out_width,
                                  uint16_t *out_height) {
    return redmcsb_amiga_graphic_expand(stream, stream_size, indexed_pixels,
                                        pixel_capacity, out_width, out_height);
}
