#include "dm2_v1_mac_media.h"
#include "dm2_v1_sound.h"

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    const char *zip = getenv("FIRESTAFF_DM2_MAC_EN_ZIP");
    DM2_V1_MacMedia media;
    DM2_V1_MusicQueueReceipt receipt;
    int result;

    if (!zip || !zip[0]) {
        puts("SKIP: DM2 Mac retail ZIP is not set");
        return 0;
    }
    if (dm2_v1_mac_media_read_zip(zip, &media) != 0 ||
        !media.application_resource || media.application_resource_size == 0u) {
        fprintf(stderr, "authentic Mac application resource fork unavailable\n");
        return 1;
    }
    result = dm2_v1_sound_queue_mac_midi(
        media.application_resource, media.application_resource_size,
        1000, 1, &receipt);
    if ((result != DM2_V1_MUSIC_QUEUE_READY &&
         result != DM2_V1_MUSIC_QUEUE_DECODER_BACKEND_UNAVAILABLE) ||
        !receipt.asset_resolved || !receipt.decoder_proven ||
        !receipt.schedule_handoff_ready || receipt.schedule_event_count == 0u) {
        fprintf(stderr,
                "authentic Mac Midi route failed: result=%d resolved=%d decoder=%d schedule=%d events=%u\n",
                result, receipt.asset_resolved, receipt.decoder_proven,
                receipt.schedule_handoff_ready, receipt.schedule_event_count);
        dm2_v1_mac_media_free(&media);
        return 1;
    }
    dm2_v1_mac_media_free(&media);
    printf("PASS: authentic Mac Midi(1000) reached SMF scheduling: result=%d events=%u\n",
           result, receipt.schedule_event_count);
    return 0;
}
