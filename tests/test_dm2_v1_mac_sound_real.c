#include "dm2_v1_mac_media.h"
#include "dm2_v1_mac_sound.h"

#include <stdio.h>
#include <stdlib.h>

int main(void) {
    DM2_V1_MacMedia media;
    DM2_V1_MacSoundSample sample;
    const char *zip = getenv("FIRESTAFF_DM2_MAC_EN_ZIP");
    if (!zip || !zip[0]) { puts("SKIP: DM2 Mac ZIP environment is not set"); return 0; }
    if (dm2_v1_mac_media_read_zip(zip, &media) != 0 ||
        dm2_v1_mac_sound_count(media.sound_resource_fork[DM2_V1_MAC_SOUND_MUSIC],
                                media.sound_resource_fork_size[DM2_V1_MAC_SOUND_MUSIC]) != 2u ||
        dm2_v1_mac_sound_count(media.sound_resource_fork[DM2_V1_MAC_SOUND_GENERAL],
                                media.sound_resource_fork_size[DM2_V1_MAC_SOUND_GENERAL]) != 19u ||
        dm2_v1_mac_sound_count(media.sound_resource_fork[DM2_V1_MAC_SOUND_WEAPON],
                                media.sound_resource_fork_size[DM2_V1_MAC_SOUND_WEAPON]) != 12u ||
        dm2_v1_mac_sound_find(media.sound_resource_fork[DM2_V1_MAC_SOUND_MUSIC],
                               media.sound_resource_fork_size[DM2_V1_MAC_SOUND_MUSIC],
                               11000, &sample) != 0 || !sample.valid ||
        sample.sample_data_size == 0u || sample.sample_rate_fixed == 0u) {
        fprintf(stderr, "authentic Mac snd resource parse failed\n");
        dm2_v1_mac_media_free(&media);
        return 1;
    }
    printf("PASS: Mac snd resources parsed: id=%d bytes=%zu rate=0x%08x\n",
           sample.resource_id, sample.sample_data_size, sample.sample_rate_fixed);
    dm2_v1_mac_media_free(&media);
    return 0;
}
