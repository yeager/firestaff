/*
 * csb_v1_save_import_path_pc34_compat.c
 *
 * Source-locked per M13_PLAN.md:280-294 (HoC delta) +
 * DEFS.H:1289 (CSBGAME.DAT magic).  v1 dispatches + stubs.
 */
#include "csb_v1_save_import_path_pc34_compat.h"

#include <string.h>
#include <stdio.h>

CSB_V1_SaveVariant csb_v1_detect_save_variant(
    const unsigned char* header, int headerLen) {
    static const unsigned char kDm1Magic[8] = {
        'R','D','M','C','S','B','1','5'
    };
    if (!header || headerLen < 8) {
        return CSB_V1_SAVE_VARIANT_UNKNOWN;
    }
    if (memcmp(header, kDm1Magic, 8) == 0) {
        return CSB_V1_SAVE_VARIANT_DM1_PC34;
    }
    /* CSB v2.x magic bytes per DEFS.H:1289.  Different
     * versions of CSB used different 8-byte magics; v1
     * only recognises v2.0 and v2.1 explicitly. */
    if (memcmp(header, "CSBGAME\0", 8) == 0) {
        /* The 9th-12th bytes hold the version (uint32 LE). */
        if (headerLen < 12) return CSB_V1_SAVE_VARIANT_UNKNOWN;
        unsigned int v = (unsigned)header[8]  |
                         ((unsigned)header[9]  << 8) |
                         ((unsigned)header[10] << 16) |
                         ((unsigned)header[11] << 24);
        if (v == 0x00000200u) return CSB_V1_SAVE_VARIANT_CSB_V20;
        if (v == 0x00000201u) return CSB_V1_SAVE_VARIANT_CSB_V21;
        return CSB_V1_SAVE_VARIANT_UNKNOWN;
    }
    return CSB_V1_SAVE_VARIANT_UNKNOWN;
}

int csb_v1_save_import_path_implemented(void) {
    /* v1: CSB-specific import path is OPEN-OMFATTANDE.  The
     * detector is wired but the loader body is a stub that
     * returns 0 (= use DM1 path).  Bumping this to 1 is
     * the entry point for the future CSB-specific loader
     * milestone (Champions GAP 3 full closure). */
    return 0;
}

int csb_v1_import_csb_save(const char* path) {
    /* Stub.  In the future this should:
     *   1. Read the CSB save file
     *   2. Detect the variant (csb_v1_detect_save_variant)
     *   3. Walk the CSB section list (different from DM1)
     *   4. Map CSB champion roster / party stat bytes to
     *      Firestaff ChampionState_Compat / PartyState
     *   5. Apply the per-version delta (v2.0 vs v2.1)
     *
     * v1 returns 0 to signal "not implemented; caller should
     * fall back to the DM1 1:1 loader via the file dialog
     * path". */
    (void)path;
    return 0;
}
