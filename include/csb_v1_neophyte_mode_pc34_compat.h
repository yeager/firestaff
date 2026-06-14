/*
 * csb_v1_neophyte_mode_pc34_compat.h
 *
 * CSB-only neophyte-skill display mode.  Source-locked per
 * ReDMCSB Character.cpp:665 (skill validation), data.cpp:88
 * (default neophyteSkills = false), Recording.cpp:246
 * (replay neophyte mode set true), and PANEL.C:26 + CEDT006.C:141
 * (NEOPHYTE rank at index 0 of the rank-name table).
 *
 * ReDMCSB: "if (neophyteSkills || skillLevel > 0) accept else
 * reject" — in DM1 the lowest valid skillLevel is 1 (NOVICE);
 * CSB extends the valid range down to 0 (NEOPHYTE) when
 * neophyteSkills is true.
 *
 * v1 (2026-06-14): single process-wide flag (no per-save
 * persistence yet — that's a follow-up gap).  M11 reads the
 * flag through m11_dm1_v1_skill_level_name_pc34 to decide
 * whether to display "NEOPHYTE" or "NOVICE" at level 0.
 */
#ifndef REDMCSB_CSB_V1_NEOPHYTE_MODE_PC34_COMPAT_H
#define REDMCSB_CSB_V1_NEOPHYTE_MODE_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* CSB neophyte-mode toggle.  When non-zero, skillLevel == 0 is
 * accepted as NEOPHYTE and displayed as such.  When zero, the
 * M11 renderer maps skillLevel == 0 to NOVICE for display (the
 * DM1 behaviour).  Default is 0.  Callers that load CSB saves
 * (csb_v1_save_load.c) set this to 1 on load. */
int  csb_v1_neophyte_skills_mode_get(void);
void csb_v1_neophyte_skills_mode_set(int enabled);

/* Helper: returns 1 when level == 0 should be displayed as
 * NEOPHYTE rather than NOVICE.  The M11 renderer calls this
 * before falling back to the rank-name table. */
int  csb_v1_neophyte_display_for_level(unsigned int level);


#ifdef __cplusplus
}
#endif

#endif /* REDMCSB_CSB_V1_NEOPHYTE_MODE_PC34_COMPAT_H */
