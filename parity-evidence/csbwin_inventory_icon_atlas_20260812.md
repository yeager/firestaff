# CSBWin C017 inventory icon atlas

## Scope

This record documents the Atari ST / CSBWin inventory pixels that Firestaff
may compose after the authentic C017 inventory surface has been decoded. It
does not establish inventory input, item transfer, attack-hand animation, or
general CSBWin save compatibility.

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

`CSBWin/Objects.h` fixes those object-name indices: `Chest=0x90`,
`OpenChest=0x91`, `OpenScroll=0x1e`, `Scroll=0x1f`, `Special_f=0xcc`,
`Special_j=0xd0`, and `Special_n=0xd4`.

## Firestaff boundary

`m11_csb_compose_csbwin_inventory_icons` uses only decoded C017, C033/C034,
C042–C048 and the C232 record from the selected, hash-admitted CSBWin media.
If any C232 destination, atlas crop, or source graphic cannot be resolved,
the complete Atari HUD composition rejects instead of substituting a PC3.4 or
host icon.

The real-data CTest `csb_v1_m11_prison_runtime_hud_pc34` reconstructs the
whole 224×136 C017 aperture from the same authentic graphics file and checks
all thirty icon destinations against M11's final framebuffer. Its expected
frame includes empty-slot symbols, wounded-slot variants, C033/C034 frames,
and the selected-hand chest/scroll presentation rule.

## Still open

The source's attacking-hand C035 overlay depends on live
`AttackingCharacterOrdinal`, which is not yet carried by Firestaff's admitted
Atari runtime receipt. It remains intentionally undrawn rather than inferred.
