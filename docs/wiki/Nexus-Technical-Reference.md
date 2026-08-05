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
and directory-trailer layout. The reviewed DMWeb `DecodePRS3` byte grammar is
implemented and verified against the real 20-frame `FACE.BIN` corpus. This is
still not a Saturn presentation proof: `MENU.BPK` output remains no-draw until
an original VDP1 capture binds decoded bytes to palette lane, placement, and
command order. No synthetic PRS3 surface is admitted.

## Gameplay Mechanics (Real-Data)

`nexus_v1_mechanics_load_level()` binds the active creature pool, pits,
altars, and doors from authenticated `LEVxx.DGN` records for the loaded map.
Creatures spawn from Structure1A actor records (type, position, and the
Z-rotation byte); unbound or hidden actors cannot move, attack, or take
damage until `nexus_v1_creature_bind_actor_model()` resolves them against MNS
roster metadata. Door open/close animation steps
(`nexus_doors_tick_animation()`) gate passability, and a candidate altar
registry (`nexus_altars_register_tagged()`) supports a fail-closed ritual
action. `NEXUS_CMD_USE_ITEM` consumes leader inventory slots (potions restore
stats; weapons/armor move to the matching equipment slot), and stepping on a
pit/chute square sets `pending_level_change` per ReDMCSB MOVESENS.C
semantics. Champion death auto-promotes the next living party member to
leader (ReDMCSB CHAMPION.C `F0319_CHAMPION_Kill`), matching total-party-death
handling when no successor remains.

Verification: `firestaff_nexus_v1_mechanics_playability_probe` (real LEV
files) and `firestaff_nexus_v1_mechanics_parity_probe` exercise this pipeline
end to end; `firestaff_nexus_v1_creature_state_determinism_probe` checks
creature-state determinism across runs.

## Startup and Verification

Launcher startup carries title, save, champion, and package/host receipts into
M11. Title readiness alone does not prove DGN rendering, SLEV/SAL sound, or
Saturn timing. The real `TITLE.CG` reveal is drawable while `MENU.BPK` stays
fail-closed awaiting PRS3 capture evidence; ACCEPT exits a completed title
instead of trapping on the blocked menu route, and M11 presentation copies
only the source-bound material framebuffer rather than substituting neutral
placeholder colours.

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
