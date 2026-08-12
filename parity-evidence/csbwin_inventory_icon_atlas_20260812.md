# CSBWin C017 inventory icon atlas

## Scope

This record documents the Atari ST / CSBWin inventory pixels and C232-owned
slot hit testing that Firestaff may use after the authentic C017 inventory
surface has been decoded. It does not establish chest contents, attack-hand
animation, or general CSBWin save compatibility.

## Original owner

`CSBWin/Character.cpp::DisplayBackpackItem` is the pixel owner.

- The selected character maps inventory item `n` to `IconDisplay[n + 8]`.
- The destination rectangle is the source coordinate minus one, with a 16×16
  `DrawItem` crop.
- `DrawItem` selects C042–C048 by C232's `Word612` first-object table.
- Slots 0–5 first receive C033 or C034 with transparent index 12; the choice
  follows the corresponding `ouches` bit.
- Empty slots still call `DrawItem`: slots 0–5 use `Special_n + 2*n` and the
  wounded neighbour; slots 10–13 use `Special_j` through `Special_m`; every
  other empty slot uses `Special_f`.
- In selected slot 1 only, `Chest` becomes `OpenChest` and `OpenScroll`
  becomes `Scroll` before the atlas crop is chosen.
- The same `IconDisplay[n + 8]` location starts the source's inclusive 18×18
  inventory click rectangle.
- `CSBCode.cpp::TAG001c6e` expands raw `GRAPHICS.DAT` item `0x822d` into the
  768-byte M653 font before inventory text can render. `ShowHideInventory`
  prints `HEALTH`, `STAMINA` and `MANA`; `Character.cpp::PrintLifeForces`
  prints HP, stamina divided by ten and mana as current/max pairs. All calls
  use `TextToViewport`, whose `TextOut_OneLine` destination begins at `y-4`.

`CSBWin/Objects.h` fixes those object-name indices: `Chest=0x90`,
`OpenChest=0x91`, `OpenScroll=0x1e`, `Scroll=0x1f`, `Special_f=0xcc`,
`Special_j=0xd0`, and `Special_n=0xd4`.

## Firestaff boundary

`m11_csb_compose_csbwin_inventory_icons` uses only decoded C017, C033/C034,
C042–C048 and the C232 record from the selected, hash-admitted CSBWin media.
If any C232 destination, atlas crop, or source graphic cannot be resolved,
the complete Atari HUD composition rejects instead of substituting a PC3.4 or
host icon.

`csb_v1_csbwin_layout_0232_inventory_slot_at_point` derives the active raw
C00..C29 slot directly from those C232 records at C017's `(48,33)` source-page
origin. M11 resolves that path before its generic live CSB command geometry,
so a C017 slot cannot be misclassified as a PC3.4 C507 zone or a viewport
click. The resulting item transaction remains the existing runtime-owned
inventory writer and rejects when its source-owned placement rules reject.

`Mouse.cpp` treats the C232 eye and mouth boxes as controls separate from the
item-slot list. In the verified CSBWin media the mouth box occupies the same
top-band coordinate range as the normal champion-status route. While C017 is
active, M11 therefore resolves those C232 controls before the top-row
dispatcher. An empty-hand mouth click remains on the existing source-command
state path, and `ShowHideInventory` clears that transient page state whenever
the selected champion changes or the inventory closes. The food-and-water
raster is now source-bound as well: a 32-bit CSBWin layout probe resolves
`DBank::Byte1832` at 26696 and `wRectPos926/950/958/966` at
27602/27578/27570/27562. Since `CSBCode.cpp` expands C232 at `Byte1832 + 2`,
the raw C232 offsets are 904/880/872/864, matching the decoder. On the active
mouth path, Firestaff copies C020/C030/C031 to those rectangles and reproduces
`Viewport.cpp::DrawFoodWaterBar`: food at
`(113,69)` in colour 5, water at `(113,92)` in colour 14, signed warning
colours 11/8, and the two-pixel black tail. The real-data CTest performs the
decoded C232 click and validates C030 plus the resulting bar pixels in the
final framebuffer.

C032 är medvetet inte bunden ännu: originalet använder `CHARDESC::poisonCount`,
inte den generella Firestaff-spegelns `poisonDose`. Att göra den likställningen
skulle vara syntetisk data. Kravet är därför en utökad CSBWin runtime-receipt
med det faktiska fältet.

The real-data CTest `csb_v1_m11_prison_runtime_hud_pc34` reconstructs the
whole 224×136 C017 aperture from the same authentic graphics file and checks
all thirty icon destinations against M11's final framebuffer. Its expected
frame includes empty-slot symbols, wounded-slot variants, C033/C034 frames,
and the selected-hand chest/scroll presentation rule.

The data-free C232 layout regression checks slot 0's exact top-left and
bottom-right inclusion plus its adjacent miss. The available legacy real save
contains no occupied inventory slot, so it cannot independently prove a
pickup mutation without fabricating a test item; it still exercises authentic
startup, C017 composition and the real C232 decoder.

The same real-data CTest now also reads raw item `0x22d` through the native
DMCSB1 LZW envelope, requires its exact 768-byte size, and rebuilds all three
life-force rows in the expected C017 aperture with the source M653 glyphs.
It compares those glyphs and source's unusual fixed three-column numeric
format directly with M11's final framebuffer. A missing, malformed or
wrong-sized raw font cannot select a PC3.4 record or a host font; it simply
leaves the source text absent while the independently verified C017 raster
remains available.

## Still open

The source's attacking-hand C035 overlay depends on live
`AttackingCharacterOrdinal`, which is not yet carried by Firestaff's admitted
Atari runtime receipt. It remains intentionally undrawn rather than inferred.
