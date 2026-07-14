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

## PRS3 Loader Evidence

`MENU.BPK` exposes bounded PRS3 entry topology, mode, dimensions, packed span,
and a directory-trailer record. Original `DM.BIN` analysis currently proves
that the PRS3 v1 loader reads payload bytes through `@R12+`, decrements R14,
and uses R2=`0x0100` with R9=`0x0000ff00` in R11 refill/control state.

These facts are deliberately insufficient for decompression. They do not prove
bit order, opcode grammar, literal/back-reference layout, output size handling,
or palette semantics. No synthetic or retail PRS3 surface is materialized from
them.

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
```
