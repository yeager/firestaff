/*
 * csb_v1_teleporter_access_pc34_compat.h
 *
 * CSB V1 Teleporter Connection + Grey Lord (Dungeon GAP 6).
 * Source-locked per ReDMCSB DUNGEON.C:1085 (creature spawn
 * rules) + GROUP.C:2090-2150 (creature activation), and
 * the BUG0_69 + CHANGE7_19 fix.
 *
 * v1 (2026-06-14): helper csb_v1_can_creature_use_teleporter
 * keyed on the creature type.  Lord Chaos (0x16), Lord
 * Order (0x18), Grey Lord (0x1a) and the Materializer
 * (0x1b) can use teleporters; other creatures cannot
 * (DM1 PC 3.4 baseline).  CSB PC 3.4 widens the
 * accessibility via CHANGE7_19 + BUG0_69.
 */
#ifndef REDMCSB_CSB_V1_TELEPORTER_ACCESS_PC34_COMPAT_H
#define REDMCSB_CSB_V1_TELEPORTER_ACCESS_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* CSB V1 teleporter-accessibility toggle.  Default 0
 * (DM1 PC 3.4: only Lord-tier creatures can teleport).
 * When 1, the source-locked CSB rule applies: Lord
 * Chaos, Lord Order, Grey Lord, and Materializer can
 * use teleporters. */
int  csb_v1_teleporter_access_get(void);
void csb_v1_teleporter_access_set(int enabled);

/* Returns 1 when the supplied creature type can use
 * teleporters.  Uses csb_v1_teleporter_access_get() to
 * decide between CSB and DM1 accessibility lists. */
int  csb_v1_can_creature_use_teleporter(int creatureType);

#ifdef __cplusplus
}
#endif

#endif /* REDMCSB_CSB_V1_TELEPORTER_ACCESS_PC34_COMPAT_H */
