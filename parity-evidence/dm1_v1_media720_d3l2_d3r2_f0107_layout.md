# DM1 PC 3.4 MEDIA720 D3L2/D3R2 F0107 layout evidence

The source is the in-place member `DATA/GRAPHICS.DAT` of
`~/.firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip`, SHA256
`2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e`.
No member was extracted to disk.

ReDMCSB `DUNVIEW.C` F0107 selects zone
`C1004 + CoordinateSet*15 + ViewWall`, F0676/F0677 pass view walls 0/1,
and `COORD.C` F0635 resolves the zone using the scaled bitmap dimensions.
Item 696's authenticated records are:

| set | D3L2 `(type,x,y)` | D3R2 `(type,x,y)` |
| ---: | --- | --- |
| 0 | `(0,31,42)` | `(0,192,43)` |
| 1 | `(0,22,48)` | `(0,202,50)` |
| 2 | `(7,30,67)` | `(7,191,66)` |
| 3 | `(0,22,51)` | `(0,202,51)` |
| 4 | `(0,22,42)` | `(0,202,43)` |
| 5 | `(0,24,45)` | `(0,202,44)` |
| 6 | `(7,19,75)` | `(7,206,75)` |
| 7 | `(5,18,41)` | `(5,205,38)` |

Record type 0 centers both axes, type 7 centers X and bottom-aligns Y, and
type 5 centers X and top-aligns Y, exactly as F0635 specifies. F0107 scales
these outer projections at 30/32 horizontally and 14/32 vertically, applies
the D3 palette, flips D3R2 only, and uses color 10 as transparent.

Item 558 is not this data. At absolute GRAPHICS.DAT offset `0x25eed` it is a
38-byte compressed/decompressed 16x7 image record with SHA256
`df28c5d26e9a7b87903ac817a21675c65ad52216e5deede4148f5286f4223e23`.

## Hall of Champions mirror material

The live viewport registry names the two source-owned outer records with
internal indices 13 and 14. They are not aliases for G0205 rows 0 and 1.
Champion-mirror admission must consequently accept `(D3,-2,13)` and
`(D3,+2,14)` before applying the ordinary F0107 C127 test. Both projections
consume the unscaled D3 base ornament, C345; the item-696 rows supply its
position, not an ornament-depth increment.

`m11_dm1_hoc_real_mirror_viewport_material` launches the unmodified PC 3.4
archive, finds all 15 authentic C127 mirror placements visible in these two
lanes, and requires an admitted material receipt backed by C345. Before this
correction all 15 reported `material=0`, `backing=-1`, and `suppress=1`.
The corrected runtime reports `material=1`, `backing=345` for the same real
placements. No generated dungeon, mirror record, or bitmap participates in
the test.
