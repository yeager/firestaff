#ifndef FIRESTAFF_DM2_V1_FMTOWNS_CDDA_MUSIC_H
#define FIRESTAFF_DM2_V1_FMTOWNS_CDDA_MUSIC_H

/*
 * dm2_v1_fmtowns_cdda_music.h — FM Towns HMP-to-CDDA track mapping.
 *
 * The PC version of DM2 has 29 HMP MIDI tracks stored in GDAT
 * category 4, type 3 (dtHMP), indices 0x00-0x1C.  SONGLIST.DAT
 * maps dungeon maps to these HMP track indices.
 *
 * The FM Towns release uses CDDA.  Its native SKULL.EXP holds the 29-byte
 * HMP-to-CDDA table at offset 0x3dac.  The receipt below is filled only by
 * parsing that span from the selected HME-242 CD image in RAM; it must not
 * be replaced with a copy of those values in Firestaff source.
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_FMTOWNS_HMP_TRACK_COUNT     29
#define DM2_FMTOWNS_SKULL_HMP_CDDA_OFFSET 0x3dacu

typedef struct {
    int valid;
    uint32_t source_offset;
    uint32_t source_size;
    uint32_t source_fnv1a;
    uint8_t hmp_to_cdda[DM2_FMTOWNS_HMP_TRACK_COUNT];
} DM2_V1_FmtownsCddaMusicReceipt;

/* Read SKULL.EXP's native table from caller-owned original-media bytes. */
int dm2_v1_fmtowns_cdda_music_parse(
    const uint8_t *skull_data, size_t skull_size,
    DM2_V1_FmtownsCddaMusicReceipt *out);

/* Map an HMP track index through an authenticated source receipt. */
int dm2_v1_fmtowns_hmp_to_cdda(
    const DM2_V1_FmtownsCddaMusicReceipt *receipt, int hmp_track);

const uint8_t *dm2_v1_fmtowns_cdda_map_table(
    const DM2_V1_FmtownsCddaMusicReceipt *receipt);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_FMTOWNS_CDDA_MUSIC_H */
