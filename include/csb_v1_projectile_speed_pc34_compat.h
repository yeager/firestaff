/*
 * csb_v1_projectile_speed_pc34_compat.h
 *
 * CSB V1 Projectile Speed Normalization (Champions / Combat
 * GAP 1, CHANGE7_20).  Source-locked per PROJEXPL.C
 * CHANGE7_20_IMPROVEMENT: "Projectiles moved slower on maps
 * other than the party map; CSB fix: Projectiles now move at
 * full speed on ALL maps."
 *
 * v1 (2026-06-14): single process-wide flag.  When set, F0825
 * uses delay=1 on every map (party map or otherwise).  When
 * unset, the DM1 behaviour is preserved (delay=1 on party
 * map, delay=3 on other maps).  Default is 0 (DM1 behaviour).
 *
 * csb_v1_save_load.c should call
 * csb_v1_projectile_speed_normalization_set(1) on save load
 * for CSB games.  (Per-save persistence is a follow-up gap.)
 */
#ifndef REDMCSB_CSB_V1_PROJECTILE_SPEED_PC34_COMPAT_H
#define REDMCSB_CSB_V1_PROJECTILE_SPEED_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

int  csb_v1_projectile_speed_normalization_get(void);
void csb_v1_projectile_speed_normalization_set(int enabled);

#ifdef __cplusplus
}
#endif

#endif /* REDMCSB_CSB_V1_PROJECTILE_SPEED_PC34_COMPAT_H */
