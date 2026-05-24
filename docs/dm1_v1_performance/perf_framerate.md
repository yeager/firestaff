# DM1 V1 — Framerate / VBLANK Timing

## ReDMCSB source

**VBLANK frequency**
- `Documentation/Engine.htm` (game time section):  
  > "vertical blank interrupt cycles (which occur at **60Hz for North American systems / 50Hz for European systems**)"

**Frame budget — input wait**
- `GAMELOOP.C:47-51` / `F0002_MAIN_GameLoop_CPSDF`:  
  - ST versions (MEDIA007): `G0318_i_WaitForInputMaximumVerticalBlankCount = 10`
  - I34/X31J versions (MEDIA722): `G0318_i_WaitForInputMaximumVerticalBlankCount = 12`
- `GAMELOOP.C:164-219`: main loop spins on keyboard + command queue until both:
  1. `G0321_B_StopWaitingForPlayerInput` is set (by VBLANK handler at line 1035)
  2. `G0301_B_GameTimeTicking` is true (also set by VBLANK handler when idle > 11 cycles)
- `BASE.C:1035`: VBLANK handler sets `G0321_B_StopWaitingForPlayerInput = C1_TRUE` every VBLANK tick

**Synchronization call**
- `DRAWVIEW.C:721-722` / `F0097_DUNGEONVIEW_DrawViewport`:  
  `M526_WaitVerticalBlank()` — blocks until the next VBLANK fires, ensuring the composed viewport bitmap hits the screen at scan-out time.
- `DEFS.H:3134-3166`: `M526_WaitVerticalBlank()` maps to `Vsync()` on ST, `F0693_WaitVerticalBlank()` on Amiga/Atari ST, `WaitTOF()` on PC/Mac.

**Conclusion**
DM1 targets one viewport redraw per VBLANK interrupt. Input wait maxes at 10–12 VBLANK cycles (~167–240 ms at 50 Hz). If no input arrives within that window the game clock still advances (idle time tick at 11 cycles without activity in older versions — documented in Engine.htm as "11 vertical blank interrupt cycles").

## Firestaff coverage
- `probes/dm1/firestaff_dm1_v1_game_loop_redraw_cadence_probe.c` — locks F0128→F0097 call chain per VBLANK
- `tools/verify_pass351_dm1_v1_live_viewport_redraw_parity_sweep.py` — sweeps redraw-before-cooldown-before-command-wait order

## Status
✅ SOURCE-LOCKED — framerate/vblank timing documented with file:line citations.
