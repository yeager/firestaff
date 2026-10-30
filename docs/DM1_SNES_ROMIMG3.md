# DM1 SNES ROMIMG3 graphics

`ROMIMG3` is a tile-based SNES graphics format. It is separate from
Firestaff's existing `IMG5` decoder: ROMIMG3 stores 8x8 tiles, with planes
0 and 1 interleaved per row, followed by planes 2 and 3 in the same order.
Within each byte, bit 7 is the leftmost pixel.

`FirestaffSnesRomimg3_DecodeTiles` converts a row-major tile grid into
row-major 4bpp pixel indexes. `FirestaffSnesRomimg3_ApplyPaletteSelectors`
applies the `TILESBYITEM` grouping and `PALSEL` values specified by the
SCK map. It produces indexes into the decoded palette; it does not load
SNES graphics from a ROM or resolve palette colors.

## Reference and verification

The SCK `dm_snes_en.map` assigns the following real DM1 SNES assets:

| Map ID | Dimensions | Selector metadata |
| --- | --- | --- |
| `999477` | 16 x 16 tiles | none |
| `999470` | 2 x 64 tiles | `PALSEL1`, 4 tiles per item |
| `999472` | 2 x 64 tiles | `PALSEL3`, 4 tiles per item |

The decoder output was compared byte-for-byte with SCK's indexed BMP export
for all three assets. The 999477 pixel-index SHA-256 is
`583cdd5d1e35098057018140cdda25dbceb41febe004671dbccbe8e6d9dfd1e9`; 999470
is `489172bcc44cdeca1c6c7665bc66b202f91ef34d04e9819f35649bba7c23fabd`; and
999472 is `beef5d3f5f7e54065029cace17f066f9335081ec83205f1a8d427c4e43f02ebf`.
The ROM images and extracted SCK outputs remain local test data and are not
part of the repository.

## Remaining integration work

The current code is a bounded pixel decoder, not a complete SNES graphics
loader. The ROMGH table and SNES banked data stream still need a proven
bounded resolver from a logical map item to its source bytes. Palette resource
loading and integration into the DM1 SNES renderer are also outstanding.
Keep authentic ROM media outside Git; use the local real-media test pattern
and SCK exports as reference evidence when implementing those pieces.
