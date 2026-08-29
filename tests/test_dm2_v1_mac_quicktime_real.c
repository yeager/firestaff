#include "dm2_v1_mac_media.h"
#include "dm2_v1_mac_movie.h"
#include "dm2_v1_mac_quicktime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int is_video_codec(uint32_t codec)
{
    return codec == UINT32_C(0x63766964) || /* cvid */
           codec == UINT32_C(0x72707a61) || /* rpza */
           codec == UINT32_C(0x726c6520);   /* rle  */
}

static int is_audio_codec(uint32_t codec)
{
    return codec == UINT32_C(0x74776f73) || /* twos */
           codec == UINT32_C(0x696d6134) || /* ima4 */
           codec == UINT32_C(0x72617720);   /* raw  */
}

int main(void)
{
    const char *zip = getenv("FIRESTAFF_DM2_MAC_EN_ZIP");
    DM2_V1_MacMedia media;
    unsigned index;
    if (!zip || !zip[0]) {
        puts("SKIP: DM2 Mac ZIP environment is not set");
        return 77;
    }
    memset(&media, 0, sizeof(media));
    if (dm2_v1_mac_media_read_zip(zip, &media) != 0) return 1;
    for (index = 0u; index < DM2_V1_MAC_MOVIE_COUNT; ++index) {
        DM2_V1_MacMovieView view;
        DM2_V1_MacQuickTimeInfo info;
        DM2_V1_MacQuickTimeSample video, audio;
        if ((media.movie_present_mask & (UINT32_C(1) << index)) == 0u) continue;
        memset(&view, 0, sizeof(view));
        memset(&info, 0, sizeof(info));
        memset(&video, 0, sizeof(video));
        memset(&audio, 0, sizeof(audio));
        if (dm2_v1_mac_movie_view_build(media.movie[index], media.movie_size[index],
                                        media.movie_moov[index], media.movie_moov_size[index],
                                        &view) != 0 ||
            !dm2_v1_mac_quicktime_inspect(view.bytes, view.size, &info)) {
            fprintf(stderr, "authentic Mac MooV %u has malformed QuickTime tables\n", index);
            dm2_v1_mac_movie_view_free(&view);
            dm2_v1_mac_media_free(&media);
            return 1;
        }
        if (
            !is_video_codec(info.video_codec_fourcc) ||
            !is_audio_codec(info.audio_codec_fourcc) ||
            info.width != 320u || info.height != 200u ||
            info.video_sample_count < 12u || info.audio_sample_count == 0u ||
            !dm2_v1_mac_quicktime_video_sample(view.bytes, view.size, 0u, &video) ||
            !dm2_v1_mac_quicktime_audio_sample(view.bytes, view.size, 0u, &audio) ||
            video.sample_size == 0u || audio.sample_size == 0u ||
            video.sample_data < view.bytes ||
            video.sample_data + video.sample_size > view.bytes + view.size ||
            audio.sample_data < view.bytes ||
            audio.sample_data + audio.sample_size > view.bytes + view.size) {
            fprintf(stderr, "authentic Mac MooV %u QuickTime table admission failed: video=%08x audio=%08x %ux%u/%ubpp frames=%u audio_samples=%u rate=%u\n",
                    index, info.video_codec_fourcc, info.audio_codec_fourcc,
                    info.width, info.height, info.video_depth_bits, info.video_sample_count,
                    info.audio_sample_count, info.audio_sample_rate_hz);
            dm2_v1_mac_movie_view_free(&view);
            dm2_v1_mac_media_free(&media);
            return 1;
        }
        printf("PASS: authentic Mac MooV %u: video=%08x/%ubpp audio=%08x frames=%u\n",
               index, info.video_codec_fourcc, info.video_depth_bits,
               info.audio_codec_fourcc, info.video_sample_count);
        dm2_v1_mac_movie_view_free(&view);
    }
    dm2_v1_mac_media_free(&media);
    return 0;
}
