# CSB V1 Bootstrap Scout — historical record

This document records the pre-runtime reconnaissance pass. It is retained for
provenance only and is not a statement of Firestaff's current CSB support.

## Superseded conclusions

The original scout correctly required a hash-bound graphics and dungeon pair
before CSB could launch. That requirement is now implemented. Current CSB
startup routes materialize verified original media and hand off to M11 without
borrowing another edition's title, program, graphics or dungeon data:

- PC DOS 3.4 remains the reference runtime route when its verified package is
  present.
- Atari ST 2.0/2.1 uses its own `ANIMATE.SCR`/`ANIMATE.DAT` chain and native
  FTLCODE handoff.
- Amiga A31M follows `TITL.DAT` → APPA → APPB → KAOS; A35M/A35E and A31E use
  their respective verified program handoffs.
- FM Towns follows `TITLE.ANM` → SWITCHTW → Game or Utility and admits
  authenticated F31 resume data.

Each route is selected by hash-verified media identity. A route is blocked
when its edition-private program material is absent; it never substitutes a
nearby PC, Atari, Amiga or Towns resource.

## Current boundary

Startup and selected native runtime handoffs are not a claim of full campaign
parity. The remaining work is tracked in `TODO.md` and includes broader
viewport/HUD coverage, complete interaction ownership, external captures and
the source-complete FM Towns F0433 writer. Consult `docs/PROJECT_STATUS.md`
for the user-facing status.
