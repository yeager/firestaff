#ifndef DM1_V1_FMTOWNS_EDM_SYM1_H
#define DM1_V1_FMTOWNS_EDM_SYM1_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Source-locked EDM.EXP SYM1 symbol table (1174 entries).
 *
 * Extracted 2026-08-07 from the shipping HMA-240 Japanese Rev 1
 * disc EDM.EXP (sha256 c888470d...ecb6) at file offset 0x46b41,
 * table size 0x51b5 bytes. Every entry maps a virtual address in
 * the loaded P3 image (relative to load base 0x200) to its Phar
 * Lap-recorded symbol name.
 *
 * Header layout:
 *   +0x00  "SYM1"
 *   +0x04  u16 version (0x000e)
 *   +0x06  u16 flags (0x0014)
 *   +0x08  u32 checksum-ish (0x51930000 in DM1 EDM.EXP)
 *   +0x0c  u16 reserved
 *   +0x0e  Special PROGRAM record: len(1)+"PROGRAM"(7)+8-byte trailing
 *   +0x1e  u32 section count / marker
 *   +0x22  First symbol: len(1) + name(N) + vaddr(4) + type(2)
 *
 * Each entry after the header is a variable-length record:
 *   1 byte  name length
 *   N bytes ASCII name
 *   4 bytes vaddr (LE u32)
 *   2 bytes type word (all shipping entries have type 0x0001)
 *
 * Entries below are sorted by vaddr for binary-search lookup.
 * Names include the leading underscore that Phar Lap prepends to
 * every C symbol (e.g. `_EXIT`, `_DECODE`) and preserve the
 * Watcom compiler's suffix convention (`__ONEXIT_TERMINATION`).
 *
 * Named vaddrs verified against Firestaff's disassembly evidence:
 *   DRAW_DMENU      @ 0x4620   -- matches menu_p3_disassembly.md
 *   DRAW_ICN_BUTTON @ 0x44f0   -- matches menu_p3_disassembly.md
 *   GET_LABEL       @ 0x43e4   -- matches menu_p3_disassembly.md
 */

typedef struct {
    uint32_t    vaddr;
    const char *name;
} dm1_v1_fmtowns_edm_sym1_entry_t;

#define DM1_V1_FMTOWNS_EDM_SYM1_COUNT  1174U

extern const dm1_v1_fmtowns_edm_sym1_entry_t
    dm1_v1_fmtowns_edm_sym1_entries[DM1_V1_FMTOWNS_EDM_SYM1_COUNT];

/* Look up the symbol name at the given vaddr via binary search.
 * Returns the symbol name on exact match, NULL otherwise. Names
 * are static string literals; caller must NOT free. */
const char *dm1_v1_fmtowns_edm_sym1_name_for_vaddr_pc34(uint32_t vaddr);

/* Look up the vaddr of a symbol by name (linear scan). Returns
 * the vaddr on match, or 0 if no symbol has that name. NULL name
 * input returns 0. */
uint32_t dm1_v1_fmtowns_edm_sym1_vaddr_for_name_pc34(const char *name);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_FMTOWNS_EDM_SYM1_H */
