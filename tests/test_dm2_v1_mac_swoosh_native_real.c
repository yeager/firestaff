#include "dm2_v1_mac_media.h"
#include "dm2_v1_mac_movie.h"
#include "dm2_v1_mac_movie_decoder.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const char *zip = getenv("FIRESTAFF_DM2_MAC_EN_ZIP");
    DM2_V1_MacMedia media;
    DM2_V1_MacMovieView view;
    DM2_V1_MacMovieDecoder decoder;
    int frame;
    int audio_total = 0;
    if (!zip || !zip[0]) { puts("SKIP: DM2 Mac ZIP environment is not set"); return 77; }
    memset(&media, 0, sizeof(media));
    memset(&view, 0, sizeof(view));
    memset(&decoder, 0, sizeof(decoder));
    if (dm2_v1_mac_media_read_zip(zip, &media) != 0 ||
        dm2_v1_mac_movie_view_build(media.movie[DM2_V1_MAC_MOVIE_SWOOSH],
                                    media.movie_size[DM2_V1_MAC_MOVIE_SWOOSH],
                                    media.movie_moov[DM2_V1_MAC_MOVIE_SWOOSH],
                                    media.movie_moov_size[DM2_V1_MAC_MOVIE_SWOOSH],
                                    &view) != 0 ||
        !dm2_v1_mac_movie_decoder_open(&decoder, view.bytes, view.size)) {
        fprintf(stderr, "authentic Mac Swoosh native open failed\n");
        dm2_v1_mac_movie_view_free(&view); dm2_v1_mac_media_free(&media); return 1;
    }
    for (frame = 0; frame < 12; ++frame) {
        uint64_t previous = decoder.presentation_time_us;
        if (!dm2_v1_mac_movie_decoder_next(&decoder) || !decoder.frame_ready ||
            decoder.width != 320 || decoder.height != 200 ||
            decoder.frame_duration_us == 0u ||
            (frame > 0 && decoder.presentation_time_us <= previous)) {
            fprintf(stderr, "authentic Mac Swoosh native frame %d failed\n", frame);
            dm2_v1_mac_movie_decoder_close(&decoder);
            dm2_v1_mac_movie_view_free(&view); dm2_v1_mac_media_free(&media); return 1;
        }
        {
            const int16_t *audio = NULL;
            int count = 0, rate = 0;
            if (dm2_v1_mac_movie_decoder_take_audio(&decoder, &audio, &count, &rate) &&
                audio && count > 0 && rate > 0) audio_total += count;
        }
    }
    if (audio_total <= 0) {
        fprintf(stderr, "authentic Mac Swoosh native PCM failed\n");
        dm2_v1_mac_movie_decoder_close(&decoder);
        dm2_v1_mac_movie_view_free(&view); dm2_v1_mac_media_free(&media); return 1;
    }
    printf("PASS: authentic Mac Swoosh native RLE/PCM: frames=%u duration=%llu audio=%d\n",
           decoder.frame_index, (unsigned long long)decoder.frame_duration_us, audio_total);
    dm2_v1_mac_movie_decoder_close(&decoder);
    dm2_v1_mac_movie_view_free(&view); dm2_v1_mac_media_free(&media);
    return 0;
}
