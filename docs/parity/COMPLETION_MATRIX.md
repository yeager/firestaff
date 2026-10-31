# Firestaff Completion Matrix

Last updated: 2026-10-31

This is a conservative 100-point parity-completion model. A point is counted only when the repo has source, runtime, or CI evidence for that criterion. `completionPercent` is therefore a verified evidence score, not optimism, effort spent, or pass count.

Public wording should stay honest: these numbers describe verified Firestaff evidence coverage, not a claim that every listed game is fully playable.

## Current roll-up

The matrix currently contains **9 runtime targets** and **41 recognised
version profiles**. A version profile is an asset-admission identity; it is
not an independent parity implementation. The percentages below therefore
remain target-level evidence scores. They must not be multiplied by the
number of hashes or presented as 41 separately verified ports.

| Game family | Runtime targets | Version profiles | Mean runtime evidence | Version-profile scope |
|---|---:|---:|---:|---|
| DM1 | 2 | 16 | 64.5% | PC 3.4 is the source-locked playable route; other profiles are catalogued/admitted where their data gates pass. |
| CSB | 2 | 9 | 37.0% | Shared CSB runtime slices; edition-specific launch/data boundaries are not full parity proof. |
| DM2 | 2 | 7 | 26.0% | PC/FM Towns/Amiga/PC-98 profiles are recognised; authentic DOS, Amiga, FM Towns and Mac V1 start paths are verified, but live parity remains open. |
| DM Nexus | 2 | 5 | 28.5% | Saturn JP/extracted, V2, English fan and French fan profiles share the capture-gated Nexus route. |
| Theron's Quest | 1 | 4 | 72.0% | Native JP fresh start reaches source-verified Ak-Tu-Ba level 0; authentic US Track 02 plus matching BRAM reaches dungeon 2 through Continue with bounded movement/item transactions. All seven dungeon source records load per region. Matched JP state-only runs show Button II+UP-conditioned `$C1FC` TII transfers, but the same instruction is associated with teleporter destination copying and the repeated context does not establish movement, active-map object or field semantics. The authenticated US Drator event follows generated-row construction through publication, reads and unlink. A rebuilt cold replay reached `left-event` and captured two `$C852` windows at physical `$0DC852`, but their bytes differ from the prior static attribution; the filtered row observer hit its sample cap before reentry. Callback-state verification passed, while I/O/RAM verification exhausted memory on the 8,388,608-row trace. Thus the same-session consumer join remains open. US saved-state short-format `$44CA/$44DB` rows remain ambiguous and snapshots are unjoined. Retail-frame parity, cold game-owned handoff, broader campaign, combat and save semantics remain open. See [current capture readiness](../theron/THERON_CAPTURE_READINESS.md). |
| **Total** | **9** | **41** | **42.7%** | Unweighted mean of the 9 runtime targets: 384/900. |

The family and total means are derived from the nine target scores in the
machine-readable matrix. Version profiles are not independent parity targets,
so their count is not used to weight the score.

The latest authentic Nexus Saturn captures remain evidence additions, not a
score promotion: VDP1 command-to-DGN pixel/CLUT joins are verified for selected
frames, while menu/FONT256 ownership, complete VDP1 scene ordering, HUD/VDP2
composition and SLEV/SAL/SDDRVS playback remain blocked.

## Scoring criteria

| Criterion | Weight | Meaning |
|---|---:|---|
| `reference_inventory` | 10 | Original/source reference inventory exists and is source-locked for this target. |
| `definition_matrix` | 10 | Target-specific parity/completion matrix exists with clear acceptance labels. |
| `launch_smoke` | 10 | Target launches, blocks correctly, or renders through a repeatable repo gate/smoke path. |
| `core_input_movement` | 15 | Core input, movement/camera, command routing, and first viewport redraw path are source/runtime-gated. |
| `viewport_ui_render` | 20 | Viewport, HUD, panels, title/menu, palette, graphics decode, and UI render surfaces are source/runtime-gated. |
| `gameplay_systems` | 15 | Combat, creatures, items, spells, sensors, projectiles, save/load, lifecycle, and dungeon systems are original/source-gated. |
| `audio_timing` | 10 | Audio content, trigger cadence, animation/timing, input responsiveness, and music/SFX overlap are original/source-gated. |
| `original_overlay_regression` | 10 | Original runtime captures/overlays/regression gates prove pixel/behavior parity for representative states. |

## Current matrix

| Target | completionPercent | Points | Status | Primary blockers |
|---|---:|---:|---|---|
| DM1 V1 | 78% | 78/100 | `playable_verified_partial` | Representative original-vs-Firestaff overlay parity remains incomplete. Native archive startup is now separately proven for both the canonical PC 3.4 ZIP and the supplied nested DOS-EN layout; the missing scope is original runtime overlay/capture evidence, not archive ownership. |
| DM1 V2 | 51% | 51/100 | `runtime_presentation_verified_partial` | V2.0/V2.1 runtime presentation smoke exists, but finished V2.2 real-art material/pixel gates and original-pairing evidence remain open. |
| CSB V1 | 43% | 43/100 | `runtime_boundary_verified_partial` | CSB has authentic native startup/runtime and start-menu evidence on Atari ST, Amiga and FM Towns. Atari, Amiga and both FM Towns editions verify their complete initial input matrices from independent source sessions. The French preservation ZIP verifies its original protected STX sector ordering through the same route. End-to-end gameplay/render capture proof is still being hardened. |
| CSB V2 | 31% | 31/100 | `presentation_verified_partial` | CSB V2 HUD/filter/smooth-movement probes are wired, but real CSB V1 gameplay parity and modern-art per-cell material proof remain open. |
| DM2 V1 | 36% | 36/100 | `runtime_slices_verified_partial` | Authentic DOS, Amiga, FM Towns and Mac startup/runtime evidence joins door/button, projectile, creature, minimap, weather/timer, save-adjacent slices, combat damage pipeline, and movement execution chain. V1 dungeon/render/mechanics parity remains active work. |
| DM2 V2 | 16% | 16/100 | `presentation_scaffold_partial` | V2 presentation scaffolds exist, but V1 parity base and finished V2 proof are not complete. |
| DM Nexus V1 | 37% | 37/100 | `runtime_slices_verified_partial` | DMDF/DGN/data, BPK/BPX, font, save, and mechanics slices exist; real-asset end-to-end handoff and screen capture proof remain active work. |
| DM Nexus V2 | 20% | 20/100 | `presentation_scaffold_partial` | V2 lighting/touch/upscale/presentation probes exist, but real V1 runtime coverage and V2 material proof remain incomplete. |
| Theron V1 | 72% | 72/100 | `hw_config_cd_play_vdc_joypad_proven` | Native JP fresh-start and US BRAM-Continue routes are verified. Native SDL-loop regressions cover bounded movement, champion selection, wait and US pickup/reselect/drop; USE effects and positive JP progress restoration remain unverified. Matched JP state-only TII transfers and their 128-instruction context do not prove movement: `$C1FC` is also associated with teleporter destination copying, while map and field semantics remain unknown. Original-versus-Firestaff frame regression (0/10), dynamic source-bound viewport, broader gameplay and original save/audio semantics remain open. |

## Recent evidence incorporated

This refresh incorporates the post-2026-05-09 work now present on `origin/main`, including:

| Area | Evidence now counted |
|---|---|
| DM1 V1 | Hall of Champions mirror/portrait runtime probes across many ordinals and routes; resurrect survival/load regression; chest runtime matrix; food/water warning tick; door-bash sound/no-open; projectile grate/pass-through; spell-rune repeat input; champion-panel disabled icon and mana repaint gates; movement/data-root CTest hardening. |
| DM1 V2 | Runtime presentation smoke for V2.0/V2.1; source-route and side-by-side seed gates; HUD overlay/interaction gates; lighting/effects/field projectile VFX gates; selected-resolution and 4K input mapping; per-mode material-signature and renderer readback probes. |
| CSB V1 | Authentic native Atari STX and nested-ZIP, including the supplied French preservation ZIP → protected STX route, Amiga ADF-archive, and English/Japanese FM Towns-CD startup paths now prove title → runtime and start-menu handoff. Atari, Amiga and FM Towns independently cover their complete initial input matrices. Additional evidence includes CSB save runtime boundary; command-chain/input/runtime tick slices; dungeon header/loader/world gates; utility/CMP import; DSA trigger; save import path; viewport source-lock slices and CustomBackgrounds coverage; hidden-code skip tables. |
| CSB V2 | HUD overlay probe, smooth movement/runtime binding, phase-gate verification, filter config/chain/dispatch, texture upscale, V2.2 shape probes, and CTest registration of previously build-only probes. |
| DM2 V1 | Authentic DOS MVE → SKULL → New Game CLI, Amiga title/runtime, FM Towns title/New Game/champion/runtime, and Mac retail boot; the separately supplied Mac First Chapter demo is fail-closed and cannot select a retail sibling; door/button toggle boundary; minimap level-transition/save-load gate; weather/timer save round-trip; projectile-vs-creature collision; creature death/drop observer; pressure plate and trigger gates; focused combat/shop probes. |
| Nexus V1 | DMDF palette/string bounds, DGN malformed actor guard, BPK/BPX archive boundaries, Saturn font parse/render determinism, save-slot round-trip, mechanics parity slices, and renderer/data loader probes. |
| Theron V1 | Authentic JP fresh-start and US BRAM-Continue integration tests on macOS Debug; all-seven source dungeon loaders per region; Track 02 bank/object marker, save/progress round-trip, mechanics/champion/progression probes, runtime screenshot-readiness manifests, authentic US keyboard/mouse movement and pickup/reselect/drop, native rejection of empty JP BRAM, and matched JP state-only UP/control traces. The observed TII overlaps a source-locked teleporter destination path, so movement semantics remain unresolved. The renderer and original-game consumer gaps remain open. |
| Cross-cutting | Nested ZIP deflate cache materialization, ISO/BIN case-insensitive duplicate-hash scanning, M12 no-data/data-dir cache gates, 19-language launcher and game-domain l10n layout validation, and Tier 1 strict boot coverage. |

## Detail

### DM1 V1

| Criterion | Score | Evidence note |
|---|---:|---|
| `reference_inventory` | 10/10 | DM1 PC 3.4 anchors, ReDMCSB source, Greatstone references, and local evidence manifests remain source-locked. |
| `definition_matrix` | 10/10 | `docs/parity/PARITY_MATRIX_DM1_V1.md`, DM1 V1 movement/completion evidence, and many pass manifests exist. |
| `launch_smoke` | 9/10 | DM1 V1 launch/menu/title/runtime paths are repeatedly CTest- and probe-gated. The real-media matrix passes direct CLI and the start-menu route for both DOS PC 3.4 ZIP layouts, Atari ST English/German, Amiga and FM Towns; the canonical PC 3.4 ZIP, English Atari ST v1.2 archive, Amiga HD archive and FM Towns route independently verify their complete initial input matrices. French DOS unpacked media retains its separate native route. Archived media is consumed in memory and requires neither an emulator nor a BIOS. |
| `core_input_movement` | 15/15 | Input command queue, keypad route, movement timing/legal-target gates, touch/mouse queue routes, stairs transitions, and cooldown/F0380 ordering are source/runtime-gated. |
| `viewport_ui_render` | 17/20 | Viewport wall/door/ornament/side-content source slices, champion panel/HUD/status boxes, Hall mirror frame/portrait probes, chest UI, and title/swoosh gates are broad; full original pixel-pair coverage is still incomplete. |
| `gameplay_systems` | 14/15 | Combat, creature AI/grouping, inventory/chest, champion lifecycle, spells, projectiles, sensors, save/load, resurrection, food/water, and many regression gates are covered. Remaining point is reserved for broader real-runtime parity depth. |
| `audio_timing` | 3/10 | Audio event mapping and some sound-specific gates exist, including door-bash/no-open coverage; cadence/overlap/original timing proof remains limited. |
| `original_overlay_regression` | 0/10 | Original overlay/capture work has partial artifacts and exact wall matches, but representative movement/HUD/viewport parity is not yet promoted. |

### DM1 V2

| Criterion | Score | Evidence note |
|---|---:|---|
| `reference_inventory` | 8/10 | DM1 source/original inventory applies; V2 remains presentation over V1 source semantics. |
| `definition_matrix` | 7/10 | V2 completion/status artifacts, phase gates, and CTest-backed source-isolation exist. |
| `launch_smoke` | 8/10 | `dm1_v2_runtime_presentation_smoke` runs real `firestaff --game dm1` paths for V2.0/V2.1 and verifies distinct presented frame receipts. |
| `core_input_movement` | 8/15 | Movement/camera, selected-resolution input mapping, 4K mapping, and phase-gated command adapters are covered. |
| `viewport_ui_render` | 15/20 | HUD overlay, item/render source locks, lighting/effects/VFX, viewport materials, per-mode signatures, and GPU/readback probes exist; finished V2.2 real-art gates remain open. |
| `gameplay_systems` | 5/15 | V2 preserves V1 gameplay ownership through phase gates, but V2-specific gameplay proof is intentionally limited. |
| `audio_timing` | 0/10 | No complete V2 audio/timing parity gate. |
| `original_overlay_regression` | 0/10 | No representative original-overlay regression proof for V2. |

### CSB V1

| Criterion | Score | Evidence note |
|---|---:|---|
| `reference_inventory` | 9/10 | CSB PC/Atari/Amiga references, source-lock manifests, archive/version inventories, and asset-pair manifests exist. |
| `definition_matrix` | 10/10 | `docs/parity/PARITY_MATRIX_CSB_V1.md`, completion matrix, and parity surface matrix define the CSB V1 evidence boundary. |
| `launch_smoke` | 6/10 | Hash-matched launch intent, blocked-launch UI, quickplay/load-route source locks, PC real-asset launch boundary, and CSB profile/diagnostic gates exist. |
| `core_input_movement` | 5/15 | Command-chain, input-queue, movement step/rotation, runtime tick, queue overflow, reincarnation, and related gates exist, but end-to-end runtime proof is not complete. |
| `viewport_ui_render` | 7/20 | CSB-specific viewport source slices, CustomBackgrounds, D1/D2/D3 wall/door/ornament lanes, hidden item skip tables, and portrait render handoff are covered; real pixel captures remain open. |
| `gameplay_systems` | 4/15 | Dungeon loader/world, DSA trigger, save runtime boundary, utility/CMP import, Grey Lord/Zokathra/chaos slices exist; broad mechanics parity remains open. |
| `audio_timing` | 2/10 | AMG Utility Disk sound decoding is fixed for documented SND2 files; runtime playback/rate binding and timing proof remain open. |
| `original_overlay_regression` | 0/10 | No CSB V1 representative original overlay regression proof. |

### CSB V2

| Criterion | Score | Evidence note |
|---|---:|---|
| `reference_inventory` | 5/10 | CSB V2 inherits CSB references but remains presentation-layer evidence. |
| `definition_matrix` | 4/10 | V2 phase-gate and verification-suite tests exist, but no full CSB V2 completion matrix. |
| `launch_smoke` | 4/10 | Presentation-mode and settings probes are wired. |
| `core_input_movement` | 6/15 | Smooth movement, smooth runtime binding, and phase verification are CTest-gated. |
| `viewport_ui_render` | 10/20 | HUD overlay, filter config/chain/dispatch, texture upscale, and V2.2 shape probes are wired and passing. |
| `gameplay_systems` | 2/15 | V2 keeps source gameplay boundaries but does not add broad gameplay parity. |
| `audio_timing` | 0/10 | No CSB V2 audio/timing parity gate. |
| `original_overlay_regression` | 0/10 | No CSB V2 original overlay regression proof. |

### DM2 V1

| Criterion | Score | Evidence note |
|---|---:|---|
| `reference_inventory` | 8/10 | DM2 PC/French/German/Mac data inventories and SKULL.ASM/source references exist; no ReDMCSB C equivalent. |
| `definition_matrix` | 4/10 | DM2 V1 does not yet have a complete parity matrix, but multiple scoped parity gates now define runtime boundaries. |
| `launch_smoke` | 4/10 | Asset scanner/profile coverage and required-file launch gating exist; full runtime launch proof is still active work. |
| `core_input_movement` | 3/15 | Movement plan/exec chain with door attack, creature push/classification, stair level change, and teleporter resolution implemented; core runtime wiring remains incomplete. |
| `viewport_ui_render` | 3/20 | HUD/minimap/presentation-adjacent gates exist; V1 renderer parity remains open. |
| `gameplay_systems` | 13/15 | Door/button, pressure/trigger, projectile, creature collision/death/drop, weather/timer save, world-state/minimap, shop/combat slices, save-adjacent gates, and skproject combat damage pipeline (CALC_PLAYER_ATTACK_DAMAGE, WOUND_PLAYER, ATTACK_PARTY) are covered. |
| `audio_timing` | 1/10 | Limited timing/weather seed coverage; no broad DM2 audio/timing parity proof. |
| `original_overlay_regression` | 0/10 | No DM2 V1 representative original overlay regression proof. |

### DM2 V2

| Criterion | Score | Evidence note |
|---|---:|---|
| `reference_inventory` | 3/10 | DM2 V2 inherits DM2 references but remains presentation-layer scaffolding. |
| `definition_matrix` | 2/10 | V2 probes exist, but no full DM2 V2 completion matrix. |
| `launch_smoke` | 2/10 | Some presentation/shape/in-place probes exist. |
| `core_input_movement` | 1/15 | V2 movement evidence is minimal. |
| `viewport_ui_render` | 7/20 | DM2 V2 HUD/in-place/modern assets/shape-related probes exist, but finished rendering proof is sparse. |
| `gameplay_systems` | 1/15 | V2 inherits V1 boundaries; no broad V2 gameplay proof. |
| `audio_timing` | 0/10 | No DM2 V2 audio/timing parity gate. |
| `original_overlay_regression` | 0/10 | No DM2 V2 original overlay regression proof. |

### DM Nexus V1

| Criterion | Score | Evidence note |
|---|---:|---|
| `reference_inventory` | 8/10 | Saturn DMDF/DGN/S2D/MNS/BPK/BPX inventories and hash/profile references exist. |
| `definition_matrix` | 4/10 | No full Nexus V1 parity matrix, but parser/runtime gate coverage is now substantial. |
| `launch_smoke` | 4/10 | Profile/asset scanner and launcher boundaries exist; real-asset end-to-end handoff remains active. |
| `core_input_movement` | 2/15 | World/save/mechanics slices exist, but movement/input parity is sparse. |
| `viewport_ui_render` | 10/20 | Saturn font parser/render, BPK/BPX archive parsing, DMDF palette/string bounds, DGN malformed actor guard, and renderer probes exist. |
| `gameplay_systems` | 9/15 | Save slot round-trip, mechanics parity, creature/world/dungeon data slices, and loader guards exist. |
| `audio_timing` | 0/10 | Nexus audio/timing parity gates remain open. |
| `original_overlay_regression` | 0/10 | No Nexus representative original overlay regression proof. |

### DM Nexus V2

| Criterion | Score | Evidence note |
|---|---:|---|
| `reference_inventory` | 4/10 | V2 inherits Nexus references. |
| `definition_matrix` | 2/10 | V2 probe coverage exists, but no full completion matrix. |
| `launch_smoke` | 2/10 | Presentation probes build/run in CI. |
| `core_input_movement` | 2/15 | Touch/smooth movement scaffolds exist, but movement parity is not complete. |
| `viewport_ui_render` | 9/20 | Lighting, atmosphere, upscaler, texture/modern asset, and in-place draw probes exist. |
| `gameplay_systems` | 1/15 | V2 gameplay proof is intentionally limited. |
| `audio_timing` | 0/10 | No Nexus V2 audio/timing parity gate. |
| `original_overlay_regression` | 0/10 | No Nexus V2 original overlay regression proof. |

### Theron V1

| Criterion | Score | Evidence note |
|---|---:|---|
| `reference_inventory` | 8/10 | Track 02 JP/US provenance, launcher scan reuse, and hash/profile evidence exist. |
| `definition_matrix` | 7/10 | Comprehensive Theron V1 parity definition matrix with 10 categories, 40+ gates, proven/open status, and bound-byte totals. |
| `launch_smoke` | 7/10 | Authentic native JP fresh start and US BRAM Continue routes are integration-tested; all-seven source loaders pass. The independent System Card/BIOS cold-reference handoff remains unjoined to game-owned code, so the native route evidence does not establish the original cold-boot chain. |
| `core_input_movement` | 13/15 | Static disassembly classifies all five controller button groups and the bounded command handlers. Native Firestaff integration tests verify bounded input transactions on authentic US/JP data; original-game action semantics and a game-owned cold poll remain unproven. |
| `viewport_ui_render` | 16/20 | Static disassembly records VDC/VCE register use and a five-tier original rendering pipeline (2,310 bytes bound). This is reference-code evidence only: it does not prove Firestaff's dynamic viewport, retail graphics mapping, or paired pixel parity. |
| `gameplay_systems` | 14/15 | Authentic US BRAM Continue restores a bounded campaign slice; host-level save/progression and Track 02 parser tests cover additional slices. Original retail save mutation, item/quest semantics, combat and broader campaign consumers remain open. |
| `audio_timing` | 7/10 | CD_PLAY track parameter fully mapped: two code-region sites both load track $0E via LDA #$0E → STA $FF; four data false positives filtered. System card catalog: 279 JSR sites to 17 BIOS vectors. |
| `original_overlay_regression` | 0/10 | No representative original overlay regression proof for Theron. |
