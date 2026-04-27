# Firestaff V2 Reference Inputs

This directory records the non-runtime reference material used to brief V2 asset packs.
It deliberately stores metadata and lookup notes, not copied game archives or generated art.

## Worker VM reference roots

| Reference root | Purpose | Repo policy |
|---|---|---|
| `~/.openclaw/data/firestaff-greatstone-atlas/` | Greatstone/SCK documentation and source-reference downloads for DM/CSB research | Keep as external evidence; cite paths only. |
| `~/.openclaw/data/firestaff-redmcsb-source/` | ReDMCSB notes and local source/disassembly candidate pointers | Keep as external evidence; cite paths only. |
| `~/.openclaw/data/firestaff-original-games/DM/` | Original game archive checksums and local extraction inputs | Never vendor archives; cite checksum lock only. |

## Current scaffold

- `reference-input-lock.json` captures the files visible on the worker VM for the above roots.
- V2 asset manifests should cite source semantics (`GRAPHICS.DAT` indices, family names, dimensions) while keeping binary originals out of git.
- Viewport-frame work remains blocked/provisional until graphic `0000` is re-locked against Greatstone/SCK and ReDMCSB evidence.

## Local validation

Run from the repo root:

```sh
python3 scripts/validate_v2_asset_manifests.py
```

Use `--strict-paths` when a pack is expected to have all manifest scaffold directories and spec documents checked in.
