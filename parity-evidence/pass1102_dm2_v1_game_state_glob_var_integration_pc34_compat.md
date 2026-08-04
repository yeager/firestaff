# Pass 1102 — DM2 V1 Game State Glob Var Integration

## What

Wire the three-tier glob var storage (pass 1100) into `DM2_V1_GameState` as
embedded `DM2_V1_GameGlobVars`. Provide actuator callback adapters
`dm2_v1_game_get_glob_var` and `dm2_v1_game_update_glob_var` that accept
`DM2_V1_GameState*` as context and delegate to the self-contained three-tier
access functions.

## Layout

`DM2_V1_GameGlobVars` is layout-identical to `DM2_V1_GlobVarState` (verified
by `_Static_assert`). Defined separately in `dm2_v1_game.h` to avoid pulling
`dm2_v1_runtime_parity_pc34_compat.h` into the include graph (which causes
conflicting type declarations between the callback-based and dedicated
implementations of several DM2 record operations).

## Fixes

Removed duplicate symbol definitions from `dm2_v1_runtime_parity_pc34_compat.c`:
- `dm2_v1_alloc_new_dbitem` — superseded by `dm2_v1_dbitem_alloc_pc34_compat.c`
- `dm2_v1_drop_creature_possession` — superseded by `dm2_v1_drop_possession_pc34_compat.c`

These caused linker duplicate symbol errors when both the runtime_parity
compilation unit and the dedicated compilation units were linked into the same
target.

## Source references

- skproject `dm2globl.cpp:21` — `DM2_UPDATE_GLOB_VAR`
- skproject `skgdtqdb.cpp:1928` — `DM2_GET_GLOB_VAR`
- `ddat.v1e0104` (8 bytes), `ddat.globalb` (64 bytes), `ddat.v1e000c` (384 bytes)

## Test

`test_dm2_v1_game_glob_var_integration` — verifies init zeroes all tiers,
round-trip through adapter callbacks across bit/byte/word tiers, and that
adapter function signatures match the actuator callback typedefs.
