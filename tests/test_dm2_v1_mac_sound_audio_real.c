#include "audio_sdl_m11.h"
#include "dm2_v1_mac_media.h"
#include "dm2_v1_mac_sound.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned int fnv1a(const unsigned char *bytes, size_t count)
{
    unsigned int hash = 2166136261u;
    size_t index;
    for (index = 0; index < count; ++index) {
        hash ^= bytes[index];
        hash *= 16777619u;
    }
    return hash ? hash : 1u;
}

int main(void)
{
    DM2_V1_MacMedia media;
    DM2_V1_MacSoundSample sample;
    M11_AudioState audio;
    const char *zip = getenv("FIRESTAFF_DM2_MAC_EN_ZIP");
    int source_rate;
    unsigned int hash;
    int ok = 1;

    if (!zip || !zip[0]) {
        puts("SKIP: DM2 Mac ZIP environment is not set");
        return 0;
    }
    memset(&media, 0, sizeof(media));
    memset(&audio, 0, sizeof(audio));
    if (dm2_v1_mac_media_read_zip(zip, &media) != 0 ||
        dm2_v1_mac_sound_find(media.sound_resource_fork[DM2_V1_MAC_SOUND_GENERAL],
                               media.sound_resource_fork_size[DM2_V1_MAC_SOUND_GENERAL],
                               10001, &sample) != 0 || !sample.valid ||
        sample.sample_data_size == 0u || sample.sample_rate_fixed == 0u ||
        sample.sample_data_size > 120000u) {
        fprintf(stderr, "authentic Mac sound transport source not found\n");
        dm2_v1_mac_media_free(&media);
        return 1;
    }
    source_rate = (int)((sample.sample_rate_fixed + 0x8000u) >> 16);
    hash = fnv1a(sample.sample_data, sample.sample_data_size);
    ok &= M11_Audio_Init(&audio);
    ok &= M11_Audio_PlayDm2MacSndPcm(&audio,
                                     (const int8_t *)sample.sample_data,
                                     (int)sample.sample_data_size,
                                     source_rate, sample.resource_id, hash);
    ok &= audio.dm2MacSndAccepted &&
          audio.dm2MacSndByteCount == (int)sample.sample_data_size &&
          audio.dm2MacSndRateHz == source_rate &&
          audio.dm2MacSndResourceId == sample.resource_id &&
          audio.dm2MacSndHash == hash && audio.dm2MacSndPcm.sampleCount > 0;
    M11_Audio_Shutdown(&audio);
    dm2_v1_mac_media_free(&media);
    if (!ok) {
        fprintf(stderr, "authentic Mac snd PCM transport failed\n");
        return 1;
    }
    printf("PASS: authentic Mac snd PCM transported: id=%d bytes=%zu rate=%d\n",
           sample.resource_id, sample.sample_data_size, source_rate);
    return 0;
}
