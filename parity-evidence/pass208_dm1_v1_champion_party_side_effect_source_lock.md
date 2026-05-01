# Pass 208 — DM1 V1 champion/party movement side-effect source lock

Status: **SOURCE-LOCKED with explicit implementation blocker**

Read-only gate: `tools/pass208_dm1_v1_champion_party_side_effect_source_lock.py`
Verification JSON: `parity-evidence/verification/dm1_v1_champion_party_side_effect_source_lock.json`

## ReDMCSB source evidence

Primary source root: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/`

- `CHAMPION.C:118-130` — `F0284_CHAMPION_SetPartyDirection` is not just a party-facing setter. If the direction changes it computes a delta, rotates every champion `Cell`, rotates every champion `Direction`, writes `G0308_i_PartyDirection`, then redraws changed object icons.
- `COMMAND.C:106-113` and `COMMAND.C:252-260` — V1 movement command routing binds turn/forward/right/back/left commands to mouse arrows and keypad movement input.
- `DUNGEON.C:35-42` — direction-to-east/north step tables define the original coordinate deltas: north `(0,-1)`, east `(+1,0)`, south `(0,+1)`, west `(-1,0)`.
- `DUNGEON.C:1264-1327` — active-group cell/direction getters/setters split party-map active-group state from stored group state; this matters for movement/teleporter side effects and should not be collapsed into pure square passability.
- `MOVESENS.C:232-313` — projectile impact handling builds a per-cell champion/creature ordinal map, checks intermediary cells for adjacent movement, and may test destination-square projectiles before actual handoff.
- `MOVESENS.C:441-451,493-518` — party move result mutates `G0306/G0307`, uses current party direction as the base direction, and applies absolute/relative teleporter rotations through `F0284_CHAMPION_SetPartyDirection`.
- `MOVESENS.C:738-821` — move result globals, same-square early return, scent/last-party-movement updates, source-square sensor removal, destination-square group deletion, destination sensor addition, and cross-map `G0327_i_NewPartyMapIndex` are ordered side effects.
- `MOVESENS.C:892-898` — non-party non-group destination sensor handoff uses the current-map/party-square predicate, separate from party-specific handoff.

## Firestaff local state

The gate confirms current Firestaff code already keeps these partial contracts source-bound:

- `memory_movement_pc34_compat.c` preserves direction-relative movement deltas (`s_dx/s_dy` and forward/right/back/left mapping).
- `dm1_v1_movement_timing_pc34_compat.c` preserves successful-step cooldown clearing, last-party-movement/scent timing, and champion-count-gated scent recording.

## Blocker

`memory_champion_state_pc34_compat.h` exposes `ChampionState_Compat.direction`, but no per-champion `Cell`/party-cell field. That blocks a faithful implementation of `CHAMPION.C:118-130` because the original rotates **both** champion `Cell` and champion `Direction` when party direction changes.

Do **not** paper over this by updating only `party.direction` or champion `direction`; that would silently miss projectile/intermediary-cell and champion-icon side effects. The merge-ready next step is a schema/API owner decision for champion party-cell representation, then a focused side-effect helper/test can be added without touching the dirty input/touch files.

## Verification

```sh
./tools/pass208_dm1_v1_champion_party_side_effect_source_lock.py
```

Expected: `ok: true`, plus blocker `champion_cell_side_effect_schema_gap` remains `BLOCKED` until champion-cell state exists.
