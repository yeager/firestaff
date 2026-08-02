#ifndef FIRESTAFF_DM2_V1_FMTOWNS_CDDA_MUSIC_H
#define FIRESTAFF_DM2_V1_FMTOWNS_CDDA_MUSIC_H

/*
 * dm2_v1_fmtowns_cdda_music.h — FM Towns HMP-to-CDDA track mapping.
 *
 * The PC version of DM2 has 29 HMP MIDI tracks stored in GDAT
 * category 4, type 3 (dtHMP), indices 0x00-0x1C.  SONGLIST.DAT
 * maps dungeon maps to these HMP track indices.
 *
 * The FM Towns version replaces HMP playback with CDDA (CD audio).
 * The disc has 9 audio tracks (disc tracks 2-10).  A 29-byte
 * mapping table in SKULL.EXP (offset 0x3dac from code base)
 * translates each HMP track index to a 1-based CDDA audio index
 * (0 = silence).
 *
 * Source: disassembly of SKULL.EXP (P3/Run386 flat-model binary,
 * HME-242 FM Towns release).  The mapping table was located by
 * scanning for a 29-byte array with values 0-9 using all 9 CDDA
 * tracks.
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_FMTOWNS_HMP_TRACK_COUNT     29

/* Map an HMP track index (0-28) to a 1-based CDDA audio index.
 * Returns 0 for silence (no CDDA track), 1-9 for audio indices.
 * The audio index maps to disc track (audio_index + 1) via CD.DAT. */
int dm2_v1_fmtowns_hmp_to_cdda(int hmp_track);

/* Return the full 29-entry mapping table (read-only). */
const uint8_t *dm2_v1_fmtowns_cdda_map_table(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_FMTOWNS_CDDA_MUSIC_H */
