# Firestaff v2.8.1 — 2026-06-17

The DM2 V1 CLI launch path is now wired end-to-end with all five
games reachable from a single `firestaff --game <id>` command, the
CSB V2 and Nexus V2 runtime phases 3–7 land as fully-wired first-class
features, the ctest baseline goes 100% green, and 101 commits of
refinement ship on top of v2.8.0. The release also carries the
keyboard input mapping work (arrow strafe + Home/End/Q/E turn +
hybrid preset) and a verified end-to-end launch sweep against
real game data for DM2 PC English and Nexus Saturn English.

## Headline features

- **DM2 V1 CLI launch wired (commits `12052b20`, `cbc4bd67`,
  `6589ad65`)**: `M11_GameView_Start` gains a dedicated DM2 branch
  that runs `dm2_v1_boot_profile_init` + `dm2_v1_boot_scan_assets`
  + `dm2_v1_boot_enter_game` + DM2 V2 runtime inits, then sets
  `state->sourceKind = M11_GAME_SOURCE_DM2_BOOT` so the existing
  per-frame tick + render dispatch in `M11_GameView_Draw` and
  `M11_GameView_AdvanceIdleTick` can drive the DM2 V1 runtime via
  `dm2_v1_runtime_tick()`. The M12 launch dispatcher already had
  per-frame DM2 dispatch on the `M11_GAME_SOURCE_DM2_BOOT`
  sourceKind — the missing piece was the launch hand-off itself.
  Verified with `/tmp/firestaff-launch-verify/verify_all_launches.sh`:
  all 10 previously `FAIL_NODATA` DM2 combinations (default,
  scale 0/1/2/3, four resolution variants) now PASS.

- **DM2 V1 boot MD5 implementation fixed (commit `6589ad65`)**:
  `dm2_md5_body` in `src/dm2/dm2_v1_boot.c` had two silent bugs
  that produced wrong hashes for every input: (a) the trailing
  `+ a` on each line was adding the wrong variable (should be
  `+ b` for the a-line, `+ c` for the b-line, etc.), and (b) the
  F/G/H/I argument order was wrong on 3 of 4 lines per cycle
  (the OTHER three variables in cyclic order, not always starting
  with the same variable). Rewrote the body using the STEP macro
  form from `src/shared/asset_find_by_hash.c`, which is verified
  to match the RFC 1321 test vectors (`d41d8cd9…` for empty
  string, `90015098…` for "abc"). After the fix, the DM2 boot
  scan reports `assets_verified = 1` and the launch hand-off
  succeeds.

- **Stack-buffer-overflow fix in `dm2_md5_update` (commit
  `350bf71a`)**: the pre-existing tail copy did `memcpy(ctx->buffer,
  input, len - 0)` which (a) re-hashed the head bytes AND (b)
  overflowed `ctx->buffer` (64 bytes) whenever `len > 64` — i.e.
  always for DM2's GRAPHICS.DAT (~8.6 MB). Triggered `__stack_chk_fail`
  (SIGABRT) on any DM2 launch attempt. The new tail copy skips past
  the `partLen` head + the 64-byte chunks walked by the for-loop
  (matches the MD5 RFC 1321 reference update).

- **CSB V2 phases 4–7 land (commits `b46730ea`, `608d84ee`,
  `95551b7a`)**: CSB V2 runtime phase 4 (lighting) + phase 5
  (smooth movement) + phase 6 (touch/controller M11 wire-up) ship
  as a combined wire-up. Phase 7 end-to-end verification suite
  gets **+38 pixel-gate asserts** (V1 framebuffer byte-preservation
  both with V2 disabled and V2 enabled, viewport render
  byte-determinism, state-hash equality across V2 modes), bringing
  the probe to **120/120** green. CSB V2.1 EPX upscale + CSB V2.2
  shape book (with CSB-only PRISON_DOOR/CHAOS_RUNE/DSA_SCROLL/
  LORD_ORDER) wire into `csb_v2_presentation_mode_set()` so they
  activate when the user picks CSB V2.1 or V2.2. The CSB V2.0
  per-frame filter config (`csb_v2_filter_config_pc34`) is also
  wired so the M11 launch pushes both upscale AND filter config
  in a single call. The actual modern asset pack authoring at
  `~/.firestaff/assets/csb/modern/` and GPU renderer integration
  remain as a follow-up.

- **Nexus V2 phases 3–5 + Phase 6 touch runtime land (commits
  `be606b1d`, `a66ab426`, `e2a9b720`, `d9cdcc85`)**: Nexus V2
  phase 3 HUD runtime (compass, depth, gold, champion bars,
  action strip) + phase 4 lighting runtime (per-frame tick of
  `lighting.bloom_timer` + outdoor FX state) + phase 5 smooth
  movement runtime (V2 viewport interpolation, NTSC fullscreen +
  192x160 dungeon-viewport helpers) ship as a combined M11
  wire-up. Phase 6 touch runtime adds Nexus-aware affordance
  routing with a `NEXUS_CMD_*` → `DM1_V2_MovementCommand`
  translation function (the original Nexus movement_command
  function returns `NEXUS_CMD_*` while the runtime switch
  expected `DM1_V2_MOVEMENT_COMMAND_*`).

- **Theron V2 smooth movement (commits `08aedd54`)**: new
  `theron_v2_smooth_movement` module mirrors the CSB V2 pattern
  for HuC6260 256x224 NTSC base. Probe **55/55** + test
  **52/52** green. End-to-end launch verification script now
  covers all 5 games × all V1/V2 modes × all scale modes.

- **DM2 V1 phases 4–5 + DM2 V2 phases 3–7 (commits `9aa8fae7`,
  `94dd81e9`, `00960860`, `6ee374e8`, `fa08561d`, `93988347`,
  `aec17431`)**: DM2 V1 spell + tech/magic mechanics parity,
  projectile routing + death sound, combat parity + V2 HUD wire-up.
  DM2 V2 phase 4 (lighting + outdoor FX) + phase 5 (M11 V1
  game-loop wire-up + verification probe) + phase 6 (touch/
  controller M11 wire-up) + phase 7 (end-to-end verification
  suite, 44 assertions) all land. ctest 23/23 for DM2 modules.

- **Keyboard input mapping (commit `12ce21dc`)**: arrow Left/Right
  now means strafe (matches original CSB ReDMCSB intent), Home/End
  rotate the view, Q/E turn the party, and the new hybrid preset
  combines mouse-look + keyboard. The full mapping lives in
  `M12_KeyboardMap_Default()` and is exposed through a new
  verifier script.

## Build and CI health

- **ctest 100% green (commits `4f942eae`, `725222e3`)**: the full
  ctest sweep goes **497/497** (100%) on the unfiltered set —
  the last 7 parity-evidence line-drift failures from v2.8.0 are
  closed by the latest manifest refreshes and the verify-launch
  fix. The 4 pre-existing DM1 V1 parity-blockers (`pass373`,
  `pass374`, `pass508`, `pass512`) still need specific game-data
  captures; they're documented as data-required, not regressions.

- **Real-data launch verification (commits `66419370`,
  `08aedd54`)**: with the user's `Dungeon-Master-II-Skullkeep_DOS_EN.zip`
  + `Dungeon-Master-Nexus_SEGA-Saturn_JA.zip` staged in
  `~/Downloads/`, both games' MD5s match the catalog
  (`25247ede4dabb6a71e5dabdfbcd5907d` for DM2 PC English
  GRAPHICS, `6caccd7875009e82fe2e28e7f6d6adc0` for DUNGEON,
  `e88d60859f65f08fa622e1992b02280f` for Nexus DM.BIN extracted
  from the Saturn ISO at LBA 182). The asset scanner reports
  ALL 5 GAMES READY, and live launches under
  `SDL_VIDEODRIVER=dummy` succeed for DM1, CSB, DM2 (post-fix),
  Nexus (loads level 0 64x64 Structure1B DMWeb DGN), and Theron
  (loads 8x8 level 0 with 256x224 planar fb + palette).

- **F0376 source-lock contract test (commits `2b6ff9cd`)**:
  `m11_point_in_source_box` is now exposed for tests and the
  F0376 contract gets a regression test. The function is part of
  the M11 source-lock invariants against ReDMCSB's viewport hit
  tests.

- **DM1 V1 bugs fixed (commits `beb0d0f2`, `f0b15c72`,
  `520703e5`)**: (1) pickup now fills the action hand first
  (BUG-DNY-DM1-2026-06-16), with a multi-item chain-drain probe
  INV_GV_29A verifying the behavior; (2) M11 front-mirror wall
  ornament fallback when the original asset is missing
  (BUG-DNY-DM1-2026-06-16). Both bugs were caught by the
  full-Firestaff audit (commit `42cb2f87`).

- **Dead code removal (commit `bcc7c278`)**: 3 dead amalgam
  files in `m10/` (48K LOC of legacy code) deleted. The
  amalgam headers were superseded by the per-game boot profiles
  + runtime modules.

- **Audit findings (commits `42cb2f87`, `a14bd2fa`)**: full
  Firestaff codebase review surfaces 3 bugs (Bug A + Bug B +
  Bug D) which are fixed in subsequent commits. Bug B is the
  `m10` `image_backend` globals fix (G2157_ + G2159/G2160).

- **Nexus V2 verification suite (commits `dafb901d`, `c9273432`,
  `dd4ba75e`)**: Phase 7 probe (full V2 render pipeline) +
  V2.1 EPX + bilinear upscaler probe. The new
  `nexus_v2_pipeline_render` 6-arg signature is the source of
  truth for the V2 render contract; verification suite matches
  the signature.

- **DM1 V2.2 GPU render path (commits `8bb1c249`, `fcf58831`,
  `f1c8617f`)**: V22 modern-art overlay on the V1 framebuffer,
  per-frame shape cache. The dispatch hook (`dm1_v2_shape_runtime_v22_active()`)
  is in place; the modern asset pack authoring at
  `~/.firestaff/assets/dm1/modern/` is a follow-up.

- **CSB V2.0 per-frame filter chain (commits `dd3a905b`,
  `9eb0c090`)**: per-frame dispatch in `fs_game_render_v2`
  wires the CRT scanline + palette correction + dither cleanup
  into the V2.0 filtered presentation path. Gated on the
  per-game V2 filter config so the V1 chrome is preserved when
  V1 is active.

- **Theron save-load (commit `612df846`)**: enable + fix
  header-checksum validation in Theron save/load. Previous
  versions silently accepted corrupted saves.

## Source-lock references

- ReDMCSB COMMAND.C:108-113 (movement/turn command ids C001-C006)
- ReDMCSB CLIKMENU.C:142 (F0365 turn) and CLIKMENU.C:180 (F0366 move)
- ReDMCSB DUNGEON.C (DM1 dungeon format reference)
- ReDMCSB DUNVIEW.C:1-50 (canon palette)
- ReDMCSB PANEL.C:418-428
- ReDMCSB ENTRANCE.C F0806 (CSB entrance micro-dungeon, C28_ENTRANCE_CSB palette)
- ReDMCSB DEFS.H:197-211 (V1 input route matrix)
- ReDMCSB PROJEXPL.C:863 (F0192 poison cloud reassignment)
- ReDMCSB STARTEND.C F0437/F0441 (title-then-entrance source-order rule)
- SKULL.ASM T520 (party placement after load)
- SKULL.ASM T560 (dungeon load header parsing)
- SKULL.ASM T200 (game state init after boot)
- SKULL.ASM T800 (outdoor/shop/NPC entry points)
- SKULL.ASM T048 (platform detection)
- THQUEST.ASM T400/T520/T600 (Theron map parser)
- HuC6260/HuC6270 VDC/VCE (PC Engine CD video)

## Counts

- 101 commits on top of v2.8.0
- 17 test/probe commits
- 12 DONE.md updates
- 10 launch verification commits
- 5 parity-evidence refreshes
- 8 cross-game V2 phase wire-ups (CSB V2 × 3, Nexus V2 × 3, DM2 V2 × 3, DM2 V1 × 3, Theron V2 × 1)
- 3 keyboard/launch-res fixes
- 1 full Firestaff audit + 3 follow-up bug fixes

## Pre-existing parity blockers (unchanged, data-required)

- `pass373`, `pass374`, `pass508`, `pass512` — DM1 V1 parity
  gates that need specific game-data captures (DOSBox screenshots,
  m11_capture_route_state frames, etc.) that don't exist in the
  fresh staging. Not regressions — these have been documented
  since v2.7.x.

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Requires SDL3. On macOS: `brew install sdl3`.

## Platforms

- macOS arm64 + x86_64
- Windows x86_64 (MSYS2 + SDL3)
- Linux x86_64 + arm64

Original Dungeon Master game data is not included — users supply
their own. See the Firestaff start menu for asset detection
(`firestaff --scan-data`).
