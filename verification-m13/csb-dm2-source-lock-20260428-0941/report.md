# CSB/DM2 source-lock follow-up — 2026-04-28 09:41 CEST

Lane: CSB/DM2 source-lock. Scope stayed evidence-only: no DM1 V1 runtime/rendering files were changed.

## Result

PASS with blockers recorded. Existing CSB/DM2 source/provenance state is internally consistent, but still not strong enough to unlock runtime/rendering:

- CSB and DM2 are catalogued in launcher/settings and asset status, but `m12_game_supported()` still intentionally gates launch support to DM1 only.
- `asset_status_m12.c` hardcoded CSB/DM2 hashes are all present in `asset_validator_checksums_m12.json`.
- Current asset status is graphics-file based only: no paired source-lock yet for CSB dungeon/runtime data or DM2 executable/data bundle.
- Local reference material exists on N2 for CSB archives, DM2 archives, DM2 disassembly (`SKULL.ASM`), and ReDMCSB documentation, with SHA256s captured in this run-dir.
- The M12 startup probe confirms CSB/DM2 remain disabled/non-launchable even if asset scanning reports matches.

## Commands and gates

| Command log | Exit | Status | Notes |
| --- | ---: | --- | --- |
| `csb_v1_bootstrap_scout.cmd` | 0 | PASS | Static CSB source-lock scout; confirms CSB slot, launch gate, graphics-only status, no CSB source-lock/capture/runtime probe yet. |
| `csb_dm2_asset_source_probe.cmd` | 0 | PASS | Cross-checks `asset_status_m12.c` CSB/DM2 hardcoded hashes against `asset_validator_checksums_m12.json`. |
| `local_reference_inventory.cmd` | 0 | PASS | Captures local CSB/DM2/ReDMCSB reference file presence and SHA256s. |
| `sevenzip_reference_probe.cmd` | 0 | PASS | Lists CSB Atari ST archive and DM2 disassembly archive; confirms CSB hard-disk files and DM2 `SKULL.ASM` member. |
| `po_validate_po_layout.cmd` | 0 | PASS | Confirms startup/dm1/csb/dm2 PO/POT layout. |
| `cmake_startup_probe_build.cmd` | 0 | PASS | Builds and runs `firestaff_m12_startup_menu_probe`; 50/50 invariants passed. Compiler emitted existing `snprintf` truncation warnings only. |

## Key evidence

- CSB checksum source: `asset_validator_checksums_m12.json` from Daniel Nylander checksum mail, 2026-04-20.
- CSB entries: 26 total; filenames include `GRAPHICS.DAT`, `HCSB.DAT`, `ANIMATE.DAT`, `ENTER.SNG`, utility-disk media.
- DM2 entries: 20 total; filenames include `GRAPHICS.DAT` and `DMCOORD.DAT`.
- Hardcoded source hashes in `asset_status_m12.c`:
  - CSB: `ebf6a57af3f27782e358c0490bfd2f2e`, `291e1bc6803e3dc4b974c60117ca5d68`, `cefaddfdf5651df2c91f61b5611a8362` — all present in JSON.
  - DM2: `25247ede4dabb6a71e5dabdfbcd5907d`, `b4d733576ea60c41737f79f212faf528`, `e52ab5e01715042b16a4dcff02052e5d` — all present in JSON.
- Local reference SHA256s captured in `local_reference_inventory.out`, including:
  - CSB Atari ST archive: `ce6e638622a099bbf15e6dacd7750ce811a52373a20b2d0f92ef6332cc47d7f5`.
  - CSB Amiga archive: `77c3b9ceb3b6d7a9cf96b7cb4801e2b7e51e6de11c5982c82342da268dfddc58`.
  - DM2 source/disassembly archive: `beb703174fe2e263d47e80f56d90b61fad30d2ce04a39e896e5205d6d698265a`.
  - Extracted `dm2-dos-asm/SKULL.ASM`: `a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099`.

## Blocker / next smallest safe fix

BLOCKER: CSB/DM2 source-lock is still graphics-catalog only. Before any launch/render path is enabled, add a dedicated source-lock validator that requires a paired media set:

1. CSB target variant: graphics + dungeon/runtime files (for example Atari ST 2.0/2.1 English hard-disk set) with exact hashes and archive provenance.
2. DM2 target variant: graphics + coordinate/data/executable/disassembly provenance, including `SKULL.ASM` as source evidence but not as a runtime unlock by itself.
3. Evidence output under `parity-evidence/` or `verification-m13/`, while preserving the current CSB/DM2 non-launch gate until paired hashes pass.

No implementation change was made in this pass because the current safe action was to consolidate and record source-lock evidence/gaps.
