#include "dm2_v1_mve_stream.h"

#include <string.h>

static uint16_t mve_le16(const uint8_t *p)
{
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8u));
}

static uint32_t mve_le32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8u) |
           ((uint32_t)p[2] << 16u) | ((uint32_t)p[3] << 24u);
}

static uint32_t mve_hash_step(uint32_t hash, uint32_t value)
{
    return (hash ^ value) * 16777619u;
}

static int mve_find_header(const uint8_t *bytes, size_t byte_count,
                           size_t *out_offset)
{
    static const uint8_t signature[] = {
        'I','n','t','e','r','p','l','a','y',' ','M','V','E',' ','F','i','l','e',
        0x1au, 0x00u, 0x1au, 0x00u, 0x00u, 0x01u, 0x33u, 0x11u
    };
    size_t at;
    if (!bytes || !out_offset || byte_count < sizeof(signature)) return 0;
    for (at = 0u; at <= byte_count - sizeof(signature); ++at) {
        if (memcmp(bytes + at, signature, sizeof(signature)) == 0) {
            *out_offset = at;
            return 1;
        }
    }
    return 0;
}

int dm2_v1_mve_stream_parse(const uint8_t *bytes, size_t byte_count,
                            DM2_V1_MveStreamReceipt *out)
{
    DM2_V1_MveStreamReceipt receipt;
    size_t offset;
    int ended = 0;

    memset(&receipt, 0, sizeof(receipt));
    if (!bytes || !out || !mve_find_header(bytes, byte_count, &offset)) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.mve_offset = (uint32_t)offset;
    offset += 26u;
    while (offset < byte_count) {
        size_t chunk_end;
        uint16_t chunk_size;
        uint16_t chunk_type;
        if (offset + 4u > byte_count) break;
        chunk_size = mve_le16(bytes + offset);
        chunk_type = mve_le16(bytes + offset + 2u);
        offset += 4u;
        if ((size_t)chunk_size > byte_count - offset) break;
        chunk_end = offset + (size_t)chunk_size;
        /* The EOS opcode ends the playback chunk, then the format carries a
         * separate empty type-5 end chunk.  It belongs to the MVE member,
         * not to the surrounding DOS executable. */
        if (ended) {
            if (chunk_type != 5u || chunk_size != 0u) goto done;
            offset = chunk_end;
            ++receipt.chunk_count;
            break;
        }
        ++receipt.chunk_count;
        while (offset < chunk_end) {
            uint16_t size;
            uint8_t opcode;
            const uint8_t *data;
            if (chunk_end - offset < 4u) goto done;
            size = mve_le16(bytes + offset);
            opcode = bytes[offset + 2u];
            offset += 4u;
            if ((size_t)size > chunk_end - offset) goto done;
            data = bytes + offset;
            offset += size;
            ++receipt.opcode_count;
            switch (opcode) {
            case 0x00u: /* end of stream */
                if (size != 0u) goto done;
                ended = 1;
                break;
            case 0x02u: /* timer */
                if (size != 6u || receipt.timer_rate_us != 0u) goto done;
                receipt.timer_rate_us = mve_le32(data);
                receipt.timer_subdivision = mve_le16(data + 4u);
                if (receipt.timer_rate_us == 0u ||
                    receipt.timer_subdivision == 0u) goto done;
                break;
            case 0x05u: /* video buffer */
                if (size < 4u || receipt.width != 0u) goto done;
                /* MVE stores its 8x8 decoding-grid dimensions here.  The
                 * DM2 stream's 40x25 grid is the original 320x200 canvas. */
                if (mve_le16(data) > UINT16_MAX / 8u ||
                    mve_le16(data + 2u) > UINT16_MAX / 8u) goto done;
                receipt.width = (uint16_t)(mve_le16(data) * 8u);
                receipt.height = (uint16_t)(mve_le16(data + 2u) * 8u);
                if (receipt.width == 0u || receipt.height == 0u ||
                    receipt.width > 640u || receipt.height > 480u) goto done;
                break;
            case 0x07u: ++receipt.display_count; break;
            case 0x08u: ++receipt.audio_frame_count; break;
            case 0x0cu: ++receipt.palette_update_count; break;
            case 0x11u: ++receipt.video_frame_count; break;
            default: break;
            }
        }
        if (offset != chunk_end) goto done;
        if (chunk_type == 5u && !ended) goto done;
    }
done:
    if (ended && offset == byte_count && receipt.timer_rate_us != 0u &&
        receipt.width != 0u && receipt.height != 0u &&
        receipt.video_frame_count != 0u) {
        uint32_t hash = 2166136261u;
        receipt.mve_byte_count = (uint32_t)(byte_count - receipt.mve_offset);
        hash = mve_hash_step(hash, receipt.mve_offset);
        hash = mve_hash_step(hash, receipt.chunk_count);
        hash = mve_hash_step(hash, receipt.video_frame_count);
        hash = mve_hash_step(hash, receipt.timer_rate_us);
        hash = mve_hash_step(hash, ((uint32_t)receipt.width << 16u) | receipt.height);
        receipt.receipt_hash = hash;
        receipt.valid = hash != 0u;
    }
    *out = receipt;
    return receipt.valid;
}
