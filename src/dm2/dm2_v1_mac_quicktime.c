#include "dm2_v1_mac_quicktime.h"

#include <string.h>

typedef struct {
    const uint8_t *bytes;
    size_t size;
    const uint8_t *stsd;
    size_t stsd_size;
    const uint8_t *stts;
    size_t stts_size;
    const uint8_t *stsc;
    size_t stsc_size;
    const uint8_t *stsz;
    size_t stsz_size;
    const uint8_t *stco;
    size_t stco_size;
    uint32_t time_scale;
    uint32_t codec;
    uint16_t width;
    uint16_t height;
    uint16_t depth_bits;
    uint16_t channels;
    uint16_t sample_bits;
    uint32_t sample_rate;
    uint32_t sample_count;
    uint32_t first_duration;
    int is_video;
} QtTrack;

static uint16_t be16(const uint8_t *p) {
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}
static uint32_t be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | p[3];
}
static uint64_t be64(const uint8_t *p) {
    return ((uint64_t)be32(p) << 32) | be32(p + 4u);
}

/* Atom payloads are never searched bytewise: compressed frames may contain a
 * fourcc by coincidence.  Each query walks only direct children of its known
 * QuickTime container. */
static int qt_child(const uint8_t *bytes, size_t size, const char type[4],
                    const uint8_t **out, size_t *out_size, unsigned wanted)
{
    size_t pos = 0u;
    unsigned seen = 0u;
    while (pos + 8u <= size) {
        uint64_t total = be32(bytes + pos);
        size_t header = 8u;
        if (total == 1u) {
            if (pos + 16u > size) return 0;
            total = be64(bytes + pos + 8u);
            header = 16u;
        } else if (total == 0u) {
            total = size - pos;
        }
        if (total < header || total > size - pos) return 0;
        if (memcmp(bytes + pos + 4u, type, 4u) == 0 && seen++ == wanted) {
            if (out) *out = bytes + pos;
            if (out_size) *out_size = (size_t)total;
            return 1;
        }
        pos += (size_t)total;
    }
    return 0;
}

static int qt_payload(const uint8_t *atom, size_t atom_size,
                      const uint8_t **out, size_t *out_size)
{
    size_t header = 8u;
    uint64_t total;
    if (!atom || atom_size < 8u) return 0;
    total = be32(atom);
    if (total == 1u) {
        if (atom_size < 16u) return 0;
        total = be64(atom + 8u);
        header = 16u;
    } else if (total == 0u) total = atom_size;
    if (total != atom_size || total < header) return 0;
    *out = atom + header;
    *out_size = atom_size - header;
    return 1;
}

static int qt_child_payload(const uint8_t *container, size_t container_size,
                            const char type[4], const uint8_t **out,
                            size_t *out_size)
{
    const uint8_t *payload;
    size_t payload_size;
    const uint8_t *child;
    size_t child_size;
    if (!qt_payload(container, container_size, &payload, &payload_size) ||
        !qt_child(payload, payload_size, type, &child, &child_size, 0u)) return 0;
    *out = child;
    *out_size = child_size;
    return 1;
}

static int qt_track_parse(const uint8_t *trak, size_t trak_size, QtTrack *out)
{
    const uint8_t *mdia, *minf, *stbl, *hdlr, *mdhd, *payload, *entry;
    const uint8_t *hdlr_payload;
    size_t mdia_size, minf_size, stbl_size, hdlr_size, mdhd_size, payload_size, hdlr_payload_size;
    uint32_t count;
    if (!trak || !out ||
        !qt_child_payload(trak, trak_size, "mdia", &mdia, &mdia_size) ||
        !qt_child_payload(mdia, mdia_size, "minf", &minf, &minf_size) ||
        !qt_child_payload(minf, minf_size, "stbl", &stbl, &stbl_size) ||
        !qt_child_payload(mdia, mdia_size, "hdlr", &hdlr, &hdlr_size) ||
        !qt_child_payload(mdia, mdia_size, "mdhd", &mdhd, &mdhd_size) ||
        !qt_child_payload(stbl, stbl_size, "stsd", &out->stsd, &out->stsd_size) ||
        !qt_child_payload(stbl, stbl_size, "stts", &out->stts, &out->stts_size) ||
        !qt_child_payload(stbl, stbl_size, "stsc", &out->stsc, &out->stsc_size) ||
        !qt_child_payload(stbl, stbl_size, "stsz", &out->stsz, &out->stsz_size)) return 0;
    if (!qt_child_payload(stbl, stbl_size, "stco", &out->stco, &out->stco_size) &&
        !qt_child_payload(stbl, stbl_size, "co64", &out->stco, &out->stco_size)) return 0;
    if (!qt_payload(hdlr, hdlr_size, &hdlr_payload, &hdlr_payload_size) || hdlr_payload_size < 12u ||
        !qt_payload(mdhd, mdhd_size, &payload, &payload_size)) return 0;
    out->is_video = memcmp(hdlr_payload + 8u, "vide", 4u) == 0;
    if (!out->is_video && memcmp(hdlr_payload + 8u, "soun", 4u) != 0)
        return 0;
    if (payload_size < 20u) return 0;
    if (payload[0] != 0u || payload_size < 24u) return 0;
    out->time_scale = be32(payload + 12u);
    if (out->time_scale == 0u) return 0;
    if (!qt_payload(out->stsd, out->stsd_size, &payload, &payload_size) ||
        payload_size < 16u || be32(payload + 4u) != 1u) return 0;
    entry = payload + 8u;
    if (be32(entry) < 16u || be32(entry) > payload_size - 8u) return 0;
    out->codec = be32(entry + 4u);
    if (out->is_video) {
        if (be32(entry) < 86u) return 0;
        out->width = be16(entry + 32u);
        out->height = be16(entry + 34u);
        out->depth_bits = (uint16_t)(be16(entry + 82u) & UINT16_C(0x001f));
    } else {
        if (be32(entry) < 36u) return 0;
        out->channels = be16(entry + 24u);
        out->sample_bits = be16(entry + 26u);
        out->sample_rate = be32(entry + 32u) >> 16;
    }
    if (!qt_payload(out->stsz, out->stsz_size, &payload, &payload_size) ||
        payload_size < 12u) return 0;
    count = be32(payload + 8u);
    if (!count || (be32(payload + 4u) == 0u &&
        count > (payload_size - 12u) / 4u)) return 0;
    out->sample_count = count;
    if (!qt_payload(out->stts, out->stts_size, &payload, &payload_size) ||
        payload_size < 16u || be32(payload + 4u) == 0u) return 0;
    out->first_duration = be32(payload + 12u);
    return out->first_duration != 0u;
}

static int qt_find_tracks(const uint8_t *bytes, size_t size,
                          QtTrack *video, QtTrack *audio)
{
    const uint8_t *moov;
    size_t moov_size, payload_size;
    const uint8_t *payload;
    unsigned i;
    int got_video = 0, got_audio = 0;
    if (!bytes || !video || !audio || !qt_child(bytes, size, "moov", &moov, &moov_size, 0u) ||
        !qt_payload(moov, moov_size, &payload, &payload_size)) return 0;
    memset(video, 0, sizeof(*video));
    memset(audio, 0, sizeof(*audio));
    for (i = 0u; ; ++i) {
        const uint8_t *trak;
        size_t trak_size;
        QtTrack candidate;
        if (!qt_child(payload, payload_size, "trak", &trak, &trak_size, i)) break;
        memset(&candidate, 0, sizeof(candidate));
        if (!qt_track_parse(trak, trak_size, &candidate)) return 0;
        if (candidate.is_video && !got_video) { *video = candidate; got_video = 1; }
        if (!candidate.is_video && !got_audio) { *audio = candidate; got_audio = 1; }
    }
    return got_video && got_audio;
}

static int qt_sample(const QtTrack *track, uint32_t index,
                     const uint8_t *bytes, size_t size,
                     DM2_V1_MacQuickTimeSample *out)
{
    const uint8_t *sizes, *stsc, *stco;
    size_t sizes_size, stsc_size, stco_size;
    uint32_t default_size, chunk_count, stsc_count;
    uint32_t sample = 0u, chunk;
    if (!track || !out || index >= track->sample_count ||
        !qt_payload(track->stsz, track->stsz_size, &sizes, &sizes_size) ||
        !qt_payload(track->stsc, track->stsc_size, &stsc, &stsc_size) ||
        !qt_payload(track->stco, track->stco_size, &stco, &stco_size) ||
        sizes_size < 12u || stsc_size < 8u || stco_size < 8u) return 0;
    default_size = be32(sizes + 4u);
    if (!default_size && sizes_size < 12u + (size_t)track->sample_count * 4u) return 0;
    stsc_count = be32(stsc + 4u);
    chunk_count = be32(stco + 4u);
    if (!stsc_count || stsc_size < 8u + (size_t)stsc_count * 12u) return 0;
    if (memcmp(track->stco + 4u, "stco", 4u) == 0) {
        if (stco_size < 8u + (size_t)chunk_count * 4u) return 0;
    } else if (stco_size < 8u + (size_t)chunk_count * 8u) return 0;
    for (chunk = 1u; chunk <= chunk_count; ++chunk) {
        uint32_t entry = 0u, e;
        uint32_t samples_per_chunk;
        uint64_t offset;
        for (e = 0u; e < stsc_count; ++e) {
            if (be32(stsc + 8u + (size_t)e * 12u) > chunk) break;
            entry = e;
        }
        samples_per_chunk = be32(stsc + 8u + (size_t)entry * 12u + 4u);
        if (!samples_per_chunk || sample > UINT32_MAX - samples_per_chunk) return 0;
        offset = memcmp(track->stco + 4u, "stco", 4u) == 0
                     ? be32(stco + 8u + (size_t)(chunk - 1u) * 4u)
                     : be64(stco + 8u + (size_t)(chunk - 1u) * 8u);
        for (uint32_t within = 0u; within < samples_per_chunk && sample < track->sample_count;
             ++within, ++sample) {
            uint32_t sample_size = default_size ? default_size : be32(sizes + 12u + (size_t)sample * 4u);
            if (offset > size || sample_size > size - (size_t)offset) return 0;
            if (sample == index) {
                memset(out, 0, sizeof(*out));
                out->codec_fourcc = track->codec;
                out->width = track->width; out->height = track->height;
                out->time_scale = track->time_scale;
                out->sample_count = track->sample_count;
                out->first_sample_duration = track->first_duration;
                out->sample_data = bytes + (size_t)offset;
                out->sample_size = sample_size;
                return 1;
            }
            offset += sample_size;
        }
    }
    return 0;
}

int dm2_v1_mac_quicktime_inspect(const uint8_t *movie_bytes, size_t movie_size,
                                 DM2_V1_MacQuickTimeInfo *out_info)
{
    QtTrack video, audio;
    if (!out_info || !qt_find_tracks(movie_bytes, movie_size, &video, &audio)) return 0;
    memset(out_info, 0, sizeof(*out_info));
    out_info->video_codec_fourcc = video.codec;
    out_info->audio_codec_fourcc = audio.codec;
    out_info->width = video.width; out_info->height = video.height;
    out_info->video_depth_bits = video.depth_bits;
    out_info->audio_channels = audio.channels;
    out_info->audio_sample_size_bits = audio.sample_bits;
    out_info->audio_sample_rate_hz = audio.sample_rate;
    out_info->video_sample_count = video.sample_count;
    out_info->audio_sample_count = audio.sample_count;
    out_info->video_time_scale = video.time_scale;
    out_info->audio_time_scale = audio.time_scale;
    out_info->video_first_sample_duration = video.first_duration;
    out_info->audio_first_sample_duration = audio.first_duration;
    return 1;
}

int dm2_v1_mac_quicktime_video_sample(const uint8_t *movie_bytes, size_t movie_size,
                                      uint32_t sample_index, DM2_V1_MacQuickTimeSample *out_sample)
{
    QtTrack video, audio;
    return qt_find_tracks(movie_bytes, movie_size, &video, &audio) &&
           qt_sample(&video, sample_index, movie_bytes, movie_size, out_sample);
}
int dm2_v1_mac_quicktime_audio_sample(const uint8_t *movie_bytes, size_t movie_size,
                                      uint32_t sample_index, DM2_V1_MacQuickTimeSample *out_sample)
{
    QtTrack video, audio;
    return qt_find_tracks(movie_bytes, movie_size, &video, &audio) &&
           qt_sample(&audio, sample_index, movie_bytes, movie_size, out_sample);
}

int dm2_v1_mac_quicktime_audio_offsets(const uint8_t *movie_bytes, size_t movie_size,
                                       size_t *out_offsets, uint32_t offset_count)
{
    QtTrack video, track;
    const uint8_t *sizes, *stsc, *stco;
    size_t sizes_size, stsc_size, stco_size;
    uint32_t default_size, chunk_count, stsc_count, sample = 0u, chunk;
    if (!out_offsets || !qt_find_tracks(movie_bytes, movie_size, &video, &track) ||
        offset_count != track.sample_count ||
        !qt_payload(track.stsz, track.stsz_size, &sizes, &sizes_size) ||
        !qt_payload(track.stsc, track.stsc_size, &stsc, &stsc_size) ||
        !qt_payload(track.stco, track.stco_size, &stco, &stco_size) ||
        sizes_size < 12u || stsc_size < 8u || stco_size < 8u) return 0;
    default_size = be32(sizes + 4u);
    if (!default_size && sizes_size < 12u + (size_t)track.sample_count * 4u) return 0;
    stsc_count = be32(stsc + 4u); chunk_count = be32(stco + 4u);
    if (!stsc_count || stsc_size < 8u + (size_t)stsc_count * 12u ||
        (memcmp(track.stco + 4u, "stco", 4u) == 0
            ? stco_size < 8u + (size_t)chunk_count * 4u
            : stco_size < 8u + (size_t)chunk_count * 8u)) return 0;
    for (chunk = 1u; chunk <= chunk_count; ++chunk) {
        uint32_t entry = 0u, e, samples_per_chunk;
        uint64_t offset;
        for (e = 0u; e < stsc_count; ++e) {
            if (be32(stsc + 8u + (size_t)e * 12u) > chunk) break;
            entry = e;
        }
        samples_per_chunk = be32(stsc + 8u + (size_t)entry * 12u + 4u);
        if (!samples_per_chunk || sample > UINT32_MAX - samples_per_chunk) return 0;
        offset = memcmp(track.stco + 4u, "stco", 4u) == 0
                     ? be32(stco + 8u + (size_t)(chunk - 1u) * 4u)
                     : be64(stco + 8u + (size_t)(chunk - 1u) * 8u);
        for (uint32_t within = 0u; within < samples_per_chunk && sample < track.sample_count;
             ++within, ++sample) {
            uint32_t sample_size = default_size ? default_size : be32(sizes + 12u + (size_t)sample * 4u);
            if (offset > movie_size || sample_size > movie_size - (size_t)offset) return 0;
            out_offsets[sample] = (size_t)offset;
            offset += sample_size;
        }
    }
    return sample == track.sample_count;
}
