# Runtime TITLE intro source lock — 2026-04-29

Problem: the preview entered the launcher/game path without any visible DM1 TITLE animation, despite the repository already containing pass-56/58 TITLE.DAT decoding probes.

ReDMCSB source evidence used as the runtime contract:

- `Toolchains/Common/Source/TITLE.C` `F0437_STARTEND_DrawTitle` loads the TITLE graphic bank and drives the frontend sequence.
- `TITLE.C:430` draws the `PRESENTS` strip.
- `TITLE.C:456-457` waits `M526_WaitVerticalBlank()` then blits each reverse-order zoom step to `C425_ZONE_TITLE_CHAOS`.
- `TITLE.C:460` calls `F0022_MAIN_Delay(20)` after the zoom.
- `TITLE.C:461` draws `CM60_NEGGRAPHIC_TITLE_STRIKES_BACK`.
- `TITLE.C:463` applies the final delay/guard before transition.

Implementation delta:

- `main_loop_m11.c` now resolves the original DM1 `TITLE` file from `FIRESTAFF_TITLE_DAT`, the matched DM1 asset directory, or common extracted DM PC 3.4 data-dir layouts.
- In V1 original presentation mode, runtime now plays all `V1_TITLE_DAT_FRAME_MAX` decoded original TITLE frames before drawing the launcher.
- The existing pass-58 frontend emits PC34 packed 4bpp screen bitmap data; runtime unpacks it to M11's one-byte indexed framebuffer before `M11_Render_PresentIndexed()`.
- `CMakeLists.txt` now links `title_dat_loader_v1.c` and `title_frontend_v1.c` into `firestaff_m11`, so the visible runtime path uses the same source-backed code as the probes.

Verification:

- `cmake --build build --target firestaff -j2` passed.
- `./run_firestaff_v1_title_frontend_probe.sh .../DungeonMasterPC34/TITLE` passed `5/5` invariants.
- `SDL_VIDEODRIVER=dummy FIRESTAFF_TITLE_DAT=.../DungeonMasterPC34/TITLE ./build/firestaff --duration 300 --data-dir .../DungeonMasterPC34` exited `0`.
- `ctest --test-dir build -R 'm11_' --output-on-failure` passed 6/7 before `m11_launcher_smoke`; the smoke script originally failed to link because its manual compile list omitted the title runtime objects. `run_firestaff_m11_phase_a_probe.sh` was updated to include them. The full launcher smoke is very slow because it rebuilds huge probes manually; direct rebuilt binary smoke with inline dummy SDL env exited `0`.
