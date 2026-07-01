# Firestaff Completion Matrix

Last updated: 2026-06-30

This is a conservative 100-point parity-completion model. A point is counted only when the repo has source, runtime, or CI evidence for that criterion. `completionPercent` is therefore a verified evidence score, not optimism, effort spent, or pass count.

Public wording should stay honest: these numbers describe verified Firestaff evidence coverage, not a claim that every listed game is fully playable.

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
| DM1 V1 | 78% | 78/100 | `playable_verified_partial` | Representative original-vs-Firestaff overlay parity remains incomplete. Current DM1 blocker is the pass162 stock-original C080/F0377/F0275/F0280 live transcript gate: FIRES.MAP provides candidate addresses, DOSBox-debug break-start accepts the BP/BPM packet, and the HoC, movement-click, movement double-click, autolock, keyboard-capture, external `cliclick`, System Events, and direct `postToPid` live probes now record FIRES startup, DATA\DUNGEON.DAT load, mapped native macOS input, and 34 stock-original memory stops each, but no C080-chain code breakpoint fires. Pre-move, cursor-warp, first-click focus/capture, DOSBox-X autolock/mouse_emulation defaults, Ctrl-F10 capture toggling, replacing the Swift/CGEvent helper with `cliclick`, macOS Accessibility/System Events clicking, and direct CGEvent `postToPid` are now insufficient explanations. The mouse=debug logs show MOUSE/INT33 initialization but no route-click motion/button event, so next work is DOSBox-X SDL/Cocoa event ingestion or debugger event-pump work before pass435 six-shot promotion can continue. |
| DM1 V2 | 51% | 51/100 | `runtime_presentation_verified_partial` | V2.0/V2.1 runtime presentation smoke exists, but finished V2.2 real-art material/pixel gates and original-pairing evidence remain open. |
| CSB V1 | 43% | 43/100 | `runtime_boundary_verified_partial` | CSB has launch/profile, save, dungeon, command-chain, utility/import, and viewport source-slice gates, but end-to-end gameplay/render capture proof is still being hardened. |
| CSB V2 | 31% | 31/100 | `presentation_verified_partial` | CSB V2 HUD/filter/smooth-movement probes are wired, but real CSB V1 gameplay parity and modern-art per-cell material proof remain open. |
| DM2 V1 | 34% | 34/100 | `runtime_slices_verified_partial` | Door/button, projectile, creature, minimap, weather/timer, and save-adjacent slices exist; V1 dungeon/render/mechanics parity and real-asset runtime proof remain active work. |
| DM2 V2 | 16% | 16/100 | `presentation_scaffold_partial` | V2 presentation scaffolds exist, but V1 parity base and finished V2 proof are not complete. |
| DM Nexus V1 | 37% | 37/100 | `runtime_slices_verified_partial` | DMDF/DGN/data, BPK/BPX, font, save, and mechanics slices exist; real-asset end-to-end handoff and screen capture proof remain active work. |
| DM Nexus V2 | 20% | 20/100 | `presentation_scaffold_partial` | V2 lighting/touch/upscale/presentation probes exist, but real V1 runtime coverage and V2 material proof remain incomplete. |
| Theron V1 | 39% | 39/100 | `track02_slices_verified_partial` | Track 02 provenance, parser, save/progression/shop/mechanics slices exist; positive real-asset launch through Track 02 and broader runtime capture remain active work. |

## Recent evidence incorporated

This refresh incorporates the post-2026-05-09 work now present on `origin/main`, including:

| Area | Evidence now counted |
|---|---|
| DM1 V1 | Hall of Champions mirror/portrait runtime probes across many ordinals and routes; resurrect survival/load regression; chest runtime matrix; food/water warning tick; door-bash sound/no-open; projectile grate/pass-through; spell-rune repeat input; champion-panel disabled icon and mana repaint gates; movement/data-root CTest hardening. |
| DM1 V2 | Runtime presentation smoke for V2.0/V2.1; source-route and side-by-side seed gates; HUD overlay/interaction gates; lighting/effects/field projectile VFX gates; selected-resolution and 4K input mapping; per-mode material-signature and renderer readback probes. |
| CSB V1 | Hash-matched launch intent boundary; CSB save runtime boundary; command-chain/input/runtime tick slices; dungeon header/loader/world gates; utility/CMP import; DSA trigger; save import path; viewport source-lock slices and CustomBackgrounds coverage; hidden-code skip tables. |
| CSB V2 | HUD overlay probe, smooth movement/runtime binding, phase-gate verification, filter config/chain/dispatch, texture upscale, V2.2 shape probes, and CTest registration of previously build-only probes. |
| DM2 V1 | Door/button toggle boundary; minimap level-transition/save-load gate; weather/timer save round-trip; projectile-vs-creature collision; creature death/drop observer; pressure plate and trigger gates; focused combat/shop probes. |
| Nexus V1 | DMDF palette/string bounds, DGN malformed actor guard, BPK/BPX archive boundaries, Saturn font parse/render determinism, save-slot round-trip, mechanics parity slices, and renderer/data loader probes. |
| Theron V1 | Track 02 bank/object marker, save/progress round-trip, shop purchase edges, launcher scan reuse, mechanics/champion/progression probes, and runtime screenshot-readiness manifests. |
| Cross-cutting | Nested ZIP deflate cache materialization, ISO/BIN case-insensitive duplicate-hash scanning, M12 no-data/data-dir cache gates, 19-language launcher and game-domain l10n layout validation, and Tier 1 strict boot coverage. |

## Detail

### DM1 V1

| Criterion | Score | Evidence note |
|---|---:|---|
| `reference_inventory` | 10/10 | DM1 PC 3.4 anchors, ReDMCSB source, Greatstone references, and local evidence manifests remain source-locked. |
| `definition_matrix` | 10/10 | `docs/parity/PARITY_MATRIX_DM1_V1.md`, DM1 V1 movement/completion evidence, and many pass manifests exist. |
| `launch_smoke` | 9/10 | DM1 V1 launch/menu/title/runtime paths are repeatedly CTest- and probe-gated; release data defaults were restored after the v3.0.3 path fix. |
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
| `core_input_movement` | 2/15 | Some minimap/transition and runtime scaffolds exist, but core movement parity remains incomplete. |
| `viewport_ui_render` | 3/20 | HUD/minimap/presentation-adjacent gates exist; V1 renderer parity remains open. |
| `gameplay_systems` | 12/15 | Door/button, pressure/trigger, projectile, creature collision/death/drop, weather/timer save, world-state/minimap, shop/combat slices, and save-adjacent gates are covered. |
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
| `definition_matrix` | 3/10 | No complete Theron V1 parity matrix, but runtime/progression/save gates now define important slices. |
| `launch_smoke` | 5/10 | Track 02 bank/object marker, launcher scan reuse, and runtime screenshot-readiness receipts exist; positive real-asset launch remains active work. |
| `core_input_movement` | 2/15 | Dungeon progression/mechanics probes exist; broad input/movement parity is not complete. |
| `viewport_ui_render` | 5/20 | Tile/viewport/runtime screenshot-readiness and presentation scaffolds exist, but real screen parity remains open. |
| `gameplay_systems` | 14/15 | Save/load progress round-trip, shop purchase edges, dungeon progression, mechanics, champion, object-marker, and Track 02 parser slices are covered. |
| `audio_timing` | 2/10 | Track 02 bank evidence exists, but audio/timing parity is sparse. |
| `original_overlay_regression` | 0/10 | No representative original overlay regression proof for Theron. |
