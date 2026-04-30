# Pass 205 — DM1 V1 original-faithful source guard

Lane: DM1 V1 original-faithful parity/evidence.

Scope: source-first guard only.  No HUD, viewport renderer, inventory, touch, CSB, runtime, or emulator-route code changed.

## ReDMCSB source anchors

- `DEFS.H:509-523` defines the platform and dungeon identifiers used by the runtime contract: `C9_PLATFORM_PC = 9`, `C10_DUNGEON_DM = 10`, plus the CSB ids that must not be conflated with DM.
- `CEDTINCD.C:181-253` routes saved-game loading through the Dungeon Master choice (`M741_FILE_ID_LOAD_DMSAVE_DAT`), reads the save header, calls `F7054_DetermineGameFormats(...)`, then validates non-mini saved games with `F7272_IsDungeonValid(..., 1)`.
- `CEDTINCU.C:18-76` resolves `SaveHeaderFormat` into `DungeonID`/`Platform`; criterion `1` accepts `C10_DUNGEON_DM` (alongside CSB ids for the shared loader) only after the platform guard compiled for that target.
- `COORD.C:1713-1722` pins the core render contract to `320x200` screen, `224x136` viewport, and `15232` bytes for the 4bpp viewport bitmap.
- `STARTUP2.C:1179-1183` installs the interface/movement mouse and keyboard tables before party map processing, which is the source-level input-routing invariant this lane can cite without inventing emulator behavior.

## Reference-data contract

The guard also checks the local DM PC 3.4 extracted reference files under `~/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34/DATA`:

| File | Bytes | SHA-256 |
| --- | ---: | --- |
| `DUNGEON.DAT` | 33357 | `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85` |
| `GRAPHICS.DAT` | 363417 | `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e` |
| `SONG.DAT` | 162482 | `71e1ba82b7f3cfeb99ae181bd9d685201bdc2b11f42643460120ddcb3470c177` |

## Added gate

`tools/verify_dm1_v1_original_faithful_source_guard.py` is a small worker-local verifier for the anchors above.  It is intentionally evidence-only: it does not launch DOSBox, inspect screenshots, or claim pixel parity.  It blocks if the ReDMCSB source path is missing, if the cited source shapes drift, or if the local DM PC 3.4 reference hashes do not match.

## Validation

```text
python3 tools/verify_dm1_v1_original_faithful_source_guard.py
python3 tools/validate_dm1_pc34_provenance.py --skip-archive --check-extracted
python3 -m py_compile tools/verify_dm1_v1_original_faithful_source_guard.py tools/validate_dm1_pc34_provenance.py
git diff --check
```

Current blocker remains unchanged: the lane still needs a deterministic original PC DM1 route that reaches party-control-ready gameplay with semantic labels before captured frames can be promoted to original-faithful pixel references.
