#ifndef DM2_V1_FMTOWNS_CD_DAT_H
#define DM2_V1_FMTOWNS_CD_DAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* FM Towns DM2 CD.DAT parser.
 *
 * CD.DAT is a 40-byte file in the DATA directory of the FM Towns DM2 disc
 * (HME-242, Victor 1994-01-28). It contains 10 records of 4 bytes each
 * that describe the CD-ROM track layout for CDDA music playback.
 *
 * Record layout: [byte0, byte1, byte2, byte3]
 *   byte0: TBIOS parameter (passed to INT 93h AH=52h CD_PLAY)
 *   byte1: TBIOS parameter
 *   byte2: TBIOS parameter (always 0x06 — data track reference?)
 *   byte3: type flag
 *     0x06 = data track descriptor (record 0)
 *     0x03 = first audio track (record 1 → disc track 2)
 *     0x02 = audio track (records 2-7 → disc tracks 3-8)
 *
 * The disc has 8 tracks: Track 1 = MODE1/2352 data, Tracks 2-8 = CDDA.
 * Record 0 describes the data track boundary; records 1-7 map sequentially
 * to audio tracks 2-8; records 8-9 are disc-end sentinel entries.
 *
 * SKULL.EXP calls TBIOS INT 93h AH=52h with AL = track_number | 0xC0
 * (0xC0 = play with auto-repeat). The track_number is the 1-based disc
 * track, so audio entries 1-7 correspond to disc tracks 2-8.
 *
 * Source: disassembly of SKULL.EXP (P3/EXP Run386 flat-model binary) */

#define DM2_FMTOWNS_CD_DAT_SIZE        40u
#define DM2_FMTOWNS_CD_DAT_ENTRY_COUNT 10u
#define DM2_FMTOWNS_CD_DAT_ENTRY_SIZE   4u
#define DM2_FMTOWNS_CDDA_TRACK_COUNT    7u

#define DM2_FMTOWNS_CD_TYPE_DATA  0x06
#define DM2_FMTOWNS_CD_TYPE_AUDIO_FIRST 0x03
#define DM2_FMTOWNS_CD_TYPE_AUDIO 0x02

typedef struct {
    uint8_t tbios_param0;
    uint8_t tbios_param1;
    uint8_t tbios_param2;
    uint8_t type_flag;
} DM2_V1_FmtownsCdDatEntry;

typedef struct {
    DM2_V1_FmtownsCdDatEntry entries[DM2_FMTOWNS_CD_DAT_ENTRY_COUNT];
    int      valid;
    uint32_t data_entries;
    uint32_t audio_entries;
    /* Resolved audio track table: audio_disc_tracks[i] = disc track number
     * for the i-th audio entry (0-based). Disc track 2 = first audio. */
    uint8_t  audio_disc_tracks[DM2_FMTOWNS_CDDA_TRACK_COUNT];
} DM2_V1_FmtownsCdDatReceipt;

/* Parse a 40-byte CD.DAT buffer.
 * Returns 0 on success, -1 on invalid input. */
int dm2_v1_fmtowns_cd_dat_parse(const uint8_t *data, size_t size,
                                DM2_V1_FmtownsCdDatReceipt *out);

/* Return the CDDA track count (audio entries with type 0x02 or 0x03). */
uint32_t dm2_v1_fmtowns_cd_dat_audio_track_count(
    const DM2_V1_FmtownsCdDatReceipt *receipt);

/* Return the 1-based disc track number for a given audio index (0-based).
 * Returns 0 if the index is out of range or receipt is invalid. */
uint8_t dm2_v1_fmtowns_cd_dat_disc_track(
    const DM2_V1_FmtownsCdDatReceipt *receipt, int audio_index);

#ifdef __cplusplus
}
#endif

#endif /* DM2_V1_FMTOWNS_CD_DAT_H */
