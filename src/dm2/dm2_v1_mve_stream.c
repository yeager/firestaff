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

static int mve_iterator_validate_opcode(uint8_t opcode, uint8_t version,
                                        uint16_t size)
{
    /* This is the opcode/version grammar in both admitted PC-DOS movies.
     * It is narrower than generic MVE on purpose: an unrecognised stream
     * must not reach a future presentation decoder as if it were DM2 media. */
    switch (opcode) {
    case 0x00u: return version == 0u && size == 0u;
    case 0x01u: return version == 0u && size == 0u;
    case 0x02u: return version == 0u && size == 6u;
    case 0x03u: return version == 0u && size == 8u;
    case 0x04u: return version == 0u && size == 0u;
    case 0x05u: return version == 2u && size == 8u;
    case 0x07u: return version == 1u && size == 6u;
    case 0x08u: return version == 0u && size >= 4u;
    case 0x09u: return version == 0u && size == 6u;
    case 0x0au: return version == 0u && size == 6u;
    case 0x0cu: return version == 0u && size == 766u;
    case 0x0fu: return version == 0u && size == 500u;
    case 0x11u: return version == 3u && size > 0u;
    case 0x13u: return version == 0u && size == 132u;
    default: return 0;
    }
}

int dm2_v1_mve_presentation_iterator_init(
    DM2_V1_MvePresentationIterator *iterator,
    const uint8_t *bytes, size_t byte_count)
{
    size_t offset;
    if (!iterator) return 0;
    memset(iterator, 0, sizeof(*iterator));
    if (!bytes || !mve_find_header(bytes, byte_count, &offset) ||
        offset > byte_count - 26u) return 0;
    iterator->bytes = bytes;
    iterator->byte_count = byte_count;
    iterator->offset = offset + 26u;
    iterator->initialized = 1;
    return 1;
}

int dm2_v1_mve_presentation_iterator_next(
    DM2_V1_MvePresentationIterator *iterator,
    DM2_V1_MvePresentation *out)
{
    DM2_V1_MvePresentation presentation;
    int have_map = 0;
    int have_video = 0;
    int have_transport13 = 0;

    if (out) memset(out, 0, sizeof(*out));
    if (!iterator || !out || !iterator->initialized || iterator->ended ||
        !iterator->bytes || iterator->offset > iterator->byte_count) return -1;
    memset(&presentation, 0, sizeof(presentation));
    presentation.presentation_index = iterator->presentation_count;
    if (iterator->timer_rate_us == 0u || iterator->timer_subdivision == 0u) {
        presentation.presentation_time_us = 0u;
    } else {
        presentation.presentation_time_us =
            (uint64_t)iterator->presentation_count *
            (uint64_t)iterator->timer_rate_us *
            (uint64_t)iterator->timer_subdivision;
    }
    while (iterator->offset < iterator->byte_count) {
        size_t chunk_end;
        uint16_t chunk_size;
        uint16_t chunk_type;
        if (iterator->offset + 4u > iterator->byte_count) return -1;
        chunk_size = mve_le16(iterator->bytes + iterator->offset);
        chunk_type = mve_le16(iterator->bytes + iterator->offset + 2u);
        iterator->offset += 4u;
        if ((size_t)chunk_size > iterator->byte_count - iterator->offset)
            return -1;
        if (chunk_type == 5u) {
            if (chunk_size != 0u) return -1;
            iterator->ended = 1;
            return have_map || have_video || have_transport13 ? -1 : 0;
        }
        chunk_end = iterator->offset + (size_t)chunk_size;
        while (iterator->offset < chunk_end) {
            const uint8_t *data;
            uint16_t size;
            uint8_t opcode;
            uint8_t version;
            size_t data_offset;
            if (chunk_end - iterator->offset < 4u) return -1;
            size = mve_le16(iterator->bytes + iterator->offset);
            opcode = iterator->bytes[iterator->offset + 2u];
            version = iterator->bytes[iterator->offset + 3u];
            iterator->offset += 4u;
            if ((size_t)size > chunk_end - iterator->offset ||
                !mve_iterator_validate_opcode(opcode, version, size)) return -1;
            data_offset = iterator->offset;
            data = iterator->bytes + data_offset;
            iterator->offset += size;
            switch (opcode) {
            case 0x00u:
                if (have_map || have_video || have_transport13) return -1;
                iterator->ended = 1;
                break;
            case 0x02u:
                if (iterator->timer_rate_us != 0u ||
                    mve_le32(data) == 0u || mve_le16(data + 4u) == 0u) return -1;
                iterator->timer_rate_us = mve_le32(data);
                iterator->timer_subdivision = mve_le16(data + 4u);
                break;
            case 0x05u:
                if (iterator->width != 0u || iterator->height != 0u ||
                    mve_le16(data) != 40u || mve_le16(data + 2u) != 25u) return -1;
                iterator->width = 320u;
                iterator->height = 200u;
                break;
            case 0x0cu:
                presentation.palette_offset = (uint32_t)data_offset;
                presentation.palette_size = size;
                break;
            case 0x0fu:
                if (have_map) return -1;
                presentation.code_map_offset = (uint32_t)data_offset;
                presentation.code_map_size = size;
                have_map = 1;
                break;
            case 0x11u:
                if (have_video) return -1;
                presentation.video_data_offset = (uint32_t)data_offset;
                presentation.video_data_size = size;
                presentation.video_version = version;
                have_video = 1;
                break;
            case 0x13u:
                if (have_transport13) return -1;
                presentation.transport13_offset = (uint32_t)data_offset;
                presentation.transport13_size = size;
                have_transport13 = 1;
                break;
            case 0x08u:
                presentation.audio_offset = (uint32_t)data_offset;
                presentation.audio_size = size;
                break;
            case 0x07u:
                if (!have_map || !have_video || !have_transport13 ||
                    iterator->timer_rate_us == 0u || iterator->width != 320u ||
                    iterator->height != 200u) return -1;
                presentation.valid = 1;
                ++iterator->presentation_count;
                *out = presentation;
                return 1;
            default:
                break;
            }
        }
        if (iterator->offset != chunk_end) return -1;
    }
    return -1;
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
