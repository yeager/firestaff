# Dungeon Master Nexus Technical Reference

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
and directory-trailer layout. Firestaff does not materialize either synthetic
or retail PRS3 surfaces until an original decoder is proved from executable or
media evidence. Archive metadata may support launcher diagnostics but is not a
graphics decoder.

## Startup and Verification

Launcher startup carries title, save, champion, and package/host receipts into
M11. Title readiness alone does not prove DGN rendering, SLEV/SAL sound, or
Saturn timing.

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
