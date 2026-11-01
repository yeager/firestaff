#include "dm2_v1_mac_media.h"
#include "dm2_v1_mac_sound.h"

#include <stdio.h>
#include <stdlib.h>

int main(void) {
    DM2_V1_MacMedia media;
    DM2_V1_MacSoundSample sample;
    const char *zip = getenv("FIRESTAFF_DM2_MAC_EN_ZIP");
    if (!zip || !zip[0]) { puts("SKIP: DM2 Mac ZIP environment is not set"); return 77; }
    if (dm2_v1_mac_media_read_zip(zip, &media) != 0) {
        fprintf(stderr, "authentic Mac media parse failed\n");
        return 1;
    }
    if (dm2_v1_mac_sound_count(media.sound_resource_fork[DM2_V1_MAC_SOUND_MUSIC],
                                media.sound_resource_fork_size[DM2_V1_MAC_SOUND_MUSIC]) == 2u &&
        dm2_v1_mac_sound_count(media.sound_resource_fork[DM2_V1_MAC_SOUND_GENERAL],
                                media.sound_resource_fork_size[DM2_V1_MAC_SOUND_GENERAL]) == 19u &&
        dm2_v1_mac_sound_count(media.sound_resource_fork[DM2_V1_MAC_SOUND_WEAPON],
                                media.sound_resource_fork_size[DM2_V1_MAC_SOUND_WEAPON]) == 12u &&
        dm2_v1_mac_sound_find(media.sound_resource_fork[DM2_V1_MAC_SOUND_MUSIC],
                               media.sound_resource_fork_size[DM2_V1_MAC_SOUND_MUSIC],
                               11000, &sample) == 0 && sample.valid) {
        /* Standalone sound-bank archives expose the game's named banks. */
    } else if (dm2_v1_mac_sound_count(media.application_resource,
                                      media.application_resource_size) == 139u &&
               dm2_v1_mac_sound_find(media.application_resource,
                                     media.application_resource_size,
                                     6000, &sample) == 0 && sample.valid &&
               sample.format == 2u && sample.sample_length == 20886u &&
               sample.sample_rate_fixed == 0x2b770000u &&
               sample.encode == 0x00u && sample.base_frequency == 0x18u) {
        /* The authentic English retail SIT stores these samples in the
         * application's Resource Manager fork instead of separate banks. */
    } else {
        fprintf(stderr, "authentic Mac snd resource parse failed\n");
        dm2_v1_mac_media_free(&media);
        return 1;
    }
    printf("PASS: Mac snd resources parsed: id=%d bytes=%zu rate=0x%08x\n",
           sample.resource_id, sample.sample_data_size, sample.sample_rate_fixed);
    dm2_v1_mac_media_free(&media);
    return 0;
}
