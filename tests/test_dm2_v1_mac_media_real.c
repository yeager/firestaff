#include "dm2_v1_mac_media.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    const char *zip = getenv("FIRESTAFF_DM2_MAC_EN_ZIP");
    DM2_V1_MacMedia media;
    if (!zip || !zip[0]) {
        puts("SKIP: FIRESTAFF_DM2_MAC_EN_ZIP is not set");
        return 0;
    }
    if (dm2_v1_mac_media_read_zip(zip, &media) != 0) {
        fprintf(stderr, "authentic Mac ZIP could not be read: %s\n", zip);
        return 1;
    }
    if (media.graphics_size != 8157169u || media.dungeon_size != 39411u ||
        media.music_map_size != 176u || media.demo) {
        fprintf(stderr, "unexpected Mac fork sizes: graphics=%zu dungeon=%zu md=%zu\n",
                media.graphics_size, media.dungeon_size, media.music_map_size);
        dm2_v1_mac_media_free(&media);
        return 1;
    }
    if ((media.movie_present_mask &
                  ((uint32_t)1u << DM2_V1_MAC_MOVIE_TITLE)) == 0u) {
        fprintf(stderr, "authentic retail Mac Title.MooV was not read\n");
        dm2_v1_mac_media_free(&media);
        return 1;
    }
    if (
        (((media.movie_resource_present_mask &
             ((uint32_t)1u << DM2_V1_MAC_MOVIE_TITLE)) == 0u) ||
         ((media.movie_moov_present_mask &
             ((uint32_t)1u << DM2_V1_MAC_MOVIE_TITLE)) == 0u) ||
         media.movie_moov_size[DM2_V1_MAC_MOVIE_TITLE] < 8u)) {
        fprintf(stderr, "authentic Mac Title.MooV resource/moov was not read\n");
        dm2_v1_mac_media_free(&media);
        return 1;
    }
    if (media.sound_resource_fork_present_mask != 0x7u ||
                  media.sound_resource_fork_size[DM2_V1_MAC_SOUND_MUSIC] != 662956u ||
                  media.sound_resource_fork_size[DM2_V1_MAC_SOUND_GENERAL] != 134562u ||
                  media.sound_resource_fork_size[DM2_V1_MAC_SOUND_WEAPON] != 50651u) {
        fprintf(stderr, "authentic Mac sound resource forks were not read\n");
        dm2_v1_mac_media_free(&media);
        return 1;
    }
    if (media.application_data_size != 484944u ||
                  media.application_resource_size != 5046234u ||
                  !media.application_data || !media.application_resource ||
                  memcmp(media.application_data, "Joy!", 4u) != 0) {
        fprintf(stderr, "authentic Mac application forks were not retained: data=%zu resource=%zu ptr=%d/%d\n",
                media.application_data_size, media.application_resource_size,
                media.application_data != NULL, media.application_resource != NULL);
        if (media.application_data) {
            fprintf(stderr, "application head=%02x%02x%02x%02x\n",
                    media.application_data[0], media.application_data[1],
                    media.application_data[2], media.application_data[3]);
        }
        dm2_v1_mac_media_free(&media);
        return 1;
    }
    {
        const uint8_t *midi = NULL;
        size_t midi_size = 0u;
        DM2_V1_MacResourceReceipt receipt;
        for (int resource_id = 1000; resource_id <= 1027; ++resource_id) {
            if (dm2_v1_mac_resource_find(media.application_resource,
                                         media.application_resource_size, "Midi",
                                         (int16_t)resource_id, &midi, &midi_size,
                                         &receipt) != 0 ||
                receipt.id != resource_id || receipt.size != midi_size ||
                midi_size < 14u || memcmp(midi, "MThd", 4u) != 0) {
                fprintf(stderr, "authentic Mac Midi resource %d was not parsed\n",
                        resource_id);
                dm2_v1_mac_media_free(&media);
                return 1;
            }
        }
        /* The original Mac event loop owns these resources.  Keep the
         * application menu boundary source-verified before any future
         * native pointer/menu dispatcher is allowed to consume a click. */
        {
            static const int16_t menu_ids[] = { 129, 130, 131 };
            size_t menu_index;
            const uint8_t *resource = NULL;
            size_t resource_size = 0u;
            for (menu_index = 0u;
                 menu_index < sizeof(menu_ids) / sizeof(menu_ids[0]);
                 ++menu_index) {
                if (dm2_v1_mac_resource_find(
                        media.application_resource,
                        media.application_resource_size, "MENU",
                        menu_ids[menu_index], &resource, &resource_size,
                        &receipt) != 0 || resource_size < 4u ||
                    receipt.id != menu_ids[menu_index]) {
                    fprintf(stderr, "authentic Mac MENU resource %d was not parsed\n",
                            menu_ids[menu_index]);
                    dm2_v1_mac_media_free(&media);
                    return 1;
                }
            }
            if (dm2_v1_mac_resource_find(
                    media.application_resource,
                    media.application_resource_size, "DITL", 132,
                    &resource, &resource_size, &receipt) != 0 ||
                resource_size < 2u || receipt.id != 132) {
                fprintf(stderr, "authentic Mac DITL(132) was not parsed\n");
                dm2_v1_mac_media_free(&media);
                return 1;
            }
        }
    }
    {
        printf("retail movie mask=0x%08x resource=0x%08x moov=0x%08x sizes=%zu,%zu,%zu,%zu,%zu moov_size=%zu head=%02x%02x%02x%02x%02x%02x%02x%02x\n",
               media.movie_present_mask, media.movie_resource_present_mask,
               media.movie_moov_present_mask, media.movie_size[0],
               media.movie_size[1], media.movie_size[2], media.movie_size[3],
               media.movie_size[4], media.movie_moov_size[0], media.movie[0][0], media.movie[0][1],
               media.movie[0][2], media.movie[0][3], media.movie[0][4],
               media.movie[0][5], media.movie[0][6], media.movie[0][7]);
    }
    dm2_v1_mac_media_free(&media);
    puts("PASS: authentic DM2 Macintosh retail HFS forks read in RAM");
    return 0;
}
