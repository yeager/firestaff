# Parity Evidence

> **Status reviewed 2026-08-25.** Evidence is cross-game and bounded. A green
> parser or fixture test is not automatically a real-media runtime claim.

## What Are Pass Documents?

The `parity-evidence/` directory contains source-lock evidence documents, each named `pass{NNN}_{description}.md`. Each document anchors a specific behavioral claim to:

1. A reference source location (ReDMCSB function/file/line or skproject symbol)
2. A binary offset or data table in the original executable
3. A runnable verification path (test binary or probe command)

Pass documents are numbered sequentially. The repository currently contains
thousands of checked-in evidence documents; use `git ls-files
parity-evidence/ | wc -l` when an exact local count is needed.

## Purpose

Source-lock evidence serves three functions:

- **Proves parity**: documents that Firestaff's implementation matches the original engine's behavior at a specific decision point
- **Prevents regression**: each pass document names a test that can be re-run to verify the claim still holds
- **Records provenance**: captures exactly which reference source location or binary offset justifies each implementation choice

## Structure of a Pass Document

A typical pass document contains:

```markdown
# pass{NNN}: {title}

## Claim

{What this document proves}

## Source Reference

{ReDMCSB file:line, skproject symbol, or binary offset}

## Evidence

{How the claim was verified — binary analysis, source comparison, test output}

## Verification

{Exact command to re-run the proof}
```

## Categories

### DM1 PC34 Compatibility (`*_pc34_compat.md`)

764 documents covering:
- Viewport wall/door/ornament routing
- Movement completion matrix
- Inventory slot placement and drag/drop
- Combat timelines
- Champion panel material
- Original-transcript capture gates
- Creature info table decode (G0243)

### CSB PC34 Compatibility

Documents covering DSA opcode core, startup presentation, entrance/credits, HUD, viewport geometry, thing/sensor runtime, combat, saves, and media.

The native Atari STX route additionally has a real-media M12→M11 proof for
the 50 Hz `ANIMATE.SCR`/`ANIMATE.DAT` title cadence, FTLCODE handoff and first
runtime HUD/viewport frame. This is bounded startup evidence, not a claim of
full campaign parity.

Its native Enter/Accept command reaches that authenticated FTLCODE handoff;
CSB `--platform pc` instead fails closed before selecting media, since no
original DOS/PC edition exists.

### DM2

Documents covering GDAT material families, G1 record graphs, creature occupancy, combat drops, sound decode.

The authentic Amiga archive route additionally verifies its source-clipped
RAW4 CHARSHEET frame (121×72 to 119×70) through the native GDAT/palette path.
This is bounded inventory evidence, not full Amiga campaign parity.

### Theron's Quest

Documents covering Track 02 IPL/stage-two handoff, the authentic JP Rev 1
MODE1/2352 initial Akutuba runtime handoff, level envelope and multi-level
object tables. The JP runtime proof is bounded; it does not claim later-level
or full gameplay parity.

The same real Track 02 is also bound by a source-only public consumer to all
seven campaign dungeons (2,266 source objects total), including
Drator/dungeon 2 (eight maps and 291 objects). It deliberately does not infer
the transition, presentation, AI, combat or item-action semantics.

### Nexus

Documents covering DGN geometry, MNS materials, Structure2/3, PRS3 topology,
SAL/MAP audio, SLEV scripts and retail CUE→BIN CDDA source ownership. The
CDDA evidence binds media only; native decoding/playback remains unproven.

The captured NBG1 receipt proves an enabled 256-colour bitmap layer, BMPNA
palette bank 0 and hardware scroll origin `(0,0)`. It is intentionally not a
retail asset-to-VDP join; STABG stays no-draw until that separate proof exists.
The full 512×256 indexed NBG1 span and its 256-entry CRAM are also decoded
from that authentic capture in native code, but remain capture-only evidence.

## Naming Convention

```
pass{NNN}_{game}_{version}_{feature}_{suffix}.md
```

- `{NNN}`: sequential pass number (e.g., `1101`)
- `{game}`: `dm1`, `csb`, `dm2`, `theron`, `nexus`
- `{version}`: `v1` (V1 compatibility layer)
- `{feature}`: descriptive feature name in snake_case
- `{suffix}`: typically `pc34_compat` for PC 3.4 compatibility claims

## What Pass Documents Are Not

- They are **not** design documents or specifications
- They are **not** test plans (the tests exist independently)
- They do **not** substitute a synthetic capture for original evidence
- They do **not** claim completeness — each document covers one specific point

## Verification

To verify all pass documents' associated tests:

```bash
ctest --test-dir build -j4
```

Individual pass documents name their specific verification command in the `## Verification` section.
