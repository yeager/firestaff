#include "dm2_v1_mve_presentation_owner.h"

#include <string.h>

int dm2_v1_mve_presentation_owner_init(
    DM2_V1_MvePresentationOwner *owner, const uint8_t *movie_bytes,
    size_t movie_byte_count)
{
    DM2_V1_MveStreamReceipt stream;

    if (!owner) return 0;
    memset(owner, 0, sizeof(*owner));
    if (!movie_bytes ||
        !dm2_v1_mve_stream_parse(movie_bytes, movie_byte_count, &stream) ||
        !stream.valid || stream.width != DM2_V1_MVE_VIDEO_WIDTH ||
        stream.height != DM2_V1_MVE_VIDEO_HEIGHT || stream.timer_rate_us == 0u ||
        stream.timer_subdivision == 0u ||
        !dm2_v1_mve_presentation_iterator_init(&owner->presentation_iterator,
                                                 movie_bytes, movie_byte_count) ||
        !dm2_v1_mve_audio_iterator_init(&owner->audio_iterator, movie_bytes,
                                         movie_byte_count))
        return 0;

    owner->movie_bytes = movie_bytes;
    owner->movie_byte_count = movie_byte_count;
    owner->presentation_interval_us = (uint64_t)stream.timer_rate_us *
                                      (uint64_t)stream.timer_subdivision;
    dm2_v1_mve_video_init(&owner->video);
    dm2_v1_mve_pcm_init(&owner->source_pcm);
    owner->initialized = 1;
    return 1;
}

int dm2_v1_mve_presentation_owner_next(
    DM2_V1_MvePresentationOwner *owner, DM2_V1_MvePresentationFrame *out)
{
    DM2_V1_MvePresentation presentation;
    int next_presentation;

    if (out) memset(out, 0, sizeof(*out));
    if (!owner || !out || !owner->initialized || owner->failed) return -1;
    if (owner->ended) return 0;

    next_presentation = dm2_v1_mve_presentation_iterator_next(
        &owner->presentation_iterator, &presentation);
    if (next_presentation == 0) {
        owner->ended = 1;
        return 0;
    }
    if (next_presentation != 1 || !presentation.valid ||
        presentation.presentation_index != owner->presented_frame_count ||
        presentation.presentation_time_us != owner->clock_time_us ||
        presentation.code_map_size != 500u || presentation.video_version != 3u) {
        owner->failed = 1;
        return -1;
    }

    if (!dm2_v1_mve_video_decode_presentation(&owner->video, &presentation,
                                                owner->movie_bytes,
                                                owner->movie_byte_count)) {
        owner->failed = 1;
        return -1;
    }

    out->valid = 1;
    out->presentation_index = presentation.presentation_index;
    out->presentation_time_us = presentation.presentation_time_us;
    out->presentation_interval_us = owner->presentation_interval_us;
    out->indexed_pixels = dm2_v1_mve_video_pixels(&owner->video);
    out->palette_rgb = dm2_v1_mve_video_palette_rgb(&owner->video);
    if (!out->indexed_pixels || !out->palette_rgb) {
        owner->failed = 1;
        memset(out, 0, sizeof(*out));
        return -1;
    }
    ++owner->presented_frame_count;
    owner->clock_time_us += owner->presentation_interval_us;
    return 1;
}

int dm2_v1_mve_presentation_owner_next_source_pcm(
    DM2_V1_MvePresentationOwner *owner, DM2_V1_MvePcmFrame *out)
{
    DM2_V1_MvePcmSourceFrame source;
    int next;

    if (out) memset(out, 0, sizeof(*out));
    if (!owner || !out || !owner->initialized || owner->failed) return -1;
    next = dm2_v1_mve_audio_iterator_next(&owner->audio_iterator, &source);
    if (next == 0) return 0;
    if (next != 1 || !source.valid ||
        !dm2_v1_mve_pcm_decode_source_frame(&owner->source_pcm, &source,
                                              owner->movie_bytes,
                                              owner->movie_byte_count, out) ||
        !out->valid) {
        owner->failed = 1;
        if (out) memset(out, 0, sizeof(*out));
        return -1;
    }
    owner->retained_pcm_bytes += out->sample_bytes;
    return 1;
}
