# External preservation archive

Historical work runs and generated analysis extracts are stored outside the Git
repository on the external drive. They were moved out of the repository on
2026-08-08 to keep builds and user documentation clean. Active verifiers are
unaffected.

## Archive location

```text
/Volumes/Extern-disk/Documents/Firestaff/archive/
```

SHA-256-manifest:

```text
/Volumes/Extern-disk/Documents/Firestaff/archive/firestaff-archive-sha256-20260808.txt
```

The manifest covers 403 files.

| Previous path | New path | Contents | Status |
|---|---|---|---|
| `verification-dm1/n2-dm1-v1-movement-core-probe-20260505/` | `archive/verification/n2-dm1-v1-movement-core-probe-20260505/` | Historical DM1 movement-probe output | Archived |
| `verification-m11/` | `archive/verification/verification-m11/` | Historical M11 captures, logs, and summaries | Archived |
| `verification-m12/` | `archive/verification/verification-m12/` | Historical M12/V2 and CSB/DM2 work runs | Archived |
| `verification-m13/` | `archive/verification/verification-m13/` | Historical CSB/DM2 source-lock report | Archived |
| `artifacts/firestaff/spanish-graphics-dat/` | `archive/research/spanish-graphics-dat/` | Generated text-analysis extracts from a Spanish GRAPHICS.DAT source | Research archive |

## What remains in the repository

- `pass610_dm1_v1_firestaff_viewport_crop_capture_gate/` remains because its
  verifier and CTest entry are active.
- `references/firestaff/dm1/` remains because the Hall of Champions map note is
  still a useful navigation reference. The referenced email attachment is not
  part of the archive unless separately supplied.
- `examples/dm2_hud_widget_synthetic/` remains because CMake and focused probes
  use it as an explicit synthetic-test fixture. It is not real DM2 art.
- `tools/verify_pass*.py` remains because these are active source-lock and
  CTest verifiers, not historical output.

## Re-running probes

Several scripts still use `verification-m11/` or `verification-m12/` as their
default output directory. If run without an explicit output path, they may
recreate those directories in the checkout. Prefer an external output path for
new exploratory runs, for example:

```bash
OUT_DIR=/Volumes/Extern-disk/Documents/Firestaff/archive/verification/new-run
```

Archived output is historical evidence only. Current claims must point to a
source-lock document, a current manifest or a reproducible test rather than to
an old worker log.
