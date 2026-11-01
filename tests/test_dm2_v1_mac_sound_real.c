#include "dm2_v1_mac_media.h"
#include "dm2_v1_mac_sound.h"

#include <stdio.h>
#include <stdlib.h>

int main(void) {
    DM2_V1_MacMedia media;
    DM2_V1_MacSoundSample sample;
    const char *zip = getenv("FIRESTAFF_DM2_MAC_EN_ZIP");
    size_t embedded_samples = 0u;
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
        int resource_id;
        /* The authentic English retail SIT stores these samples in the
         * application's Resource Manager fork instead of separate banks.
         * Validate every source ID in its bounded 6000..6540 domain; the
         * retail fork contains 139 non-contiguous format-2 resources. */
        for (resource_id = 6000; resource_id <= 6540; ++resource_id) {
            DM2_V1_MacSoundSample embedded;
            if (dm2_v1_mac_sound_find(media.application_resource,
                                      media.application_resource_size,
                                      (int16_t)resource_id,
                                      &embedded) != 0) {
                continue;
            }
            if (!embedded.valid || embedded.resource_id != resource_id ||
                embedded.format != 2u || embedded.sample_data == NULL ||
                embedded.sample_length == 0u ||
                embedded.sample_data_size != embedded.sample_length ||
                embedded.sample_rate_fixed == 0u || embedded.encode != 0u) {
                fprintf(stderr, "invalid embedded Mac snd resource %d\n",
                        resource_id);
                dm2_v1_mac_media_free(&media);
                return 1;
            }
            ++embedded_samples;
        }
        if (embedded_samples != 139u) {
            fprintf(stderr,
                    "parsed %zu embedded Mac snd resources, expected 139\n",
                    embedded_samples);
            dm2_v1_mac_media_free(&media);
            return 1;
        }
    } else {
        fprintf(stderr, "authentic Mac snd resource parse failed\n");
        dm2_v1_mac_media_free(&media);
        return 1;
    }
    printf("PASS: Mac snd resources parsed: id=%d bytes=%zu rate=0x%08x embedded=%zu\n",
           sample.resource_id, sample.sample_data_size, sample.sample_rate_fixed,
           embedded_samples);
    dm2_v1_mac_media_free(&media);
    return 0;
}
