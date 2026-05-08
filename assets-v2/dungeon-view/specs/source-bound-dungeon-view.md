# Source-bound dungeon view contracts

This spec is a manifest-only V2 contract for DM1 dungeon-view layout bindings. It is intentionally not a generated-art acceptance record.

`source-evidenced-layout-contract` entries must remain locked to ReDMCSB DM1 source evidence under `Toolchains/Common/Source` and may describe source viewport/layout dimensions before any 4K/1080p rebuilt art exists.

Relevant ReDMCSB anchors used by `firestaff-v2-dungeon-view-source-bound.manifest.json`:

- `DEFS.H:2348-2377` binds floor, ceiling, wall-set, stairs, and door-set graphic constants for PC34/I34E.
- `DEFS.H:2430-2458` binds the stairs graphic count and down-stairs bitmap ordinals.
- `DUNVIEW.C:2037-2054` loads floor/ceiling graphic pairs.
- `DUNVIEW.C:2962-3002` draws floor/ceiling into the 224x136 viewport buffers.
- `DUNVIEW.C:3048-3065`, `3082-3096`, and `3113-3130` route wall, door, and stairs/pit bitmaps through viewport blits.
- `DUNVIEW.C:4218-4304` routes closed/destroyed door drawing.
- `DUNVIEW.C:8418-8449` handles flipped side-lane floor/ceiling/wall setup.
- `DUNVIEW.C:8488-8499` draws D3L, D3R, then D3C in the far row before nearer rows.

Blocked status means these are source-locked layout contracts only; final V2 artwork/capture acceptance remains a later gate.
