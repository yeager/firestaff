# DM1 V1 original collision overlay/runtime cases

Status: DM1_V1_ORIGINAL_COLLISION_OVERLAY_RUNTIME_CASES_LOCKED

This gate loads the canonical local DM1 PC `DUNGEON.DAT` from `~/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/DUNGEON.DAT` and verifies representative original-data cases for the collision/door parity row.

## Source Locks

- `CLIKMENU.C:F0366:264-328` resolves movement target squares and blocks walls, closed doors, and closed real fakewalls before accepted movement reaches `F0267`.
- `DUNGEON.C:F0172:2522-2706` converts closed fakewalls to wall aspect, open fakewalls to corridor aspect, and exposes door front/side aspect plus door state.
- `CLIKVIEW.C:F0377:352-385` computes the front door square and requires decoded `DOOR->Button` before adding a `C10_EVENT_DOOR` toggle.
- `TIMELINE.C:F0241/F0244:711-910` resolves door toggle/animation state changes for open/closed runtime door states.

## Gate

- `firestaff_dm1_v1_original_collision_overlay_runtime_probe` scans canonical DM1 PC `DUNGEON.DAT` for original wall, closed door, fakewall, and decoded button-door cases.
- The same probe mutates one original closed door through the Firestaff door toggle owner to assert the runtime open-door collision and overlay-open state.
