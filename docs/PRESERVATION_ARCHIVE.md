# External preservation archive

Historiska arbetskörningar och genererade analysutdrag ligger utanför Git-
repot på externa disken. De är flyttade från repot 2026-08-08 för att hålla
bygget och användardokumentationen rena. De aktiva verifierarna påverkas inte.

## Archive location

```text
/Volumes/Extern-disk/Documents/Firestaff/archive/
```

SHA-256-manifest:

```text
/Volumes/Extern-disk/Documents/Firestaff/archive/firestaff-archive-sha256-20260808.txt
```

Manifestet omfattar 403 filer.

| Tidigare sökväg | Ny sökväg | Innehåll | Status |
|---|---|---|---|
| `verification-dm1/n2-dm1-v1-movement-core-probe-20260505/` | `archive/verification/n2-dm1-v1-movement-core-probe-20260505/` | Historisk DM1 movement-probe-output | Arkiverad |
| `verification-m11/` | `archive/verification/verification-m11/` | Historiska M11-captures, logs och sammanfattningar | Arkiverad |
| `verification-m12/` | `archive/verification/verification-m12/` | Historiska M12/V2- och CSB/DM2-arbetskörningar | Arkiverad |
| `verification-m13/` | `archive/verification/verification-m13/` | Historisk CSB/DM2 source-lock-rapport | Arkiverad |
| `artifacts/firestaff/spanish-graphics-dat/` | `archive/research/spanish-graphics-dat/` | Genererade textanalysutdrag från en spansk GRAPHICS.DAT-källa | Forskningsarkiv |

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
