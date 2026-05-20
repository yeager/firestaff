# TITLE.DAT swoosh path/decode closure - 2026-05-20

Scope: closes the stale TODO item "`Swoosh animation not playing - TITLE.DAT path/decode issue`" for the DM1 V1 TITLE zoom/swoosh animation. This does not claim standalone PC `SWOOSH` executable logo playback beyond the existing SWSH source schedule test.

ReDMCSB evidence:

- `Toolchains/Common/Source/TITLE.C:424-430` loads `C001_GRAPHIC_TITLE`, registers the TITLE negative graphics, fades to pre-title palette, and draws `PRESENTS`.
- `TITLE.C:433-446` builds the shrinked TITLE bitmaps used by the zoom.
- `TITLE.C:452-458` fades, waits one vertical blank per zoom step, blits reverse-order zoom frames to `C425_ZONE_TITLE_CHAOS`, and frees each bitmap.
- `TITLE.C:460-463` delays, draws `CM60_NEGGRAPHIC_TITLE_STRIKES_BACK`, fades to the final pre-title palette, then applies the final guard delay before handoff.
- Ambiguity guard: standalone PC `SWOOSH` is separate. `SWSH.C:10-47` starts PSG sound, applies palette commands, then `Pexec`s `START.PRG`; `SWSH.C:54-68` sets the physical screen target; `SWSH.C:101-143` decodes the embedded logo bitmap; `SWSH.C:281-308` defines the palette command stream. Firestaff keeps this as `swsh_frontend_pc34_compat` source-schedule evidence.

Firestaff evidence:

- Runtime path resolution is in `src/engine/main_loop_m11.c:m11_find_title_dat_for_intro`, including `FIRESTAFF_TITLE_DAT`, matched DM1 asset parents/grandparents, data-dir layouts, and canonical OpenClaw DM1 anchors.
- Runtime playback is in `src/engine/main_loop_m11.c:m11_play_redmcsb_title_intro_if_available`: it decodes all 53 `TITLE` frames through `V1_TitleFrontend_RenderFrameToScreen`, unpacks to M11 indexed framebuffer, presents with `VGA_PALETTE_PC34_SPECIAL_TITLE`, and uses the source timing helpers for frame/final guard delays.
- Decode path is `src/frontend/title_dat_loader_v1.c` and frontend packing is `src/frontend/title_frontend_v1.c`.

Verification run in this worktree:

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build build --target test_title_frontend_runtime_cadence_pc34_compat test_swsh_frontend_pc34_compat_integration --parallel 4
ctest --test-dir build -R 'title_frontend_runtime_cadence_source_lock|swsh_frontend_source_animation_schedule' --output-on-failure
cc -Iinclude probes/v1/firestaff_v1_pass56_title_dat_probe.c src/frontend/title_dat_loader_v1.c -o build/firestaff_v1_pass56_title_dat_probe
cc -Iinclude probes/v1/firestaff_v1_pass57_title_render_probe.c src/frontend/title_dat_loader_v1.c -o build/firestaff_v1_pass57_title_render_probe
cc -Iinclude probes/v1/firestaff_v1_pass58_title_frontend_probe.c src/frontend/title_frontend_v1.c src/frontend/title_dat_loader_v1.c -o build/firestaff_v1_pass58_title_frontend_probe
./build/firestaff_v1_pass56_title_dat_probe
./build/firestaff_v1_pass57_title_render_probe
./build/firestaff_v1_pass58_title_frontend_probe
```

Results:

- `ctest`: 2/2 passed (`title_frontend_runtime_cadence_source_lock`, `swsh_frontend_source_animation_schedule`).
- pass56: local `TITLE` at `/Users/bosse/.firestaff/data/TITLE`, 12002 bytes, 59 records, 2 `EN` + 51 `DL`, 53 320x200 frames, 8/8 invariants, fingerprint `0x6ce154a7`.
- pass57: decoded 53 original TITLE frames, palette split 37 + 16, index fingerprint `0xb4e5d330`, RGB fingerprint `0x143fa969`, 6/6 invariants.
- pass58: frontend renders original TITLE data into the 320x200 V1 screen bitmap, reaches first/boundary/last frames, and sampled packed frames match pass57 output, 5/5 invariants.

Conclusion: the TODO marker is stale for the named `TITLE.DAT` path/decode failure. The current V1 original handoff can resolve and decode the local original TITLE file, and the source-locked runtime cadence/SWSH schedule gates pass.
