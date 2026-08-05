# Nexus DGN and PRS3 Internals

## Package and Geometry Evidence

Nexus is a Saturn target. Firestaff keeps package identity, DMDF metadata, DGN
geometry, static material, Structure2, BPK/PRS3, scripts, and sound as separate
evidence layers. A successful title route does not imply a valid dungeon route.

## Structure1B and Related Families

Structure1B provides the typed grid/height base used by the DGN plan. The
renderer projects floor, ceiling, and wall corner heights from typed values.
The post-grid record families are bounded rather than guessed from offsets.

Structure1F is split by observed ownership. Direct-coordinate records can be
represented as items, floor decoration, or floor sensors. Alcove/wall records
remain Structure1A-bound until an original mapping proves their cell/draw role.
This distinction prevents a plausible byte row from becoming a false world
object.

## Static MNS Route

`SN_FLOOR.MNS` and `SN_WALL.MNS` are paired retail sources. Their bounded TEXT
sections and BGR555 descriptors feed static Structure1B floor/ceiling/wall
plans only after both canonical package hashes verify. MNS provenance remains
separate from BPK provenance; a valid MENU.BPK cannot satisfy a missing MNS
material dependency.

## Structure2

Structure2 descriptor boundaries are validated against the post-`FFFF` payload
span. The corpus currently proves every nonzero descriptor offset across
LEV00-LEV15 remains in that span. This is a safety and provenance result, not
a pixel decoder. Image payload grammar, palette coupling, animation flags, and
timing remain blocked until proven.

## Structure3 Face Topology

Structure3 face rows retain bounded vertex-index topology only. The parser
counts triangle/quad slots plus each row's one-, two-, three-, or
four-distinct-index form and checks that the accounting covers every valid
face row. This is source-format provenance, not a winding, normal-plane,
transform, material, palette, or draw claim.

It also accounts for each entry-local vertex table as fully or partially
referenced by its bounded face rows. This is only a raw index-coverage result;
it does not establish a surface, visibility, winding, normal-plane, transform,
material, palette, or drawing rule.

The same no-draw receipt also counts connected vertex-index components within
each Structure3 entry. A component joins only distinct indexes that co-occur
in a bounded face row; it is not an edge direction, winding, surface,
normal-plane, transform, material, palette, or drawing claim.

Each entry is also accounted as having zero, one, or multiple such components.
That partition covers the bounded Structure3 entry directory without assigning
any geometric or rendering meaning to the component count.

The receipt additionally retains each entry-local unordered pair of distinct
indexes that co-occurs in a face row, separating distinct pairs from repeated
occurrences. This is row-incidence accounting only, not an edge direction,
winding, surface, normal-plane, transform, material, palette, or drawing rule.

It also partitions those distinct pairs into one-row and multiple-row
occurrences, retaining the maximum observed occurrence count per entry. This
still does not turn a shared pair into an edge or establish winding, surface,
normal-plane, transform, material, palette, or drawing behavior.

The documented face-to-normal ordinal relationship is retained separately:
within each bounded entry, row `n` of the face region is paired only with row
`n` of the count-matched normal region. Across the retail `LEV00.DGN` through
`LEV15.DGN` corpus, this covers 1,144 entries and 18,478 pairs, all within the
existing fixed-point unit tolerance. This remains correspondence provenance;
it does not identify a normal plane, orientation, surface, palette, texture,
or draw operation.

Face rows also retain signed-16.16 face-plane/normal coherence: each stored
normal is checked against its bounded face edges within an exact fixed-point
tolerance, and triangle winding signs are recorded as corpus measurements
only. The parser does not infer a front face, back-face culling rule,
transform, projection, UV grammar, or drawing order. Texture and palette
semantics remain outside this receipt.

## Mesh Semantic Handoff

The renderer receives a single Structure3 mesh-semantic receipt only after
bounded topology, signed fixed-point vectors, and entry-local face/normal
ordinal pairs agree. On the retail `LEV00.DGN` through `LEV15.DGN` corpus this
is evidence completeness, not rendering readiness: the receipt explicitly
requires an original Saturn capture and marks it unavailable. It therefore
blocks normal-plane use, transforms, texture/palette decoding, and draw
commands until a capture ties those operations to the original runtime.

DONE: the startup capture gate now consumes an admitted packet through the
live engine: it retains the canonical loaded `LEVxx.DGN` bytes, extracts the
selected typed face/vertex/normal rows, and copies the exact captured texture
span into engine-owned storage. The copied texture bytes remain opaque and the
engine keeps the real-mesh draw blocker asserted.

TODO: admit a real original-Saturn packet that proves texture, palette,
transform, culling, and draw semantics before any Structure3 source cache can
reach rasterization.

## PRS3 Loader Evidence

`MENU.BPK` exposes bounded PRS3 entry topology, mode, dimensions, packed span,
and a directory-trailer record. The reviewed DMWeb `DecodePRS3` grammar is now
implemented for bounded source inspection: control bits are consumed
least-significant-bit first; a one bit copies one literal byte; a zero bit reads
a 12-bit window position and a four-bit length; and the DMWeb negative-window
adjustment is applied before bounded output.

The implementation is admitted as a byte decoder, not as a Saturn presentation
route. Real `FACE.BIN` decodes all 20 retail 56x56 PRS3 frames and their 64-entry
BGR555 palettes in `test_nexus_v1_face_bin`. `MENU.BPK` remains no-draw because
the decoder result is not yet joined to the original Saturn VDP1 upload,
palette lane, placement, and command order. No synthetic or fallback PRS3
surface is materialized.

## Title/Menu Route Status

The M11 presentation gate now keeps the real `TITLE.CG` reveal drawable while
`MENU.BPK` remains fail-closed pending PRS3 capture evidence, and ACCEPT exits
a completed title rather than trapping the player on a blocked menu route.
Three PRS3/UI decode gates stay intentionally blocked pending original
Saturn evidence:

* `nexus_ui_load_stabg` (STABG.BIN cell decode) — inert until original Saturn
  evidence proves pixel order.
* `nexus_ui_load_face_record` (FACE.BIN startup presentation) —
  decoded bytes remain diagnostic-only; promotion is blocked until the
  original Saturn capture binds the PRS3 output to the VDP1 palette lane,
  placement, and command order.
* Title capture-surface plus Saturn timing/frame capture evidence.

These are class-(c) blocks, not fixture gaps: do not relax probe assertions
against them.

## Saturn VDP1 Capture Gate

DONE: the VDP1 capture schema now requires first and last SH-2 input-read and
output-write sequence numbers. Each interval must start after the first loader
opcode, end before its observed return, and the VDP1 command must follow that
return. The capture still binds only to the exact `MENU.BPK` stream plan and
`DM.BIN` fingerprint; it does not authorize PRS3 decoding or drawing.

TODO: ingest a real Saturn/emulator capture containing those ordered events,
the output fingerprint, and the VDP1 command/source range. Until then there is
no accepted capture and no VDP1 texture route.

## SLEV Task Receipt

DONE: `tests/test_nexus_v1_script_vm.c` consumes every `SLEV00.BIN` through
`SLEV15.BIN` file from `FIRESTAFF_NEXUS_DATA_DIR`. It locks the observed
36-byte SH-2 entry receipt, including the terminal `MOV.L @(disp,PC),R0` in
the fixed `RTS` delay slot, bounded profile accounting, and the runtime block
against condition/action dispatch.

TODO: the delay-slot fact does not establish either literal target's ownership,
task-body opcodes, entry-point ownership, host callbacks, or trigger semantics.
The receipt must not be promoted into a dispatcher without hash-bound Saturn
execution or capture evidence.

## SAL Opaque Prefix Receipt

DONE: the retail `SNDLEV00.SAL` through `SNDLEV15.SAL` corpus shares an exact
33-byte opaque prefix: ASCII `dsp01.EXB`, 23 zero bytes, then byte `0x02`.
`nexus_v1_audio_sal_opaque_prefix_receipt()` records only this common byte
fact, and the corpus test rejects a truncated or altered prefix.

TODO: this is not a SAL container grammar, payload boundary, codec, sample
directory, MAP selector meaning, Saturn driver ABI, or playback binding. The
receipt explicitly blocks decode until original execution or capture evidence
establishes those semantics.

## Host Route

Launcher, runtime, and host receipts preserve whether a static MNS route was
actually consumed. A non-MNS plan requires its own complete current-level
materialization evidence. Missing Structure2 evidence blocks dependent commands
before they reach host drawing.

## Verification

```bash
./build/test_nexus_v1_dgn_geometry_readiness
./build/test_nexus_v1_dgn_material_raster
./build/test_nexus_v1_bpk_surface_class
./build/firestaff_nexus_v1_prs3_loader_media_probe
./build/firestaff_nexus_v1_dgn_material_corpus_probe
FIRESTAFF_NEXUS_DATA_DIR=/path/to/nexus ./build/test_nexus_v1_face_bin
FIRESTAFF_NEXUS_DATA_DIR=/path/to/nexus ./build/test_nexus_v1_script_vm
```
