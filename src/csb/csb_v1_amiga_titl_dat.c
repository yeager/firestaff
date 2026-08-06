#include "csb_v1_amiga_titl_dat.h"

#include <string.h>

/* ReDMCSB APPA.C:51-53 invokes SWSH followed by ANIM with FTL_TITL.  The
 * Amiga TITL.DAT item order and VBL durations below are independently
 * catalogued by Greatstone's CSB Amiga 3.1 EN/FR/GE extraction. */

static uint16_t csb_v1_rd16be(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

static int csb_v1_amiga_titl_take_record(const uint8_t *data, size_t size,
                                         size_t *offset,
                                         char first_tag, char second_tag,
                                         const uint8_t **payload,
                                         uint16_t *payload_size)
{
    size_t at = *offset;
    uint16_t length;

    if (size - at < 4u || data[at] != (uint8_t)first_tag ||
        data[at + 1u] != (uint8_t)second_tag) {
        return -1;
    }
    length = csb_v1_rd16be(data + at + 2u);
    at += 4u;
    if ((size_t)length > size - at) {
        return -1;
    }
    if (size - at < (size_t)length + 2u) {
        return -1;
    }
    *payload = data + at;
    *payload_size = length;
    /* ANIMSTEP's ByteCount starts after its attribute word. The two final
     * bytes below therefore complete that body; they are not a separate
     * record trailer. ReDMCSB ANIM.C F1179 advances by sizeof(ANIMSTEP) +
     * ByteCount, exactly to this next record boundary. */
    *offset = at + length + 2u;
    return 0;
}

int csb_v1_amiga_titl_dat_decode(const uint8_t *data, size_t size,
                                 CSB_V1_AmigaTitlSchedule *out)
{
    const uint8_t *payload;
    uint16_t payload_size;
    size_t offset = 0u;
    uint32_t total;
    uint16_t i;

    if (!data || !out) {
        return -1;
    }
    memset(out, 0, sizeof(*out));

    if (csb_v1_amiga_titl_take_record(data, size, &offset,
                                      'A', 'N', &payload, &payload_size) != 0 ||
        payload_size != 8u ||
        csb_v1_rd16be(payload + 2u) != CSB_V1_AMIGA_TITL_WIDTH ||
        csb_v1_rd16be(payload + 4u) != CSB_V1_AMIGA_TITL_HEIGHT ||
        csb_v1_rd16be(payload + 6u) != CSB_V1_AMIGA_TITL_BIT_DEPTH) {
        return -1;
    }

    if (csb_v1_amiga_titl_take_record(data, size, &offset,
                                      'P', 'L', &payload, &payload_size) != 0 ||
        payload_size != 66u) {
        return -1;
    }
    if (csb_v1_amiga_titl_take_record(data, size, &offset,
                                      'E', 'N', &payload, &payload_size) != 0 ||
        payload_size < 2u) {
        return -1;
    }

    total = csb_v1_rd16be(payload);
    out->initial_duration_vbl = (uint16_t)total;
    for (i = 0u; i < CSB_V1_AMIGA_TITL_DELTA_COUNT; ++i) {
        if (csb_v1_amiga_titl_take_record(data, size, &offset,
                                          'D', 'L',
                                          &payload, &payload_size) != 0 ||
            payload_size < 2u) {
            return -1;
        }
        out->delta_durations_vbl[i] = csb_v1_rd16be(payload);
        total += out->delta_durations_vbl[i];
    }
    if (csb_v1_amiga_titl_take_record(data, size, &offset,
                                      'D', 'O', &payload, &payload_size) != 0 ||
        payload_size != 0u || offset != size) {
        return -1;
    }

    out->width = CSB_V1_AMIGA_TITL_WIDTH;
    out->height = CSB_V1_AMIGA_TITL_HEIGHT;
    out->bit_depth = CSB_V1_AMIGA_TITL_BIT_DEPTH;
    out->delta_count = CSB_V1_AMIGA_TITL_DELTA_COUNT;
    out->total_duration_vbl = total;
    return 0;
}

int csb_v1_amiga_titl_dat_decode_palette(const uint8_t *data, size_t size,
                                         CSB_V1_AmigaTitlPalette *out)
{
    const uint8_t *payload;
    uint16_t payload_size;
    uint16_t color_count;
    uint16_t index;
    size_t offset = 0u;

    if (!data || !out ||
        csb_v1_amiga_titl_take_record(data, size, &offset,
                                      'A', 'N', &payload, &payload_size) != 0 ||
        payload_size != 8u ||
        csb_v1_amiga_titl_take_record(data, size, &offset,
                                      'P', 'L', &payload, &payload_size) != 0 ||
        payload_size < 2u || payload_size > 66u) {
        return -1;
    }
    /* ReDMCSB ANIM.C F1181 sees PL_DATA at the byte after Attribute. Its
     * ByteCount body includes the final two bytes immediately before EN. */
    color_count = csb_v1_rd16be(payload + 2u);
    if (color_count > 16u || (size_t)color_count * 4u + 2u != payload_size) {
        return -1;
    }
    memset(out, 0, sizeof(*out));
    for (index = 0u; index < color_count; ++index) {
        const uint8_t *color = payload + 4u + (size_t)index * 4u;
        if (color[0] >= 16u || color[1] >= 16u || color[2] >= 16u ||
            color[3] >= 16u) {
            return -1;
        }
        out->rgb4[color[0]][0] = color[1];
        out->rgb4[color[0]][1] = color[2];
        out->rgb4[color[0]][2] = color[3];
    }
    out->color_count = color_count;
    return 0;
}

/* ReDMCSB EXPAND.C F0466, built by GRF1.C for MEDIA619_A31E_A31M, expands
 * this command stream into four planar words.  The base EN destination is
 * cleared by ANIM.C F1219, so writing the resulting colour index directly
 * produces the same indexed pixels without host-created source data. */
static int csb_v1_amiga_grf1_decode_base(const uint8_t *stream,
                                         size_t stream_size,
                                         uint8_t *pixels, size_t capacity,
                                         CSB_V1_AmigaTitlFrameReceipt *out)
{
    uint16_t width;
    uint16_t height;
    size_t total;
    size_t source = 4u;
    size_t pixel = 0u;

    if (!stream || !pixels || stream_size < 5u) return 0;
    width = csb_v1_rd16be(stream);
    height = csb_v1_rd16be(stream + 2u);
    if (width != CSB_V1_AMIGA_TITL_WIDTH ||
        height != CSB_V1_AMIGA_TITL_HEIGHT ||
        height > SIZE_MAX / width ||
        (total = (size_t)width * height) > capacity) {
        return 0;
    }
    memset(pixels, 0, total);
    while (pixel < total) {
        uint8_t command;
        uint8_t color;
        size_t count;

        if (source >= stream_size) return 0;
        command = stream[source++];
        color = (uint8_t)(command & 0x0fu);
        if ((command & 0x80u) == 0u) {
            count = (size_t)(command >> 4u) + 1u;
        } else {
            if (source >= stream_size) return 0;
            count = (size_t)stream[source++] + 1u;
            if ((command & 0x40u) != 0u) {
                if (source >= stream_size) return 0;
                count = (count - 1u) * 256u + (size_t)stream[source++] + 1u;
            }
        }
        if (count == 0u || count > total - pixel) return 0;
        /* EXPAND.C only interprets bits 4 and 5 as literal/copy selectors
         * in its multi-byte-command branch.  Short commands such as 0x17
         * are still ordinary colour-7 fills. */
        if ((command & 0x80u) == 0u || (command & 0x10u) == 0u) {
            memset(pixels + pixel, color, count);
            pixel += count;
        } else if ((command & 0x20u) == 0u) {
            if ((count & 1u) != 0u) {
                pixels[pixel++] = color;
                --count;
            }
            while (count != 0u) {
                uint8_t packed;
                if (source >= stream_size) return 0;
                packed = stream[source++];
                pixels[pixel++] = (uint8_t)(packed >> 4u);
                pixels[pixel++] = (uint8_t)(packed & 0x0fu);
                count -= 2u;
            }
        } else {
            size_t previous = (size_t)width;
            if (pixel < previous || count > total - pixel) return 0;
            while (count-- != 0u) {
                pixels[pixel] = pixels[pixel - previous];
                ++pixel;
            }
        }
    }
    if (out) {
        out->width = width;
        out->height = height;
        out->source_bytes_consumed = source;
        out->decoded_pixel_count = pixel;
    }
    return 1;
}

int csb_v1_amiga_titl_dat_decode_initial_frame(
    const uint8_t *data, size_t size, uint8_t *indexed_pixels,
    size_t indexed_pixel_capacity, CSB_V1_AmigaTitlFrameReceipt *out)
{
    const uint8_t *payload;
    uint16_t payload_size;
    size_t offset = 0u;

    if (out) memset(out, 0, sizeof(*out));
    if (!data || !indexed_pixels ||
        csb_v1_amiga_titl_take_record(data, size, &offset,
                                      'A', 'N', &payload, &payload_size) != 0 ||
        payload_size != 8u ||
        csb_v1_amiga_titl_take_record(data, size, &offset,
                                      'P', 'L', &payload, &payload_size) != 0 ||
        payload_size != 66u ||
        csb_v1_amiga_titl_take_record(data, size, &offset,
                                      'E', 'N', &payload, &payload_size) != 0 ||
        payload_size < 4u) {
        return 0;
    }
    /* F1204 passes only the address after ANIMSTEP to the GRF1 expander;
     * unlike the ANIM reader, it does not pass ByteCount.  The final EN
     * command therefore deliberately consumes six bytes that begin the
     * following DL record before reaching the 320x200 destination limit.
     * Keep the whole TITL.DAT tail available, while still validating EN's
     * declared record above.  ReDMCSB ANIM.C F1182 / EXPAND.C F0466. */
    return csb_v1_amiga_grf1_decode_base(payload + 2u,
                                          size - (size_t)((payload + 2u) - data),
                                          indexed_pixels,
                                          indexed_pixel_capacity, out);
}
