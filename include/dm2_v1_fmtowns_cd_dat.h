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
 * (HME-242, Victor 1994). It contains 10 records of 4 bytes each, mapping
 * the CDDA audio tracks used for in-game music.
 *
 * Record layout: [byte0, byte1, byte2, byte3]
 *   byte0..byte2: MSF-like fields (interpretation TBD — possibly start
 *                 positions or durations referenced by TBIOS CD API)
 *   byte3: type flag (0x06=data track ref, 0x03=first audio, 0x02=audio)
 *
 * The disc has 8 tracks: Track 1 = MODE1/2352 data, Tracks 2-8 = CDDA.
 * Record 0 references the data track boundary; records 1-7 map to
 * audio tracks 2-8; records 8-9 appear to be disc-end markers. */

#define DM2_FMTOWNS_CD_DAT_SIZE        40u
#define DM2_FMTOWNS_CD_DAT_ENTRY_COUNT 10u
#define DM2_FMTOWNS_CD_DAT_ENTRY_SIZE   4u
#define DM2_FMTOWNS_CDDA_TRACK_COUNT    7u

typedef struct {
    uint8_t byte0;
    uint8_t byte1;
    uint8_t byte2;
    uint8_t type_flag;
} DM2_V1_FmtownsCdDatEntry;

typedef struct {
    DM2_V1_FmtownsCdDatEntry entries[DM2_FMTOWNS_CD_DAT_ENTRY_COUNT];
    int      valid;
    uint32_t data_entries;
    uint32_t audio_entries;
} DM2_V1_FmtownsCdDatReceipt;

/* Parse a 40-byte CD.DAT buffer.
 * Returns 0 on success, -1 on invalid input. */
int dm2_v1_fmtowns_cd_dat_parse(const uint8_t *data, size_t size,
                                DM2_V1_FmtownsCdDatReceipt *out);

/* Return the CDDA track count (audio entries with type 0x02 or 0x03). */
uint32_t dm2_v1_fmtowns_cd_dat_audio_track_count(
    const DM2_V1_FmtownsCdDatReceipt *receipt);

#ifdef __cplusplus
}
#endif

#endif /* DM2_V1_FMTOWNS_CD_DAT_H */
