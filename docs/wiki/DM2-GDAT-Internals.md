# DM2 GDAT Internals

## Reference Model

DM2 V1 follows skproject's typed graphics and runtime model. `GRAPHICS.DAT`
is not treated as a flat sprite sheet: typed GDAT records define palette,
interface, title, map graphics, map-chip, and placement ownership.

## Boot Admission and Failure Semantics

The DM2 boot profile is valid only after both supported original graphics and
dungeon hashes resolve. M11 receives a boot receipt instead of probing files
itself. If the receipt lacks required source visuals, launch remains at M12;
the viewport is not cleared and no generated dungeon floor/ceiling is drawn.

The title route specifically requires the original `TITLE/0 dt07/4` surface.
Failure to materialize that surface is a start failure, not permission to show
a placeholder title or continue into an AI-generated menu.

## Palette Chain

The renderer binds indexed GDAT pixels through the source chain:

```text
dtPalIRGB -> dtPalette16 -> selected material/interface palette -> framebuffer
```

Palette metadata alone is not consumption. A render receipt records the raw
GDAT address, decoded material, palette binding, source rectangle, destination
rectangle, and final draw category. Door ornaments and destroyed-door masks use
this material path; raw index writes are not accepted as a GDAT draw.

## Graphics Sets and Map Chips

Each map chooses a `GRAPHICSSET`. The active style participates in GDAT address
resolution and cache identity, so a wall decoded for one style cannot be reused
on a map selecting another style. The runtime consumes typed material classes
for floor, ceiling, wall, door panel/frame/overlay, creature, item, carried
item, and projectile map chips.

For indoor floor and ceiling, `UPDATE_GFXSET` now retains the validated,
decoded pair through the M11 frame. The viewport consumes those exact plan
pixels and their per-image local palettes directly; it does not issue a second
asset callback that could accidentally select a different graphics set. A plan
whose graphics-set index or command hash no longer matches the active map is a
blocked no-draw frame: it does not consult the callback route or paint a
fallback plane.

The M11 plan also materializes the only established `QUERY_BLIT_RECT` grammar
slice for viewport planes. `INTERFACE_GENERAL/0/dt04/0` ceiling record 700
and floor record 701 must resolve through source root anchors 11 and 14,
respectively, via the `x=1` reference to the `(0,0,224,136)` clip record. This
yields the ceiling at `(0,0)` and the floor bottom-aligned to that viewport;
the renderer draws the ceiling before the floor. Chained variants, alternate
anchors, source crops, and additional clips remain rejected and produce no
source-material draw.

If a required material class cannot be resolved, the frame records a blocked
no-draw receipt. It must not paint a conventional-color approximation.

The direct G1 creature scene path is also corpus-gated: the root `DB4` record's
`b4` creature type selects `CREATURES/<type>/F9` through the map-chip virtual
address. The runtime handoff accepts only the decoded source image and its
local palette. Generic terrain resolution is not consulted for this root
class; an absent or rejected F9 image yields a blocked handoff rather than a
substitute creature.

## G1 Byte-Square Format and Record Graph

The G1 map format was long ambiguous about its `w0` field. DM2-001 proved by
corpus analysis that on real PC G1 `DUNGEON.DAT`, `GenericRecord::w0` is
**game data** (e.g. a creature's stored state byte), not a next-record link.
The `record_graph_complete` flag (in `dm2_v1_dungeon_loader.h` and
`dm2_v1_record_pool_pc34_compat.h`) reflects whether the loader proved a
complete, resolvable record graph for the loaded dungeon:

- `record_graph_complete = 1` on the real 39,437-byte PC `DUNGEON.DAT` (28
  maps, 2,859 records, 2,360 ground-stack entries): every ground-stack entry
  resolves to a record, including entries in the G1 extension pools.
- The companion `g1_w0_chains_disabled` flag (`dm2_v1_dungeon_loader.h:1216`)
  disables `w0`-as-next-link traversal specifically when the loaded data is
  real PC G1 data (`dm2_v1_dungeon_loader.c:487`) — because on real data
  `w0` is not a chain link. Synthetic skproject test fixtures do not set this
  flag, so their `w0` chains keep resolving as before, and existing
  fixture-based tests are unaffected.
- `get_thing_record` resolves DB3/DB4 extension records; `get_next_thing`
  returns `END_MARKER` for the G1 format (there is no next-thing chain to
  walk on real data).

### Extension records

The ground-stack table's declared capacity exceeds the number of directly
typed roots. Corpus proof (`parity-evidence`, DM2-001 analysis) established
the extension-pool layout in the real PC G1 `DUNGEON.DAT`:

- **DB3 extension**: byte range `[23826, 29626)`, 8 bytes each, extending the
  DB3 index space from 299 to 1024 (indices `299..1023`).
- **DB4 extension**: byte range `[29626, 31658)`, 16 bytes each, extending
  the DB4 index space from 173 to 300 (indices `173..299`).
- A final 9-byte tail remains untyped/unused.

Do not derive further record-type transforms or widen these ranges without
new corpus-plus-source evidence; only DB3 and DB4 have proven extension
layouts.

## Interface Tables

`dt07/2` interface data is materialized as bounded primary, secondary, and
command-tail spans. `dt07/0x0A` Rect14 placement data is decoded separately and
carried from boot to the runtime host receipt. This makes placement provenance
available to the HUD consumer without re-parsing raw GDAT in M11.

The host receipt is fail-closed: missing, truncated, or inconsistent Rect14
data cannot be presented as a source-backed HUD layout.

## Creature V5 Animation and Occupancy

The direct G1 creature scene path is corpus-gated (see Graphics Sets above
for the DB4/F9 handoff). Beyond the base F9 image handoff, the runtime now
carries occupancy and animation evidence:

- `DM2_V1_G1CreatureV5RuntimeReceipt` carries the creature's V5 animation
  chain evidence for the render plan. All 33 direct DB4 roots in the proven
  corpus stay fail-closed for their V5 images — those images are 8bpp and
  require a separate decode path that is not yet source-locked; the receipt
  records this rather than substituting a placeholder frame.
- `DM2_V1_G1DirectCreatureRoot` carries the record-owned cursor used to walk
  creature state without touching `w0` chain semantics.

Occupancy grid (`dm2_v1_viewport_creature_occupancy_5x5` and
`dm2_v1_viewport_occupancy_grid_coords`, source-locked against
`SkWinCore.cpp`'s `QUERY_CREATURE_5x5_POS` and `DRAW_STATIC_OBJECT`'s
occupancy walk):

- Encodes a 5x5 position grid relative to the party, plus a display-order
  index used to decide draw order among creatures sharing a cell region.
- Direction resolution follows `(party_dir - creature_dir) & 3` exactly, per
  `SkGlobal.cpp`'s direction tables.
- `DRAW_FLYING_ITEM` selection consumes this occupancy evidence to pick the
  correct scale and image fields for a flying (thrown/dropped-in-air) item,
  rather than a fixed default scale.

Tests: `tests/test_dm2_v1_creature_occupancy_flying_item.c` (34/34),
`tests/test_dm2_v1_g1_creature_v5_animation.c` (38/38, name approximate — see
`test_dm2_v1_creature_occupancy_flying_item` binary), and probe
`probes/dm2/firestaff_dm2_v1_creature_occupancy_probe.c`.

## Combat Drops

`dm2_v1_drops_resolve_gdat_creature_drops()` reads the eleven-entry
per-creature drop table directly from real GDAT data (not a hardcoded
table) and resolves it to concrete drop words using RNG-gated selection
that mirrors `SkWinCore.cpp`'s creature-death drop route. This is part of
the combat/defense route: a creature's death drop is only materialized when
the GDAT-sourced table and RNG gate both resolve; there is no synthetic
fallback drop. Verified by `test_dm2_v1_drops_gdat_real_data` (skip-safe
without real data present).

## Sound: PCM Decode and Voice Allocation

`dm2_v1_sound_decode_gdat_pcm()` decodes the unsigned 8-bit mono PCM payload
of a GDAT sound entry:

- Format: unsigned 8-bit mono, 6000 Hz sample rate, per a two-byte format
  header preceding the raw sample data.
- Conversion: each raw byte is converted `byte ^ 0x80`, matching SKWin's
  `0x80 + raw_byte` alloc-time conversion exactly (XOR and addition are
  equivalent for the top bit flip used here).

Voice allocation owns `MAX_SB = 16` voice slots (SKWin's `MAX_SB` constant).
Playback binds through an SDL3 backend: a 6000 Hz U8 mono stream with
additive `sdlAudMix`-shaped mixing across active voices, matching the
original's software-mixing behavior rather than resampling to a modern
rate.

`dm2_v1_sound_bind_gdat_loader()` wires the `DM2_PLAY_MUSIC`,
`DM2_PLAY_SOUND`, and `DM2_QUERY_SND_ENTRY_INDEX` entry points to this real
GDAT-backed decode and mixing path. `DM2_QUERY_SND_ENTRY_INDEX` keeps the
original 1-based linear scan semantics.

Tested by `test_dm2_v1_sound_gdat_real_data` (PCM-decode and
voice-allocation coverage).

## Scene and Weather State

M10 owns current map, party, door, creature, item, missile, and weather state.
Indoor frames publish clear conditions. Outdoor frames route the actual runtime
weather words and source graphics-set sky/ground material. Weather control data
does not create procedural rain/mist/thunder pixels until a verified original
overlay asset is consumed.

## Complete-Support Receipt

The receipt combines, but does not conflate:

* source title/menu consumption;
* HUD palette, portrait, interface, and Rect14 evidence;
* dungeon floor/ceiling/wall/map-chip evidence;
* creature/door/item/projectile render categories;
* save corpus scan facts;
* no-fallback visual state.

It is valid only if every mandatory source-backed component is valid. A green
unit test for one material class is not a full-playability claim.

## Verification

```bash
./build/test_dm2_v1_boot_profile_smoke
./build/test_dm2_v1_runtime_handoff_smoke
./build/test_dm2_v1_lighting_falloff_boundary
./build/test_dm2_v1_weather_no_synthetic_overlay
FIRESTAFF_DM2_DATA_DIR="$HOME/.firestaff/data/dm2/data" \\
  ./build/test_dm2_v1_gdat_scene_plan_viewport_real_data
FIRESTAFF_DM2_DATA_DIR="$HOME/.firestaff/data/dm2/data" \\
  ./build/test_dm2_v1_g1_scene_creature_gdat_real_data
./build/test_dm2_v1_creature_occupancy_flying_item
./build/test_dm2_v1_drops_gdat_real_data
./build/test_dm2_v1_sound_gdat_real_data
```
