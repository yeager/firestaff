# Dungeon Master Nexus Technical Reference

> **Status reviewed 2026-09-09.** The retail Japanese CUE/BIN now drives a
> native MAPD title sequence. Menu, HUD, dungeon composition and full
> playability remain open because each needs its own source-bound display
> consumer evidence.

## Scope

Nexus targets the Saturn DMDF/DGN data family. The engine separates disc
discovery, DMDF metadata, DGN geometry, static materials, startup/save routes,
scripts, and audio. A parsable filename is never treated as source identity.

## DGN Geometry

`LEVxx.DGN` parsing begins with typed Structure1B geometry. The renderer uses
decoded floor, ceiling, and wall corner heights instead of flattening the world
into a raw grid. Structure1F records retain verified ownership: direct items,
floor decoration, and floor sensors are separate from Structure1A-bound alcove
and wall records. Opaque fields do not become guessed draw/collision commands.

## Static Materials

Static floor, ceiling, and wall commands consume paired retail `SN_FLOOR.MNS`
and `SN_WALL.MNS` TEXT banks. Both canonical package receipts are required
before decoded pixels reach the viewport. The MNS route has separate provenance
from BPK; missing MNS data never authorizes a prefix import or flat fallback.

Structure2 descriptors are bounded provenance only until their payload and
palette grammar are proved. Commands needing an unproved animated payload are
no-draw.

## MENU.BPK / PRS3

`MENU.BPK` PRS3 entries are inspected for bounded topology, mode, dimensions,
and directory-trailer layout. The reviewed DMWeb `DecodePRS3` byte grammar is
implemented and verified against the real 20-frame `FACE.BIN` corpus. This is
still not a Saturn presentation proof: `MENU.BPK` output remains no-draw until
an original VDP1 capture binds decoded bytes to palette lane, placement, and
command order. No synthetic PRS3 surface is admitted.

## Gameplay Mechanics (Real-Data)

`nexus_v1_mechanics_load_level()` admits the active creature pool and
hash-bound Structure1Fa floor items from authenticated `LEVxx.DGN` records.
Structure1Fb floor decorations remain in the decoded raw DGN model; they are
not registered as Vi altars because the model/aspect-to-altar and Saturn event
joins are unproven. Door, pit, and other event semantics likewise remain
capture-gated. The Structure1Fb declaration in `include/nexus_v1_dgn.h`
(`Nexus_V1_DgnFloorDecor`) describes model/texture fields, not altar ownership;
see `docs/wiki/Nexus-DGN-and-PRS3-Internals.md`, “Structure1B and Related
Families.” `NEXUS_CMD_USE_ITEM` and combat mutations are not admitted for
retail sessions without a Saturn action-owner receipt.

Verification: `firestaff_nexus_v1_mechanics_playability_probe` (real LEV
files) and `firestaff_nexus_v1_mechanics_parity_probe` exercise this pipeline
end to end; `firestaff_nexus_v1_creature_state_determinism_probe` checks
creature-state determinism across runs.

## Startup and Verification

Launcher startup carries title, save, champion, and package/host receipts into
M11. Title readiness alone does not prove DGN rendering, SLEV/SAL sound, or
Saturn timing. The real `TITLE.CG`/MAPD sequence is decoded and rendered
natively when its complete retail plane and palette receipt is admitted.
`MENU.BPK` stays fail-closed awaiting PRS3 capture evidence; ACCEPT exits a
completed title instead of trapping on the blocked menu route, and M11
presentation copies only source-bound material rather than substituting
neutral placeholder colours.

`nexus_v1_title_mapd_real` reads `TITLE.BIN` and `TITLE.CG` through the native
CUE/ISO reader when they are present only in the original Track 1 image. It
uses bounded RAM buffers and does not require an extracted disc tree. The test
source-binds all five MAPD/TIBG maps and their palette receipts. The native
title renderer consumes this bounded map sequence directly; it does not
authorize the separate menu compositor, HUD or dungeon renderer, each of
which still requires its own same-revision VDP consumer evidence.

```bash
cmake --build build --target test_nexus_v1_dgn_geometry_readiness \
  test_nexus_v1_dgn_material_raster test_nexus_v1_bpk_surface_class \
  test_nexus_v1_startup_menu_pc34_compat --parallel
./build/test_nexus_v1_dgn_geometry_readiness
./build/test_nexus_v1_dgn_material_raster
./build/test_nexus_v1_bpk_surface_class
./build/test_nexus_v1_startup_menu_pc34_compat
```

For DGN record ownership, MNS material provenance, Structure2, and PRS3
evidence, see [Nexus DGN and PRS3 Internals](Nexus-DGN-and-PRS3-Internals).
