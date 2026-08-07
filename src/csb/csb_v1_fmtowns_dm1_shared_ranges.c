#include "csb_v1_fmtowns_dm1_shared_ranges.h"
#include <stddef.h>

/* Byte-verified 2026-08-07 against real disc extracts:
 *   DM1 EDM.EXP   sha256... (from docs/fmtowns/all_games_real_data_hashes.json)
 *   CSB CHTWE.EXP sha256... (same manifest)
 * Every range below survived a bidirectional walk from a shared
 * seed byte until the first inequality on either side.
 */
const csb_v1_fmtowns_dm1_shared_range_t
csb_v1_fmtowns_dm1_shared_ranges[CSB_V1_FMTOWNS_DM1_SHARED_RANGE_COUNT] = {
    /* OICON descriptor + neighbours */
    { 0x226a8u, 0x28144u,  1816u, 0x224a8u, 0x27f44u },
    /* Immediately-following data block */
    { 0x23439u, 0x28fa1u,  1511u, 0x23239u, 0x28da1u },
    /* Menu/spell tables block */
    { 0x247e8u, 0x2a394u,  6842u, 0x245e8u, 0x2a194u },
    /* Large shared payload — 33194 bytes, largest single shared
     * region between the two binaries. Contains music/spell/
     * layout tables that are byte-identical across DM1 and CSB. */
    { 0x29776u, 0x2d666u, 33194u, 0x29576u, 0x2d466u },
};

int csb_v1_fmtowns_dm1_to_csb_file_offset_pc34(
        uint32_t dm1_file_offset, uint32_t *csb_file_offset_out) {
    if (!csb_file_offset_out) return 0;
    for (int i = 0; i < CSB_V1_FMTOWNS_DM1_SHARED_RANGE_COUNT; ++i) {
        const csb_v1_fmtowns_dm1_shared_range_t *r =
            &csb_v1_fmtowns_dm1_shared_ranges[i];
        if (dm1_file_offset >= r->dm1_file_offset &&
            dm1_file_offset <  r->dm1_file_offset + r->length_bytes) {
            *csb_file_offset_out = r->csb_file_offset +
                                    (dm1_file_offset - r->dm1_file_offset);
            return 1;
        }
    }
    return 0;
}
