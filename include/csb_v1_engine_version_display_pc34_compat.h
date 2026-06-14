/*
 * csb_v1_engine_version_display_pc34_compat.h
 *
 * CSB V1 Engine Version Display (Graphics GAP 2, CHANGE7_36,
 * CHANGE8_13).  Source-locked per ReDMCSB DIALOG.C:2014-2023
 * (engine version 2.0/2.1 printed in top right corner of
 * dialog boxes).  CHANGE8_13 hardcodes CSB version 2.1.
 *
 * v1 (2026-06-14): variant-aware helper
 * csb_v1_engine_version_display_get() returns "v2.0" for
 * DM1 PC 3.4, "v2.1" for CSB PC 3.4, or "v2.0" for the
 * default (DM1) when the CSB flag is off.
 */
#ifndef REDMCSB_CSB_V1_ENGINE_VERSION_DISPLAY_PC34_COMPAT_H
#define REDMCSB_CSB_V1_ENGINE_VERSION_DISPLAY_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Returns the source-locked version string for the
 * current game variant.  Caller passes the M12_Changelog_
 * VersionString-style "2.7.x" form; this helper
 * conditionally appends ".0" for DM1 (engine version 2.0)
 * or ".1" for CSB (engine version 2.1) to match
 * CHANGE8_13's hardcoded value.  Returns a static
 * string; do not free. */
const char* csb_v1_engine_version_display_get(void);

/* Variant toggle.  Default 0 (DM1, displays v2.0). */
void csb_v1_engine_version_display_set_csb(int isCsb);
int  csb_v1_engine_version_display_is_csb(void);

#ifdef __cplusplus
}
#endif

#endif /* REDMCSB_CSB_V1_ENGINE_VERSION_DISPLAY_PC34_COMPAT_H */
