#ifndef FIRESTAFF_DM2_V1_FMTOWNS_MUSIC_LOOKUP_H
#define FIRESTAFF_DM2_V1_FMTOWNS_MUSIC_LOOKUP_H

/*
 * dm2_v1_fmtowns_music_lookup.h — End-to-end dungeon map → CDDA track lookup.
 *
 * Chains: SONGLIST.DAT (map→HMP) → SKULL.EXP table (HMP→CDDA) → CD.DAT (CDDA→disc).
 * PC path:   SONGLIST.DAT (map→HMP) → play HMP file directly.
 * FM Towns:  SONGLIST.DAT (map→HMP) → g_hmp_to_cdda[HMP] → CDDA index → disc track.
 */

#include <stdint.h>

#include "dm2_v1_fmtowns_cdda_music.h"
#include "dm2_v1_songlist_dat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Returns the CDDA audio index (1-9) for a given dungeon map on FM Towns.
 * Returns 0 (silence) if the map has no music or is out of range. */
static inline int dm2_v1_fmtowns_cdda_for_map(
    const DM2_V1_SonglistDat *songlist, int map_index)
{
    int hmp = dm2_v1_songlist_dat_track_for_map(songlist, map_index);
    if (hmp < 0) return 0;
    return dm2_v1_fmtowns_hmp_to_cdda(hmp);
}

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_FMTOWNS_MUSIC_LOOKUP_H */
