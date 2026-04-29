# Pass 166 pass 2 — ReDMCSB source audit

Date: 2026-04-29  
Host: N2 (`Firestaff-Worker-VM`)  
Related runtime rerun: `parity-evidence/pass166_pass2_reincarnate_only.md`  
Runtime commit: `437fcfb Record pass166 reincarnate-only rerun evidence`

## Verdict

Pass 2 must stay **blocked**. The reincarnate-only route used the right *candidate button* coordinate, but ReDMCSB shows the earlier required precondition was not satisfied: the party must first be in dungeon view, facing a front-wall champion portrait sensor (`C127`), and the portrait click must pass through the dungeon-view click handler before `F0280_CHAMPION_AddCandidateChampionToParty()` can run.

The recorded pass 2 route shows `title_or_menu` / `entrance_menu` states around the portrait/reincarnate clicks and then collapses to known static no-party hash `48ed3743ab6a`. That is exactly what the source predicts when `C160/C161` is clicked without a prior `C127 → F0280` candidate transition.

## ReDMCSB source locks

Source root checked:

`~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/`

### 1. Entrance enter click is source-valid, but only loads the dungeon

- `COMMAND.C:346-352` maps entrance mouse zones: `C200_COMMAND_ENTRANCE_ENTER_DUNGEON` uses `C407_ZONE_ENTRANCE_ENTER`.
- `COMMAND.C:557,564,569,574` maps keyboard variants/Return to the same entrance-enter command.
- `COMMAND.C:2438-2441` sets `G0298_B_NewGame = C001_MODE_LOAD_DUNGEON` for `C200_COMMAND_ENTRANCE_ENTER_DUNGEON`.

Interpretation: pass 2's initial enter/click can load the dungeon, but it does not by itself create a champion candidate.

### 2. Portrait click geometry is source-valid only in dungeon viewport coordinates

- `DUNVIEW.C:525`: `G0109_auc_Graphic558_Box_ChampionPortraitOnWall = { 96, 127, 35, 63 }`.
- `COORD.C:1693,1698`: PC viewport origin is `G2067_i_ViewportScreenX = 0`, `G2068_i_ViewportScreenY = 33`.

Derived source center for a visible wall portrait:

- viewport-relative center: `x=(96+127)/2=111`, `y=(35+63)/2=49`
- screen center: `x=111`, `y=49+33=82`

Interpretation: pass 2's `click_111_82` is the right geometry **only if the current view is an actual front-wall champion portrait in the dungeon viewport**.

### 3. Candidate creation requires the dungeon-view click handler and C127 sensor

- `CLIKVIEW.C:348-349` subtracts viewport origin for PC dungeon-view clicks.
- `CLIKVIEW.C:407-431` routes C05/front-wall ornament/door-button hits to `F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor()` when not facing an alcove.
- `MOVESENS.C:1392` explicitly permits `C127_SENSOR_WALL_CHAMPION_PORTRAIT` even when there is no leader champion.
- `MOVESENS.C:1501-1502` maps `C127_SENSOR_WALL_CHAMPION_PORTRAIT` to `F0280_CHAMPION_AddCandidateChampionToParty(L0758_ui_SensorData)`.
- `REVIVE.C:63+` defines `F0280_CHAMPION_AddCandidateChampionToParty()` and it returns early if the leader hand is not empty or the party is full.

Interpretation: the source path is:

`dungeon viewport click → C05 front-wall zone → F0372 touch front-wall sensor → C127 wall champion portrait sensor → F0280 add candidate`

Pass 2 did not prove that chain; its portrait click happened while classifier still saw menu/title states.

### 4. Reincarnate button coordinate is source-valid, but only after F0280 candidate state

- `COMMAND.C:236-237` PC panel boxes: resurrect `x=108..158 y=90..138`, reincarnate `x=161..211 y=90..138`.
- `COMMAND.C:509-510` maps `M664_ZONE_RESURRECT` / `M665_ZONE_REINCARNATE` to `C160/C161` commands in viewport-relative panel mode.
- `COMMAND.C:1990` dispatches `C160..C162` to `F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel()`.

Derived source center for reincarnate:

- `x=(161+211)/2=186`
- `y=(90+138)/2=114` (pass 2 used `115`, still inside the box)

Interpretation: pass 2's reincarnate click is inside the source box, but it is low-value unless source evidence proves `F0280` already created a candidate.

## Runtime evidence cross-check

From `parity-evidence/pass166_pass2_reincarnate_only.md`:

- `click_111_82_4` → `graphics_320x200_unclassified`
- `after_source_portrait_111_82` → `title_or_menu`
- `click_186_115_6` → `title_or_menu`
- `after_source_c161_reincarnate` → `entrance_menu`
- later movement/F1/F4 rows → repeated `dungeon_gameplay` hash `48ed3743ab6a`

This does **not** satisfy the ReDMCSB source chain. The route never demonstrates a front-wall champion portrait sensor firing before C161.

## Correct next pass

Do not run more C160/C161 panel permutations from the static/no-party frame.

The next source-backed pass must first prove the precondition:

1. Enter dungeon from entrance.
2. Navigate to a tile/facing where the front wall has a `C127_SENSOR_WALL_CHAMPION_PORTRAIT`.
3. Click the source-derived portrait center `x=111,y=82`.
4. Verify a visible non-static candidate/resurrect-reincarnate state before clicking `C160`/`C161`.
5. Only then click reincarnate center `x=186,y=114` or resurrect center `x=133,y=114`.

Until step 4 is proven, pass 2 remains negative evidence only.
