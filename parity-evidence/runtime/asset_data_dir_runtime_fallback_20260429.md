# Runtime asset data-dir fallback source lock — 2026-04-29

## Problem

The V1 startup/title/entrance runtime can have the correct ReDMCSB-backed code path but still show no title animation if `M12_AssetStatus` reports the preferred/default originals root (`~/.firestaff/originals`) while the verified PC34 files actually live in the legacy runtime data root (`~/.firestaff/data`).

On N2, the verified files are:

- `/home/trv2/.firestaff/data/DUNGEON.DAT` md5 `766450c940651fc021c92fe5d0d0b3a6`
- `/home/trv2/.firestaff/data/GRAPHICS.DAT` md5 `fa6b1aa29e191418713bf2cda93d962e`
- `/home/trv2/.firestaff/data/TITLE` md5 `05c2ab94ce4dffe51b63985f7b0d1822`

## Fix

- `FSP_ResolveDataDir()` now preserves existing POSIX legacy `~/.firestaff/data` before falling through to XDG `<user-data-dir>/data`.
- `M12_AssetStatus_Scan()` now resolves `status->dataDir` to the root that actually matched the first verified game asset set, instead of keeping a requested/default root that did not match.

This keeps the already-landed ReDMCSB title/entrance paths fed with the real `TITLE` and `GRAPHICS.DAT` assets at launch.

## Verification

Ad-hoc data-dir probe after patch:

```text
dataDir=/home/trv2/.firestaff/data
dm1=1
```

Regression gates:

```text
cmake --build build --target firestaff -j2
./run_firestaff_m11_game_view_probe.sh  # 592/592 invariants passed
./test_entrance_frontend_pc34_compat_integration  # 31 door steps, 11 rattles, 38 source animation steps
```
