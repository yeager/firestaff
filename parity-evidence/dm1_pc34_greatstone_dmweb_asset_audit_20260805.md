# DM1 PC 3.4 asset audit: Greatstone and DMWeb

Date: 2026-08-06

## Sources

- Greatstone `GRAPHICS.DAT`: http://greatstone.free.fr/dm/db_data/dm_pc_34/graphics.dat/graphics.dat.html
- Greatstone `TITLE`: http://greatstone.free.fr/dm/db_data/dm_pc_34/title/title.html
- DMWeb data files: http://dmweb.free.fr/community/documentation/file-formats/data-files/
- DMWeb dungeon files: http://dmweb.free.fr/community/documentation/file-formats/dungeon-files/
- DMWeb saved-game files: http://dmweb.free.fr/community/documentation/file-formats/saved-game-files/
- DMWeb PC notes: http://dmweb.free.fr/community/documentation/miscellaneous/dungeon-master-for-pc/
- DMWeb Atari ST history: http://dmweb.free.fr/community/documentation/miscellaneous/dungeon-master-for-atari-st-history/

## Findings

The local DOS PC 3.4 package contains the authenticated source data used by
the DM1 runtime:

- `GRAPHICS.DAT`: 713 records, 363417 bytes, SHA-256
  `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e`.
- `TITLE`: 12002 bytes, SHA-256
  `adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745`.
- `TITLE` manifest: 59 records: one `AN`, one `BR`, one `P8`, two `PL`, two
  `EN`, 51 `DL`, and one `DO`; 53 decoded 320x200 frame records.

The normal PC/F20 title path consumes real `GRAPHICS.DAT` record 1
(`C001_GRAPHIC_TITLE`, 320x200). The `TITLE` file is an authenticated
fallback/provenance decoder, not a synthetic replacement for C001.

The existing local SCK/Greatstone pixel comparison covers 22 critical records
and reports zero differing pixels. It includes the title source, action/spell
HUD surfaces, champion panels, item sheet, floor/ceiling, door frame, wall
depths, stairs, wall ornaments, and projectile/object sources.

On 2026-08-06 the complete Greatstone `GRAPHICS.DAT` index was fetched to a
temporary directory outside the repository. It contains 543 published PNG
entries: 542 `IMG3` raster records and the separate `0695.FNT1` interface
font. The local M11 decoder was compared against all 542 published `IMG3`
records using decoded indexed pixels and dimensions. Result: **542/542 exact
matches**, with zero dimension or pixel-digest differences. No downloaded
reference media was added to the repository.

Greatstone's `0696` entry is labelled unknown word data and has no `IMG3`
reference image. ReDMCSB `COORD.C` F0640 identifies it as
`C696_GRAPHIC_LAYOUT`, the original `0xFC0D` layout-range table. It is
classified as non-raster source data and never enters the generic bitmap path.
`0695` is consumed by the source-bound 1bpp interface-font loader. The
authoritative SND3 records are also rejected before IMG3 decoding.

Focused title tests pass: 56/56 palette/step invariants, 371 C001 fallback
checks, the 53-frame real `TITLE` decode probe, and the ReDMCSB cadence gate.

## Not yet proven

This proves every published Greatstone `IMG3` raster reference, but does not
claim that all 713 records have a public raster reference or that the packaged
macOS window matches DOSBox pixel-for-pixel. Remaining scope is the source
consumer/capture for special records (especially 0696) and a real packaged
macOS app capture.
