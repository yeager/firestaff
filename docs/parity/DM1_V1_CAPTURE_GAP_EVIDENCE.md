# DM1 V1 Original Capture Gap Evidence

**Lane:** DM1 V1 finish-quality - original DOS capture/parity evidence lane
**Date:** 2026-05-28; updated 2026-06-20 with pass1052/pass1053/pass1055/pass1056
**Branch:** `dm1v1-capture-gap-evidence-20260528`
**Author:** subagent (MiniMax-M2.7)

## Scope

This document inventories the concrete paired original PC 3.4 evidence that must exist
before viewport, wall, collision, creature-chain, and champion-panel parity can be
marked `MATCHED` in the DM1 V1 parity ledger.

**Standard:** Every parity row that claims `MATCHED` for pixel/content correctness must
have at least one paired original-capture <-> Firestaff-output comparison as evidence.
Source-lock documentation alone (ReDMCSB citations, constant audits, probe invariants)
is sufficient for `SOURCE_LOCKED` but NOT for `MATCHED` pixel/content parity.

---

## 1. Canonical Reference Data

| Item | Value |
|------|-------|
| Game | Dungeon Master, PC DOS, English, v3.4 |
| DUNGEON.DAT SHA256 | `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85` |
| GRAPHICS.DAT SHA256 | `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e` |
| TITLE SHA256 | (see `SHA256SUMS` in `firestaff-original-games/DM/`) |
| Source archive | `ReDMCSB_WIP20210206/Toolchains/Common/Source/` |
| Local archive | `~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/` |
| Canonical game dir | `~/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/` |
| Greatstone reference | `http://greatstone.free.fr/dm/db_data/dm_pc_34/` |

---

## 2. Evidence Inventory by Area

### 2a. Viewport

| Evidence Item | Status | Path | Issue |
|---|---|---|---|
| 4x original DOSBox turn-cycle frames | PARTIAL OK | `verification-screens/pass1052-dm1-original-route-24h-turncycle/` | Clean original DM1 PC 3.4 capture: 2 `dungeon_gameplay` + 2 `wall_closeup`, 0 duplicate raw hashes, pass80 PASS. pass1056 now gates the pass1054 Firestaff nearest-neighbor pairing artifacts, but full same-state viewport parity still needs a debugger-observed route pair. |
| 6x original DOSBox crops (224x136) | IMPAIRED EXISTS (impaired) | `firestaff-release-v0.3.28/verification-m11/lane4-original-overlay-20260428-0917/pass94-diagnostic/viewport_224x136/` | Historical failed route. Frames 03-06 have duplicate SHA256 `701689e73fc0b3f4aa027182a9c1f5059ae90279d164dd42329c7b96092c5d4c`; pass80 reclassifies frames 03-04 as `entrance_menu` and 05-06 as `wall_closeup`. |
| 6x Firestaff V1 captures (VGA PPM) | OK EXISTS | `firestaff-v2-gap-manifest/verification-m11/lane3-inventory-followup-20260428-0914/` (selected files) | No paired original to compare against. |
| Source-locked viewport probe | OK EXISTS | `probes/dm1/firestaff_dm1_v1_viewport_draw_order_probe.c` | Documents draw-order contract from ReDMCSB; no paired capture. |
| Door occlusion pixel gate | OK EXISTS | `probes/dm1/firestaff_dm1_v1_door_occlusion_pixel_gate.c` | Documents pixel-level occlusion contract; no paired capture. |
| Viewport palette probe | OK EXISTS | `probes/dm1/firestaff_dm1_v1_viewport_palette_as_before_probe.c` | Palette-as-before for screenshot comparison; no paired capture. |

**Gap:** Pass1052 supersedes the broken pass94 route for initial viewport evidence by
capturing two clean `dungeon_gameplay` frames. pass1054/pass1056 now provide a
reproducible Firestaff nearest-neighbor pairing gate over those crops, including
one exact wall row. Viewport content is still not `MATCHED` because the nonzero
gameplay rows are scout candidates, not debugger-confirmed same-state pairs.

**Minimum needed for `MATCHED`:**
- At least 3 clean original `dungeon_gameplay` frames: (a) start-state 3x3 dungeon viewport,
  (b) after one legal forward step, (c) after a turn
- Each frame must be: 320x200 raw -> cropped to 224x136 (viewport region at y=33)
- Firestaff output from identical game state and input sequence
- Pixel-difference measurement (MAE, max delta) between original crop and Firestaff output

---

### 2b. Wall Composition

| Evidence Item | Status | Path | Issue |
|---|---|---|---|
| Wall composition contract probe | OK EXISTS | `probes/dm1/firestaff_dm1_v1_wall_composition_contract_probe.c` | Source-locked wall-set/flipping/occlusion contract; no paired capture. |
| Walls occlusion blockers probe | OK EXISTS | `probes/dm1/firestaff_dm1_v1_walls_occlusion_blockers_probe.c` | Source-locked wall occlusion logic; no paired capture. |
| Door occlusion pixel gate | OK EXISTS (see above) | `probes/dm1/firestaff_dm1_v1_door_occlusion_pixel_gate.c` | Source-locked pixel-level door occlusion; no paired capture. |
| Side contents center-blocker probe | OK EXISTS | `probes/dm1/firestaff_dm1_v1_side_contents_center_blocker_probe.c` | Documents side-panel center-blocker behavior; no paired capture. |
| Original-to-Firestaff wall crop diff | PARTIAL MATCH | `verification-screens/pass1054-dm1-original-firestaff-viewport-wall-diff/` | One wall crop is exact: original `02_left_1_wall_original_viewport_224x136.png` matches Firestaff `hall_1_4_dirE_viewport_224x136.ppm` with 0 changed pixels and MAE 0. |
| Original wall screenshot | PARTIAL OK | `verification-screens/pass1052-dm1-original-route-24h-turncycle/` | Pass1052 has 2 clean original `wall_closeup` frames. One now has an exact pass1054 Firestaff match; broader wall-state route still needs more pairings. |

**Gap:** Wall evidence is no longer original-capture-empty, and pass1054 proves one
exact original-to-Firestaff wall-crop match. Wall composition parity for the whole
surface still cannot be marked globally `MATCHED` until multiple deterministic wall
states are paired and compared.

**Minimum needed for `MATCHED`:**
- 3 original screenshots: (a) front-wall view (D3C visible), (b) side-wall view
  (D3L or D3R dominant), (c) front-alcove view (D3C + D3L/D3R)
- Each paired with Firestaff output from identical dungeon state
- Semantic check: correct wall-set indices, correct flip orientation, correct occlusion

---

### 2c. Collision

| Evidence Item | Status | Path | Issue |
|---|---|---|---|
| Collision overlay runtime probe | OK EXISTS | `probes/dm1/firestaff_dm1_v1_original_collision_overlay_runtime_probe.c` | Source-locked collision + overlay query; no paired original transcript. |
| Fakewall view collision probe | OK EXISTS | `probes/dm1/firestaff_dm1_v1_original_fakewall_view_collision_probe.c` | Source-locked fakewall collision logic; no paired original transcript. |
| Playable route probe | OK EXISTS | `probes/dm1/firestaff_dm1_v1_playable_route_probe.c` | Documents canonical movement route; no paired original transcript. |
| Original closed-door collision/stasis capture | PARTIAL OK | `verification-screens/pass1055-dm1-original-closed-door-collision/` | Pass1055 captures original PC 3.4 closed-door stasis: `door_before`, `after_viewport_click`, and `after_kp5` have identical raw and viewport-crop SHA256 values. |
| Firestaff closed-door semantic pair | PARTIAL PAIR | `probes/dm1/firestaff_dm1_v1_pass1055_closed_door_pair_probe.c` | Replays the pass1055 movement sequence to `map=0 x=6 y=9 dir=3`, sees closed door square `0x94` at `(5,9)`, and blocks the next forward command with no party movement. |
| Full original collision transcript | MISSING MISSING | - | No paired original DOS transcript yet covers wall, door, fakewall, and door-state responses. |

**Gap:** Collision probes verify that Firestaff's collision logic matches ReDMCSB
source. Pass1055 adds original DM1 PC 3.4 runtime evidence for one closed-door
stasis case plus a Firestaff-side semantic pair for the same movement sequence.
No pixel comparison and no full wall/door/fakewall transcript has been promoted
yet, so collision cannot be marked globally `MATCHED`.

**Minimum needed for `MATCHED`:**
- Deterministic collision transcript: record party movement commands and game responses
  (BLOCKED/OPEN, door state changes, fakewall toggles) from original DM1 PC 3.4
- Same sequence run through Firestaff
- Bit-exact match on all collision responses

---

### 2d. Creature-Chain

| Evidence Item | Status | Path | Issue |
|---|---|---|---|
| Creature render integration test | OK EXISTS | `tests/test_dm1_v1_creature_render_pc34_compat_integration.c` | Source-locked aspect table, bitmap index, pose, flip, palette; no paired capture. |
| Creature AI behavior test | OK EXISTS | `tests/test_dm1_v1_creature_ai_behavior_pc34_compat.c` | Source-locked AI logic; no paired original transcript. |
| Lane3 creature captures | IMPAIRED FIRESTAFF ONLY | `firestaff-v2-gap-manifest/verification-m11/lane3-inventory-followup-20260428-0914/35_focused_d1c_trolin_creature_vga.ppm` etc. | Firestaff output only; no paired original DM1 PC 3.4 screenshot of any creature. |
| Original creature screenshot | MISSING MISSING | - | No paired original DM1 PC 3.4 creature screenshot exists. |

**Gap:** No paired original DM1 PC 3.4 screenshot of a creature in the viewport exists.
The creature-chain z-order (D2C creature after D2C items, before D1C items) is
source-locked in `firestaff_dm1_v1_viewport_draw_order_probe.c` and verified by
source-lock tests, but pixel-level creature rendering has not been compared against
an original screenshot.

**Minimum needed for `MATCHED`:**
- 2 original screenshots: (a) creature in D2C cell, (b) creature in D1C cell
- Each paired with Firestaff output from identical dungeon state
- Semantic checks: correct creature aspect/index, correct pose, correct scale
  (D2=20x20, D3=16x16), correct horizontal flip, correct palette

---

### 2e. Champion-Panel

| Evidence Item | Status | Path | Issue |
|---|---|---|---|
| Champion panel HUD test | OK EXISTS | `tests/test_dm1_v1_champion_panel_hud_pc34_compat.c` | Source-locked geometry/constants; no paired capture. |
| Champion stats test | OK EXISTS | `tests/test_dm1_v1_champion_stats_pc34_compat.c` | Source-locked bar graph logic; no paired capture. |
| Lane3 champion HUD captures | IMPAIRED FIRESTAFF ONLY | `firestaff-v2-gap-manifest/verification-m11/lane3-inventory-followup-20260428-0914/party_hud_four_champions_vga.ppm`, `party_hud_statusbox_gfx_vga.ppm` | Firestaff V1 output only; no paired original DM1 PC 3.4 champion panel screenshot. |
| Original champion candidate/resurrect panel | PARTIAL OK | `verification-screens/pass1053-dm1-original-champion-candidate-panel/` | Pass1053 tracks pass455 original candidate panel (`click:111,82`) and post-C160 terminal/HUD transition (`click:130,115`). Not a full four-champion HUD pair. |
| Original four-champion party HUD screenshot | MISSING MISSING | - | No paired original DM1 PC 3.4 four-champion party HUD screenshot exists. |

**Gap:** The champion panel geometry, status-box stride, portrait positions, and
bar-graph layout are all source-locked and probe-verified. Pass1053 now adds original
PC 3.4 evidence for the candidate/resurrect panel transition, but no paired full
four-champion party HUD or single-champion status panel screenshot exists yet to verify
pixel-level rendering correctness.

**Minimum needed for `MATCHED`:**
- 2 original screenshots: (a) four-champion party HUD (portraits + status boxes + bar graphs),
  (b) single-champion status panel
- Each paired with Firestaff output from identical game state
- Semantic checks: correct portrait atlas indexing, correct slot stride (69 px), correct
  bar-graph widths (4px) and heights (<=25px), correct HP/stamina/mana color palette

---

## 3. Summary: Gap vs. Existing Artifacts

| Area | Source-Locked Probe | Firestaff Capture | Original Capture | Pairing | Blocking Issue |
|------|--------------------|--------------------|-----------------|---------|---------------|
| Viewport | OK | OK | PARTIAL OK (pass1052) | SCOUT PAIRING (pass1056) | Clean original gameplay frames exist; pass1056 gates Firestaff nearest-neighbor pairing artifacts, but same-state viewport parity is still missing |
| Wall | OK | OK | PARTIAL OK (pass1052) | PARTIAL MATCH (pass1054) | One exact wall-crop match exists; broader wall-state route still needs more pairings |
| Collision | OK | MISSING | PARTIAL OK (pass1055 closed door) | PARTIAL PAIR (semantic) | One original closed-door stasis capture and one Firestaff semantic pair exist; full paired transcript still missing |
| Creature-chain | OK | IMPAIRED Firestaff-only | MISSING | MISSING | No original creature screenshot exists |
| Champion-panel | OK | IMPAIRED Firestaff-only | PARTIAL OK (pass1053 candidate/resurrect panel) | MISSING | Full four-champion HUD/status-panel pairing still missing |

**Conclusion:** Existing Firestaff-side gates, source locks, and runtime routing are complete.
Pass1052 and pass1053 reduce the original-reference gap: viewport, wall, and the
candidate/resurrect champion panel now have clean original PC 3.4 frames. Pass1054
promotes one exact original-to-Firestaff wall-crop match, and pass1056 turns that
pairing evidence into a CTest-backed gate. The remaining `MATCHED`
blockers are same-state Firestaff pairing/pixel-diff promotion for viewport and wall
states, a full paired collision transcript, a creature screenshot, and full four-champion/status-panel
original captures.

---

## 4. Reference: Existing Capture Sessions

| Session | Date | Host | Outcome |
|---------|------|------|---------|
| `lane3-inventory-followup-20260428-0914` | 2026-04-28 | N2 | Firestaff-only captures; original route not reached |
| `lane4-original-overlay-20260428-0917` (pass94) | 2026-04-28 | N2 | Original captures attempted; DOSBox route failed; frames are entrance_menu/wall_closeup |
| `lane1-original-faithful-parity-20260428-0931` | 2026-04-28 | N2 | Unknown outcome (not yet examined) |

---

## 5. Minimum Required Paired Capture Set

To close the gap for all five areas simultaneously, the following captures are needed
in one session (or multiple sessions producing the same deterministic game state):

| # | Label | Game State | What to Capture | Format |
|---|-------|-----------|----------------|--------|
| 1 | `viewport_start` | Map 0, (1,3), SOUTH, pre-move | 320x200 raw + 224x136 viewport crop | PPM |
| 2 | `viewport_after_step` | Map 0, (1,4), SOUTH, post-one-step | 320x200 raw + 224x136 viewport crop | PPM |
| 3 | `viewport_creature` | Map 0, (x,y) with creature in D2C | 320x200 raw + 224x136 viewport crop | PPM |
| 4 | `wall_front` | Map 0, facing D3C wall | 320x200 raw + 224x136 viewport crop | PPM |
| 5 | `wall_side` | Map 0, facing D3L or D3R wall | 320x200 raw + 224x136 viewport crop | PPM |
| 6 | `collision_wall` | Attempt step into wall | Collision response transcript | TXT |
| 7 | `collision_door` | Attempt step into closed door | Collision response transcript | TXT |
| 8 | `champion_hud` | Four champions visible | 320x200 raw + champion-panel crop | PPM |
| 9 | `champion_status` | Single champion detail | 320x200 raw + status-box crop | PPM |

Each original capture must be paired with:
1. Identical DUNGEON.DAT SHA256 (`d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85`)
2. Identical GRAPHICS.DAT SHA256 (`2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e`)
3. Identical input sequence (so Firestaff can reproduce the same state)
4. Firestaff output from that same state and sequence

---

## 6. Honest Status Labels

Given the above gap inventory, the correct parity status labels for the five areas are:

| Area | Current Label | Honest Label | Reason |
|------|-------------|--------------|--------|
| Viewport | `MATCHED` (bounds) / `KNOWN_DIFF` (content) | `SCOUT_PAIRING_AVAILABLE` | Pass1052 original gameplay frames exist; pass1056 gates pass1054 nearest-neighbor Firestaff pairing artifacts, but the nonzero rows are not same-state parity |
| Wall | `KNOWN_DIFF` (narrowed) | `PARTIAL_MATCH` | Pass1054 has one exact original-to-Firestaff wall-crop match; broader wall route still partial |
| Collision | (source-lock + pass1055 original closed-door stasis + Firestaff semantic pair) | `PARTIAL_PAIR` | Pass1055 captures one original closed-door stasis case and a Firestaff-side movement/collision pair; pixel comparison and broader transcript are still missing |
| Creature-chain | (source-lock only) | `BLOCKED_ON_REFERENCE` | No original creature screenshot exists |
| Champion-panel | (source-lock only) | `PARTIAL_REFERENCE_CAPTURED` | Pass1053 candidate/resurrect panel exists; full party HUD/status-panel pair is still missing |

**Recommendation:** Keep the content/pixel sub-rows out of `MATCHED` until same-state
Firestaff comparisons exist. Viewport and champion candidate/resurrect panel can now
be labelled `PARTIAL_REFERENCE_CAPTURED`; wall can be labelled `PARTIAL_MATCH`;
collision can be labelled `PARTIAL_PAIR` for the single closed-door semantic
case; creature-chain remains `BLOCKED_ON_REFERENCE`.
