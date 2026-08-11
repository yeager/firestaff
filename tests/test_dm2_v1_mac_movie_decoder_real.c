#include "dm2_v1_mac_media.h"
#include "dm2_v1_mac_movie.h"
#include "dm2_v1_mac_movie_decoder.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    DM2_V1_MacMedia media;
    DM2_V1_MacMovieView view;
    DM2_V1_MacMovieDecoder decoder;
    const char *zip = getenv("FIRESTAFF_DM2_MAC_EN_ZIP");
    int ok;
    int opened;
    int advanced;
    int audio_total = 0;
    int frame;

    if (!zip || !zip[0]) {
        puts("SKIP: DM2 Mac ZIP environment is not set");
        return 0;
    }
    memset(&media, 0, sizeof(media));
    memset(&view, 0, sizeof(view));
    memset(&decoder, 0, sizeof(decoder));
    if (dm2_v1_mac_media_read_zip(zip, &media) != 0 ||
        dm2_v1_mac_movie_view_build(
            media.movie[DM2_V1_MAC_MOVIE_TITLE],
            media.movie_size[DM2_V1_MAC_MOVIE_TITLE],
            media.movie_moov[DM2_V1_MAC_MOVIE_TITLE],
            media.movie_moov_size[DM2_V1_MAC_MOVIE_TITLE], &view) != 0) {
        fprintf(stderr, "authentic Mac Title.MooV view was not built\n");
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
                                                &count, &rate)) {
            if (audio && count > 0 && rate > 0) audio_total += count;
        }
        if (frame != 11) advanced = dm2_v1_mac_movie_decoder_next(&decoder);
    }
    ok = opened && advanced && decoder.frame_ready &&
         decoder.width == 320 && decoder.height == 200 &&
         decoder.frame_index >= 1u && audio_total > 0;
    if (!ok) {
        fprintf(stderr, "authentic Mac QuickTime frame decode failed: open=%d next=%d ready=%d rejected=%d ended=%d size=%zu\n",
                opened, advanced,
                decoder.frame_ready, decoder.rejected, decoder.ended, view.size);
        dm2_v1_mac_movie_decoder_close(&decoder);
        dm2_v1_mac_movie_view_free(&view);
        dm2_v1_mac_media_free(&media);
        return 1;
    }
    printf("PASS: authentic Mac QuickTime frame decoded: %dx%d time=%llu\n",
           decoder.width, decoder.height,
           (unsigned long long)decoder.presentation_time_us);
    dm2_v1_mac_movie_decoder_close(&decoder);
    dm2_v1_mac_movie_view_free(&view);
    dm2_v1_mac_media_free(&media);
    return 0;
}
