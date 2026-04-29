# Entrance runtime transition source lock — 2026-04-29

Patch target: `main_loop_m11.c` V1-original launch path.

ReDMCSB source anchors:
- `ENTRANCE.C:409-421` builds the 5x5 entrance micro-dungeon behind the door screen.
- `ENTRANCE.C:426-443` fades/curtains to the entrance palette/screen.
- `ENTRANCE.C:446-595` draws C004 entrance screen, C002/C003 door graphics, and button/text surfaces.
- `ENTRANCE.C:850-883` waits in the entrance input loop.
- `ENTRANCE.C:906-935` plays the switch sound and delays `F0022_MAIN_Delay(20)` before door open.
- `ENTRANCE.C:142-304` / `F0438_STARTEND_OpenEntranceDoors()` runs animation steps 1..31, rattling on `step % 3 == 1`, moving door boxes 4px per step, guarded by one VBlank per step.

Firestaff compatibility implementation:
- Existing source schedule is `ENTRANCE_Compat_GetSourceAnimationStep()` / `ENTRANCE_Compat_GetDoorAnimationStep()`.
- `main_loop_m11.c` now calls that schedule in V1 original mode immediately after selected DM1 launch succeeds and before final game-view draw.
- This replaces the previous hard cut from launcher to live viewport with a runtime transition matching the ReDMCSB event order and 31-step door timing/geometry.
- Door graphics are currently palette-fill placeholders using the source door boxes. This is intentionally bounded: decoded C002/C003/C004 GRAPHICS.DAT blits can replace the fill without changing source timing/geometry.

Verification:
- `cmake --build build --target firestaff -j2` passed.
- `./run_firestaff_m11_game_view_probe.sh` passed: 592/592 invariants.
- `./test_entrance_frontend_pc34_compat_integration` passed: 31 door steps, 11 rattles, 38 source animation steps.

## 2026-04-29 follow-up: C004 runtime blit

The entrance pre-open surface now blits GRAPHICS.DAT graphic 4 (`C004`, 320x200) through the M11 asset loader when available, matching the ReDMCSB `ENTRANCE.C:446-595` source stage that draws the entrance screen before the door-opening loop.

C002/C003 door-panel blits remain bounded follow-up work; the timing and source door boxes from `ENTRANCE.C:142-304` / DATA.C are unchanged.

Verification after patch:

```text
cmake --build build --target firestaff -j2
./run_firestaff_m11_game_view_probe.sh  # 592/592 invariants passed
./test_entrance_frontend_pc34_compat_integration  # entranceSourceAnimationScheduleInvariantOk=1
```
