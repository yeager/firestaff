# MD5 / SHA256 Hash Harmonization

## Background

Firestaff identifies game data files by hash in two independent places:

1. **MD5** (32 hex chars) is embedded as compile-time constants in:
   - `src/shared/asset_status_m12.c` — `g_requiredFiles[]` and
     `g_dm1GraphicsNames`/`g_csbGraphicsNames`/etc. tables.
   - `src/shared/asset_find_by_hash.c` — per-file scan-and-match loop.
   - `src/dm2/dm2_v1_boot.c` and `src/dm2/dm2_v1_game.c` — DM2 PC
     version detection (`md5_matches(profile->graphics_md5, ...)`).

   These drive the M12 launcher's "which DM version is this?" prompt
   and runtime version detection (DM1 PC 3.4 EN vs ML vs Atari 1.2,
   CSB PC 3.4 vs Atari ST 2.0/2.1 vs Amiga 3.5, etc.). MD5 is small
   enough to inline as 32-char strings and fast enough to compute
   during filesystem scans.

2. **SHA256** (64 hex chars) lives in `docs/VERIFIED_HASHES.md` and is
   used by:
   - `tools/asset-validate/compare_to_greatstone.py` — cross-reference
     user files against the registry.
   - `tools/verify_pass445_dm1_v1_pc34_data_hash_lock.py` and friends.
   - External auditing tools (`shasum -a 256`).

   SHA256 has no known collisions in the DM/CSB/DM2/Nexus/Theron game
   data universe and is the modern best-practice hash for integrity
   verification.

Both systems **must agree** on what bytes constitute "DM1 PC 3.4
English GRAPHICS.DAT". If the MD5 in `g_requiredFiles[]` says
`fa6b1aa29e191418713bf2cda93d962e` but the SHA256 in
`docs/VERIFIED_HASHES.md` says `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e`,
the launcher would accept a file that the auditor would later reject.

## Why we don't migrate to one hash

- **MD5 stays in C source**: 32-hex chars inline cleanly into arrays,
  embed into per-version version-detection tables, and can be compared
  byte-by-byte with `strcmp` during scan. Migrating to SHA256 in those
  tables would double the constant-data size and complicate the
  version-detection flow that depends on per-MD5 routing.

- **SHA256 stays in docs**: `docs/VERIFIED_HASHES.md` is human-editable
  Markdown, easy to extend with new games/files, and can be checked
  with off-the-shelf `sha256sum`. Users copying the registry into
  spreadsheets or build pipelines work with 64-char hex.

- **Both** also let us spot-check each other — if a user's file passes
  the runtime MD5 check but fails the registry SHA256, something is
  wrong (truncated download, wrong file version, etc.).

## How the cross-check works

`tools/asset-validate/compare_md5_to_sha256.py` enforces the invariant:

1. Parse every MD5 hex string literal in the source files listed in
   `EMBEDDING_SOURCES`.
2. Parse `docs/VERIFIED_HASHES.md` into `(game, filename, sha256, size)`
   entries (supports both the table form and the bullet form).
3. For each registry entry whose local file exists under
   `~/.firestaff/data/`, compute MD5 + SHA256 and verify both:
   - SHA256 matches the registry.
   - MD5 matches the curated runtime table
     (`RUNTIME_MD5_TABLE` in `compare_md5_to_sha256.py`), if one is
     listed for that `(game, filename)` pair.
   - File size matches the registry.
4. Emits a per-file table and exits non-zero if anything mismatches.

A companion script, `tools/asset-validate/compute_md5_for_registry.py`,
prints a human-readable MD5 ↔ SHA256 cross-table for every registry
entry (local or missing) so adding new files is straightforward.

## Curation policy

When adding a new asset identity:

1. Compute SHA256 of the local file:
   `shasum -a 256 path/to/file`
2. Add a row to `docs/VERIFIED_HASHES.md` in either the Core Files
   table or the All Files bullet list.
3. If the file is detected at runtime (DM1/CSB/DM2/Nexus core assets),
   also compute MD5:
   `md5 path/to/file`
   and embed it in the relevant C table
   (`g_requiredFiles[]`, `g_dm1GraphicsNames`, etc.).
4. Add the `(game, filename, md5)` triple to `RUNTIME_MD5_TABLE` in
   `compare_md5_to_sha256.py`.
5. Run `python3 tools/asset-validate/compare_md5_to_sha256.py` and
   confirm a clean PASS.

## When to run the harmonization check

- Before tagging a release.
- After any edit to `docs/VERIFIED_HASHES.md`.
- After any edit to `src/shared/asset_status_m12.c`,
  `src/shared/asset_find_by_hash.c`, or the DM2 boot/game tables.
- When upgrading DM1/CSB/DM2/Nexus game data to a new patch level
  (the hashes will shift).

## Current state (Tier 2 #7 closeout)

`compare_md5_to_sha256.py` reports `PASS: all runtime MD5 + registry
SHA256 agree` for the 19 files currently registered with local copies
under `~/.firestaff/data/`:

- dm1 / DUNGEON.DAT
- dm1 / GRAPHICS.DAT
- dm1-multilingual / DUNGEON.DAT, DUNGEONF.DAT, DUNGEONG.DAT,
  GRAPHICS.DAT, SONG.DAT
- csb / DUNGEON.DAT, GRAPHICS.DAT
- dm2 / DUNGEON.DAT, GRAPHICS.DAT
- nexus / DM.BIN (+ 137 others in the registry)

Files tracked only by SHA256 (no embedded MD5) — such as most
`nexus/*.MNS` files — are verified by hash alone and their MD5 column
is informational in the comparison output.

## Future work (Tier 3+)

- Auto-generate `RUNTIME_MD5_TABLE` from the C source files so we
  don't keep two copies of the same constants in sync.
- Optionally migrate Nexus/Theron tables to SHA256 if/when those games
  grow enough runtime identity tables to warrant the cost.
- Add the harmonization check to CI as a pre-release gate.