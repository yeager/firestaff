# DM1 all-graphics parity — phase 297–396: M11 champion mirror recruit flow

## Scope

Wire the existing source-backed champion mirror catalog into the M11 game-view interaction path without inventing champion data.

## Source anchors

- `firestaff_pc34_core_amalgam.c:7906-7910` declares the source panel mouse commands:
  - `C160_COMMAND_CLICK_IN_PANEL_RESURRECT`
  - `C161_COMMAND_CLICK_IN_PANEL_REINCARNATE`
  - `C162_COMMAND_CLICK_IN_PANEL_CANCEL`
  - zones `M664_ZONE_RESURRECT`, `M665_ZONE_REINCARNATE`, `M666_ZONE_CANCEL`
- `firestaff_pc34_core_amalgam.c:8349-8354` routes clicks in `M568_PANEL_RESURRECT_REINCARNATE` to `F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel`.
- `firestaff_pc34_core_amalgam.c:10698-10703` draws the resurrect/reincarnate panel when a candidate champion is active.
- `firestaff_pc34_core_amalgam.c:10714-10715` makes the candidate champion panel override normal inventory panel rendering.

## Implemented

- Added M11 front-cell mirror lookup by matching visible `THING_TYPE_TEXTSTRING` entries against the DUNGEON.DAT-backed `ChampionMirrorCatalog` text-string index.
- Added M11 candidate mirror panel state:
  - `candidateMirrorOrdinal`
  - `candidateMirrorPanelActive`
- Added public M11 APIs:
  - `M11_GameView_GetFrontMirrorOrdinal`
  - `M11_GameView_SelectFrontMirrorCandidate`
  - `M11_GameView_ConfirmMirrorCandidate`
  - `M11_GameView_CancelMirrorCandidate`
- Changed front-cell inspect for champion mirror TextStrings from generic text-plaque dialog to source-backed mirror candidate selection.
- Routed ACCEPT/ACTION while the candidate panel is active to confirm/resurrect; BACK cancels.
- Fixed two accidental embedded-NUL char literal warnings in mirror name/title output guards.

## New invariants

- `INV_GV_405`: M11 resolves the source mirror TextString in the front viewport cell.
- `INV_GV_406`: M11 mirror click opens a source-backed resurrect/reincarnate candidate panel.
- `INV_GV_407`: M11 mirror panel resurrect command recruits the selected champion.

## Verification

- `ctest --test-dir build --output-on-failure`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- Probe result: `453/453 invariants passed`

## Probe excerpt

```text
PASS INV_GV_400 M11 game view builds champion mirror catalog from DUNGEON.DAT at start
PASS INV_GV_401 M11 mirror catalog exposes display name by ordinal
PASS INV_GV_402 M11 mirror catalog exposes display title by ordinal
PASS INV_GV_403 M11 can recruit champion identity by mirror catalog display name
PASS INV_GV_404 M11 mirror ordinal recruit is idempotent for already-present champion
PASS INV_GV_405 M11 resolves the source mirror TextString in the front viewport cell
PASS INV_GV_406 M11 mirror click opens a source-backed resurrect/reincarnate candidate panel
PASS INV_GV_407 M11 mirror panel resurrect command recruits the selected champion
PASS INV_GV_344 projectile travel: runtime-only projectile is reflected in viewport cell summary
PASS INV_GV_347 projectile detonation: explosion is fireball type and appears in viewport cell summary
# summary: 453/453 invariants passed
```
