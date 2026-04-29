# Pass 156 — DM1/V1 TITLE.C source timing lock

Evidence source: `~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/TITLE.C`, function `F0437_STARTEND_DrawTitle`.

Locked control-flow facts for the PC/ST title path:

- Zoom loop renders 18 generated title zoom steps.
- Each zoom step is preceded by exactly one `M526_WaitVerticalBlank()`.
- After the zoom loop, source waits two more vertical blanks.
- After the Master/Strikes Back blit + fade, source waits one final `M526_WaitVerticalBlank()` with BUG0_71 noting fast-computer timing risk before entrance/menu transition.
- The implementation keeps the decoded 53-frame TITLE.DAT bank as the render source, but exposes `V1_TitleFrontend_GetSourceTimingEvidence()` so probes/runtime can distinguish source-locked timing evidence from sparse emulator screenshots.

Validation target: `run_firestaff_v1_title_menu_cadence_layout_probe.sh` now reports `originalCadenceClaim=source-locked-pc-st-title-c` and asserts the locked constants above.


## Pass 175 update — source animation event schedule

`title_frontend_v1.[ch]` now exposes `V1_TitleFrontend_GetSourceAnimationStep()` for the ReDMCSB PC/F20 `TITLE.C` event schedule, separate from the decoded 53-frame `TITLE` bank. Locked schedule:

- step 1: `TITLE.C:319-324` presents strip at box `0,90,320,16`.
- steps 2..19: 18 reverse-order zoom blits from shrink index 17 -> 0, each with one `M526_WaitVerticalBlank()` (`TITLE.C:340-360`, `TITLE.C:385-387`). First zoom box is `136,74,48,12`; final zoom box is `0,40,320,80`.
- steps 20..21: two post-zoom VBlank waits (`TITLE.C:395-396`).
- step 22: Master/Strikes Back blit to `0,118,320,57` (`TITLE.C:397-402`; box source from `DATA.C:122`).
- step 23: final BUG0_71 guard VBlank before transition (`TITLE.C:409` / earlier equivalent `TITLE.C:251`).

Validation: `parity-evidence/pass175_title_source_animation_schedule.txt` reports `titleSourceAnimationScheduleInvariantOk=1`, `titleSourceTimingInvariantOk=1`, and `titleHandoffInvariantOk=1`.
