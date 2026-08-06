# Nexus runtime capture contract

This document describes the boundary between decoded Saturn source files and
runtime evidence. `MENU.BPK`, `TITLE.BIN`/`TITLE.CG`, `STABG.BIN`, `FACE.BIN`,
`DM.BIN`, `LEV##.DGN`, `SLEV##.BIN`, `SNDLEV##.SAL/.MAP` and `SDDRVS.TSK`
may be parsed and hash-verified from user-supplied data, but those facts do
not identify the Saturn VDP1/VDP2 destination, CLUT owner, command order,
runtime selector, or SH-2 consumer by themselves.

## Required producer

The capture launchers under `probes/nexus/` are operator-only. A capture-capable
instrumented Saturn emulator must create the binary artifact itself. Firestaff
does not copy BIOS, disc, source, or guessed trace bytes into an artifact.

The launchers now check that the selected Mednafen binary advertises the
corresponding `FIRESTAFF_NEXUS_*_OUTPUT` hook before writing a manifest or
starting the emulator. Stock Mednafen is therefore rejected early (exit 78):
its normal video snapshots or movie recording are useful visual evidence, but
they do not prove VDP register/VRAM ownership or SLEV/SAL event dispatch.
The raw Saturn launcher uses a non-quiet string scan for this check because
`set -o pipefail` makes `strings | grep -q` report a false failure when grep
closes the pipe after the first match.

For startup/input experiments the launcher can request an operator-owned
controller window with `--press-start-frame N --press-start-length N` and
choose the active-low Saturn button mask with `--press-button-mask`. The
default `0x10` is START, `0x20` is A, and `0x30` tests both. The external
Mednafen SMPC hook drives that real gamepad bit window and releases it on the
following frame. This is input provenance only: it does not write SH-2 state,
VDP memory, host pixels, or a guessed menu. A resulting menu/HUD/viewport claim
still requires the corresponding source identity and VDP1/VDP2 consumer
artifact.

The raw witness can be inspected with
`scripts/analyze_nexus_saturn_runtime_capture.py`. It reports SHA-256 values
for each captured VDP1/VDP2 region and can require a region to differ between
adjacent frames. This proves observed runtime change in the producer without
identifying a menu, HUD, viewport, CLUT, or consumer; the tool always keeps
semantic admission blocked.

The capture patch now emits `FIRESTAFF_NEXUS_SATURN_VDP1_RAW_V2` with an
explicit VDP1 state line (`TVMR`, `FBCR`, `PTMR`, `EDSR`, `LOPR`, `COPR`, the
return pointer and framebuffer selector) before the unchanged raw VRAM/FB
payload. V1 witnesses remain readable. The state line is an observation of
the emulator's VDP1 model, not by itself proof that a particular MENU.BPK,
DGN, HUD or viewport record owns the command stream.

## Artifact families

| Route | Magic | Evidence still required |
|---|---|---|
| PRS3/menu replay | `NXSPRS3M` | source stream, SH-2 execution trace, PALT/VDP1 consumer and frame identity |
| VDP1 material | `NXSVDP1C` | DGN/face/descriptor/image/palette identity, command words, VRAM window and destination |
| SLEV/SAL runtime | `NXSLSC01` | task body, event selector, MAP row, SAL window and SDDRVS consumer |
| Structure3 topology | `NXS3TOP1` | face/vertex ownership, transform/culling route and frame order |

Each verifier checks the artifact header, source hashes, bounded payload and
identity lanes. A manifest alone is not a capture; a source decoder alone is
not a presentation or gameplay proof. Until the corresponding artifact is
admitted, production routes remain no-draw or playback-gated.

## Source references

The byte-format baseline is the [DMWeb Nexus file-format documentation](http://dmweb.free.fr/community/documentation/dungeon-master-nexus/file-formats/),
including the [ITEM.IBS description](http://dmweb.free.fr/community/documentation/dungeon-master-nexus/item.ibs-file/)
and [DGN documentation](http://dmweb.free.fr/community/documentation/dungeon-master-nexus/dgn-files/).
Disassembly and runtime ownership must be established from the Saturn program
and an execution capture; inferred offsets or host screenshots must not be
promoted to production semantics.
