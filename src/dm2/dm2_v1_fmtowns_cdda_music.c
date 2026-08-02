/*
 * dm2_v1_fmtowns_cdda_music.c — FM Towns HMP-to-CDDA track mapping.
 *
 * Source: SKULL.EXP offset 0x3dac (P3 code base), 29-byte table.
 * Disassembled from FM Towns DM2 (HME-242).
 */

#include "dm2_v1_fmtowns_cdda_music.h"

/* SKULL.EXP offset 0x3dac: HMP track index -> 1-based CDDA audio index.
 * 0 = silence (HMP tracks 0-3 have no CDDA equivalent).
 * Audio index 1-9 maps to disc tracks 2-10 via CD.DAT. */
static const uint8_t g_hmp_to_cdda[DM2_FMTOWNS_HMP_TRACK_COUNT] = {
    0, 0, 0, 0,     /* HMP 00-03: silence */
    1, 2, 3, 4,     /* HMP 04-07: CDDA 1-4 (disc tracks 2-5) */
    5, 6,           /* HMP 08-09: CDDA 5-6 (disc tracks 6-7) */
    2, 3, 4, 5,     /* HMP 0A-0D: CDDA 2-5 (disc tracks 3-6) */
    6, 7,           /* HMP 0E-0F: CDDA 6-7 (disc tracks 7-8) */
    4, 5, 6, 7,     /* HMP 10-13: CDDA 4-7 (disc tracks 5-8) */
    7, 9,           /* HMP 14-15: CDDA 7,9 (disc tracks 8,10) */
    2, 2,           /* HMP 16-17: CDDA 2 (disc track 3) */
    3, 4,           /* HMP 18-19: CDDA 3-4 (disc tracks 4-5) */
    6, 7, 8,        /* HMP 1A-1C: CDDA 6-8 (disc tracks 7-9) */
};

int dm2_v1_fmtowns_hmp_to_cdda(int hmp_track) {
    if (hmp_track < 0 || hmp_track >= DM2_FMTOWNS_HMP_TRACK_COUNT)
        return 0;
    return (int)g_hmp_to_cdda[hmp_track];
}

const uint8_t *dm2_v1_fmtowns_cdda_map_table(void) {
    return g_hmp_to_cdda;
}
