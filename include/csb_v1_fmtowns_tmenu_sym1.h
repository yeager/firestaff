#ifndef CSB_V1_FMTOWNS_TMENU_SYM1_H
#define CSB_V1_FMTOWNS_TMENU_SYM1_H

#include <stdint.h>
#include "dm1_v1_fmtowns_edm_sym1.h"  /* reuses the entry struct */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Source-locked CSB TMENU.EXP SYM1 symbol table (1724 entries).
 *
 * Extracted 2026-08-07 from the CSB FM Towns launcher TMENU.EXP
 * (Victor 1990 disc). SYM1 blob at file offset 0x33a92, size
 * 31610 bytes. Same Phar Lap SYM1 wire format as DM1 EDM.EXP
 * (see `dm1_v1_fmtowns_edm_sym1.h`); reuses the entry struct.
 *
 * Contents include:
 *   * Phar Lap runtime symbols (`_mw*`, `_MW*`) same as DM1
 *     (identical Watcom C RTL stripped in).
 *   * TownsOS launcher symbols (CD_Drive_Reset, CHAR_SIZE,
 *     CARET_ONOFF).
 *   * Japanese-flavour developer names (BED_hentai, zahyouXY)
 *     that don't appear in the shipping game English EDM.EXP.
 *
 * This lets Firestaff's launcher-side integration (menu / disc
 * boot) reference CSB TMENU functions by their canonical Phar
 * Lap-recorded names — same lookup semantics as DM1 EDM SYM1.
 */

#define CSB_V1_FMTOWNS_TMENU_SYM1_COUNT  1724U

extern const dm1_v1_fmtowns_edm_sym1_entry_t
    csb_v1_fmtowns_tmenu_sym1_entries[CSB_V1_FMTOWNS_TMENU_SYM1_COUNT];

/* Binary-search lookup mirroring the DM1 API. */
const char *csb_v1_fmtowns_tmenu_sym1_name_for_vaddr_pc34(uint32_t vaddr);
uint32_t    csb_v1_fmtowns_tmenu_sym1_vaddr_for_name_pc34(const char *name);

#ifdef __cplusplus
}
#endif

#endif /* CSB_V1_FMTOWNS_TMENU_SYM1_H */
