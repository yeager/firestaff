/*
 * csb_v1_dungeon_header_pc34_compat.h
 *
 * CSB V1 dungeon header parsing (Dungeon GAP 1, level count
 * 24 vs DM1 14).  Source-locked per ReDMCSB CEDTINC8.C:101-118
 * (CSBGAME.DAT vs DMSAVE.DAT routing) and M13_PLAN.md:303
 * (CSB DUNGEON.DAT header layout).
 *
 * v1 (2026-06-14): single helper that returns the expected
 * level count for a given dungeon-variant tag.  Callers
 * (csb_v1_dungeon_loader.c) use this to size their level
 * table dynamically instead of hardcoding 14 (DM1) or
 * 16 (legacy v1 cap).
 */
#ifndef REDMCSB_CSB_V1_DUNGEON_HEADER_PC34_COMPAT_H
#define REDMCSB_CSB_V1_DUNGEON_HEADER_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Dungeon variant tags (per CEDTINC8.C dispatch). */
#define CSB_V1_DUNGEON_VARIANT_DM1_PC34       0
#define CSB_V1_DUNGEON_VARIANT_CSB          1

/* Returns the source-locked level count for a variant.
 * DM1 PC 3.4: NumLevel() = 14.
 * CSB:        NumLevel() = 24.
 * The cap is 24 because CSB's 24-level layout is the widest
 * supported by the ReDMCSB decompilation; CSB v1.07 also
 * shipped 24.
 */
int csb_v1_dungeon_header_num_level(int variant);

/* Returns 1 when variant is CSB (CSBGAME.DAT header), 0
 * otherwise.  Used by the dungeon loader to pick the
 * CSB-inc8 routing path vs the DM1 routing path. */
int csb_v1_dungeon_header_is_csb(int variant);

#ifdef __cplusplus
}
#endif

#endif /* REDMCSB_CSB_V1_DUNGEON_HEADER_PC34_COMPAT_H */
