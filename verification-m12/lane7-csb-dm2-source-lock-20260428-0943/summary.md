# Lane 7 — CSB/DM2 source-lock follow-up

Run dir: `verification-m12/lane7-csb-dm2-source-lock-20260428-0943/`

## Result

PASS for source-lock/provenance evidence only:

- Added `tools/validate_csb_dm2_source_lock.py`.
- Generated `parity-evidence/overlays/csb_dm2_source_lock.json`.
- Generated `parity-evidence/csb_dm2_source_lock.md`.
- Verified 6 local original archives and 17 selected archive members.
- Recorded missing gates without claiming CSB/DM2 runtime or visual parity.

## Locked coverage

- CSB Amiga hard-disk candidate: archive + selected dungeon/graphics member hashes locked; still not curated/canonical.
- CSB Atari ST hard-disk candidate: archive + selected dungeon/graphics member hashes locked; still not curated/canonical.
- DM2 DOS EN: archive + dungeon/graphics/songlist/runtime member hashes locked.
- DM2 DOS CD/layout candidate: archive + same core data/runtime hashes locked.
- DM2 DOS repack: archive + same core data hashes locked; runtime differs from EN/CD candidate.
- DM2 source disassembly: archive + `SKULL.ASM` member hash locked.

## Missing gates / blockers

- No curated CSB extracted source-manifest is present in repo.
- No CSB equivalent of the DM1 Greatstone source-lock helper exists yet.
- No DM2 source-manifest gate maps `SKULL.ASM` symbols to Firestaff runtime paths yet.
- No CSB/DM2 original capture or rendering parity probe is enabled.

## Commands

See `commands.log` for full output.

- `python3 -m py_compile tools/validate_csb_dm2_source_lock.py`
- `python3 tools/validate_csb_dm2_source_lock.py --json-out parity-evidence/overlays/csb_dm2_source_lock.json --markdown-out parity-evidence/csb_dm2_source_lock.md`
- `python3 tools/csb_v1_bootstrap_scout.py`
