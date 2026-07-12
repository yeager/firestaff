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

If a required material class cannot be resolved, the frame records a blocked
no-draw receipt. It must not paint a conventional-color approximation.

## Interface Tables

`dt07/2` interface data is materialized as bounded primary, secondary, and
command-tail spans. `dt07/0x0A` Rect14 placement data is decoded separately and
carried from boot to the runtime host receipt. This makes placement provenance
available to the HUD consumer without re-parsing raw GDAT in M11.

The host receipt is fail-closed: missing, truncated, or inconsistent Rect14
data cannot be presented as a source-backed HUD layout.

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
```
