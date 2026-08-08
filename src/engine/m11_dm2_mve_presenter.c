#include "m11_dm2_mve_presenter.h"

#include <stdlib.h>
#include <string.h>

static void m11_dm2_mve_presenter_fail(M11_Dm2MvePresenter *presenter)
{
    if (presenter) presenter->failed = 1;
}

static int m11_dm2_mve_presenter_queue_boundary(
    M11_Dm2MvePresenter *presenter,
    const DM2_V1_MveDisplayAudioBoundary *boundary)
{
    uint32_t packet;

    if (!presenter || !boundary || !boundary->valid) return 0;
    for (packet = 0u; packet < boundary->audio_packet_count; ++packet) {
        DM2_V1_MvePcmFrame pcm;
        const int next = dm2_v1_mve_presentation_owner_next_source_pcm(
            &presenter->source, &pcm);
        if (next != 1 || !pcm.valid ||
            pcm.source_sequence !=
                (uint16_t)(boundary->first_audio_source_sequence + packet) ||
            !dm2_v1_mve_audio_sdl_owner_queue(&presenter->audio, &pcm)) {
            return 0;
        }
    }
    return 1;
}

int m11_dm2_mve_presenter_open(
    M11_Dm2MvePresenter *presenter, const uint8_t *verified_movie_bytes,
    size_t verified_movie_byte_count, uint64_t clock_origin_us,
    M11_Dm2MvePresentIndexedFn present_indexed, void *present_context)
{
    DM2_V1_MveTimelineReceipt timeline;
    DM2_V1_MveDisplayAudioBoundary *boundaries;
    size_t boundary_bytes;

    if (!presenter) return 0;
    memset(presenter, 0, sizeof(*presenter));
    if (!verified_movie_bytes || verified_movie_byte_count == 0u ||
        !present_indexed ||
        !dm2_v1_mve_presentation_owner_init(&presenter->source,
                                             verified_movie_bytes,
                                             verified_movie_byte_count) ||
        !dm2_v1_mve_display_audio_timeline(verified_movie_bytes,
                                            verified_movie_byte_count,
                                            NULL, 0u, &timeline) ||
        !timeline.valid || timeline.presentation_count == 0u) {
        m11_dm2_mve_presenter_close(presenter);
        return 0;
    }
    boundary_bytes = (size_t)timeline.presentation_count * sizeof(*boundaries);
    if (boundary_bytes / sizeof(*boundaries) != timeline.presentation_count) {
        m11_dm2_mve_presenter_close(presenter);
        return 0;
    }
    boundaries = (DM2_V1_MveDisplayAudioBoundary *)calloc(
        timeline.presentation_count, sizeof(*boundaries));
    if (!boundaries ||
        !dm2_v1_mve_display_audio_timeline(verified_movie_bytes,
                                            verified_movie_byte_count,
                                            boundaries,
                                            timeline.presentation_count,
                                            &timeline) ||
        !dm2_v1_mve_audio_sdl_owner_open(&presenter->audio)) {
        free(boundaries);
        m11_dm2_mve_presenter_close(presenter);
        return 0;
    }
    presenter->boundaries = boundaries;
    presenter->boundary_count = timeline.presentation_count;
    presenter->clock_origin_us = clock_origin_us;
    presenter->last_host_time_us = clock_origin_us;
    presenter->present_indexed = present_indexed;
    presenter->present_context = present_context;
    presenter->initialized = 1;
    return 1;
}

int m11_dm2_mve_presenter_advance(M11_Dm2MvePresenter *presenter,
                                  uint64_t monotonic_now_us)
{
    const DM2_V1_MveDisplayAudioBoundary *boundary;
    DM2_V1_MvePresentationFrame frame;
    uint64_t due_time_us;
    int next;

    if (!presenter || !presenter->initialized || presenter->failed) return -1;
    if (presenter->ended) return 0;
    if (monotonic_now_us < presenter->last_host_time_us ||
        presenter->next_presentation_index >= presenter->boundary_count) {
        m11_dm2_mve_presenter_fail(presenter);
        return -1;
    }
    presenter->last_host_time_us = monotonic_now_us;
    boundary = &presenter->boundaries[presenter->next_presentation_index];
    if (!boundary->valid ||
        boundary->presentation_index != presenter->next_presentation_index ||
        boundary->presentation_time_us > UINT64_MAX - presenter->clock_origin_us) {
        m11_dm2_mve_presenter_fail(presenter);
        return -1;
    }
    due_time_us = presenter->clock_origin_us + boundary->presentation_time_us;
    if (monotonic_now_us < due_time_us) return 0;
    if (!m11_dm2_mve_presenter_queue_boundary(presenter, boundary)) {
        m11_dm2_mve_presenter_fail(presenter);
        return -1;
    }
    next = dm2_v1_mve_presentation_owner_next(&presenter->source, &frame);
    if (next != 1 || !frame.valid ||
        frame.presentation_index != boundary->presentation_index ||
        frame.presentation_time_us != boundary->presentation_time_us ||
        !presenter->present_indexed(presenter->present_context,
                                    frame.indexed_pixels,
                                    (const uint8_t (*)[3])frame.palette_rgb,
                                    frame.presentation_index,
                                    frame.presentation_time_us)) {
        m11_dm2_mve_presenter_fail(presenter);
        return -1;
    }
    ++presenter->next_presentation_index;
    if (presenter->next_presentation_index == presenter->boundary_count) {
        if (dm2_v1_mve_presentation_owner_next(&presenter->source, &frame) != 0) {
            m11_dm2_mve_presenter_fail(presenter);
            return -1;
        }
        presenter->ended = 1;
    }
    return 1;
}

void m11_dm2_mve_presenter_close(M11_Dm2MvePresenter *presenter)
{
    if (!presenter) return;
    dm2_v1_mve_audio_sdl_owner_close(&presenter->audio);
    free(presenter->boundaries);
    memset(presenter, 0, sizeof(*presenter));
}
