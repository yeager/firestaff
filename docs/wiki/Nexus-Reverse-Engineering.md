# DM Nexus (Sega Saturn) — Reverse Engineering Wiki

DM Nexus is the Sega Saturn Dungeon Master release reimplemented by Firestaff.
Unlike DM1/CSB (ReDMCSB) and DM2 (skproject), **no reference source code
exists for DM Nexus**. Every format documented here was derived by direct
inspection of the retail Saturn disc image and the `DM.BIN` executable —
hex analysis, structural admission (byte-exact partitioning with provenance
receipts), and SHA-256/FNV-1a64 hash pinning against the verified retail
assets. Naming follows Firestaff's `nexus_v1_*` / `nexus_v2_*` /
`nexus_v22_*` convention (see project `CLAUDE.md`).

This page is a reference index into `src/nexus/`, `include/nexus_*.h`, and
`parity-evidence/` — it does not replace reading those files.

---

## 1. Module Registry

`src/nexus/` contains **182** `.c` files, matched 1:1 by **182** headers in
`include/nexus_*.h`. Grouped by subsystem:

### Disc / archive / boot admission
| File | Purpose |
|---|---|
| `nexus_v1_0dmstrt_structure_admission.c` | `0DMSTRT.BIN` boot-library byte-partition admission (regions, gaps, fixup tables) |
| `nexus_v1_boot_profile.c` | Saturn boot profile |
| `nexus_v1_iso_reader.c` | ISO9660/CD sector reader for the Saturn disc image |
| `nexus_v1_launcher.c` | Game launcher/bootstrap |
| `nexus_v1_saturn_card_discovery.c` | Saturn backup RAM cartridge/card discovery |
| `nexus_v1_saturn_save_capture.c` | Saturn save-state capture |
| `nexus_v1_lev_corpus_discovery.c` | `LEVxx.DGN` corpus discovery/enumeration |
| `nexus_v1_slev_sal_asset_discovery.c` | `SLEV`/`SAL` audio-asset discovery |
| `nexus_v1_sndlev_map_provenance.c` | `SN_*` sound/level map provenance |
| `nexus_v1_saturn_capture_campaign_import.c`, `nexus_v1_multi_level_capture_campaign_launcher.c` | Batch multi-level capture campaign tooling |
| `nexus_v1_compat_gate.c` | Compatibility/feature gating |
| `nexus_v1_rest.c`, `nexus_v1_res.c` | Resource-table admission |
| `nexus_v1_raw_bin.c` | Generic raw `.BIN` binding |
| `nexus_v1_stmp.c` | STMP asset admission |

### BPK archive / PRS3 compression (MENU.BPK)
| File | Purpose |
|---|---|
| `nexus_v1_bpk_archive.c` | BPPK/BMPD directory parsing, entry prefixes, surface classes, palette trailer, runtime render/decode/upload routing |
| `nexus_v1_bppk.c`, `nexus_v1_bpx_bpk.c` | BPPK container variants |
| `nexus_v1_prs3.c`, `nexus_v1_prs3_decode.c`, `nexus_v1_prs3_decoder_admission.c` | PRS3 sub-header inspection and decode gate |
| `nexus_v1_prs3_capture_trace_schema.c` | PRS3 capture-trace schema |
| `nexus_v1_prs3_dgn_placement_adapter.c` | Bridges PRS3 surfaces to DGN placement |
| `nexus_v1_prs3_execution_capture_admission.c` | Execution capture admission |
| `nexus_v1_prs3_loader_control_flow.c` | Loader control-flow evidence |
| `nexus_v1_prs3_multi_capture_adjudicator.c` | Multi-capture cross-adjudication |
| `nexus_v1_prs3_original_execution_import.c` | Original Saturn execution trace import |
| `nexus_v1_prs3_sh2_subset_trace.c` | SH-2 instruction-subset trace evidence |
| `nexus_v1_prs3_structure2_abi.c`, `nexus_v1_prs3_structure2_intake.c` | Structure2 ABI/intake for PRS3-bearing descriptors |
| `nexus_v1_prs3_vdp1_capture_replay.c`, `nexus_v1_prs1_vdp1_consumer_evidence.c` | VDP1 (Saturn sprite processor) capture replay/consumer evidence |

### DGN dungeon geometry
| File | Purpose |
|---|---|
| `nexus_v1_dgn.c` | Top-level DGN decode (grid, textures, doors, floor/wall records) |
| `nexus_v1_dgn_mesh.c` | DGN-to-mesh conversion |
| `nexus_v1_dgn_texture_decode.c` | DGN texture decode |
| `nexus_v1_dgn_face_material_provenance.c` | Face/material provenance binding |
| `nexus_v1_dgn_multi_level_capture_adjudicator.c` | Cross-level DGN capture adjudication |
| `nexus_v1_dgn_runtime_materialization.c` | Runtime materialization of decoded DGN data |
| `nexus_v1_dgn_scene_runtime_plan.c` | Scene runtime plan assembly |
| `nexus_v1_dungeon.c` | Dungeon state container |
| `nexus_v1_squares.c` | Per-square state |
| `nexus_v1_dmdf_model.c` | DMDF 3D model container (shared by DGN Structure3 and MNS) |

### Structure1A / Structure1F / Structure3 admission (DGN sub-records)
25 dedicated admission modules, one per DGN record class:
`nexus_v1_structure1a_field_admission.c`, `nexus_v1_structure1a_target_admission.c`,
`nexus_v1_structure1f_alcove_admission.c`, `nexus_v1_structure1f_alcove_structure3_row_admission.c`,
`nexus_v1_structure1f_corpus_capture_plan.c`, `nexus_v1_structure1f_directory_admission.c`,
`nexus_v1_structure1f_floor_decoration_admission.c`, `nexus_v1_structure1f_floor_sensor_admission.c`,
`nexus_v1_structure1f_item_admission.c`, `nexus_v1_structure1f_payload_owner_admission.c`,
`nexus_v1_structure1f_placement_binding.c`, `nexus_v1_structure1f_wall_decoration_admission.c`,
`nexus_v1_structure1f_wall_decoration_structure3_row_admission.c`,
`nexus_v1_structure3_capture_manifest.c`, `nexus_v1_structure3_entry_admission.c`,
`nexus_v1_structure3_entry_tag_admission.c`, `nexus_v1_structure3_face_admission.c`,
`nexus_v1_structure3_face_index_prefix_admission.c`, `nexus_v1_structure3_face_tail_admission.c`,
`nexus_v1_structure3_face_texturing_capture_plan.c`, `nexus_v1_structure3_face_vertex_admission.c`,
`nexus_v1_structure3_face_vertex_set_admission.c`, `nexus_v1_structure3_first_region_row_admission.c`,
`nexus_v1_structure3_normal_admission.c`, `nexus_v1_structure3_second_region_row_admission.c`,
`nexus_v1_structure3_target_admission.c`, `nexus_v1_structure3_third_region_row_admission.c`.

### Materials / models (MNS, FACE, SMAP)
| File | Purpose |
|---|---|
| `nexus_v1_mns.c` | MNS mesh/skeleton/MOTN-animation decode (creature/model bank) |
| `nexus_v1_face_bin.c` | `FACE.BIN` champion portrait admission |
| `nexus_v1_smap_bin.c` | `SMAP` texture-atlas admission |
| `nexus_v1_owner_material_capture_admission.c`, `nexus_v1_owner_material_capture_artifact_preflight.c`, `nexus_v1_owner_material_capture_campaign.c`, `nexus_v1_owner_material_capture_campaign_artifact.c` | SN_FLOOR/SN_WALL material-owner capture pipeline |
| `nexus_v1_palette.c` | Palette admission |
| `nexus_v22_shape_cache_pc34.c`, `nexus_v22_modern_assets_pc34.c` | Modern shape cache / asset bridge (V2.2 presentation layer) |

### Title / boot screens / fonts
| File | Purpose |
|---|---|
| `nexus_v1_title.c`, `nexus_v1_title_cg.c`, `nexus_v1_title_sequence.c` | Title screen (`TITLE.CG`) and boot sequence |
| `nexus_v1_title_cnfd_payload_admission.c`, `nexus_v1_title_dgt2_pp_payload_admission.c`, `nexus_v1_title_mapd_tibg_admission.c`, `nexus_v1_title_res_corpus_receipt.c`, `nexus_v1_title_titl_pp_payload_admission.c` | Title-asset sub-record admission (CNFD/DGT2/MAPD/TIBG/TITL payloads) |
| `nexus_v1_warning_dgt2_*` (6 files) | Sega/legal warning-screen DGT2 descriptor/payload/presentation pipeline |
| `nexus_v1_logobg_dg2.c` | Logo background |
| `nexus_v1_startup_layout.c`, `nexus_v1_startup_menu.c` | Startup menu layout |
| `nexus_v1_font012.c`, `nexus_v1_font_s2d.c`, `nexus_v1_saturn_font.c` | Bitmap fonts |
| `nexus_v1_font256_s2d_admission.c`, `nexus_v1_font256_s2d_first_section_capture.c`, `nexus_v1_font256_s2d_section_corpus_receipt.c`, `nexus_v1_font256_s2d_section_witness.c`, `nexus_v1_font256_s2d_subrecord_grammar.c` | 256-glyph S2D font-section sub-record admission |
| `nexus_v1_s2d_glyph_decode.c`, `nexus_v1_s2d_text_layout.c` | S2D glyph decode/text layout |
| `nexus_v1_rlowfix_text.c`, `nexus_v1_screen_text.c`, `nexus_v1_text.c` | Text tables and rendering |

### Gameplay systems
| File | Purpose |
|---|---|
| `nexus_v1_champions.c`, `nexus_v1_champion_panel.c` | Champion roster and UI panel |
| `nexus_v1_combat.c` | Combat resolution |
| `nexus_v1_creatures.c`, `nexus_v1_creature_names.c` | Creature roster, spawn, AI tick, DGN actor binding |
| `nexus_v1_containers.c`, `nexus_v1_inventory.c`, `nexus_v1_item_use.c`, `nexus_v1_item_ibs.c` | Items, inventory, `ITEM.IBS` database |
| `nexus_v1_drops.c`, `nexus_v1_throw.c`, `nexus_v1_projectiles.c` | Item drop/throw/projectile physics |
| `nexus_v1_doors.c`, `nexus_v1_switches.c`, `nexus_v1_triggers.c`, `nexus_v1_traps.c`, `nexus_v1_fountain.c` | Dungeon interactables |
| `nexus_v1_magic.c`, `nexus_v1_spell_effects.c` | Spellcasting |
| `nexus_v1_experience.c`, `nexus_v1_encumbrance.c`, `nexus_v1_hunger.c`, `nexus_v1_status.c`, `nexus_v1_damage_indicator.c` | Champion stat systems |
| `nexus_v1_formation.c`, `nexus_v1_movement.c`, `nexus_v1_action_timer.c` | Party formation/movement/action timers |
| `nexus_v1_mechanics.c` (+ `nexus_v1_mechanics_fwd.h`) | Core rule mechanics |
| `nexus_v1_shop.c`, `nexus_v1_dialogue.c`, `nexus_v1_messages.c` | Shops and NPC dialogue |
| `nexus_v1_gameover.c`, `nexus_v1_save_load.c`, `nexus_v1_level_transition.c`, `nexus_v1_spawner.c` | Game flow |
| `nexus_v1_light.c`, `nexus_v1_light_overflow.c`, `nexus_v1_light_runtime.c` | Lighting model |
| `nexus_v1_automap.c` | Automap |
| `nexus_v1_script_vm.c` | Level scripting VM |
| `nexus_v1_world.c`, `nexus_v1_game.c`, `nexus_v1_engine.c` | Top-level world/game/engine state |
| `nexus_v1_sound.c`, `nexus_v1_audio_receipt.c`, `nexus_v1_sal_capture_plan.c`, `nexus_v1_sal_container_provenance.c`, `nexus_v1_sal_payload_capture_admission.c`, `nexus_v1_slev_task_body_capture_plan.c` | SAL/SLEV audio pipeline |

### Rendering / presentation (V1 raster, V2 modern, V22 bridge)
| File | Purpose |
|---|---|
| `nexus_v1_rasterizer.c`, `nexus_v1_viewport.c`, `nexus_v1_math3d.c` | Original-style raster viewport and 3D math |
| `nexus_v1_ui_surfaces.c`, `nexus_v1_hud_layout.c`, `nexus_v1_hud_hit_rects.c`, `nexus_v1_click_route.c` | HUD/UI surfaces and hit-testing |
| `nexus_v2_render_pipeline.c` | Modern render pipeline |
| `nexus_v2_hud_overlay.c`, `nexus_v2_hud_runtime.c`, `nexus_v2_hud_runtime_noop.c` | Modern HUD overlay |
| `nexus_v2_lighting.c`, `nexus_v2_lighting_runtime.c`, `nexus_v2_atmosphere.c`, `nexus_v2_particles.c` | Modern lighting/atmosphere/particles |
| `nexus_v2_smooth_movement.c`, `nexus_v2_smooth_movement_runtime.c` | Smooth (non-tile-snapped) movement presentation |
| `nexus_v2_touch_controller_affordance.c`, `nexus_v2_touch_runtime.c` | Touch input affordances |
| `nexus_v2_upscaler.c`, `nexus_v2_config.c` | Upscaling and presentation config |
| `nexus_v22_inplace_draw_pc34.c`, `nexus_v22_phase_gate_pc34.c` (as `nexus_v2_phase_gate_pc34.c`) | V2.2 bridge draw path and phase gating |

Total: 182 `.c` / 182 `.h` pairs (`ls src/nexus/*.c | wc -l` = 182; `ls
include/nexus_*.h | wc -l` = 182).

---

## 2. Saturn Disc Structure

DM Nexus ships on a Sega Saturn CD (ISO9660-derived layout, read via
`nexus_v1_iso_reader.c`). Key asset classes identified on the retail disc:

| Path pattern | Format | Consumer module |
|---|---|---|
| `LEVxx.DGN` | Per-level dungeon geometry (grid, actors, items, sensors) | `nexus_v1_dgn.c`, Structure1A/1F/3 admission modules |
| `SN_FLOOR.MNS` | Floor material bank (MNS mesh/texture container used for floor tiles) | `nexus_v1_mns.c`, `nexus_v1_owner_material_capture_*.c` |
| `SN_WALL.MNS` | Wall material bank | `nexus_v1_mns.c`, `nexus_v1_owner_material_capture_*.c` |
| `MENU.BPK` | BPPK-framed archive of PRS3-compressed menu/UI bitmaps, ending in a `PALT` palette trailer | `nexus_v1_bpk_archive.c`, `nexus_v1_bppk.c` |
| `TITLE.CG` | Title screen image container | `nexus_v1_title_cg.c` |
| `ITEM.IBS` | Item database: declarations, inventory/floor images, palettes | `nexus_v1_item_ibs.c` |
| `*.SMAP` | Texture atlas / sprite map binary | `nexus_v1_smap_bin.c` |
| `FACE.BIN` | Champion portrait bitmaps | `nexus_v1_face_bin.c` |
| `*.SAL` / `SLEVxx` | Audio containers (streamed level/effect audio) | `nexus_v1_sal_*.c`, `nexus_v1_slev_sal_asset_discovery.c`, `nexus_v1_sndlev_map_provenance.c` |
| `0DMSTRT.BIN` | Saturn boot-library image (39516 bytes, SHA-256 pinned) | `nexus_v1_0dmstrt_structure_admission.c` |
| `RLOWFIX.BIN` | Creature stat table (`CRET` records) and other fixed low-memory tables | `nexus_v1_creatures.c` (`nexus_v1_creatures_load_cret`) |
| `DM.BIN` | Main SH-2 executable; source of extracted combat/AI data tables | `nexus_v1_creatures.h` combat table getters |

`0DMSTRT.BIN` provenance (from `nexus_v1_0dmstrt_structure_admission.h`):
exact 39516-byte size, SHA-256 `8a026f15...d20b6`, and a fully-covering
8-span byte partition (2 dense regions, 3 zero gaps, a tail ISO-style
descriptor stamped `GFS_SBL`/`CD001`, and 23 tagged fixup entries with a
repeating `0x0601` tag). The module explicitly binds **provenance only** —
no byte is assigned instruction/data/relocation semantics, and no load or
execution route is permitted from it.

---

## 3. DGN File Format

Defined in `include/nexus_v1_dgn.h` and elaborated across the
`nexus_v1_structure1a_*` / `nexus_v1_structure1f_*` / `nexus_v1_structure3_*`
admission modules plus `include/nexus_v1_0dmstrt_structure_admission.h`
(admission-pattern reference for byte-exact receipts).

### Layout constants
| Constant | Value | Meaning |
|---|---|---|
| `NEXUS_DGN_BLOCK_SIZE` | 2048 | Disc sector/block granularity |
| `NEXUS_DGN_GRID_SIZE` | 64 | Level grid is 64×64 cells |
| `NEXUS_DGN_CELL_SIZE` | 8 bytes | Per-cell grid record |
| `NEXUS_DGN_MODEL_REF_SIZE` | 24 | Structure3 model reference record |
| `NEXUS_DGN_TEX_DESC_SIZE` | 20 | Texture descriptor size |

### `Nexus_V1_DgnDecodeResult` (top-level decode)
Tracks three logical sections per level: **s1** (Structure1, geometry/grid),
**s2** (Structure2, secondary descriptors), **s3** (Structure3, 3D models),
each with a block offset/count and byte size, plus derived sub-offsets
`s1b_offset` .. `s1g_offset` for the Structure1 sub-records, texture/door/
model counts, and grid/section content hashes (`grid_hash`, `s2_hash`) for
change detection.

### Structure1A — actors, alcoves, walls (grid cells)
`Nexus_V1_DgnCell` (per grid cell): `floor_word`, `height` (signed),
`model_ref` (link into Structure3), plus derived fields resolved during
decode: `floor_tex_index`, `floor_rotation`, `floor_flip_x/y`, `slope`,
`ceiling_tex_sel`, `has_door`. Actor/model references are carried via
`Nexus_V1_DgnModelRef` (`flags`, `model_index`, `rotation`).
`nexus_v1_structure1a_field_admission.c` / `nexus_v1_structure1a_target_admission.c`
admit the raw field layout and per-record target validation ahead of decode.

### Structure1B — geometry (floor/ceiling/wall corner heights)
Represented through the same `Nexus_V1_DgnCell` grid plus texture
descriptors (`Nexus_V1_DgnTexDesc`: `image_id`, `encoding`, `palette_id`,
`width`/`height`, `image_offset`, `palette_offset`). `Nexus_V1_DgnDoor`
records (`y`, `x`, `flags`, `orientation_and_index`, `model_index`, `width`)
sit alongside the grid for door placement.

### Structure1F — items, floor decorations, floor/wall sensors, alcoves
Six fixed-size sub-record types, each with a distinguishing leading marker
byte and a dedicated admission module:

| Struct | Marker | Size | Fields |
|---|---|---|---|
| `Nexus_V1_DgnFloorItem` (1Fa) | `0x10` | 8 B | x, y, location (NW/NE/SE/SW/Center), `item_id` (ITEM.IBS index), magic attr1/attr2 |
| `Nexus_V1_DgnFloorDecor` (1Fb) | `0x11` | 12 B | x, y, signed offset_x/y, `model_index`, `rotation`, `type` (0x03=model, 0x82=texture), tex width/height |
| `Nexus_V1_DgnFloorSensor` (1Fc) | `0x12` | 16 B | x, y, enabled/disabled model refs, active_width/height (0..80), `sensor_type`, dest x/y/orientation |
| `Nexus_V1_DgnAlcove` (1Fd) | `0x20` | 12 B | `face_number`, `model_ref` (Structure1A index), rotation_y, horiz/vert pos, `item_id` (0xFF=empty) |
| `Nexus_V1_DgnWallDecor` (1Fe) | `0x21` | 12 B | `face_number`, `model_ref`, rotation_y, horiz/vert pos, `aspect`, `decor_type`, tex width/height |
| `Nexus_V1_DgnWallSensor` (1Ff) | `0x22` | 16 B | `face_number`, `model_ref`, `aspect_disabled/enabled`, `sensor_type` (0x10/0x13=door button, 0x60/0x63=champion, 0x8B=inscription), `trigger_item` |

Each type has directory/corpus/placement admission modules
(`nexus_v1_structure1f_directory_admission.c`,
`nexus_v1_structure1f_corpus_capture_plan.c`,
`nexus_v1_structure1f_placement_binding.c`,
`nexus_v1_structure1f_payload_owner_admission.c`) that bound record counts
and offsets against the disc-observed archive before decode is trusted.

### Structure2 descriptors
Secondary per-level descriptor block (`s2_block_offset/count/data_size`,
`s2_hash` in `Nexus_V1_DgnDecodeResult`). PRS3-side ABI/intake for
Structure2 payloads is handled by `nexus_v1_prs3_structure2_abi.c` and
`nexus_v1_prs3_structure2_intake.c` since some Structure2 content is
PRS3-compressed.

### Structure3 — 3D model geometry
27 admission modules (`nexus_v1_structure3_*`) admit: entry directory
(`_entry_admission`), entry tags (`_entry_tag_admission`), face records
(`_face_admission`, `_face_index_prefix_admission`, `_face_tail_admission`,
`_face_texturing_capture_plan`, `_face_vertex_admission`,
`_face_vertex_set_admission`), region rows (`_first/_second/_third_region_row_admission`),
normals (`_normal_admission`), and per-record targets
(`_target_admission`), plus a capture manifest
(`nexus_v1_structure3_capture_manifest.c`) tying them together. This is the
mesh/model bank referenced by Structure1A `model_ref` and Structure1F
`model_index`/`model_ref` fields, and by `Nexus_V1_DgnModelRef`.

---

## 4. BPK Archive Format (MENU.BPK)

Documented in `include/nexus_v1_bpk_archive.h` from real MENU.BPK byte
inspection (parity pass1082-1084).

### Container magics
| Constant | Value | Meaning |
|---|---|---|
| `NEXUS_V1_BPK_MAGIC_BPPK` | `'BPPK'` | Outer archive magic |
| `NEXUS_V1_BPK_MAGIC_BMPD` | `'BMPD'` | Directory/bitmap-data section magic |
| `NEXUS_V1_BPK_MAGIC_PRS3` | `'PRS3'` | Per-entry compressed-payload magic |
| `NEXUS_V1_BPK_MAGIC_PALT` | `'PALT'` | End-of-archive palette trailer magic |

### `BpkArchiveInfo` / `BpkEntry` / `BpkEntryPrefix`
`Nexus_V1_BpkArchiveInfo` records outer/BMPD sizes and candidate-offset
counts. `Nexus_V1_BpkEntry` holds per-entry offset/size/payload
offset+size and a `has_prs3` flag. Every entry begins with a fixed
**20-byte prefix** (`NEXUS_V1_BPK_ENTRY_PREFIX_BYTES`), captured verbatim in
`Nexus_V1_BpkEntryPrefix.raw[20]`:

```
bytes  0..4  : unknown u32 (likely compressed-data CRC32-class hash)
bytes  4..8  : unknown u32 (likely original-data hash)
bytes  8..12 : unknown u32 (secondary hash or trailer marker)
bytes 12..14 : width (BE uint16), cross-checked against PRS3+8 pixel count
byte  14     : reserved (0x00 observed)
byte  15     : height (BE uint8)
bytes 16..19 : reserved/zero, except byte 19 = pixel-mode flag
```

### PRS3 compression
Every PRS3 sub-header is 12 bytes: magic `'PRS3'`, a constant BE version
word `0x00000001`, then a BE `pixel_count` (== width × height in every
observed entry). **The PRS3 compression algorithm itself remains
unsupported/undecoded** — the header defines an extensive ladder of
evidence-only probes (never a working decoder) to characterize it without
claiming semantics:

- `nexus_v1_bpk_archive_prs3_payload_evidence` — leading BE u32 tracks
  payload size closely (`header_minus_payload` typically 4-7 bytes),
  byte-frequency and 4-quadrant byte-class histograms per entry.
- `nexus_v1_bpk_archive_prs3_stream_plan` / `_prs3_compression_descriptor` —
  bounded, source-locked framing of the compressed span (offset/length/hash)
  without decoding it.
- `nexus_v1_bpk_archive_prs3_candidate_evidence[_with_bit_order]` — trial
  literal/back-reference opcode grammar evaluated in both LSB-first and
  MSB-first control-bit orders, diagnostic only.
- `nexus_v1_bpk_archive_prs3_framing_evidence` / `_prs3_framed_decode_evidence`
  — compares the post-header BE/LE word against the directory-bounded
  stream span; exact-completion trials of the same trial grammar.
- `nexus_v1_bpk_archive_prs3_opcode_prefix_witness` — walks a bounded
  number of trial opcodes, recording consumed control/operand bytes.
- `nexus_v1_bpk_archive_prs3_decoded_output_proof_gate` — the terminal gate:
  even a caller-supplied decoded sidecar matching by exact byte count and
  FNV-1a64 **cannot** authorize runtime upload without independent original
  Saturn opcode-grammar provenance (`decoder_promoted` stays 0 throughout
  every evidence path in this header).

All decode/render/upload routing enums (`Nexus_V1_BpkRuntimeRenderRoute`,
`Nexus_V1_BpkRuntimeDecodeRoute`, `Nexus_V1_BpkRuntimeUploadRoute`) have
explicit `BLOCKED_PRS3` states — the engine fails closed on PRS3 content
system-wide until the codec is proven.

### Surface classes
`Nexus_V1_BpkSurfaceClass` maps the mode byte (offset 19) to a pixel
format:

| Mode value | Surface class | bpp |
|---|---|---|
| 6 | `INDEXED_8BPP` | 1 |
| 14 | `RGB565` | 2 |
| 22 | `RGB888` | 3 |
| 30 | `RGBA8888` | 4 |
| 10 | `DIRECTORY_TRAILER` (unique to entry 0) | 0 |
| other | `UNKNOWN` | 0 |

`nexus_v1_bpk_mode_to_surface_class()` / `nexus_v1_bpk_mode_to_bpp()`
implement this lookup. Stored (non-PRS3) entries can be fully extracted via
`nexus_v1_bpk_archive_extract_stored_surface()`; PRS3 entries are
structurally decoded (`nexus_v1_bpk_archive_decode_surface`) but remain
blocked from renderer handoff.

### Palette trailer
The canonical retail MENU.BPK ends with a bounded `PALT` record: BE record
byte count, a 256-entry count, then 512 opaque bytes (`Nexus_V1_BpkPaletteTrailerReceipt`).
The parser retains only source framing — it does not assign colour format
or CLUT semantics.

---

## 5. MNS Material Format

Defined in `include/nexus_v1_mns.h`. MNS is a joint-hierarchy 3D mesh +
skeletal-animation container used for both creature models (per
`Nexus_CreatureType.model_file`, e.g. `"SCORPION.MNS"`) and the
`SN_FLOOR.MNS` / `SN_WALL.MNS` material banks.

| Constant | Value |
|---|---|
| `NEXUS_MNS_MAGIC` | `'DFDM'` (DMDF container tag, LE) |
| `NEXUS_MNS_MOTN_MAGIC` | `'NTOM'` (animation section) |
| `NEXUS_MNS_TEXT_MAGIC` | `'TXET'` (texture section) |
| `NEXUS_MNS_JOINT_SIZE` | 52 bytes |
| `NEXUS_MNS_MESH_DESC_SIZE` | 24 bytes |
| `NEXUS_MNS_VERTEX_SIZE` | 12 bytes |
| `NEXUS_MNS_FACE_SIZE` | 12 bytes |

Structure: a joint hierarchy (`Nexus_V1_MnsJoint`: origin xyz, mesh/sibling/
child offsets, optional embedded `Nexus_V1_MnsMesh`), each mesh holding
vertex (`Nexus_V1_MnsVertex`, int32 xyz) and face (`Nexus_V1_MnsFace`:
up to 4 vertex indices, quad flag, flip flags, texture index) arrays, plus
a texture table (`Nexus_V1_MnsTextureDesc`). Animation is stored as
`MOTN` keyframe tables — up to 8 tables of up to 64 frames
(`Nexus_V1_MnsMotnFrame`: duration in 30fps units, per-joint x/y/z
rotation) — driven at runtime through `Nexus_V1_MnsAnimState` (table/frame
index, tick accumulator, loop/finished flags), sampled into a
`Nexus_V1_MnsPose` and applied to mesh vertices via
`nexus_v1_mns_anim_transform_vertices()`.

`nexus_v1_owner_material_capture_admission.c` and its `_artifact_preflight`,
`_campaign`, `_campaign_artifact` siblings implement the byte-exact
provenance capture pipeline that binds `SN_FLOOR.MNS`/`SN_WALL.MNS` bytes
to an "owner" (a specific level/material role) before the runtime trusts
them as floor/wall materials.

---

## 6. Creature and Combat System

`include/nexus_v1_creatures.h` / `src/nexus/nexus_v1_creatures.c` and
`include/nexus_v1_combat.h` / `src/nexus/nexus_v1_combat.c`.

### Roster and stats
`Nexus_CreatureType`: name, `model_file` (MNS path), health/attack/defense/
speed, `experience_value`, plus fields decoded straight from the `CRET`
record (`RLOWFIX.BIN`, 96 bytes/creature, 30 creatures,
`NEXUS_CRET_RECORD_SIZE`/`NEXUS_CRET_COUNT`): `detection_range`,
`ai_type` (byte 0), `attack_count` (byte 1), `behav_flags` (byte 3),
`ranged_type` (byte 4), `poison` (byte 13). `cret_raw[96]` retains the
whole raw record; `cret_bound` gates whether it was actually sourced from
`RLOWFIX.BIN`. `ai_func_ptr` is an SH-2 function pointer extracted from
`DM.BIN` offset `0x0383A8` (the creature AI dispatch table, including its
leading `0xFFFFFFFF` sentinel), retained as
raw provenance rather than executed.

`nexus_v1_creatures_load_cret()` reads the CRET table at `RLOWFIX.BIN`
offset `0xF2B4` and wires HP/attack/defense/speed/etc. into the roster.

### DGN actor binding (evidence-gated)
`Nexus_Creature` instances spawned from real dungeon data carry
`actor_ref_bound` (1 only for an authenticated Structure1A record with kind
byte `01h`/`02h` and a unique Structure1B owner cell), `hidden` (mirrors
the Structure1B invisible-by-default bit — DMWeb: set for the Grey Lord on
LEV1.DGN), `structure3_model_index`, and `model_signature` (FNV-1a64 of the
extracted Structure3 mesh). Binding a roster type to an actor model is
explicit and fail-closed:

- `nexus_v1_creature_bind_actor_model()` registers a
  `(model_signature, structure3_model_index) -> type_index` entry in
  `Nexus_V1_CreatureActorBinding[NEXUS_MAX_ACTOR_BINDINGS]`.
- `nexus_v1_creature_spawn_actor()` spawns from an authenticated DGN
  record; if no proven binding exists yet the actor spawns with
  `type_index == -1` (no AI, no combat) until
  `nexus_v1_creature_rebind_unbound()` resolves it.
- `real_actor_spawn_count` tracks live DGN-sourced spawns; a non-zero count
  blocks synthetic/fixture creature spawns from being promoted.

MNS metadata binding is separately gated:
`nexus_v1_creature_bind_mns_metadata()` opens the named `*.MNS` file,
requires the DMDF magic (via `nexus_v1_dmdf_model.c`), and only then sets
`mns_bound`/`mns_size`/`mns_fnv1a64`.

### Combat data tables (extracted from DM.BIN)
`nexus_v1_combat.c`/`.h` expose read-only accessors over static tables
lifted directly from `DM.BIN yam\cresub.c` region `0x03B5A0`-`0x03B620`
(plus a couple at `0x0604B5xx`): attack-type permutation (8),
XP thresholds (6), stat bitmask powers-of-2 (6), special item IDs (3),
stat index identity table (6), damage thresholds (6, `{128,128,128,128,128,0}`),
two 4-entry per-class parameter tables (`{4,18,11,25}` /
`{0,5,40,26}`), combat type indices (7), combat flag bits (4,
`{32,16,8,4}`), and combat action permutation (5, `{0,4,2,5,1}`). These are
raw extracted bytes, not reconstructed algorithms — the surrounding
combat-resolution logic in `nexus_v1_combat.c` is Firestaff's own
reimplementation built on top of them.

---

## 7. Key Data Structures

| Struct | Header | Role |
|---|---|---|
| `Nexus_V1_DgnDecodeResult` | `nexus_v1_dgn.h` | Top-level per-level DGN decode (s1/s2/s3 sections, counts, hashes) |
| `Nexus_V1_DgnCell` | `nexus_v1_dgn.h` | One 64×64-grid cell (floor/height/model ref/door) |
| `Nexus_V1_DgnFloorItem/FloorDecor/FloorSensor/Alcove/WallDecor/WallSensor` | `nexus_v1_dgn.h` | Structure1F fixed-size sub-records |
| `Nexus_V1_0DmstrtStructureReceipt` | `nexus_v1_0dmstrt_structure_admission.h` | Boot-image byte-partition provenance receipt |
| `Nexus_V1_BpkArchiveInfo/BpkEntry/BpkEntryPrefix` | `nexus_v1_bpk_archive.h` | MENU.BPK directory/entry framing |
| `Nexus_V1_BpkPrs3Info/StreamPlan/PayloadEvidence/CandidateEvidence/FramedEvalEvidence/OpcodePrefixEvidence/DecodedOutputProofReceipt` | `nexus_v1_bpk_archive.h` | PRS3 evidence ladder (never a working decoder) |
| `Nexus_V1_BpkRuntimeRenderReceipt/DecodeReceipt/UploadReceipt/UploadRow` | `nexus_v1_bpk_archive.h` | Fail-closed render/decode/upload routing |
| `Nexus_V1_MnsJoint/MnsMesh/MnsVertex/MnsFace/MnsTextureDesc` | `nexus_v1_mns.h` | MNS skeletal mesh |
| `Nexus_V1_MnsMotnFrame/MnsMotnTable/MnsAnimState/MnsJointPose/MnsPose` | `nexus_v1_mns.h` | MNS keyframe animation runtime |
| `Nexus_V1_ItemIbsDecl` | `nexus_v1_item_ibs.h` | Item declaration (40-byte record) |
| `Nexus_V1_ItemIbsFloorDesc` | `nexus_v1_item_ibs.h` | Floor-item image descriptor |
| `Nexus_V1_ItemIbsDecodeResult` | `nexus_v1_item_ibs.h` | Full ITEM.IBS decode (items, palettes, image hashes) |
| `Nexus_CreatureType` | `nexus_v1_creatures.h` | Creature roster entry (stats + CRET + MNS binding) |
| `Nexus_Creature` | `nexus_v1_creatures.h` | Live creature instance (DGN actor provenance fields) |
| `Nexus_V1_CreatureActorBinding` | `nexus_v1_creatures.h` | Evidence-gated actor-model → roster-type binding |
| `Nexus_V1_CreatureManager` | `nexus_v1_creatures.h` | Roster + active-pool + binding-table container |

---

## 8. Parity Evidence

Nexus-related documents in `parity-evidence/`:

- `parity-evidence/nexus/` — directory containing `canonical_hashes.txt`
  (pinned SHA-256 hashes of retail Nexus assets) and `file_manifest.json`
  (disc file inventory).
- `parity-evidence/nexus_v1_runtime_screenshot_readiness.md`
- `parity-evidence/nexus_v1_track1_real_screen_capture_readiness.md`
- `parity-evidence/nexus_v1_water_fire_crossing_gate_disassembly.md`
- `parity-evidence/pass216_nexus_saturn_hardware_evidence.md`
- `parity-evidence/external/dm_nexus_dmweb_cheats_hacks_ref.md` (external DMWeb reference notes)
- `parity-evidence/verification/nexus_v1_runtime_screenshot_readiness/`,
  `parity-evidence/verification/nexus_v1_track1_real_screen_capture_readiness/`,
  `parity-evidence/verification/nexus_v2_verification_suite_source_lock.json`
  — machine-checkable verification artifacts backing the above passes.

12 markdown files under `parity-evidence/` mention "nexus" in content
(cross-references from shared/DM1 passes), while the files above are the
Nexus-specific pass documents proper. Individual `nexus_*_pc34_compat`-style
`pass{NNN}` documents (the numbering scheme used heavily for DM1/CSB) are
not used for Nexus; instead Nexus evidence is tracked as named readiness/
provenance documents plus the `pass216` hardware-evidence baseline, since
there is no ReDMCSB-equivalent reference source to diff against — every
claim in these documents must be self-supporting from disc/binary bytes
alone (hash pins, structural admission, byte partitions), which is why the
BPK/PRS3 evidence ladder in section 4 is built as a chain of independent,
non-decoding receipts rather than a single decoder implementation.
