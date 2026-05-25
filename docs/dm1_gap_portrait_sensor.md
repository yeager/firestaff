# GAP: Portrait Sensor / Champion Portrait Swap in m11_game_view.c

## Status
**GAP — Latent portrait sensorData routing gap; pass449/pass450 blocked on missing DOS runtime capture**

## Source Location
`src/dm1/dm1_v1_resurrection_pc34_compat.c`, function `F0866_RESURRECTION_RouteChampionPortraitClick_Compat` (line 203).

Related: `src/dm1/dm1_v1_sensor_trigger_pc34_compat.c` — sensor trigger dispatch.

## Gap Description

The champion portrait wall sensor in DM1 V1 is a `DM1_SENSOR_WALL_CHAMPION_PORTRAIT` sensor that uses `sensorData` to encode which champion portrait slot (0–7) was clicked. The routing function `F0866_RESURRECTION_RouteChampionPortraitClick_Compat` reads `in->sensorData` directly and returns it as `out.championPortraitIndex`.

```c
out.championPortraitIndex = in->sensorData;   // line 221
```

`in->sensorData` is the raw `sensorData` field from the sensor that triggered the click event. This value comes from the dungeon data (RE-DUNGEON.DAT or equivalent) as laid down by the dungeon editor.

**The gap:** In `m11_game_view.c` (the viewport rendering/input layer), there is a latent code path where champion portrait click events are routed to the wrong `sensorData` value — portrait swap can occur between the intended portrait and an adjacent slot. The issue manifests when `pass449` (portrait swap under specific lighting/angle) or `pass450` (portrait swap on diagonal facing) is triggered.

## Known Portrait Sensor Data Usage

In `dm1_v1_resurrection_pc34_compat.c:398`, the test harness hardcodes `sensorData = 7` for a champion portrait click:
```c
in.sensorData = 7;
```

This indicates that portrait index 7 is the "last slot" (party size 4 + candidates). The `championPortraitIndex` is used to select which champion portrait graphic is shown in the candidate panel.

## Sensor Data Field Meaning per Type

From `dm1_v1_sensor_trigger_pc34_compat.c`:
- `sensorData` = 0: generic trigger
- `sensorData` = 1–4: directional exits
- `sensorData` = object type: storage/chest selectors
- `sensorData` = 0–7: champion portrait slot index

The portrait sensor is the only sensor type where `sensorData` encodes a UI selection index rather than a game world value.

## pass449 / pass450 Blocked

- **pass449**: Runtime capture of champion portrait swap under specific game state (party size 3, facing west, champion departed). Blocked because DOS runtime environment not available on current build infrastructure.
- **pass450**: Runtime capture of diagonal-facing portrait swap. Same block.

Without these captures, the exact conditions that cause the portrait swap are unconfirmed. The gap is inferred from the code structure — `sensorData` is forwarded without validation that it falls within the valid portrait slot range when the sensor type is `DM1_SENSOR_WALL_CHAMPION_PORTRAIT`.

## Code Validation Present

The routing function does check `sensorType`:
```c
if (in->sensorType != DM1_SENSOR_WALL_CHAMPION_PORTRAIT) return out;
```

But it does **not** validate that `sensorData` is in the range [0, 7] before returning it as `championPortraitIndex`. An out-of-range `sensorData` from a malicious or corrupted dungeon file would propagate directly to the portrait selection.

## Required Fix (Non-Blocking)
1. Add bounds check: `if (in->sensorData > 7) return out;` in F0866
2. Capture pass449 and pass450 DOS runtime traces to confirm portrait swap conditions
3. Add integration test for portrait click with `sensorData = 8` (out of bounds)

## Impact
- Normal gameplay: portrait slots 0–7 are well within valid range; no visible bug
- Malicious dungeon: out-of-range `sensorData` could cause undefined portrait index
- Modding: dungeon designers placing portrait sensors with incorrect `sensorData` get silent mis-routing
