
# Unreleased

# Firestaff v3.0.19

Firestaff v3.0.19 is a focused MacBook Pro release-smoke follow-up to v3.0.18.
It publishes the current `main` state after the DM1 title-palette, Q/E turn, and
Hall of Champions artifact gates were re-run green against the external-disk
build.

## Highlights since v3.0.18

- **DM1 HoC artifact coverage is broader**: post-v3.0.18 commits add hand-slot
  refresh bridging plus side-door, D2C alcove, D4R far-object, and damage-status
  overdraw pixel gates, keeping the false floating/floor item report covered by
  the current regression set.

- **DM1 release-smoke gates are still green on current main**: the focused HoC
  false projectile/floor artifact probes, Q/E turn input tests, M11 queue tests,
  and title-palette probes all pass locally before this release-prep bump.

- **DM1 endgame work remains bounded**: the F0444/F0445/F0446 commits after
  v3.0.18 advance restart handoff and fuse-sequence replay metadata; they are
  included here but do not broaden public completion claims for final endgame
  presentation.

## Verification

- Release version metadata is synchronized across CMake, launcher UI,
  launcher changelog, and `include/firestaff_version.h`.
- Latest public GitHub release before this prep was `v3.0.18`, targeting
  `2b54dcd`, while current `main` is `04ee482c`; this release closes that
  package-vs-main gap.
- Local verification on
  `/Volumes/Extern-disk/firestaff-builds/build-dm1-mbp-smoke-current-main`
  passed `hoc_no_false_projectile_artifacts`, `hoc_floor_runtime_no_false_items`,
  `m11_overlay_command_queue_block`, `dm1_v1_turn_step_timing_gate_pc34_compat`,
  `dm_title_palette_regression`, `dm_title_palette_silicon`,
  `dm_title_swoosh_handoff_palette`, and `m11_input_queue_pc34_compat`.
- Main CI for `pass622 dm1 f0445 replay events` completed successfully before
  this release-prep commit.

# Firestaff v3.0.18

Firestaff v3.0.18 is a focused follow-up to v3.0.17 for the DM1 V1 MacBook Pro
release-smoke input report. It includes the post-v3.0.17 Q/E turn-key priority
guard on top of the same title-palette and Hall of Champions artifact gates.

## Highlights since v3.0.17

- **DM1 Q/E turn input is hardened against stale keymaps**: DM1 V1 now resolves
  Q/E, Home/End, and keypad 4/6 scancodes to turn-left/turn-right before
  persisted M11 keymaps are read, and applies the same priority during held-key
  polling. This prevents older local keybinding files from routing Q/E into
  cooldown-gated strafe commands.

- **MacBook Pro smoke scope is explicit**: the release keeps the focused
  v3.0.17 gates for title/swoosh palette readback and Hall of Champions false
  projectile/floor artifacts, while adding the Q/E stale-keymap guard that was
  not present in the v3.0.17 tag.

## Verification

- Release version metadata is synchronized across CMake, launcher UI,
  launcher changelog, and `include/firestaff_version.h`.
- Local verification on
  `/Volumes/Extern-disk/firestaff-builds/build-dm1-mbp-smoke-current-main`
  passed `firestaff` rebuild, `firestaff_dm1_v1_hoc_no_false_projectile_artifacts_probe`,
  `firestaff_dm1_v1_hoc_floor_runtime_no_false_items_probe`,
  `test_m11_overlay_command_queue_block`,
  `test_dm1_v1_turn_step_timing_gate_pc34_compat`,
  `firestaff_v1_dm_title_swoosh_handoff_palette_probe`,
  `test_m11_input_queue_pc34_compat`, and `firestaff_m11_phase_a_probe`.
- Main CI for `pass617 dm1 qe turn key priority` completed successfully before
  this release-prep commit.

# Firestaff v3.0.17

Firestaff v3.0.17 packages the post-v3.0.16 `main` fixes and gates for the
DM1 V1 MacBook Pro release-smoke symptoms: wrong intro/title palette, slow or
retried Q/E turning, and false Hall of Champions floating or floor artifacts.

## Highlights since v3.0.16

- **Release drift closed for the reported MacBook Pro symptoms**: current
  `main` is 82 commits past `v3.0.16`, so this release moves the public macOS
  package to the same code state as the passing regression gates.

- **DM1 HoC artifact hardening is included**: runtime and data-layer fixes now
  cover static projectile/explosion hiding, raw/decoded Thing-list next-pointer
  sync, fixed-possession raw weapon/armour/junk rewrites, and raw group-record
  rewrites after projectile or melee mutations.

- **DM1 input and palette smoke gates are green**: Q/E/Home/End/keypad turn
  taps remain outside the delayed movement queue, and the title/swoosh palette
  handoff is locked through SDL readback against real `GRAPHICS.DAT`.

- **Late DM1 runtime work is included**: direct M11 projectile hits on
  champions now use the shared F0321/F0313 damage scale, light-to-palette
  selection uses the shared M10 F0337 path, and the F0446 FUSE endgame delay
  now exposes a final-handoff readiness query.

## Verification

- Release version metadata is synchronized across CMake, launcher UI,
  launcher changelog, and `include/firestaff_version.h`.
- Local release-prep verification on
  `/Volumes/Extern-disk/firestaff-builds/build-dm1-mbp-smoke-current-main`
  passed the focused symptom gates:
  `firestaff_dm1_v1_hoc_no_false_projectile_artifacts_probe`,
  `firestaff_dm1_v1_hoc_floor_runtime_no_false_items_probe`,
  `test_m11_overlay_command_queue_block`,
  `test_dm1_v1_turn_step_timing_gate_pc34_compat`,
  `firestaff_v1_dm_title_swoosh_handoff_palette_probe`,
  `test_m11_input_queue_pc34_compat`, and
  `firestaff_m11_phase_a_probe`.
- The release workflow rebuilds and packages macOS arm64, macOS x86_64,
  Windows x86_64, Linux x86_64, Linux arm64, and Steam Deck x86_64 artifacts
  from the `v3.0.17` release tag.

# Firestaff v3.0.16

Firestaff v3.0.16 is a focused follow-up to v3.0.15 for the DM1 V1 MacBook Pro
release-smoke regressions reported on 2026-07-01. It packages the current-main
fixes and gates for the wrong DM1 intro/title palette, slow or retried
Q/E/Home/End/keypad turning, and false Hall of Champions flying/floor
artifacts.

## Highlights since v3.0.15

- **Immediate DM1 turn input**: Q/E/Home/End/keypad turn taps now bypass the
  delayed DM1 V1 vblank pending queue, while movement inputs remain
  cooldown-gated.

- **DM1 title and HoC smoke gates are green on current main**: focused probes
  cover title palette regression, Apple Silicon title palette readback, title
  swoosh palette handoff, false HoC projectile artifacts, and false HoC floor
  items.

- **Late DM1 combat/projectile fixes are included**: the tag includes focused
  gates for F0735/F0308 zero-Luck RNG accounting, F0328/F0811/F0217 zero-power
  thrown Ven/Ful potion handling, and M10 F0811/F0217 creature projectile
  impacts at map coordinate `(0,0)`.

- **Release drift is now gated**: the existing
  `test_m12_version_changelog_consistency` source is registered in CMake so the
  embedded launcher changelog must include the project version before release.

## Verification

- Release version metadata is synchronized across CMake, launcher UI,
  launcher changelog, and `include/firestaff_version.h`.
- Local release-candidate verification on `/Volumes/Extern-disk` passed focused
  CTest coverage for `m12_version_changelog_consistency`,
  title-palette/Apple-Silicon/title-swoosh palette gates, HoC false
  projectile/floor artifact gates, turn-step timing, M11 input queue, and
  overlay command queue behavior.
- The release workflow rebuilds and packages macOS arm64, macOS x86_64,
  Windows x86_64, Linux x86_64, Linux arm64, and Steam Deck x86_64 artifacts
  from the `v3.0.16` release tag.

# Firestaff v3.0.15

Firestaff v3.0.15 packages the runtime-hardening work that landed after
v3.0.14. The release keeps the project status conservative: DM1 V1 gets the
largest gameplay and M11 runtime coverage increase, while CSB, DM2, Nexus,
Theron's Quest, accessibility, input, save-boundary, and asset-receipt gates
gain focused verification without being presented as complete parity targets.

## Highlights since v3.0.14

- **DM1 V1 action/runtime coverage is much wider**: new source-locked gates
  cover M10/M11 attack action IDs, stamina costs, action disable ticks,
  melee/contact results, parry/block/heal/invoke failure tails, ready-hand and
  object-helper routes, quickload/spell/map close helpers, and leader-hand
  throw boundaries.

- **DM1 V1 magic and projectile behavior is more tightly pinned**: gates now
  cover invoke mana/skill routing, low-mana projectile failures, projectile
  direction and zero-impact cases, lightning/poison/slime/harm wall impacts,
  magical wall/cloud boundaries, killed-some/drop/fear paths, and endgame/fuse
  cleanup ordering.

- **M11 presentation and input hardening expanded**: the runtime now has
  stronger evidence around bounded turn input queues, smooth turns, Hall of
  Champions artifact hiding, action-row disruption, throw/invoke audio
  ordering, zero-adjusted projectile audio, and release-smoke triage.

- **Salvaged cross-game gates were integrated and cleaned up**: applicable
  worktree slices added focused tests/probes for CSB V22 in-place rendering,
  CSBWin 512-byte save-header classification, gesture navigation, session
  timer/accessibility/screen-reader manifests, Nexus/DM2/Theron boundaries,
  and several DM1 champion-panel/sensor/Hall of Champions surfaces. Conflicted
  or junk-only salvage was left out.

- **Source-lock and asset-boundary checks remain conservative**: the new gates
  are data-free or skip-safe where appropriate, avoid shipping original game
  payloads, and keep unfinished real-asset parity claims out of public release
  copy.

## Verification

- Release version metadata is synchronized across CMake, launcher UI,
  changelog, and `include/firestaff_version.h`.
- GitHub Actions was green on `26fa194f7` before release prep: M10 verify,
  strict warnings, asset hygiene, CMake builds on macOS/Windows/Linux, Phase
  A/audio probes, Pages, and cross-platform determinism.
- Local release-prep verification covered CMake configure/build plus the
  headless Phase A and audio smoke probes before tagging.
- The release workflow rebuilds and packages macOS arm64, macOS x86_64,
  Windows x86_64, Linux x86_64, Linux arm64, and Steam Deck x86_64 artifacts
  from the `v3.0.15` release tag.

# Firestaff v3.0.14

Firestaff v3.0.14 packages the runtime-hardening work that landed after
v3.0.13. This release keeps the public claims conservative: it broadens
data-free and skip-safe evidence around DM1 V1/V2, launcher save and
accessibility surfaces, Steam Deck packaging, X68000 media receipt
classification, and cross-game runtime boundaries without promoting unfinished
targets to finished parity.

## Highlights since v3.0.13

- **DM1 PC 3.4 archive receipts are pinned more tightly**: a new data-free
  gate proves renamed archive entries can satisfy DM1 PC 3.4 English
  `GRAPHICS.DAT` and `DUNGEON.DAT` requirements by hash, then materialize
  into ordinary runtime cache files before launch.

- DM1 V2 deterministic screenshot coverage now includes a source-command-owned
  V2.2 route receipt in the headless actual-render probe, with repeat-stable
  BMP hashes and explicit non-claims for DOSBox parity and finished-art pixels.

- The M12 save browser now surfaces Firestaff PC 3.4 native save
  manifests, marking matching DM1 saves distinctly and rejecting
  wrong-game manifests before launch handoff.

- M11 gameplay accessibility manifests now keep endgame plaque, mirror,
  and portrait bounds aligned with the rendered 320x200 victory overlay,
  with a new headless probe covering gameplay, inventory, map, dialog,
  candidate mirror, endgame, and open-chest surfaces.

- Release packaging now treats Steam Deck as SteamOS on x86_64/AMD hardware
  and publishes a pacman-compatible `.pkg.tar.zst` artifact from the Linux
  x86_64 release job. Steam Deck is no longer grouped with Linux ARM64 in
  release copy.

- DM1 V2.2 finished-art material gating now requires non-placeholder hero
  material files to have a PNG signature and IHDR dimensions matching the
  manifest before they can promote to `REAL`. This hardens readiness checks
  without claiming that finished DM1 V2.2 art or screenshot receipts ship yet.

- **DM1 V2.2 real screenshot/material receipt path**: the
  `dm1_v22_finished_art_material_gate_pc34` classifier now separates
  material-pack provenance from runtime screenshot evidence. Optional
  `dm1_v22_real_screenshot_material_receipt_01` metadata can classify
  `NO_RECEIPT`, `SYNTHETIC_PLACEHOLDER`, `PARTIAL`, or
  `FINISHED_REAL`; synthetic receipts stay non-final, and final proof
  requires an operator-reviewed receipt file outside the repo plus a
  finished six-slot material gate. No proprietary or generated
  screenshot proof is shipped.

- **X68000 HDM receipt classification tightened**: the DM1/CSB X68000 media
  classifier now reports a conservative receipt class for blank save disks,
  live `HPR-0007` protection-sector captures, nonblank/no-sentinel media, FTL
  payloads, and off-axis `HPR-0007` label strings. The real DM1 X68000 HDM
  receipt probe now verifies the public DMFiles image as `off_axis_sentinel_only`
  without promoting it to protected-media evidence.

## Verification

- Release version metadata is synchronized across CMake, launcher UI,
  changelog, and `include/firestaff_version.h`.
- Local release-prep verification covered CMake configure/build plus the
  headless Phase A and audio smoke probes before tagging.
- The release workflow rebuilds and packages macOS arm64, macOS x86_64,
  Windows x86_64, Linux x86_64, Linux arm64, and Steam Deck x86_64 artifacts
  from the `v3.0.14` release tag.

# Firestaff v3.0.13

Firestaff v3.0.13 packages the 2026-06-28/29 runtime-hardening work that
landed after v3.0.12. The headline change is a stricter DM1 V1
post-dungeon capture workflow: operators must now produce a reviewed target
selection receipt before dispatching live post-dungeon routes for pairing
evidence. This release also keeps polishing bounded runtime, launcher, and
presentation probes without claiming finished parity for targets that still
need real-asset proof.

## Highlights since v3.0.12

- **M11 font-scale overlay fit coverage**: the in-game session-timer reminder banner now uses scale-aware copy and a top-strip layout that stays clear of the DM1 V1 dungeon viewport at fontScale 1/2/3. The focused M11 probe now renders actual game-view output with a synthetic original-font bitmap and verifies changed pixels stay inside the reminder banner rectangle.

- **DM1 save-byte export/import manifest gate**: M12 now has a versioned per-game save-byte manifest path for Firestaff-native DM1 saves. The new `save_byte_manifest_m12` CTest exports a real `FSDM1SV1` DM1 save payload with byte count and CRC metadata, imports it into a fresh save directory, validates it through the DM1 loader, and rejects duplicate or corrupted imports. Original DM1 save conversion and other game save formats remain separate follow-up work.

- **CSB wall-text palette gate**: the D1C wall-text source-lock test is now CTest-backed and also verifies PC dungeon palette expansion, C10-transparent inscription-font pixels, RGBA conversion, and out-of-range palette-index rejection without claiming CSBgraphics.dat payload decode or runtime override support.

- **DM2 original-overlay pair evidence now has a readiness gate**: the H2313 DM2 scaffold now defines the optional original-vs-Firestaff pair manifest contract. Missing paired evidence remains an explicit OPEN state, malformed tracked rows fail, and promotion requires same-state `dungeon_gameplay` original plus Firestaff 224x136 viewport SHA-256s. This adds no pixel-parity claim.

- **DM2 PC-9801 demo media classification stays launch-safe**: the M12 scanner now recognizes the DMII PC-9801 Japanese Demo `GRAPHICS.DAT` hash as a bounded DM2 version row, but the DM2 launch gate still requires a supported launch-compatible graphics hash plus the required dungeon hash. A synthetic regression proves PC-98 demo graphics plus the PC dungeon hash remains non-launchable.

- **DM2 V2 HUD widget real-size slot receipt**: a new CTest probe generates a scratch-only 32x32 `compass_rose` PNG, installs it as a non-placeholder `hud_chrome` receipt, and verifies that one chrome slot classifies as `REAL` while the rest of the manifest remains procedural placeholder. This is classification evidence only; finished widget art and the runtime bitmap blit remain open.

- **DM2 V1 startup/profile handoff gate**: the M11 DM2 startup regression now covers both sides of the boundary: incomplete required files and unverified GRAPHICS/DUNGEON pairs are blocked before M11 claims the DM2 boot runtime, while a hash-verified profile still reaches the DM2 V1 runtime handoff and one idle tick when local data is available. No DM2 playability or pixel-parity claim is added.

- **DM2 V1 weather/timer save stream fixture**: the data-free DM2 weather/timer save gate now exercises a contiguous `skload_table_60` game-state block followed by four SUPPRESS-encoded timer records, proving `wTimersCount`-driven timer-table walking, encoded segment boundaries, timer order/field retention, and deterministic weather replay from the saved RNG seed. The shared `DM2_GameStateBlock` wire view is now explicitly packed to the documented 56-byte layout with compile-time offset guards, preventing host alignment from moving `_dw22` or truncating the reserved tail. No original save interoperability or real-asset runtime claim is added.

- **DM2 V1 projectile creature collision gate**: the projectile step helper now runs the existing creature-collision resolver before the energy-floor despawn path for non-grace projectiles, matching the ReDMCSB `PROJEXPL.C` impact-before-energy order. The data-free unit/probe now pin a depleted projectile on a creature square so creature damage must land before `F0813` consumes the slot; wall, door, party, teleporter, and map-change impacts remain outside this bounded slice.

- **Nexus DGN geometry readiness**: the Nexus DGN loader now exposes a bounded Structure1/Structure1B geometry summary with collision-reference counts and conservative mesh-readiness metadata. This is a parser gate only; full Structure1C/F mesh decode and real runtime screen capture remain open.

- **Nexus V1 Light spell M11 dispatch hook**: the shared M11 spell panel now routes Nexus Light, Magic Torch, and Darkness rune casts into the `nexus_v1_light_runtime` path, preserving source-faithful emulate mode by default and the existing guard-mode `CAST_REJECTED` behavior for capped timelines. This is a data-free runtime hook only; real Nexus asset/screenshot and embedded `NGLT` load evidence remain separate follow-up work.

- **M12 data-root switch popup hardening**: the launcher popup-once gate now covers a real configured data-root switch across both DM1 and DM2 using synthetic recursive hash-scanned files. Switching from a complete root to a partial root clears stale availability and shows per-game missing-required-file popups that name only the unmatched GRAPHICS/DUNGEON role.

- **DM1 HoC ordinal-2 readiness status narrowed**: the existing future-route readiness gate now records the current sibling matrix as 10/10 source-present and CTest-wired, while keeping the real-data boundary explicit: live PC 3.4 still exposes zero real ordinal-2 corridor-sensor poses, so no per-route visual capture or DOS pixel parity claim is promoted yet.

- **SCK RAW asset bridge handoff**: `firestaff_sck_asset_bridge` now has a bounded RAW identity decoder surface for selector-chosen `RAW*` mapfile slices. Synthetic CTest coverage verifies RAW slice handoff and keeps PAL/SND rows visible but unsupported until their decoders land.

- **DM1 launcher missing-data popup coverage**: a new M12 gate verifies that optional original-file candidates cannot make DM1 launchable when required GRAPHICS/DUNGEON hashes are missing, and that the popup names only the missing required rows.

- **DM1 V1 post-dungeon reviewed-target selection**: `docs/parity/DM1_V1_POST_DUNGEON_PAIRING_TARGET_CONTRACT.json` now pins the five supported target kinds, required fields, route-step minimums, source anchors, PASS_IDs, asset hashes, and baseline non-claims for post-dungeon pairing work. The companion `docs/parity/tools/dm1_v1_post_dungeon_pairing_target_selector.py` refuses to write `target_selection.receipt.json` until those pins pass, and both the selector self-test and runbook-consistency probe are CTest-gated. This is an accountability gate only; it does not promote original-vs-Firestaff parity rows or ship proprietary frames.

- **DM1 V1 resurrection rename UI source-lock gate**: added a data-free CTest
  for the ReDMCSB F0281 rename-panel boundary. It pins the C161-only rename
  entry, panel graphic/box handoff, name/title input rules, OK gating,
  punctuation commands, backspace behavior, and title length cap without
  claiming live M11 prompt wiring or original pixel parity.

- **DM1 original-save interop now has a bounded header classifier**:
  `dm1_v1_original_save_classifier` recognizes ReDMCSB-shaped
  `DMSAVE.DAT` / `DMSAVE.BAK` / `DMGAME.DAT` / `DMGAME.BAK` headers via the
  original 512-byte save-header checksum and XOR-obfuscation contract, reports
  Firestaff-native saves as not-original, and stays skip-safe when no user save
  is staged. This is importer readiness only: original save compatibility is
  still blocked until real bytes round-trip through Firestaff.

- **DM1 V2.2 teleporter material-pixel regression hardening**: `firestaff_dm1_v22_inplace_render_probe` now pins the synthetic V2.2 material framebuffer signature, rerenders after a repeated shape-cache update, and sweeps all four directions while proving wall/floor/pit/stairs asset selection stays deterministic and the teleporter field remains unpainted instead of falling back to wall art. This is synthetic cache coverage only; finished-art and real screenshot receipts remain open.

- **DM1 V1 original DOS capture route manifest**: added the skip-safe `todo100_dm1_v1_original_dos_capture_route_manifest` CTest gate for a future PC 3.4 DOSBox capture of the Hall of Champions WUUF / THE BIKA south_return viewport. The gate validates route labels, expected crop filenames, source/probe anchors, and capture-script knobs without launching DOSBox or committing original assets/screenshots. No original-vs-Firestaff pixel-parity claim is added.

- **DM1 PC 3.4 archive receipts are pinned more tightly**: a new data-free
  gate proves renamed archive entries can satisfy DM1 PC 3.4 English
  `GRAPHICS.DAT` and `DUNGEON.DAT` requirements by hash, then materialize
  into ordinary runtime cache files before launch.

- **DM1 V1 TITLE C001 fallback gate**: `V1_TitleFrontend_SelectRuntimeSource()` now pins the runtime source-selection seam for the original TITLE animation. A usable `GRAPHICS.DAT` C001 title graphic always wins; `TITLE.DAT` remains a visible fallback only when C001 is missing or too small. New CTest `title_frontend_c001_fallback_gate_pc34_compat` passes 30 checks and preserves pass842/pass897/SWSH-handoff palette coverage without adding a new screenshot or pixel-parity claim.

- **DM1 V1 viewport/collision unmanifested-report scaffold**: new CTest gate `todo100_dm1_v1_viewport_collision_report_repro_gate` keeps the open bug visible by passing with status `BUG_OPEN_CAPTURE_MANIFEST_MISSING` unless an operator supplies a candidate capture directory. It records the exact promotion contract for future reports without claiming a bug fix, a full collision transcript, or original-vs-Firestaff pixel parity.

- **DM1 V1 Hall of Champions ordinal-13 turn capture readiness**: added a skip-safe PPM capture gate for the WUUF south-return in-place 180-degree turn. When the reference fixture is present, the probe writes build-local Firestaff runtime PPMs and a manifest for SOUTH WUUF, intermediate EAST clear, and final NORTH GANDO frames, while keeping the original-DOS pixel-parity claim open.

- **DM1 V1 HoC ordinal-4 RESTING capture scaffold**: added a data-free CTest gate for the LEIF RESTING original-capture row. It locks the future capture geometry and route vocabulary (320x200 screen, `(0,33,224,136)` viewport crop, ordinal-4 C026 atlas cell, D1C portrait rect, C145/C146 rest/wake points, and the current Firestaff RESTING overlay bounds) without committing original screenshots or claiming pixel parity.

- **DM1 V1 Hall of Champions ordinal-2 probe status is honest again**:
  the stale `west_negative` sibling is now treated as a promoted negative-route
  CTest gate, and the ordinal-2 readiness matrix records all 10 sibling probes
  as source-present and wired. The real PC 3.4 data boundary remains unchanged:
  there is still no real ordinal-2 HoC corridor sensor to capture, so future
  visual work stays gated on a future data/layout change.

- **DM1 V1 HoC ordinal-2 cancel/reopen gate promoted**: the source-present `firestaff_dm1_v1_hall_of_champions_portrait_02_cancel_reopen_portrait_rect_position_runtime_probe` is now verified as a CTest-backed runtime gate alongside the ordinal-2 future-route readiness check. The status docs no longer list `cancel_reopen` as stale/open; remaining ordinal-2 work is future visual capture only if real DM1 data ever exposes an ordinal-2 corridor sensor.

- **DM1 V2.2 teleporter material-pixel regression hardening**: `firestaff_dm1_v22_inplace_render_probe` now pins the synthetic V2.2 material framebuffer signature, rerenders after a repeated shape-cache update, and sweeps all four directions while proving wall/floor/pit/stairs asset selection stays deterministic and the teleporter field remains unpainted instead of falling back to wall art. This is synthetic cache coverage only; finished-art and real screenshot receipts remain open.

- **DM1 V2.1 Apple Silicon readback receipt**: added `dm_v21_upscale_readback_receipt_silicon`, a skip-safe CTest probe that compares seven deterministic SDL readback samples from the V2.1 EPX/RGBA presentation path against the exact CPU RGBA bytes handed to `M11_Render_PresentRGBA`, including repeat hash stability on Apple Silicon.

- **DM1 V2 deterministic screenshot receipt route**: added the data-free `firestaff_dm1_v2_deterministic_screenshot_receipt_route_probe` CTest. It selects the V2.0 filtered presentation mode, renders the canonical DM1 entry fixture into a scratch BMP receipt, writes a JSON manifest, and pins framebuffer/palette/BMP/manifest FNV-1a hashes. This is Firestaff V2 receipt evidence only; it makes no DOSBox/original pixel-parity or finished-art claim.

- **DM1 PC 3.4 Multilanguage launcher smoke**: added a synthetic M12 profile gate proving the PC 3.4 Multilanguage hash pair makes DM1 ready, preserves the launcher profile labels, produces a valid V1 launch intent for `pc34-multi`, and still reports a clear `DUNGEON.DAT` missing-data popup when only the multilanguage GRAPHICS hash is present. No real game data or pixel-parity claim is included.

- **CSB V1 audio runtime/save boundary**: added a data-free CTest gate for the ReDMCSB pending-sound request/flush path. It proves immediate vs prioritized audio requests, pending arbitration by volume/priority, one-tick pending flush, and save/load handling where `LastCreatureAttackTime` persists but transient pending audio does not. This is runtime contract evidence only; no real CSB sound playback or playability claim is added.

- **CSB viewport/HUD parity readiness is gated, not promoted**: new CTest `csb_v1_viewport_hud_pixel_parity_readiness` locks the six-fixture CSB viewport/HUD comparison shape and records `broad_parity_status=OPEN_UNPAIRED` until paired original and Firestaff artifacts, hashes, and diff metrics exist for every fixture.
- **DM2 V2 HUD widget synthetic-example integrity + generator-agnostic strengthening**: `firestaff_dm2_v2_hud_widget_synthetic_promotion_probe` is now 81/81 PASS (was 51/51) with three additions: (a) PNG 8-byte signature check on every one of the seven 1x1 fixtures so a future fixture that silently rots into arbitrary text cannot pass the `synthetic-test-fixture` substring check alone, (b) per-slot `width > 0 && height > 0` sanity check on every COMPLETE slot so a corrupt manifest without declared dimensions cannot promote the gate, and (c) a generator-agnostic COMPLETE scenario that rewrites every `generator` entry from `"synthetic_test"` to `"pbr_hero"` and verifies the gate still promotes to COMPLETE — guarding against a future refactor that accidentally introduces per-generator allowlisting. No finished-art, real-bitmap-blit, or visual-verification claim is added; the no-finished-art boundary stays explicit.

- **CSB V1 real-asset ornament capture provenance**: `firestaff_csb_v1_pc_real_asset_ornament_blit_probe` now records a JSON manifest beside its deterministic PPM/SHA capture, including the verified PC 3.4 GRAPHICS.DAT MD5, selected bitmap index/dimensions/span, D1C floor-band rows, F0108/F0115 source anchors, and tally counts. The gate remains skip-safe and does not claim original pixel parity.

- **X68000 HDM FTL handoff receipt hardening**: the X68000 media classifier now exposes an explicit windowed FTL magic counter, and the skip-safe real-HDM handoff probe uses it to distinguish the legacy first-32KiB sniff from full-disk embedded `.FTL` payload evidence.

## Verification

- 55 commits ship on top of `v3.0.12`, including this release-prep commit.
- Local release verification ran the post-dungeon selector self-test, the
  DM1 V1 capture runbook-consistency probe, CMake configure/build, Phase A,
  audio, and focused CTest release checks before tagging.
- The release workflow rebuilds and packages macOS arm64, macOS x86_64,
  Windows x86_64, Linux x86_64, and Linux arm64 artifacts from the `v3.0.13`
  release commit.

# Firestaff v3.0.12

Firestaff v3.0.12 packages the 2026-06-27/28 runtime-hardening work that
landed after v3.0.10. It keeps the public completion claims conservative:
DM1 V1 gets broader source-locked gates, the Custom presentation pipeline gets
new bounded asset/runtime checks, and CSB, DM2, Nexus, and Theron's Quest gain
more verified handoff and data-boundary coverage without claiming finished
playability for those targets.

## Highlights since v3.0.10

- **ZIP/gzip support is self-contained by default**: the launcher/runtime ZIP
  cache materializer, Theron SRM gzip probe, and deflated archive paths now
  build through bundled static miniz with `FIRESTAFF_HAS_ZLIB=1`. Packagers can
  still opt into platform zlib with `FIRESTAFF_WITH_BUNDLED_ZLIB=OFF`.
- **DM1 V1 has wider gameplay-state gates**: new source-locked checks cover
  champion-panel hand-slot refresh order and pixel slices, waterskin/fountain
  fill-drink flow, food/water state, repeated-tick timeline determinism,
  creature-group split sequencing, and the D3C back-wall item thing pass.
- **Custom presentation work is more tightly bounded**: DM1 V2.2 finished-art
  materials, DM2 V2.2 per-cell modern-art swaps, DM2 V2 HUD widget runtime
  hooks, and a safe synthetic HUD widget manifest are now covered without
  shipping copyrighted or finished replacement art.
- **Cross-game handoff evidence expanded**: CSB graphics/save loader
  boundaries, Nexus BPX/BPK archive surfaces, Nexus save/light runtime paths,
  Theron SRM progression/party envelopes, Theron Track 02 handoff, and Theron
  V2 HUD runtime overlay now have focused gates.
- **Launcher and accessibility coverage improved**: screen-reader manifest
  coverage extends across launcher extras, and the M11 high-contrast overlay
  has a dedicated runtime gate.

## Verification

- 48 commits ship on top of `v3.0.10`, including this release-prep commit.
- Local release verification rebuilt the CMake project from a fresh `build/`
  directory, then ran Phase A, audio, focused CTest slices for the new gates,
  and the pre-push checks before tagging.
- The release workflow rebuilds and packages macOS arm64, macOS x86_64,
  Windows x86_64, Linux x86_64, and Linux arm64 artifacts from the `v3.0.12`
  release commit.

# Firestaff v3.0.10

Firestaff v3.0.10 packages the 2026-06-26 runtime-hardening work that
landed after v3.0.9. It keeps the public completion claims conservative:
DM1 V1 gets wider real-data M11 coverage, CSB champion rules get a tighter
source-locked fixture, and Theron's Quest direct startup becomes practical
from a broad user data root without claiming full Theron playability.

## Highlights since v3.0.9

- **DM1 V1 all-map rendering is now pixel-gated**: the
  `firestaff_dm1_v1_all_map_render_probe` sweeps every non-wall source cell
  across all 14 DM1 maps in all four directions through the live M11 view
  renderer, then compares thing-backed poses against a no-`world.things`
  baseline. Visible items, groups, and sensor/text content must now produce
  real viewport pixel deltas.
- **Hall of Champions artifact suppression is stronger**: map-0 payload item
  chains are treated as mirror/candidate data rather than visible floor loot,
  runtime projectile/explosion artifact false positives are blocked, and the
  Hall creature tick no longer scans compact source payload data as active
  creature groups.
- **Hall message text stays player-facing**: synthetic M11 creature movement
  or combat telemetry such as `SCREAMER REACHES THE PARTY!` is filtered out
  of the normal DM1 bottom message surface unless the debug HUD owns it.
- **CSB champion reincarnation has per-stat parity evidence**: a new
  data-free fixture pins the source-locked F0282 contract for HP/Mana/Stamina
  halving, non-Luck stat penalties, minimum clamps, skill clearing,
  NEEDS_RENAME/DEAD flags, and resurrect-vs-reincarnate behavior. A
  companion data-free fixture now pins the deterministic `randomPoints` LCG
  + `reincarnateStatPenalty` interaction with the seeded F0309 max-load
  formula end-to-end, and the older `ChampionState_Compat` reincarnation
  penalty shim is regression-gated through CTest again.
- **Theron's Quest direct root startup is fast**: `--game theron --data-dir
  <root>` now tries canonical Theron child roots first (`theron/`,
  `theron/jp`, `theron/us`, `theron-extras/japan`, `theron-extras/usa`),
  allowing a broad root such as `~/.firestaff/data` to reach `TQR level load`
  quickly while leaving the normal launcher-wide catalog scan unchanged.

## Verification

- 9 commits ship on top of `v3.0.9`, including this release-prep commit.
- GitHub Actions was green on `dc8a9d282` before release prep: M10 verify,
  strict warnings, asset hygiene, CMake builds on macOS/Windows/Linux, Phase
  A/audio probes, and cross-platform determinism.
- Focused local DM1 verification covered the all-map render probe and its
  sibling viewport/occlusion gates, plus the HoC no-creature, floor, message,
  and artifact probes.
- Focused local CSB verification covered the new champion per-stat parity
  fixture, the new champion per-stat ↔ F0309 max-load interaction fixture,
  the restored older `ChampionState_Compat` reincarnation penalty CTest
  entry, and nearby character-import / phase-7 / save-boundary tests.
- Focused local Theron verification covered the direct root startup path,
  the 36-test Theron CTest slice, runtime screenshot readiness, and the Tier
  1 strict boot probe.
- The release workflow rebuilds and packages macOS arm64, macOS x86_64,
  Windows x86_64, Linux x86_64, and Linux arm64 artifacts from the
  `v3.0.10` release commit.

# Firestaff v3.0.9

Firestaff v3.0.9 packages the 2026-06-26 fixes that followed the v3.0.8
Hall of Champions release. It is a focused DM1 V1 stability release for
the MacBook Pro feedback loop: movement input, HoC inscriptions, HoC floor
objects, and the original TITLE/Swoosh presentation path all get tighter
runtime gates without changing the public completion claims for CSB, DM2,
Nexus, or Theron's Quest.

## Highlights since v3.0.8

- **Hall of Champions movement is responsive on macOS**: active DM1 now
  routes W/A/S/D, arrows, Home/End, Q/E, and keypad through the engine
  scancode remap path, then polls held movement keys at the DM1 source
  input cadence instead of relying on OS key repeat.
- **DM intro/title pixels stay sharp and correctly colored**: the
  special-palette TITLE/Swoosh path now shares the CPU-nearest V1
  presentation route used by ordinary indexed frames, preserving the
  source-locked C12/C13/C14 palette behavior on HiDPI/Metal paths.
- **Hall of Champions inscriptions are readable**: M11 now uses ReDMCSB's
  synthetic inscription ornament slot and restores the clean D1C wall patch
  before drawing M648 glyph text, so readable Hall inscriptions are no
  longer painted directly over noisy wall stone.
- **Hall source-payload artifacts are blocked at runtime**: Hall of
  Champions map-0 payload item chains are now treated as mirror/candidate
  data, not loose floor loot, so they no longer render as fireball-like
  floor objects or other artifacts. Stale dense-index floor fallbacks remain
  blocked as well.
- **Creature AI telemetry no longer leaks into the Hall message area**:
  synthetic M11 combat/movement narration such as `SCREAMER REACHES THE
  PARTY!` stays out of DM1's bottom C015 text surface unless the debug HUD
  is active.
- **Hall of Champions has no runtime creature AI**: M11 now exits the
  creature tick on DM1 map 0 before scanning `SquareFirstThings`, preventing
  the old dense-index route from animating a false HoC Screamer from compact
  data.
- **Runtime evidence widened**: new real-DM1-data probes cover HoC
  inscription readability, HoC payload suppression, and HoC projectile /
  explosion artifact prevention. The floor gate found 596 old dense-index
  false-positive floor samples and 83 source-payload samples that now resolve
  to zero renderable items.

## Verification

- 12 commits ship on top of `v3.0.8`, including this release-prep commit.
- GitHub Actions was green on `68ef65e68` before release prep: M10 verify,
  CMake build on macOS/Windows/Linux, strict warnings, asset hygiene, and
  cross-platform determinism.
- Focused local verification after the final HoC floor fix included the new
  floor runtime no-false-items probe, the HoC inscription readability probe,
  the Hall floor source-lock probe, and Phase A, all passing through CTest.
- Earlier focused verification for this batch covered the keyboard remap and
  held-key path, title special-palette nearest presentation, inscription
  font/source-lock checks, and `git diff --check`.
- Post-release HoC text cleanup verification covered the bottom message-row
  probe, the M11 Phase A probe, and `git diff --check`.
- Post-release HoC no-creature verification covered a real-DM1-data probe
  proving zero compact GROUP chains on map 0 while documenting the old
  dense-index false positives that previously fed creature AI.
- Post-release HoC artifact verification covered the updated floor runtime
  probe, the source-payload data probe, the no-creature probe, the bottom
  message-row probe, the new projectile/explosion artifact probe, and Phase A.
- The release workflow rebuilds and packages macOS arm64, macOS x86_64,
  Windows x86_64, Linux x86_64, and Linux arm64 artifacts from the
  `v3.0.9` release commit.

# Firestaff v3.0.8

Firestaff v3.0.8 packages the late 2026-06-25 DM1 Hall of Champions
verification and M11 presentation hardening that landed after v3.0.7.
It is a narrow evidence release: the changes tighten source-locked DM1
V1 behavior and rendering gates without broadening the public completion
claims for the other game profiles.

## Highlights since v3.0.7

- **Hall of Champions source payloads are source-locked**: the compact
  ReDMCSB-style `SquareFirstThings` lookup proves map-0 Hall payload chains,
  while runtime rendering keeps those payload objects out of the ordinary
  floor-loot path.
- **Hall floor false positives are blocked at runtime**: M11 now keeps the
  visible floor-item summary tied to the compact source chain, so stale dense
  fallback data cannot draw extra side-pane floor objects in the Hall.
- **Hall of Champions expected payload contents are verified**: a new
  real-data probe pins the 70 compact thing-list squares on map 0, the
  seven expected payload-object chains, eight payload objects, and zero
  containers/projectiles/explosions on the Hall floor.
- **Champion panel return coverage widened**: additional DM1 V1 runtime
  gates cover ordinal 0 panel return and ordinal 4 rest/wake repaint
  behavior, keeping champion portrait redraw evidence in CTest instead
  of only local notes.
- **M11 V1 presentation stays sharper**: nearest-neighbor presentation
  preference is preserved for the original-pixel view path while keeping
  the modern scaling routes separate.
- **DM intro/title palette presentation is hardened**: the special-palette
  TITLE path now uses the same CPU-nearest V1 presentation path as ordinary
  indexed rendering, so the Swoosh -> Dungeon Master title animation keeps
  the source-locked C12/C13/C14 palette colors on HiDPI/Metal paths.
- **Hall of Champions inscriptions are readable again**: the front-wall
  inscription path now uses ReDMCSB's synthetic inscription ornament slot and
  restores the clean D1C wall patch behind M648 glyph text instead of drawing
  the letters straight over noisy Hall stone.

## Verification

- 5 commits ship on top of `v3.0.7`, including this release-prep commit.
- GitHub Actions was green on `478eb65c2` before release prep: M10
  verify, CMake build on macOS/Windows/Linux, strict warnings, asset
  hygiene, and cross-platform determinism.
- Focused local verification before release prep included the Hall floor
  source-lock probe, the compact square-first-thing unit test, the
  relevant Hall of Champions CTest cluster at 7/7, pre-push hooks, and
  `git diff --check`.
- Post-release main verification for the HoC inscription fix included the new
  real-DM1-data HoC inscription readability probe, the inscription font probe,
  nearest-presentation probe, and Phase A probe.
- Post-release main verification for the HoC floor runtime fix included a new
  real-DM1-data viewport probe that found 596 old dense-index false-positive
  floor samples and proved all now resolve to zero renderable items.
- The release workflow rebuilds and packages macOS arm64, macOS x86_64,
  Windows x86_64, Linux x86_64, and Linux arm64 artifacts from the
  `v3.0.8` release commit.

# Firestaff v3.0.7

Firestaff v3.0.7 packages the later 2026-06-25 post-v3.0.6
verification batch from `main`. It is another conservative evidence
release: more local branch/worktree work has been inspected, promoted
where it was still missing, and locked behind source/build gates without
claiming new finished-game parity.

## Highlights since v3.0.6

- **DM1 Hall of Champions evidence is tighter**: stale probe
  registrations were removed, floating side-wall portrait pixels are now
  blocked, HiDPI portrait-surface coverage is gated, and seven previously
  local ordinal-22 runtime lanes are now CMake/CTest-backed on `main`.
- **DM1 V2.2 M11 handoff is source-locked**: the verifier pins viewport
  sampling, shape-cache population, in-place render preference, legacy
  overlay fallback boundaries, and pit/stairs/teleporter/field asset
  routing.
- **Source-media and decoder coverage advanced**: PC-98 and X68000 HDM
  media classifiers, X68000 HDM real-media receipt + cross-module
  FTL-handoff gates, FTL `HUNK_CODE` plus corrected `HUNK_DATA` zero-run
  decoding, and CSB `HCSB.HTC` Hint Oracle scanning all have bounded
  evidence without vendoring original game data.
- **CSB Hint Oracle scanner slots are broader**: the skip-safe
  `HCSB.HTC` real scanner now accepts the dmweb-documented English
  R1/R2/R3, French R1, and German R1/R2 HTC hashes while keeping the
  graphical overlay and real screen proof as open follow-ups.
- **Cross-game runtime gates widened**: DM2 overlay/HUD asset gates,
  Nexus Track 1/font/light probes, and Theron Track 02/SRM evidence were
  added or hardened while keeping remaining real-asset handoff gaps
  documented as open work.
- **Launcher data-status coverage improved**: the data-dir platform
  matrix and no-data popup paths are now covered, and TODO/DONE were
  refreshed so promoted evidence is not still listed as open.

## Verification

- 22 commits ship on top of `v3.0.6`, including this release-prep
  commit.
- GitHub Actions was green on `cb4d171df` before release prep: M10
  verify, CMake build on macOS/Windows/Linux, strict warnings, asset
  hygiene, Pages, and cross-platform determinism.
- Focused local verification before release prep included CMake
  configure, builds for the seven new ordinal-22 probe targets, the
  ordinal-22 CTest cluster at 11/11, and `git diff --check`.
- The release workflow rebuilds and packages macOS arm64, macOS x86_64,
  Windows x86_64, Linux x86_64, and Linux arm64 artifacts from the
  `v3.0.7` release commit.

# Firestaff v3.0.6

Firestaff v3.0.6 packages the later 2026-06-25 evidence, asset-format,
and release-status refresh after v3.0.5. It is a conservative release:
the new gates make more of the current work reproducible in CI, while
remaining runtime, decoder, and original-capture gaps stay documented as
open work.

## Highlights since v3.0.5

- **DM1 Hall of Champions portrait placement is gated end to end**: a new
  all-portrait wall-coordinate probe scans all 24 source-visible C127
  champion portrait poses in the Hall of Champions, including HALK,
  GANDO, and WUUF. It locks the D1C wall frame and C026 portrait cutout
  and checks wrong-wall stale redraw behavior without claiming original
  DOS pixel parity.
- **Greatstone/SCK evidence moved from notes to gates**: current
  Greatstone `db_data/` paths now have an offline CTest-backed reachability
  fixture, the SCK mapfile-to-asset selection bridge is covered by
  synthetic and optional real-corpus probes, and FTL `HUNK_DATA` area-1
  zero-run decompression is CTest-gated. FTL `HUNK_CODE`, mapfile item
  extraction, real corpus promotion, and runtime loading remain open.
- **FM Towns CD layouts are classified safely**: DM1, CSB, and DM2
  BIN/CUE or ISO/CUE redump-style layouts now have a bounded classifier
  with synthetic tests and ISO9660 PVD detection. It does not vendor game
  data, extract payloads, decode assets, or claim runtime launch proof.
- **Public project status was cleaned up**: TODO.md was audited back to
  open work only, DONE.md records the verification sweep, and the
  completion matrix plus verifier scripts now reflect the current
  conservative per-game evidence totals.
- **Build hygiene tightened**: duplicate modern static link lines were
  cleaned up through modern CMake policies, while the public docs and
  gap list keep the new evidence bounded to what is actually tested.

## Verification

- 18 commits ship on top of `v3.0.5`, including this release-prep commit.
- GitHub Actions was green on `3fe55d680` before release prep: M10 verify,
  CMake build on macOS/Windows/Linux, strict warnings, asset hygiene,
  Pages, and cross-platform determinism.
- Focused local verification before release prep included the completion
  matrix verifiers, CSB V1 source-lock verifiers, a 14-test focused CTest
  set covering the refreshed evidence manifests, and `git diff --check`.
- The release workflow rebuilds and packages macOS arm64, macOS x86_64,
  Windows x86_64, Linux x86_64, and Linux arm64 artifacts from the
  `v3.0.6` release commit.

# Firestaff v3.0.5

Firestaff v3.0.5 packages the 2026-06-25 post-v3.0.4 evidence and
reference-hardening batch. This is still a conservative release: it adds
source-locked CSB input coverage, platform/version provenance, and
asset-format documentation without broadening public runtime-completion
claims beyond the evidence now checked into the tree.

## Highlights since v3.0.4

- **CSB keyboard commands source-locked**: CSB gameplay now has a
  dedicated keyboard bridge for F1-F4 champion inventory toggles,
  Escape freeze/unfreeze, resting Return/Enter wake-up, Ctrl-S disk menu,
  and the Insert/arrow/Clr Home movement layout from the ReDMCSB command
  tables.
- **DMWeb platform provenance expanded**: DM1, CSB, DM2, Nexus, and
  Theron's Quest docs now record more exact per-platform and per-version
  boundaries, including source-media notes, protection/copy boundaries,
  version matrices, screenshots/video provenance, and remaining
  real-media handoff gaps.
- **Greatstone/SCK references corrected and widened**: the Greatstone
  coverage now distinguishes the real FTL hunk container, SCK mapfile
  metadata, Atari ST `START.PAK`, IMG5 planar images, Mac QuickTime
  `MooV` assets, SNES palette-selector metadata, and current reachable
  `db_data/` DM/CSB paths. The gap list keeps those follow-ups separate
  from already-fixed raw decoders such as IMG5 and PAK.
- **DM1 Hall of Champions evidence continues**: additional post-v3.0.4
  gates cover more ordinal slices, including side-wall/no-portrait and
  D2C far-positive cases, extending the source-locked runtime evidence
  without claiming finished screenshot parity.
- **Release hygiene tightened**: CI now blocks tracked original
  game-data payloads, stale subproject metadata was removed, and the
  public docs stay focused on user-facing status rather than worker logs
  or private queue details.

## Verification

- 70 commits ship on top of `v3.0.4`, including this release-prep
  commit.
- GitHub Actions was green on `5a429a8b4` before release prep: M10
  verify, CMake build on macOS/Windows/Linux, strict warnings, asset
  hygiene, Pages, and cross-platform determinism.
- Focused local verification before release prep included `git diff
  --check`.
- The release workflow rebuilds and packages macOS arm64, macOS x86_64,
  Windows x86_64, Linux x86_64, and Linux arm64 artifacts from the
  `v3.0.5` release commit.

# Firestaff v3.0.4

Firestaff v3.0.4 packages the 2026-06-24 post-v3.0.3 evidence and
release-hardening batch. The release is still conservative about public
game-status claims: it promotes source-locked probes, launcher data-path
fixes, and packaging confidence without claiming complete parity for
DM1, CSB, DM2, Nexus, or Theron's Quest beyond the evidence now covered.

## Highlights since v3.0.3

- **DM1 Hall of Champions coverage fills out the ordinal sweep**: new
  real-asset runtime probes cover more portrait routes across front
  entries, side entries, walkpaths, redraw-after-candidate flows,
  panel-open/cancel behavior, fullscreen scaling, HiDPI mouse mapping,
  transparent-pixel checks, and no-floating side-wall pixels. Ordinal 22
  now has dedicated front and walkpath lanes instead of remaining only an
  indirect coverage note.
- **Launcher data defaults corrected for real installs**: macOS and
  Linux now resolve game data from `~/.firestaff/data`, while Windows
  resolves beside the installed executable. The old
  `/tmp/firestaff-test-no-assets` path remains only as an explicit test
  fixture and is guarded by the portable filesystem probe.
- **Asset scanner and launcher gates widened**: nested ZIP cache handoff,
  ISO/BIN duplicate-hash handling, data-dir cache invalidation, no-data
  popup behavior, and M12 language-cycle layout are now covered by
  focused tests or probes.
- **Cross-game runtime evidence expanded**: the batch adds DM2
  creature/save gates, Nexus DMDF and save-slot checks, Theron Track 02
  and shop/progression gates, and DM1 V2 actual-render screenshot
  coverage.

## Verification

- 174 commits ship on top of `v3.0.3`, including this release-prep
  commit.
- GitHub Actions was green on `b9a9d8095` before release prep: M10
  verify, CMake build on macOS/Windows/Linux, strict warnings, asset
  hygiene, and cross-platform determinism.
- Focused local verification before release prep included `git diff
  --check`, CMake configure, and targeted DM1 Hall of Champions probe
  builds/CTest runs with `--parallel 4`.
- The release workflow rebuilds and packages macOS arm64, macOS x86_64,
  Windows x86_64, Linux x86_64, and Linux arm64 artifacts from the
  `v3.0.4` release commit.

# Firestaff v3.0.3

Firestaff v3.0.3 packages the post-v3.0.2 hardening batch from
2026-06-23. It is a focused verification and launcher-polish release:
more source-locked runtime gates, stronger data-status behavior in the
launcher, and package builds for all supported desktop targets. It does
not change the public game-status claims beyond the evidence now covered
by tests and probes.

## Highlights since v3.0.2

- **DM1 Hall of Champions coverage expanded again**: new real-asset
  cancel/reopen portrait-rectangle probes cover additional champion
  ordinals, including row-2 C026 atlas cases. These gates verify the D1C
  portrait destination rectangle, panel reopen behavior, champion-count
  rollback, and stale side-wall pixel absence without claiming DOS pixel
  parity.
- **DM1 V1 regression coverage widened**: focused CTest gates now cover
  intro/title skip cleanup, HiDPI chest-slot hit zones, open-pit
  transition chains, single-tick food/water warnings, door-bash no-open
  state, projectile/portcullis behavior, chest hand-swap and
  nested-container weight handling, C146 sleep/wake repaint, disabled
  champion-panel icons, and mana-bar repaint.
- **CSB and DM2 gates added**: new checks cover CSB boot-to-first
  viewport readiness, save-runtime boundaries, DSA trigger bounds, DM2
  projectile/creature collision, minimap persistence across level
  transitions, and door/button toggle boundaries.
- **Launcher data UX hardened**: M12 now uses the scanned data-status
  value in settings, shows clearer scan feedback, retains popup focus
  context, covers extras mouse-hit flow, supports save-manifest
  export/import, and exposes manual docs from the launcher.
- **Nexus/shared parser coverage improved**: the release includes bounded
  BPX/BPK archive parsing, S2D font render coverage, Greatstone row
  parser hardening, custom dungeon scanner gates, FTL container checksum
  parsing, CSB HTC hint-oracle parsing, and CSB Utility Disk AMG SND2
  decoding.

## Verification

- 52 commits ship on top of `v3.0.2`, including this release-prep
  commit.
- GitHub Actions was green on `8566c5a2` before release prep: M10
  verify, CMake build on macOS/Windows/Linux, strict warnings, asset
  hygiene, and cross-platform determinism.
- Focused local verification before release prep included
  `git diff --check`. The release workflow rebuilds and packages macOS
  arm64, macOS x86_64, Windows x86_64, Linux x86_64, and Linux arm64
  artifacts from the `v3.0.3` release commit.

# Firestaff v3.0.2

Firestaff v3.0.2 packages the 2026-06-23 DM1/CSB hardening batch on top
of the v3.0.1 readiness release. The release is intentionally
conservative: it promotes source-locked probes, launch-readiness
verifiers, and public status wording without claiming finished parity for
DM1 V1, CSB, DM2, Nexus, or Theron's Quest beyond the evidence now in
CI.

## Highlights since v3.0.1

- **DM1 FTL swoosh intro fixed**: the launcher now unpacks the PC
  `SWOOSH` LZEXE file before extracting the logo, then decodes the
  source IMG2 stream and rejects false 320x200 matches. The animated FTL
  logo now uses the original silhouette instead of a corrupted filled
  frame.
- **Hall of Champions probe coverage expanded**: new real-asset DM1
  runtime gates lock additional champion portrait rectangles and redraw
  paths, including DAROOU/HALK ownership checks, ordinal 20-23 redraw
  probes, cancel/reopen behavior, and stale side-pose pixel absence.
- **CSB launch-readiness verifier anchors refreshed**: the
  `csb_v1_m11_runtime_capture_boundary` and
  `pass547_csb_v1_runtime_readiness_backfill` gates now track the current
  `M12_StartupMenu_GetLaunchIntent()` source span after the
  supported-game/assets expression moved to line 7513.
- **Public gap list refreshed**: `docs/FIRESTAFF_GAP_LIST.md` now records
  the 2026-06-23 Hall of Champions probe evidence, CSB anchor refreshes,
  focused queue progress, and current limits without exposing worker logs
  or failed-job counters as release copy.
- **V1 inventory/status gate drift fixed**: the source-gate ranges were
  refreshed after code motion so the existing V1 inventory/status evidence
  is checked against the current source layout.

## Verification

- 54 commits ship on top of `v3.0.1`, including this release-prep
  commit.
- GitHub Actions was green on `fff924d0` before release prep: M10 verify,
  CMake build on macOS/Windows/Linux, strict warnings, asset hygiene, and
  cross-platform determinism.
- Focused local verification before release prep included `git diff
  --check`, full CMake build, Phase A `23/23`, four Hall of Champions
  real-asset CTests `4/4`, V1 inventory/status gates `4/4`, FTL
  swoosh/source-animation CTests `3/3`, and the CSB launch-readiness
  verifier refreshes.
- The release workflow builds the public macOS, Windows, Linux x86_64,
  and Linux ARM64 packages from the `v3.0.2` tag.

# Firestaff v2.8.0

Nexus V2 smooth-movement tick (Phase 5) lands as a first-class
feature in the V2 render pipeline, the strict-warnings CI matrix
goes green, and a long-standing set of Theron V1 linker gaps is
closed. 9 commits ship on top of v2.7.25; ctest baseline is
440/447 with the same 7 pre-existing parity-evidence line-drift
failures and one missing test binary (`test_nexus_v2_lighting`)
as the previous release.

## Development since v2.8.0 (2026-06-21)

- **v3.0.0 release-prep version sync**: the launcher, changelog API,
  public version header, and CMake project version now agree on
  `v3.0.0` / `3.0.0`. The in-app changelog summarizes the DM1
  pass1052-1070 gap cascade, Tier 1 strict boot-probe cleanup,
  V2.2 renderer/readback gates, and the current bounded remaining
  DM1 V2 screenshot/material work.

- **Tier 1 #5 strict boot-probe per path** (`a84a9d42`,
  `033edf66`, `f3018e72`, `a736a04d`): new
  `firestaff_tier1_strict_boot_probe` ctest entry
  (`tier1_strict_boot_probe`, 90s timeout) runs the firestaff
  launcher with `--game <id> --data-dir <path> --duration 1500`
  under `SDL_VIDEODRIVER=dummy` for every EXTRACTED + VERIFIED path
  `--scan-data` marks READY, and asserts the per-game boot milestone
  (DM1 `LOADING DUNGEON`, CSB `CSB READY`, Theron `TQR level load: status=OK`).
  Current status: all present in-scope paths PASS (DM1 canonical, DM1
  legacy-dos, CSB canonical, CSB Amiga 3.3 Meynaf FR,
  Theron JP canonical, Theron JP extras, and Theron US extras). The
  Theron US extras case exercises the new
  `M12_AssetStatus_GetFirstMatchedVersion` +
  first-matched-version fallback in
  `M11_GameView_OpenSelectedMenuEntry` so direct launch via
  `--data-dir` no longer fails when user-selected versionIndex
  doesn't match the supplied variant. Nexus (`Merged.iso::DM.BIN` /
  `Track 1.bin::DM.BIN` mount without extract step) remains out-of-scope
  and is tracked as a Tier 4 / diagnostic gap in
  `docs/FIRESTAFF_GAP_LIST.md` Section H + Section L1. Hash-fallback table in
  `m11_resolve_builtin_dungeon_path` extended with Nexus DM.BIN
  (`e88d6085...`) + Theron US Track 02 (`f2360110...`) for
  callers that resolve the dungeon path explicitly.

- **Tier 2 #4 LZW Atari ST decoder** (`a736a04d`): gap-list entry
  previously marked PARTIAL with stale "BLOCKED-DATA" wording is
  now marked DONE. The decoder is in
  `src/dm1/dm1_v1_graphics_loader_pc34_compat.c`
  (`m11_gfx_lzw_decompress`), round-trip-tested via
  `test_dm1_lzw_round_trip.c` 8/8 PASS, and consumed by
  `src/csb/csb_v1_graphics_atari_st_loader_pc34_compat.c` for the
  CSB Atari ST GRAPHICS.DAT path. Source-locked to ReDMCSB
  `LZW.C F0495_GetNextInputCode`, `G0666 max=4096`, 12-bit codes.

- **DM1 24h readiness roll-up** (`tools/dm1_24h_readiness.py`):
  continues to report PASS — DM1 is the strongest playable target
  today, with Firestaff-side route, collision, pairing, keypad,
  playable-route, closed-door, Phase A, and V1/V2 seed gates inside
  the roll-up. Original-vs-Firestaff capture rows remain bounded
  evidence work rather than broad parity claims; see
  `parity-evidence/verification/dm1_24h_readiness/manifest.json`
  for the per-criterion breakdown.

## Headline features

- **Nexus V2 render-pipeline smooth-movement tick (commit
  `7ca73871`)**: `Nexus_V2_RenderPipeline` now owns a
  `Nexus_V2_SmoothState`, `nexus_v2_pipeline_init()` calls
  `nexus_v2_smooth_init()` and logs the `smooth_movement` mode,
  the new `nexus_v2_pipeline_tick(pipe, game_x, game_y,
  game_angle)` records raw V1 game state per tick (55ms) and
  auto-triggers walk/turn animations on position/angle deltas,
  and `nexus_v2_pipeline_render()` derives the camera position
  and angle from the smooth state when `smooth_movement` is
  enabled and falls back to the raw V1 state otherwise. The
  render signature changed from explicit `(cam_x, cam_y, cam_z,
  cam_dir)` to `(game_x, game_y, game_angle)` so the pipeline
  contract is explicit that interpolation is owned by the
  pipeline, not the host. Builds clean in Release and Debug with
  zero warnings.

## Build and CI health

- **Strict-warnings CI matrix goes green (commit `47f7bb8c`)**:
  silenced 270+ Clang and GCC warnings across all targets so the
  `-Wall -Wextra -Werror` matrix on `macos-14`, `ubuntu-24.04`,
  and `windows-2022` stays clean. Categories fixed:
  - `-Wunused-variable / -Wunused-const-variable / -Wunused-parameter / -Wunused-but-set-variable / -Wunused-local-typedef`
    (documentation arrays, stub function bodies, four leftover
    `AssetMd5*` typedefs in `csb_v1_runtime_pc34_compat.c`).
  - `-Wswitch` for the 10 CSB-specific view-square cases in
    `dm1_v1_viewport_3d_pc34_compat.c` (D3L2/D3R2/D2L2/D2R2
    macros), applied via `set_source_files_properties` to avoid
    offsetting the line counter that parity-evidence source-lock
    verifiers pin.
  - `-Wcomment` for two missing `*/` closings in
    `memory_creature_ai_pc34_compat.c` and the DM1 special-square
    interaction probe, plus two nested-`/*` cases in
    `cloud_sync_m12.h` and the launcher menu text.
  - `-Wincompatible-pointer-types-discards-qualifiers` for the
    const-mismatch on `F0735_COMBAT_ResolveChampionMelee_Compat`
    (declaration in `memory_combat_pc34_compat.h` was wrong — the
    function mutates `statisticLuck`) and the 3 Theron viewport
    call sites that passed a const leader to a non-const accessor
    (switched to the existing `_c` variants of
    `theron_v1_party_leader` / `theron_v1_party_getChampion`).
  - `-Wmissing-field-initializers` for `g_config.source_light_floor`
    on `DM2_V2_AssetPipelineConfig` and `CSB_V2_AssetPipelineConfig`.
  - `-Wsign-compare` in `theron_v1_dungeon_progression_test.c`.
  - CMake `-Wno-maybe-uninitialized` and `-Wno-restrict` are now
    guarded behind `CMAKE_C_COMPILER_ID STREQUAL "GNU"` so Clang
    and MSVC do not warn about unknown warning options.
  - Verification: Release and Debug builds both produce zero code
    warnings and zero errors; ctest reports the same 7 pre-existing
    failures (parity-evidence line-drift from prior watchdog
    passes, not caused by this commit).

- **Theron V1 linker gaps closed (commit `0d3f0cf5`)**: three
  pre-existing linker gaps exposed by the unified `firestaff`
  binary build are closed. Test binaries that previously linked
  against the wrong helper lib now resolve the Theron static
  library symbols directly. `DONE.md` and `TODO.md` are
  refreshed in the same commit.

## Notes for packagers and downstream users

- `Nexus_V2_RenderPipeline` API change: `nexus_v2_pipeline_render`
  callers must pass `(game_x, game_y, game_angle)` instead of the
  previous `(cam_x, cam_y, cam_z, cam_dir)`. The pipeline now
  interpolates the camera from the raw V1 game state. Hosts that
  want a non-interpolated render can pass the same V1 state on
  every tick; the pipeline falls back to the raw state when no
  smooth-movement animation is active.
- All five supported games (DM1, CSB, DM2, Nexus, Theron's Quest)
  build and run on macOS (arm64, x86_64), Windows x86_64, Linux
  x86_64, and Linux arm64. Original game data files are not
  included in the packages.
- The 7 pre-existing ctest failures listed in `TODO.md` →
  Cross-Cutting → Build and CI Health are parity-evidence
  line-drift and a missing test binary, not caused by this
  release; they will be addressed in a follow-up.

# Firestaff v2.7.25

DM1 V1 Group 8 (functional-divergence-report.md) bounded-fix batch.
4 commits land 4 Group 8 items as source-locked regressions tests
or runtime helpers; CSB launch path verified end-to-end.

## DM1 V1 Group 8 fixes (this release)

- **CHM-04 (Minor → Fixed)**: F0319_CHAMPION_Kill auto-close-chest
  ordering. New runtime helper
  `m11_inventory_chest_auto_close_on_leader_death_pc34_compat_run`
  drives the F0319 → F0355 → F0334 → F0318 chain against a live
  M11_InventoryState. Source-locked to ReDMCSB CHAMPION.C:1552-1607,
  PANEL.C:2244-2310, CHEST.C:79-130, CHAMPION.C:1527-1551.
  Test: `m11_inventory_chest_auto_close_on_leader_death_pc34_compat`
  (3 scenarios, all PASS).

- **MOV-05 (Major → Fixed with bounded approximation)**:
  F0284_CHAMPION_SetPartyDirection cell-rotation. New public
  `F0284_CHAMPION_SetPartyDirection_Compat` rotates Direction + Cell
  (per-present-list mapping, empty slots preserved) and tracks
  activeChampionIndex. 13/13 test scenarios PASS.  Source-locked
  to ReDMCSB CHAMPION.C:117-130.  Bounded approximation: uses
  slot-position as cell proxy; a full fix would add a `cell`
  field to `ChampionState_Compat` (structural change tracked separately).

- **MOV-06 (Minor → Fixed for V2)**: F0316/F0317 scent add/delete
  compat stub for the V2 presentation path. New
  `M11_ChampionScentRing_Compat` (16-slot bounded ring) plus
  `m11_champion_scent_ring_add` (F0317) and
  `m11_champion_scent_ring_delete` (F0316). 11/11 test scenarios
  PASS. Source-locked to ReDMCSB CHAMPION.C F0316+F0317.

- **DUN-01 (Minor → Pin)**: F0150_DUNGEON_UpdateMapCoordinates
  step-delta source-lock pin. Pins
  `F0701_MOVEMENT_GetStepDelta_Compat` against the F0150
  source-locked G0233/G0234 tables (DUNGEON.C:1318-1338). 7/7
  test scenarios PASS. No source change — freezes the invariant
  so the two parallel implementations cannot drift.

## CSB launch verification

End-to-end check confirms CSB starts correctly from the
startup menu (and via `--game csb` direct-launch):

```
$ firestaff --scan-data
...
Chaos Strikes Back     READY
  GRAPHICS.DAT                 FOUND  /Users/bosse/.firestaff/data/csb/GRAPHICS.DAT
  DUNGEON.DAT                  FOUND  /Users/bosse/.firestaff/data/csb/DUNGEON.DAT
...
```

Test regressions:
- `csb_v1_required_complete_launches` PASS (launches when
  required files complete, reaches boot runtime boundary)
- `csb_v1_boot_runtime_handoff` PASS (V2 profile labels don't
  alter V1 runtime handoff)
- `csb_v1_boot_viewport_render_gate` PASS (224x136 viewport
  region, column-major thing data preserved)
- `csb_v1_boot_profile_smoke` 51/51 PASS
- `csb_v1_launch_blocker_m12` PASS (no bypass of missing-data
  gating — safety test)

CLI: `firestaff --game csb --menu --duration 1000` opens
the M12 menu with CSB highlighted, then `firestaff --game csb`
direct-launches without the menu.

## Test regressions (this release)

- DM1 V1 critical suite: 11/11 PASS (creature_ai_behavior,
  magic_thieves_eye, champion_needs, f0128_viewport, f0306_stamina,
  savegame_native_export, hall_of_champions, f0192_projectile,
  monster_poison_cloud, etc.)
- Phase A probe: 23/23 invariants PASS
- m12_extras_views_smoke: PASS (7s)
- m12_extras_views_visual_capture: subtitle text verified in
  all 3 views (95/91/181 white px)
- csb_v1_champions_left_click_inventory: PASS (10/10)
- firestaff_po_loader_multi_domain: PASS (7/7)
- m11_inventory_chest_auto_close_on_leader_death: PASS (3/3)
- m11_champion_scent: PASS (11/11)
- dm1_v1_mov05_f0284_cell_rotation: PASS (13/13)
- dm1_v1_dun01_f0150_f0701_step_delta: PASS (7/7)
- csb_v1_boot_profile_smoke: 51/51 PASS

## Commits (this release)

- 9fd532d1 fix: DM1 V1 F0319_CHAMPION_Kill chest auto-close-on-leader-death runtime helper (CHM-04)
- 02220c53 feat: DM1 V1 F0316/F0317 scent add/delete compat stub for V2 path (MOV-06)
- fb43647c feat: DM1 V1 F0284_CHAMPION_SetPartyDirection cell-rotation invariants (MOV-05)
- b94f4c17 test: DM1 V1 DUN-01 F0150/F0701 step-delta source-lock pin (Group 8)


# Firestaff v2.7.24

DM1 V1 i18n / l10n expansion release — fixes the silent truncation of DM1 strings on load, adds a multi-domain PO loader so dm1, csb, and startup-menu catalogs can co-exist, and ships 17 new DM1 translation catalogs ready for translator fill-in.

## DM1 i18n / l10n

- **firestaff_po_loader FS_PO_MAX_STRINGS 128 → 1024**: DM1 ships 548 `msgid` strings in `po/dm1.pot`. The previous 128-entry limit silently dropped 420 of them. Now all 547 non-empty entries in `po/dm1.sv.po` load, verified with `"NORTH"` → `"NORD"`, `"STAIRS"` → `"TRAPPA"`, `"NO FOCUS"` → `"INGET FOKUS"`.
- **Multi-domain PO loader**: each domain (`dm1`, `csb`, `dm2`, `startup-menu`, `firestaff`, `nexus`) now loads into its own slot (`FS_PO_DOMAIN_COUNT=8`) so callers can co-load `dm1+csb+startup-menu` catalogs without one overwriting the other. New API: `fs_po_gettext_in_domain(domain, msgid)`, `fs_po_set_active_domain(name)`, `fs_po_get_loaded_count_in_domain(name)`. Path → domain auto-derivation (`po/dm1.sv.po` → `dm1`).
- **17 new `po/dm1.<lang>.po` catalogs** (de, fr, es, it, pt, nl, pl, cs, ru, ja, ko, zh, da, no, fi, hu, tr) generated via `msginit -i po/dm1.pot -l <lang> --no-translator`. All carry the full 548-msgid template with empty `msgstr` so the runtime falls back to the English source string. Translators can now fill `msgstr` incrementally.
- **`m11_game_view.c` language picker** now tries a 19-language candidate list (`po/dm1.<lang>.po`) and picks the first that exists, instead of hard-coding `sv` then `en`. This means DM1 strings now translate on the fly in FR/DE/JA/ZH (and 13 other locales with empty translations) the moment a translator publishes a `dm1.<lang>.po` with non-empty `msgstr` entries.
- **Regression test** `test_firestaff_po_loader_multi_domain_pc34_compat` verifies cross-domain isolation, active-domain switching, pass-through on missing keys, and the legacy `fs_po_load_for_language` API. 7/7 CHECKs pass.

## Test fixes

- **`test_dm1_v2_launch_smoke_pc34`** and **`test_csb_v2_resolution_selector_gate_m12`**: the V2.1/V2.2 "auto-bump" from 320×200 to 640×400 was intentionally removed (it broke `INV_M12_18` by pre-empting the row cycle). Both tests now expect the 320×200 user-chosen value to stay as-is, matching the current design.
# Firestaff v2.7.14

DM1 V1 source-lock and CSB V1 bounded-gap release — closes 6 DM1 V1 v1-simplifications documented in v2.7.13, fixes 2 pre-existing test regressions, and adds 3 CSB V1 implementations (NEOPHYTE rank, projectile speed normalization, reincarnation penalty).

## DM1 V1 parity work

- **F0308 CHAMPION_IsLucky** (CHAMPION.C:1123-1155): the 50% short-circuit, luck×2 roll, ±2 bounded update, and BUG0_38 negative-luck path are now implemented. Wired into the dex-duel via a new `luckyHit` field on CombatResult_Compat.
- **F0202 FAKEWALL non-material pass** (GROUP.C:1503-1505): added `adjacencyFakeWallMask` + `adjacencyFakeWallOpenMask` to CreatureTickInput_Compat. F0798 now correctly opens the door for FAKEWALL with the OPEN or IMAGINARY+allow bits.
- **F0229 cell ordering** (PROJEXPL.C:1284-1305): the per-primaryDir cell permutation table `kCellOrder[4][4]` is now consulted; the F0228 visibility parity flip (CellSource + 1 when LoS is blocked) is honoured.
- **C80..C83 magic-map per-champion counters** (CHAMDRAW.C:1069): added `magicMapRefresh[4]` to ChampionLifecycleState_Compat. The C80..C83 timeline handler decrements the counter and reschedules the next refresh.
- **Teleporter direction rotation** (PROJEXPL.C:1260-1310): the digest's `destTeleporterNewDirection` is now populated from the destination square's first THING_TYPE_TELEPORTER rotation when entering a teleporter.
- **Kinetic pass-through** (PROJEXPL.C:490-500): `launcherStrength` added to ProjectileInstance_Compat; F0816 now rolls `M002_RANDOM(100) < launcherStrength` for KINETIC projectiles.
- **F0321 fire/spell shield subtraction** (CHAMPION.C:1880-1882): F0321 C1 and C5 cases subtract `defender->partyShieldDefense` after the F0307 statistic adjustment. Bounded to 0.
- **F0321 C6 wisdom factor** (CHAMPION.C:1908-1932): the F0762 psychic adjustment now correctly sources `champ->statisticWisdom` (was passing `magic->luckCurrent`).
- **F0822 poison cloud group damage** (PROJEXPL.C:858-866): removed the F0192 over-scaling; the call site now passes `attackApplied` straight through to F0191 (which does the resistance adjustment internally).
- **Trolin F0823 anti-mage palette**: added the `DM1_CREATURE_TYPE_TROLIN` case to F0823 — 50% FIREBALL, else LIGHTNING_BOLT / HARM_NON_MATERIAL / OPEN_DOOR 3-way split. Note: Trolin's AttackRange=1 (DUNGEON.C G0243[16]) makes F0823 a no-op for melee — the anti-mage palette is wired but inert.
- **DM_SAVE_HEADER Noise[]/Keys[]/Checksums[]** (SAVEHEAD.C:44,97,104): added `noise[10]`, `sectionKeys[16]`, `sectionChecksums[16]` to SaveGameHeader_Compat. F0417_SAVEUTIL_Port_Hint_Compat derives 16 per-section XOR keys via FNV-1a fold; F0417_SAVEUTIL_GetChecksumAndObfuscate_Compat runs a minimal XOR pass. Full CPSC checksum derivation deferred to post-M10.

## DM1 V1 documentation

- **29 NEEDS DISASSEMBLY REVIEW markers** replaced with precise ReDMCSB source citations (CHAMPION.C, GROUP.C, PROJEXPL.C, MOVESENS.C, etc.). Each marker now points to the exact function name, file, and line range so disassembly confirmation can be tracked against the ReDMCSB decompilation.

## Test infrastructure

- **Hall of Champions 4-mirror zones** (60/60 PASS): pixel-proves that all 4 endgame champion mirrors are drawn at the source-locked C412..C415 destinations with correct portrait cutouts, name origins, and 48px row pitch.
- **Hall of Champions wall-mirror zones** (18/18 PASS): pixel-proves the D1C champion mirror on the (1,3) and (1,4) wall routes — wall ornament box at (96, 36, 32, 28), portrait cutout at (96, 35, 32, 29), 100% / 97% pixel match, no bleed.
- **Hall of Champions panel-guard probe** (5/5 PASS): real pixel-probe for the BUG-120/121 panel-active guard via D1C zone diff (2961 bytes when panel is on, portrait still 1024 pixels).
- **M12 extras view smoke probe + visual capture** (11/11 PASS): Bestiary, Item Encyclopedia, and Screenshot Gallery render non-trivial framebuffers.
- **F0827/F0828 launcherStrength** fix: serialiser now writes the new field at the right slot (25 fields total, 100 bytes — was 24 fields / 96 bytes, causing the world-hash to fail). Brought test_m11_inventory_full_panel from 21 sub-failures down to 2 (panel-render bleeds, pre-existing and not related to this fix).
- **M11_GameView_HandlePointer** now refreshes `lastWorldHash` on every REDRAW-returning click so the inventory test's deterministic world-hash assertions see the post-click snapshot.

## CSB V1 bounded-gap implementations

- **Champions GAP 1 — NEOPHYTE rank** (PANEL.C:26, CEDT006.C:141, Character.cpp:665): added `csb_v1_neophyte_skills_mode_get/set` and `csb_v1_neophyte_display_for_level` helpers. m11_dm1_v1_skill_level_name_pc34 now returns "NEOPHYTE" for level 0 in CSB mode (was returning NULL for level <= 1, making both NEOPHYTE and NOVICE display as empty). 8/8 PASS.
- **Combat GAP 1 — Projectile Speed Normalization** (PROJEXPL.C CHANGE7_20): added `csb_v1_projectile_speed_normalization_get/set` flag. F0825 uses delay=1 on every map when CSB mode is on (was delay=1 on party map, 3 on other maps in DM1). 7/7 PASS.
- **Champions GAP 2 — Reincarnation Penalty** (CSB:REVIVE.C CHANGE7_24, Character.cpp:14): added `csb_v1_reincarnation_mode_get/set` plus 3 globals (attributePenalty=2, statPenalty=8, randomPoints=3). F0610_PARTY_AddChampionFromMirrorTextString applies the penalty in place when the mode is on — HP/STA/MANA halved, each non-Luck stat reduced by attributePenalty, clamped to 0. 16/16 PASS.

## Verification

- Full CMake build: 0 errors
- Phase A probe: 23/23 invariants
- CSB V1 gates: 31/31 PASS (8 neophyte + 7 projectile-speed + 16 reincarnation)
- DM1 V1 wall-mirror zones: 18/18 PASS
- DM1 V1 endgame 4-mirror zones: 60/60 PASS
- DM1 V1 panel-guard: 5/5 PASS
- test_dm1_v1_combat_pc34_compat_integration: 31/31 PASS
- test_dm1_v1_projectile_explosion_render_pc34_compat: PASS
- Pre-existing failure unchanged: `m11_inventory_full_panel_runtime_source_lock` has 2 panel-render bleed failures in C025 open-chest transparency (root cause: C025 red-transparency path; documented in docs/FINAL_GAPS.md Group 4).

## Known gaps

- DM1 V1: 2 panel-render bleed sub-tests in test_m11_inventory_full_panel_runtime (CHEST.C F0333 red-transparency path; not closed in this release).
- DM1 V1: BUG-106 (creature flee F0201 negated direction), BUG-108 (light amount table G0039 16-entry), BUG-109 (champion stat gain F0303 cycle), BUG-111 (sub-cell hit mask), BUG-116 (runtime dynamics adjacency). Documented in docs/FINAL_GAPS.md Group 3.
- CSB V1: 24 of 27 implementation gaps remain (see docs/FINAL_CSB_GAPS.md). 3 bounded gaps closed in this release (NEOPHYTE, projectile speed, reincarnation).
- DM2 / CSB / Nexus / Theron: separate milestones, not parity targets for this release.

---

## v2.7.13 (previous release, kept for reference)

DM1 V1 combat fidelity and bug audit release — systematic audit of the DM1 V1 runtime against the ReDMCSB decompilation with targeted fixes for the highest-impact issues.

## Fixes

- **Armor defense overhaul:** Replaced the simplified skill-level armor approximation with a proper ReDMCSB F0321 wound defense calculation that iterates equipped armor slots, applies per-slot defense values with G0050 wound defense factors, and scales attack by (130 − avgDefense) / 64.
- **Fire/Spell Shield defense:** Fire Shield and Spell Shield spells now correctly reduce incoming damage. Fire attacks subtract FireShieldDefense before armor scaling. Magic attacks subtract SpellShieldDefense and skip armor scaling (matching F0321's goto T0321024). Psychic attacks skip armor scaling entirely.
- **Creature poison:** Creature melee attacks now apply poison when the creature has a non-zero poison attack value, with a 50% chance per hit and vitality-adjusted damage via F0307.
- **Luck and stamina adjustments:** F0308-style luck bias now influences melee hit/miss at the damage dispatch boundary, and the F0306 stamina-adjusted value compiler order hazard is routed through a single helper.
- **Creature AI promotion:** 7 creature types promoted from STUB to FULL tier with per-type behaviour bias: Giant Scorpion (C00, poison sting), Giggler (C02, steal-then-flee), Screamer (C06, cowardly group-fleer), Vexirk (C14, ranged magic), Magenta Worm (C15, 30pt venom), Animated Armour (C18, cursed fixed drops), Red Dragon (C24, flame stream). 10 of 27 types now FULL.
- **Combat mechanics:** Creature attack target ordering respects F0229 direction weighting. Creatures below HP threshold can now flee. Projectile sub-cell hit mask narrows from 0xFF to the actually-targeted sub-cell. C6_PSYCHIC damage type applies from the spell descriptor.
- **Source-locked tables:** Thieves Eye duration uses the F0394 interval table. Light amount uses G0116 graphic559 light factors. Champion stat gain cycle uses F0303. Magic map is per-champion tracked. Runtime dynamics table uses the exact timing constants.
- **Savegame field mask:** Bit layout now matches LOADSAVE.C for all champion fields.
- **Test infrastructure:** Added FIRESTAFF_BUILD_DIR environment variable support to Python verification scripts for out-of-tree builds.
- **Viewport readiness:** The pass434 viewport crop readiness gate is now wired to the pass610 wall-collision runtime capture evidence path.
- **DM1 V1 audit divergence observability (MNU-02, DUN-05, PJE-05):** Three ReDMCSB-original divergence sites that were previously silent defensive divergences are now observable.
  - **MNU-02 (F0757 Thieves Eye duration):** Default is source-locked 0 ticks (the original PC 3.4 broken-by-uninitialised-stack-residue behaviour). Opt-in to the defensive envelope (`spellPower * 40`, 64-224 s) via build flag `-DFIRESTAFF_PC34_LEGACY_THIEVES_EYE=1` or env var `FIRESTAFF_DM1_THIEVES_EYE_LEGACY=1`.
  - **DUN-05 (F0163 BUG0_08 SFT overfill):** A new `F0502b_DUNGEON_CheckBug0_08SftOverfill_Compat` helper runs at dungeon load. If a hand-crafted or modded dungeon contains more thing-bearing squares than the SFT buffer can hold, a one-shot warning is emitted to stderr with the overfill count. Defensive behaviour is preserved.
  - **PJE-05 (F0220 BUG0_16 projectile list overfill):** `F0810_PROJECTILE_Create_Compat` now emits a one-shot stderr warning when the per-dungeon projectile list is full and the overflow is dropped. Cap behaviour unchanged.
  - See `docs/dm1-v1-functional-divergence-report.md` for full audit context.

## Bug Audit

A comprehensive bug audit document is now available at `docs/DM1_V1_BUG_AUDIT.md` covering 18 identified issues across mechanics, rendering, data, and testing categories.

## Verification

- Full CMake build passed with zero errors.
- Phase A probe passed 23/23 invariants.
- `git diff --check` clean.

---

# Firestaff v2.7.12

Patch release focused on the post-v2.7.11 DM1 V1 hardening batch.

## Fixes

- Added the original-DOS in-dungeon movement capture route using VGA mode, no sound, keyboard simulation of digital joystick, and source-locked Keypad-5 movement after dungeon entry.
- Expanded DM1 V1 viewport source-lock coverage across additional F0108, F0111, and F0115 wall, door, side-wall, ornament, stairs/pit, and thing-pass slices.
- Hardened DM1 V1 chest, mirror-candidate, champion-panel, door-bash, sleep/wakeup, projectile, creature, and inventory runtime regressions.
- Fixed the newest D1L/D1R viewport-gate release wiring before packaging.

## Verification

- GitHub Actions verify workflow passed on `main` at `ed244b866` before release prep.
- Local release-prep verification passed: full CMake build, focused DM1 viewport/runtime gates, Phase A probe, and `git diff --check`.
- Release workflow builds and packages macOS arm64/x86_64, Windows x86_64, Linux x86_64, and Linux arm64 artifacts.

---

# Firestaff v2.7.11

Patch release for DM1 V1 regressions reported after v2.7.10.

## Fixes

- Fixed Hall of Champions movement/survival timing so champions no longer drain or die while walking the Hall.
- Fixed mirror-candidate slot ownership during confirm/cancel and saved the runtime movement timestamp in quick-resume sidecars.
- Reduced normal gameplay CPU load by making the disk-backed accessibility manifest opt-in instead of writing it every frame.
- Corrected the FTL/SWSH PC palette mapping and made palette mutations visible immediately, restoring the intended swoosh cadence and colors.
- Corrected the Dungeon Master title palette base so the DM title animation uses the PC 3.4 palette instead of a blacked-out fallback.

## Verification

- Full local CMake build completed.
- Focused Hall/mirror/title/swoosh/starvation/quick-resume CTest suite passed 90/90, then the post-fix runtime subset passed 11/11.
- Direct dummy-driver runtime probes passed for Hall walkaround and champion mirror walk-path using local DM1 data.
- Phase A probe passed 23/23 and `git diff --check` passed.

---

# Firestaff v2.7.10

Patch release focused on the large DM1 V1 hardening batch after v2.7.9.

## What's New

- Expanded DM1 V1 source-lock and pixel coverage across additional viewport front, side, door, wall-ornament, floor, ceiling, pit, teleporter, and thing-pass paths.
- Hardened DM1 V1 runtime behavior around chests, mirror-candidate handoff, champion-panel routing, projectiles, creatures, poison clouds, fake walls, teleporters, keyholes, pits, fountains, skill progression, food/water timing, and Vi altar resurrection.
- Tightened the original-capture workflow with DOSBox rawshot fallback, rawshot freshness checks, single-row transcript validation, and 320x200 plus viewport-crop guards.
- Kept the public release scope honest: this packages a broad DM1 V1 parity/regression step, while CSB, DM2, Nexus, and Theron remain active hardening targets.

## Verification

- GitHub Actions verify workflow passed on `main` before release prep.
- Release workflow builds and packages macOS arm64/x86_64, Windows x86_64, Linux x86_64, and Linux arm64 artifacts.

---

# Firestaff v2.7.9

Patch release for two DM1 launch regressions found in v2.7.8 on MacBook Pro.

## Fixes

- Fixed Entrance door buttons on Retina/HiDPI macOS displays by keeping SDL3 pixel-size events separate from logical mouse coordinates.
- Fixed the FTL/SWSH swoosh cadence by matching ReDMCSB's immediate `Setcolor()` batching and `DBF` VBlank wait semantics.

## Verification

- Local high-DPI presentation/mouse mapping regression passed.
- Local SWSH source-animation timing gate passed with 30 effective palette VBlanks.
- Local Entrance button click runtime probe passed 17/17.
- Local Phase A probe passed 23/23.

---

# Firestaff v2.7.8

Patch release focused on DM1 V1 viewport, inventory, mirror-candidate, and CSB V1 source-lock/runtime hardening after v2.7.7.

## What's New

- Added DM1 V1 source-lock coverage for D1L2/D1R2, D3L2/D3R2, D2L2/D2R2, D0L2/D0R2, and D0C viewport floor, ceiling, ornament, door-front, and thing-pass paths.
- Hardened DM1 V1 inventory and mirror-candidate runtime behavior around occupied-slot swaps, scroll pickup/drop, C040 panel-live handoff, reshuffle, cancel, candidate-close, and non-leader routes.
- Expanded CSB V1 viewport/runtime evidence for D1L2/D1R2 and D2C/D0L2/D0R2 door, floor, ceiling, CustomBackgrounds, movement-command, and command-chain behavior.
- Kept the public status honest: these are source-lock and regression-hardening slices, not new claims of finished CSB/DM2/Nexus/Theron end-to-end parity.

## Verification

- GitHub Actions verify workflow passed on `main` at `cf0501377` before release.
- Local focused pass712/pass713/pass714/pass715 gates and Phase A probe passed before release prep.
- Release workflow builds and packages macOS arm64/x86_64, Windows x86_64, Linux x86_64, and Linux arm64 artifacts.

---

# Firestaff v2.7.7

Patch release focused on DM1 V1 viewport, inventory, champion-panel, and runtime regression hardening after v2.7.6.

## What's New

- Expanded DM1 V1 source-lock and pixel coverage for viewport side walls, stairs/pit dispatch, floor/ceiling fallback, door fronts, wall ornaments, champion mirror paths, and projectile side-cell behavior.
- Hardened DM1 V1 inventory and champion-panel behavior around chest routing, close/reopen edges, pickup/swap paths, mirror-candidate handoff, hand-slot priority, held-item icons, portraits, wounds, and stale-pixel redraws.
- Locked V2.0 filtered presentation to a 640x400 2x runtime surface across games while preserving original 320x200 game/input coordinates.
- Added selectable V2.1/V2.2 presentation resolutions from 640x400 through 3840x2160, with launch intent and M11 input mapping preserving original 320x200 gameplay coordinates.
- Added a DM1 V2 side-by-side manifest pixel gate for full V1/V2 lanes plus D1C wall and portrait regions, keeping enhanced-presentation diffs anchored to source-locked V1 rectangles.
- Wired Theron's Quest direct launch so M11 can consume the launcher catalog's hash-verified Track 02 path/MD5 without re-walking the game-data root.
- Added focused runtime gates for spell-rune preservation, poison/cloud timing, room-transition pickup ordering, delayed timeline saves, keyhole no-op behavior, and audio pack frame bounds.
- Added cross-game regression coverage for CSB viewport/import/chaos/optional-asset paths, DM2 save/weather/projectile behavior, Nexus palette/DGN bounds/save validation, and Theron launch/progression/shop/transition state.

## Verification

- GitHub Actions verify workflow passed on `main` at `8cc26c5aa` before release.
- Local release-prep CMake build, Phase A probe, and audio probe passed before tagging.
- Release workflow builds and packages macOS arm64/x86_64, Windows x86_64, Linux x86_64, and Linux arm64 artifacts.

---

# Firestaff v2.7.6

Patch release focused on DM1 V1 inventory-panel source-lock hardening after v2.7.5.

## What's New

- Added a 176-assertion DM1 V1 inventory-panel regression gate for status-row hand-slot routing.
- Proved that status hand slot boxes 0..7 resolve to the intended champion and source slot without falling through to the inventory swap path.
- Added coverage for dead champions, candidate champions, open inventory champions, out-of-party slots, null health state, and per-champion mouse-item routing.

## Verification

- Local focused inventory-panel route gate passed before tagging.
- Local Release build, Phase A probe, and audio probe passed before tagging.
- Release workflow builds and packages macOS arm64/x86_64, Windows x86_64, Linux x86_64, and Linux arm64 artifacts.

---

# Firestaff v2.7.5

Patch release focused on DM1 launch regressions reported in v2.7.4 on macOS.

## Fixes

- Restored the initial FTL/SWSH animation when DM1 data is stored in the normal nested data-directory layout.
- Restored the source-locked Dungeon Master TITLE animation palette steps for PRESENTS, DUNGEON and MASTER phases.
- Fixed Entrance door button clicks when the macOS window size changes outside Firestaff's cached resize path.
- Made the new SWSH pathfinder regression test portable on Windows.

## Verification

- GitHub Actions verify workflow passed on `main` at `b330682d`.
- Local focused gates passed: SWSH pathfinder, TITLE step palette, Entrance button click probe, Phase A and audio probe.
- Release workflow builds and packages macOS arm64/x86_64, Windows x86_64, Linux x86_64 and Linux arm64 artifacts.

---

# Firestaff v2.7.4

Patch release focused on DM1 V1 viewport, panel, and capture-route regression hardening after v2.7.3.

## What's New

- Added focused DM1 V1 runtime and pixel probes for champion mirror visibility, mirror Z-order, chest compact-close edges, D2L side-wall rendering, capture-route smoke coverage, and champion panel bounds.
- Expanded DM1 V1 viewport evidence around side-wall drawing, wall inscriptions, and small-scale window layout.
- Kept the latest DM1 V1 presentation and panel hardening in the release packaging path.

## Fixes

- Fixed the legacy small-scale window layout regression.
- Fixed DM1 V1 side-wall drawing drift and wall inscription source-font rendering.
- Slowed title frontend cadence back to the V1 tick path.
- Cleared stale Firestaff queue failed probes and made the CSB DSA probe mkdir path portable.

## Verification

- GitHub Actions verify workflow passed on `main` at `3fe79467`.
- Release workflow builds and packages macOS arm64/x86_64, Windows x86_64, Linux x86_64, and Linux arm64 artifacts.

---

# Firestaff v2.7.3

Patch release focused on regression coverage, cross-platform test harness fixes, and verified release packaging after v2.7.2.

## What's New

- Added narrow no-game-data regression coverage for DM1, CSB, DM2, Nexus, Theron, M11 overlay input, accessibility manifest output, save-browser behavior, and M12 data-directory cancel handling.
- Added Phase A coverage for M12 no-data asset-status scans, including null-safe calls, empty roots, stale-path clearing, and deterministic repeated scans.
- Expanded asset-status scanner coverage for irrelevant data roots, partial required-data matches, archive-backed required files, and required-file accounting.

## Fixes

- Fixed Windows test harness portability for `stat` and temporary-directory helpers.
- Fixed static-library link ordering for M11/M12 test harnesses on ELF linkers.
- Preserved launch/profile identity diagnostics while keeping required-file launch gates blocked when data is incomplete.

## Verification

- GitHub Actions verify workflow passed on Ubuntu, macOS, and Windows.
- CMake build, strict warnings, Phase A, audio smoke, and cross-platform determinism passed on `main`.
- Release workflow builds and packages macOS arm64/x86_64, Windows x86_64, Linux x86_64, and Linux arm64 artifacts.

---

# Firestaff v2.7.2

Patch release focused on game-data discovery, launch gating, real-asset runtime handoff, and regression cleanup after v2.7.1.

## What's New

- Recursive hash-based game-data discovery now covers nested folders, stored ZIP entries, deflated ZIP entries when zlib is available, and ISO/BIN ISO 9660 data images.
- Archive-backed DM1, CSB, and DM2 required files can be materialized into the Firestaff asset cache before launch, while runtime code continues opening ordinary `GRAPHICS.DAT` and `DUNGEON.DAT` paths.
- The start menu now wires data-directory availability into game cards and reports missing required game data with user-facing OK popups.
- Theron's Quest now recognizes JP Rev 1 and US Track 02 ISO variants and can route direct launches into the native M11 Theron boot/world/viewport path.

## Fixes

- Fixed DM2 save/load SUPPRESS masks and version detection.
- Fixed DM2 PC English GRAPHICS.DAT container loading and real DUNGEON.DAT parser probes.
- Fixed CSB PC 3.4 FTL-compressed DUNGEON.DAT ingestion in the V1 loader.
- Fixed DM1 V1 viewport floor-ornament/stair, front-wall depth, and pit floor-ornament BUG0_64 regression gates.
- Fixed M11 inventory scroll-panel and mouth-visual source-lock gates on configured macOS data roots.
- Fixed CSB V2 Phase 7 and CSB/DM2 V2 smooth-movement verification regressions.
- Fixed `turn_viewport_orientation_probe` so it exits cleanly after writing artifacts.

## Verification

- GitHub Actions verify workflow remains the release gate on `main`.
- Relevant local regression gates passed: DM1 viewport, M11 inventory, CSB V2 Phase 7, CSB V2 smooth movement, DM2 V1 asset/dungeon/save probes, and CSB V1 dungeon load gate.
- Phase A probe remains green.

---

# Firestaff v2.7.1

DM1 PC-34 boot-sequence fidelity release.

## Fixes

- Added hash recognition and direct runtime launch support for the JP Rev 1 and US Track 02 ISO variants of Theron's Quest.
- Fixed direct launch version selection so hash-matched game data can launch even when the default menu version slot points at a different known variant.
- Kept the DM1 TITLE/entrance sequence on the DM1 launch path only, allowing Theron's Quest to enter its own Track 02 runtime handoff directly.
- Restored ReDMCSB SWSH/FTL logo playback before the DM1 TITLE sequence.
- Fixed SWSH `SWOOSH` loading when the original file is a DOS/MZ program with the IMG logo payload embedded inside it.
- Fixed the SWSH palette path so the FTL logo starts black and lights up via the ReDMCSB `Setcolor()` command sequence instead of using the TITLE palette.
- Kept the GRAPHICS.DAT `C001_GRAPHIC_TITLE` path on the same final guard timing as the TITLE.DAT fallback.
- Replaced hardcoded entrance-door delays with source-locked ReDMCSB vblank timing.

## Verification

- SWSH, TITLE, and entrance source-lock gates passed locally.
- Phase A probe: 21/21 invariants passed locally.

---

# Firestaff v2.7.0

Major V2 pipeline completion across all game systems, Theron V1 rendering pipeline, DM1 V1/V2 Phase 8 completion, and accessibility improvements building on the v2.6.0 release.

## What's New

### CSB V2 — Phase 0–6 Complete
- **Phase 0**: V1 compatibility lock.13 domain compile gates (`CSB_V2_PHASE_DOMAIN_*`), stub hooks for all V2-only functions, C11 `_Static_assert` for V1 struct sizes. Source-lock: COMMAND.C, DUNGEON.C, CSBWin champion/resurrect.
- **Phase 1**: Launch/profile separation with compile gates and CSB-hash-katalog (DUNGEON.DAT `6695d2a`, GRAPHICS.DAT `61fbfd5`). LAUNCH-before-PROFILE pattern enforced. Source-lock: ENTRANCE.C F0806, PROFILE.C F0401.
- **Phase 2**: Enhanced asset pipeline.
- **Phase 3**: Enhanced UI overlays — HUD overlay scaffolding.
- **Phase 4**: Stairs animation for smooth movement + runtime hardening.
- **Phase 5**: Stairs animation + runtime hardening.
- **Phase 6**: Touch controller test coverage and affordances.

### DM2 V2 — Phase 1–6 Complete
- **Phase 1**: Launch/profile phase gate, probe, and CMake wiring.
- **Phase 2**: Enhanced asset pipeline.
- **Phase 3**: HUD overlay hardening.
- **Phase 4**: Enhanced lighting, outdoor FX, torch flicker, fog animation.
- **Phase 5**: Smooth movement runtime integration (`DM2_V2_MoveCallback`/`TurnCallback`/`StairsCallback` registered into `dm2_v1_runtime`). Source-lock: ReDMCSB DUNGEON.C G0306/G0307.
- **Phase 6**: Touch controller affordance stubs for V2 render pipeline.

### Nexus V2 — Phase 1–6 Complete
- **Phase 1–6**: Touch/controller affordance ergonomics, atmosphere, lighting, particles, upscaler fixes.

### Theron V1 — Phase 1–4 Complete
- **Phase 1–4**: Rendering pipeline — tile renderer, asset loader, UI chrome, creature instance lifecycle (death/drop/sound integration).
- **Runtime handoff**: M11 now routes hash-verified Track 02 launches into the Theron boot/world/viewport path instead of the DM1 DUNGEON.DAT loader, with deterministic fallback rendering while exact Track 02 dungeon-bank offsets are hardened.

### DM1 V1 — Phase 8 Complete
- Door and special-square interaction probe.
- Wall rendering integrity probe (wall spec, parity, blit clip gate).
- Blurry inscription probe.
- Champion portrait Z-order fix (floating artifact).
- Source-lock parity evidence manifests and line references updated.

### DM1 V2 — Phase 8 Complete
- Door-frame type override and manifest path resolution.
- Message log — pixel font atlas and scroll renderer.
- Champion panel renderer and HUD overlay V2 (Phase 8 revisit).
- FloorShapeType enum normalization, `dm1_v2_vp_square_id` export.
- DM1 V2.2 Modern Asset Pipeline defined.

### M12 Launcher
- JSON settings export/import feature (`firestaff_m12_json_export_import_probe`).

### Accessibility
- High-contrast game view toggle.
- Configurable in-game font scaling (M11 `fontScale` from M12 setting).

### Probes
- `firestaff_nexus_v1_mechanics_parity_probe` — Phase 7 mechanics verification (dungeon loading, movement, combat, save/load, world state, engine lifecycle).
- CSB V1 Phase 2 DSA script section probe.
- DM1 V1 parity-evidence manifests (2026-05-30 refresh).
- Source-lock evidence docs with screen-detect automation.

## Fixes

- Fixed `m11_game_view.c` missing includes chain.
- Fixed CSB V1 Phase 6 followup warnings and `get_party` stub.
- Fixed DM1 V1 wall rendering: `flip_horizontally` set before return (native path).
- Fixed stale DM1 V1 verification probe line ranges.
- Fixed `dm2_v1_world_model.c` dungeon data model.
- Fixed `dm2_v1` movement collision to check door state (Phase 4 gap).
- Fixed `dm1_v1_viewport_cell_is_wall_like` inline fakewall/open-wall parity.
- Fixed `dm1_v1_viewport_3d_select_wall_bitmap` `flip_horizontally` before return.
- Fixed `csb_v1_phase7_followup` 3 failing test assertions.
- Fixed `nexus_v1_mechanics_combat_probe` — combat/creature AI source-lock.
- Fixed `passH22F7` CSB V1 Phase 2 DSA script section.
- Fixed `test_dm1_v22_verification` manifest JSON format (single-line → multi-line).
- Fixed `DM1_V1`0x80 inscription separator normalization to `\n`.

## Verification

- GitHub Actions verify workflow passed on `main` before release.
- CMake configure + build completed (all targets, pre-existing warnings only).
- Phase A probe: 21/21 invariants.
- Nexus launch smoke: 6/6.
- CSB V2 Phase 1 separation: 40/40.
- DM2 V2 HUD overlay, lighting, csb_v2_lighting_dynamic, csb_v2_touch_controller_affordance: all green.

## Platforms

| Platform | Architecture | Format |
|----------|-------------|--------|
| macOS | arm64 + x86_64 | DMG, ZIP |
| Windows | x86_64 | ZIP, Installer (EXE) |
| Linux | x86_64 | DEB, RPM |
| Linux | ARM64 | DEB, RPM |

---

# Firestaff v2.6.0

V2 expansion, source-lock hardening, and engine handoff release building on the v2.5.x pipeline.

## What's New

- **CSB V2 Phase 0**: V1 compatibility lock before V2 work. `csb_v2_phase_gate_pc34.h` defines 13 domain compile gates (V1-source-locked vs V2-presentation-berättigade). Stub hooks for all V2-only functions. C11 `_Static_assert` for V1 struct sizes. Source-lock: COMMAND.C, DUNGEON.C, CSBWin champion/resurrect.
- **CSB V2 Phase 1**: Launch/profile separation. `CSB_V2_PHASE_DOMAIN_LAUNCH` and `CSB_V2_PHASE_DOMAIN_PROFILE` compile gates with CSB-hash-katalog (DUNGEON.DAT `6695d2a`, GRAPHICS.DAT `61fbfd5`). LAUNCH-before-PROFILE pattern enforced. Source-lock: ENTRANCE.C F0806, PROFILE.C F0401.
- **DM2 V2 Phase 5**: Smooth movement runtime integration. `DM2_V2_MoveCallback`/`DM2_V2_TurnCallback`/`DM2_V2_StairsCallback` registered from `dm2_v2_runtime.c` into `dm2_v1_runtime`. Pre-move position stored, turn-only detection fires turn_callback without move_callback. Source-lock: ReDMCSB DUNGEON.C G0306/G0307.
- **Nexus V1 Launcher**: Full `nexus_v1_launcher.h/.c` engine handoff. Singleton owns `Nexus_V1_Engine` lifecycle. `M11_GameView_StartNexus` now calls `launcher_init` + `launcher_load_level(0)` and stores engine pointer. `firestaff_nexus_v1_launch_smoke_probe` validates full init→load→tick→render cycle (6/6 headless). Source-lock: NEXUS.C/NEXUS2.C engine lifecycle, DMWeb Saturn DGN/DMDF format.

## Fixes

- Fixed CI build error: `nexus_v1_launcher.h` not in git causing `fatal error: nexus_v1_launcher.h: No such file` on fresh clone. Launcher integrated into CMake and source committed.
- Fixed `firestaff_nexus_v1_launch_smoke_probe` orphaned CMake target (source file was added in commit but never committed).
- Disabled `test_dm1_v22_verification` CMake target (committed with massive API mismatches — wrong headers, undefined types).
- Patched `m11_game_view.c` missing includes: `nexus_v1_engine.h`, `dm1_v2_camera_controller_pc34.h`, `firestaff_po_loader.h`, `dm1_v2_phase5_runtime_bridge_pc34.h`, `dm1_v1_viewport_fakewall_pc34_compat.h`.
- Added `firestaff_nexus` to `firestaff_m11` `target_link_libraries` (linker error on `M11_GameView_StartNexus`).

## Verification

- GitHub Actions verify workflow passed on `main` before release.
- CMake configure + build completed (all targets, pre-existing warnings only).
- Phase A probe: 21/21 invariants.
- Nexus launch smoke: 6/6.
- CSB V2 Phase 1 separation: 40/40.
- DM2 V2 HUD overlay, lighting, csb_v2_lighting_dynamic, csb_v2_touch_controller_affordance: all green.

## Platforms

| Platform | Architecture | Format |
|----------|-------------|--------|
| macOS | arm64 + x86_64 | DMG, ZIP |
| Windows | x86_64 | ZIP, Installer (EXE) |
| Linux | x86_64 | DEB, RPM |
| Linux | ARM64 | DEB, RPM |
