#ifndef FMTOWNS_PHARLAP_ALL_GAMES_H
#define FMTOWNS_PHARLAP_ALL_GAMES_H

#include <stdint.h>
#include "dm1_v1_fmtowns_pharlap_bridge.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Cross-game FM Towns Phar Lap P3 binary profile.
 *
 * Extends `dm1_v1_fmtowns_pharlap_bridge` beyond DM1 with byte-
 * verified call-slot profiles for every shipping FM Towns binary
 * across DM1 + CSB + DM2. Every count is a real measurement from
 * the extracted Track 01 image of the shipping disc.
 *
 * All 11 binaries use the SAME 4-slot Phar Lap bridge layout
 * (fs:[0x20] TBIOS, fs:[0x40] secondary, fs:[0x48] timing,
 * fs:[0x80] hardware init) verified in
 * `dm1_v1_fmtowns_pharlap_bridge`. That layout is therefore not a
 * DM1-only convention — it is the standard TownsOS / Phar Lap
 * runtime contract for every game on the platform.
 *
 * Discs used:
 *   DM1  : Dungeon Master (Japan) (En,Ja) (Rev 1)  (HMA-240, 1992)
 *   CSB  : Dungeon Master CSB Expansion FM-Towns   (Victor, 1990)
 *   DM2  : Dungeon Master II Skull Keep FM-Towns   (Victor, 1993)
 */

typedef struct {
    const char *name;
    const char *game;
    uint32_t    file_size;
    uint32_t    init_eip;
    uint32_t    sym1_offset;
    uint32_t    sym1_size;
    uint32_t    slot_tbios;
    uint32_t    slot_secondary;
    uint32_t    slot_timing;
    uint32_t    slot_hardware_init;
    uint32_t    slot_total;
} fmtowns_pharlap_binary_profile_t;

#define FMTOWNS_PHARLAP_BINARY_COUNT  11U

extern const fmtowns_pharlap_binary_profile_t
    fmtowns_pharlap_binary_profiles[FMTOWNS_PHARLAP_BINARY_COUNT];

/* Return the profile for `name` (short binary name, case-sensitive).
 * Returns NULL if the binary is not in the table. */
const fmtowns_pharlap_binary_profile_t *
fmtowns_pharlap_binary_profile_for_name_pc34(const char *name);

/* Return 1 iff every binary in the table uses only the 4 canonical
 * Phar Lap slots (no other fs:[disp32] calls with disp outside
 * {0x20, 0x40, 0x48, 0x80}). This is a compile-time invariant. */
int fmtowns_pharlap_all_binaries_use_canonical_slots_only_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FMTOWNS_PHARLAP_ALL_GAMES_H */
