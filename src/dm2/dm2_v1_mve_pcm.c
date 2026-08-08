#include "dm2_v1_mve_pcm.h"

#include <string.h>

static uint16_t mve_pcm_le16(const uint8_t *p)
{
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8u));
}

static int mve_pcm_find_header(const uint8_t *bytes, size_t byte_count,
                               size_t *out_offset)
{
    static const uint8_t magic[] = {
        'I','n','t','e','r','p','l','a','y',' ','M','V','E',' ',
        'F','i','l','e',0x1a,0x00,0x1a,0x00,0x00,0x01,0x33,0x11
    };
    size_t at;
    if (!bytes || !out_offset || byte_count < sizeof(magic) + 6u) return 0;
    for (at = 0u; at <= byte_count - sizeof(magic); ++at) {
        if (memcmp(bytes + at, magic, sizeof(magic)) == 0) {
            *out_offset = at;
            return 1;
        }
    }
    return 0;
}

static int mve_pcm_known_opcode(uint8_t opcode, uint8_t version, uint16_t size)
{
    switch (opcode) {
    case 0x00u: case 0x01u: return version == 0u && size == 0u;
    case 0x02u: return version == 0u && size == 6u;
    case 0x03u: return version == 0u && size == 8u;
    case 0x04u: return version == 0u && size == 0u;
    case 0x05u: return version == 2u && size == 8u;
    case 0x07u: return version == 1u && size == 6u;
    case 0x08u: return version == 0u && size >= 6u;
    case 0x09u: return version == 0u && size == 6u;
    case 0x0au: return version == 0u && size == 6u;
    case 0x0cu: return version == 0u && size == 766u;
    case 0x0fu: return version == 0u && size == 500u;
    case 0x11u: return version == 3u && size > 0u;
    case 0x13u: return version == 0u && size == 132u;
    default: return 0;
    }
}

void dm2_v1_mve_pcm_init(DM2_V1_MvePcm *pcm)
{
    if (pcm) memset(pcm, 0, sizeof(*pcm));
}

int dm2_v1_mve_pcm_decode_source_frame(
    DM2_V1_MvePcm *pcm, const DM2_V1_MvePcmSourceFrame *source,
    const uint8_t *movie_bytes, size_t movie_byte_count,
    DM2_V1_MvePcmFrame *out)
{
    const uint8_t *frame;
    uint16_t payload_bytes;
    DM2_V1_MvePcmFrame decoded;

    if (out) memset(out, 0, sizeof(*out));
    if (!pcm || !source || !movie_bytes || !source->valid ||
        source->data_offset > movie_byte_count || source->data_size < 6u ||
        source->data_size > movie_byte_count - source->data_offset ||
        source->compressed != 0u || source->channels != 2u ||
        source->bits != 8u || source->sample_rate != 22050u)
        return 0;
    if (!pcm->initialized) {
        pcm->sample_rate = source->sample_rate;
        pcm->channels = source->channels;
        pcm->bits = source->bits;
        pcm->initialized = 1;
    }
    if (pcm->sample_rate != source->sample_rate || pcm->channels != source->channels ||
        pcm->bits != source->bits) return 0;

    frame = movie_bytes + source->data_offset;
    payload_bytes = mve_pcm_le16(frame + 4u);
    if (payload_bytes != (uint16_t)(source->data_size - 6u) ||
        (payload_bytes & 1u) != 0u || mve_pcm_le16(frame + 2u) != 1u) return 0;
    if (pcm->have_source_sequence &&
        mve_pcm_le16(frame) != pcm->next_source_sequence) return 0;

    decoded.valid = 1;
    decoded.samples = frame + 6u;
    decoded.sample_bytes = payload_bytes;
    decoded.sample_frames = payload_bytes / 2u;
    decoded.source_sequence = mve_pcm_le16(frame);
    decoded.source_stream_mask = mve_pcm_le16(frame + 2u);
    pcm->next_source_sequence = (uint16_t)(decoded.source_sequence + 1u);
    pcm->have_source_sequence = 1;
    ++pcm->decoded_frame_count;
    if (out) *out = decoded;
    return 1;
}

int dm2_v1_mve_pcm_decode_presentation(
    DM2_V1_MvePcm *pcm, const DM2_V1_MvePresentation *presentation,
    const uint8_t *movie_bytes, size_t movie_byte_count,
    DM2_V1_MvePcmFrame *out)
{
    DM2_V1_MvePcmSourceFrame source;
    if (!presentation || !presentation->valid || presentation->audio_size == 0u)
        return 0;
    memset(&source, 0, sizeof(source));
    source.valid = 1;
    source.data_offset = presentation->audio_offset;
    source.data_size = presentation->audio_size;
    source.sample_rate = presentation->audio_sample_rate;
    source.channels = presentation->audio_channels;
    source.bits = presentation->audio_bits;
    source.compressed = presentation->audio_compressed;
    return dm2_v1_mve_pcm_decode_source_frame(pcm, &source, movie_bytes,
                                               movie_byte_count, out);
}

int dm2_v1_mve_audio_iterator_init(DM2_V1_MveAudioIterator *iterator,
                                   const uint8_t *movie_bytes,
                                   size_t movie_byte_count)
{
    size_t mve_offset;
    if (!iterator) return 0;
    memset(iterator, 0, sizeof(*iterator));
    if (!mve_pcm_find_header(movie_bytes, movie_byte_count, &mve_offset) ||
        mve_offset > movie_byte_count - 26u) return 0;
    iterator->bytes = movie_bytes;
    iterator->byte_count = movie_byte_count;
    iterator->offset = mve_offset + 26u;
    iterator->initialized = 1;
    return 1;
}

int dm2_v1_mve_audio_iterator_next(DM2_V1_MveAudioIterator *iterator,
                                   DM2_V1_MvePcmSourceFrame *out)
{
    if (out) memset(out, 0, sizeof(*out));
    if (!iterator || !out || !iterator->initialized || iterator->ended) return -1;
    while (iterator->offset < iterator->byte_count) {
        size_t chunk_end;
        uint16_t chunk_size;
        uint16_t chunk_type;
        if (iterator->chunk_end == 0u) {
            if (iterator->byte_count - iterator->offset < 4u) return -1;
            chunk_size = mve_pcm_le16(iterator->bytes + iterator->offset);
            chunk_type = mve_pcm_le16(iterator->bytes + iterator->offset + 2u);
            iterator->offset += 4u;
            if (chunk_size > iterator->byte_count - iterator->offset) return -1;
            if (chunk_type == 5u) {
                if (chunk_size != 0u) return -1;
                iterator->ended = 1;
                return 0;
            }
            iterator->chunk_end = iterator->offset + chunk_size;
        }
        chunk_end = iterator->chunk_end;
        while (iterator->offset < chunk_end) {
            const uint8_t *data;
            size_t data_offset;
            uint16_t size;
            uint8_t opcode, version;
            if (chunk_end - iterator->offset < 4u) return -1;
            size = mve_pcm_le16(iterator->bytes + iterator->offset);
            opcode = iterator->bytes[iterator->offset + 2u];
            version = iterator->bytes[iterator->offset + 3u];
            iterator->offset += 4u;
            if (size > chunk_end - iterator->offset ||
                !mve_pcm_known_opcode(opcode, version, size)) return -1;
            data_offset = iterator->offset;
            data = iterator->bytes + data_offset;
            iterator->offset += size;
            if (opcode == 0x03u) {
                const uint16_t flags = mve_pcm_le16(data + 2u);
                if (iterator->audio_initialized || mve_pcm_le16(data + 4u) == 0u ||
                    (flags & (uint16_t)~7u) != 0u || (flags & 4u) != 0u) return -1;
                iterator->sample_rate = mve_pcm_le16(data + 4u);
                iterator->channels = (uint8_t)((flags & 1u) + 1u);
                iterator->bits = (uint8_t)((((flags >> 1u) & 1u) + 1u) * 8u);
                iterator->compressed = 0u;
                iterator->audio_initialized = 1;
            } else if (opcode == 0x08u) {
                if (!iterator->audio_initialized || mve_pcm_le16(data + 4u) !=
                    (uint16_t)(size - 6u)) return -1;
                out->valid = 1;
                out->data_offset = (uint32_t)data_offset;
                out->data_size = size;
                out->sample_rate = iterator->sample_rate;
                out->channels = iterator->channels;
                out->bits = iterator->bits;
                out->compressed = iterator->compressed;
                out->source_sequence = mve_pcm_le16(data);
                out->source_stream_mask = mve_pcm_le16(data + 2u);
                ++iterator->audio_frame_count;
                return 1;
            }
        }
        if (iterator->offset != chunk_end) return -1;
        iterator->chunk_end = 0u;
    }
    return -1;
}
