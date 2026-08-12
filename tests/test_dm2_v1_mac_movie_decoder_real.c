#include "dm2_v1_mac_media.h"
#include "dm2_v1_mac_movie.h"
#include "dm2_v1_mac_movie_decoder.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    DM2_V1_MacMedia media;
    const char *zip = getenv("FIRESTAFF_DM2_MAC_EN_ZIP");
    static const int movie_indices[] = {
        DM2_V1_MAC_MOVIE_TITLE,
        DM2_V1_MAC_MOVIE_SWOOSH,
        DM2_V1_MAC_MOVIE_CREDITS,
        DM2_V1_MAC_MOVIE_ENDING
    };
    static const char *const movie_names[] = {
        "Title.MooV", "Swoosh.MooV", "Credits.MooV", "Ending.MooV"
    };
    size_t movie;

    if (!zip || !zip[0]) {
        puts("SKIP: DM2 Mac ZIP environment is not set");
        return 0;
    }
    memset(&media, 0, sizeof(media));
    if (dm2_v1_mac_media_read_zip(zip, &media) != 0 ||
        media.movie_present_mask != (((uint32_t)1u << DM2_V1_MAC_MOVIE_TITLE) |
                                     ((uint32_t)1u << DM2_V1_MAC_MOVIE_SWOOSH) |
                                     ((uint32_t)1u << DM2_V1_MAC_MOVIE_CREDITS) |
                                     ((uint32_t)1u << DM2_V1_MAC_MOVIE_ENDING)) ||
        (media.movie_present_mask & ((uint32_t)1u << DM2_V1_MAC_MOVIE_STORY)) != 0u) {
        fprintf(stderr, "authentic Mac retail movie presence changed: mask=0x%08x\n",
                media.movie_present_mask);
        dm2_v1_mac_media_free(&media);
        return 1;
    }

    for (movie = 0u; movie < sizeof(movie_indices) / sizeof(movie_indices[0]); ++movie) {
        DM2_V1_MacMovieView view;
        DM2_V1_MacMovieDecoder decoder;
        int opened;
        int advanced;
        int audio_total = 0;
        int frame;
        uint64_t previous_time = 0u;
        const int index = movie_indices[movie];

        memset(&view, 0, sizeof(view));
        memset(&decoder, 0, sizeof(decoder));
        if (dm2_v1_mac_movie_view_build(
                media.movie[index], media.movie_size[index],
                media.movie_moov[index], media.movie_moov_size[index],
                &view) != 0) {
            fprintf(stderr, "authentic Mac %s view was not built\n",
                    movie_names[movie]);
            dm2_v1_mac_media_free(&media);
            return 1;
        }
        opened = dm2_v1_mac_movie_decoder_open(&decoder, view.bytes, view.size);
        advanced = opened && dm2_v1_mac_movie_decoder_next(&decoder);
        for (frame = 0; advanced && frame < 12; ++frame) {
            const int16_t *audio = NULL;
            int count = 0;
            int rate = 0;
            if (dm2_v1_mac_movie_decoder_take_audio(&decoder, &audio,
                                                    &count, &rate) &&
                audio && count > 0 && rate > 0) {
                audio_total += count;
            }
            if (decoder.frame_duration_us == 0u ||
                (frame > 0 && decoder.presentation_time_us <= previous_time)) {
                fprintf(stderr, "authentic Mac %s has invalid frame timing: frame=%d time=%llu duration=%llu previous=%llu\n",
                        movie_names[movie], frame,
                        (unsigned long long)decoder.presentation_time_us,
                        (unsigned long long)decoder.frame_duration_us,
                        (unsigned long long)previous_time);
                dm2_v1_mac_movie_decoder_close(&decoder);
                dm2_v1_mac_movie_view_free(&view);
                dm2_v1_mac_media_free(&media);
                return 1;
            }
            previous_time = decoder.presentation_time_us;
            if (frame != 11) advanced = dm2_v1_mac_movie_decoder_next(&decoder);
        }
        if (!(opened && advanced && decoder.frame_ready &&
              decoder.width == 320 && decoder.height == 200 &&
              decoder.frame_index >= 1u && audio_total > 0)) {
            fprintf(stderr, "authentic Mac %s decode failed: open=%d next=%d ready=%d rejected=%d ended=%d size=%zu audio=%d\n",
                    movie_names[movie], opened, advanced,
                    decoder.frame_ready, decoder.rejected, decoder.ended,
                    view.size, audio_total);
            dm2_v1_mac_movie_decoder_close(&decoder);
            dm2_v1_mac_movie_view_free(&view);
            dm2_v1_mac_media_free(&media);
            return 1;
        }
        printf("PASS: authentic Mac %s decoded: %dx%d time=%llu\n",
               movie_names[movie], decoder.width, decoder.height,
               (unsigned long long)decoder.presentation_time_us);
        dm2_v1_mac_movie_decoder_close(&decoder);
        dm2_v1_mac_movie_view_free(&view);
    }
    dm2_v1_mac_media_free(&media);
    return 0;
}
