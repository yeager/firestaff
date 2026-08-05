# DM1 PC 3.4 asset audit: Greatstone and DMWeb

Date: 2026-08-05

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

Focused title tests pass: 56/56 palette/step invariants, 371 C001 fallback
checks, the 53-frame real `TITLE` decode probe, and the ReDMCSB cadence gate.

## Not yet proven

This does not claim that all 713 records have been visually compared or that
the packaged macOS window matches DOSBox pixel-for-pixel. The remaining audit
is a complete 713-record decoded-pixel comparison plus a real Mac app capture.
