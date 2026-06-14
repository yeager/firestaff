/*
 * csb_v1_version_checker_pc34_compat.h
 *
 * CSB V1 Version Checker Sensor (Dungeon GAP 3, CHANGE7_23,
 * CHANGE8_06).  Source-locked per ReDMCSB MOVESENS.C
 * CHANGE7_23 (version-gated floor sensor) and BugsAndChanges.htm
 * CHANGE8_06 (engine version 21 hardcoded for CSB 2.1).
 *
 * Sensor type 9 (C009_VERSION_CHECKER) triggers only when
 *   data_value <= csb_engine_version().
 *
 * v1 (2026-06-14): single process-wide engine version with
 * the source-locked CSB 2.1 default.  Per-save persistence is
 * a follow-up gap.
 */
#ifndef REDMCSB_CSB_V1_VERSION_CHECKER_PC34_COMPAT_H
#define REDMCSB_CSB_V1_VERSION_CHECKER_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Engine version helpers.  CSB 2.1's engine version is 21
 * (CHANGE8_06 hardcodes this in MOVESENS.C).  DM1 PC 3.4's
 * engine version is 0.  Default is CSB 2.1 (21). */
int  csb_v1_engine_version_get(void);
void csb_v1_engine_version_set(int version);

/* Source-locked: returns 1 when data_value <= engineVersion
 * (CHANGE7_23 trigger condition).  DM1 PC 3.4 always returns
 * 1 (engine version 0, but 0 <= any data_value).  CSB 2.1
 * returns 1 only when data_value <= 21. */
int  csb_v1_version_checker_passes(int dataValue);

/* Convenience: returns 1 when this Firestaff build should
 * act like CSB 2.1 (engine version 21, version checker
 * active).  Returns 0 for DM1 PC 3.4 (engine version 0). */
int  csb_v1_is_csb_v21_mode(void);

#ifdef __cplusplus
}
#endif

#endif /* REDMCSB_CSB_V1_VERSION_CHECKER_PC34_COMPAT_H */
