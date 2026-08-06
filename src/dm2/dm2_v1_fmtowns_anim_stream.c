#include "dm2_v1_fmtowns_anim_stream.h"

#include <string.h>

static uint16_t read_be16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8u) | p[1]);
}

static uint32_t fnv1a32(const uint8_t *data, size_t size)
{
    uint32_t hash = 2166136261u;
    size_t index;
    for (index = 0u; index < size; ++index) {
        hash ^= data[index];
        hash *= 16777619u;
    }
    return hash;
}

static uint8_t pixel_get(const uint8_t *pixels, uint32_t offset)
{
    const uint8_t packed = pixels[offset >> 1u];
    return (uint8_t)((offset & 1u) ? (packed & 0x0fu) : (packed >> 4u));
}

static void pixel_set(uint8_t *pixels, uint32_t offset, uint8_t color)
{
    uint8_t *packed = pixels + (offset >> 1u);
    if (offset & 1u) *packed = (uint8_t)((*packed & 0xf0u) | color);
    else *packed = (uint8_t)((*packed & 0x0fu) | (color << 4u));
}

/* SKWIN/SkWinCore.cpp:1110 `ANIM_DECODE_IMG1`, source-locked to 0759:0330.
 * `src_size` is bounded by the enclosing original EN/DL record. */
static int decode_img1(const uint8_t *src, size_t src_size,
                       uint8_t *pixels, size_t pixel_bytes,
                       uint16_t expected_width, uint16_t expected_height,
                       uint32_t *out_commands)
{
    uint16_t width;
    uint16_t height;
    uint32_t even_width;
    uint32_t total_pixels;
    uint32_t destination = 0u;
    size_t source = 4u;
    uint32_t commands = 0u;

    if (!src || !pixels || src_size < 5u) return 0;
    width = read_be16(src);
    height = read_be16(src + 2u);
    even_width = ((uint32_t)width + 1u) & ~1u;
    total_pixels = even_width * height;
    if (width != expected_width || height != expected_height ||
        total_pixels / 2u > pixel_bytes) return 0;

    while (destination < total_pixels) {
        uint8_t op;
        uint32_t count;
        uint32_t index;
        if (source >= src_size) return 0;
        op = src[source++];
        ++commands;
        if ((op & 0x80u) == 0u) {
            count = (uint32_t)(op >> 4u) + 1u;
            if (count > total_pixels - destination) return 0;
            for (index = 0u; index < count; ++index)
                pixel_set(pixels, destination + index, (uint8_t)(op & 0x0fu));
            destination += count;
            continue;
        }
        switch (op & 0x30u) {
        case 0x00u:
        case 0x10u:
        case 0x30u:
            if (op & 0x40u) {
                if (source + 2u > src_size) return 0;
                count = (uint32_t)read_be16(src + source) + 1u;
                source += 2u;
            } else {
                if (source >= src_size) return 0;
                count = (uint32_t)src[source++] + 1u;
            }
            if (count > total_pixels - destination) return 0;
            if ((op & 0x30u) == 0x00u) {
                for (index = 0u; index < count; ++index)
                    pixel_set(pixels, destination + index, (uint8_t)(op & 0x0fu));
                destination += count;
            } else if ((op & 0x30u) == 0x10u) {
                if (count & 1u) {
                    pixel_set(pixels, destination++, (uint8_t)(op & 0x0fu));
                    --count;
                }
                if (source + count / 2u > src_size) return 0;
                for (index = 0u; index < count; ++index) {
                    uint8_t packed = src[source + (index >> 1u)];
                    pixel_set(pixels, destination + index,
                              (uint8_t)((index & 1u) ? (packed & 0x0fu)
                                                     : (packed >> 4u)));
                }
                source += count / 2u;
                destination += count;
            } else {
                if (destination < even_width || count >= total_pixels - destination)
                    return 0;
                /* 0759:02c6 uses forward MOVS semantics.  Per-pixel forward
                 * copying preserves that behaviour even if ranges overlap. */
                for (index = 0u; index < count; ++index)
                    pixel_set(pixels, destination + index,
                              pixel_get(pixels, destination - even_width + index));
                destination += count;
                pixel_set(pixels, destination++, (uint8_t)(op & 0x0fu));
            }
            break;
        case 0x20u:
            count = ((uint32_t)(op >> 2u) & 16u) | (op & 15u);
            if (count == 0x1du) {
                if (source >= src_size) return 0;
                count = (uint32_t)src[source++] + 1u;
            } else if (count == 0x1eu) {
                if (source >= src_size) return 0;
                count = (uint32_t)src[source++] + 0x101u;
            } else if (count == 0x1fu) {
                if (source + 2u > src_size) return 0;
                count = (uint32_t)read_be16(src + source) + 1u;
                source += 2u;
            } else {
                ++count;
            }
            if (count > total_pixels - destination) return 0;
            /* 0759:0310 displays the already-decoded run but does not alter
             * the retained 4bpp canvas. */
            destination += count;
            break;
        default:
            return 0;
        }
    }
    if (out_commands) *out_commands = commands;
    return 1;
}

static int add_chunk(DM2_V1_FmtownsAnimStreamReceipt *receipt,
                     uint16_t tag)
{
    switch (tag) {
    case 0x414eu: ++receipt->an_count; break; /* AN */
    case 0x504cu: ++receipt->pl_count; break; /* PL */
    case 0x454eu: ++receipt->en_count; break; /* EN */
    case 0x444cu: ++receipt->dl_count; break; /* DL */
    case 0x5344u: ++receipt->sd_count; break; /* SD */
    case 0x4252u: ++receipt->br_count; break; /* BR */
    case 0x534fu: ++receipt->so_count; break; /* SO */
    case 0x444fu: ++receipt->do_count; break; /* DO */
    case 0x464fu: ++receipt->fo_count; break; /* FO */
    case 0x4e45u: ++receipt->ne_count; break; /* NE */
    case 0x424eu: ++receipt->bn_count; break; /* BN */
    default: return 0;
    }
    ++receipt->chunk_count;
    return 1;
}

int dm2_v1_fmtowns_anim_stream_parse(
    const uint8_t *data, size_t data_size,
    DM2_V1_FmtownsAnimStreamReceipt *out)
{
    DM2_V1_FmtownsAnimStreamReceipt receipt;
    size_t offset = 0u;

    memset(&receipt, 0, sizeof(receipt));
    if (!data || data_size < 6u || data_size > UINT32_MAX) {
        if (out) *out = receipt;
        return 0;
    }
    while (offset + 6u <= data_size) {
        const uint16_t tag = read_be16(data + offset);
        const size_t payload_size = read_be16(data + offset + 2u);
        const size_t record_size = 6u + payload_size;

        if (record_size > data_size - offset || !add_chunk(&receipt, tag)) {
            if (out) *out = receipt;
            return 0;
        }
        if (tag == 0x414eu) {
            /* AN: two reserved bytes, width, height, bit depth, trailer. */
            if (payload_size != 8u) {
                if (out) *out = receipt;
                return 0;
            }
            /* END has two identical AN phases.  Preserve the first header
             * as the stream canvas identity, but reject a later phase that
             * would silently change it. */
            if (receipt.an_count == 1u) {
                receipt.width = read_be16(data + offset + 6u);
                receipt.height = read_be16(data + offset + 8u);
                receipt.bit_depth = read_be16(data + offset + 10u);
                receipt.an_trailer = read_be16(data + offset + 12u);
            } else if (receipt.width != read_be16(data + offset + 6u) ||
                       receipt.height != read_be16(data + offset + 8u) ||
                       receipt.bit_depth != read_be16(data + offset + 10u) ||
                       receipt.an_trailer != read_be16(data + offset + 12u)) {
                if (out) *out = receipt;
                return 0;
            }
        }
        offset += record_size;
    }
    if (offset != data_size || receipt.an_count == 0u || receipt.do_count != 1u) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.byte_count = (uint32_t)data_size;
    if (out) *out = receipt;
    return 1;
}

int dm2_v1_fmtowns_anim_stream_is_hme242_swoosh(
    const DM2_V1_FmtownsAnimStreamReceipt *r)
{
    return r && r->valid && r->width == 0u && r->height == 0u &&
           r->bit_depth == 4u && r->an_trailer == 2u &&
           r->chunk_count == 22u && r->an_count == 1u && r->pl_count == 1u &&
           r->en_count == 1u && r->dl_count == 18u && r->do_count == 1u &&
           r->sd_count == 0u && r->br_count == 0u && r->so_count == 0u &&
           r->fo_count == 0u && r->ne_count == 0u && r->bn_count == 0u;
}

int dm2_v1_fmtowns_anim_stream_is_hme242_title(
    const DM2_V1_FmtownsAnimStreamReceipt *r)
{
    return r && r->valid && r->width == 320u && r->height == 200u &&
           r->bit_depth == 4u && r->an_trailer == 3u &&
           r->chunk_count == 235u && r->an_count == 1u && r->pl_count == 1u &&
           r->en_count == 1u && r->dl_count == 224u && r->sd_count == 1u &&
           r->br_count == 1u && r->so_count == 5u && r->do_count == 1u &&
           r->fo_count == 0u && r->ne_count == 0u && r->bn_count == 0u;
}

int dm2_v1_fmtowns_anim_stream_is_hme242_end(
    const DM2_V1_FmtownsAnimStreamReceipt *r)
{
    return r && r->valid && r->width == 320u && r->height == 200u &&
           r->bit_depth == 4u && r->an_trailer == 3u &&
           r->chunk_count == 401u && r->an_count == 2u && r->pl_count == 5u &&
           r->en_count == 3u && r->dl_count == 382u && r->br_count == 2u &&
           r->do_count == 1u && r->fo_count == 2u && r->ne_count == 2u &&
           r->bn_count == 2u && r->sd_count == 0u && r->so_count == 0u;
}

int dm2_v1_fmtowns_anim_stream_decode_frame(
    const uint8_t *data, size_t data_size, uint32_t requested_frame,
    uint8_t *out_pixels, size_t out_pixel_capacity,
    DM2_V1_FmtownsAnimFrameReceipt *out)
{
    typedef struct {
        size_t resume_offset;
        uint16_t remaining;
    } FmtownsAnimLoop;
    DM2_V1_FmtownsAnimFrameReceipt receipt;
    DM2_V1_FmtownsAnimStreamReceipt stream;
    size_t offset = 0u;
    uint32_t frame = 0u;
    uint32_t dispatch_count = 0u;
    FmtownsAnimLoop loops[8];
    unsigned int loop_depth = 0u;
    size_t canvas_bytes;
    uint16_t canvas_width;
    uint16_t canvas_height;

    memset(&receipt, 0, sizeof(receipt));
    if (!data || !out_pixels ||
        !dm2_v1_fmtowns_anim_stream_parse(data, data_size, &stream) ||
        stream.bit_depth != 4u) {
        if (out) *out = receipt;
        return 0;
    }
    canvas_width = stream.width;
    canvas_height = stream.height;
    canvas_bytes = 0u;
    if (canvas_width != 0u || canvas_height != 0u) {
        if (canvas_width == 0u || canvas_height == 0u) {
            if (out) *out = receipt;
            return 0;
        }
        canvas_bytes = (((size_t)canvas_width + 1u) & ~(size_t)1u) *
                       canvas_height / 2u;
        if (canvas_bytes > out_pixel_capacity) {
            if (out) *out = receipt;
            return 0;
        }
        memset(out_pixels, 0, canvas_bytes);
    }
    while (offset + 6u <= data_size) {
        const uint16_t tag = read_be16(data + offset);
        const size_t payload_size = read_be16(data + offset + 2u);
        const uint16_t attribute = read_be16(data + offset + 4u);
        const size_t next_offset = offset + payload_size + 6u;
        const uint8_t *image;
        size_t image_size;
        uint32_t commands = 0u;

        if (payload_size + 6u > data_size - offset) break;
        /* SKWIN's 0759:0F64 interpreter can revisit a bounded FO/NE body.
         * The original code admits only loop counts 1..9 after decrement;
         * retain a small source-shaped stack and a hard dispatch bound so a
         * damaged source stream cannot turn the RAM-only reader into a host
         * animation loop. Source: SKProject skibmio.cpp 0759:1160-121E. */
        if (++dispatch_count > stream.chunk_count * 9u + 8u) {
            if (out) *out = receipt;
            return 0;
        }
        if (tag == 0x464fu) { /* FO */
            if (loop_depth >= sizeof(loops) / sizeof(loops[0])) {
                if (out) *out = receipt;
                return 0;
            }
            loops[loop_depth].resume_offset = next_offset;
            loops[loop_depth].remaining = attribute;
            ++loop_depth;
            offset = next_offset;
            continue;
        }
        if (tag == 0x4e45u) { /* NE */
            uint16_t remaining;
            if (loop_depth == 0u) {
                if (out) *out = receipt;
                return 0;
            }
            remaining = (uint16_t)(loops[loop_depth - 1u].remaining - 1u);
            loops[loop_depth - 1u].remaining = remaining;
            if (remaining > 0u && remaining < 10u) {
                offset = loops[loop_depth - 1u].resume_offset;
            } else {
                --loop_depth;
                offset = next_offset;
            }
            continue;
        }
        if (tag != 0x454eu && tag != 0x444cu) {
            offset = next_offset;
            continue;
        }
        /* SkWinCore.cpp 0759:0F64: normal records have a two-byte duration
         * before IMG1; FF81 records have a two-byte marker as well. */
        if (payload_size < 6u) {
            if (out) *out = receipt;
            return 0;
        }
        image = data + offset + 6u;
        /* 0759:0F64 hands IMG1 a pointer into one contiguous file arena,
         * not an isolated record copy.  Some retail records deliberately
         * finish their final run across the following record boundary.  Keep
         * the file-wide bound while preserving that original decoder ABI. */
        image_size = data_size - (size_t)(image - data);
        if (image[0] == 0xffu && image[1] == 0x81u) {
            if (image_size < 6u) {
                if (out) *out = receipt;
                return 0;
            }
            image += 2u;
            image_size -= 2u;
        }
        /* HME-242 SWOOSH has AN 0x0 but its first EN record is an IMG1
         * 320x200 canvas, followed by equally-sized deltas.  SKWIN's
         * ANIM_DECODE_IMG1 reads dimensions from every IMG1 payload; infer
         * only this absent AN canvas from that first source record. */
        if (canvas_width == 0u && canvas_height == 0u) {
            if (image_size < 4u) {
                if (out) *out = receipt;
                return 0;
            }
            canvas_width = read_be16(image);
            canvas_height = read_be16(image + 2u);
            canvas_bytes = (((size_t)canvas_width + 1u) & ~(size_t)1u) *
                           canvas_height / 2u;
            if (canvas_width == 0u || canvas_height == 0u ||
                canvas_bytes > out_pixel_capacity) {
                if (out) *out = receipt;
                return 0;
            }
            memset(out_pixels, 0, canvas_bytes);
        }
        if (!decode_img1(image, image_size, out_pixels, canvas_bytes,
                         canvas_width, canvas_height, &commands)) {
            if (out) *out = receipt;
            return 0;
        }
        if (frame == requested_frame) {
            receipt.valid = 1;
            receipt.width = canvas_width;
            receipt.height = canvas_height;
            receipt.bit_depth = stream.bit_depth;
            receipt.requested_frame = requested_frame;
            receipt.decoded_frame_count = frame + 1u;
            receipt.display_duration = read_be16(data + offset + 4u);
            receipt.source_bytes_consumed = (uint32_t)next_offset;
            receipt.compressed_command_count = commands;
            receipt.output_fnv1a = fnv1a32(out_pixels, canvas_bytes);
            if (out) *out = receipt;
            return 1;
        }
        ++frame;
        offset = next_offset;
    }
    if (out) *out = receipt;
    return 0;
}

static int dm2_v1_fmtowns_anim_stream_decode_palette_record(
    const uint8_t *data, size_t data_size, size_t offset,
    DM2_V1_FmtownsAnimPaletteReceipt *out)
{
    DM2_V1_FmtownsAnimPaletteReceipt receipt;
    size_t payload_size;
    const uint8_t *payload;
    uint16_t count;
    uint16_t index;

    memset(&receipt, 0, sizeof(receipt));
    if (!data || offset + 6u > data_size ||
        read_be16(data + offset) != 0x504cu) {
        if (out) *out = receipt;
        return 0;
    }
    payload_size = read_be16(data + offset + 2u);
    payload = data + offset + 6u;
    /* SkWinCore.cpp 0759:1018 reads 64 bytes from record+8: the first two
     * payload bytes are the count and the sixteen entries follow. */
    if (payload_size + 6u > data_size - offset || payload_size < 2u) {
        if (out) *out = receipt;
        return 0;
    }
    count = read_be16(payload);
    if (count != DM2_V1_FMTOWNS_ANIM_PALETTE_COLORS ||
        payload_size < 2u + (size_t)count * 4u) {
        if (out) *out = receipt;
        return 0;
    }
    for (index = 0u; index < count; ++index) {
        const uint8_t *entry = payload + 2u + (size_t)index * 4u;
        if (entry[0] >= DM2_V1_FMTOWNS_ANIM_PALETTE_COLORS) {
            if (out) *out = receipt;
            return 0;
        }
        receipt.rgb4[entry[0]][0] = entry[1];
        receipt.rgb4[entry[0]][1] = entry[2];
        receipt.rgb4[entry[0]][2] = entry[3];
    }
    receipt.valid = 1;
    receipt.color_count = count;
    receipt.source_record_offset = (uint32_t)offset;
    receipt.output_fnv1a = fnv1a32((const uint8_t *)receipt.rgb4,
                                   sizeof(receipt.rgb4));
    if (out) *out = receipt;
    return receipt.valid;
}

int dm2_v1_fmtowns_anim_stream_decode_palette(
    const uint8_t *data, size_t data_size,
    DM2_V1_FmtownsAnimPaletteReceipt *out)
{
    DM2_V1_FmtownsAnimStreamReceipt stream;
    DM2_V1_FmtownsAnimPaletteReceipt receipt;
    size_t offset = 0u;

    memset(&receipt, 0, sizeof(receipt));
    if (!data || !dm2_v1_fmtowns_anim_stream_parse(data, data_size, &stream)) {
        if (out) *out = receipt;
        return 0;
    }
    while (offset + 6u <= data_size) {
        const size_t payload_size = read_be16(data + offset + 2u);
        if (payload_size + 6u > data_size - offset) break;
        if (read_be16(data + offset) == 0x504cu &&
            !dm2_v1_fmtowns_anim_stream_decode_palette_record(
                data, data_size, offset, &receipt)) {
            if (out) *out = receipt;
            return 0;
        }
        offset += payload_size + 6u;
    }
    if (out) *out = receipt;
    return receipt.valid;
}

int dm2_v1_fmtowns_anim_stream_decode_palette_for_frame(
    const uint8_t *data, size_t data_size, uint32_t requested_frame,
    DM2_V1_FmtownsAnimPaletteReceipt *out)
{
    typedef struct {
        size_t resume_offset;
        uint16_t remaining;
    } FmtownsAnimLoop;
    DM2_V1_FmtownsAnimStreamReceipt stream;
    DM2_V1_FmtownsAnimPaletteReceipt receipt;
    FmtownsAnimLoop loops[8];
    size_t offset = 0u;
    uint32_t frame = 0u;
    uint32_t dispatch_count = 0u;
    unsigned int loop_depth = 0u;

    memset(&receipt, 0, sizeof(receipt));
    if (!data || !dm2_v1_fmtowns_anim_stream_parse(data, data_size, &stream)) {
        if (out) *out = receipt;
        return 0;
    }
    while (offset + 6u <= data_size) {
        const uint16_t tag = read_be16(data + offset);
        const size_t payload_size = read_be16(data + offset + 2u);
        const uint16_t attribute = read_be16(data + offset + 4u);
        const size_t next_offset = offset + payload_size + 6u;
        if (payload_size + 6u > data_size - offset ||
            ++dispatch_count > stream.chunk_count * 9u + 8u) {
            if (out) *out = receipt;
            return 0;
        }
        if (tag == 0x464fu) { /* FO */
            if (loop_depth >= sizeof(loops) / sizeof(loops[0])) {
                if (out) *out = receipt;
                return 0;
            }
            loops[loop_depth].resume_offset = next_offset;
            loops[loop_depth].remaining = attribute;
            ++loop_depth;
            offset = next_offset;
            continue;
        }
        if (tag == 0x4e45u) { /* NE */
            uint16_t remaining;
            if (loop_depth == 0u) {
                if (out) *out = receipt;
                return 0;
            }
            remaining = (uint16_t)(loops[loop_depth - 1u].remaining - 1u);
            loops[loop_depth - 1u].remaining = remaining;
            if (remaining > 0u && remaining < 10u) {
                offset = loops[loop_depth - 1u].resume_offset;
            } else {
                --loop_depth;
                offset = next_offset;
            }
            continue;
        }
        if (tag == 0x504cu &&
            !dm2_v1_fmtowns_anim_stream_decode_palette_record(
                data, data_size, offset, &receipt)) {
            if (out) *out = receipt;
            return 0;
        }
        if (tag == 0x454eu || tag == 0x444cu) { /* EN / DL */
            if (frame == requested_frame) {
                if (out) *out = receipt;
                return receipt.valid;
            }
            ++frame;
        }
        offset = next_offset;
    }
    if (out) *out = receipt;
    return 0;
}

int dm2_v1_fmtowns_anim_stream_decode_title_sound(
    const uint8_t *data, size_t data_size,
    DM2_V1_FmtownsAnimSoundReceipt *out)
{
    DM2_V1_FmtownsAnimStreamReceipt stream;
    DM2_V1_FmtownsAnimSoundReceipt receipt;
    size_t offset = 0u;
    uint32_t frame_count = 0u;
    int have_sound = 0;

    memset(&receipt, 0, sizeof(receipt));
    if (!data ||
        !dm2_v1_fmtowns_anim_stream_parse(data, data_size, &stream) ||
        !dm2_v1_fmtowns_anim_stream_is_hme242_title(&stream)) {
        if (out) *out = receipt;
        return 0;
    }

    while (offset + 6u <= data_size) {
        const uint16_t tag = read_be16(data + offset);
        const size_t payload_size = read_be16(data + offset + 2u);
        const uint16_t attribute = read_be16(data + offset + 4u);
        const uint8_t *payload = data + offset + 6u;

        if (payload_size > data_size - offset - 6u) break;
        if (tag == 0x5344u) { /* SD */
            const uint16_t sample_count =
                payload_size >= 2u ? read_be16(payload) : 0u;
            /* SKWIN 0759:0E33 stores each SD at a zero-based array slot;
             * TITLE's one SD uses slot zero.  SO's documented index is
             * one-based and therefore selects this source span with 0001. */
            if (have_sound || attribute != 0u || payload_size < 2u ||
                (size_t)sample_count + 2u != payload_size) {
                if (out) *out = receipt;
                return 0;
            }
            have_sound = 1;
            receipt.source_record_offset = (uint32_t)offset;
            receipt.sample_count = sample_count;
            receipt.samples = (const int8_t *)(const void *)(payload + 2u);
            receipt.sample_fnv1a = fnv1a32(payload + 2u, sample_count);
        } else if (tag == 0x534fu) { /* SO */
            DM2_V1_FmtownsAnimSoundEventReceipt *event;
            /* DMWeb Animations: SO carries a one-based SD/SF/TD index plus
             * left volume, right volume and BE frequency.  DMWeb also calls
             * TITLE's 03E8 frequency invalid; SKWIN 0759:0EF0 supplies the
             * effective fixed 5500 Hz argument to _0759_0739. */
            if (!have_sound || payload_size != 4u || attribute != 1u ||
                receipt.event_count >= DM2_V1_FMTOWNS_TITLE_SOUND_EVENT_COUNT) {
                if (out) *out = receipt;
                return 0;
            }
            event = &receipt.events[receipt.event_count++];
            event->source_record_offset = (uint32_t)offset;
            event->preceding_frame_count = frame_count;
            event->sound_index = attribute;
            event->left_volume = payload[0];
            event->right_volume = payload[1];
            event->source_frequency_hz = read_be16(payload + 2u);
            event->player_frequency_hz = 5500u;
        } else if (tag == 0x444cu || tag == 0x454eu) { /* DL / EN */
            ++frame_count;
        }
        offset += payload_size + 6u;
    }
    if (!have_sound || receipt.event_count !=
                           DM2_V1_FMTOWNS_TITLE_SOUND_EVENT_COUNT) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.valid = 1;
    if (out) *out = receipt;
    return 1;
}
