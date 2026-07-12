/* Skip-safe DM2 title music proof.  skproject/SKULLWIN/c_midi.cpp:11-37
 * opens DATA/%02x.hmp.mid with load_midi() and loops it; this probe verifies
 * the real title stream reaches Firestaff's backend-ready MIDI receipt without
 * synthesizing PCM or selecting SKWin's alternate OGG mode. */
#include "dm2_v1_sound.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const char *path = getenv("FIRESTAFF_DM2_HMP_PATH");
    FILE *file;
    long length;
    unsigned char *data = NULL;
    DM2_V1_MusicStreamReceipt receipt;
    DM2_V1_MusicQueueReceipt queue_receipt;

    if (!path || !path[0])
        path = "/Users/bosse/Documents/skproject-codex-ref/SKULLWIN/Data/00.hmp.mid";
    file = fopen(path, "rb");
    if (!file) {
        printf("SKIP DM2 real HMP title file unavailable: %s\n", path);
        return 0;
    }
    if (fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) <= 0 ||
        (unsigned long)length > DM2_V1_MUSIC_MAX_FILE_BYTES ||
        fseek(file, 0, SEEK_SET) != 0 ||
        !(data = (unsigned char *)malloc((size_t)length)) ||
        fread(data, 1, (size_t)length, file) != (size_t)length) {
        fclose(file);
        free(data);
        fprintf(stderr, "DM2 real HMP title file could not be read safely\n");
        return 1;
    }
    fclose(file);
    if (dm2_v1_sound_inspect_music_data(data, (size_t)length, &receipt) !=
            DM2_V1_MUSIC_INSPECT_OK || receipt.track_count == 0 ||
        receipt.event_count == 0 || !receipt.midi_handoff_ready ||
        receipt.pcm_handoff_ready || !receipt.schedule_handoff_ready ||
        receipt.loop_duration_us == 0 || receipt.schedule_event_count == 0) {
        free(data);
        fprintf(stderr, "DM2 real title stream did not produce MIDI-only receipt (result=%d tracks=%u events=%u)\n",
                (int)receipt.result, receipt.track_count, receipt.event_count);
        return 1;
    }
    /* `path` is the original DATA/00.hmp.mid sidecar selected by SKWin.
     * Bind its containing DATA directory through Firestaff's hash-gated
     * queue boundary and prove the parser, not a PCM substitute, is ready. */
    {
        const char *slash = strrchr(path, '/');
        char root[512];
        size_t root_length;
        if (!slash || (size_t)(slash - path) >= sizeof(root)) {
            free(data);
            fprintf(stderr, "DM2 real title path has no bounded DATA root\n");
            return 1;
        }
        root_length = (size_t)(slash - path);
        memcpy(root, path, root_length);
        root[root_length] = '\0';
        dm2_v1_sound_bind_verified_music_assets(root, 1);
        if (dm2_v1_sound_queue_music(0, 1, &queue_receipt) !=
                DM2_V1_MUSIC_QUEUE_DECODER_BACKEND_UNAVAILABLE ||
            !queue_receipt.asset_resolved || !queue_receipt.request_queued ||
            !queue_receipt.decoder_proven || !queue_receipt.midi_handoff_ready ||
            queue_receipt.pcm_handoff_ready || queue_receipt.backend_proven ||
            !queue_receipt.schedule_handoff_ready ||
            queue_receipt.loop_duration_us == 0) {
            free(data);
            fprintf(stderr, "DM2 real title queue did not preserve MIDI-only backend boundary\n");
            return 1;
        }
    }
    printf("PASS DM2 real title stream: format=%d tracks=%u events=%u bytes=%u\n",
           (int)receipt.format, receipt.track_count, receipt.event_count,
           receipt.file_size);
    free(data);
    return 0;
}
