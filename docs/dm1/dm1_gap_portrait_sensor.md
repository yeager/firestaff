# RESOLVED: Portrait Sensor / Champion Portrait Ordinal in m11_game_view.c

## Status

**RESOLVED — previous 0-7 bound was a stale audit assumption; source-locked portrait ordinals are 0-23 and the M11 front-cell route already rejects out-of-catalog ordinals.**

## Source Location

`src/dm1/dm1_v1_resurrection_pc34_compat.c`, function `F0866_RESURRECTION_RouteChampionPortraitClick_Compat`.

`src/engine/m11_game_view.c`, function `m11_front_cell_mirror_ordinal`.

Related: `src/dm1/dm1_v1_sensor_trigger_pc34_compat.c` -- sensor trigger dispatch.

## Finding

The champion portrait wall sensor in DM1 V1 is a `DM1_SENSOR_WALL_CHAMPION_PORTRAIT` sensor that uses `sensorData` to encode the champion portrait atlas ordinal. The previous audit called this a 0-7 slot range and proposed rejecting `sensorData > 7`.

That was incorrect. ReDMCSB routes `M040_DATA(sensor)` directly to `F0280_CHAMPION_AddCandidateChampionToParty`, and the same value is used for the wall portrait draw path. The portrait graphic is `C026_GRAPHIC_CHAMPION_PORTRAITS`, a 256x87 strip of 32x29 portraits: 8 columns by 3 rows, or ordinals 0-23.

```c
out.championPortraitIndex = in->sensorData;
```

`in->sensorData` is the raw `sensorData` field from the C127 sensor that triggered the click event. This value comes from the dungeon data and is source-owned by the Hall of Champions mirror layout.

## Known Portrait Sensor Data Usage

The resurrection compatibility test keeps both cross-row ordinals live:

```c
in.sensorData = 11;
...
in.sensorData = 23;
```

Both must route to `F0280`; otherwise Firestaff would reject valid Hall of Champions portraits beyond the first atlas row.

## Sensor Data Field Meaning per Type

From `dm1_v1_sensor_trigger_pc34_compat.c`:

- `sensorData = 0`: generic trigger
- `sensorData = 1-4`: directional exits
- `sensorData = object type`: storage/chest selectors
- `sensorData = 0-23`: champion portrait atlas ordinal for C127 Hall portraits

The portrait sensor is the only sensor type where `sensorData` encodes a champion portrait atlas ordinal rather than a game world value.

## Source-Lock

- `DEFS.H:821-826` defines `M027_PORTRAIT_X(index)` and `M028_PORTRAIT_Y(index)` as 8-column atlas math.
- `DEFS.H:2186` defines `C026_GRAPHIC_CHAMPION_PORTRAITS`.
- `DUNGEON.C:2608-2612` stores `M000_INDEX_TO_ORDINAL(M040_DATA(sensor))` as `G0289_i_DungeonView_ChampionPortraitOrdinal`.
- `DUNVIEW.C:3913-3928` decrements that ordinal and blits `(ordinal & 7) * 32`, `(ordinal >> 3) * 29`.
- `MOVESENS.C:1501-1503` calls `F0280_CHAMPION_AddCandidateChampionToParty(M040_DATA(sensor))`.
- `REVIVE.C:142-167` blits the selected portrait from `C026_GRAPHIC_CHAMPION_PORTRAITS` using the same `M027/M028` atlas macros.

## Runtime Guard

`m11_front_cell_mirror_ordinal()` reads the C127 sensor on the actual front square and accepts the ordinal only when it is within `state->mirrorCatalog.count`. Current comment text documents the source shape as 24 portraits, 8 columns by 3 rows.

This is the correct bound for malformed data in the M11 runtime. Adding `if (in->sensorData > 7) return out;` to `F0866` would be a regression.

## Verification

- `test_dm1_v1_resurrection_pc34_compat` asserts that `sensorData = 11` and `sensorData = 23` both reach `F0280`.
- The M11 source path already carries the catalog-count guard for real runtime clicks.

## Impact

No gameplay fix is required here. The DM1 finish queue should not carry this as an open code blocker; remaining champion-panel work belongs to original-runtime pairing and broader panel pixel evidence, not a portrait `sensorData` bounds change.
