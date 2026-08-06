# Parity Evidence

> **Status reviewed 2026-08-06.** Evidence is cross-game and bounded. A green
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

### DM2

Documents covering GDAT material families, G1 record graphs, creature occupancy, combat drops, sound decode.

### Theron's Quest

Documents covering Track 02 IPL/stage-two handoff, level envelope, multi-level object tables.

### Nexus

Documents covering DGN geometry, MNS materials, Structure2/3, PRS3 topology, SAL/MAP audio, SLEV scripts.

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
