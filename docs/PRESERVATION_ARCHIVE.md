# External preservation archive

Historical work runs and generated analysis extracts are stored outside the Git
repository on the external drive. They were moved out of the repository on
2026-08-08 to keep builds and user documentation clean. Active verifiers are
unaffected.

## Archive location

The archive location is intentionally not recorded in version control. It is
operator-owned storage rather than a project dependency; maintainers can use
their own archive root and keep its SHA-256 manifest alongside the archive.

The manifest covers 403 files.

| Previous path | New path | Contents | Status |
|---|---|---|---|
| `verification-dm1/n2-dm1-v1-movement-core-probe-20260505/` | `archive/verification/n2-dm1-v1-movement-core-probe-20260505/` | Historical DM1 movement-probe output | Archived |
| `verification-m11/` | `archive/verification/verification-m11/` | Historical M11 captures, logs, and summaries | Archived |
| `verification-m12/` | `archive/verification/verification-m12/` | Historical M12/V2 and CSB/DM2 work runs | Archived |
| `verification-m13/` | `archive/verification/verification-m13/` | Historical CSB/DM2 source-lock report | Archived |
| `artifacts/firestaff/spanish-graphics-dat/` | `archive/research/spanish-graphics-dat/` | Generated text-analysis extracts from a Spanish GRAPHICS.DAT source | Research archive |

## What remains in the repository

- `parity-evidence/fixtures_dm1_v1_wall_collision_runtime_capture/` remains
  because it is the retained Firestaff-side fixture for an active verifier and
  CTest entry.
- `references/firestaff/dm1/` remains because the Hall of Champions map note is
  still a useful navigation reference. The referenced email attachment is not
  part of the archive unless separately supplied.
- `examples/dm2_hud_widget_synthetic/` remains because CMake and focused probes
  use it as an explicit synthetic-test fixture. It is not real DM2 art.
- `tools/verification/legacy/` contains retained, standalone source-analysis
  scripts. Active CTest verifiers stay in `tools/`.

## Re-running probes

Several scripts still use `verification-m11/` or `verification-m12/` as their
default output directory. If run without an explicit output path, they may
recreate those directories in the checkout. Prefer an output path outside the
repository for new exploratory runs, for example:

```bash
OUT_DIR=/path/outside/the/repository/firestaff-verification/new-run
```

Archived output is historical evidence only. Current claims must point to a
source-lock document, a current manifest or a reproducible test rather than to
an old worker log.
