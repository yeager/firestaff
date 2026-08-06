#include "dm2_v1_sound.h"

#include <stdio.h>
#include <string.h>

static void put_le32(unsigned char *p, unsigned value)
{
    p[0] = (unsigned char)value;
    p[1] = (unsigned char)(value >> 8);
    p[2] = (unsigned char)(value >> 16);
    p[3] = (unsigned char)(value >> 24);
}

int main(void)
{
    static const unsigned char smf[] = {
        'M','T','h','d', 0,0,0,6, 0,0, 0,1, 0,96,
        'M','T','r','k', 0,0,0,8,
        0, 0x90,0x3c,0x40, 0x60, 0xff,0x2f,0
    };
    unsigned char hmp[920];
    DM2_V1_MusicStreamReceipt receipt;

    if (dm2_v1_sound_inspect_music_data(smf, sizeof(smf), &receipt) !=
            DM2_V1_MUSIC_INSPECT_OK ||
        receipt.format != DM2_V1_MUSIC_FORMAT_STANDARD_MIDI ||
        receipt.track_count != 1 || receipt.event_count != 2 ||
        receipt.channel_event_count != 1 || receipt.meta_event_count != 1 ||
        receipt.tracks[0].end_of_track_count != 1 ||
        receipt.duration_ticks != 96 || receipt.loop_duration_us != 500000 ||
        receipt.schedule_event_count != 2 || !receipt.schedule_handoff_ready ||
        receipt.midi_handoff_ready != 1 || receipt.pcm_handoff_ready != 0) {
        fprintf(stderr, "SMF title-stream handoff was not established\n");
        return 1;
    }

    memset(hmp, 0, sizeof(hmp));
    memcpy(hmp, "HMIMIDIP013195", 14);
    put_le32(hmp + 48, 1);
    put_le32(hmp + 56, 120);
    /* DM2's HMP 013195 payload keeps a four-byte branch offset after its
     * 900-byte header; the first 12-byte chunk starts at 904. */
    put_le32(hmp + 908, 16);
    hmp[916] = 0x80; hmp[917] = 0x90; hmp[918] = 0x3c; hmp[919] = 0x40;
    if (dm2_v1_sound_inspect_music_data(hmp, sizeof(hmp), &receipt) !=
            DM2_V1_MUSIC_INSPECT_OK ||
        receipt.format != DM2_V1_MUSIC_FORMAT_HMP_V1 ||
        receipt.track_count != 1 || receipt.event_count != 1 ||
        receipt.channel_event_count != 1 || receipt.midi_handoff_ready != 0 ||
        receipt.schedule_handoff_ready != 0 || receipt.schedule_event_count != 0) {
        fprintf(stderr, "native HMP diagnostic incorrectly opened MIDI handoff\n");
        return 1;
    }

    hmp[916] = 0x00; /* Makes event data serve as the HMP VLQ continuation. */
    if (dm2_v1_sound_inspect_music_data(hmp, sizeof(hmp), &receipt) !=
            DM2_V1_MUSIC_INSPECT_BAD_EVENT || receipt.midi_handoff_ready != 0) {
        fprintf(stderr, "malformed HMP event was accepted\n");
        return 1;
    }
    puts("PASS DM2 HMP inspection remains diagnostic while SMF can hand off");
    return 0;
}
