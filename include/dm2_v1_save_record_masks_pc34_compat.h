#ifndef DM2_V1_SAVE_RECORD_MASKS_PC34_COMPAT_H
#define DM2_V1_SAVE_RECORD_MASKS_PC34_COMPAT_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DM2 SKSAVE per-record-type SUPPRESS mask tables.
 * Source: skproject dm2data.cpp vsgame[120], table1d64db[16],
 *         skrecord.cpp table_recordsizes[16].
 *
 * The SUPPRESS writer uses these masks to decide which bits of each
 * record word are written to the save stream.  A 1-bit in the mask
 * means "write this bit"; a 0-bit means "suppress (skip)". */

#define DM2_RECORD_TYPE_COUNT 16

/* Per-record-type byte sizes (table_recordsizes[16]).
 * Types 11-13 have size 0 (unused). */
const uint8_t *dm2_v1_save_record_sizes(void);

/* Default per-record-type suppress mask pointers.
 * Returns NULL for types that have no mask (type 1, 11-13).
 * The returned pointer is into the vsgame master array and has
 * dm2_v1_save_record_sizes()[type] valid bytes. */
const uint8_t *dm2_v1_save_record_mask_for_type(int record_type);

/* Alternate creature mask (v1d648f = vsgame+0x2c): used when
 * DM2_QUERY_CREATURE_AI_SPEC_FLAGS returns flag bit 0 set. */
const uint8_t *dm2_v1_save_record_mask_creature_ai_spec(void);

/* Alternate container mask for map containers (v1d64b7 = vsgame+0x54):
 * used when DM2_IS_CONTAINER_MAP returns nonzero. */
const uint8_t *dm2_v1_save_record_mask_container_map(void);

/* Alternate misc-item mask for moneybox containers.
 * Default (v1d64bf = vsgame+0x5c) vs moneybox (v1d64c3 = vsgame+0x60). */
const uint8_t *dm2_v1_save_record_mask_misc_default(void);
const uint8_t *dm2_v1_save_record_mask_misc_moneybox(void);

/* Type 0xe alternate mask for v1d6521-active mode (v1d64d3 = vsgame+0x70). */
const uint8_t *dm2_v1_save_record_mask_type_0e_nested(void);

/* Master suppress mask array (vsgame[120]). */
const uint8_t *dm2_v1_save_vsgame_raw(size_t *out_size);

#ifdef __cplusplus
}
#endif

#endif /* DM2_V1_SAVE_RECORD_MASKS_PC34_COMPAT_H */
