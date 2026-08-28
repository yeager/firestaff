# Nexus placeholder and provenance audit

Date: 2026-08-08

This is a source-faithful inventory of retail data, isolated test data, and
what still lacks an authentic Saturn consumer capture. A parser that can read
bytes does not by itself prove VDP1/VDP2 placement, CLUT, HUD, viewport, sound,
or gameplay semantics.

## Verified retail data

- `DM.BIN`, `TM.BIN`, `FACE.BIN`, `FONT256.S2D`, `ITEM.IBS`, `MENU.BPK`,
  `LEV00.DGN`–`LEV15.DGN`, `SLEV00.BIN`–`SLEV15.BIN`,
  `SNDLEV##.SAL/.MAP` and `SDDRVS.TSK` are read from the operator-owned Nexus
  directory and hash-checked before use in real-data probes.
- `ITEM.IBS` is verified with 243 declarations, 223 regular images, and 109
  floor images (`nexus_v1_item_ibs`).
- The `DM.BIN` HUD layout is verified as 80 entries and the hitrect table as 40
  entries. This is source-owned geometry, not proof of Saturn's final
  composition.
- All 16 DGN levels have a source-bound Structure3-face campaign with 18,478
  no-draw targets. The campaign ledger still requires an original Saturn
  capture and permits no decoder or renderer.

## Isolated synthetic/test paths

- The BPX0/BPX3 contracts in `nexus_v1_bpx_bpk.c` are used only in explicit
  probe/test targets; CMake excludes the file from Nexus production sources.
- The DGN-material probe uses a local synthetic pose. It is no longer in
  `include/nexus_v1_game.h`; the coordinates exist only in
  `tests/test_nexus_v1_dgn_material_raster.c`.
- Fixture decoders for S2D text, lighting, PRS3 contracts, and legacy mechanics
  remain explicit test paths. They must not deliver M11 pixels or Nexus runtime
  state.

## Blocked no-op/fallback boundaries

`NEXUS_V1_RF_NO_3D_ENGINE` and `NX_UNSUPPFEAT_3D_RASTERIZER` in the standard
profile mean a Firestaff admission gate, not that retail Nexus is
a 2D game or that DGN lacks 3D geometry. The authenticated DGN corpus contains
Structure1A/1F/2/3 geometry; Firestaff may activate a 3D raster path only when
the same Saturn capture binds transform, culling, VDP1 command, texture, and
CLUT to a concrete face. Until then, the standard profile's no-3D status is the
source-faithful safe path and must not be replaced by a host or synthetic
renderer.

- `nexus_v1_drops.c` fabricates no DM1-shaped loot or gold table.
- `nexus_v1_item_use.c` does not modify inventory or champion state; ITEM.IBS
  proves declarations/icons/materials, not Saturn action dispatch.
- `nexus_v1_title_sequence.c` contains host scheduling times, but M11's
  title/warning surface requires authentic capture. Timing metadata must not
  be presented as retail animation.
- `nexus_v1_sound.c` does not decode SAL to host PCM. SLEV dispatch, MAP event,
  SAL format, SDDRVS handoff, and playback require a common Saturn/SCSP/68K
  execution capture.
- The latest authentic SCSP corridor verifies 16 SLEV, 16 MAP, and 16 SAL files
  plus SDDRVS against retail hashes. It contains four sound-CPU mailbox writes
  with raw value `0x02` and five main-CPU entries (`0x0200:3`, `0x0002:2`). PC
  `0x3224` is byte-exactly bound to SDDRVS offset `0x2220` and its
  command-byte→driver-state/SCSP register handler. Event selector, MAP row,
  SAL sample, codec, and playback remain `unproven`/`blocked`; no semantics are
  assigned to other observed PCs.
- DGN Structure3, ITEM/VDP1 textures, CLUT, HUD/viewport composition, and
  startup/menu presentation remain no-draw or capture-gated.

## Runtime capture status

The external directory now contains 38 validated `runtime-vdp12.raw` files.
The inventory counts 12 reset/no-layer, 14 RBG0, 100 NBG1, and 14 other active
VDP2 frames. The new eight-frame European capture
`run-french-hold-starta-skip18000` has eight active VDP1-state frames and
`NBG1` as its only active VDP2 layer. All still have
`asset_consumer_identity=unbound` and
`startup_menu_hud_viewport_identity=unbound`.

The new capture provides a stronger European runtime observation, but not an
asset binding: VDP1's type-2 source span `0x63e00..0x6bf80` (33,280 bytes)
matches neither verified MNS surfaces, DGN Structure2 surfaces, nor the retail
file's raw-byte domain. VDP2 shows NBG1 bitmap mode with `CHCTLA=0x1211`, but
no exact TITLE/MENU/STABG/DGN palette match. It is therefore useful as negative
source-join evidence, not as state proof for startup, menu, HUD, or viewport.

The authentic active VDP1 witnesses prove Saturn hardware state and
command-to-VRAM corridors, but their source span has no exact binding yet to
retail MNS, DGN, ITEM, MENU, TITLE, or CLUT. A reset capture does not prove
startup or menu.

The capture inventory also shows `manifest_binding=missing` for older artifacts
without a raw hash. A launcher-produced file may be listed as `verified` only
when the manifest's `raw_sha256` and `raw_bytes` match exactly. A manifest with
an incomplete or incorrect raw hash becomes `mismatch` and cannot be used as
presentation evidence.

`writer-code.trace` and `vdp1-writes.trace` are also separate external
diagnostic artifacts. They must not be connected to a `runtime-vdp12.raw` from
another run merely because PC, VRAM address, or byte prefix happen to match.
The current writer receipt (`PC=0x06013098`, `VRAM=0x47c00`) therefore still
has `runtime_code_source_identity=unbound`; same-run identity, relocated/
decompressed code ownership, and the retail asset must first be shown in a
common capture package.

## Verification commands

```sh
FIRESTAFF_NEXUS_DATA_DIR=/Users/bosse/.firestaff/data/nexus \
  ctest --test-dir build --output-on-failure -R \
  'nexus_v1_(startup_menu_source|title_mapd_real|dgn_geometry_readiness|dgn_face_mesh_corpus|startup_media_gate|slev_task_corpus_receipt|item_ibs|sal_map_corpus)$'

python3 tools/verify_nexus_production_source_boundary.py
python3 scripts/analyze_nexus_capture_inventory.py \
  /Volumes/Extern-disk/nexus-saturn-capture
```

As long as a required consumer capture is missing, the source-faithful correct
action is to keep the gate closed, not replace it with synthetic pixels,
guessed timing, DM1 loot, or host PCM.
