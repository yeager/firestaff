# DM1 V1 All In-Game Graphics Fix Plan

Date: 2026-04-25
Scope: Firestaff **V1 / DM1 PC 3.4 in-game runtime graphics only**.

This plan is deliberately evidence-first.  The current in-game screen is not ready to be polished by taste: every visible region must be locked against original DM1 / ReDMCSB / `GRAPHICS.DAT` evidence, then verified by runtime screenshots.

## Ground rules

1. **Do not overwrite current local work.**  At plan time the worktree already has local edits in:
   - `asset_loader_m11.c` — odd-width / padded-nibble decode and asset tracing work in progress.
   - `m11_game_view.c` — debug-text suppression and V1 message-surface changes in progress.
   - `probes/m11/firestaff_m11_game_view_probe.c` — probe updates in progress.
   - `audio_sdl_m11.c` — unrelated local edit; do not touch for graphics work unless explicitly scoped.
2. **V1 and V2 are separate.**  V1 must use original DM1 PC 3.4 assets, geometry, palette and scaling.  No `assets-v2`, `v2-assets`, 4K/upscaled comparison material, or generated redesigns may be used as V1 truth.
3. **Firestaff exports are raw evidence, not truth by themselves.**  Greatstone/SCK locks index/name/color presentation; ReDMCSB locks code ownership/usage; original DOSBox/native captures lock full-screen runtime composition.
4. **No “looks close” acceptance.**  Each region needs a source-bound expected rectangle/assets list and a runtime screenshot/probe proving it.
5. **Commit granularity target:** every phase below should be small enough to commit independently after its probes/screenshots pass.  This plan does not create commits.

## Current visual problem inventory by screen region

### 1. Whole screen / chrome

Observed / reported problems:

- Normal V1 in-game view still reads as a prototype when debug/helper surfaces leak.
- Previous screenshots contained text such as `G GRAB P DROP`, `GOOD`, `TICK 0`, `MENU BUTTON`, `SAVE DATA`, explicit status cards, and telemetry-like logs.
- Current `m11_game_view.c` local diff moves several of these behind `state->showDebugHUD`, but this is not yet a complete source-bound audit.
- The screen is still built from procedural boxes in places where original DM1 zones/assets should own the composition.

Evidence to use:

- `M11_V1_SCREENSHOT_GAP_SPEC.md` G1/G2/P1.
- ReDMCSB `DEFS.H` zone constants:
  - `C002_ZONE_SCREEN`
  - `C007_ZONE_VIEWPORT`
  - `C011_ZONE_ACTION_AREA`
  - `C013_ZONE_SPELL_AREA`
  - `C015_ZONE_MESSAGE_AREA`
  - `C101_ZONE_PANEL`
  - `C151..C218` champion status/name/hand/slot zones
- Original DM1 runtime screenshots captured through DOSBox/native capture, not PGM-only grayscale output.
- Greatstone DM PC 3.4 `graphics.dat` page for source asset identity.

### 2. Viewport / dungeon scene

Observed / likely problems:

- Viewport rectangle is partially source-anchored from pass 40, but the actual drawn wall/floor/ceiling/depth composition still needs a fresh “all graphics” audit.
- Current Firestaff references may have floor/ceiling semantics reversed for `0078`/`0079` according to `GRAPHICS_DAT_EXPORT_MAPPING_AUDIT.md`.
- Procedural dungeon rendering and helper overlays may still diverge from original `DRAWVIEW` ownership.
- Lighting/palette level may be correct at the table level but not proven through full RGB runtime screenshots for multiple depths/light amounts.

Evidence to use:

- ReDMCSB `DEFS.H` viewport constants and visible-square diagrams:
  - view lanes/squares/cells
  - view walls `M575..M587`
  - view floors `M588..M596`
  - door ornaments/buttons and shift sets
- ReDMCSB `DRAWVIEW.C` / `GRAPH*.C` if recovered from `ReDMCSB_WIP20210206.7z`; if not unpacked, fetch/extract local archive before coding this phase.
- Existing pass 40 viewport evidence and `parity-evidence/pass40_viewport_lock.md` if present.
- `GRAPHICS_DAT_EXPORT_MAPPING_AUDIT.md` for `0078`/`0079` reversal suspicion.
- DOSBox/native 320x200 captures at known dungeon positions/directions.

### 3. Right-side champion/status column

Observed / likely problems:

- Champion status presentation has had passes for stride/bar graphs, but full original DM1 panel composition is not locked yet.
- `0007`/`0008` were previously mislabeled as left/right frames; true identity is alive/dead champion status box family, with `0007` noted by ReDMCSB as “never used” in original code.
- Champion name/hand/status zones need to match `C151..C218`, not Firestaff-invented rows.
- Damage indicators, poison/shield state, food/water, active champion highlights and hand boxes need a region-by-region audit.

Evidence to use:

- ReDMCSB zones:
  - `C151..C154` champion status-box name/hands
  - `C159..C162` champion names
  - `C175..C178` status boxes
  - `C187..C190` bar graph zones
  - `C195..` first bar graph
  - `C211..C218` ready/action hand slot boxes
- `DEFS.H` graphic names:
  - `C007_GRAPHIC_STATUS_BOX`
  - `C008_GRAPHIC_STATUS_BOX_DEAD_CHAMPION`
  - `C033_GRAPHIC_SLOT_BOX_NORMAL`
  - `C034_GRAPHIC_SLOT_BOX_WOUNDED`
  - `C035_GRAPHIC_SLOT_BOX_ACTING_HAND`
- Existing evidence: `verification-m11/pass41-status-box-stride/`, `verification-m11/pass43-bar-graphs/`.
- Greatstone `graphics.dat` asset identity for status/slot families.

### 4. Action area / PASS strip

Observed / reported problem:

- User specifically reported distorted PASS/action-area asset.
- `C010_GRAPHIC_MENU_ACTION_AREA` is `87x45`, odd width.  Current local diff in `asset_loader_m11.c` addresses padded nibble row stride; this must be verified, not assumed.
- Action menu zones are probably not drawn from the exact original zone set yet.

Evidence to use:

- `GRAPHICS_DAT_EXPORT_MAPPING_AUDIT.md`: `0010` confirmed action-area background, dimensions `87x45`.
- ReDMCSB `DEFS.H`:
  - `C010_GRAPHIC_MENU_ACTION_AREA`
  - `C011_ZONE_ACTION_AREA`
  - `C075_ZONE_ACTION_RESULT`
  - `C077_ZONE_ACTION_AREA_TWO_ACTIONS_MENU`
  - `C079_ZONE_ACTION_AREA_ONE_ACTION_MENU`
  - `C082..C084_ZONE_ACTION_AREA_ACTION_*`
  - `C089..C092_ZONE_ACTION_AREA_CHAMPION_*_ACTION`
  - `C093_ZONE_ACTION_AREA_CHAMPION_0_ACTION_ICON`
  - `C098_ZONE_ACTION_AREA_PASS`
- Greatstone/SCK rendered color reference for graphic 10.
- Local `extracted-graphics-v1/pgm/0010*` and color export only after palette verification.

### 5. Spell/rune area

Observed / likely problems:

- Current spell panel is procedural and overlay-like; it uses C011 label cells in places but is not yet proven to match original DM1 spell-area zones.
- Helper-heavy labels such as `POWER`, `ELEMENT`, `FORM`, `CLASS`, mana `current/max`, dim future rows, and `CAST` display need validation against original DM1.  If not original, they should not be permanent V1 chrome.
- `C009_GRAPHIC_MENU_SPELL_AREA_BACKGROUND` is source-identified; usage needs exact runtime binding.

Evidence to use:

- `GRAPHICS_DAT_EXPORT_MAPPING_AUDIT.md`: `0009` confirmed spell-area background, `87x25`.
- ReDMCSB `DEFS.H`:
  - `C009_GRAPHIC_MENU_SPELL_AREA_BACKGROUND`
  - `C013_ZONE_SPELL_AREA`
  - `C221_ZONE_SPELL_AREA_SET_MAGIC_CASTER`
  - `C224_ZONE_SPELL_AREA_MAGIC_CASTER_TAB`
  - `C245..C250_ZONE_SPELL_AREA_SYMBOL_*`
  - `C252_ZONE_SPELL_AREA_CAST_SPELL`
  - `C254_ZONE_SPELL_AREA_RECANT_SYMBOL`
  - `C255..C260_ZONE_SPELL_AREA_AVAILABLE_SYMBOL_*`
  - `C261..` champion symbol zones
- Existing pass 44 spell label evidence in `verification-m11/pass44-spell-labels/`.
- Original DM1 capture of spell panel open, rune selection, cast-ready, and recant states.

### 6. Bottom message / text surfaces

Observed / reported problem:

- “Massor av debug text” must be gone from normal V1.
- Current local diff intentionally stops rendering rolling event/debug text in normal V1, but the rule must become a source-bound whitelist, not a blanket accidental silence.
- DM1 does have legitimate text surfaces: message area, wall-text/plaque/dialog, action result, champion/object names.  These must remain if source-bound.

Evidence to use:

- ReDMCSB zones:
  - `C015_ZONE_MESSAGE_AREA`
  - `C017_ZONE_LEADER_HAND_OBJECT_NAME`
  - `C075_ZONE_ACTION_RESULT`
  - `C506_ZONE_OBJECT_DESCRIPTION`
  - dialog zones `C450..C471`
- DM1 text functions in ReDMCSB (`TEXT.C`/message log surfaces if recovered).
- Original captures for movement blocked, object name, wall plaque, action result, spell failure/success, champion damage.

### 7. Inventory / panel overlays

Observed / likely problems:

- Inventory still risks reading as a slot inspector rather than original DM equipment interaction.
- `C020_GRAPHIC_PANEL_EMPTY` is a known panel/inventory background candidate, but exact usage must be verified.
- Slot boxes must distinguish normal/wounded/acting hand; `0034` must not be used as generic highlight.

Evidence to use:

- `C020_GRAPHIC_PANEL_EMPTY` / index `0020` (`144x73`).
- ReDMCSB zones:
  - `C101_ZONE_PANEL`
  - `C500..C506` food/water/poison/object description
  - `C507..C542` inventory/chest slot boxes
- `C033..C035` slot graphics.
- Greatstone/SCK inventory panel reference and original runtime inventory capture.

### 8. Items, creatures, projectiles, explosions, overlays

Observed / likely problems:

- Existing probes show coverage for side ornaments/items/creatures/projectiles/explosions, but that is not the same as pixel parity.
- Creature family sprites, scaling, z-order, half-square placement, animation frame choice, and attack/damage overlays are not fully audited.
- Current attack cue has procedural diagonal slash marks; if not original DM1, it must be removed or replaced with original asset/effect behavior.
- Projectile/explosion graphics need exact source indices, palette level behavior, and depth placement.

Evidence to use:

- ReDMCSB view cells and half-square creature cells in `DEFS.H`.
- ReDMCSB creature/projectile/explosion draw paths (`DRAWVIEW.C`, creature/thing scan order; recover if missing).
- Existing probe outputs:
  - `verification-m11/game-view/15_side_ornament_item_creature_count_fidelity.pgm`
  - `16_projectile_facing_creature_attack_ornament.pgm`
  - `17_floor_ornament_creature_aspect.pgm`
  - `21_projectile_in_flight.pgm`
  - `22_explosion_after_advance*.pgm`
- Original captures with controlled states, generated from real DM1 runtime or ReDMCSB-compatible native run, not hallucinated/AI-generated images.

### 9. Palette and RGB output

Observed / likely problems:

- `vga_palette_pc34_compat.c` contains source-backed PC 3.4 VGA palette and brightness tables, and pass 46/68 probes are green.
- Still, screenshots used for visual acceptance must be RGB indexed through this palette, not grayscale `.pgm` inspection.
- Full-screen path must prove base dungeon palette vs special entrance/credits palettes do not leak or get applied to normal dungeon UI.

Evidence to use:

- `vga_palette_pc34_compat.c`:
  - `G9010_auc_VgaPaletteAll_Compat[6][16][3]`
  - `G9011_auc_VgaPaletteCredits_Compat`
  - `G9012_auc_VgaPaletteEntrance_Compat`
- Probe logs:
  - `verification-m11/pass46-vga-palette/pass46_vga_palette_probe.log` — 7/7 PASS.
  - `verification-m11/pass68-special-palettes/pass68_special_palette_probe.log` — 6/6 PASS.
- Original DOSBox/native RGB screenshots for dungeon at multiple light levels.

## Execution phases

Each phase should finish with a small diff, dedicated evidence note, probe output, screenshot set, and no V2 asset contamination.

### Phase 0 — Freeze evidence and preserve local work

Goal: create a reproducible baseline without modifying unrelated local changes.

Tasks:

1. Save current `git status --short` and `git diff --stat` into an evidence note.
2. Record current screenshot/probe artifacts under a new timestamped folder, e.g. `verification-m11/dm1-all-graphics/baseline/`.
3. Generate both indexed `.pgm` and RGB `.png` outputs for the same frames.
4. Capture normal V1 and debug HUD V1 separately.
5. List every currently visible text string in normal V1 screenshots by OCR/manual crop review.

Required verification:

- `run_firestaff_m11_game_view_probe.sh` or equivalent existing M11 game-view gate.
- RGB screenshot for:
  - normal in-game start
  - action area visible
  - spell panel open
  - inventory panel open
  - projectile/explosion fixture
- Evidence note: `parity-evidence/dm1_all_graphics_phase0_baseline.md`.

Commit size: documentation/artifacts only if committed later; no code required.

### Phase 1 — Hard V1/V2 asset boundary

Goal: ensure V1 cannot silently use V2/upscaled/generated assets.

Tasks:

1. Trace all asset lookup paths used by `m11_game_view.c`, `asset_loader_m11.c`, launch source selection, and screenshot probes.
2. Add/verify probe assertions that V1 `assetsAvailable` resolves to original `GRAPHICS.DAT` entries, not files under `assets-v2/` or `v2-assets/`.
3. Add a screenshot metadata line or evidence manifest listing `GRAPHICS.DAT` SHA-256 and data root.
4. Audit `tools/build_original_vs_4k_asset_pdf.py` and any original-vs-4k scripts: mark them explicitly **not valid V1 truth** unless rebuilt from locked source table.

Required verification:

- V1 asset-source probe: original `GRAPHICS.DAT` path + SHA-256 (`2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e`) appears in evidence.
- Grep report showing no V1 runtime path reads `assets-v2`/`v2-assets`.

Commit size: one boundary/assertion commit.

### Phase 2 — Remove debug/prototype text from normal V1

Goal: normal in-game view contains only source-bound DM1 text surfaces.

Tasks:

1. Build a whitelist table of allowed normal V1 text surfaces:
   - dungeon/title if original/source-bound
   - message area `C015`
   - leader hand object name `C017`
   - action result `C075`
   - wall/plaque/dialog text from dungeon data
   - champion names / original UI labels where source-bound
2. Move every other helper/prototype string behind `state->showDebugHUD` or out of V1.
3. Add probe checks that normal V1 screenshots do **not** contain known bad strings:
   - `G GRAB`, `P DROP`, `TICK`, `GOOD`, `SAVE DATA`, `MENU BUTTON`, `HERE`, `AHEAD`, `F0`, `W0`, telemetry/action debug wording.
4. Keep debug HUD mode intentionally available and visibly separate.

Required verification:

- Normal V1 screenshot text audit: zero banned debug strings.
- Debug HUD screenshot still shows diagnostic strings when `FIRESTAFF_DEBUG_HUD=1`.
- M11 game-view probe remains green.

Commit size: one debug-text containment commit.

### Phase 3 — Prove/fix `GRAPHICS.DAT` decode, especially odd-width rows

Goal: every bitmap entry decodes with correct dimensions, no heap overwrite, and row padding handled correctly.

Tasks:

1. Turn current `asset_loader_m11.c` local odd-width stride work into a clean audited fix.
2. Add exhaustive decode probe for all bitmap entries:
   - dimensions match header/Greatstone where available
   - pixel values remain `0..15`
   - allocation guard bytes unchanged after decode
   - odd-width entries use row stride `ceil(evenWidth/2)`, not continuous `w*h` nibble stream.
3. Add focused fixture for index `0010`:
   - expected dimensions `87x45`
   - rendered PASS text is not sheared/shifted between rows
   - compare against Greatstone/SCK or trusted color capture.
4. Add focused fixtures for `0009`, `0033..0035`, `0020`, `0078`, `0079`, at least one odd and one even width family.

Required verification:

- ASan build/probe for asset decode guard bytes.
- `FIRESTAFF_ASSET_TRACE=1` trace saved for `0009`, `0010`, `0020`, `0033..0035`.
- RGB PNG contact sheet of decoded originals from Firestaff compared to Greatstone/SCK refs.

Commit size: one asset-decode commit.

### Phase 4 — Lock palette and RGB screenshot path

Goal: no more judging DM1 graphics from grayscale PGM alone.

Tasks:

1. Keep `.pgm` for indexed regression, but generate `.png`/`.ppm` through `G9010_auc_VgaPaletteAll_Compat` for visual review.
2. Add screenshot metadata with palette level and special palette ID.
3. Compare Firestaff base palette RGB against original DOSBox/native capture for at least one static UI asset (`0010` is ideal after decode fix).
4. Verify dungeon lighting levels `LIGHT0..LIGHT5` produce original table colors in runtime output, not linear attenuation.
5. Verify entrance/credits special palettes cannot affect normal dungeon UI.

Required verification:

- Pass 46 and pass 68 probes still PASS.
- New RGB screenshot comparison note includes color sample table by pixel coordinate.
- PGM-only artifacts are explicitly marked “indexed only, not visual color truth”.

Commit size: one screenshot/palette tooling commit.

### Phase 5 — Rebuild action/PASS area from original zones/assets

Goal: action area matches original DM1 geometry and `C010` presentation.

Tasks:

1. Place `C010_GRAPHIC_MENU_ACTION_AREA` into `C011_ZONE_ACTION_AREA` coordinates recovered from ReDMCSB zone table.
2. Reconstruct one-action/two-action menu zones (`C077`, `C079`, `C082..C084`) and PASS zone `C098`.
3. Remove procedural substitutes where original asset/zone owns the pixels.
4. Add click/hit-test assertions tied to the same zones so visual and input geometry do not diverge.
5. Capture action area states:
   - no champion/action available -> PASS state
   - one action
   - two/three actions
   - post-click action result in `C075`.

Required verification:

- Pixel crop compare for `C010` area against Greatstone/SCK/ref capture.
- Probe asserts `0010` dimensions `87x45` and no row shear.
- Runtime screenshot crops for all action-menu states.

Commit size: one action-area commit.

### Phase 6 — Rebuild champion/status/right-side HUD from original zones

Goal: right-side HUD reads as DM1 champion/status UI, not Firestaff utility panel.

Tasks:

1. Recover exact zone coordinates for champion status/name/hand/bar/slot regions.
2. Replace invented panel layout with original zone-driven draw order.
3. Use correct status/slot graphics:
   - alive/dead status box semantics for `0007`/`0008` as source permits
   - slot normal/wounded/acting hand `0033..0035`
4. Verify champion color table and bar fill behavior.
5. Remove `LOAD`, `SAVE DATA`, `MENU BUTTON`, map/status utility cards from normal V1; keep them in launcher/pause/debug if needed.

Required verification:

- Screenshot crop compare for four-champion status area.
- Probe for alive/dead/wounded/acting-hand variants.
- Normal V1 banned-text probe remains green.

Commit size: one champion/status HUD commit.

### Phase 7 — Rebuild spell/rune area from original zones/assets

Goal: spell UI uses DM1 spell-area asset/zone ownership.

Tasks:

1. Place `C009_GRAPHIC_MENU_SPELL_AREA_BACKGROUND` in `C013_ZONE_SPELL_AREA`.
2. Use original symbol/caster/cast/recant zones (`C221`, `C224`, `C245..C255`, `C261..`).
3. Remove non-original helper labels or move them to debug/help mode.
4. Verify rune labels/cells from pass 44 against original asset indices.
5. Capture rune-entry states and cast/recant states.

Required verification:

- RGB crop compare of spell area background and rune cells.
- Probe for zone coordinates and state transitions.
- Normal V1 text whitelist remains satisfied.

Commit size: one spell-area commit.

### Phase 8 — Rebuild inventory/panel overlays from original panel zones

Goal: inventory/equipment panel uses DM1 panel assets/zones, not a database-style slot inspector.

Tasks:

1. Place `C020_GRAPHIC_PANEL_EMPTY` and `C101_ZONE_PANEL` according to original usage.
2. Rebuild equipment/body/hand/container slot layout from `C507..C542` zones.
3. Use `0033`, `0034`, `0035` only for their correct roles.
4. Reduce slot-name text; object icons and spatial layout should do the work.
5. Verify food/water/poison/object description zones `C500..C506`.

Required verification:

- Inventory screenshot crop compare against original runtime capture.
- Probe for every inventory slot zone rectangle.
- Probe for wounded/acting-hand slot variant correctness.

Commit size: one inventory panel commit.

### Phase 9 — Lock viewport geometry, wall/floor/ceiling assets, and lighting

Goal: dungeon view is source-bound, not just plausible.

Tasks:

1. Recover draw constants from ReDMCSB `DRAWVIEW.C`/`GRAPH*.C` or local reconstructed equivalents.
2. Resolve `0078`/`0079` floor/ceiling mapping before changing code; update docs and code comments.
3. Build controlled dungeon positions that show:
   - straight corridor
   - front wall
   - side wall
   - door closed/open/animating
   - stairs/pit/teleporter where visible
   - wall ornament/front ornament/side ornament
   - floor ornament
4. Compare full viewport crops against original DOSBox/native captures at same map/x/y/facing.
5. Verify per-depth palette levels under multiple light amounts.

Required verification:

- Viewport crop comparison report with per-region MAE/max delta, plus human-readable overlays.
- Probe for floor/ceiling asset mapping and wall draw order.
- RGB screenshot set, not only PGM.

Commit size: likely split into two commits if needed: geometry/assets, then lighting/compare tooling.

### Phase 10 — Audit items, creatures, projectiles, explosions, and z-order

Goal: moving/interactive viewport graphics are original-source bound.

Tasks:

1. Inventory all in-game sprite categories used by V1:
   - items on floor/alcove
   - wall/door/floor ornaments
   - creature families and animation frames
   - projectiles in flight
   - explosions/spell impacts
   - damage-to-creature overlay (`C014`) and numeric damage display
2. For each category, record index source, draw cell, scale, palette level, and z-order.
3. Replace procedural non-original effects (e.g. diagonal attack slash cue) with source-bound behavior or hide behind debug.
4. Build controlled fixtures with overlapping item+creature+projectile+ornament to catch ordering bugs.
5. Verify half-square creature placement and group count rendering.

Required verification:

- Sprite audit table in evidence note.
- Runtime screenshot fixtures for each category.
- Probe asserts draw ordering by sentinel pixel/crop where possible.
- ASan run for sprite decode/blit bounds.

Commit size: split by category if needed: items/ornaments, creatures, projectiles/explosions/overlays.

### Phase 11 — Full-screen parity capture and regression gate

Goal: “all DM1 graphics fixed” becomes a repeatable gate.

Tasks:

1. Create `tools/capture_dm1_v1_graphics_suite.*` or extend existing probes to emit a stable screenshot suite:
   - main in-game normal
   - action menu states
   - spell panel states
   - inventory panel states
   - viewport geometry states
   - sprite/effect states
   - palette/light states
2. Create compare tooling against original captures/crops.
3. Generate a final report with per-region pass/fail and known tolerances.
4. Add CI-safe probes that do not require proprietary assets unless `FIRESTAFF_DATA` is present; skip with explicit message otherwise.

Required verification:

- All existing M10/M11/M12 relevant probes still PASS.
- New DM1 graphics suite PASS with source-bound screenshot report.
- No banned debug text in normal V1.
- No V2 asset paths in V1 report.

Commit size: one verification-gate commit.

## Required evidence sources by region

| Region | Primary original evidence | ReDMCSB/code evidence | Firestaff/local evidence |
|---|---|---|---|
| Whole screen/chrome | DOSBox/native 320x200 captures | `DEFS.H` zones, recovered UI draw functions | `M11_V1_SCREENSHOT_GAP_SPEC.md`, current `m11_game_view.c` |
| Viewport | Original captures at fixed map/x/y/facing | view lanes/squares/cells/walls/floors; `DRAWVIEW.C` | pass 40 evidence, `verification-m11/game-view/*.pgm` |
| Action/PASS | Greatstone/SCK graphic 10 color ref | `C010`, `C011`, `C075`, `C077`, `C079`, `C082..C098` | `asset_loader_m11.c`, export audit, new crop compare |
| Spell area | Greatstone/SCK graphic 9, original spell screenshots | `C009`, `C013`, `C221..C261` | pass 44 evidence, spell panel code |
| Champion/status | Original party HUD screenshots | `C151..C218`, status/slot graphics defs | pass 41/43 evidence, export audit |
| Inventory/panel | Original inventory screenshots | `C020`, `C101`, `C500..C542`, `C033..C035` | current inventory panel code/probes |
| Text/message | Original message/dialog screenshots | `C015`, `C017`, `C075`, `C450..C471`, TEXT.C if recovered | local message log/status code |
| Sprites/effects | Original controlled runtime captures | view cell orders, creature/projectile/explosion draw paths | game-view sprite/effect probes |
| Palette | DOSBox/native RGB captures | `VIDEODRV.C` palette tables | `vga_palette_pc34_compat.c`, pass 46/68 logs |

## Risk list

### Heap corruption / memory safety

- Odd-width IMG3 entries can overrun or misalign rows if decoded as continuous nibble streams.
- Temporary header/fake-header decode path can hide under-allocation if expanded capacity is based only on `w*h` rather than padded stride.
- Scaled blits can write out of bounds on negative/edge placements for side sprites, projectiles, explosions, and overlays.
- Existing local `asset_loader_m11.c` guard-byte diagnostic must be converted into durable ASan/probe coverage before trusting it.

Mitigation:

- Guard bytes around every decode buffer in probe builds.
- ASan run for exhaustive asset decode and controlled sprite suite.
- Explicit bounds checks in blit helpers; probes for edge placements.

### Asset decode / semantic mapping

- Correct decode does not imply correct semantic use.
- Known wrong/suspicious mappings:
  - `0007`/`0008` are not left/right status frames.
  - `0034` is wounded slot, not generic highlight.
  - `0000` must not be treated as trusted viewport frame.
  - `0078`/`0079` floor/ceiling likely reversed; re-lock before using.
- Greatstone/SCK may disagree with Firestaff labels; Firestaff loses unless stronger evidence exists.

Mitigation:

- Locked asset table before each phase.
- Evidence note per asset family.
- No V2/upscaled/generated assets as V1 references.

### Palette / visual comparison

- Grayscale PGM screenshots are insufficient for color acceptance.
- Palette level errors can look “okay” in indexed output but wrong in RGB.
- Special entrance/credits palettes may leak if palette namespace is not explicit.

Mitigation:

- Always pair indexed PGM with RGB PNG/PPM rendered through source palette.
- Record palette level/special palette in screenshot metadata.
- Keep pass 46/68 as gates and add runtime color-sample comparisons.

### Debug text leakage

- Many gameplay paths call `m11_set_status` / `m11_log_event`; routing these blindly to the screen recreates debug spam.
- Removing all text would also be wrong because DM1 has legitimate message/dialog/action-result surfaces.
- Save/load/menu affordances may be useful but are not normal DM1 in-game chrome.

Mitigation:

- Source-bound whitelist for visible text surfaces.
- Banned-string probe on normal V1 screenshots.
- Debug HUD screenshot/probe to ensure diagnostics remain available only when requested.

### Geometry / z-order

- Zone coordinates, source rectangles, inclusive/exclusive bounds, and scaling are easy to shift by one pixel.
- Sprite z-order errors often only appear when item+creature+projectile+ornament overlap.
- Half-square creature placement and side/front wall ownership are especially easy to fake incorrectly.

Mitigation:

- Recover zone table and draw order before broad changes.
- Use controlled fixture states and crop overlays.
- Split commits by region/category.

## Definition of done: “all DM1 graphics fixed”

The claim is allowed only when all of the following are true:

1. **Normal V1 has no prototype/debug/helper text.**  Banned-string screenshot probe passes; debug HUD still works only when explicitly enabled.
2. **Every visible V1 screen region has a source-bound owner.**  Viewport, champion/status, action/PASS, spell, inventory/panel, message/dialog, item/creature/projectile/explosion regions each have an evidence note mapping source zones/assets/functions to Firestaff code.
3. **`GRAPHICS.DAT` decoding is proven.**  Exhaustive decode probe passes with guard bytes/ASan; odd-width `0010` action/PASS graphic is visibly and programmatically correct.
4. **Palette is real DM PC 3.4.**  RGB screenshots are generated from `G9010`/special source palettes; no visual acceptance relies on grayscale PGM alone.
5. **Right-side HUD/action/spell/inventory match DM1 zone composition.**  No invented utility dashboard remains in normal V1.
6. **Viewport geometry and dungeon graphics are source-bound.**  Wall/floor/ceiling/door/stair/ornament rendering is locked against ReDMCSB/original captures, including the `0078`/`0079` decision.
7. **Sprites/effects are audited.**  Items, creatures, projectiles, explosions, damage overlays, scaling and z-order have controlled runtime screenshots and source-index tables.
8. **V1 is isolated from V2.**  Verification proves V1 runtime uses original `GRAPHICS.DAT` and never `assets-v2`/`v2-assets`/4K generated assets as source truth.
9. **Regression gates pass.**  M10/M11 relevant probes, new DM1 graphics suite, palette probes, ASan decode/sprite probe, and launcher smoke all pass or skip only for missing proprietary assets with explicit skip text.
10. **Final evidence report exists.**  A human can open one report and see original source refs, Firestaff runtime screenshots, crop comparisons, tolerances, and remaining explicit non-parity exceptions.  If any exception remains, the status is not “all graphics fixed”; it is “all known except X”.
