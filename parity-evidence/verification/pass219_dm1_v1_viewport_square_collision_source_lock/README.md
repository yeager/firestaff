# pass219 DM1 V1 viewport square/collision source lock

Scope: source-locked blocker/probe for tying Firestaff viewport visible cells to real dungeon square data and movement collision state.

Gate:

```sh
scripts/verify_dm1_v1_viewport_square_collision_source_lock.py --json
```

Recorded result: `source_gate.exit` = 0.

## Locked ReDMCSB contract

- `DUNGEON.C:F0150` computes relative map coordinates from party direction using the same direction step tables used by movement.
- `DUNGEON.C:F0151` reads the actual current-map square byte from `G0271_ppuc_CurrentMapData[x][y]`; out-of-bounds probes are wall squares.
- `DUNVIEW.C:F0128` enumerates visible viewport cells by repeatedly applying `F0150` to party map coordinates, then passes concrete map X/Y into D3/D2/D1/D0 square draw routines.
- `DUNGEON.C:F0172` promotes the square byte into visible square aspect state: wall, pit, fakewall, door side/front, door state, door thing index, first drawable thing.
- `CLIKMENU.C:F0366` uses the same `F0150` + `F0151` target square for movement blocking: wall, door state, fakewall flags, and group occupancy.
- `MOVESENS.C:F0267` rereads destination square bytes for teleporter/pit chains and mutates party X/Y from that state.
- `DRAWVIEW.C:F0097` only presents the viewport buffer after the map-backed draw path has composed it.

## Firestaff blocker implication

A structural viewport draw plan is not enough. To retire this blocker, Firestaff needs a visible-cell probe/provider whose inputs are party map X/Y/facing and whose output is sourced from the dungeon square store (or an equivalent F0150/F0151/F0172 adapter). The rendered wall/door/fakewall/pit/teleporter/content decisions must agree with the movement collision path for the same target square before visual parity can be claimed.
