# Pass 156 — DM1/V1 TITLE.C source timing lock

Evidence source: `~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/TITLE.C`, function `F0437_STARTEND_DrawTitle`.

Locked control-flow facts for the PC/ST title path:

- Zoom loop renders 18 generated title zoom steps.
- Each zoom step is preceded by exactly one `M526_WaitVerticalBlank()`.
- After the zoom loop, source waits two more vertical blanks.
- After the Master/Strikes Back blit + fade, source waits one final `M526_WaitVerticalBlank()` with BUG0_71 noting fast-computer timing risk before entrance/menu transition.
- The implementation keeps the decoded 53-frame TITLE.DAT bank as the render source, but exposes `V1_TitleFrontend_GetSourceTimingEvidence()` so probes/runtime can distinguish source-locked timing evidence from sparse emulator screenshots.

Validation target: `run_firestaff_v1_title_menu_cadence_layout_probe.sh` now reports `originalCadenceClaim=source-locked-pc-st-title-c` and asserts the locked constants above.
