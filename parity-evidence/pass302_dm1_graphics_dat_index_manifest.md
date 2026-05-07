# Pass302 — DM1 GRAPHICS.DAT index manifest for pass298

Status: **passed**. Pixel parity is not claimed, and no bitmap dump files are emitted.

This is a bounded source/data inventory for the three original `GRAPHICS.DAT` records required by pass298 entry-viewport bitmap materialization:

- `78` — `floor.base` / `M650_GRAPHIC_FLOOR_SET_0_FLOOR` / `G2108_Floor` / `C701_ZONE_VIEWPORT_FLOOR_AREA`
- `79` — `ceiling.base` / `M651_GRAPHIC_FLOOR_SET_0_CEILING` / `G2109_Ceiling` / `C700_ZONE_VIEWPORT_CEILING_AREA`
- `107` — `wall.front.blocking.d3c` / `C107_GRAPHIC_WALLSET_0_D3C` / `G2107_WallSet[C14_WALL_D3C]` / `C704_ZONE_WALL_D3C`

Canonical source:

- `<firestaff-original-games>/_canonical/dm1/GRAPHICS.DAT`
- SHA-256: `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e`
- Header: signature `0x8001`, item count `713`, header bytes `5708`, file bytes `363417`

Deterministic records are in `parity-evidence/verification/pass302_dm1_graphics_dat_index_manifest.json` and include file offset, compressed byte count, width/height, compressed-record SHA-256, IMG3 packed SHA-256, and unpacked 1-byte-per-pixel SHA-256.

Verification:

```sh
python3 -m py_compile tools/verify_pass302_dm1_graphics_dat_index_manifest.py
python3 tools/verify_pass302_dm1_graphics_dat_index_manifest.py
```
