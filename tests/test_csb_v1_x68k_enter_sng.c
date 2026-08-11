#include "csb_v1_x68k_enter_sng.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_file(const char *path, unsigned char **out, size_t *out_size) {
    FILE *fp = fopen(path, "rb");
    long size;
    if (!fp || fseek(fp, 0, SEEK_END) != 0 || (size = ftell(fp)) <= 0 ||
        fseek(fp, 0, SEEK_SET) != 0 || !(*out = malloc((size_t)size)) ||
        fread(*out, 1u, (size_t)size, fp) != (size_t)size) {
        if (fp) fclose(fp);
        free(*out); return 0;
    }
    fclose(fp); *out_size = (size_t)size; return 1;
}

int main(int argc, char **argv) {
    static const unsigned char midi[] = {
        'M','T','h','d',0,0,0,6,0,1,0,1,1,0,
        'M','T','r','k',0,0,0,19,
        0,0xff,0x51,3,7,0xa1,0x20,
        0,0x90,0x3c,0x64,0x81,0x70,0x3c,0,
        0,0xff,0x2f,0
    };
    CSB_V1_X68kEnterSngReceipt receipt;
    if (!csb_v1_x68k_enter_sng_probe(midi, sizeof(midi), &receipt) ||
        receipt.format != 1u || receipt.track_count != 1u ||
        receipt.ticks_per_quarter_note != 256u || receipt.event_count != 4u ||
        receipt.channel_event_count != 2u || receipt.meta_event_count != 2u ||
        receipt.note_on_count != 1u || receipt.tempo_event_count != 1u ||
        receipt.end_of_track_count != 1u || receipt.longest_track_ticks != 240u ||
        csb_v1_x68k_enter_sng_probe(midi, sizeof(midi) - 1u, NULL)) return 1;
    if (argc == 2) {
        unsigned char *hdm = NULL;
        size_t hdm_size = 0u;
        if (!read_file(argv[1], &hdm, &hdm_size) ||
            !csb_v1_x68k_enter_sng_probe_hdm(hdm, hdm_size, &receipt) ||
            receipt.format != 1u || receipt.track_count != 9u ||
            receipt.ticks_per_quarter_note != 480u || receipt.event_count != 3205u ||
            receipt.note_on_count != 1587u || receipt.tempo_event_count != 1u ||
            receipt.end_of_track_count != 9u ||
            receipt.longest_track_ticks != 146431u) {
            free(hdm); return 1;
        }
        printf("PASS: original CSB X68000 ENTER.SNG: events=%u notes=%u tempos=%u ticks=%llu\n",
               receipt.event_count, receipt.note_on_count, receipt.tempo_event_count,
               (unsigned long long)receipt.longest_track_ticks);
        free(hdm);
    }
    puts("test_csb_v1_x68k_enter_sng: PASS");
    return 0;
}
