/*
 * csb_v1_cmp_import_pc34_compat.h
 *
 * CSB V1 Utility Disk Champion Portrait (.CMP) import flow.
 *
 * The CSB Utility Disk Champion Editor writes a champion's
 * metadata + portrait to a 496-byte .CMP file. The on-disk
 * format is parsed by FirestaffCmp_Decode() (see
 * firestaff_cmp_decode.h). This module glues that decoder
 * to the CSB V1 Champion runtime data structure
 * (CSB_V1_Champion in csb_v1_character_pc34_compat.h):
 *
 *   CMP file -> FirestaffCmp_Decode -> CSB_V1_Champion
 *
 * Three import entry points:
 *
 *   csb_v1_cmp_import_champion
 *     Decodes one 496-byte CMP buffer and writes the metadata
 *     + portrait into a CSB_V1_Champion record. The portrait
 *     bytes are copied into the champion's 3712-byte portrait
 *     slot (CSB_V1_PORTRAIT_BYTE_COUNT), preserving the Amiga
 *     4bpp chunky-to-planar layout produced by the CMP encoder
 *     (no bitplane conversion here -- that lives in
 *     ReDMCSB PORTRAIT.C F0515_CHAMPION_ConvertPortraits
 *     ToAtariSTPlanar and is a Tier 3 follow-up).
 *
 *   csb_v1_cmp_import_to_party
 *     Decodes a CMP buffer and inserts the resulting champion
 *     into a CSB_V1_PartyState at the next free slot
 *     (ChampionCount). Returns the slot index on success.
 *
 *   csb_v1_cmp_import_self_test
 *     Round-trip self-test. Returns 0 on success, -1 on failure.
 *
 * Source:
 *   - ReDMCSB DEFS.H CMP typedef (size 496 = 32 header + 464 portrait).
 *   - ReDMCSB PORTRAIT.C F0515_CHAMPION_ConvertPortraitsToAtariSTPlanar.
 *   - ReDMCSB CEDT002.C / CEDT021.C (Utility Disk Champion Editor flow).
 *   - CSBWin/CedtData.cpp CMP load/save (CSB Utility Disk tool).
 */
#ifndef FIRESTAFF_CSB_V1_CMP_IMPORT_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_CMP_IMPORT_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#include "firestaff_cmp_decode.h"
#include "csb_v1_character_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Import a 496-byte CMP buffer into a CSB_V1_Champion record.
 *
 * Returns:
 *   0  on success; champion->Name, champion->Title, and
 *      champion->Portrait[0..463] are populated.
 *  -1  on invalid arguments (NULL pointer, data_size < 496).
 *  -2  on invalid CMP magic (cmp_i_C/cmp_i_E != 0).
 *  -3  on invalid Name/Title characters.
 *  -4  on party/champion full.
 *
 * The portrait is copied verbatim from the CMP file into the
 * first 464 bytes of the champion's Portrait slot. The
 * remaining CSB_V1_PORTRAIT_BYTE_COUNT - 464 = 3248 bytes
 * are left untouched (callers that need zeroed padding
 * should initialise the champion first via
 * csb_v1_champion_init()).
 *
 * Note: this function does NOT set champion stats, skills,
 * vitals, or equipment. Those come from the DM1 champion
 * record, not from the CMP file (CMP only carries the
 * visual portrait + name/title for the Utility Disk editor).
 * Use csb_v1_character_import_dm1_save() / _dm1_buffer() to
 * import a full DM1 champion into a slot, then call
 * csb_v1_cmp_import_champion() to overlay the portrait from
 * the corresponding CMP.
 */
int csb_v1_cmp_import_champion(CSB_V1_Champion* champion,
                                const uint8_t*   cmp_data,
                                size_t           cmp_size);

/*
 * Import a CMP buffer into a CSB_V1_PartyState at the next
 * free slot. The champion slot is initialised via
 * csb_v1_champion_init() first (so vitals start at defaults
 * and unused portrait bytes are zeroed), then the CMP
 * metadata + portrait are applied.
 *
 * On success returns the slot index (0..CSB_V1_MAX_CHAMPIONS-1).
 * On failure returns a negative error code (same as
 * csb_v1_cmp_import_champion).
 */
int csb_v1_cmp_import_to_party(CSB_V1_PartyState* party,
                                const uint8_t*     cmp_data,
                                size_t             cmp_size);

/*
 * Round-trip self-test. Returns 0 on success, -1 on failure.
 * Exercises valid CMP import, bad magic, bad name, party-full,
 * portrait bytes copied, and Name/Title copied.
 */
int csb_v1_cmp_import_self_test(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_CMP_IMPORT_PC34_COMPAT_H */